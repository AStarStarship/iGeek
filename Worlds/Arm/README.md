# ArmWorld

Status: **experimental** — 2-link planar arm reacher with joint torque
control. Gym subclass (AArmGym) with manifest. Not validated as a
learned policy; no PPO training run demonstrated. 8 categorical actions
(4 per joint), 6-dim observation, 2000-step episodes.

Build: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j 4`
