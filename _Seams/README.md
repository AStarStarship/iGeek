# iGeek _Seams

The core framework self-test for [iGeek](../). A single translation unit
(`_Main.cpp`) pulls in the whole core RL/agent layer through the `.hxx` impl
chain and runs the checks in `core_test.cpp`:

- `TTensor` linear kernels (matmul, softmax, ...) against hand-computed values
- `TLinearPolicy` forward determinism + stochastic action sampling
- `TPPO` over a toy vectorized `Gym` (finite stats, no crash)
- the three generic agent tiers (`TTableAgent`, `TReflexAgent`, `TModelAgent`)
- `LinearProbe` end-to-end (capture -> records -> train -> accuracy)

Build and run from the iGeek root:

```sh
cmake -S . -B build_core -DCMAKE_BUILD_TYPE=Release
cmake --build build_core -j
./build_core/igeeK_core        # exit 0 = pass
```

Sanitizer build (ASan/UBSan/float-cast, leak detection on by default):

```sh
cmake -S . -B build_core_san -DCMAKE_BUILD_TYPE=Debug -DIGEEK_SANITIZE=ON
cmake --build build_core_san -j
ASAN_OPTIONS=detect_leaks=1 ./build_core_san/igeeK_core
```

Files:

- `_Config.h` — seam config (`IGEEK_CORE` / `IGEEK_RELEASE`).
- `_Main.cpp` — single-TU entrypoint; includes the core impl chain.
- `core_test.cpp` — the checks (defines `::_::Core`).
- `00.Core.hxx` / `22.Release.hxx` — seam-gated units (kept for the
  seam-tree convention; the plain `main` in `_Main.cpp` is the live path).

## License

Copyright [AStarship™](https://astarship.net).
