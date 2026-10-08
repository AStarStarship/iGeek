// Copyright AStarship <https://astarship.net>.
// PacWorld.hxx — headless Pac-Man env + vectorized Gym implementations.
// No C++ stdlib. Math from <math.h> (C). Deterministic per seed.
#include "PacWorld.h"
#include <math.h>

namespace _ {

/* Deterministic LCG (64-bit) — same family as the iGeek policy init. */
IUD PacWorldEnv::NextRand() {
  rng_ = rng_ * 6364136223846793005ULL + 1442695040888963407ULL;
  return (rng_ >> 11) & 0xFFFFFFFFu;
}

PacWorldEnv::PacWorldEnv(ISC cols, ISC rows, IUD seed)
    : cols_(cols < 7 ? 7 : cols), rows_(rows < 7 ? 7 : rows), seed_(seed),
      grid_(NILP), obs_(NILP), pac_x_(1), pac_y_(1), pac_dir_(ActionStay),
      ghost_count_(0), steps_(0), lives_(LifeCount), score_(0),
      done_(false), won_(false), rng_(seed_ ? seed_ : 1) {
  for (ISC i = 0; i < 16; ++i) {
    ghost_x_[i] = 0; ghost_y_[i] = 0; ghost_dir_[i] = ActionStay;
    ghost_weak_[i] = 0;
  }
  grid_ = new IUA[cols_ * rows_]();
  obs_ = new FPC[ObsLength]();
  GenerateMaze();
  Reset();
}

PacWorldEnv::~PacWorldEnv() {
  if (grid_ != NILP) { delete[] grid_; grid_ = NILP; }
  if (obs_ != NILP) { delete[] obs_; obs_ = NILP; }
}

ISC PacWorldEnv::Clamp(ISC v, ISC lo, ISC hi) const {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

BOL PacWorldEnv::IsWall(ISC x, ISC y) const {
  if (x < 0 || y < 0 || x >= cols_ || y >= rows_) return true;
  return grid_[y * cols_ + x] == CellWall;
}

/* Seeded maze: a solid border, then a random interior of walls (~30%) with
   guaranteed open start + ghost cells. Deterministic per seed_ via rng_. */
void PacWorldEnv::GenerateMaze() {
  rng_ = seed_ ? seed_ : 1;
  for (ISC y = 0; y < rows_; ++y) {
    for (ISC x = 0; x < cols_; ++x) {
      BOL border = (x == 0 || y == 0 || x == cols_ - 1 || y == rows_ - 1);
      if (border) { grid_[y * cols_ + x] = CellWall; continue; }
      IUD r = NextRand();
      if (r < 0x40000000u) {        // ~25% walls in the interior.
        grid_[y * cols_ + x] = CellWall;
      } else {
        grid_[y * cols_ + x] = CellEmpty;
      }
    }
  }
  // Carve a guaranteed-open start cell for pac + scatter dots in open cells.
  pac_x_ = 1; pac_y_ = 1;
  grid_[pac_y_ * cols_ + pac_x_] = CellEmpty;
  // Open the four neighbours of start if they're walls (so pac can move).
  grid_[pac_y_ * cols_ + pac_x_ + 1] = CellEmpty;
  grid_[(pac_y_ + 1) * cols_ + pac_x_] = CellEmpty;
  // Scatter dots in ~60% of open interior cells.
  for (ISC y = 1; y < rows_ - 1; ++y) {
    for (ISC x = 1; x < cols_ - 1; ++x) {
      ISC idx = y * cols_ + x;
      if (grid_[idx] != CellEmpty) continue;
      if (x == pac_x_ && y == pac_y_) continue;
      IUD r = NextRand();
      if (r < 0x90000000u) grid_[idx] = CellDot;
    }
  }
  // A few superdots (weak triggers) + a bonus.
  ISC placed_super = 0, placed_bonus = 0;
  for (ISC y = 1; y < rows_ - 1 && (placed_super < 2 || placed_bonus < 1); ++y) {
    for (ISC x = 1; x < cols_ - 1 && (placed_super < 2 || placed_bonus < 1); ++x) {
      ISC idx = y * cols_ + x;
      if (grid_[idx] != CellDot) continue;
      IUD r = NextRand();
      if (placed_super < 2 && r < 0x10000000u) { grid_[idx] = CellSuperDot; ++placed_super; }
      else if (placed_bonus < 1 && r > 0xF0000000u) { grid_[idx] = CellBonus; ++placed_bonus; }
    }
  }
}

void PacWorldEnv::PlaceGhosts() {
  ghost_count_ = 0;
  // Place ghosts in open cells far from pac (deterministic scan).
  for (ISC y = 1; y < rows_ - 1 && ghost_count_ < 3; ++y) {
    for (ISC x = 1; x < cols_ - 1 && ghost_count_ < 3; ++x) {
      ISC idx = y * cols_ + x;
      if (grid_[idx] != CellEmpty) continue;
      if (x == pac_x_ && y == pac_y_) continue;
      // Require some distance from pac so the game starts fair.
      ISC dx = x - pac_x_, dy = y - pac_y_;
      if (dx < 0) dx = -dx;
      if (dy < 0) dy = -dy;
      if (dx + dy < 4) continue;
      ghost_x_[ghost_count_] = x;
      ghost_y_[ghost_count_] = y;
      ghost_dir_[ghost_count_] = ActionStay;
      ghost_weak_[ghost_count_] = 0;
      grid_[idx] = CellEmpty;  // ensure the spawn cell is open.
      ++ghost_count_;
    }
  }
  // Guarantee at least 1 ghost if none placed (tiny maps).
  if (ghost_count_ == 0) {
    for (ISC y = 1; y < rows_ - 1 && ghost_count_ < 1; ++y)
      for (ISC x = 1; x < cols_ - 1 && ghost_count_ < 1; ++x) {
        ISC idx = y * cols_ + x;
        if (grid_[idx] == CellEmpty && !(x == pac_x_ && y == pac_y_)) {
          ghost_x_[0] = x; ghost_y_[0] = y;
          ghost_dir_[0] = ActionStay; ghost_weak_[0] = 0;
          ++ghost_count_;
        }
      }
  }
}

void PacWorldEnv::ResetGhosts() {
  for (ISC i = 0; i < ghost_count_; ++i) {
    // Respawn at the original spawn cell; if it became a dot, keep the cell.
    ghost_weak_[i] = 0;
    ghost_dir_[i] = ActionStay;
  }
}

void PacWorldEnv::Reset() {
  // Regenerate a fresh maze per episode for variety, but keep the SAME
  // layout within an episode. Episodes are i.i.d. per seed ONLY IF every
  // reset returns to the same starting point of the RNG stream — so rewind
  // to seed_ before regenerating. Without the rewind, consecutive episodes
  // saw consecutive LCG states and the layout drifted every episode (the
  // maze regenerated in place mid-RNG-stream).
  rng_ = seed_;
  GenerateMaze();
  pac_x_ = 1; pac_y_ = 1;
  pac_dir_ = ActionStay;
  steps_ = 0;
  lives_ = LifeCount;
  score_ = 0;
  done_ = false;
  won_ = false;
  PlaceGhosts();
}

FPC PacWorldEnv::ComputeReward(Crabs* crabs, EnvGoal achieved_goal,
                               EnvGoal desired_goal, const CHA* info) {
  (void)crabs; (void)achieved_goal; (void)desired_goal; (void)info;
  // The per-step scalar reward is returned by Step(); ComputeReward is the
  // iGeek Env interface hook — return the last step's reward stored in score.
  return (FPC)score_;
}

/* Fill obs_ — the caller must pass a buffer of ObsLength floats. Here we
   write into a static-ish per-env buffer returned via a friend; but the
   cleanest no-stdlib path is: Observe() writes into the env's own buffer and
   the Gym copies it out. We store obs in a member (allocated in ctor). */
// (See the .h — obs_ is accessed by the Gym via a dedicated method.)

void PacWorldEnv::Observe() {
  // obs_ is a member buffer (FPC[ObsLength]) — declared in the .h as part of
  // the env's POD state. We fill it here.
  // Clear.
  for (ISC i = 0; i < ObsLength; ++i) obs_[i] = 0.0f;
  for (ISC dy = -ObsRadius; dy <= ObsRadius; ++dy) {
    for (ISC dx = -ObsRadius; dx <= ObsRadius; ++dx) {
      ISC wx = Clamp(pac_x_ + dx, 0, cols_ - 1);
      ISC wy = Clamp(pac_y_ + dy, 0, rows_ - 1);
      ISC cell = grid_[wy * cols_ + wx];
      // Window index: (dy+R) * (2R+1) + (dx+R).
      ISC widx = (dy + ObsRadius) * (2 * ObsRadius + 1) + (dx + ObsRadius);
      // Channel 0: wall.
      if (cell == CellWall) obs_[widx * ObsChannels + 0] = 1.0f;
      // Channel 1: dot.
      if (cell == CellDot) obs_[widx * ObsChannels + 1] = 1.0f;
      // Channel 2: superdot.
      if (cell == CellSuperDot) obs_[widx * ObsChannels + 2] = 1.0f;
      // Channel 3: bonus.
      if (cell == CellBonus) obs_[widx * ObsChannels + 3] = 1.0f;
      // Channel 6: pac (self, always at window center).
      if (dx == 0 && dy == 0) obs_[widx * ObsChannels + 6] = 1.0f;
    }
  }
  // Ghost channels (4 = strong ghost, 5 = weak ghost).
  for (ISC g = 0; g < ghost_count_; ++g) {
    ISC ddx = ghost_x_[g] - pac_x_;
    ISC ddy = ghost_y_[g] - pac_y_;
    if (ddx < -ObsRadius || ddx > ObsRadius ||
        ddy < -ObsRadius || ddy > ObsRadius) continue;
    ISC widx = (ddy + ObsRadius) * (2 * ObsRadius + 1) + (ddx + ObsRadius);
    if (ghost_weak_[g] > 0) obs_[widx * ObsChannels + 5] = 1.0f;
    else obs_[widx * ObsChannels + 4] = 1.0f;
  }
  // Progress feature (last): steps / MaxSteps.
  obs_[ObsLength - 1] = (FPC)steps_ / (FPC)MaxSteps;
}

/* Greedy ghost chase: pick the legal (non-wall) direction that minimizes
   Manhattan distance to pac; no reversal. Tiebreak by rng_. */
void PacWorldEnv::MoveGhost(ISC g) {
  if (ghost_weak_[g] > 0) { --ghost_weak_[g]; }
  const ISC dirs[5][2] = {
    {0, 0}, {0, -1}, {0, 1}, {-1, 0}, {1, 0}  // Stay,Up,Down,Left,Right
  };
  ISC best_dir = ghost_dir_[g];
  ISC best_dist = 0x7FFFFFFF;
  BOL have_best = false;
  for (ISC d = 1; d < 5; ++d) {  // skip Stay.
    // No reversal: if d is the exact opposite of the current dir, skip.
    if (d == 1 && ghost_dir_[g] == 2) continue;
    if (d == 2 && ghost_dir_[g] == 1) continue;
    if (d == 3 && ghost_dir_[g] == 4) continue;
    if (d == 4 && ghost_dir_[g] == 3) continue;
    ISC nx = ghost_x_[g] + dirs[d][0];
    ISC ny = ghost_y_[g] + dirs[d][1];
    if (IsWall(nx, ny)) continue;
    ISC tx = nx - pac_x_, ty = ny - pac_y_;
    if (tx < 0) tx = -tx;
    if (ty < 0) ty = -ty;
    ISC dist = tx + ty;
    if (!have_best || dist < best_dist) {
      best_dist = dist; best_dir = d; have_best = true;
    }
  }
  if (!have_best) {
    // All blocked: pick any open non-reversal, else stay.
    for (ISC d = 1; d < 5; ++d) {
      if (d == 1 && ghost_dir_[g] == 2) continue;
      if (d == 2 && ghost_dir_[g] == 1) continue;
      if (d == 3 && ghost_dir_[g] == 4) continue;
      if (d == 4 && ghost_dir_[g] == 3) continue;
      if (!IsWall(ghost_x_[g] + dirs[d][0], ghost_y_[g] + dirs[d][1])) {
        best_dir = d; break;
      }
    }
  }
  ghost_dir_[g] = best_dir;
  ghost_x_[g] += dirs[best_dir][0];
  ghost_y_[g] += dirs[best_dir][1];
}

FPC PacWorldEnv::Step(ISC action) {
  if (done_) { Observe(); return 0.0f; }
  action = Clamp(action, ActionStay, ActionRight);
  pac_dir_ = action;
  // Move pac one cell (Stay = no move).
  if (action != ActionStay) {
    ISC dx = 0, dy = 0;
    if (action == ActionUp) dy = -1;
    else if (action == ActionDown) dy = 1;
    else if (action == ActionLeft) dx = -1;
    else if (action == ActionRight) dx = 1;
    ISC nx = pac_x_ + dx, ny = pac_y_ + dy;
    if (!IsWall(nx, ny)) { pac_x_ = nx; pac_y_ = ny; }
  }
  FPC reward = (FPC)StepPenalty;
  // Eat what's under pac.
  {
    ISC idx = pac_y_ * cols_ + pac_x_;
    ISC cell = grid_[idx];
    if (cell == CellDot) { reward += (FPC)DotReward; grid_[idx] = CellEmpty; }
    else if (cell == CellSuperDot) {
      reward += (FPC)SuperDotReward; grid_[idx] = CellEmpty;
      for (ISC g = 0; g < ghost_count_; ++g) ghost_weak_[g] = WeakSteps;
    } else if (cell == CellBonus) { reward += (FPC)BonusReward; grid_[idx] = CellEmpty; }
  }
  // Move ghosts.
  for (ISC g = 0; g < ghost_count_; ++g) MoveGhost(g);
  // Collision check.
  for (ISC g = 0; g < ghost_count_; ++g) {
    if (ghost_x_[g] == pac_x_ && ghost_y_[g] == pac_y_) {
      if (ghost_weak_[g] > 0) {
        // Eat the weak ghost: +100, respawn it.
        reward += (FPC)GhostEatReward;
        score_ += GhostEatReward;
        // Respawn the ghost at a random open cell.
        ISC tries = 0;
        while (tries < 64) {
          ISC rx = 1 + (ISC)(NextRand() % (ISC)(cols_ - 2));
          ISC ry = 1 + (ISC)(NextRand() % (ISC)(rows_ - 2));
          if (!IsWall(rx, ry) && !(rx == pac_x_ && ry == pac_y_)) {
            ghost_x_[g] = rx; ghost_y_[g] = ry; break;
          }
          ++tries;
        }
        ghost_weak_[g] = 0;
      } else {
        // Hit by a strong ghost: lose a life.
        reward += (FPC)CollisionPenalty;
        --lives_;
        // Reset pac + ghosts to start.
        pac_x_ = 1; pac_y_ = 1; pac_dir_ = ActionStay;
        ResetGhosts();
        // Respawn ghosts at their spawn (regenerate layout-free respawn).
        PlaceGhosts();
        if (lives_ <= 0) { done_ = true; won_ = false; Observe(); return reward; }
      }
    }
  }
  ++steps_;
  // Win: all dots eaten.
  {
    BOL any_dot = false;
    for (ISC i = 0; i < cols_ * rows_; ++i)
      if (grid_[i] == CellDot || grid_[i] == CellSuperDot || grid_[i] == CellBonus) { any_dot = true; break; }
    if (!any_dot) { done_ = true; won_ = true; reward += (FPC)WinBonus; }
  }
  if (steps_ >= MaxSteps) { done_ = true; }
  Observe();
  return reward;
}

/* --- PacWorldGym ------------------------------------------------------- */
PacWorldGym::PacWorldGym(ISC envs, ISC cols, ISC rows, IUD seed)
    : Gym("pacworld"), env_count_used_(0) {
  env_count_used_ = envs < 1 ? 1 : (envs > GymEnvMax ? GymEnvMax : envs);
  env_count_ = env_count_used_;
  obs_len_ = PacWorldEnv::ObsLength;
  action_count_ = PacWorldEnv::ActionCount;
  observations_ = new FPC[env_count_ * obs_len_]();
  actions_ = new ISC[env_count_]();
  rewards_ = new FPC[env_count_]();
  terminals_ = new FPC[env_count_]();
  action_mask_ = new FPC[env_count_ * action_count_]();
  for (ISC i = 0; i < env_count_; ++i) {
    envs_[i] = new PacWorldEnv(cols, rows, seed + (IUD)i * 0x9E37u);
    for (ISC j = 0; j < action_count_; ++j)
      action_mask_[i * action_count_ + j] = 1.0f;
  }
  ResetBatch();
}

PacWorldGym::~PacWorldGym() {
  for (ISC i = 0; i < env_count_used_; ++i) {
    if (envs_[i] != NILP) delete envs_[i];
    envs_[i] = NILP;
  }
  if (observations_ != NILP) delete[] observations_;
  if (actions_ != NILP) delete[] actions_;
  if (rewards_ != NILP) delete[] rewards_;
  if (terminals_ != NILP) delete[] terminals_;
  if (action_mask_ != NILP) delete[] action_mask_;
  observations_ = NILP; actions_ = NILP; rewards_ = NILP;
  terminals_ = NILP; action_mask_ = NILP;
}

void PacWorldGym::ResetBatch() {
  for (ISC i = 0; i < env_count_; ++i) {
    envs_[i]->Reset();
    envs_[i]->Observe();
    // Copy the env's obs into the batch array.
    for (ISC j = 0; j < obs_len_; ++j)
      observations_[i * obs_len_ + j] = envs_[i]->Obs()[j];
    rewards_[i] = 0.0f;
    terminals_[i] = envs_[i]->Done() ? 1.0f : 0.0f;
    for (ISC a = 0; a < action_count_; ++a)
      action_mask_[i * action_count_ + a] = 1.0f;
  }
}

void PacWorldGym::StepBatch() {
  for (ISC i = 0; i < env_count_; ++i) {
    ISC a = (actions_ != NILP) ? actions_[i] : 0;
    FPC r = envs_[i]->Step(a);
    rewards_[i] = r;
    terminals_[i] = envs_[i]->Done() ? 1.0f : 0.0f;
    if (envs_[i]->Done()) {
      // Auto-reset a finished env so the batch stays in lockstep.
      envs_[i]->Reset();
    }
    envs_[i]->Observe();
    for (ISC j = 0; j < obs_len_; ++j)
      observations_[i * obs_len_ + j] = envs_[i]->Obs()[j];
    for (ISC aa = 0; aa < action_count_; ++aa)
      action_mask_[i * action_count_ + aa] = 1.0f;
  }
}

}  // namespace _
