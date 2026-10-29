# iGeekMazeWorld

Status: **simulation-only** — single-TU ASCII maze solver, headless
builds on Linux g++. Not a Gym subclass; no PPO loop. A* solver is
correct (unique shortest path in a perfect maze) but has no learned
component.

ASCII terminal maze-solver environment. A seeded recursive-backtracker generates
a perfect (tree) maze; an A* solver finds the unique shortest path from start
`S` to exit `E`. Headless (no Windows console I/O, no `GetAsyncKeyState`) so it
builds on Linux g++.

## Layout (single-TU, one .cpp)

- `MazeWorld.h` / `MazeWorld.hxx` — `MazeWorld` struct: seeded LCG maze
  generation + A* + `COut` terminal render. No-stdlib (no std containers, no
  exceptions, no heap — all fixed-size arrays).
- `_Config.h` — defines the local `IGEEK_MAZEWORLD` seam ID (24) and sets
  `SEAM`, mirroring the SpaceWorld local-seam pattern.
- `_Seams/_Main.cpp` — the ONLY .cpp file. Includes `_Seams/_Main.hxx` (the
  entry header) for rapid single-TU compilation.
- `_Seams/_Main.hxx` — generates a seeded maze, solves it with A*, prints the
  grid + path length. I/O is raw `write()` (no `<cstdio>`).
- `CMakeLists.txt` — builds `mazeworld` from `_Seams/_Main.cpp`, registers
  `mazeworld_probe` in CTest.

## Build

```
cd iGeek/MazeWorld
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

Expect `mazeworld_probe` to pass (exit 0, `MAZEWORLD_OK path_len=N`).

## Verification

The probe is deterministic: seed 42, 31x21 grid, A* from (1,1) to (29,19).
Pass = a path is found and printed. This is a maze-generation + A*
correctness check, not an RL learning claim.

## Provenance

Converted from the legacy `igeek.autopilot` Script2/MSVC project
(2014-2019, Cale McCollough, MPL-2.0, UTF-16-LE, Windows) into the new
iGeek/CrabsTK single-TU format. Legacy sources (UTF-16, `GetAsyncKeyState`
console loop, `.vcxproj`) are retired. The maze-generation + A* logic is new
AStarship-licensed code.

## License

Copyright [AStarship™](https://astarship.net). Legacy provenance is the
Captain's — do not relicense or strip legacy headers without instruction.
