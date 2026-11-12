# Copyright AStarship <https://astarship.net>.
"""Single-process browser host. Sessions are ephemeral, server-side play chips."""
from pathlib import Path
import secrets
import threading
from flask import Flask, jsonify, request, send_from_directory

try:
  from .cards_engine import Table, BlackjackRules, PokerRules
except ImportError:
  from cards_engine import Table, BlackjackRules, PokerRules


Root = Path(__file__).resolve().parent


def CreateApp():
  app = Flask(__name__, static_folder=None)
  app.config['MAX_CONTENT_LENGTH'] = 4096
  tables = {}
  lock = threading.RLock()

  @app.after_request
  def Headers(response):
    response.headers['Cache-Control'] = 'no-store'
    response.headers['X-Content-Type-Options'] = 'nosniff'
    return response

  @app.get('/')
  def Index():
    return send_from_directory(Root, 'cards.html')

  @app.get('/assets/cards/<path:filename>')
  def CardAsset(filename):
    return send_from_directory(Root / 'assets' / 'cards', filename)

  @app.get('/api/manifest')
  def Manifest():
    return jsonify({
      'name': 'Cards World', 'version': 1,
      'rules': {'blackjack': BlackjackRules, 'poker': PokerRules},
      'observation': {
        'fields': ['game', 'phase', 'done', 'message', 'player', 'opponent',
                   'board', 'player_total', 'opponent_total', 'bankroll',
                   'opponent_bankroll', 'pot', 'actions', 'rules'],
        'cards': 'A,2..10,J,Q,K followed by c,d,h,s; hidden cards are back.',
        'visibility': 'Dealer hole card hidden until done; poker bot cards only at showdown.',
        'totals': 'Poker totals and hidden dealer total are null.',
        'phases': {'blackjack': ['player', 'done'],
                   'poker': ['preflop', 'flop', 'turn', 'river', 'done']}},
      'actions': {'blackjack': ['hit', 'stand', 'double'],
                  'poker': ['fold', 'check', 'call', 'raise'],
                  'protocol': 'POST /api/action {action:string}; only listed state actions legal. '
                              'POST /api/new {game:blackjack|poker} when done or switching mode.'},
      'rewards': 'Round reward = final bankroll minus bankroll before ante/blinds. '
                 'Blackjack normal win +10, loss -10, natural +15, push 0; double +/-20. '
                 'Poker reward is net chips won/lost. Initial stacks 200 each. '
                 'Mode switch resets both stacks to 200. Play chips only; no monetary value.',
      'sessions': 'Opaque HttpOnly cookie; state in this server process; restart clears sessions.'})

  def Dispatch(operation):
    payload = None
    if operation != 'state':
      payload = request.get_json(silent=True)
      key = 'game' if operation == 'new' else 'action'
      if (not isinstance(payload, dict) or set(payload) != {key} or
          not isinstance(payload[key], str)):
        return jsonify(error='Expected a JSON object containing only ' + key), 400
      if operation == 'new' and payload['game'] not in ('blackjack', 'poker'):
        return jsonify(error='Unknown game'), 400
    with lock:
      token = request.cookies.get('cards_session')
      fresh = token not in tables
      if fresh:
        token = secrets.token_urlsafe(32)
        table = Table()
      else:
        table = tables[token]
      try:
        if operation == 'new':
          table.New(payload['game'])
        elif operation == 'action':
          table.Action(payload['action'])
      except ValueError as error:
        return jsonify(error=str(error)), 400
      tables[token] = table
      response = jsonify(table.State())
      if fresh:
        response.set_cookie('cards_session', token, httponly=True,
                            samesite='Strict', secure=request.is_secure)
      return response

  @app.get('/api/state')
  def State():
    return Dispatch('state')

  @app.post('/api/new')
  def New():
    return Dispatch('new')

  @app.post('/api/action')
  def Action():
    return Dispatch('action')

  return app


app = CreateApp()

if __name__ == '__main__':
  app.run(host='0.0.0.0', port=5003, debug=False)
