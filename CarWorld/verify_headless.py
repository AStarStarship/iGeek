# Copyright AStarship <https://astarship.net>.
"""Reproducible counted CPU evidence; exclusive-create archives, no downloads."""
import argparse
import collections
import csv
from datetime import datetime, timezone
import hashlib
import json
import math
from pathlib import Path
import platform
import re
import shutil
import statistics
import subprocess

SCENARIOS = ('straight', 'curves', 'obstacle', 'gravel', 'ice', 'uphill',
             'downhill', 'launch', 'ice_curves')
ROOT = Path(__file__).resolve().parents[1]
FLEET = ROOT.parent


def digest(path):
  return hashlib.sha256(path.read_bytes()).hexdigest()


def read_csv(path):
  with path.open() as file:
    return list(csv.DictReader(file))


def parse_total(text):
  line = next(line for line in text.splitlines() if line.startswith('TOTAL '))
  fields = dict(part.split('=', 1) for part in line.split()[1:])
  goals, total = fields['goals'].split('/')
  result = {key: float(fields[key]) for key in
            ('simulated_s', 'wall_s', 'frames_per_s', 'realtime_factor')}
  result.update(goals=int(goals), total=int(total), frames=int(fields['frames']),
                physics_steps=int(fields['physics_steps']))
  assert all(math.isfinite(v) and v > 0 for v in result.values())
  assert result['physics_steps'] == 4 * result['frames']
  assert math.isclose(result['simulated_s'], 0.02 * result['frames'], abs_tol=1e-6)
  assert math.isclose(result['frames_per_s'], result['frames'] / result['wall_s'], rel_tol=1e-5)
  # CLI prints rates to 0.1 units; allow half a printed unit plus clock rounding.
  assert math.isclose(result['realtime_factor'], result['simulated_s'] / result['wall_s'],
                      rel_tol=1e-5, abs_tol=0.051)
  return result


def segment_hits_box(a, b, box):
  enter, leave = 0.0, 1.0
  for origin, end, low, high in zip(a, b, box[:2], box[2:]):
    delta = end - origin
    if abs(delta) < 1e-14:
      if origin < low or origin > high:
        return False
    else:
      near, far = sorted(((low - origin) / delta, (high - origin) / delta))
      enter, leave = max(enter, near), min(leave, far)
      if enter > leave:
        return False
  return True


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('--binary', type=Path, default=ROOT / 'CarWorld/build/carworld')
  parser.add_argument('--out', type=Path, required=True)
  parser.add_argument('--episodes', type=int, default=100)
  args = parser.parse_args()
  assert 1 <= args.episodes <= 10000
  binary = args.binary.resolve()
  assert binary.is_file()
  output = args.out.resolve()
  output.mkdir(parents=True, exist_ok=False)

  def run(name, *flags, expected=0, program=binary):
    result = subprocess.run([str(program), *map(str, flags)], cwd=ROOT,
                            capture_output=True, text=True, timeout=300)
    (output / (name + '.stdout.txt')).write_text(result.stdout)
    (output / (name + '.stderr.txt')).write_text(result.stderr)
    assert result.returncode == expected, (name, result.returncode, result.stderr)
    return result.stdout

  # Freeze every project-owned compiler dependency (GNU/Clang .o.d), plus tools.
  sources = set()
  for dependency in (binary.parent / 'CMakeFiles').glob('*.dir/*.o.d'):
    tokens = dependency.read_text().replace('\\\n', ' ').split(':', 1)[1].split()
    for token in tokens:
      path = Path(token).resolve()
      if path.is_file() and path.is_relative_to(FLEET):
        sources.add(path)
  assert ROOT / 'CarWorld/CarPhysics.hxx' in sources
  assert ROOT / 'BikeWorld/VehicleRoute.hxx' in sources
  for name in ('CMakeLists.txt', 'cli_tests.py', 'verify_headless.py', 'verification_tests.py'):
    sources.add(ROOT / 'CarWorld' / name)
  initial_hashes = {str(p.relative_to(FLEET)): digest(p) for p in sorted(sources)}
  for path in sources:
    destination = output / 'code' / path.relative_to(FLEET)
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(path, destination)
  (output / 'bin').mkdir()
  for name in ('carworld', 'carworld_tests', 'carworld_ppo_smoke'):
    shutil.copy2(binary.parent / name, output / 'bin' / name)
  executable = output / 'bin/carworld'
  run('physics_tests', program=output / 'bin/carworld_tests')
  run('ppo_smoke', program=output / 'bin/carworld_ppo_smoke')
  run('release_ctest', '--test-dir', binary.parent, '--output-on-failure', program=Path('ctest'))
  sanitized = binary.parent / 'sanitized'
  assert (sanitized / 'carworld').is_file(), 'build sanitizers first'
  run('sanitizer_ctest', '--test-dir', sanitized, '--output-on-failure', program=Path('ctest'))

  total_expected = len(SCENARIOS) * args.episodes
  reports = [output / 'drives.csv', output / 'replay.csv']
  for index, path in enumerate(reports):
    total = parse_total(run(f'drives_{index}', '--scenario', 'all', '--episodes',
                            args.episodes, '--seed', 7, '--csv', path, program=executable))
    assert total['goals'] == total['total'] == total_expected
  assert reports[0].read_bytes() == reports[1].read_bytes()
  rows = read_csv(reports[0])
  assert len(rows) == total_expected
  groups = collections.defaultdict(list)
  for row in rows:
    assert row['status'] == 'goal'
    assert all(math.isfinite(float(v)) for k, v in row.items() if k not in ('scenario', 'status'))
    assert float(row['goal_error_m']) < 1.2 and float(row['final_speed_mps']) < 0.1
    assert float(row['peak_cross_track_m']) < 4
    assert int(row['physics_steps']) == 4 * int(row['frames'])
    assert math.isclose(float(row['sim_time_s']), 0.02 * int(row['frames']), abs_tol=1e-7)
    groups[row['scenario']].append(row)
  assert set(groups) == set(SCENARIOS)
  summary = {}
  for scenario in SCENARIOS:
    group = groups[scenario]
    assert len(group) == args.episodes
    assert [int(r['seed']) for r in group] == list(range(7, 7 + args.episodes))
    summary[scenario] = dict(goals=len(group), mean_sim_time_s=statistics.mean(
        float(r['sim_time_s']) for r in group), peak_cross_track_m=max(
        float(r['peak_cross_track_m']) for r in group), worst_goal_error_m=max(
        float(r['goal_error_m']) for r in group), peak_lateral_accel_mps2=max(
        float(r['peak_lateral_accel_mps2']) for r in group))

  traces = {}
  for scenario in ('obstacle', 'ice_curves'):
    trace = output / (scenario + '_trace.csv')
    metrics = output / (scenario + '_single.csv')
    run(scenario, '--scenario', scenario, '--trace', trace, '--csv', metrics, program=executable)
    frames, metric = read_csv(trace), read_csv(metrics)[0]
    assert len(frames) == int(metric['frames']) + 1
    assert frames[0]['time_s'] == '0.000000' and frames[-1]['status'] == 'goal'
    previous = None
    for index, frame in enumerate(frames):
      assert all(math.isfinite(float(v)) for k, v in frame.items() if k != 'status')
      assert math.isclose(float(frame['time_s']), index * 0.02, abs_tol=1e-7)
      point = float(frame['x_m']), float(frame['y_m'])
      assert 2.65 < point[0] < 117.35 and 2.65 < point[1] < 61.35
      if scenario == 'obstacle' and previous is not None:
        assert not segment_hits_box(previous, point, (47.35, 22.35, 61.65, 43.65))
      previous = point
    traces[scenario] = dict(rows=len(frames), terminal=frames[-1]['status'],
                            swept_body_check=scenario == 'obstacle')

  run('emergency', '--scenario', 'straight', '--barrier', '--trace',
      output / 'emergency_trace.csv', '--csv', output / 'emergency.csv',
      expected=1, program=executable)
  emergency = read_csv(output / 'emergency.csv')[0]
  final = read_csv(output / 'emergency_trace.csv')[-1]
  gap = 45 - 2.65 - float(final['x_m'])
  assert emergency['status'] == 'blocked' and gap > 0.25
  assert float(emergency['final_speed_mps']) < 0.1
  run('open_loop', '--scenario', 'obstacle', '--open-loop', '--csv',
      output / 'open_loop.csv', expected=1, program=executable)
  assert read_csv(output / 'open_loop.csv')[0]['status'] == 'collision'
  samples = [parse_total(run(f'timing_{i}', '--scenario', 'all', '--episodes',
                             args.episodes, '--seed', 7, program=executable)) for i in range(3)]
  for sample in samples:
    assert sample['goals'] == sample['total'] == total_expected
    assert sample['frames'] == sum(int(r['frames']) for r in rows)
  assert initial_hashes == {str(p.relative_to(FLEET)): digest(p) for p in sorted(sources)}
  artifacts = {str(p.relative_to(output)): digest(p) for p in sorted(output.rglob('*')) if p.is_file()}
  result = dict(scope='fixed fixtures, seeded initial pose, classical driver; not learned autonomy',
                created_utc=datetime.now(timezone.utc).isoformat(), machine=platform.uname()._asdict(),
                episodes_per_fixture=args.episodes, drives=len(rows), goals=len(rows),
                deterministic_csv_replay=True, physics_substep_s=0.005, control_frame_s=0.02,
                fixtures=summary, traces=traces, emergency_gap_m=gap,
                throughput_samples=samples,
                median_frames_per_s=statistics.median(s['frames_per_s'] for s in samples),
                median_realtime_factor=statistics.median(s['realtime_factor'] for s in samples),
                throughput_scope='one CPU thread; fixture construction/planning/controller/physics/observations; '
                                 'no CSV/trace, startup, planner allocation or final output flush',
                source_sha256=initial_hashes, artifact_sha256=artifacts)
  (output / 'verification_summary.json').write_text(json.dumps(result, indent=2) + '\n')
  print(json.dumps({k: result[k] for k in ('drives', 'goals', 'deterministic_csv_replay',
      'fixtures', 'emergency_gap_m', 'median_frames_per_s', 'median_realtime_factor')}, indent=2))
  print('VERIFICATION_ARCHIVE ' + str(output))


if __name__ == '__main__':
  main()
