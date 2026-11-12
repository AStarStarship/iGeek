# Copyright AStarship <https://astarship.net>.
import unittest


def CountSolutions(board):
  """Independent test-only MRV counter, stopping at two solutions."""
  board = board.copy()

  def Search():
    choices = []
    for cell, digit in enumerate(board):
      if digit:
        continue
      row, col = divmod(cell, 9)
      peers = set(board[row * 9:row * 9 + 9]) | set(board[col::9])
      peers |= {board[r * 9 + c] for r in range(row // 3 * 3, row // 3 * 3 + 3)
                for c in range(col // 3 * 3, col // 3 * 3 + 3)}
      values = set(range(1, 10)) - peers
      if not values:
        return 0
      choices.append((len(values), cell, values))
    if not choices:
      return 1
    _, cell, values = min(choices)
    count = 0
    for digit in values:
      board[cell] = digit
      count += Search()
      board[cell] = 0
      if count >= 2:
        break
    return count

  return Search()


class SudokuTests(unittest.TestCase):
  def test_state_sessions(self):
    import sudoku_server as server
    first = server.app.test_client()
    second = server.app.test_client()
    response = first.get('/api/state')
    self.assertEqual(response.status_code, 200)
    state = response.get_json()
    self.assertEqual(set(state), {'board', 'givens', 'candidates', 'done',
                                 'moves', 'hints', 'message', 'puzzle'})
    self.assertEqual(state['candidates'][2], [1, 2, 4])
    self.assertEqual(state['candidates'][0], [])
    self.assertTrue(state['givens'][0])
    self.assertFalse(state['done'])
    self.assertEqual(second.get('/api/state').json, state)
    self.assertNotEqual(first.get_cookie('sudoku_session').value,
                        second.get_cookie('sudoku_session').value)

  def test_actions(self):
    import sudoku_server as server
    client = server.app.test_client()
    other = server.app.test_client()
    initial = client.get('/api/state').json
    for payload in ({'action': 'place', 'cell': 0, 'digit': 0},
                    {'action': 'place', 'cell': 2, 'digit': 5}):
      self.assertEqual(client.post('/api/action', json=payload).status_code, 400)
      self.assertEqual(client.get('/api/state').json, initial)
    placed = client.post('/api/action', json={
        'action': 'place', 'cell': 2, 'digit': 1}).json
    self.assertEqual(placed['board'][2], 1)
    self.assertEqual(placed['moves'], 1)
    self.assertEqual(other.get('/api/state').json, initial)
    for payload in ({'action': 'solve'}, {'action': 'hint', 'cell': 3}):
      self.assertEqual(client.post('/api/action', json=payload).status_code, 400)
      self.assertEqual(client.get('/api/state').json, placed)
    edited = client.post('/api/action', json={
        'action': 'place', 'cell': 2, 'digit': 4}).json
    self.assertEqual(edited['board'][2], 4)
    erased = client.post('/api/action', json={
        'action': 'place', 'cell': 2, 'digit': 0}).json
    self.assertEqual(erased['board'][2], 0)
    hinted = client.post('/api/action', json={'action': 'hint', 'cell': 2}).json
    self.assertEqual(hinted['board'][2], 4)
    self.assertEqual(hinted['hints'], 1)
    self.assertEqual(client.post('/api/action', json={
        'action': 'hint', 'cell': 2}).status_code, 400)
    solved = client.post('/api/action', json={'action': 'solve'}).json
    self.assertTrue(solved['done'])
    self.assertEqual(solved['candidates'], [[] for _ in range(81)])
    reset = client.post('/api/action', json={'action': 'reset'}).json
    self.assertEqual(reset, initial)

  def test_generation_and_validation(self):
    import sudoku_server as server
    client = server.app.test_client()
    initial = client.get('/api/state').json
    bad = [None, [], {}, {'action': []}, {'action': 'unknown'},
           {'action': 'solve', 'extra': 1},
           {'action': 'place', 'cell': True, 'digit': 1},
           {'action': 'place', 'cell': 81, 'digit': 1},
           {'action': 'place', 'cell': -1, 'digit': 1},
           {'action': 'place', 'cell': 2, 'digit': False},
           {'action': 'place', 'cell': 2, 'digit': 10},
           {'action': 'place', 'cell': 2, 'digit': 1.0}]
    for payload in bad:
      response = client.post('/api/action', json=payload)
      self.assertEqual(response.status_code, 400, payload)
      self.assertIn('error', response.json)
      self.assertEqual(client.get('/api/state').json, initial)
    for payload in ({'puzzle': 'generated', 'seed': True},
                    {'puzzle': 'bad', 'seed': 1}, {'puzzle': []},
                    {'puzzle': 'classic', 'seed': 0, 'extra': 1}):
      self.assertEqual(client.post('/api/new', json=payload).status_code, 400)
      self.assertEqual(client.get('/api/state').json, initial)
    for seed in (0, 1, 7, -12):
      payload = {'puzzle': 'generated', 'seed': seed}
      state = client.post('/api/new', json=payload).json
      self.assertEqual(state, client.post('/api/new', json=payload).json)
      self.assertEqual(sum(state['givens']), sum(initial['givens']))
      self.assertTrue(server.Engine.Verify(server.Engine.Solve(state['board'])))
      self.assertEqual(CountSolutions(state['board']), 1)
      client.post('/api/action', json={'action': 'solve'})
      self.assertEqual(client.post('/api/action', json={'action': 'reset'}).json, state)
    self.assertEqual(client.post('/api/new', json={
        'puzzle': 'classic', 'seed': 0}).json, initial)
    self.assertEqual(client.post('/api/action', data='{',
                                content_type='application/json').status_code, 400)
    self.assertEqual(client.get('/api/state').json, initial)

  def test_routes_manifest(self):
    import sudoku_server as server
    client = server.app.test_client()
    manifest = client.get('/api/manifest')
    self.assertEqual(manifest.status_code, 200)
    self.assertEqual(manifest.json['mode'], 'browser-human')
    self.assertIn('observation', manifest.json)
    self.assertIn('actions', manifest.json)
    for route in ('/', '/sudoku.html'):
      with client.get(route) as response:
        self.assertEqual(response.status_code, 200)
    self.assertEqual(client.get('/sudoku_server.py').status_code, 404)
    self.assertEqual(client.get('/api/action').status_code, 405)

  def test_native_solver(self):
    import sudoku_server as server
    board = server.Engine.Default()
    self.assertEqual(len(board), 81)
    self.assertEqual(board[:3], [5, 3, 0])
    self.assertTrue(server.Engine.Valid(board, 2, 4))
    self.assertFalse(server.Engine.Valid(board, 2, 5))
    solved = server.Engine.Solve(board)
    self.assertTrue(server.Engine.Verify(solved))
    self.assertEqual(board[2], 0)
    invalid = board.copy()
    invalid[2] = 5
    self.assertIsNone(server.Engine.Solve(invalid))
    invalid[2] = 1
    self.assertIsNone(server.Engine.Solve(invalid))
    self.assertIsNone(server.Engine.Solve([1] * 81))


if __name__ == '__main__':
  unittest.main()
