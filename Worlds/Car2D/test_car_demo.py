# Copyright AStarship <https://astarship.net>.
import unittest


def RunUntilDone(car, mode='pilot', frames=5, limit=500, **control):
  for _ in range(limit):
    if car.State()['done']:
      break
    car.Step(mode, frames, **control)
  return car.State()


class CarDemoTests(unittest.TestCase):
  def test_native_episode(self):
    from car_demo import ACarDemo
    with ACarDemo('straight', 7) as car:
      first = car.State()
      self.assertEqual(first['status'], 'running')
      self.assertEqual(len(first['observation']), 22)
      state = RunUntilDone(car)
      self.assertEqual(state['status'], 'goal')
      self.assertLess(state['speed'], .1)
      self.assertLess(state['goal_distance'], 1.2)
      self.assertGreater(state['time'], 0)

  def test_all_scenarios_reach_goal(self):
    from car_demo import ACarDemo
    for scenario in ('straight', 'curves', 'obstacle', 'gravel', 'ice',
                     'uphill', 'downhill', 'launch', 'ice_curves'):
      with ACarDemo(scenario, 41) as car:
        state = RunUntilDone(car, limit=1000)
        self.assertEqual(state['status'], 'goal', scenario)
        self.assertLess(state['speed'], .1, scenario)

  def test_manual_drive_and_brake(self):
    from car_demo import ACarDemo
    with ACarDemo('straight', 7) as car:
      for _ in range(60):
        car.Step('manual', 5, throttle=1.0)
      moving = car.State()
      self.assertGreater(moving['speed'], 10, 'full throttle should accelerate')
      for _ in range(60):
        car.Step('manual', 5, throttle=0.0, brake=1.0)
      stopped = car.State()
      self.assertLess(stopped['speed'], .5, 'full brake should stop the car')
      self.assertGreater(moving['distance'], stopped['distance'] * 0.5)

  def test_invalid_input_rejected_without_mutation(self):
    from car_demo import ACarDemo
    with ACarDemo('straight', 7) as car:
      before = car.State()
      for bad in (dict(steer=2.0), dict(throttle=5), dict(brake=-1), dict(steer=float('nan')),
                  dict(frames=0), dict(frames=99), dict(throttle='fast')):
        with self.assertRaises(ValueError, msg=bad):
          car.Step('manual', **{'frames': 5, **bad})
        for invalid_mode in ('autopilot', '', 'MANUAL'):
          with self.assertRaises(ValueError):
            car.Step(invalid_mode)
        self.assertEqual(car.State(), before)
      with ACarDemo('obstacle', 7) as obstacle:
        self.assertEqual(car.State(), before)

  def test_barrier_stops_driver(self):
    from car_demo import ACarDemo
    with ACarDemo('straight', 7) as car:
      car.Barrier()
      state = RunUntilDone(car, limit=1000)
      self.assertIn(state['status'], ('blocked', 'collision'))
      self.assertLessEqual(state['x'], 50, 'driver must not pass through the barrier wall')

  def test_open_loop_obstacle_collides(self):
    from car_demo import ACarDemo
    with ACarDemo('obstacle', 7) as car:
      state = RunUntilDone(car, mode='manual', throttle=0.35, steer=0.0, limit=1500)
      self.assertEqual(state['status'], 'collision')
      self.assertLessEqual(state['x'], 59, 'collision must stop at the wall face')

  def test_map_and_route(self):
    from car_demo import ACarDemo
    with ACarDemo('obstacle', 7) as car:
      world = car.Map()
      self.assertEqual((world['width'], world['height']), (120, 64))
      self.assertEqual(len(world['tiles']), 120 * 64)
      self.assertGreaterEqual(world['route'][0], [12.5, 32.5])
      self.assertAlmostEqual(world['route'][-1][0], 108.5, places=3)
      self.assertAlmostEqual(world['route'][-1][1], 32.5, places=3)
      self.assertIn(106, world['tiles'])
      wall_cells = world['tiles']
      for x in range(50, 59):
        for y in range(25, 41):
          self.assertEqual(wall_cells[y * 120 + x], 106)

  def test_seed_determinism(self):
    from car_demo import ACarDemo
    traces = []
    for _ in range(2):
      with ACarDemo('curves', 99) as car:
        trace = []
        for _ in range(40):
          car.Step('pilot', 5)
          state = car.State()
          trace.append((round(state['x'], 9), round(state['y'], 9), state['status']))
        traces.append(trace)
    self.assertEqual(traces[0], traces[1])
    self.assertGreater(len(traces[0]), 0)

  def test_reverse_gear(self):
    from car_demo import ACarDemo
    with ACarDemo('straight', 7) as car:
      self.assertGreater(car.State()['forward'], 0)
      for _ in range(30):
        car.Step('manual', 5, throttle=1.0, reverse=1)
      state = car.State()
      self.assertLess(state['forward'], 0, 'reverse gear should move the car backward')
      for _ in range(30):
        car.Step('manual', 5, throttle=1.0, reverse=0)
      forward = car.State()
      self.assertGreater(forward['forward'], 0, 'forward gear should recover from reverse')

  def test_invalid_reverse_rejected(self):
    from car_demo import ACarDemo
    with ACarDemo('straight', 7) as car:
      before = car.State()
      for bad in (2, -1, 0.5, True, 'reverse'):
        with self.assertRaises(ValueError, msg=bad):
          car.Step('manual', frames=5, reverse=bad)
        self.assertEqual(car.State(), before)

  def test_track_scenario(self):
    from car_demo import ACarDemo
    with ACarDemo('track', 7) as car:
      world = car.Map()
      self.assertEqual(len(world['route']), 8)
      self.assertEqual(world['route'][0], [60.0, 48.0])
      tiles = world['tiles']
      self.assertIn(106, tiles, 'border walls')
      self.assertIn(103, tiles, 'grass infield')
      self.assertEqual(tiles[48 * 120 + 60], 100, 'start/finish is asphalt')
      state = car.State()
      self.assertEqual(state['status'], 'running')
      self.assertAlmostEqual(state['x'], 60, places=1)
      self.assertLess(abs(state['y'] - 48), 0.16, 'seed jitter perturbs start y by <= 0.15')


if __name__ == '__main__':
  unittest.main()
