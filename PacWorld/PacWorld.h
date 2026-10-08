// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_PACWORLD_DECL
#define IGEEK_PACWORLD_DECL
#include <_Config.h>
#include "../Env.h"
#include "../Gym.h"
#include "../Tensor.h"
namespace _ {

/* Headless Pac-Man world for agent training.
   Ported from the SFML iGeekPacWorld game rules to the no-stdlib ASCIICrabs
   type system, grid-based + discrete (one cell per action) so the PPO loop
   can step it deterministically. No SFML, no std containers.

   Rules (from the original PacWorld):
     - Grid maze of cells: Wall / Empty / Dot / SuperDot / Bonus.
     - Pac starts at the marked cell; N ghosts at the ghost cells.
     - Pac moves one cell per step in a chosen direction; turning is only
       possible at a cell (no sub-cell motion in the headless version).
     - Eating a Dot +5, SuperDot +25 (ghosts become weak for WeakSteps),
       Bonus +500.
     - Ghosts: at each step they move one cell toward pac (greedy: pick the
       legal direction minimizing Manhattan distance to pac; no reversal).
       A weak ghost is edible: pac eating it scores +100 and respawns it.
     - Collision with a non-weak ghost: pac loses a life; at 0 lives the
       episode is over (terminal). All dots eaten: episode over (won).
     - Fixed step budget (MaxSteps) also terminates.

   Observation (fixed length, Puffer "scalar vector"): a (ObsRadius*2+1)^2
   local window around pac. Each cell is one-hot over {wall, dot, superdot,
   bonus, ghost, weakghost, pac}. That's 7 channels * (2R+1)^2 cells. With
   R=3 -> 7*49 = 343 features. Plus 1 "episode-progress" feature
   (steps/MaxSteps). obs_len = 344.
   Action space: 5 (Stay, Up, Down, Left, Right).
*/
class PacWorldEnv {
 public:
  enum {
    ObsRadius = 3,                 //< Local window radius around pac.
    ObsCells = (2 * ObsRadius + 1) * (2 * ObsRadius + 1),  // 49
    ObsChannels = 7,               //< wall,dot,super,bonus,ghost,weak,pac
    ObsLength = ObsCells * ObsChannels + 1,  // 343 + 1 progress
    ActionCount = 5,               //< Stay,Up,Down,Left,Right
    ActionStay = 0,
    ActionUp = 1,
    ActionDown = 2,
    ActionLeft = 3,
    ActionRight = 4,
    CellWall = 0,
    CellEmpty = 1,
    CellDot = 2,
    CellSuperDot = 3,
    CellBonus = 4,
    MaxSteps = 200,                //< Episode step budget.
    LifeCount = 3,                 //< Lives per episode.
    WeakSteps = 12,                //< Ghosts weak after a superdot.
    DotReward = 5,
    SuperDotReward = 25,
    BonusReward = 500,
    GhostEatReward = 100,
    CollisionPenalty = -10,
    WinBonus = 50,
  };
  static constexpr FPC StepPenalty = -0.1f;  //< Per-step cost (encourage speed).

  /* @param cols Grid width (cells). @param rows Grid height.
     @param seed Maze-generation seed (deterministic layout per seed). */
  PacWorldEnv(ISC cols, ISC rows, IUD seed);
  ~PacWorldEnv();

  /* iGeek Env interface. */
  virtual FPC ComputeReward(Crabs* crabs, EnvGoal achieved_goal,
                            EnvGoal desired_goal, const CHA* info);
  virtual ISC EnvCount() { return 1; }
  virtual void Reset();
  virtual BOL Done() { return done_; }

  /* Number of distinct observations the policy sees (for the policy ctor). */
  ISC ObsLen() const { return ObsLength; }

  /* Fill obs_ (length ObsLength) from the current state. */
  void Observe();

  /* Apply an action (0..4); returns the scalar reward for this step and
     updates obs_. Sets done_ when the episode ends. */
  FPC Step(ISC action);

  /* Grid + agent state access (for the Gym to copy into batch arrays). */
  const IUA* Grid() const { return grid_; }
  ISC Cols() const { return cols_; }
  ISC Rows() const { return rows_; }
  ISC PacX() const { return pac_x_; }
  ISC PacY() const { return pac_y_; }
  ISC GhostX(ISC i) const { return ghost_x_[i]; }
  ISC GhostY(ISC i) const { return ghost_y_[i]; }
  ISC Steps() const { return steps_; }
  ISC Lives() const { return lives_; }
  ISC GhostCount() const { return ghost_count_; }

  /* The observation buffer (length ObsLength), filled by Observe(). The
     Gym copies it into its batch array. */
  const FPC* Obs() const { return obs_; }

 private:
  ISC cols_;
  ISC rows_;
  IUD seed_;
  IUA* grid_;              //< [cols_*rows_] cell type.
  FPC* obs_;               //< [ObsLength] observation buffer.
  ISC pac_x_, pac_y_;      //< Pac cell position.
  ISC pac_dir_;            //< Current pac direction (0..4, 0=stay).
  ISC ghost_x_[16], ghost_y_[16];  //< Ghost positions.
  ISC ghost_dir_[16];      //< Ghost directions.
  IUC ghost_weak_[16];     //< Weakness countdown per ghost.
  ISC ghost_count_;
  ISC steps_;
  ISC lives_;
  ISC score_;
  BOL done_;
  BOL won_;
  IUD rng_;                //< Deterministic per-episode RNG (maze + tiebreaks).

  IUD NextRand();          //< Next uint in [0, 2^32).
  void GenerateMaze();     //< Seeded maze: border walls + random interior.
  void PlaceGhosts();
  void MoveGhost(ISC i);
  ISC Clamp(ISC v, ISC lo, ISC hi) const;
  BOL IsWall(ISC x, ISC y) const;
  void ResetGhosts();
};

/* Vectorized PacWorld Gym for PPO. Batches N parallel PacWorldEnvs and
   exposes the flat obs/actions/rewards/terminals/action-mask arrays the
   TPPO loop steps in one call. Inherits iGeek's Gym (the interface TPPO
   takes); overrides StepBatch()/ResetBatch(). */
class PacWorldGym : public Gym {
 public:
  /* @param envs Number of parallel mazes (1..GymEnvMax).
     @param cols/rows Grid size. @param seed Base seed (each env offset). */
  PacWorldGym(ISC envs, ISC cols, ISC rows, IUD seed);
  ~PacWorldGym() override;

  void ResetBatch() override;
  void StepBatch() override;

  PacWorldEnv& Env(ISC i) { return *envs_[i]; }

 private:
  PacWorldEnv* envs_[GymEnvMax];
  ISC env_count_used_;
};

}  // namespace _
#endif
