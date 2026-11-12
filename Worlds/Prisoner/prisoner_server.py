# Copyright AStarship <https://astarship.net>.
"""Prisoner's Dilemma browser demo — Flask server on port 5008.

Serves a UI for two-agent repeated Prisoner's Dilemma.
Sessions are isolated; state lives in the server process only.
No real shell access, no filesystem, no secret reads.
"""
import sys
import secrets
import threading
from pathlib import Path

from flask import Flask, jsonify, request, send_from_directory

# Ensure the engine is importable regardless of execution context
_ENGINE_PATH = str(Path(__file__).resolve().parent)
if _ENGINE_PATH not in sys.path:
  sys.path.insert(0, _ENGINE_PATH)

from prisoner_engine import PrisonerGame

Root = Path(__file__).resolve().parent


def CreateApp():
  app = Flask(__name__, static_folder=None)
  app.config['MAX_CONTENT_LENGTH'] = 4096
  sessions = {}
  lock = threading.RLock()

  @app.after_request
  def Headers(response):
    response.headers['Cache-Control'] = 'no-store'
    response.headers['X-Content-Type-Options'] = 'nosniff'
    return response

  @app.get('/')
  def Index():
    return send_from_directory(Root, 'prisoner.html')

  @app.get('/assets/<path:filename>')
  def Asset(filename):
    return send_from_directory(Root / 'assets', filename)

  @app.get('/api/manifest')
  def Manifest():
    return jsonify({
      'name': 'PrisonerWorld browser demo',
      'version': 1,
      'rules': {
        'description': 'Repeated Prisoner\'s Dilemma; two agents choose Cooperate or Defect each round.',
        'payoff_matrix': {'T': 5, 'R': 3, 'P': 1, 'S': 0},
        'max_rounds': 64,
      },
      'observation': {
        'fields': ['round', 'player_total', 'ai_total', 'player_action', 'ai_action',
                   'done', 'ai_strategy'],
        'meaning': {
          'round': 'current round (0..63)',
          'player_total': 'cumulative player reward',
          'ai_total': 'cumulative AI reward',
          'player_action': '0=Cooperate, 1=Defect',
          'ai_action': '0=Cooperate, 1=Defect',
          'done': 'episode ended',
          'ai_strategy': 'current AI strategy',
        },
      },
      'actions': {
        'allowed': ['cooperate', 'defect'],
        'schema': {'action': ['cooperate', 'defect']},
      },
      'sessions': 'Opaque HttpOnly cookie; state in this server process; restart clears.',
    })

  @app.get('/api/state')
  def State():
    with lock:
      token = request.cookies.get('prisoner_session')
      if token not in sessions:
        token = secrets.token_urlsafe(32)
        sessions[token] = {
          'game': PrisonerGame(),
          'last_valid_state': {},
        }
      session = sessions[token]
      state = session['game'].state()
      session['last_valid_state'] = state.copy()
    return jsonify(state)

  @app.post('/api/action')
  def Action():
    with lock:
      token = request.cookies.get('prisoner_session')
      if token not in sessions:
        return jsonify(error='No session; call /api/new first'), 400
      session = sessions[token]
      payload = request.get_json(silent=True)
      if payload is None or 'action' not in payload:
        return jsonify(error='Expected JSON {action: "cooperate"|"defect"}'), 400
      action = payload['action']
      if action not in ('cooperate', 'defect'):
        # Invalid action — state is NOT mutated
        return jsonify(error='Invalid action; allowed: cooperate, defect'), 400
      game = session['game']
      result = game.step(PrisonerGame.C if action == 'cooperate' else PrisonerGame.D)
      # Save the last valid state for inspection
      session['last_valid_state'] = result.copy()
    return jsonify(result)

  @app.post('/api/new')
  def New():
    with lock:
      token = secrets.token_urlsafe(32)
      sessions[token] = {
        'game': PrisonerGame(),
        'last_valid_state': {},
      }
      response = jsonify(sessions[token]['game'].state())
      response.set_cookie(
        'prisoner_session', token, httponly=True,
        samesite='Strict', secure=request.is_secure)
    return response

  @app.post('/api/reset')
  def Reset():
    with lock:
      token = request.cookies.get('prisoner_session')
      if token not in sessions:
        return jsonify(error='No session'), 400
      session = sessions[token]
      # Keep the AI strategy, start a fresh episode
      game_type = session['game'].ai_strategy
      session['game'] = PrisonerGame(ai=game_type)
      response = jsonify(session['game'].state())
      session['last_valid_state'] = {}
    return response

  return app


app = CreateApp()

if __name__ == '__main__':
  # Bind to 0.0.0.0 so the browser can reach it from any interface
  app.run(host='0.0.0.0', port=5008, debug=False)
