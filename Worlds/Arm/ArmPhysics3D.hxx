// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekArmWorldPhysics3DHXX
#define _iGeekArmWorldPhysics3DHXX
#include "ArmPhysics.h"
#include "../../../CrabsTK/Math/Blas.h"
#include "../../CrabsTK/Math/Blas.h"
#if _Seam >= _iGeekArmWorldCore
namespace iGeek {
using namespace ::_;
using namespace ::CTK;

/* 3D two-link arm with spherical workspace, extending the 2D core.
   The physics uses row-major 3x3 inertia matrix via Math::GemmRR and
   3-vector forces via Math::GemvR.
   Joint angles: j1 (base rotation), j2 (elbow), j3 (wrist pitch).
   Tip position is spherical: (x,y,z) from forward kinematics. */
struct ArmParams3D {
  FPD length_1 = 0.20, length_2 = 0.20, length_3 = 0.15;
  FPD mass_1 = 2.0, mass_2 = 1.0, mass_3 = 0.5;
  FPD inertia_1 = 0.05, inertia_2 = 0.0167, inertia_3 = 0.004;
  FPD max_torque = 3.0;
  FPD damping = 0.01;
  FPD dt = 0.02;
  FPD half_step = 0.01;
};
/* Arm state: 3 absolute joint angles + 3 angular velocities + time + spherical tip. */
struct alignas(ACPUCacheLineSize) TArmState3D {
  FPD j1 = 0, j2 = 0, j3 = 0;     // Absolute joint angles, rad
  FPD w1 = 0, w2 = 0, w3 = 0;     // Angular velocities, rad/s
  FPD time = 0;
  FPD tip_x = 0, tip_y = 0, tip_z = 0;  // End-effector position, m (spherical)
  FPD reach = 0;             // SquareRoot(tip_x^2 + tip_y^2 + tip_z^2), m
};
/* Per-frame actuator command: 3 joint torques, N m. Clamped in TArmStep3D. */
struct TArmControl3D {
  FPD tau_1 = 0, tau_2 = 0, tau_3 = 0;
};
/* 3x3 inertia matrix row-major layout. */
enum { ArmMatSize = 3, ArmVecSize = 3 };
/* Closed-form three-link forward kinematics from absolute joint angles.
   tip_x = l1*cos(j1) + l2*cos(j1+j2) + l3*cos(j1+j2+j3)*cos(j3)
   tip_y = l1*sin(j1) + l2*sin(j1+j2) + l3*sin(j1+j2+j3)*cos(j3)
   tip_z = l3*sin(j3)  (pure pitch at wrist for simplicity) */
void TArmFK3D(ArmParams3D& params, FPD j1, FPD j2, FPD j3,
          FPD& tip_x, FPD& tip_y, FPD& tip_z);
/* Compute inertia (I11,I12,I13, I21,I22,I23, I31,I32,I33) and gravity terms.
   @return false for invalid inputs; forces are untouched. */
BOL TArmForces3D(ArmParams3D& params, TArmState3D& state,
                 TArmDynamicsTerms3D& forces);
/* One 0.02 s frame = four 0.01 s RK4 substeps of the torque model.
   Joint angles are clamped to [0, pi/2]; velocity zeros at the bound.
   @return false for invalid inputs; state is untouched. */
BOL TArmStep3D(TArmState3D& state, ArmParams3D& params,
               TArmControl3D& control);
/* True when all fields are finite and angles are within [0, pi/2]. */
BOL TArmStateValid3D(TArmState3D& state);
/* True when params are positive and finite. */
BOL ArmParamsValid3D(ArmParams3D& params);
/* 3x3 inertia matrix, used in dynamics solve. */
struct TArmDynamicsTerms3D {
  FPD I[3*3];  // row-major: I11 I12 I13 I21 I22 I23 I31 I32 I33
};
/* Gravity vector for 3-link arm. */
struct TArmGravity3D {
  FPD grav_1 = 0, grav_2 = 0, grav_3 = 0;
};
/* Solve I * acc = tau + C*w + g for acceleration using 3x3 GemmRR + GemvR. */
BOL TArmDynamics3D(ArmParams3D& params, FPD j1, FPD j2, FPD j3, FPD w1, FPD w2,
                   FPD w3, FPD tau_1, FPD tau_2, FPD tau_3, FPD* acc);
/* Hash for 3D target drawing. */
static IUD TArmHash3D(IUD x) {
  x ^= x >> 12; x = x * 0x9E3779B97F4A7C15ull; x ^= x >> 25;
  x = x * 0xC2B2AE3D27D4EB4Full; x ^= x >> 27;
  return x;
}
/* Status name for 3D. */
const CHA* TArmStatusName3D(ISC status) {
  const CHA* names[] = {"running", "reach", "timeout", "invalid"};
  return status >= ArmRunning3D && status <= ArmInvalid3D ? names[status] : "?";
}
} //< namespace iGeek
#endif
#endif