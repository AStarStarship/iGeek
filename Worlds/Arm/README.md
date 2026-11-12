# ArmWorld

Status: **experimental** — 2-link planar arm reacher with joint torque
control. Gym subclass (AArmGym) with manifest. Not validated as a
learned policy; no PPO training run demonstrated. 8 categorical actions
(4 per joint), 6-dim observation, 2000-step episodes.

Build from the iGeek root:

    cmake -S _Seams -B build/arm -DCMAKE_BUILD_TYPE=Release
    cmake --build build/arm --target ArmPhysicsChecks ArmHeadless -j 4
    ctest --test-dir build/arm --output-on-failure

## Planar dynamics contract

Angles are absolute link orientations from +x; +y is up. Each link's
center of mass is at half its length. `inertia_1` and `inertia_2` are
independent COM inertias, not automatically recomputed uniform-rod inertias.
For c1=l1/2, c2=l2/2 and d=j1-j2:

    M11 = I1 + m1*c1*c1 + m2*l1*l1
    M22 = I2 + m2*c2*c2
    M12 = m2*l1*c2*cos(d)
    b = m2*l1*c2*sin(d)
    M*qdd = [tau1 - b*w2*w2 - damping*w1 - (m1*c1+m2*l1)*g*cos(j1),
             tau2 + b*w1*w1 - damping*w2 - m2*c2*g*cos(j2)]

The existing torque API supplies generalized torques conjugate to absolute
angles. Physical shoulder/elbow motor torques (u1,u2) map to (u1-u2,u2);
independent generalized-torque limits are not equivalent to motor limits.

Each frame integrates exactly `dt`, using `ceil(dt/half_step)` equal RK4
substeps; counts above 4096 are rejected. Failed evaluations leave the input
state unchanged. Joint stops remain a simplified end-of-frame angle clamp
with outward velocity cancellation, not a coupled impact/contact solver.
Energy conservation is tested in free flight, not across those clamps.

Reset computes tip position from the joint angles. Seeded targets are sampled
in joint space and mapped through FK; this guarantees kinematic reachability
for the configured lengths, not controllability under the torque cap. The
corrected target distribution and physics change trajectories for old seeds.

`ArmPhysicsChecks` tests analytic inertia/gravity, independent COM-energy
conservation, gravity compensation, timestep consistency, atomic failure,
reset kinematics, target IK, substep accounting and torque saturation.
`ArmHeadless` separately exercises the existing demo and Room-backed rendering.
The 3D files remain incomplete scaffolding; these checks validate only 2D.
