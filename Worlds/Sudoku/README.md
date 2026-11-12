# SudokuWorld

Status: **experimental** — 9x9 Sudoku puzzle solving as an RL environment.
Gym subclass (ASudokuGym) with manifest. 81-dim observation, 810 actions
(cell*10 + digit), 729-step episodes. Not validated as a learned policy;
no PPO training run demonstrated.

## WebGL browser demo

From the iGeek root:

    python3 Worlds/Sudoku/sudoku_server.py

Open http://localhost:5004 or the server's LAN address. The Flask development
server binds 0.0.0.0; use a trusted LAN, not public Internet hosting.
Requires Flask and g++. Per-browser games live in server memory.

The board backgrounds, grid and highlights render through WebGL; accessible
HTML cells overlay digits and candidates. Select a cell, type 1–9, use arrow
keys to navigate, and Backspace/Delete to erase. Number-pad buttons support
mouse/touch. Includes legal candidates, hints, native search solve, reset,
and seeded puzzle variants. Generated puzzles permute the classic puzzle's
digits/rows/bands/columns/stacks, preserving its unique solution rather than
claiming independently generated difficulty levels.

The server compiles the verbatim pure-board section of SudokuWorld.hxx into a
C ABI shared library using sudoku_bridge.cpp, then calls it through ctypes.
It is not a Python rewrite of the solver. Full-header/Gym integration remains
blocked by upstream dependencies; the demo isolates board functions rather
than compiling ASudokuGym. Build products go to SUDOKU_SCRATCH or TMPDIR
(fallback: ~/.cache/sudoku), not the repository.

Human-mode editing allows erase/replacement and has no training rewards.
Hints and Solve use native backtracking, not a learned AI policy.
GET /api/manifest documents these differences and the browser API.

Run retained backend tests (including native compilation and uniqueness):

    python3 -m unittest discover -s Worlds/Sudoku -p test_sudoku.py -v

Existing native build note (not verified by this browser-demo work):
`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j 4`
