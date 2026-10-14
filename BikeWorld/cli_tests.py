# Copyright AStarship <https://astarship.net>.
"""Black-box CLI checks; Python is test tooling, not a simulator dependency."""
import csv
import math
import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
  binary = Path(sys.argv[1]).resolve()
  assert binary.is_file(), f"headless CLI not built: {binary}"

  def run(*args, expected=0):
    result = subprocess.run([str(binary), *args], capture_output=True,
                            text=True, timeout=120)
    assert result.returncode == expected, (args, result.stdout, result.stderr)
    return result.stdout

  assert "--scenario" in run("--help")
  for args in [("--episodes", "-1"), ("--seed", "-1"),
               ("--episodes", "1junk"), ("--scenario", "unknown"),
               ("--scenario",), ("--unknown",)]:
    run(*args, expected=2)
  with tempfile.TemporaryDirectory(dir=os.environ.get("TMPDIR")) as directory:
    root = Path(directory)
    reports = [root / "a.csv", root / "b.csv"]
    for report in reports:
      run("--scenario", "all", "--episodes", "2", "--seed", "7",
          "--csv", str(report))
    assert reports[0].read_bytes() == reports[1].read_bytes()
    with reports[0].open() as handle:
      rows = list(csv.DictReader(handle))
    assert len(rows) == 16
    assert len({row["scenario"] for row in rows}) == 8
    assert all(row["status"] == "goal" for row in rows)
    assert all(float(row["goal_error_m"]) < 1.2 for row in rows)
    assert all(float(row["final_speed_mps"]) < 0.1 for row in rows)
    before = reports[0].read_bytes()
    run("--csv", str(reports[0]), expected=2)
    assert reports[0].read_bytes() == before
    trace = root / "trace.csv"
    run("--scenario", "obstacle", "--seed", "7", "--trace", str(trace))
    with trace.open() as handle:
      frames = list(csv.DictReader(handle))
    assert len(frames) > 1000
    assert frames[-1]["status"] == "goal"
    assert all(math.isfinite(float(frame["roll_rad"])) for frame in frames)
    assert float(frames[-1]["x_m"]) - float(frames[0]["x_m"]) > 90
    raw = root / "raw.csv"
    run("--scenario", "straight", "--seed", "7", "--unassisted",
        "--csv", str(raw), expected=1)
    with raw.open() as handle:
      row = next(csv.DictReader(handle))
    assert row["status"] == "fallen"
  print("BIKEWORLD_CLI_TESTS passed (replay, eight fixtures, trace, failures, IO)")


if __name__ == "__main__":
  main()
