// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_PHYSICS_H
#define CRABS_BIKEWORLD_PHYSICS_H
#include "_Config.h"
#if SEAM >= CRABS_BIKEWORLD
namespace iGeek {
/** Local contact material. Slopes are dz/dx and dz/dy, not angles. */
struct TVehicleSurface
{
  FPD friction = 0.85, rolling = 0.006;
  FPD slope_x = 0, slope_y = 0;
};
/** SI units. Rider+bike are one rigid body; static axle loads in this model. */
struct TBikeParams
{
  FPD mass = 95, front_arm = 0.60, rear_arm = 0.50;
  FPD cg_height = 0.90, yaw_inertia = 28, roll_inertia = 76.95;
  FPD front_stiffness = 2200, rear_stiffness = 2640;
  FPD drag_area = 0.50, air_density = 1.225;
  FPD drive_force = 160, drive_power = 350, brake_force = 650;
  FPD steer_limit = 0.55, steer_rate = 1.8, steer_tau = 0.06;
  FPD roll_damping = 0.5, fall_angle = 0.70;
};
/** Steering is an angle target; pedals and brake are [0,1]. */
struct TBikeControl
{
  FPD steer = 0, throttle = 0, brake = 0;
  BOL foot_down = false;  //< Explicit rider support, only below 0.45 m/s.
};
/** Continuous body-frame velocities and world-frame pose, FP64. */
struct alignas(ACPUCacheLineSize) TBikeState
{
  FPD x = 0, y = 0, heading = 0, forward = 0, lateral = 0;
  FPD yaw_rate = 0, roll = 0, roll_rate = 0, steer = 0;
  FPD time = 0, distance = 0, lateral_accel = 0, utilization = 0;
  BOL fallen = false, supported = false;
};
/** RK4 of single-track tire-force + inverted-pendulum roll dynamics.
 * @return false for invalid input; state is untouched on invalid input.
 * @pre dt is finite and in (0,0.01]; caller handles collisions separately.
 */
BOL TBikeStep(TBikeState& bike, const TBikeParams& params,
              const TBikeControl& control, const TVehicleSurface& front,
              const TVehicleSurface& rear, FPD dt);
} //< namespace iGeek
#endif
#endif
