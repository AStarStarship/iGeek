// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekCarWorldPhysicsH
#define _iGeekCarWorldPhysicsH
#include "CarConfig.h"
#include "../../Worlds/Bike/BikePhysics.h"
#if _Seam >= _iGeekCarWorldCore
namespace iGeek {
enum { CarFrontLeft, CarFrontRight, CarRearLeft, CarRearRight, CarWheelCount };
/** SI units. Planar rigid body; four independent tire forces, static axle loads. */
struct TCarParams
{
  FPD mass = 1400, front_arm = 1.15, rear_arm = 1.55, track = 1.60;
  FPD yaw_inertia = 2500, front_stiffness = 80000, rear_stiffness = 80000;
  FPD drag_area = 0.65, air_density = 1.225;
  FPD drive_force = 7000, drive_power = 60000, brake_force = 16000;
  FPD brake_bias = 0.65, steer_limit = 0.55, steer_rate = 0.8, steer_tau = 0.15;
  // Heading-independent square envelope covers a 4.5 x 1.8 metre chassis.
  FPD body_radius = 2.65;
};
struct TCarControl
{
  FPD steer = 0, throttle = 0, brake = 0, reverse = 0;
};
struct TCarContacts
{
  TVehicleSurface wheels[CarWheelCount];
};
struct alignas(ACPUCacheLineSize) TCarState
{
  FPD x = 0, y = 0, heading = 0, forward = 0, lateral = 0;
  FPD yaw_rate = 0, steer = 0, time = 0, distance = 0;
  FPD longitudinal_accel = 0, lateral_accel = 0, utilization = 0;
};
/** Forward-only planar car. Control is steer angle plus throttle/brake [0,1].
 * @return false without mutating state for invalid/nonfinite input.
 * @pre dt in (0,0.01]; caller handles collisions separately.
 */
BOL TCarStep(TCarState& car, TCarParams& params,
             TCarControl& control, TCarContacts& contacts, FPD dt);
} //< namespace iGeek
#endif
#endif
