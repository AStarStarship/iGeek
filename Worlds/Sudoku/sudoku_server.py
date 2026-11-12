# Copyright AStarship <https://astarship.net>.
"""Human-mode Sudoku demo; native board functions, not a learned policy.

Run: python Worlds/Sudoku/sudoku_server.py (Flask, port 5004).
Requires g++ and Flask. No Python solver fallback is used. Full native header
inclusion currently fails in Gym/ASCIICrabs (Room.hpp path, CSizeMin, etc.).
Instead compile the unmodified pure-board section of SudokuWorld.hxx, selected
by explicit structural markers. Native gym/reward logic is not compiled.
Build products live exclusively in TMPDIR (Hermes scratch) or the configured
SUDOKU_SCRATCH directory. Sessions are in-memory, single-process demo storage.
"""
import ctypes
import os
from pathlib import Path
import subprocess
import tempfile

Directory = Path(__file__).resolve().parent


class ANativeBoard:
  def __init__(self):
    source = (Directory / 'SudokuWorld.hxx').read_text()
    start = source.index('enum {\n  SudokuCells = 81,')
    end = source.index('/* ASudokuGym', start)
    root = Path(os.environ.get('SUDOKU_SCRATCH', os.environ.get(
        'TMPDIR', str(Path.home() / '.cache' / 'sudoku'))))
    root.mkdir(parents=True, exist_ok=True)
    self.build_dir = Path(tempfile.mkdtemp(prefix='sudoku-native-', dir=root))
    (self.build_dir / 'SudokuBoardNative.hxx').write_text(source[start:end])
    self.library_path = self.build_dir / 'sudoku_bridge.so'
    subprocess.run(['g++', '-std=c++17', '-O2', '-shared', '-fPIC',
        str(Directory / 'sudoku_bridge.cpp'), '-I', str(self.build_dir),
        '-o', str(self.library_path)], check=True, capture_output=True, text=True)
    self.library = ctypes.CDLL(str(self.library_path))
    self.array_type = ctypes.c_uint64 * 81
    for name in ('Default', 'Valid', 'Verify', 'Solve'):
      function = getattr(self.library, 'SudokuBridge' + name)
      function.argtypes = [ctypes.POINTER(ctypes.c_uint64)]
      function.restype = ctypes.c_int
      if name == 'Default':
        function.restype = None
      if name == 'Valid':
        function.argtypes += [ctypes.c_int, ctypes.c_int]

  def Array(self, board):
    if len(board) != 81 or any(type(digit) is not int or not 0 <= digit <= 9
                              for digit in board):
      raise ValueError('Board must contain 81 integer digits in 0..9.')
    return self.array_type(*board)

  def Default(self):
    board = self.array_type()
    self.library.SudokuBridgeDefault(board)
    return list(board)

  def Valid(self, board, cell, digit):
    return bool(self.library.SudokuBridgeValid(self.Array(board), cell, digit))

  def Verify(self, board):
    return bool(self.library.SudokuBridgeVerify(self.Array(board)))

  def Solve(self, board):
    work = self.Array(board)
    return list(work) if self.library.SudokuBridgeSolve(work) else None


Engine = ANativeBoard()

import secrets
import threading
from flask import Flask, g, jsonify, request, send_from_directory

app = Flask(__name__, static_folder=None)
app.config['MAX_CONTENT_LENGTH'] = 4096
sessions = {}
sessions_lock = threading.RLock()


def NewGame(board=None, puzzle='classic'):
  board = Engine.Default() if board is None else board
  return {'board': board.copy(), 'initial': board.copy(),
          'givens': [bool(digit) for digit in board], 'moves': 0, 'hints': 0,
          'puzzle': puzzle, 'message': 'Choose an empty cell and a digit.'}


def State(game):
  return {key: game[key] for key in ('board', 'givens', 'moves', 'hints',
          'message', 'puzzle')} | {
      'done': Engine.Verify(game['board']),
      'candidates': [[] if digit else [value for value in range(1, 10)
          if Engine.Valid(game['board'], cell, value)]
          for cell, digit in enumerate(game['board'])]}


@app.before_request
def LockSession():
  sessions_lock.acquire()
  token = request.cookies.get('sudoku_session')
  if token not in sessions:
    token = secrets.token_urlsafe(32)
    sessions[token] = NewGame()
  g.session_token = token
  g.game = sessions[token]
  g.session_locked = True


@app.after_request
def SessionCookie(response):
  response.set_cookie('sudoku_session', g.session_token, httponly=True,
                      samesite='Strict', secure=request.is_secure)
  response.headers['Cache-Control'] = 'no-store'
  return response


@app.teardown_request
def UnlockSession(error):
  if getattr(g, 'session_locked', False):
    sessions_lock.release()


@app.get('/api/state')
def GetState():
  return jsonify(State(g.game))


def Payload():
  payload = request.get_json(silent=True)
  if type(payload) is not dict:
    raise ValueError('Expected a JSON object.')
  return payload


def Integer(value, low, high):
  if type(value) is not int or not low <= value <= high:
    raise ValueError(f'Expected an integer in {low}..{high}.')
  return value


@app.errorhandler(ValueError)
def Invalid(error):
  return jsonify(error=str(error)), 400


@app.post('/api/action')
def Action():
  payload = Payload()
  action = payload.get('action')
  fields = {'place': {'action', 'cell', 'digit'},
            'hint': {'action', 'cell'}, 'solve': {'action'}, 'reset': {'action'}}
  if type(action) is not str or action not in fields or set(payload) != fields[action]:
    raise ValueError('Unknown action, missing field, or extraneous field.')
  game = g.game
  cell = 0
  if action in ('place', 'hint'):
    cell = Integer(payload['cell'], 0, 80)
    if game['givens'][cell]:
      raise ValueError('Given cells are immutable.')
  if action == 'place':
    digit = Integer(payload['digit'], 0, 9)
    if digit and not Engine.Valid(game['board'], cell, digit):
      raise ValueError('Digit conflicts with its row, column, or box.')
    game['board'][cell] = digit
    game['moves'] += 1
    game['message'] = 'Digit placed.' if digit else 'Cell erased.'
  elif action in ('hint', 'solve'):
    if action == 'hint' and game['board'][cell]:
      raise ValueError('Hints require an editable empty cell.')
    solved = Engine.Solve(game['board'])
    if solved is None:
      raise ValueError('Current entries have no solution. Edit or erase an entry.')
    if action == 'hint':
      game['board'][cell] = solved[cell]
      game['hints'] += 1
      game['message'] = 'Native backtracking solver hint (not a learned policy).'
    else:
      game['board'] = solved
      game['message'] = 'Solved by native backtracking (not a learned policy).'
  else:
    game = NewGame(game['initial'], game['puzzle'])
    sessions[g.session_token] = game
  return jsonify(State(game))


def Generate(seed):
  import random
  rng = random.Random(seed)

  def Order():
    groups = [0, 1, 2]
    rng.shuffle(groups)
    result = []
    for group in groups:
      inside = [0, 1, 2]
      rng.shuffle(inside)
      result.extend(group * 3 + item for item in inside)
    return result

  rows, cols = Order(), Order()
  digits = list(range(1, 10))
  rng.shuffle(digits)
  digits = [0] + digits
  board = Engine.Default()
  return [digits[board[row * 9 + col]] for row in rows for col in cols]


@app.post('/api/new')
def NewPuzzle():
  payload = Payload()
  if set(payload) != {'puzzle', 'seed'} or type(payload.get('puzzle')) is not str:
    raise ValueError('Expected exactly puzzle and seed.')
  puzzle = payload['puzzle']
  if puzzle not in ('classic', 'generated'):
    raise ValueError('Unknown puzzle.')
  seed = Integer(payload['seed'], -(2 ** 53 - 1), 2 ** 53 - 1)
  game = NewGame(Generate(seed) if puzzle == 'generated' else None, puzzle)
  sessions[g.session_token] = game
  return jsonify(State(game))


@app.get('/')
@app.get('/sudoku.html')
def Index():
  return send_from_directory(Directory, 'sudoku.html')


@app.get('/api/manifest')
def Manifest():
  return jsonify({
      'name': 'SudokuWorld browser demo', 'version': '1.0',
      'mode': 'browser-human',
      'engine': 'C ABI compiled from verbatim SudokuWorld.hxx pure-board slice',
      'policy': 'Human actions; hint/solve use deterministic native backtracking, not learning.',
      'reward': None,
      'native_gym_difference': 'Not ASudokuGym: no hidden-target rewards, allows editable entries and erase. No native gym reward equivalence.',
      'observation': {
          'board': '81 row-major integers 0..9; 0 is empty; cell = row*9+column',
          'givens': '81 booleans; true means immutable puzzle clue',
          'candidates': '81 digit arrays; empty for filled cells; local row/column/box legality only, not guaranteed solvable',
          'done': 'boolean: filled board passes native SudokuVerify',
          'moves': 'integer: accepted place/erase requests since reset/new, including no-ops; excludes hints/solve',
          'hints': 'integer: accepted hints since reset/new',
          'message': 'human-readable last successful action status',
          'puzzle': 'classic or generated; never includes hidden solution'},
      'actions': {
          'GET /api/state': 'Returns observation',
          'POST /api/action': [
              {'action': 'place', 'cell': 'integer 0..80, non-given', 'digit': 'integer 0..9, 0 erases; conflicts rejected'},
              {'action': 'hint', 'cell': 'integer 0..80, editable empty cell; fills one solver digit'},
              {'action': 'solve', 'effect': 'solves current entries; unsolvable is an error, never overwrites entries'},
              {'action': 'reset', 'effect': 'restores current initial puzzle and clears counters'}],
          'POST /api/new': {'puzzle': 'classic|generated', 'seed': 'required integer in -(2^53-1)..2^53-1; ignored for classic'},
          'GET /api/manifest': 'This contract'},
      'responses': 'Successful mutations return observation. Invalid JSON, types, bounds, missing/extra fields or illegal actions: HTTP 400 {error:string}, no game mutation. Booleans are not integers.',
      'generation': 'Seeded digit, row/band, column/stack permutations of the unique classic puzzle preserve uniqueness. Not native hole generation.',
      'sessions': 'Opaque HttpOnly SameSite=Strict cookie; locked in-memory single-process sessions; restart clears games.',
      'routes': ['/', '/sudoku.html', '/api/state', '/api/action', '/api/new', '/api/manifest']})


if __name__ == '__main__':
  app.run(host='0.0.0.0', port=5004, debug=False, threaded=True)
