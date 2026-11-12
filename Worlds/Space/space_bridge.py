# Copyright AStarship <https://astarship.net>.
"""ctypes bridge to the surviving SpaceWorld physics (no Environment layer).

Compiles the pure, self-contained SpaceWorld headers verbatim:
SpaceMath.hxx, SpacePhysics.hxx, SpaceRcs.hxx, SpaceCaps.hxx, SpacePid.h.

Two documented build shims are required and applied ONLY here (the shipped
headers are untouched):
  1. ASCIICrabs/_ConfigHeader.h unconditionally #undefs _Seam, so the world seam
     is reasserted after SpaceConfig.h.
  2. SpaceMath.hxx calls _::SquareRoot / IsFinite / Absolute unqualified inside
     namespace iGeek (and even iGeek::IsFinite), and uses sin/cos/acos without
     std::; SpacePhysics.h uses _::ACPUCacheLineSize unqualified. The bridge
     injects using-declarations in namespace iGeek before the .hxx includes.
  3. SpaceMath.hxx / SpaceCaps.hxx include "../../CrabsTK/Math/Blas.h", which
     resolves from Worlds/ (not from Worlds/Space/) — the bridge passes
     -I Worlds so the real /home/astar/AStarStarship/CrabsTK/Math/Blas.h is found.

Not compiled: SpaceEnv (Environment/Gym layer). The demo drives the native
TPaceStep/TSpacePid physics directly.

Build products live exclusively in TMPDIR (Hermes scratch) or SPACE_SCRATCH.
"""
import ctypes
import os
import subprocess
import tempfile
from pathlib import Path

Directory = Path(__file__).resolve().parent
WorldsDir = Directory.parent
CrabsDir = Directory.parent.parent.parent / "ASCIICrabs"

BridgeSource = r"""// Copyright AStarship <https://astarship.net>.
#include "SpaceConfig.h"
#undef _Seam
#define _Seam _iGeekSpaceWorldCore
#include "SpaceMath.h"
#include <cmath>
#include "../../CrabsTK/Math/Blas.h"
namespace iGeek {
using ::_::SquareRoot;
using ::_::IsFinite;
using ::_::Absolute;
using ::_::ACPUCacheLineSize;
using std::sin;
using std::cos;
using std::acos;
}
#include "SpaceMath.hxx"
#include "SpacePhysics.hxx"
#include "SpaceRcs.hxx"
#include "SpaceCaps.hxx"
#include "SpacePid.h"

// Array layouts (kept in sync with space_bridge.py):
//   state[14]   x,y,z, qx,qy,qz,qw, vx,vy,vz, wx,wy,wz, time
//   params[10]  mass, hx,hy,hz, Ixx,Iyy,Izz, dt, omega_max, speed_max
//   control[6]  fx,fy,fz, tx,ty,tz
//   caps[15]    lin_force3, lin_accel3, ang_torque3, ang_accel3, inertia3

static void ReadState(const double* in, iGeek::TSpaceState& s) {
  s.x = in[0]; s.y = in[1]; s.z = in[2];
  s.qx = in[3]; s.qy = in[4]; s.qz = in[5]; s.qw = in[6];
  s.vx = in[7]; s.vy = in[8]; s.vz = in[9];
  s.wx = in[10]; s.wy = in[11]; s.wz = in[12];
  s.time = in[13];
}
static void WriteState(iGeek::TSpaceState& s, double* out) {
  out[0] = s.x; out[1] = s.y; out[2] = s.z;
  out[3] = s.qx; out[4] = s.qy; out[5] = s.qz; out[6] = s.qw;
  out[7] = s.vx; out[8] = s.vy; out[9] = s.vz;
  out[10] = s.wx; out[11] = s.wy; out[12] = s.wz;
  out[13] = s.time;
}
static void ReadParams(const double* in, iGeek::TSpaceParams& p) {
  p.mass = in[0];
  p.half.x = in[1]; p.half.y = in[2]; p.half.z = in[3];
  p.Ixx = in[4]; p.Iyy = in[5]; p.Izz = in[6];
  p.dt = in[7]; p.omega_max = in[8]; p.speed_max = in[9];
}
static void WriteParams(iGeek::TSpaceParams& p, double* out) {
  out[0] = p.mass;
  out[1] = p.half.x; out[2] = p.half.y; out[3] = p.half.z;
  out[4] = p.Ixx; out[5] = p.Iyy; out[6] = p.Izz;
  out[7] = p.dt; out[8] = p.omega_max; out[9] = p.speed_max;
}

extern "C" {

void SpaceBridgeDefaultParams(double* out) {
  iGeek::TSpaceParams p;
  p.mass = 1000.0;
  p.half = iGeek::TSpaceHalf(0.5, 0.5, 1.0);
  p.dt = 1.0 / 60.0;
  p.omega_max = 10.0;
  p.speed_max = 1000.0;
  iGeek::TSpaceInertia(p);
  WriteParams(p, out);
}

void SpaceBridgeInertia(double* params) {
  iGeek::TSpaceParams p;
  ReadParams(params, p);
  iGeek::TSpaceInertia(p);
  WriteParams(p, params);
}

void SpaceBridgeDefaultState(double* out) {
  iGeek::TSpaceState s;
  WriteState(s, out);
}

int SpaceBridgeParamsValid(const double* params) {
  iGeek::TSpaceParams p;
  ReadParams(params, p);
  return iGeek::TSpaceParamsValid(p) ? 1 : 0;
}

int SpaceBridgeStateValid(const double* state) {
  iGeek::TSpaceState s;
  ReadState(state, s);
  return iGeek::TSpaceStateValid(s) ? 1 : 0;
}

int SpaceBridgeStep(double* state, const double* params, const double* control) {
  iGeek::TSpaceState s;
  iGeek::TSpaceParams p;
  iGeek::TSpaceControl c;
  ReadState(state, s);
  ReadParams(params, p);
  c.fx = control[0]; c.fy = control[1]; c.fz = control[2];
  c.tx = control[3]; c.ty = control[4]; c.tz = control[5];
  const bool ok = iGeek::TSpaceStep(s, p, c);
  if (ok) WriteState(s, state);
  return ok ? 1 : 0;
}

void SpaceBridgeCaps(const double* params, const double* thrust_max, double* out) {
  iGeek::TSpaceParams p;
  ReadParams(params, p);
  iGeek::TSpaceThruster thrusters[iGeek::SpaceRcsTotal];
  iGeek::TSpaceRcsBuild(thrusters, p.half);
  double caps_thrust[iGeek::SpaceRcsTotal];
  for (int i = 0; i < iGeek::SpaceRcsTotal; ++i) caps_thrust[i] = thrust_max[i];
  iGeek::TSpaceCaps caps;
  iGeek::TSpaceCapsCompute(caps, p.mass, p.half, thrusters, caps_thrust);
  out[0] = caps.lin_force.x; out[1] = caps.lin_force.y; out[2] = caps.lin_force.z;
  out[3] = caps.lin_accel.x; out[4] = caps.lin_accel.y; out[5] = caps.lin_accel.z;
  out[6] = caps.ang_torque.x; out[7] = caps.ang_torque.y; out[8] = caps.ang_torque.z;
  out[9] = caps.ang_accel.x; out[10] = caps.ang_accel.y; out[11] = caps.ang_accel.z;
  out[12] = caps.inertia.x; out[13] = caps.inertia.y; out[14] = caps.inertia.z;
}

// 24 thrusters: 6 doubles each (position xyz, direction xyz).
void SpaceBridgeRcs(double* out, const double* half) {
  iGeek::TSpaceHalf h;
  h.x = half[0]; h.y = half[1]; h.z = half[2];
  iGeek::TSpaceThruster thrusters[iGeek::SpaceRcsTotal];
  iGeek::TSpaceRcsBuild(thrusters, h);
  for (int i = 0; i < iGeek::SpaceRcsTotal; ++i) {
    out[i * 6 + 0] = thrusters[i].position.x;
    out[i * 6 + 1] = thrusters[i].position.y;
    out[i * 6 + 2] = thrusters[i].position.z;
    out[i * 6 + 3] = thrusters[i].force_dir.x;
    out[i * 6 + 4] = thrusters[i].force_dir.y;
    out[i * 6 + 5] = thrusters[i].force_dir.z;
  }
}

// Rotation damping autopilot: native TSpacePid.Update + native TSpaceStep,
// mirroring Worlds/Space/SpaceWorldMain.hxx cancel_rotation.
// pid[9] = integral3, last_error3, last_derivative3 (persisted by the caller).
int SpaceBridgeStabilize(double* state, const double* params, double* pid,
                         int steps, double max_angular_momentum) {
  iGeek::TSpaceState s;
  iGeek::TSpaceParams p;
  ReadState(state, s);
  ReadParams(params, p);
  iGeek::TSpacePid controller;
  controller.kp = 4.0;
  controller.ki = 0.05;
  controller.kd = 0.5;
  controller.max_integral = 2.0;
  controller.integral = iGeek::TSpaceVec(pid[0], pid[1], pid[2]);
  controller.last_error = iGeek::TSpaceVec(pid[3], pid[4], pid[5]);
  controller.last_derivative = iGeek::TSpaceVec(pid[6], pid[7], pid[8]);
  int applied = 0;
  for (int i = 0; i < steps; ++i) {
    if (!iGeek::TSpaceStateValid(s) || !iGeek::TSpaceParamsValid(p)) break;
    const double lx = p.Ixx * s.wx;
    const double ly = p.Iyy * s.wy;
    const double lz = p.Izz * s.wz;
    auto clamp = [max_angular_momentum](double v) {
      if (v > max_angular_momentum) return max_angular_momentum;
      if (v < -max_angular_momentum) return -max_angular_momentum;
      return v;
    };
    iGeek::TSpaceVec error(-lx, -ly, -lz);
    error.x = clamp(error.x); error.y = clamp(error.y); error.z = clamp(error.z);
    iGeek::TSpaceVec torque = controller.Update(error, p.dt);
    iGeek::TSpaceControl c;
    c.fx = 0; c.fy = 0; c.fz = 0;
    c.tx = torque.x; c.ty = torque.y; c.tz = torque.z;
    if (!iGeek::TSpaceStep(s, p, c)) break;
    ++applied;
  }
  WriteState(s, state);
  pid[0] = controller.integral.x; pid[1] = controller.integral.y; pid[2] = controller.integral.z;
  pid[3] = controller.last_error.x; pid[4] = controller.last_error.y; pid[5] = controller.last_error.z;
  pid[6] = controller.last_derivative.x; pid[7] = controller.last_derivative.y;
  pid[8] = controller.last_derivative.z;
  return applied;
}

}
"""


class ANativeSpace:
  def __init__(self):
    root = Path(os.environ.get("SPACE_SCRATCH", os.environ.get(
        "TMPDIR", str(Path.home() / ".cache" / "space"))))
    root.mkdir(parents=True, exist_ok=True)
    self.build_dir = Path(tempfile.mkdtemp(prefix="space-native-", dir=root))
    source = self.build_dir / "space_bridge.cpp"
    source.write_text(BridgeSource)
    self.library_path = self.build_dir / "space_bridge.so"
    subprocess.run(
        ["g++", "-std=c++17", "-O2", "-shared", "-fPIC", str(source),
         "-I", str(Directory), "-I", str(WorldsDir), "-I", str(CrabsDir),
         "-o", str(self.library_path)],
        check=True, capture_output=True, text=True)
    self.library = ctypes.CDLL(str(self.library_path))

    double = ctypes.c_double
    pointer = ctypes.POINTER(double)
    library = self.library
    library.SpaceBridgeDefaultParams.argtypes = [pointer]
    library.SpaceBridgeInertia.argtypes = [pointer]
    library.SpaceBridgeDefaultState.argtypes = [pointer]
    library.SpaceBridgeParamsValid.argtypes = [pointer]
    library.SpaceBridgeParamsValid.restype = ctypes.c_int
    library.SpaceBridgeStateValid.argtypes = [pointer]
    library.SpaceBridgeStateValid.restype = ctypes.c_int
    library.SpaceBridgeStep.argtypes = [pointer, pointer, pointer]
    library.SpaceBridgeStep.restype = ctypes.c_int
    library.SpaceBridgeCaps.argtypes = [pointer, pointer, pointer]
    library.SpaceBridgeRcs.argtypes = [pointer, pointer]
    library.SpaceBridgeStabilize.argtypes = [
        pointer, pointer, pointer, ctypes.c_int, double]
    library.SpaceBridgeStabilize.restype = ctypes.c_int

    self.StateSize = 14
    self.ParamsSize = 10
    self.ControlSize = 6
    self.CapsSize = 15
    self.RcsCount = 24

  def DefaultParams(self):
    out = (ctypes.c_double * self.ParamsSize)()
    self.library.SpaceBridgeDefaultParams(out)
    return list(out)

  def DefaultState(self):
    out = (ctypes.c_double * self.StateSize)()
    self.library.SpaceBridgeDefaultState(out)
    return list(out)


Engine = ANativeSpace()
