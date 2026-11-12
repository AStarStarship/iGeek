# Copyright AStarship <https://astarship.net>.
"""SpaceWorld browser demo (Flask, port 5009).

6-DoF zero-g spacecraft driven by the surviving native physics:
TSpaceStep, TSpaceInertia, TSpacePid (rotation damping autopilot) and the RCS
layout, compiled verbatim by space_bridge.py. The SpaceEnv / Environment layer
is NOT compiled; the seeded initial tumble mirrors SpaceEnv::Reset's documented
formula, and everything else is native.

Run: python Worlds/Space/space_server.py
Sessions are in-memory, single-process spacecraft, one per cookie.
"""
from collections import OrderedDict
import ctypes
import math
from pathlib import Path
import secrets
import threading

from flask import Flask, jsonify, request, send_from_directory

try:
  from .space_bridge import Engine
except ImportError:
  from space_bridge import Engine

Directory = Path(__file__).resolve().parent
MaxSeed = 2 ** 53 - 1
MaxSessions = 256
MaxForce = 1000.0
MaxTorque = 500.0
MaxBurnSteps = 120
MaxStabilizeSteps = 600


def DoubleArray(values, name, limit):
  if type(values) is not list or len(values) != 3:
    raise ValueError(f"{name} must be a 3-element array.")
  out = []
  for value in values:
    if type(value) not in (int, float) or not math.isfinite(value):
      raise ValueError(f"{name} must contain finite numbers.")
    if abs(value) > limit:
      raise ValueError(f"{name} components must be within +/-{limit}.")
    out.append(float(value))
  return out


class ASpaceSession:
  def __init__(self, seed=1):
    self.Params = (ctypes.c_double * Engine.ParamsSize)()
    Engine.library.SpaceBridgeDefaultParams(self.Params)
    self.State = (ctypes.c_double * Engine.StateSize)()
    self.Pid = (ctypes.c_double * 9)()
    self.seed = 1
    self.total_steps = 0
    self.burns = 0
    self.message = "Idle. Fire a thruster or run the stabilizer."
    self.Reset(seed)

  def Reset(self, seed=None):
    if seed is not None:
      self.seed = seed
    Engine.library.SpaceBridgeDefaultState(self.State)
    # Mirrors SpaceEnv::Reset's seeded tumble (env layer not compiled).
    self.State[10] = float(((self.seed + 1) % 7) - 3) * 0.1
    self.State[11] = float(((self.seed + 3) % 5) - 2) * 0.1
    self.State[12] = float(((self.seed + 5) % 9) - 4) * 0.1
    for i in range(9):
      self.Pid[i] = 0.0
    self.total_steps = 0
    self.burns = 0
    self.message = "Reset: identity attitude, seeded tumble, at rest at the origin."

  def Step(self, control, steps):
    Control = (ctypes.c_double * Engine.ControlSize)(*control)
    applied = 0
    for _ in range(steps):
      if Engine.library.SpaceBridgeStep(self.State, self.Params, Control) != 1:
        break
      applied += 1
    self.total_steps += applied
    return applied

  def Burn(self, force, torque, steps):
    control = [force[0], force[1], force[2], torque[0], torque[1], torque[2]]
    applied = self.Step(control, steps)
    self.burns += 1
    self.message = (f"Burn applied for {applied} native 60 Hz steps "
                    f"(force {force}, torque {torque}).")
    if applied != steps:
      self.message = "Native step rejected the control (state/params invalid)."

  def Stabilize(self, steps):
    applied = Engine.library.SpaceBridgeStabilize(
        self.State, self.Params, self.Pid, steps, 2.0)
    self.total_steps += applied
    self.message = (f"Native PID rotation damping ran {applied} steps "
                    f"(cancel_rotation autopilot).")

  def Spec(self):
    thrust = (ctypes.c_double * Engine.RcsCount)(*([500.0] * Engine.RcsCount))
    Caps = (ctypes.c_double * Engine.CapsSize)()
    Engine.library.SpaceBridgeCaps(self.Params, thrust, Caps)
    Rcs = (ctypes.c_double * (Engine.RcsCount * 6))()
    Half = (ctypes.c_double * 3)(self.Params[1], self.Params[2], self.Params[3])
    Engine.library.SpaceBridgeRcs(Rcs, Half)
    return {
        "mass": self.Params[0],
        "half": [self.Params[1], self.Params[2], self.Params[3]],
        "dt": self.Params[7],
        "inertia": [Caps[12], Caps[13], Caps[14]],
        "caps": {
            "lin_force": [round(Caps[i], 4) for i in (0, 1, 2)],
            "lin_accel": [round(Caps[i], 4) for i in (3, 4, 5)],
            "ang_torque": [round(Caps[i], 4) for i in (6, 7, 8)],
            "ang_accel": [round(Caps[i], 4) for i in (9, 10, 11)],
        },
        "thrusters": [[round(v, 4) for v in Rcs[i * 6:(i + 1) * 6]]
                      for i in range(Engine.RcsCount)],
    }

  def StateDict(self):
    s = list(self.State)
    qx, qy, qz, qw = s[3], s[4], s[5], s[6]
    # ZYX Euler angles for the HUD (display only, not simulation state).
    roll = math.atan2(2 * (qw * qx + qy * qz), 1 - 2 * (qx * qx + qy * qy))
    pitch = math.asin(max(-1.0, min(1.0, 2 * (qw * qy - qz * qx))))
    yaw = math.atan2(2 * (qw * qz + qx * qy), 1 - 2 * (qy * qy + qz * qz))
    return {
        "position": [round(v, 6) for v in s[0:3]],
        "quaternion": [round(v, 6) for v in s[3:7]],
        "velocity": [round(v, 6) for v in s[7:10]],
        "angular": [round(v, 6) for v in s[10:13]],
        "time": round(s[13], 4),
        "speed": round(math.sqrt(s[7] ** 2 + s[8] ** 2 + s[9] ** 2), 6),
        "spin": round(math.sqrt(s[10] ** 2 + s[11] ** 2 + s[12] ** 2), 6),
        "attitude": [round(roll, 4), round(pitch, 4), round(yaw, 4)],
        "seed": self.seed,
        "total_steps": self.total_steps,
        "burns": self.burns,
        "message": self.message,
    }


def CreateApp():
  app = Flask(__name__, static_folder=None)
  app.config["MAX_CONTENT_LENGTH"] = 4096
  sessions = OrderedDict()
  lock = threading.RLock()

  @app.after_request
  def Headers(response):
    response.headers["Cache-Control"] = "no-store"
    response.headers["X-Content-Type-Options"] = "nosniff"
    return response

  def Dispatch():
    token = request.cookies.get("space_session")
    if token not in sessions:
      token = secrets.token_hex(16)
      sessions[token] = ASpaceSession(1)
      while len(sessions) > MaxSessions:
        sessions.popitem(last=False)
    sessions.move_to_end(token)
    return token, sessions[token]

  def Payload():
    payload = request.get_json(silent=True)
    if type(payload) is not dict:
      raise ValueError("Expected a JSON object.")
    return payload

  def Integer(value, low, high, name):
    if type(value) is not int or not low <= value <= high:
      raise ValueError(f"{name} must be an integer in {low}..{high}.")
    return value

  @app.errorhandler(ValueError)
  def Invalid(error):
    return jsonify(error=str(error)), 400

  @app.get("/")
  @app.get("/space.html")
  def Index():
    return send_from_directory(Directory, "space.html")

  @app.get("/api/state")
  def State():
    with lock:
      token, craft = Dispatch()
      response = jsonify(craft.StateDict())
      response.set_cookie("space_session", token, httponly=True,
                          samesite="Strict", secure=request.is_secure)
      return response

  @app.get("/api/spec")
  def Spec():
    with lock:
      _, craft = Dispatch()
      return jsonify(craft.Spec())

  @app.post("/api/new")
  def New():
    with lock:
      token, craft = Dispatch()
      payload = Payload()
      if set(payload) != {"seed"}:
        raise ValueError("Expected exactly seed.")
      seed = Integer(payload["seed"], 0, MaxSeed, "seed")
      craft.Reset(seed)
      response = jsonify(craft.StateDict())
      response.set_cookie("space_session", token, httponly=True,
                          samesite="Strict", secure=request.is_secure)
      return response

  @app.post("/api/action")
  def Action():
    with lock:
      token, craft = Dispatch()
      payload = Payload()
      action = payload.get("action")
      if type(action) is not str:
        raise ValueError("Expected a string action.")
      if action == "burn":
        if set(payload) != {"action", "force", "torque", "steps"}:
          raise ValueError("burn requires exactly action, force, torque and steps.")
        force = DoubleArray(payload["force"], "force", MaxForce)
        torque = DoubleArray(payload["torque"], "torque", MaxTorque)
        steps = Integer(payload["steps"], 1, MaxBurnSteps, "steps")
        craft.Burn(force, torque, steps)
      elif action == "stabilize":
        if set(payload) != {"action", "steps"}:
          raise ValueError("stabilize requires exactly action and steps.")
        steps = Integer(payload["steps"], 1, MaxStabilizeSteps, "steps")
        craft.Stabilize(steps)
      elif action == "reset":
        if set(payload) != {"action"}:
          raise ValueError("reset takes no extra fields.")
        craft.Reset()
      else:
        raise ValueError("Unknown action.")
      response = jsonify(craft.StateDict())
      response.set_cookie("space_session", token, httponly=True,
                          samesite="Strict", secure=request.is_secure)
      return response

  @app.get("/api/manifest")
  def Manifest():
    return jsonify({
        "name": "SpaceWorld browser demo",
        "version": "1.0",
        "mode": "browser-human",
        "engine": "native",
        "engine_detail":
            "C ABI compiled from verbatim SpaceMath.hxx, SpacePhysics.hxx, "
            "SpaceRcs.hxx, SpaceCaps.hxx and SpacePid.h (TSpaceStep, TSpaceInertia, "
            "TSpacePid, RCS layout).",
        "native_gym_difference":
            "SpaceEnv is not compiled. The seeded initial tumble mirrors "
            "SpaceEnv::Reset's documented formula in Python; stabilization uses the "
            "native TSpacePid.Update + TSpaceStep and mirrors SpaceWorldMain.hxx "
            "cancel_rotation. No PPO/Gym reward loop is claimed.",
        "observation": {
            "position": "[x, y, z] metres, world frame",
            "quaternion": "[qx, qy, qz, qw] unit quaternion",
            "velocity": "[vx, vy, vz] m/s, world frame",
            "angular": "[wx, wy, wz] rad/s, body frame",
            "time": "simulated seconds",
            "speed": "|velocity| m/s",
            "spin": "|angular| rad/s",
            "attitude": "[roll, pitch, yaw] radians, display-only ZYX Euler",
            "seed": "tumble seed, 0..2^53-1",
            "total_steps": "native 60 Hz steps applied since reset",
            "burns": "burn actions since reset",
            "message": "human-readable last action status"},
        "actions": {
            "GET /api/state": "Returns observation",
            "GET /api/spec": "Static mass/inertia/caps/24-thruster RCS layout",
            "POST /api/action": [
                {"action": "burn", "force": "[fx,fy,fz] N world, each within +/-1000",
                 "torque": "[tx,ty,tz] N m body, each within +/-500",
                 "steps": "integer 1..120 at 60 Hz (max 2 s)"},
                {"action": "stabilize", "steps": "integer 1..600; native PID rotation damping"},
                {"action": "reset", "effect": "same seed tumble, state at rest at origin"}],
            "POST /api/new": {"seed": "integer 0..2^53-1"},
            "GET /api/manifest": "This contract"},
        "responses": "Successful mutations return observation. Invalid JSON, types, "
                     "bounds, extra fields or unknown actions: HTTP 400 {error:string} "
                     "and no state mutation. Booleans are not numbers.",
        "sessions": "Opaque HttpOnly SameSite=Strict cookie; in-memory single-process; "
                    "restart clears craft; up to 256 sessions, oldest evicted.",
        "routes": ["/", "/space.html", "/api/state", "/api/spec", "/api/action",
                   "/api/new", "/api/manifest"]})

  return app


app = CreateApp()

if __name__ == "__main__":
  app.run(host="0.0.0.0", port=5009, debug=False, threaded=True)
