# Copyright AStarship <https://astarship.net>.
"""ctypes adapter to the real ACarEnv, TileMap, planner and four-tire plant."""
import ctypes as ct
import math
import os
from pathlib import Path
import subprocess
import tempfile

Root = Path(__file__).resolve().parents[2]
Directory = Path(__file__).resolve().parent
Scenarios = ('straight', 'curves', 'obstacle', 'gravel', 'ice', 'uphill',
             'downhill', 'launch', 'ice_curves', 'track')
Statuses = ('running', 'goal', 'collision', 'timeout', 'invalid', 'blocked', 'stopped')


def Build():
  scratch = Path(os.environ.get('TMPDIR', Path.home() / '.cache/carworld'))
  scratch.mkdir(parents=True, exist_ok=True)
  build = Path(tempfile.mkdtemp(prefix='car-browser-', dir=scratch))
  definitions = '\n'.join(line for line in (Root / '_Seams/_Seams.cpp').read_text().splitlines()
                          if line.startswith('#define '))
  prelude = definitions + '''
#include <ASCIICrabs/_Config.h>
#include <ASCIICrabs/Math.hxx>
#define _Seam _SeamN
#include <ASCIICrabs/_Package.hxx>
#include <CrabsTK/Exo/_Package.hxx>
// Adapter-local overload resolves existing VehicleRoute integer Absolute calls.
namespace _ { inline ISC Absolute(ISC value) { return value < 0 ? -value : value; } }
'''
  for name in ('EnvironmentManifest.hxx', 'Environment.hxx',
               'Worlds/Bike/TileRuntime.hxx', 'Worlds/Car2D/CarScenarios.hxx',
               'Worlds/Car2D/car_bridge.cpp'):
    prelude += f'#include "{Root / name}"\n'
  source = build / 'CarBrowser.cpp'
  source.write_text(prelude)
  library = build / 'car_browser.so'
  command = ['g++', '-std=c++23', '-O2', '-shared', '-fPIC', str(source), '-o', str(library)]
  command += ['-I' + str(path) for path in (Root / '_Seams', Root, Root.parent,
    Root.parent / 'ASCIICrabs', Root.parent / 'ASCIICrabs/_Seams',
    Root.parent / 'CrabsTK', Root.parent / 'CrabsTK/Exo')]
  result = subprocess.run(command, capture_output=True, text=True)
  (build / 'compile.log').write_text(result.stdout + result.stderr)
  if result.returncode:
    raise RuntimeError(f'Native car build failed; see {build / "compile.log"}')
  return ct.CDLL(str(library))


Native = Build()
Native.CarBrowserCreate.argtypes = [ct.c_int32, ct.c_uint64]
Native.CarBrowserCreate.restype = ct.c_void_p
Native.CarBrowserDestroy.argtypes = [ct.c_void_p]
Native.CarBrowserDestroy.restype = None
Native.CarBrowserStep.argtypes = [ct.c_void_p, ct.c_int32, ct.c_int32] + [ct.c_double] * 4
Native.CarBrowserStep.restype = ct.c_double
Native.CarBrowserBarrier.argtypes = [ct.c_void_p]
Native.CarBrowserBarrier.restype = None
Native.CarBrowserState.argtypes = [ct.c_void_p, ct.POINTER(ct.c_double), ct.POINTER(ct.c_float)]
Native.CarBrowserState.restype = None
Native.CarBrowserMap.argtypes = [ct.c_void_p, ct.POINTER(ct.c_int32), ct.POINTER(ct.c_double)]
Native.CarBrowserMap.restype = ct.c_int32


class ACarDemo:
  def __init__(self, scenario='straight', seed=7):
    if scenario not in Scenarios or type(seed) is not int or not 0 <= seed <= 2**32-1:
      raise ValueError('Invalid scenario or seed')
    self.handle = Native.CarBrowserCreate(Scenarios.index(scenario), seed)
    if not self.handle:
      raise RuntimeError('Native scenario planning failed')
    self.scenario, self.seed, self.reward = scenario, seed, 0
    self.barrier = False

  def __enter__(self):
    return self

  def __exit__(self, *args):
    self.Close()

  def Close(self):
    if self.handle:
      Native.CarBrowserDestroy(self.handle)
      self.handle = None

  def Step(self, mode='pilot', frames=5, steer=0, throttle=0, brake=0, reverse=0):
    if mode not in ('pilot', 'manual', 'stop') or type(frames) is not int or not 1 <= frames <= 50:
      raise ValueError('Invalid control mode or frame count')
    for value, low, high in ((steer, -.55, .55), (throttle, 0, 1), (brake, 0, 1)):
      if type(value) not in (int, float) or isinstance(value, bool) or \
              not math.isfinite(value) or not low <= value <= high:
        raise ValueError('Control outside finite actuator range')
    if isinstance(reverse, bool) or reverse not in (0, 1):
      raise ValueError('Reverse must be 0 or 1')
    self.reward += Native.CarBrowserStep(self.handle, ('pilot', 'manual', 'stop').index(mode),
                                        frames, steer, throttle, brake, float(reverse))

  def Barrier(self):
    if not self.barrier:
      Native.CarBrowserBarrier(self.handle)
      self.barrier = True

  def State(self):
    values, observations = (ct.c_double * 15)(), (ct.c_float * 22)()
    Native.CarBrowserState(self.handle, values, observations)
    fields = ('x', 'y', 'heading', 'forward', 'lateral', 'yaw_rate', 'steer', 'time',
              'distance', 'longitudinal_accel', 'lateral_accel', 'utilization')
    state = dict(zip(fields, values))
    state.update(status=Statuses[int(values[12])], done=values[12] != 0,
      goal_distance=values[13], frames=int(values[14]), observation=list(observations),
      speed=math.hypot(values[3], values[4]), scenario=self.scenario, seed=self.seed,
      reward=self.reward, barrier=self.barrier)
    return state

  def Map(self):
    tiles, points = (ct.c_int32 * (120*64))(), (ct.c_double * 512)()
    count = Native.CarBrowserMap(self.handle, tiles, points)
    return dict(width=120, height=64, tiles=list(tiles),
                route=[[points[i*2], points[i*2+1]] for i in range(count)])
