# Copyright AStarship <https://astarship.net>.
import importlib
import unittest


class CardsTests(unittest.TestCase):
  def TestPokerRanks(self):
    engine = importlib.import_module('cards_engine')
    hands = ['As Kd 9h 6c 3s', 'As Ad 9h 6c 3s', 'As Ad 9h 9c 3s',
             'As Ad Ah 6c 3s', 'As 2d 3h 4c 5s', 'As Js 9s 6s 3s',
             'As Ad Ah 6c 6s', 'As Ad Ah Ac 3s', '9s 10s Js Qs Ks']
    scores = [engine.Evaluate(hand.split()) for hand in hands]
    self.assertEqual([score[0] for score in scores], list(range(9)))
    self.assertEqual(scores, sorted(scores))
    self.assertLess(engine.Evaluate('As 2d 3h 4c 5s'.split()),
                    engine.Evaluate('2s 3d 4h 5c 6s'.split()))
    self.assertGreater(engine.Evaluate('As Ad Kh Qc 3s'.split()),
                       engine.Evaluate('Ah Ac Kd Jc 9s'.split()))
    self.assertEqual(engine.Evaluate('As Ks Qs Js 10s 2h 3d'.split()),
                     engine.Evaluate('As Ks Qs Js 10s 9h 9d'.split()))

  def TestPokerRound(self):
    engine = importlib.import_module('cards_engine')
    for seed in range(40):
      import random
      deck = [rank + suit for suit in 'cdhs' for rank in engine.Ranks]
      random.Random(seed).shuffle(deck)
      table = engine.Table('poker', deck)
      self.assertEqual(table.State()['opponent'], ['back', 'back'])
      self.assertEqual(table.pot, 15)
      phases = {table.phase}
      for turn in range(100):
        self.assertEqual(table.bankroll + table.opponent_bankroll + table.pot, 400)
        if table.done:
          break
        actions = table.Actions()
        action = 'check' if 'check' in actions else 'call'
        table.Action(action)
        phases.add(table.phase)
      self.assertTrue(table.done)
      self.assertEqual(len(table.board), 5)
      self.assertEqual(table.pot, 0)
      self.assertTrue(table.reveal)
      self.assertTrue({'preflop', 'flop', 'turn', 'river', 'done'} <= phases)

  def TestServer(self):
    server = importlib.import_module('cards_server')
    app = server.CreateApp()
    first, second = app.test_client(), app.test_client()
    state = first.get('/api/state').get_json()
    other = second.get('/api/state').get_json()
    self.assertEqual(set(state), {'game', 'phase', 'done', 'message', 'player',
                     'opponent', 'board', 'player_total', 'opponent_total',
                     'bankroll', 'opponent_bankroll', 'pot', 'actions', 'rules'})
    self.assertNotEqual(first.get_cookie('cards_session').value,
                        second.get_cookie('cards_session').value)
    for payload in [None, [], {}, {'action': 12}, {'action': 'cheat'},
                    {'action': 'stand', 'extra': 1}]:
      response = first.post('/api/action', json=payload)
      self.assertEqual(response.status_code, 400)
      self.assertEqual(first.get('/api/state').get_json(), state)
    response = first.post('/api/new', json={'game': 'poker'})
    self.assertEqual(response.status_code, 200)
    poker = response.get_json()
    self.assertEqual(poker['opponent'], ['back', 'back'])
    self.assertEqual(first.post('/api/new', json={'game': 'poker'}).status_code, 400)
    self.assertEqual(first.get('/api/state').get_json(), poker)
    self.assertEqual(second.get('/api/state').get_json(), other)
    self.assertEqual(first.post('/api/action', json={'action': 'fold'}).status_code, 200)
    self.assertEqual(first.get('/api/state').get_json()['opponent'], ['back', 'back'])
    manifest = first.get('/api/manifest').get_json()
    self.assertTrue({'rules', 'observation', 'actions', 'rewards'} <= set(manifest))
    self.assertIn('no-store', first.get('/api/state').headers['Cache-Control'])

  def TestBlackjackOutcomes(self):
    import copy
    engine = importlib.import_module('cards_engine')
    cases = [(['As', 'Kh', '9d', '8c'], None, 215),
             (['As', 'Kh', 'Ad', 'Qc'], None, 200),
             (['9s', 'Kh', 'Ad', 'Qc'], None, 190),
             (['10s', '8h', '9d', '9c'], 'stand', 200),
             (['10s', '8h', '9d', '7c', 'Kh'], 'hit', 190),
             (['5s', '6h', '9d', '8c', 'Kh'], 'double', 220),
             (['9s', '6h', '9d', '8c', 'Kh'], 'double', 180)]
    for deck, action, expected in cases:
      table = engine.Table(deck=deck)
      if action:
        table.Action(action)
      self.assertTrue(table.done)
      self.assertEqual(table.bankroll, expected)
      self.assertEqual(table.bankroll + table.opponent_bankroll, 400)
      before = copy.deepcopy(vars(table))
      with self.assertRaises(ValueError):
        table.Action('stand')
      self.assertEqual(vars(table), before)
    self.assertEqual(engine.Total(['As', 'Ah', '9d']), 21)
    table = engine.Table(deck=['2s', '3h', '9d', '8c', '2h'])
    table.Action('hit')
    self.assertNotIn('double', table.Actions())
    before = copy.deepcopy(vars(table))
    for action in ['double', 'fold', None, [], 1]:
      with self.assertRaises(ValueError):
        table.Action(action)
      self.assertEqual(vars(table), before)

  def TestPokerCapsConservation(self):
    import random
    engine = importlib.import_module('cards_engine')
    for seed in range(150):
      rng = random.Random(seed)
      table = engine.Table('poker')
      table.done = True
      table.bankroll = seed % 39 + 1
      table.opponent_bankroll = 400 - table.bankroll
      table.New('poker')
      for turn in range(100):
        self.assertGreaterEqual(table.bankroll, 0)
        self.assertGreaterEqual(table.opponent_bankroll, 0)
        self.assertEqual(table.bankroll + table.opponent_bankroll + table.pot, 400)
        if table.done:
          break
        if table.raises >= 4:
          self.assertNotIn('raise', table.Actions())
        actions = table.Actions()
        table.Action(rng.choice(actions))
      self.assertTrue(table.done)
      self.assertEqual(table.pot, 0)
    table = engine.Table('poker', ['2c', '3d', '4c', '5d', 'As', 'Ks', 'Qs', 'Js', '10s'])
    while not table.done:
      table.Action('check' if 'check' in table.Actions() else 'call')
    self.assertEqual(table.bankroll, 200)
    self.assertEqual(table.opponent_bankroll, 200)
    self.assertEqual(table.message, 'Split pot.')

  def TestPokerKickers(self):
    engine = importlib.import_module('cards_engine')
    comparisons = [
      ('As Kd 9h 6c 3s', 'As Qd Jh 8c 7s'),
      ('As Ad Kh Kc 3s', 'Ah Ac Qd Qc Js'),
      ('As Ad Kh Kc 4s', 'Ah Ac Kd Ks 3s'),
      ('As Ad Ah Kc 3s', 'Ac Ah As Qc Js'),
      ('As Js 9s 6s 4s', 'Ah Jh 9h 6h 3h'),
      ('As Ad Ah 6c 6s', 'Ks Kd Kh Qc Qs'),
      ('As Ad Ah 7c 7s', 'As Ad Ah 6c 6s'),
      ('As Ad Ah Ac 4s', 'As Ad Ah Ac 3s')]
    for better, worse in comparisons:
      self.assertGreater(engine.Evaluate(better.split()), engine.Evaluate(worse.split()))
    self.assertEqual(engine.Evaluate('As Ad Ah Ks Kd Kh 2s'.split()), (6, 14, 13))
    self.assertEqual(engine.Evaluate('As Ad Ks Kd Qs Qd 2s'.split()), (2, 14, 13, 12))
    for cards in [[], ['As'] * 5]:
      with self.assertRaises(ValueError):
        engine.Evaluate(cards)

  def TestSessionSecurity(self):
    from concurrent.futures import ThreadPoolExecutor
    server = importlib.import_module('cards_server')
    app = server.CreateApp()
    client = app.test_client()
    client.get('/api/state')
    client.post('/api/new', json={'game': 'poker'})
    token = client.get_cookie('cards_session').value
    self.assertEqual(len(token), 43)
    self.assertTrue(client.get_cookie('cards_session').http_only)
    before = client.get('/api/state').get_json()
    for payload in [{'game': 'invalid'}, {'game': []}, {'game': 'poker', 'deck': []}]:
      self.assertEqual(client.post('/api/new', json=payload).status_code, 400)
      self.assertEqual(client.get('/api/state').get_json(), before)
    self.assertEqual(client.post('/api/action', data='{',
                                content_type='application/json').status_code, 400)
    self.assertEqual(client.get('/api/state').get_json(), before)

    def Fold(_):
      parallel = app.test_client()
      parallel.set_cookie('cards_session', token)
      return parallel.post('/api/action', json={'action': 'fold'}).status_code

    with ThreadPoolExecutor(max_workers=2) as pool:
      self.assertEqual(sorted(pool.map(Fold, range(2))), [200, 400])
    after = client.get('/api/state').get_json()
    self.assertEqual(after['bankroll'] + after['opponent_bankroll'], 400)
    self.assertEqual(client.post('/api/new', json={'game': 'poker'}).status_code, 200)
    forged = app.test_client()
    forged.set_cookie('cards_session', 'chosen-by-client')
    forged.get('/api/state')
    self.assertNotEqual(forged.get_cookie('cards_session').value, 'chosen-by-client')

  def TestBlackjack(self):
    engine = importlib.import_module('cards_engine')
    table = engine.Table(deck=['10c', '8d', 'As', '6h', '2c'])
    state = table.State()
    self.assertEqual(state['player_total'], 18)
    self.assertEqual(state['opponent'], ['As', 'back'])
    self.assertIsNone(state['opponent_total'])
    self.assertEqual(state['actions'], ['hit', 'stand', 'double'])
    table.Action('stand')
    self.assertEqual(table.State()['opponent_total'], 17)
    self.assertEqual(table.bankroll, 210)
    self.assertEqual(table.pot, 0)
    self.assertTrue(table.done)


def LoadTests(loader, tests, pattern):
  loader.testMethodPrefix = 'Test'
  return loader.loadTestsFromTestCase(CardsTests)


# unittest's discovery hook is a fixed external protocol name.
load_tests = LoadTests

if __name__ == '__main__':
  unittest.main(verbosity=2)
