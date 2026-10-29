# iGeek Engineering Plan

Updated: 2026-10-20.
Scope: public-safe native simulation, control, measurement and integration.
This consolidated plan replaces conflicting historical handoffs. A completed
smoke test is not acceptance of physics, a learning algorithm or an autopilot.

## 1. Mission and repository boundaries

iGeek is a headless-first C++ environment engine for robotics, vehicle control,
games and scientific computing. Build reproducible Worlds with explicit
physics, contiguous observations/actions and measurable agent behavior.

- iGeek owns environments, simulators, classical reference controllers,
  model-neutral Gym/Room interfaces, deterministic replay and metrics.
- ASCIICrabs owns abstract typed storage, schemas and Room/Slot primitives.
- CrabsTK owns reusable numerical kernels and concrete OS/device adapters.
- External policies connect through documented observation/action interfaces.
  Keep private model implementations, unpublished research, internal machine
  paths, confidential fixtures and private project handoffs OUT of this tree.
- Quaternion rotation math is ordinary shared robotics infrastructure. Keep
  it available for car, arm and spacecraft attitude/control work; do not
  remove useful public mathematics because another project also uses it.
- Publishing a license or source code is not a confidentiality mechanism.
  Review release contents before publication. LICENSE.md governs this tree;
  dependency licenses and third-party notices require separate review.

## 2. Coordination and change discipline

- The coordinator is the sole editor of this plan. Human-supervised terminal
  workers receive bounded tasks and explicit path allowlists; editor/Copilot
  assistance uses the same one-owner-per-file rule.
- First task currently scoped to the platform worker: CrabsTK IPC duplex
  payload isolation and its regression, ONLY in that module. This document
  does not dispatch a worker or authorize additional shared-file edits.
- Pause an existing owner before handing its files to another editor. Report
  upstream defects rather than silently expanding into a sibling repository.
- Preserve dirty/untracked work. No commits, pushes, resets, stashes, branch
  changes, blanket reverts or deletion without explicit human authorization.
- Read root/nested AGENTS.md and actual API definitions before implementation.
  Check live Git state; old branch names and completion messages are not proof.
- If a board record is explicitly requested, always target board `astarship`.
  Keep task bodies public-safe; a card is not evidence that work executed.

## 3. Current evidence and limits

The following are scoped results from the latest engineering review, not a
new test run performed for this documentation update:

- CarWorld: fresh Release and address/undefined/float-cast sanitizer builds
  passed the six configured CTest cases.
- Classical car pilot and hybrid speed-planning/pure-pursuit pilot each
  reached 900/900 goals across nine fixed layouts and 100 initial-pose seeds.
  Layout generalization and learned driving were NOT established.
- Native car Room binding passed manual/autopilot takeover route tests.
  This does not establish a complete interpreter or process transport.
- ASCIICrabs configured Room seam and focused Room binding tests passed;
  this is not every seam, integer width, platform or template instantiation.
- CrabsTK Math scalar/MKL focused checks passed. Optional CUDA host-ASan
  acceptance remains unresolved; CPU success is not GPU acceptance.
- Root trainer/interface smoke tests execute, but algorithm correctness and
  learned improvement remain OPEN. Existing test counts miss confirmed bugs.
- BikeWorld has a standalone headless build/test entry point. Other existing
  Worlds need individually verified status; directory presence is not support.
- ArmWorld and the spacecraft autopilot are roadmap work, not implemented or
  validated capabilities claimed by this plan. No new World is launched here.

## 4. Acceptance blockers and bounded repair order

### P0.1 Transport and storage integrity

Owner boundary: CrabsTK IPC; ASCIICrabs Slot repairs need a separate handoff.
Do not integrate process transport into a World until these contracts pass.

- Request and reply payload regions must be disjoint, not merely their counters.
  Regression: publish different outstanding bytes in BOTH directions before
  either endpoint reads; verify byte-exact payloads through repeated wrap.
- Enforce checked size/length arithmetic before narrowing or allocation. Reject
  malformed headers, invalid ring capacities and impossible counter distances.
  Test word-width boundaries, rounding overflow and bounded failure behavior.
- Prove publication ordering and wait-predicate correctness. Force a producer
  publication between predicate check and wait setup; data must not be stranded
  behind a timeout or unbounded sleep. Test separate processes, not only forked
  sequential request/reply exchanges.
- Define creator/attacher ownership, readiness, move/close behavior, peer closure
  and resource cleanup. Test both shutdown orders and partial startup.
- Slot metadata and payload must not overlap, and every pointer/span must stay
  inside caller storage. Add canaries and sanitizer-backed boundary tests.
- Native structs are not wire formats: explicitly encode version, types,
  lengths, byte order and bounds; never deserialize executable pointers.

### P0.2 Training algorithm and evaluation correctness

Owner boundary: root PPO/policy modules and the affected World's evaluator.
Do not present finite statistics or changing weights as algorithm validation.

- Recompute updated policy outputs for each optimization epoch; implement and
  test the advertised probability ratios, clipping and objective coefficients.
- Apply the CURRENT transition's terminal mask. Bootstrap nonterminal rollout
  ends correctly; do not mix terminal state values with auto-reset observations.
- Preserve unnormalized critic return targets when normalizing advantages.
  Check policy/value gradients against an independently expressed reference.
- Do not reset every rollout to the same opening segment when claiming full
  episodes were learned. Record actual visited states, episode ends and horizon.
- Evaluate each environment's first terminal independently. Auto-reset flags
  last one step; never wait for all environments to terminate simultaneously.
- Compare untrained and constant-action baselines on the SAME distinct seeds.
  Separate repeated deterministic episodes from independent trials. Save actual
  validation-selected weights; do not label a scalar score a checkpoint.
- Keep raw-actuator learning separate from speed selection atop a classical
  driver. Require measured improvement for a learned-competence claim.

### P0.3 Car controller and estimator regressions

Owner boundary: CarWorld only, after an explicit single-owner handoff.

- Every controller mode must honor stop requests and declared speed limits.
  Insert an obstacle after planning: a controlled stop is required, not a
  collision hidden by a successful nominal route benchmark.
- Restore material/friction awareness and obstacle preview in hybrid modes.
  Reject unknown CLI mode values rather than silently selecting a fallback.
- Correct the EKF state-transition Jacobian. Test stationary/straight/turning
  covariance propagation and measurement updates against an independent model.
- Treat truth-fed estimator tests as interface checks. Declare sensor sampling,
  noise, latency and available state; verify estimation error independently.
- Make public headers compile with their declared includes; no accidental
  transitive dependency on another application's include order.

### P0.4 Experimental 3D car physics

Keep the current planar plant available while repairing the opt-in extension.
Do not describe integrated state-variable count as mechanical degrees of freedom.

- Restore physically justified spring/damper moments and dimensional consistency;
  remove arbitrary gains used only to make short tests produce expected signs.
- A perturbed parked car must settle under damping rather than diverge. Check
  long runs, gravity equilibrium, unequal axle loads and force/moment balance.
- Terrain heights must affect contact/suspension state. Test bumps, wheel lift,
  unilateral contact and load redistribution; unused inputs are not features.
- Correct and independently verify tire curves and force signs across the full
  declared slip range, including combined longitudinal/lateral saturation.
- Derivatives must be functions of the declared state/inputs, not acceleration
  left over from another RK stage. Verify timestep convergence.
- Assert every step succeeds; invalid input must be rejected AND leave the full
  state unchanged. Tests must not silently break loops or ignore failed steps.
- A tested physics header is not a 3D environment: CLI/Gym integration and its
  observations, collision behavior and episode metrics need separate gates.

## 5. Shared quaternion foundation for robotic and vehicle autopilots

Status: required generic capability; inventory/reuse existing math before
adding APIs. No new quaternion implementation is claimed by this plan.

- Freeze one convention: scalar-first (w,x,y,z), right-handed active rotations
  on column vectors, body-to-world orientation and body-frame angular velocity.
  Each World declares its actual world/body axes and units at the boundary.
- Support identity, conjugate/inverse, normalization with invalid-input checks,
  Hamilton composition, vector rotation and matrix conversion. Rotation products
  are order-dependent. q and -q represent the same attitude.
- Keep attitude integration separate from controllers and state estimators.
  Use a documented quaternion derivative or exponential-map update consistent
  with the declared frame; Euler angles are diagnostic outputs, not hidden state.
- Test known-axis rotations, composition order, inverse round trips, q/-q
  equivalence, normalization drift, near-half-turn errors and constant angular
  velocity against analytic/reference trajectories. Preserve sign continuity
  where interpolation or differentiation requires it.
- Test rotation matrices for orthogonality/determinant and compare FP64 outputs
  with an independent PyTorch reference. Validate shape, aliasing and buffer
  capacity; no new heap allocation in control/physics ticks.
- Reusable math belongs in CrabsTK/ASCIICrabs under an accepted API. World-specific
  dynamics, gains, sensors and actuator policies remain in the appropriate World.

## 6. ArmWorld — next robotic-arm environment

Status: planned; ArmWorld/ was absent at the latest inspection. Assign a
separate owner/path scope before implementation. Headless CPU first.

1. Two revolute joints with explicit SI link geometry, mass/inertia, gravity,
   damping, torque bounds and joint limits. Raw torque dynamics must work
   without a controller; no hidden stabilizer or teleporting inverse kinematics.
2. Forward kinematics and Jacobian versus independent FP64/autograd and finite
   differences; dynamics versus gravity-balance, energy and half-step tests.
3. Deterministic reach/hold tasks plus unreachable targets and limit violations.
   Label the classical reference controller; score final error, settling,
   overshoot, control effort and violations, not just crossing a target once.
4. Preallocated Gym buffers, deterministic seeded reset, first-terminal episode
   accounting and optional native Room command/reply binding.
5. Contact, self-collision, grasping, payload changes and additional joints are
   separate extensions. Reaching does not establish manipulation or safety.
6. Add 3D orientation tasks using the shared quaternion contract only after
   the simpler physical and numerical gates pass. Compare learned policies
   through external model-neutral buffers after trainer acceptance.

## 7. Spacecraft autopilot — headless roadmap

Status: planned; directory/API naming and implementation are not assigned.
Use standard quaternion attitude mechanics and documented actuator constraints.

1. Begin with rigid-body attitude only: orientation quaternion, angular velocity,
   positive-definite inertia and commanded body torque. Declare the inertial
   frame; conserve angular momentum/energy in torque-free reference cases.
2. Add a labeled classical attitude/rate controller with bounded actuators.
   Test q/-q target equivalence, near-half-turn maneuvers, saturation, settling,
   disturbance rejection, invalid measurements and a defined safe fallback.
3. Model thrusters or reaction wheels explicitly before claiming actuator
   realism: force/torque allocation, limits, delay and relevant resource budgets.
   Preserve raw plant stepping for independent controller comparisons.
4. Extend to translational plus rotational motion only after attitude acceptance.
   Declare gravity/environment assumptions; free-space maneuvering, orbital
   rendezvous, docking and atmospheric flight are different validation regimes.
5. Verify analytic cases, independent FP64 references, step convergence and
   deterministic replay. Then add declared sensor models/estimator tests.
6. Integrate versioned observation/action buffers and episode metrics. Hardware
   operation, flight qualification and safety certification are NOT implied by
   a simulation benchmark or a trained policy's fixture success.

## 8. Core API, other Worlds and release hygiene

After the blockers above, take one bounded task at a time; do not bulk-refactor.

- Document Gym buffer ownership, shape, mutability and lifetime; make unsupported
  methods fail explicitly rather than silently succeeding as no-op stubs.
- Keep declaration/implementation naming coherent. Audit duplicate include
  guards, stale module paths and single-TU assumptions before API extraction.
- Label each World's README as tested headless, simulation-only, experimental
  or legacy, with an actual build entry point and measured limitations.
- Reuse VehicleMap/VehicleRoute and TileWorld occupancy. Do not infer collision
  solely from material IDs or fork duplicate planners/storage types.
- Verify Room lifecycle, Floor/Wall ownership and interpreter paths separately
  from the accepted native binding subset. Keep operating-system work downstream.
- Preserve current Math backend behavior and explicit opt-in selection. CPU
  scalar/MKL and GPU device tests are separate; real linked execution is required.
- Review stale graphics dependencies, source-header attribution, textbook/third-
  party content, licenses/notices and documentation links before a release.
  Do not relocate/delete existing assets or change licenses without approval.
- Keep compiled binaries, generated traces, machine-local evidence and private
  archives out of release payloads. A file scrub is not a Git-history scrub.

## 9. Engineering and verification contract

- C++23; canonical Crabs scalar types; functional casts; namespace _;
  Chimera+ naming; two-space indentation; .h/.hpp/.hxx module conventions.
- Fixed-capacity/preallocated contiguous buffers; no VLAs, C++ standard
  containers, network calls, sleeps or heap allocation in simulation ticks.
  C math/I/O is intentional; existing runtime/I/O dependencies must be disclosed.
- Validate finite values and bounds before casts/indexing or state mutation.
  Rejected operations must not partially advance physics or corrupt output.
- Write a failing behavior test first, fix the smallest owned scope, then run
  Release, relevant sanitizers, deterministic replay and affected consumers.
- Use analytic checks and independent PyTorch/autograd references, not only
  matching two implementations that share the same mistake. Freeze tolerances
  before comparison and cover every parameter/input family affected.
- Keep interface execution, numerical correctness, physical fidelity, learned
  improvement and performance as distinct gates. Do not redefine a known failing
  scenario as passing just to improve a completion count.
- Measure with CLOCK_MONOTONIC and 64-bit counters. Report setup/IO scope,
  control frames, physics substeps, simulated time, wall time and batch size.
  CPU timings do not establish GPU rates or a matched-framework speedup.
- For comparisons use identical equations, precision, fixtures, masks, seeds,
  thread budgets and outputs. Separate kernel, allocation, transfer and full-loop
  timing; synchronize device work explicitly where applicable.
- Capture exact commands, exits, source/config hashes and machine-readable
  metrics. Count/deduplicate episodes programmatically. Publish only sanitized,
  task-relevant evidence; preserve failures and state coverage limits plainly.

## 10. Build entry points and handoff format

From the iGeek root, use fresh build directories outside the source tree.
Do not reuse a stale binary as evidence for changed sources.

    cmake -S . -B "$TMPDIR/iGeekCoreRelease" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$TMPDIR/iGeekCoreRelease" -j4
    ctest --test-dir "$TMPDIR/iGeekCoreRelease" --output-on-failure

    cmake -S CarWorld -B "$TMPDIR/CarWorldRelease" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$TMPDIR/CarWorldRelease" -j4
    ctest --test-dir "$TMPDIR/CarWorldRelease" --output-on-failure

    cmake -S BikeWorld -B "$TMPDIR/BikeWorldRelease" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$TMPDIR/BikeWorldRelease" -j4
    ctest --test-dir "$TMPDIR/BikeWorldRelease" --output-on-failure

For separate Debug sanitizer builds, use IGEEK_SANITIZE, CARWORLD_SANITIZE or
BIKEWORLD_SANITIZE respectively. Do not conflate a single subdirectory's
CTest result with the whole repository or all dependencies.

Each worker handoff must state:

- Task, owner, allowed paths and read-only dependencies.
- Baseline/reproducer, failing expectation and implemented change.
- Exact commands, exit codes, source/config identity and evidence location.
- Counts and numerical tolerances; every unresolved failure and unsupported case.
- Proposed next bounded task, without changing this plan or claiming acceptance.

Next checkpoint: accept one transport regression repair, assign remaining P0
repairs individually, and freeze generic quaternion conventions before starting
new arm or spacecraft implementation. No worker is started by this document.
