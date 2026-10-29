// Copyright AStarship <https://astarship.net>.
// 3D physics unit tests: flat ground rest, throttle pitch, turn roll, brake dive.
#include "../BikeWorld/TileRuntime.hxx"
#include "CarPhysics3D.h"
#include "CarPilot.hxx"
#include <stdio.h>
using namespace _;
static ISC checks = 0, failures = 0;
static void Check(const CHA* name, BOL ok) {
  ++checks;
  if (!ok) { ++failures; D_COUT("FAIL " << name << "\n"); }
}
ISC main() {
  // Flat map.
  TileMap map(1.0f, 120, 64);
  TCarParams3D p;
  TCarParams3DDerive(p);
  TCarContacts3D contacts;
  for (ISC i = 0; i < CarWheelCount; ++i) {
    contacts.wheels[i] = TVehicleSample(map, 32.5, 32.5);
    contacts.ground_height[i] = 0;
  }
  // Test 1: flat ground, zero control, car at rest -> stays level.
  TCarState3D car;
  car.x = 32.5; car.y = 32.5; car.heading = 0;
  TCarControl zero;
  for (ISC frame = 0; frame < 100; ++frame) {
    for (ISC sub = 0; sub < 4; ++sub)
      if (!TCarStep3D(car, p, zero, contacts, 0.005)) break;
  }
  Check("3d: rest stays level (pitch)", fabs(car.pitch) < 0.005);
  Check("3d: rest stays level (roll)", fabs(car.roll) < 0.005);
  Check("3d: rest no movement", fabs(car.x - 32.5) < 1e-6);
  FPD fz_sum = 0;
  for (ISC i = 0; i < CarWheelCount; ++i) fz_sum += car.vertical_tire_force[i];
  Check("3d: Fz closure ~ m*g", fabs(fz_sum - p.base.mass * BikeGravity) <
      p.base.mass * BikeGravity * 0.05);
  // Test 2: full throttle at speed -> pitch up (nose up > 0).
  TCarState3D car2;
  car2.x = 32.5; car2.y = 32.5; car2.heading = 0; car2.forward = 6;
  TCarControl throttle; throttle.throttle = 1;
  FPD max_pitch = 0;
  for (ISC frame = 0; frame < 25; ++frame) {
    for (ISC sub = 0; sub < 4; ++sub)
      if (!TCarStep3D(car2, p, throttle, contacts, 0.005)) break;
    max_pitch = TBikeMax(max_pitch, car2.pitch);
  }
  Check("3d: throttle pitches up", max_pitch > 0.005);
  Check("3d: throttle pitch bounded", max_pitch < 0.05);
  // Test 3: hard left turn at speed -> roll.
  TCarState3D car3;
  car3.x = 32.5; car3.y = 32.5; car3.heading = 0; car3.forward = 12;
  TCarControl turn; turn.steer = p.base.steer_limit;
  FPD max_roll = 0;
  for (ISC frame = 0; frame < 25; ++frame) {
    for (ISC sub = 0; sub < 4; ++sub)
      if (!TCarStep3D(car3, p, turn, contacts, 0.005)) break;
    max_roll = TBikeMax(max_roll, fabs(car3.roll));
  }
  Check("3d: turn rolls", max_roll > 0.01);
  Check("3d: turn roll bounded", max_roll < 0.3);
  // Test 4: braking at speed -> nose dive (pitch < 0).
  TCarState3D car4;
  car4.x = 32.5; car4.y = 32.5; car4.heading = 0; car4.forward = 12;
  TCarControl brake; brake.brake = 1;
  FPD min_pitch = 0;
  for (ISC frame = 0; frame < 25; ++frame) {
    for (ISC sub = 0; sub < 4; ++sub)
      if (!TCarStep3D(car4, p, brake, contacts, 0.005)) break;
    min_pitch = TBikeMin(min_pitch, car4.pitch);
  }
  Check("3d: brake nose dives", min_pitch < -0.002);
  Check("3d: brake slows car", car4.forward < 12);
  // Test 5: determinism.
  TCarState3D car5a, car5b;
  car5a.x = car5b.x = 32.5; car5a.y = car5b.y = 32.5;
  car5a.forward = car5b.forward = 5;
  TCarControl ctrl; ctrl.throttle = 0.3; ctrl.steer = 0.05;
  for (ISC frame = 0; frame < 50; ++frame) {
    for (ISC sub = 0; sub < 4; ++sub) {
      TCarStep3D(car5a, p, ctrl, contacts, 0.005);
      TCarStep3D(car5b, p, ctrl, contacts, 0.005);
    }
  }
  Check("3d: deterministic", car5a.x == car5b.x && car5a.y == car5b.y &&
      car5a.pitch == car5b.pitch && car5a.roll == car5b.roll);
  // Test 6: invalid input (NaN) -> no mutation.
  TCarState3D car6; car6.x = 32.5; car6.y = 32.5; car6.forward = 5;
  TCarControl bad; bad.throttle = FPD(1) / FPD(0);  // NaN.
  FPD old_x = car6.x;
  BOL valid = TCarStep3D(car6, p, bad, contacts, 0.005);
  Check("3d: invalid input rejected", !valid || car6.x == old_x);
  D_COUT("CARWORLD_3D checks=" << checks << " failures=" << failures << "\n");
  return failures == 0 ? 0 : 1;
}
