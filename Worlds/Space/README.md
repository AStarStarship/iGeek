# SpaceWorld

Status: **roadmap** — spacecraft physics and autopilot. Not implemented
or validated. No Gym subclass, no PPO loop. Directory contains physics
primitives (SpaceMath, SpacePhysics, SpaceAlloc, SpaceCaps) but no
working environment or test entry point.

Exception: the **pure physics** (`TSpaceStep`, `TSpaceInertia`, `TSpacePid`,
RCS layout, capabilities) now compiles and is exercised by the browser demo
below. `SpaceEnv` / the Gym layer is still not implemented.

## Browser demo (WebGL, port 5009)

6-DoF zero-g spacecraft: fire RCS force/torque burns, watch the body axes and
velocity vector in a raw WebGL 3D view, then run the native PID rotation-damping
autopilot. All motion comes from the surviving native code:
`space_bridge.py` compiles `SpaceMath.hxx`, `SpacePhysics.hxx`, `SpaceRcs.hxx`,
`SpaceCaps.hxx` and `SpacePid.h` verbatim into a shared library and calls
`TSpaceStep` / `TSpacePid.Update` through ctypes.

Build shims (bridge-only; shipped headers untouched): the ASCIICrabs config
header unconditionally `#undef`s `_Seam`, so the world seam is reasserted;
`SpaceMath.hxx` calls `_::SquareRoot`/`IsFinite`/`Absolute` and `sin`/`cos`/`acos`
without qualifying them, so the bridge injects using-declarations; and the
`../../CrabsTK/Math/Blas.h` include only resolves with `-I Worlds`.

**Not compiled:** `SpaceEnv`/`Environment`. The seeded initial tumble mirrors
`SpaceEnv::Reset`'s documented formula in Python, and stabilization mirrors
`SpaceWorldMain.hxx` `cancel_rotation` using the native PID and native step. No
Gym reward loop is claimed.

Files: `space_bridge.py` (native), `space_server.py` (Flask), `space.html`
(WebGL 3D), `test_space.py` (24 retained tests).

```
cd iGeek
python3 -m venv .venv-space && . .venv-space/bin/activate
pip install flask pytest
python -m pytest Worlds/Space/test_space.py -v
python Worlds/Space/space_server.py        # http://127.0.0.1:5009/
```

Controls: torque and force buttons fire a 0.5/1/2 s burn; `Stabilize 2 s`/`10 s`
(or `Space`) runs the native PID damping; `R` reset, `N` new seeded tumble.

See AGENT_PLAN.md: "ArmWorld and the spacecraft autopilot are roadmap
work, not implemented or validated capabilities claimed by this plan."
