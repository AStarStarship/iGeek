// Copyright AStarship <https://astarship.net>.
// Included by the generated single-TU ArmPhysicsChecks target.
#include <cstdio>
#include <cmath>
using namespace iGeek;
static ISC failures = 0;
static void Check(BOL ok, const CHA* name) {
  printf("%s %s\n", ok ? "PASS" : "FAIL", name);
  if (!ok) ++failures;
}
int main() {
  ArmParams p;
  ArmState s;
  ArmDynamicsTerms f;
  Check(ArmForces(p, s, f), "force evaluation");
  Check(std::abs(f.grav_1 + (p.mass_1 * p.length_1 / 2 +
      p.mass_2 * p.length_1) * 9.81) < 1e-12 &&
      std::abs(f.grav_2 + p.mass_2 * p.length_2 / 2 * 9.81) < 1e-12,
      "horizontal gravity equals negative potential gradient");
  Check(std::abs(f.I11 - (p.inertia_1 + p.mass_1 * p.length_1 *
      p.length_1 / 4 + p.mass_2 * p.length_1 * p.length_1)) < 1e-12 &&
      std::abs(f.I12 - p.mass_2 * p.length_1 * p.length_2 / 2) < 1e-12,
      "absolute-angle inertia from COM kinetic energy");
  // A frame must agree with two half-duration frames (same RK substep).
  ArmState whole, split;
  whole.j1 = 1.0; whole.j2 = 0.8; whole.w1 = 0.1; whole.w2 = -0.2;
  split = whole;
  ArmControl control;
  ArmParams short_frame = p;
  short_frame.dt = p.dt / 2;
  short_frame.half_step = p.half_step / 2;
  BOL ok = ArmStep(whole, p, control) && ArmStep(split, short_frame, control) &&
      ArmStep(split, short_frame, control);
  // Compare against a tiny-time derivative to catch a shared time scale error.
  ArmParams tiny = p;
  tiny.dt = 1e-6; tiny.half_step = 5e-7;
  ArmState drift;
  drift.j1 = 1; drift.j2 = 0.8; drift.w1 = 0.1;
  ok = ok && ArmStep(drift, tiny, control);
  Check(ok && std::abs((drift.j1 - 1) / tiny.dt - 0.1) < 1e-4,
      "integrated elapsed time matches the clock");
  Check(ok && std::abs(whole.j1 - split.j1) < 1e-6 &&
      std::abs(whole.w1 - split.w1) < 1e-5, "frame subdivision convergence");
  ArmParams overflow = p;
  overflow.length_1 = 1e100;
  ArmState unchanged;
  unchanged.j1 = 1; unchanged.j2 = 0.8;
  ArmState before = unchanged;
  Check(!ArmStep(unchanged, overflow, control) &&
      unchanged.j1 == before.j1 && unchanged.j2 == before.j2 &&
      unchanged.w1 == before.w1 && unchanged.w2 == before.w2 &&
      unchanged.time == before.time && unchanged.tip_x == before.tip_x &&
      unchanged.tip_y == before.tip_y && unchanged.reach == before.reach,
      "failed intermediate dynamics leaves every state field unchanged");
  AArmEnv env(p, 7);
  FPD tx, ty;
  ArmFK(p, env.State().j1, env.State().j2, tx, ty);
  Check(std::abs(env.State().tip_x - tx) < 1e-12 &&
      std::abs(env.State().tip_y - ty) < 1e-12 &&
      std::abs(env.State().reach - std::hypot(tx, ty)) < 1e-12,
      "reset initializes cached forward kinematics");
  BOL reachable = true;
  for (IUD seed = 1; seed <= 256; ++seed) {
    env.ResetSeed(seed);
    FPD r = std::hypot(env.TargetX(), env.TargetY());
    FPD phi = std::atan2(env.TargetY(), env.TargetX());
    FPD delta = std::acos(r / (2 * p.length_1));
    reachable = reachable && std::isfinite(delta) && phi - delta >= -1e-12 &&
        phi + delta <= 1.5707963267948966 + 1e-12;
  }
  Check(reachable, "256 seeded targets have in-bounds inverse kinematics");
  // Independent energy oracle: COM translational velocities + COM rotation.
  auto energy = [](ArmParams& p, ArmState& s) {
    FPD vx1 = -p.length_1 / 2 * std::sin(s.j1) * s.w1;
    FPD vy1 = p.length_1 / 2 * std::cos(s.j1) * s.w1;
    FPD vx2 = 2 * vx1 - p.length_2 / 2 * std::sin(s.j2) * s.w2;
    FPD vy2 = 2 * vy1 + p.length_2 / 2 * std::cos(s.j2) * s.w2;
    return (p.mass_1 * (vx1 * vx1 + vy1 * vy1) +
        p.mass_2 * (vx2 * vx2 + vy2 * vy2) +
        p.inertia_1 * s.w1 * s.w1 + p.inertia_2 * s.w2 * s.w2) / 2 +
        9.81 * ((p.mass_1 / 2 + p.mass_2) * p.length_1 * std::sin(s.j1) +
        p.mass_2 * p.length_2 / 2 * std::sin(s.j2));
  };
  ArmParams conservative = p;
  conservative.damping = 0;
  conservative.dt = 0.001; conservative.half_step = 0.0005;
  ArmState free;
  free.j1 = 1.1; free.j2 = 0.7; free.w1 = 0.3; free.w2 = -0.2;
  FPD initial_energy = energy(conservative, free), error = 0;
  BOL integrated = true;
  for (ISC i = 0; i < 100; ++i) {
    integrated = ArmStep(free, conservative, control) && integrated;
    error = std::fmax(error, std::abs(energy(conservative, free) - initial_energy));
    integrated = integrated && free.j1 > 0 && free.j2 > 0 &&
        free.j1 < 1.5707963267948966 && free.j2 < 1.5707963267948966;
  }
  Check(integrated && error < 1e-9, "free-flight mechanical energy conservation");
  printf("maximum energy drift=%.12g J\n", error);
  ArmState held;
  held.j1 = 1.1; held.j2 = 0.7;
  ArmForces(p, held, f);
  ArmControl balance{-f.grav_1, -f.grav_2};
  BOL balanced = true;
  for (ISC i = 0; i < 100; ++i) balanced = ArmStep(held, p, balance) && balanced;
  Check(balanced && std::abs(held.j1 - 1.1) < 1e-10 &&
      std::abs(held.j2 - 0.7) < 1e-10, "gravity compensation holds for two seconds");
  ArmParams odd = p;
  odd.half_step = 0.007;
  Check(TArmSubsteps(odd) == 3, "non-dividing substep rounds up");
  env.ResetSeed(7);
  env.StepControl(control);
  Check(env.Metrics().physics_steps == IUD(TArmSubsteps(p)), "substep telemetry matches integration");
  ArmParams too_small = p;
  too_small.half_step = 1e-300;
  Check(!ArmParamsValid(too_small), "pathological substep count rejected before cast");
  FPD acceleration[2] = {17, 19};
  Check(!TArmDynamics(p, 1, 1, 0, 0, NAN, 0, acceleration) &&
      acceleration[0] == 17 && acceleration[1] == 19, "NaN dynamics preserves output");
  ArmState saturated, capped;
  saturated.j1 = 1; saturated.j2 = 0.8; capped = saturated;
  ArmControl huge{1e30, -1e30}, cap{p.max_torque, -p.max_torque};
  Check(ArmStep(saturated, p, huge) && ArmStep(capped, p, cap) &&
      saturated.j1 == capped.j1 && saturated.j2 == capped.j2 &&
      saturated.w1 == capped.w1 && saturated.w2 == capped.w2,
      "torque saturation is deterministic");
  printf("physics failures=%d\n", failures);
  return failures ? 1 : 0;
}
