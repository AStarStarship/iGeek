// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_ARMWORLD_PHYSICS_H
#define CRABS_ARMWORLD_PHYSICS_H
#include "ArmConfig.h"
#if SEAM >= CRABS_ARMWORLD
namespace iGeek {
/** Two-link planar arm, SI units, FP64. Joint angles are absolute (rad),
 * clamped to [0, pi/2] at the physics layer so the policy cannot escape the
 * declared workspace. Torques are clamped to |tau| <= max_torque.
 */
struct TArmParams
{
  FPD length_1 = 0.20, length_2 = 0.20;      //< Link lengths, m (0.5 m reach).
  FPD mass_1 = 2.0, mass_2 = 1.0;            //< Link point masses, kg.
  FPD inertia_1 = 0.05, inertia_2 = 0.0167;  //< I = m*l^2/3, kg m^2.
  FPD max_torque = 3.0;                      //< |tau| cap, N m.
  FPD damping = 0.01;                        //< Viscous joint damping, N m s.
  FPD dt = 0.02;                             //< Control frame, s.
  FPD half_step = 0.01;                      //< RK4 substep, s.
};
/** Arm state: absolute joint angles + angular velocities, time, reach. */
struct alignas(ACPUCacheLineSize) TArmState
{
  FPD j1 = 0, j2 = 0;        //< Absolute joint angles, rad.
  FPD w1 = 0, w2 = 0;        //< Angular velocities, rad/s.
  FPD time = 0;              //< Simulated time, s.
  FPD tip_x = 0, tip_y = 0;  //< End-effector position, m (forward kinematics).
  FPD reach = 0;             //< |tip|, m.
};
/** Per-frame actuator command: joint torques, N m. Clamped in TArmStep. */
struct TArmControl
{
  FPD tau_1 = 0, tau_2 = 0;
};
/** Arm dynamics: inertia terms + gravity of the 2-link model. */
struct TArmDynamicsTerms
{
  FPD I11 = 0, I12 = 0, I12_base = 0, I22 = 0;
  FPD c12 = 0;
  FPD grav_1 = 0, grav_2 = 0;
};
/** Closed-form two-link forward kinematics from absolute joint angles. */
void TArmFK(const TArmParams& params, FPD j1, FPD j2,
           FPD& tip_x, FPD& tip_y);
/** Inertia (I11, I12, I22), Coriolis c12 and gravity terms of the model.
 * @return false for invalid inputs; forces are untouched.
 */
BOL TArmForces(const TArmParams& params, const TArmState& state,
              TArmDynamicsTerms& forces);
/** One 0.02 s frame = four 0.01 s RK4 substeps of the torque model.
 * Joint angles are clamped to [0, pi/2]; velocity zeros at the bound.
 * @return false for invalid inputs; state is untouched.
 */
BOL TArmStep(TArmState& state, const TArmParams& params,
            const TArmControl& control);
/** True when all fields are finite and angles are within [0, pi/2]. */
BOL TArmStateValid(const TArmState& state);
/** True when params are positive and finite. */
BOL TArmParamsValid(const TArmParams& params);
} //< namespace iGeek
#endif
#endif
