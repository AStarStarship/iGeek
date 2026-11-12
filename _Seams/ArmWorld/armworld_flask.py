#!/usr/bin/env python3
"""ArmWorld Python Flask WebGL — real-time 2-link arm simulation in browser.

Serves the demo page at http://127.0.0.1:5000 and streams simulation state
over WebSocket (flask-socketio). Pure-Python RK4 dynamics matching the
C++ ArmStep semantics: torque clamp, RK4 substeps, joint bounds [0, pi/2],
velocity zeroing at bounds, forward kinematics output.
"""

import os
import math

from flask import Flask, send_from_directory, jsonify
from flask_socketio import SocketIO, emit

# ---------------------------------------------------------------------------
# Physical parameters (match the C++ defaults)
# ---------------------------------------------------------------------------
ARM_L1, ARM_L2 = 0.20, 0.20        # m
ARM_M1, ARM_M2 = 2.0, 1.0          # kg
ARM_I1, ARM_I2 = 0.05, 0.0167      # kg m^2
ARM_MAX_TORQUE, ARM_DAMPING = 3.0, 0.01
ARM_DT, ARM_HALF_STEP = 0.02, 0.01 # s
GRAVITY = 9.81

arm_state = {
    "j1": 0.0, "j2": 0.0, "w1": 0.0, "w2": 0.0,
    "time": 0.0, "tip_x": 0.20, "tip_y": 0.0, "reach": 0.20,
}


def _dynamics(j1, j2, w1, w2, tau1, tau2):
    """Return (dj1, dj2, dw1, dw2) at the given state (one-frame torques only)."""
    lc1, lc2 = 0.5 * ARM_L1, 0.5 * ARM_L2
    c12 = -ARM_M2 * ARM_L1 * lc2 * math.sin(j1 - j2)
    I11 = (ARM_I1 + ARM_M1 * lc1 * lc1 + ARM_M2 * ARM_L1 * ARM_L1
           + ARM_M2 * lc2 * lc2 + 2.0 * ARM_M2 * ARM_L1 * lc2 * math.cos(j1 - j2))
    I12 = ARM_M2 * ARM_L1 * lc2 * math.cos(j1 - j2)
    I22 = ARM_I2 + ARM_M2 * lc2 * lc2
    grav1 = (-(ARM_M1 * lc1 + ARM_M2 * ARM_L1) * GRAVITY * math.sin(j1)
             - ARM_M2 * lc2 * GRAVITY * math.sin(j2))
    grav2 = -ARM_M2 * lc2 * GRAVITY * math.sin(j2)
    f1 = tau1 + c12 * w2 * w2 - ARM_DAMPING * w1 + grav1
    f2 = tau2 - c12 * w1 * w1 - ARM_DAMPING * w2 + grav2
    det = I11 * I22 - I12 * I12
    if abs(det) < 1e-12:
        return w1, w2, 0.0, 0.0
    a1 = (I22 * f1 - I12 * f2) / det
    a2 = (-I12 * f1 + I11 * f2) / det
    return w1, w2, a1, a2


def _arm_fk(j1, j2, l1=ARM_L1, l2=ARM_L2):
    tip_x = l1 * math.cos(j1) + l2 * math.cos(j2)
    tip_y = l1 * math.sin(j1) + l2 * math.sin(j2)
    reach = math.hypot(tip_x, tip_y)
    return tip_x, tip_y, reach


def arm_step(state, tau1, tau2, dt=ARM_DT, half_step=ARM_HALF_STEP):
    """Advance the simulation by one control frame (dt). Returns the dict."""
    tau1 = max(-ARM_MAX_TORQUE, min(ARM_MAX_TORQUE, tau1))
    tau2 = max(-ARM_MAX_TORQUE, min(ARM_MAX_TORQUE, tau2))
    n_sub = int(round(dt / half_step))
    if n_sub < 1:
        n_sub = 1
    j1, j2 = state["j1"], state["j2"]
    w1, w2 = state["w1"], state["w2"]
    for _ in range(n_sub):
        h = dt / n_sub
        k1 = _dynamics(j1, j2, w1, w2, tau1, tau2)
        k2 = _dynamics(j1 + h / 2 * k1[0], j2 + h / 2 * k1[1],
                       w1 + h / 2 * k1[2], w2 + h / 2 * k1[3], tau1, tau2)
        k3 = _dynamics(j1 + h / 2 * k2[0], j2 + h / 2 * k2[1],
                       w1 + h / 2 * k2[2], w2 + h / 2 * k2[3], tau1, tau2)
        k4 = _dynamics(j1 + h * k3[0], j2 + h * k3[1],
                       w1 + h * k3[2], w2 + h * k3[3], tau1, tau2)
        j1 += h / 6.0 * (k1[0] + 2 * k2[0] + 2 * k3[0] + k4[0])
        j2 += h / 6.0 * (k1[1] + 2 * k2[1] + 2 * k3[1] + k4[1])
        w1 += h / 6.0 * (k1[2] + 2 * k2[2] + 2 * k3[2] + k4[2])
        w2 += h / 6.0 * (k1[3] + 2 * k2[3] + 2 * k3[3] + k4[3])
    # Joint bounds [0, pi/2], zero velocity into the bound
    j1 = max(0.0, min(math.pi / 2, j1))
    j2 = max(0.0, min(math.pi / 2, j2))
    if j1 <= 0.0 and w1 < 0.0:
        w1 = 0.0
    if j2 <= 0.0 and w2 < 0.0:
        w2 = 0.0
    tip_x, tip_y, reach = _arm_fk(j1, j2)
    state["j1"], state["j2"] = j1, j2
    state["w1"], state["w2"] = w1, w2
    state["time"] += dt
    state["tip_x"], state["tip_y"], state["reach"] = tip_x, tip_y, reach
    return state


def _snapshot():
    return {
        "j1": arm_state["j1"], "j2": arm_state["j2"],
        "w1": arm_state["w1"], "w2": arm_state["w2"],
        "tip_x": arm_state["tip_x"], "tip_y": arm_state["tip_y"],
        "reach": arm_state["reach"], "time": arm_state["time"],
    }


# ---------------------------------------------------------------------------
# Flask app
# ---------------------------------------------------------------------------
app = Flask(__name__)
app.config["SECRET_KEY"] = "armworld-secret-key"
socketio = SocketIO(app, cors_allowed_origins="*", async_mode="threading")


@app.route("/")
def index():
    return send_from_directory(os.path.dirname(os.path.abspath(__file__)),
                               "armworld.html")


@app.route("/api/status")
def api_status():
    return jsonify(_snapshot())


@app.route("/action/<cmd>")
def action(cmd):
    tau = 0.7 * ARM_MAX_TORQUE
    torques = {
        "hold": (0.0, 0.0), "inc1": (tau, 0.0), "dec1": (-tau, 0.0),
        "inc2": (0.0, tau), "dec2": (0.0, -tau),
    }
    if cmd not in torques:
        return jsonify({"error": "unknown cmd"}), 400
    arm_step(arm_state, *torques[cmd])
    socketio.emit("state", _snapshot())
    return jsonify({"status": "ok", "action": cmd})


@socketio.on("connect")
def on_connect():
    emit("state", _snapshot())


@socketio.on("torque")
def on_torque(data):
    try:
        t1 = float(data.get("tau1", 0.0))
        t2 = float(data.get("tau2", 0.0))
    except (TypeError, ValueError):
        t1 = t2 = 0.0
    arm_step(arm_state, t1, t2)
    emit("state", _snapshot())


if __name__ == "__main__":
    arm_step(arm_state, 0.0, 0.0)
    print("ArmWorld demo: http://<host-ip>:5000", flush=True)
    socketio.run(app, host="0.0.0.0", port=5000,
                 debug=False, use_reloader=False,
                 allow_unsafe_werkzeug=True)
