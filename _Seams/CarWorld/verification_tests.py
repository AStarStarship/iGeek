# Copyright AStarship <https://astarship.net>.
"""Tests for independent evidence parsing and swept-trace validation."""
import unittest
from verify_headless import parse_total, segment_hits_box


class VerificationTests(unittest.TestCase):
  def test_accept_actual_rounded_cli_timing(self):
    # Captured native output; CLI reports rates to one decimal place.
    line = ('TOTAL goals=900/900 frames=814685 physics_steps=3258740 '
            'simulated_s=16293.700 wall_s=11.038133 frames_per_s=73806.4 '
            'realtime_factor=1476.1')
    result = parse_total(line)
    self.assertEqual(result['total'], 900)
    self.assertEqual(result['frames'], 814685)

  def test_reject_corrupt_step_count(self):
    line = ('TOTAL goals=1/1 frames=100 physics_steps=401 simulated_s=2.000 '
            'wall_s=1.000000 frames_per_s=100.0 realtime_factor=2.0')
    with self.assertRaises(AssertionError):
      parse_total(line)

  def test_reject_nonfinite_timing(self):
    line = ('TOTAL goals=1/1 frames=100 physics_steps=400 simulated_s=2.000 '
            'wall_s=nan frames_per_s=100.0 realtime_factor=2.0')
    with self.assertRaises(AssertionError):
      parse_total(line)

  def test_swept_body_trace_detects_crossing_and_corner_contact(self):
    box = (47.35, 22.35, 61.65, 43.65)
    self.assertTrue(segment_hits_box((40, 32), (70, 32), box))
    self.assertTrue(segment_hits_box((40, 22.35), (70, 22.35), box))
    self.assertFalse(segment_hits_box((40, 18), (70, 18), box))


if __name__ == '__main__':
  unittest.main()
