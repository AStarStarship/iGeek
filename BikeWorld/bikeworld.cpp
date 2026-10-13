// Copyright AStarship <https://astarship.net>.
#include "TileRuntime.hxx"
#include "BikeScenarios.hxx"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
using namespace _;

static void Help() {
  printf("BikeWorld native headless simulator\n"
      "  --scenario all|straight|curves|obstacle|gravel|ice|uphill|downhill|launch\n"
      "  --episodes N    1..10000 seeded rides per scenario (default 1)\n"
      "  --seed N        unsigned 64-bit first seed (default 7)\n"
      "  --csv PATH      exclusive-create per-ride metrics CSV\n"
      "  --trace PATH    exclusive-create 50 Hz trace (one ride only)\n"
      "  --unassisted    bypass pilot; falling is reported as failure\n"
      "  --help\n");
}
static BOL ParseUInt(const CHA* text, IUD& value) {
  if (text == NILP || *text == 0) return false;
  for (const CHA* p = text; *p; ++p) if (*p < '0' || *p > '9') return false;
  errno = 0; CHA* end = NILP;
  value = IUD(strtoull(text, &end, 10));
  return errno == 0 && end != NILP && *end == 0;
}
static FPD Seconds() {
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) return 0;
  return FPD(time.tv_sec) + FPD(time.tv_nsec) * 1e-9;
}
static void Trace(FILE* file, const ABikeEnv& env) {
  if (file == NILP) return;
  const TBikeState& s = env.State();
  fprintf(file, "%.6f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,"
      "%.9f,%.9f,%d,%d,%s\n", s.time, s.x, s.y, s.heading, s.forward,
      s.lateral, s.yaw_rate, s.roll, s.roll_rate, s.steer,
      s.lateral_accel, s.utilization, ISC(s.supported), ISC(s.fallen),
      TBikeStatusName(env.Status()));
}
ISC main(ISC argc, CHA** argv) {
  ISC first = 0, last = BikeScenarioCount;
  IUD episodes = 1, seed = 7;
  const CHA* csv_path = NILP; const CHA* trace_path = NILP;
  BOL unassisted = false;
  for (ISC i = 1; i < argc; ++i) {
    const CHA* flag = argv[i];
    if (strcmp(flag, "--help") == 0) { Help(); return 0; }
    if (strcmp(flag, "--unassisted") == 0) { unassisted = true; continue; }
    if (i + 1 >= argc) { fprintf(stderr, "Missing value: %s\n", flag); return 2; }
    const CHA* value = argv[++i];
    if (strcmp(flag, "--scenario") == 0) {
      if (strcmp(value, "all") == 0) { first = 0; last = BikeScenarioCount; }
      else {
        first = -1;
        for (ISC s = 0; s < BikeScenarioCount; ++s)
          if (strcmp(value, TBikeScenarioName(s)) == 0) first = s;
        if (first < 0) { fprintf(stderr, "Unknown scenario: %s\n", value); return 2; }
        last = first + 1;
      }
    } else if (strcmp(flag, "--episodes") == 0) {
      if (!ParseUInt(value, episodes) || episodes < 1 || episodes > 10000) {
        fprintf(stderr, "Invalid episode count: %s\n", value); return 2;
      }
    } else if (strcmp(flag, "--seed") == 0) {
      if (!ParseUInt(value, seed)) { fprintf(stderr, "Invalid seed: %s\n", value); return 2; }
    } else if (strcmp(flag, "--csv") == 0) csv_path = value;
    else if (strcmp(flag, "--trace") == 0) trace_path = value;
    else { fprintf(stderr, "Unknown flag: %s\n", flag); return 2; }
  }
  if (trace_path != NILP && (last - first != 1 || episodes != 1)) {
    fprintf(stderr, "Trace requires one scenario and one episode.\n"); return 2;
  }
  FILE* csv = NILP; FILE* trace = NILP;
  if (csv_path != NILP) {
    csv = fopen(csv_path, "wx");
    if (csv == NILP) { perror(csv_path); return 2; }
    fprintf(csv, "scenario,seed,status,sim_time_s,distance_m,goal_error_m,"
        "final_speed_mps,peak_roll_rad,peak_cross_track_m,rms_cross_track_m,"
        "max_speed_mps,frames,physics_steps\n");
  }
  if (trace_path != NILP) {
    trace = fopen(trace_path, "wx");
    if (trace == NILP) { perror(trace_path); if (csv != NILP) fclose(csv); return 2; }
    fprintf(trace, "time_s,x_m,y_m,heading_rad,forward_mps,lateral_mps,"
        "yaw_rate_radps,roll_rad,roll_rate_radps,steer_rad,lateral_accel_mps2,"
        "tire_utilization,supported,fallen,status\n");
  }
  AVehiclePlanner* planner = new AVehiclePlanner;
  IUD total = 0, passed = 0, frames = 0, physics_steps = 0;
  FPD simulated = 0, started = Seconds();
  for (ISC scenario = first; scenario < last; ++scenario) {
    TileMap map(1.0f, 120, 64);
    TVehicleRoute route; TBikeState initial;
    if (!TBikeScenarioBuild(map, scenario, *planner, route, initial)) {
      fprintf(stderr, "Cannot build route: %s\n", TBikeScenarioName(scenario));
      delete planner; if (csv != NILP) fclose(csv); if (trace != NILP) fclose(trace);
      return 1;
    }
    IUD goals = 0; FPD peak_roll = 0, peak_cross_track = 0;
    for (IUD episode = 0; episode < episodes; ++episode) {
      ABikeEnv env(map, route, initial, seed + episode);
      Trace(trace, env);
      while (!env.Done()) {
        if (unassisted) env.StepControl(TBikeControl());
        else env.Step();
        Trace(trace, env);
      }
      const TBikeState& s = env.State(); const TBikeMetrics& m = env.Metrics();
      ++total;
      if (env.Status() == BikeGoal) { ++passed; ++goals; }
      frames += m.frames; physics_steps += m.physics_steps; simulated += s.time;
      peak_roll = TBikeMax(peak_roll, m.max_roll);
      peak_cross_track = TBikeMax(peak_cross_track, m.max_cross_track);
      FPD rms = m.frames ? sqrt(m.cross_track_squared / FPD(m.frames)) : 0;
      if (csv != NILP)
        fprintf(csv, "%s,%" PRIu64 ",%s,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,"
            "%.9f,%.9f,%" PRIu64 ",%" PRIu64 "\n",
            TBikeScenarioName(scenario), seed + episode, TBikeStatusName(env.Status()),
            s.time, s.distance, env.GoalDistance(), s.forward, m.max_roll,
            m.max_cross_track, rms, m.max_speed, m.frames, m.physics_steps);
    }
    printf("%-10s goals=%" PRIu64 "/%" PRIu64 " waypoints=%d "
        "peak_roll_deg=%.3f peak_cross_track_m=%.3f\n",
        TBikeScenarioName(scenario), goals, episodes, route.count,
        peak_roll * 180 / BikePi, peak_cross_track);
  }
  FPD wall = Seconds() - started;
  delete planner;
  BOL io_ok = true;
  if (csv != NILP) { if (ferror(csv)) io_ok = false; if (fclose(csv) != 0) io_ok = false; }
  if (trace != NILP) { if (ferror(trace)) io_ok = false; if (fclose(trace) != 0) io_ok = false; }
  if (!io_ok) { fprintf(stderr, "Output write failed.\n"); return 2; }
  printf("TOTAL goals=%" PRIu64 "/%" PRIu64 " frames=%" PRIu64
      " physics_steps=%" PRIu64 " simulated_s=%.3f wall_s=%.6f "
      "frames_per_s=%.1f realtime_factor=%.1f\n", passed, total, frames,
      physics_steps, simulated, wall, wall > 0 ? FPD(frames) / wall : 0,
      wall > 0 ? simulated / wall : 0);
  return passed == total ? 0 : 1;
}
