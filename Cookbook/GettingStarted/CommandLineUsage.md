# [iGeek Cookbook](../)

## [Getting Started](./)

### Command Line Usage

All iGeek binaries are headless: they run to completion, print evidence to
stdout, and return a process exit code (0 = pass). There is no interactive
UI in the engine core.

| Binary | Built from | What it does |
|---|---|---|
| `igeeK_core` | repo root (`cmake -S .`) | Core framework self-test: TTensor kernels vs hand values, TLinearPolicy determinism + sampling, TPPO over a toy Gym, the 3 agent tiers, LinearProbe end-to-end. |
| `bikeworld` | `BikeWorld/` | 2D bicycle plant: classical reference driver drives 8 goals; prints goal count, frame count, simulated seconds, and wall seconds. |
| `carworld` | `CarWorld/` | 2D car plant: classical driver drives 9 goals with the 4-tire friction model; same evidence format. |
| `iGeekCardsWorld` / `blackjack_demo` / `blackjack_demo_tests` | `CardsWorld/` | Blackjack Gym. `iGeekCardsWorld` runs the seam test suite (card logic, Gym tensor ops, PolicyNet forward + gradcheck, PPO training to a learned rule) and exits 0 on pass; `blackjack_demo` is the demo run. |
| `HRM` | `Models/HRM/` | Hierarchical Reasoning Model — Sudoku solver research (H/L level recurrent processing; prints puzzle + outcome). |
| `SudokuWorld` | `SudokuWorld/` | Standalone backtracking Sudoku demo (no RL). |

Sanitizer builds are selected at configure time (see the next section); the
binary name is unchanged. Run a sanitized build under a leak check:

```Bash
ASAN_OPTIONS=detect_leaks=1 ./build_core_san/igeeK_core
```

Exit codes are the CI signal: a world's seam binary exiting 0 means its unit
tests passed; exiting non-zero (or the process being killed, e.g. timeout)
means it did not.

**[<< Previous Section: Quickstart Guide](./QuickstartGuide.md) | [Next Section: Configuration and Build System >>](./ConfigurationAndBuildSystem.md)**

## License

Copyright [AStarship™](https://astarship.net).
