# Copyright AStarship <https://astarship.net>.
"""Browser host for the native CarWorld 2D demo (real ACarEnv via C ABI)."""
import secrets
import threading
from pathlib import Path
from flask import Flask, jsonify, request, send_from_directory

from car_demo import ACarDemo, Scenarios


Directory = Path(__file__).resolve().parent


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
    return send_from_directory(Directory, 'car.html')

  @app.get('/api/manifest')
  def Manifest():
    return jsonify({
      'name': 'CarWorld 2D browser demo', 'version': '1',
      'engine': 'Native iGeek ACarEnv + TileMap + AVehiclePlanner via C ABI (car_bridge.cpp); '
                'same four-tire RK4 plant, classical pilot, and swept collisions as the headless binary.',
      'mode': 'browser-human; actions are identical to ACarEnv::Step / StepControl',
      'observation': {
        'state': 'x, y, heading, forward, lateral, yaw_rate, steer, time, distance, '
                 'longitudinal_accel, lateral_accel, utilization, status, goal_distance, frames, speed',
        'observation': 'The native 22-element ACarEnv observation vector, unchanged',
        'map': '120x64 metres, 1 m tiles; 106 = solid wall, other values are terrain material',
        'route': 'Planned world-space waypoints from the native AVehiclePlanner'},
      'actions': {
        'pilot': {'speed_mps': '5 slow | 8 cruise | 12 fast (classical driver steers and brakes)'},
        'manual': {'steer': '[-0.55, 0.55] rad target', 'throttle': '[0, 1]', 'brake': '[0, 1]',
                   'reverse': '0 = forward gear, 1 = reverse gear'},
        'stop': 'Classical driver at stop speed (0 m/s)',
        'frames': 'Simulated 50 Hz frames per request, 1..50 (default 5)'},
      'rewards': 'Native per-frame reward from ACarEnv (route progress, cross-track, goal/collision).',
      'sessions': 'Opaque HttpOnly SameSite=Strict cookie; state in this server process; restart clears cars.'})

  def Game(token, scenario=None, seed=None):
    if token in tables and (scenario is None or tables[token].scenario == scenario):
      return tables[token]
    if token in tables:
      tables[token].Close()
    demo = ACarDemo(scenario or 'straight', seed if seed is not None else 7)
    tables[token] = demo
    return demo

  def Dispatch(operation, payload=None):
    with lock:
      token = request.cookies.get('car_session')
      fresh = token not in tables
      if fresh:
        token = secrets.token_urlsafe(32)
      if operation == 'new':
        if (not isinstance(payload, dict) or set(payload) != {'scenario', 'seed'} or
            payload['scenario'] not in Scenarios or
            type(payload['seed']) is not int or not 0 <= payload['seed'] <= 2 ** 32 - 1):
          return jsonify(error='Expected {scenario: name, seed: uint32}'), 400
        demo = Game(token, payload['scenario'], payload['seed'])
        response = jsonify({'status': 'ok', 'scenario': demo.scenario, 'seed': demo.seed})
      elif operation == 'action':
        if (not isinstance(payload, dict) or 'action' not in payload):
          return jsonify(error='Expected {action: pilot|manual|stop, ...}'), 400
        demo = Game(token)
        try:
          demo.Step(payload['action'], payload.get('frames', 5),
                    payload.get('steer', 0), payload.get('throttle', 0),
                    payload.get('brake', 0), payload.get('reverse', 0))
        except ValueError as error:
          return jsonify(error=str(error)), 400
        response = jsonify({'status': 'ok'})
      else:
        demo = Game(token)
        response = jsonify({'state': demo.State(), 'map': demo.Map()})
      if fresh:
        response.set_cookie('car_session', token, httponly=True,
                            samesite='Strict', secure=request.is_secure)
      return response

  @app.get('/api/state')
  def State():
    return Dispatch('state')

  @app.post('/api/new')
  def New():
    return Dispatch('new', request.get_json(silent=True))

  @app.post('/api/action')
  def Action():
    return Dispatch('action', request.get_json(silent=True))

  return app


app = CreateApp()

if __name__ == '__main__':
  app.run(host='0.0.0.0', port=5005, debug=False, threaded=True)
