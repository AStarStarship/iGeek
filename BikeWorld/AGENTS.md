# BikeWorld agent notes

Follow ../AGENTS.md and the AStarship Chimera+ style. Use Crabs scalar types,
functional casts, namespace _, two-space indentation, and .h/.hxx modules.
Do not introduce C++ std containers, external physics engines, network access,
or LLM inference into the headless step path. C math/I/O are intentional.

Build from iGeek:

    cmake -S BikeWorld -B BikeWorld/build -DCMAKE_BUILD_TYPE=Release
    cmake --build BikeWorld/build -j 4
    ctest --test-dir BikeWorld/build --output-on-failure

ASan/UBSan/float-cast checks:

    cmake -S BikeWorld -B BikeWorld/build/sanitized -DCMAKE_BUILD_TYPE=Debug -DBIKEWORLD_SANITIZE=ON
    cmake --build BikeWorld/build/sanitized -j 4
    ASAN_OPTIONS=detect_leaks=1 ctest --test-dir BikeWorld/build/sanitized --output-on-failure

Keep planning/terrain/contact in VehicleRoute and VehicleMap; TileWorld owns
occupancy. Do not duplicate its map or infer collision from material IDs.
Map/route must outlive ABikeEnv/ABikeGym. Fixed-capacity planner workspaces go
off the hot stack. Never use VLAs. No heap allocation in a control/physics tick.

Physics is FP64 SI units, four 0.005 s RK4 steps per 0.02 s control frame.
Keep raw StepControl separate from the classical pilot. Foot support is an
explicit low-speed approximation, not hidden high-speed stabilization.
Report falls/crashes as failures. Goal means an upright supported stop, not
merely passing a waypoint. Validate bad inputs without advancing physics.

PPO smoke proves API/numerical compatibility, not learned bicycle balance.
The current root PPO loop resets at each rollout; note this before claiming
long-horizon learning. Hardware/GPU inference is not part of this baseline.

For stress evidence run --scenario all --episodes 100 --seed 7 --csv with a
fresh filename in build/. Existing files are intentionally never overwritten.
Aggregate/count the CSV programmatically. Do not commit generated artifacts.
Read README.md for the measured model limits; do not call it hardware-ready.

All kanban operations explicitly target board astarship. Preserve concurrent
unrelated work; do not commit/push or switch branches unless asked.
