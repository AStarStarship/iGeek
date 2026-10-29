// Copyright AStarship <https://astarship.net>.
#include "../BikeWorld/TileRuntime.hxx"
#include "CarScenarios.hxx"
#include "CarEKF.h"
#include "CarMpc.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
using namespace _;
static void Help() {
  D_COUT("CarWorld native headless 2D simulator\n"
      "  --scenario all|straight|curves|obstacle|gravel|ice|uphill|downhill|launch|ice_curves\n"
      "  --episodes N    1..10000 seeded drives per scenario (default 1)\n"
      "  --seed N        unsigned 64-bit first seed (default 7)\n"
      "  --action cruise|slow|fast|stop|crawl (default cruise)\n"
      "  --csv PATH      exclusive-create per-drive metrics CSV\n"
      "  --trace PATH    exclusive-create 50 Hz trace (one drive only)\n"
      "  --barrier       add a wall after planning (emergency-stop control)\n"
      "  --open-loop     bypass driver: fixed 0 steer, 0.35 throttle\n"
      "  --pilot pilot|mpc|ekf_mpc  classical|grid-MPC|EKF+MPC (default pilot)\n"
      "  --help\n");
}
static BOL ParseUInt(const CHA* text, IUD& value) {
  if (text == NILP || *text == 0) return false;
  for (const CHA* p = text; *p; ++p) if (*p < '0' || *p > '9') return false;
  errno = 0; CHA* end = NILP; value = IUD(strtoull(text, &end, 10));
  return errno == 0 && end != NILP && *end == 0;
}
static FPD Seconds() {
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) return 0;
  return FPD(time.tv_sec) + FPD(time.tv_nsec) * 1e-9;
}
static void Trace(FILE* file, const ACarEnv& env) {
  if (file == NILP) return;
  const TCarState& s = env.State();
  fD_COUT(file, "%.6f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f," << s.time << "\n");
}
ISC main(ISC argc, CHA** argv) {
  ISC first = 0, last = CarScenarioCount, action = CarCruise;
  IUD episodes = 1, seed = 7;
  const CHA* csv_path = NILP; const CHA* trace_path = NILP;
  BOL barrier = false, open_loop = false;
  ISC pilot_mode = 0;  // 0=pilot, 1=mpc, 2=ekf_mpc
  for (ISC i = 1; i < argc; ++i) {
    const CHA* flag = argv[i];
    if (strcmp(flag, "--help") == 0) { Help(); return 0; }
    if (strcmp(flag, "--barrier") == 0) { barrier = true; continue; }
    if (strcmp(flag, "--open-loop") == 0) { open_loop = true; continue; }
    if (i + 1 >= argc) { fD_COUT(stderr, "Missing value: %s\n", flag); return 2; }
    const CHA* value = argv[++i];
    if (strcmp(flag, "--pilot") == 0) {
      if (strcmp(value, "mpc") == 0) pilot_mode = 1;
      else if (strcmp(value, "ekf_mpc") == 0) pilot_mode = 2;
      else pilot_mode = 0;
      continue;
    }
    if (strcmp(flag, "--scenario") == 0) {
      if (strcmp(value, "all") == 0) { first = 0; last = CarScenarioCount; }
      else {
        first = -1;
        for (ISC s = 0; s < CarScenarioCount; ++s)
          if (strcmp(value, TCarScenarioName(s)) == 0) first = s;
        if (first < 0) { fD_COUT(stderr, "Unknown scenario: %s\n", value); return 2; }
        last = first + 1;
      }
    } else if (strcmp(flag, "--episodes") == 0) {
      if (!ParseUInt(value, episodes) || episodes < 1 || episodes > 10000) {
        fD_COUT(stderr, "Invalid episode count: %s\n", value); return 2;
      }
    } else if (strcmp(flag, "--seed") == 0) {
      if (!ParseUInt(value, seed)) { fD_COUT(stderr, "Invalid seed: %s\n", value); return 2; }
    } else if (strcmp(flag, "--action") == 0) {
      const CHA* names[] = {"cruise", "slow", "fast", "stop", "crawl"}; action = -1;
      for (ISC a = 0; a < CarActionCount; ++a) if (strcmp(value, names[a]) == 0) action = a;
      if (action < 0) { fD_COUT(stderr, "Invalid action: %s\n", value); return 2; }
    } else if (strcmp(flag, "--csv") == 0) csv_path = value;
    else if (strcmp(flag, "--trace") == 0) trace_path = value;
    else { fD_COUT(stderr, "Unknown flag: %s\n", flag); return 2; }
  }
  if (seed > UINT64_MAX - (episodes - 1)) {
    fD_COUT(stderr, "Episode seed range overflows uint64.\n"); return 2;
  }
  if (trace_path != NILP && (last - first != 1 || episodes != 1)) {
    fD_COUT(stderr, "Trace requires one scenario and one episode.\n"); return 2;
  }
  if (trace_path != NILP && csv_path != NILP && strcmp(trace_path, csv_path) == 0) {
    fD_COUT(stderr, "CSV and trace paths must differ.\n"); return 2;
  }
  FILE* csv = NILP; FILE* trace = NILP;
  if (csv_path != NILP) {
    csv = fopen(csv_path, "wx"); if (csv == NILP) { perror(csv_path); return 2; }
    fD_COUT(csv, "scenario,seed,status,sim_time_s,distance_m,goal_error_m,final_speed_mps,"
        "peak_cross_track_m,rms_cross_track_m,max_speed_mps,peak_lateral_accel_mps2,"
        "peak_tire_demand,frames,physics_steps\n");
  }
  if (trace_path != NILP) {
    trace = fopen(trace_path, "wx");
    if (trace == NILP) { perror(trace_path); if (csv != NILP) fclose(csv); return 2; }
    fD_COUT(trace, "time_s,x_m,y_m,heading_rad,forward_mps,lateral_mps,yaw_rate_radps,"
        "steer_rad,longitudinal_accel_mps2,lateral_accel_mps2,tire_demand,status\n");
  }
  AVehiclePlanner* planner = new AVehiclePlanner;
  IUD total = 0, passed = 0, frames = 0, physics_steps = 0;
  FPD simulated = 0, started = Seconds();
  for (ISC scenario = first; scenario < last; ++scenario) {
    TileMap map(1.0f, 120, 64); TVehicleRoute route; TCarState initial;
    if (!TCarScenarioBuild(map, scenario, *planner, route, initial)) {
      fD_COUT(stderr, "Cannot build route: %s\n", TCarScenarioName(scenario));
      delete planner; if (csv != NILP) fclose(csv); if (trace != NILP) fclose(trace); return 1;
    }
    if (barrier) for (ISC y = 0; y < 64; ++y) map.AddTile(45, y, TIntRect(), true, VehicleWall);
    IUD goals = 0; FPD peak_cross_track = 0, peak_accel = 0;
    for (IUD episode = 0; episode < episodes; ++episode) {
      ACarEnv env(map, route, initial, seed + episode); Trace(trace, env);
      // MPC + EKF state (only when pilot_mode > 0).
      TCarParams p = env.Params();
      AMpcPilot mpc(p.front_arm + p.rear_arm);
      ACarEKF ekf;
      if (pilot_mode > 0) ekf.Reset(initial.x, initial.y, initial.heading, initial.forward);
      while (!env.Done()) {
        if (open_loop) { TCarControl control; control.throttle = 0.35; env.StepControl(control); }
        else if (pilot_mode > 0) {
          const TCarState& s = env.State();
          if (pilot_mode == 2) {
            // EKF: predict + update with "sensors" (ground truth for now).
            ekf.Predict(s.steer, s.longitudinal_accel, 0.02);
            ekf.UpdateGps(s.x, s.y);
            ekf.UpdateOdometry(s.forward);
          }
          // MPC control from (EKF-estimated or direct) state.
          TCarState estimated = s;
          if (pilot_mode == 2) {
            estimated.x = ekf.X(); estimated.y = ekf.Y();
            estimated.heading = ekf.Heading();
            estimated.forward = ekf.Speed();
          }
          TCarControl control = mpc.Control(map, route, estimated, env.Params(), 12.0);
          env.StepControl(control);
        }
        else env.Step(action);
        Trace(trace, env);
      }
      const TCarState& s = env.State(); const TCarMetrics& m = env.Metrics();
      ++total; if (env.Status() == CarGoal) { ++passed; ++goals; }
      frames += m.frames; physics_steps += m.physics_steps; simulated += s.time;
      peak_cross_track = TBikeMax(peak_cross_track, m.max_cross_track);
      peak_accel = TBikeMax(peak_accel, m.max_lateral_accel);
      FPD rms = m.frames ? sqrt(m.cross_track_squared / FPD(m.frames)) : 0;
      if (csv != NILP)
        fD_COUT(csv, "%s,%" PRIu64 ",%s,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,"
            "%.9f,%.9f,%" PRIu64 ",%" PRIu64 "\n", TCarScenarioName(scenario),
            seed + episode, TCarStatusName(env.Status()), s.time, s.distance,
            env.GoalDistance(), hypot(s.forward, s.lateral), m.max_cross_track, rms,
            m.max_speed, m.max_lateral_accel, m.max_utilization, m.frames, m.physics_steps);
    }
    D_COUT("%-10s goals=%" PRIu64 "/%" PRIu64 " waypoints=" << TCarScenarioName(scenario) << " peak_cross_track_m=%.3f "
        "peak_lateral_accel_mps2=%.3f\n");
  }
  FPD wall = Seconds() - started; delete planner;
  BOL io_ok = true;
  if (csv != NILP) { if (ferror(csv)) io_ok = false; if (fclose(csv) != 0) io_ok = false; }
  if (trace != NILP) { if (ferror(trace)) io_ok = false; if (fclose(trace) != 0) io_ok = false; }
  if (!io_ok) { fD_COUT(stderr, "Output write failed.\n"); return 2; }
  D_COUT("TOTAL goals=%" PRIu64 "/%" PRIu64 " frames=%" PRIu64 " physics_steps=%" PRIu64
      " simulated_s=%.3f wall_s=%.6f frames_per_s=%.1f realtime_factor=%.1f\n");
  return passed == total ? 0 : 1;
}
