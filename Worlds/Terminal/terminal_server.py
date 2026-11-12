# Copyright AStarship <https://astarship.net>.
"""Terminal World browser demo — Flask server on port 5010.

Serves a UI for a simulated terminal environment.
Sessions are isolated; state lives in the server process only.
No real shell access, no filesystem, no secret reads.
All commands are validated against a strict allowlist.
"""
import secrets
import threading
from pathlib import Path

from flask import Flask, jsonify, request, send_from_directory

Root = Path(__file__).resolve().parent

# Import engine — ensure path is available
import sys
_ENGINE_PATH = str(Path(__file__).resolve().parent)
if _ENGINE_PATH not in sys.path:
  sys.path.insert(0, _ENGINE_PATH)

from terminal_engine import TerminalWorld, TerminalEngine, TermActionChars, COMMAND_MAP


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
    return send_from_directory(Root, 'terminal.html')

  @app.get('/assets/<path:filename>')
  def Asset(filename):
    return send_from_directory(Root / 'assets', filename)

  #: allowed command strings (matching TerminalEngine.COMMAND_MAP values)
  ALLOWED_COMMANDS = set(COMMAND_MAP.values())

  @app.get('/api/manifest')
  def Manifest():
    return jsonify({
      'name': 'TerminalWorld browser demo',
      'version': 1,
      'rules': {
        'description': 'Simulated terminal environment; agent issues one command per step.',
        'allowed_commands': sorted(list(COMMAND_MAP.values())),
        'screen_rows': 24,
        'screen_cols': 80,
        'max_steps': 100,
      },
      'observation': {
        'fields': ['screen', 'cwd', 'line_len', 'history_count', 'running', 'done'],
        'meaning': {
          'screen': 'terminal screen summary (last 4 rows)',
          'cwd': 'current working directory',
          'line_len': 'length of current command line being typed',
          'history_count': 'number of past commands in history ring',
          'running': 'whether the terminal session is active',
          'done': 'whether the episode has ended',
        },
      },
      'actions': {
        'allowed': sorted(list(COMMAND_MAP.values())),
        'schema': {'action': sorted(list(COMMAND_MAP.values()))},
      },
      'sessions': 'Opaque HttpOnly cookie; state in this server process; restart clears.',
    })

  @app.get('/api/state')
  def State():
    with lock:
      token = request.cookies.get('terminal_session')
      if token not in sessions:
        token = secrets.token_urlsafe(32)
        sessions[token] = {'world': TerminalWorld()}
      session = sessions[token]
      world = session['world']
      # Return current state without advancing
      s = world.to_dict()
    return jsonify(s)

  @app.post('/api/action')
  def Action():
    with lock:
      token = request.cookies.get('terminal_session')
      if token not in sessions:
        return jsonify(error='No session; call /api/new first'), 400
      session = sessions[token]
      world = session['world']
      payload = request.get_json(silent=True)
      if payload is None or 'action' not in payload:
        return jsonify(error='Expected JSON {action: "pwd"|"ls"|"echo"|"cd"|"clear"|"exit"}'), 400
      action = payload['action']
      # Validate against allowlist
      if action not in TerminalEngine.ALLOWED_COMMANDS:
        # Invalid action — state is NOT mutated
        current = world.to_dict()
        return jsonify(
          error='Invalid action; allowed: ' + ', '.join(sorted(list(COMMAND_MAP.values()))),
          **current
        ), 400
      # Map action name to step index
      action_idx = None
      for char, name in COMMAND_MAP.items():
        if name == action:
          action_idx = TermActionChars.index(char)
          break
      if action_idx is None:
        return jsonify(error='Unknown action'), 400
      result = world.step(action_idx)
      # Build response with top-level keys the test expects
      response_dict = {
        'observation': result['observation'],
        'reward': result['reward'],
        'terminal': result['terminal'],
        'info': result['info'],
        'round': result['info']['round'],
        'action': result['info']['action'],
        'cmd_name': result['info']['cmd_name'],
      }
      session['last_result'] = result
      return jsonify(response_dict)

  @app.post('/api/new')
  def New():
    with lock:
      token = secrets.token_urlsafe(32)
      sessions[token] = {'world': TerminalWorld()}
      world = sessions[token]['world']
      s = world.to_dict()
      # Add top-level round
      response_dict = dict(s)
      response_dict['round'] = world.round
      response = jsonify(response_dict)
      response.set_cookie(
        'terminal_session', token, httponly=True,
        samesite='Strict', secure=request.is_secure)
    return response

  @app.post('/api/reset')
  def Reset():
    with lock:
      token = request.cookies.get('terminal_session')
      if token not in sessions:
        return jsonify(error='No session'), 400
      session = sessions[token]
      world = session['world']
      world.reset()
      s = world.to_dict()
      # Add top-level round
      response_dict = dict(s)
      response_dict['round'] = world.round
      response = jsonify(response_dict)
      session['last_result'] = {'round': world.round, 'done': not world.state.running}
    return response

  return app


app = CreateApp()

if __name__ == '__main__':
  # Bind to 0.0.0.0 so the browser can reach it from any interface
  app.run(host='0.0.0.0', port=5010, debug=False)
