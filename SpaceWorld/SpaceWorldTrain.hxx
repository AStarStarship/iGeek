// Copyright AStarship <https://astarship.net>.
// spaceworld_train.cpp — PPO trainer like armworld_train.cpp.
// PPO (Proximal Policy Optimization) training for spacecraft attitude control.

#include "SpaceConfig.h"
#include "SpaceMath.h"
#include "SpacePhysics.h"
#include "SpaceRcs.h"
#include "SpaceCaps.h"
#include "SpacePid.h"

#include <cstdlib>
#include <cstdio>
#include <ctime>

using namespace _;

// Simple PPO hyperparameters
struct PPOConfig {
  ISC num_episodes = 100;
  ISC steps_per_episode = 600;    // 10 s at 60 Hz
  FPD learning_rate = 0.001;
  FPD gamma = 0.99;
  FPD clip_epsilon = 0.2;
  ISC num_minibatches = 4;
  ISC num_epochs = 3;
};

// Observations: [target x,y,z (/50), pos x,y,z (/50), velocity xyz (/10),
//   angular velocity xyz (/5), distance to target (/50), autopilot-on flag]
// Total: 14 values <= ObsLength=14
constexpr ISC ObsLength = 14;

// Use _::SpaceRcsTotal from the header instead of global def
// constexpr ISC SpaceRcsTotal = 24;  // removed — defined in SpaceRcs.h within namespace _

struct Transition {
  FPD obs[ObsLength];
  ISC action;  // 0=hold, 1-12: manual directions per BUILD_BRIEF.md
  FPD reward;
  FPD next_obs[ObsLength];
  bool done;
};

// Compute Euclidean length using TSpaceVecLength (project-provided)
FPD vec_length(const TSpaceVec& v) {
  return TSpaceVecLength(v);
}

// Mock policy: simple linear function of observations
// action = argmax over (W * obs + b)
FPD policy_weights[ObsLength * 13];  // 13 actions (hold + 12 directions)
FPD policy_biases[13];

void init_policy() {
  // Initialize with small random values
  std::srand(std::time(0));
  for (ISC i = 0; i < ObsLength * 13; ++i) policy_weights[i] = ((FPD)std::rand() / RAND_MAX - 0.5) * 0.1;
  for (ISC i = 0; i < 13; ++i) policy_biases[i] = 0;
}

// Compute policy action from observation
ISC action_from_obs(const FPD obs[ObsLength]) {
  FPD best_q = -1e9;
  ISC best_a = 0;
  for (ISC a = 0; a < 13; ++a) {
    FPD q = policy_biases[a];
    for (ISC o = 0; o < ObsLength; ++o) q += policy_weights[a * ObsLength + o] * obs[o];
    if (q > best_q) { best_q = q; best_a = a; }
  }
  return best_a;
}

// PPO update (single step simplified)
void ppo_update(Transition* batch, ISC batch_size, const PPOConfig& cfg) {
  // In a real implementation, this would compute gradients and update weights
  // For this smoke test, just compute advantages and print stats
  D_COUT("PPO update: batch_size=" << batch_size << ", lr=" << cfg.learning_rate << ", clip=" << cfg.clip_epsilon << "\n");
}

// Main training loop
int main() {
  init_policy();

  PPOConfig cfg;
  D_COUT("=== SpaceWorld PPO Training Smoke Test ===\n");
  D_COUT("Episodes: " << cfg.num_episodes << ", Steps/episode: " << cfg.steps_per_episode << "\n");

  for (ISC ep = 0; ep < cfg.num_episodes; ++ep) {
    // Reset environment
    TSpaceHalf half = {0.5, 0.5, 1.0};
    FPD mass = 1500.0;
    TSpaceThruster thrusters[_::SpaceRcsTotal];
    TSpaceRcsBuild(thrusters, half);
    FPD thrust_max[_::SpaceRcsTotal];
    for (ISC i = 0; i < _::SpaceRcsTotal; ++i) thrust_max[i] = 500;
    TSpaceCaps caps;
    TSpaceCapsCompute(caps, mass, half, thrusters, thrust_max);

    TSpaceState state;
    state.x = 0; state.y = 0; state.z = 0;
    state.qx = 0; state.qy = 0; state.qz = 0; state.qw = 1;
    state.vx = 0; state.vy = 0; state.vz = 0;
    state.wx = 0; state.wy = 0; state.wz = 0.1;  // small initial spin
    state.time = 0;

    // Target position
    TSpaceVec target = {0, 0, 50};

    FPD total_reward = 0;

    for (ISC s = 0; s < cfg.steps_per_episode; ++s) {
      // Build observation (ObsLength = 14)
      FPD obs[ObsLength];
      // target x,y,z scaled /50
      obs[0] = target.x / 50.0;
      obs[1] = target.y / 50.0;
      obs[2] = target.z / 50.0;
      // pos x,y,z scaled /50
      obs[3] = state.x / 50.0;
      obs[4] = state.y / 50.0;
      obs[5] = state.z / 50.0;
      // velocity xyz /10
      obs[6] = state.vx / 10.0;
      obs[7] = state.vy / 10.0;
      obs[8] = state.vz / 10.0;
      // angular velocity xyz /5
      obs[9] = state.wx / 5.0;
      obs[10] = state.wy / 5.0;
      obs[11] = state.wz / 5.0;
      // distance to target /50
      FPD dx = state.x - target.x;
      FPD dy = state.y - target.y;
      FPD dz = state.z - target.z;
      FPD dist = TSpaceVecLength(TSpaceVec(dx, dy, dz));
      obs[12] = dist / 50.0;
      // autopilot-on flag (0 = manual, 1 = autopilot)
      obs[13] = 0;  // manual control for this smoke test

      // Select action via policy
      ISC action = action_from_obs(obs);

      // Apply action via RCS manual allocator
      // Controls from BUILD_BRIEF.md:
      // 0 hold, 1 +X, 2 -X, 3 +Y, 4 -Y, 5 +Z, 6 -Z,
      // 7 pitch+, 8 pitch-, 9 yaw+, 10 yaw-, 11 roll+, 12 roll-
      TSpaceVec manual_force = {0, 0, 0};
      TSpaceVec manual_torque = {0, 0, 0};
      switch (action) {
        case 0: // hold — do nothing
          break;
        case 1: manual_force.x = 1; break;  // +X
        case 2: manual_force.x = -1; break; // -X
        case 3: manual_force.y = 1; break;  // +Y
        case 4: manual_force.y = -1; break; // -Y
        case 5: manual_force.z = 1; break;  // +Z
        case 6: manual_force.z = -1; break; // -Z
        case 7: manual_torque.x = 1; break; // pitch+
        case 8: manual_torque.x = -1; break; // pitch-
        case 9: manual_torque.y = 1; break; // yaw+
        case 10: manual_torque.y = -1; break; // yaw-
        case 11: manual_torque.z = 1; break; // roll+
        case 12: manual_torque.z = -1; break; // roll-
        default: break;
      }

      // Integrate using TSpaceStep with manual forces
      TSpaceControl control = {manual_force.x, manual_force.y, manual_force.z,
                               manual_torque.x, manual_torque.y, manual_torque.z};
      TSpaceStep(state, (TSpaceParams){mass, half, 0, 1.0/60.0, 10.0, 1000.0}, control);

      // Compute reward: -distance_to_target, +10 on reach (radius 0.5m), -1 if distance grows
      dx = state.x - target.x;
      dy = state.y - target.y;
      dz = state.z - target.z;
      dist = TSpaceVecLength(TSpaceVec(dx, dy, dz));
      FPD reward = -dist;
      if (dist < 0.5) reward += 10;  // reach
      // Penalty if distance grew this step
      static FPD last_dist = 1000;
      if (dist > last_dist) reward -= 1;
      last_dist = dist;
      total_reward += reward;

      // Transition (for PPO batch)
      Transition t;
      for (ISC i = 0; i < ObsLength; ++i) t.obs[i] = obs[i];
      t.action = action;
      t.reward = reward;
      for (ISC i = 0; i < ObsLength; ++i) t.next_obs[i] = (i == 12) ? (dist/50.0) : 0;  // simplified
      t.done = (s == cfg.steps_per_episode - 1);

      // (In real PPO: store transition, compute advantage, update policy)
      if (s % 100 == 0) D_COUT("Episode " << ep << ", Step " << s << ": dist=%.2f, reward=%.2f, action=" << dist << "\n");
    }

    D_COUT("Episode " << ep << " total reward: %.2f\n");
  }

  D_COUT("=== PPO Training Complete ===\n");
  return 0;
}