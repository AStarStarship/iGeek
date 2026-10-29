# Copyright AStarship <https://astarship.net>.
"""Native CLI black-box checks. Python is test tooling only."""
import csv
import math
import os
from pathlib import Path
import subprocess
import sys
import tempfile

SCENARIOS = {'straight', 'curves', 'obstacle', 'gravel', 'ice',
             'uphill', 'downhill', 'launch', 'ice_curves'}


def main():
  binary = Path(sys.argv[1]).resolve()
  assert binary.is_file(), f'headless car CLI not built: {binary}'

  def run(*args, expected=0):
    result = subprocess.run([str(binary), *args], capture_output=True,
                            text=True, timeout=120)
    assert result.returncode == expected, (args, result.stdout, result.stderr)
    return result.stdout

  assert '--scenario' in run('--help')
  for args in [('--episodes', '-1'), ('--seed', '-1'), ('--episodes', '0'),
               ('--episodes', '1junk'), ('--seed', '18446744073709551616'),
               ('--seed', '18446744073709551615', '--episodes', '2'),
               ('--scenario', 'unknown'), ('--scenario',), ('--unknown',),
               ('--action', 'bogus')]:
    run(*args, expected=2)
  with tempfile.TemporaryDirectory(dir=os.environ.get('TMPDIR')) as directory:
    root = Path(directory)
    reports = [root / 'a.csv', root / 'b.csv']
    for path in reports:
      output = run('--scenario', 'all', '--episodes', '2', '--seed', '7', '--csv', str(path))
      assert 'TOTAL goals=18/18' in output
    assert reports[0].read_bytes() == reports[1].read_bytes()
    with reports[0].open() as file:
      rows = list(csv.DictReader(file))
    assert len(rows) == 18 and {r['scenario'] for r in rows} == SCENARIOS
    for row in rows:
      assert row['status'] == 'goal'
      assert float(row['goal_error_m']) < 1.2
      assert float(row['final_speed_mps']) < 0.1
      assert float(row['peak_cross_track_m']) < 4
      assert int(row['physics_steps']) == 4 * int(row['frames'])
      assert math.isclose(float(row['sim_time_s']), 0.02 * int(row['frames']), abs_tol=1e-7)
      assert all(math.isfinite(float(v)) for k, v in row.items()
                 if k not in {'scenario', 'status'})
    before = reports[0].read_bytes()
    run('--csv', str(reports[0]), expected=2)
    assert reports[0].read_bytes() == before
    run('--trace', str(root / 'invalid_trace.csv'), expected=2)
    assert not (root / 'invalid_trace.csv').exists()
    run('--scenario', 'straight', '--csv', str(root / 'same.csv'),
        '--trace', str(root / 'same.csv'), expected=2)
    assert not (root / 'same.csv').exists()
    trace = root / 'trace.csv'
    run('--scenario', 'obstacle', '--trace', str(trace))
    with trace.open() as file:
      frames = list(csv.DictReader(file))
    assert len(frames) > 500 and frames[-1]['status'] == 'goal'
    assert float(frames[0]['time_s']) == 0
    assert all(math.isfinite(float(v)) for frame in frames for k, v in frame.items() if k != 'status')
    run('--scenario', 'straight', '--barrier', '--csv', str(root / 'emergency.csv'), expected=1)
    with (root / 'emergency.csv').open() as file:
      emergency = next(csv.DictReader(file))
    assert emergency['status'] == 'blocked'
    assert float(emergency['final_speed_mps']) < 0.1
    run('--scenario', 'obstacle', '--open-loop', '--csv', str(root / 'raw.csv'), expected=1)
    with (root / 'raw.csv').open() as file:
      raw = next(csv.DictReader(file))
    assert raw['status'] == 'collision'
    run('--scenario', 'straight', '--action', 'stop', expected=1)
  print('CARWORLD_CLI_TESTS passed (nine fixtures, replay, trace, negative controls, IO)')


if __name__ == '__main__':
  main()
