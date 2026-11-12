# Copyright AStarship <https://astarship.net>.
import unittest


class CarServerTests(unittest.TestCase):
  def test_server(self):
    import importlib
    server = importlib.import_module('car_server')
    app = server.CreateApp()
    first, second = app.test_client(), app.test_client()
    state = first.get('/api/state')
    self.assertEqual(state.status_code, 200)
    body = state.get_json()
    self.assertEqual(set(body), {'state', 'map'})
    self.assertEqual(set(body['state']),
                     {'x', 'y', 'heading', 'forward', 'lateral', 'yaw_rate', 'steer',
                      'time', 'distance', 'longitudinal_accel', 'lateral_accel',
                      'utilization', 'status', 'done', 'goal_distance', 'frames',
                      'speed', 'observation', 'scenario', 'seed', 'reward', 'barrier'})
    self.assertEqual(len(body['state']['observation']), 22)
    self.assertEqual((body['map']['width'], body['map']['height']), (120, 64))
    self.assertEqual(len(body['map']['tiles']), 120 * 64)
    self.assertGreater(len(body['map']['route']), 1)

    # Session isolation: distinct cookies, separate cars.
    other = second.get('/api/state').get_json()
    self.assertNotEqual(first.get_cookie('car_session').value,
                        second.get_cookie('car_session').value)
    first.post('/api/action', json={'action': 'manual', 'frames': 10, 'throttle': 1})
    self.assertEqual(second.get('/api/state').get_json(), other)

    # Invalid action payloads leave state untouched.
    before = first.get('/api/state').get_json()
    for payload in [None, [], {}, {'action': 'cheat'}, {'action': 'manual', 'steer': 99},
                    {'action': 'manual', 'frames': 'ten'}, {'action': 'manual', 'throttle': True}]:
      self.assertEqual(first.post('/api/action', json=payload).status_code, 400, payload)
    self.assertEqual(first.get('/api/state').get_json(), before)

    # Malformed JSON.
    self.assertEqual(first.post('/api/action', data='{', content_type='application/json').status_code, 400)

    # New scenario.
    self.assertEqual(first.post('/api/new', json={'scenario': 'ice', 'seed': 12}).status_code, 200)
    self.assertEqual(first.get('/api/state').get_json()['state']['scenario'], 'ice')
    for payload in [{'scenario': 'nope', 'seed': 1}, {'scenario': 'ice'},
                    {'scenario': 'ice', 'seed': '12'}, {'scenario': 'ice', 'seed': -1},
                    {'scenario': 'ice', 'seed': 12, 'extra': 1}]:
      self.assertEqual(first.post('/api/new', json=payload).status_code, 400, payload)
    self.assertEqual(first.get('/api/state').get_json()['state']['scenario'], 'ice')

    # Manifest.
    manifest = first.get('/api/manifest').get_json()
    self.assertIn('engine', manifest)
    self.assertIn('actions', manifest)
    self.assertIn('no-store', first.get('/api/state').headers['Cache-Control'])
    self.assertEqual(first.get('/').status_code, 200)

  def test_session_security(self):
    import importlib
    from concurrent.futures import ThreadPoolExecutor
    server = importlib.import_module('car_server')
    app = server.CreateApp()
    client = app.test_client()
    client.get('/api/state')
    token = client.get_cookie('car_session').value
    self.assertEqual(len(token), 43)
    self.assertTrue(client.get_cookie('car_session').http_only)

    def Step(_):
      parallel = app.test_client()
      parallel.set_cookie('car_session', token)
      return parallel.post('/api/action', json={'action': 'manual', 'frames': 2,
                                                'throttle': 1}).status_code

    with ThreadPoolExecutor(max_workers=2) as pool:
      self.assertEqual(sorted(pool.map(Step, range(2))), [200, 200])
    state = client.get('/api/state').get_json()['state']
    self.assertGreaterEqual(state['x'], 12.5)
    forged = app.test_client()
    forged.set_cookie('car_session', 'chosen-by-client')
    forged.get('/api/state')
    self.assertNotEqual(forged.get_cookie('car_session').value, 'chosen-by-client')


if __name__ == '__main__':
  unittest.main()
