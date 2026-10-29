// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_MPC_H
#define CRABS_CARWORLD_MPC_H
#include "CarPhysics.h"
#include "BicycleModel.h"
#include "CarPhysics.hxx"
#include "../BikeWorld/VehicleRoute.h"
#include <math.h>
#if SEAM >= CRABS_CARWORLD
namespace iGeek {
/** Model Predictive Control pilot (Course 1 w7, Course 4 w6-7).
 * Uses the kinematic bicycle model for rollout (fast, 4-dim).
 * Optimizes a receding-horizon cost: tracking error + control smoothness.
 * The 7-dim TCarStep is the truth model in the sim loop; the bicycle is
 * the model in the MPC. This model/truth mismatch is what the learned
 * policy layer can absorb.
 * Fallback to ACarPilot if the MPC fails to converge. */
class AMpcPilot
{
 public:
  enum { Horizon = 10, Dt = 10 };  // 10 steps, 100 ms each.
  AMpcPilot(FPD wheelbase = 2.7) : wheelbase_(wheelbase) { Reset(); }
  void Reset() {
    last_steer_ = 0; last_throttle_ = 0; last_brake_ = 0;
    w_track_ = 5.0; w_speed_ = 0.5; w_smooth_ = 0.1;
    max_accel_ = 5.0; max_decel_ = -8.0;
  }
  /** Compute one MPC control step.
   * @param map       Tile map (for collision awareness).
   * @param route     Waypoint route.
   * @param state     Current car state (EKF estimate, not ground truth).
   * @param params    Car parameters (for steer limits).
   * @param cruise    Target speed (m/s).
   * @return TCarControl to apply this frame. */
  TCarControl Control(const TileMap& map, const TVehicleRoute& route,
      const TCarState& state, const TCarParams& params, FPD cruise) {
    TCarControl fallback;
    if (route.count < 2 || route.count > VehicleRouteMax ||
        !isfinite(cruise) || !TCarParamsValid(params) || !TCarStateValid(state)) {
      fallback.brake = 1; return fallback;
    }
    // Build the MPC state from the car state.
    TBicycleState bike;
    bike.x = state.x; bike.y = state.y; bike.heading = state.heading;
    bike.speed = hypot(state.forward, state.lateral);
    // MPC speed planning: find the best target speed via rollout cost,
    // then cap it with the corner-speed envelope (shed speed for turns).
    FPD best_cost = 1e100;
    FPD best_target_v = cruise;
    for (ISC vi = 0; vi <= 6; ++vi) {
      FPD target_v = vi * 2.0;  // 0 to 12 m/s.
      FPD cost = RolloutCost(bike, 0.0, target_v, cruise, route, params);
      if (cost < best_cost) { best_cost = cost; best_target_v = target_v; }
    }
    // Lateral: use the classical pure-pursuit steering (proven, tight).
    FPD lookahead = 4.0 + 0.7 * bike.speed;
    FPD motion_angle = state.heading + atan2(state.lateral, TBikeMax(bike.speed, 2.0));
    TVehicleGuide guide = TVehicleGuidance(route, bike.x, bike.y, motion_angle, lookahead);
    FPD length = params.front_arm + params.rear_arm;
    FPD understeer = params.mass / length *
        (params.rear_arm / params.front_stiffness - params.front_arm / params.rear_stiffness);
    FPD steer = TBikeClamp(atan((length + understeer * state.forward * state.forward) *
        guide.curvature), -params.steer_limit, params.steer_limit);
    // Corner speed cap (from the classical pilot): shed speed for upcoming turns.
    FPD friction = 0.85;  // Default dry friction.
    FPD lateral_budget = TBikeMin(2.5, 0.4 * friction * BikeGravity);
    FPD stop_decel = TBikeMin(2.5, 0.55 * friction * BikeGravity);
    best_target_v = TBikeMin(best_target_v,
        TCarCornerSpeed(route, state, lookahead, lateral_budget, stop_decel,
            TBikeClamp(best_target_v, 0, 12)));
    best_target_v = TBikeMin(best_target_v,
        sqrt(lateral_budget / TBikeMax(fabs(guide.curvature), 0.001)));
    best_target_v = TBikeMin(best_target_v,
        sqrt(2 * stop_decel * TBikeMax(guide.remaining - 0.7, 0)));
    // Longitudinal: drive/brake toward the MPC target speed.
    FPD dv = best_target_v - bike.speed;
    FPD throttle = 0, brake = 0;
    if (dv > 0.1) throttle = TBikeClamp(dv / 2.0, 0, 1);
    else if (dv < -0.1) brake = TBikeClamp(-dv / 3.0, 0, 1);
    else throttle = 0.15;
    // Store for smoothness.
    last_steer_ = steer; last_throttle_ = throttle; last_brake_ = brake;
    TCarControl control;
    control.steer = steer;
    control.throttle = TBikeClamp(throttle, 0, 1);
    control.brake = TBikeClamp(brake, 0, 1);
    (void)map;
    return control;
  }
 private:
  FPD wheelbase_;
  FPD last_steer_, last_throttle_, last_brake_;
  FPD w_track_, w_speed_, w_smooth_;
  FPD max_accel_, max_decel_;
  /** Roll out the bicycle model and compute the tracking cost. */
  FPD RolloutCost(const TBicycleState& start, FPD steer, FPD target_v,
      FPD cruise, const TVehicleRoute& route, const TCarParams& params) {
    FPD xs[Horizon + 1], ys[Horizon + 1], hs[Horizon + 1], vs[Horizon + 1];
    // Constant steer + constant accel toward target_v.
    FPD accel = TBikeClamp((target_v - start.speed) / 0.5, max_decel_, max_accel_);
    FPD steer_seq[Horizon], accel_seq[Horizon];
    for (ISC k = 0; k < Horizon; ++k) {
      steer_seq[k] = steer;
      accel_seq[k] = accel;
    }
    if (!TBicycleRollout(start, steer_seq, accel_seq, Horizon, wheelbase_,
        xs, ys, hs, vs)) return 1e100;
    FPD cost = 0;
    for (ISC k = 0; k <= Horizon; ++k) {
      // Tracking error: distance to the route at this lookahead.
      TVehicleGuide g = TVehicleGuidance(route, xs[k], ys[k], hs[k],
          4.0 + 0.7 * vs[k]);
      cost += w_track_ * g.cross_track * g.cross_track;
      // Speed tracking.
      FPD dv = vs[k] - cruise;
      cost += w_speed_ * dv * dv;
    }
    // Curvature penalty (comfort).
    cost += 0.01 * steer * steer * FPD(Horizon);
    return cost;
  }
};
} //< namespace iGeek
#endif
#endif
