# SudokuWorld

Status: **experimental** — 9x9 Sudoku puzzle solving as an RL environment.
Gym subclass (ASudokuGym) with manifest. 81-dim observation, 810 actions
(cell*10 + digit), 729-step episodes. Not validated as a learned policy;
no PPO training run demonstrated.

Build: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j 4`
