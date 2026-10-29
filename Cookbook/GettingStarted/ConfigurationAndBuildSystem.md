# [iGeek Cookbook](../)

## [Getting Started](./)

### Configuration and Build System

iGeek is a **no-stdlib C++23** engine. Every build is CMake + GCC (or Clang)
with `-std=c++2b`; there are no external dependencies for the core or the
worlds listed below (MKL/OpenBLAS is optional for the math helpers in
`CrabsTK/Math`).

**Repository layout.** iGeek expects two sibling repos at the same level:

```
AStarStarship/
  iGeek/          <- this repo (engine core + worlds)
  ASCIICrabs/     <- Crabs core: no-stdlib ASCII types, containers, I/O
  CrabsTK/        <- Crabs toolkit: reusable modules
```

The engine core (root `CMakeLists.txt`) builds `igeeK_core`, a single
translation unit that pulls in the whole RL/agent layer and runs its
self-test. Each world with a `CMakeLists.txt` builds standalone against the
root and its siblings:

```Bash
# core self-test
cmake -S . -B build_core -DCMAKE_BUILD_TYPE=Release
cmake --build build_core -j

# a world (any of: BikeWorld, CarWorld, CardsWorld, Models/HRM, SudokuWorld)
cmake -S BikeWorld -B BikeWorld/build -DCMAKE_BUILD_TYPE=Release
cmake --build BikeWorld/build -j
```

**Sanitizer builds.** The root core takes a sanitizer option (Address +
Undefined + float-cast-overflow, with recovery disabled):

```Bash
cmake -S . -B build_core_san -DCMAKE_BUILD_TYPE=Debug -DIGEEK_SANITIZE=ON
cmake --build build_core_san -j
ASAN_OPTIONS=detect_leaks=1 ./build_core_san/igeeK_core
```

**The seam system.** The engine uses the ASCIICrabs seam build convention:
each build tree picks a `SEAM` / `SEAM_N` macro, and headers gate their
bodies behind `#if SEAM >= ...`. A world's normal build compiles the seam
test units (its `_Seams/` tree); the seam binary's exit code is the unit-test
result. There is no CTest, GoogleTest, or Catch2 anywhere in the tree.

**Requirements.** GCC 13+ (C++23), CMake 3.16+, a POSIX shell. Linux is the
primary target; the engine is headless and console-only.

**[<< Previous Section: Command Line Usage](./CommandLineUsage.md) | [Next Section: Mission and Vision >>](./MissionAndVision.md)**

## License

Copyright [AStarship™](https://astarship.net).
