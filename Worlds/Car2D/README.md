# iGeek Autopilot

Status: **tested headless** — Release + sanitizer builds pass CTest;
classical car pilot and hybrid speed-planning/pure-pursuit pilot each
reached 900/900 goals across nine fixed layouts and 100 initial-pose
seeds. Layout generalization and learned driving NOT established.

An autopilot mod for the [Interactive Gym Environment and Education Kit](https://github.com/AStarStarship/iGeek).

## License

Copyright [AStarship™](https://astarship.net).

## Browser demo (WebGL, port 5005)

Interactive top-down driving demo backed by the real native `ACarEnv` four-tire
RK4 plant via a C ABI (`car_bridge.cpp` + `car_demo.py` ctypes adapter).

- **Drive**: drag the on-screen steering wheel, or W/↑ (forward) and D/↓
  (reverse gear) to accelerate. ←/→ steer via keyboard.
- **Reverse gear**: the native plant now supports reverse (`TCarControl.reverse`);
  forward behavior is byte-identical when `reverse = 0`.
- **Race track**: a new `track` scenario (rounded-rectangle circuit on grass with
  border walls) with a JS lap timer — free drive, no hard win condition.

Run:

```
python3 Worlds/Car2D/car_server.py     # http://<host>:5005/
python3 -m unittest test_car_demo test_car_server   # from Worlds/Car2D/
```

