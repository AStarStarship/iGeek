// Copyright AStarship <https://astarship.net>.
//
// Classical Search seam unit: graph search (BFS, UCS, IDS, DLS, RBFS).
// Gated on IGEEK_CLASSICAL_SEARCH.
#if SEAM >= IGEEK_CLASSICAL_CORE
#if SEAM >= IGEEK_CLASSICAL_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
#include "Search.hxx"
#include <cstdio>
#include <cmath>
namespace iGeek {

static int s_pass = 0;
static int s_fail = 0;

static void SCHECK(const char* name, BOL ok) {
  if (ok) {
    s_pass++;
    D_COUT("PASS " << name << '\n');
  } else {
    s_fail++;
    D_COUT("FAIL " << name << '\n');
  }
}

/* A simple number-state problem: state is a single IUC (0..goal).
   Actions: 0 = +1, 1 = -2 (if state > 0). Goal: state == goal_val.
   Unit cost. This gives a deterministic, testable search space. */
static ISC s_goal_val = 5;

static BOL SGoalTest(const IUC* state, ISC state_words) {
  (void)state_words;
  return state[0] == IUC(s_goal_val);
}

static ISC SActions(const IUC* state, ISC state_words, IUC* out, ISC cap) {
  (void)state_words;
  ISC n = 0;
  // Action 0: +1 (always available).
  if (n < cap) out[n++] = 0;
  // Action 1: -2 (only if state > 0).
  if (state[0] > 0 && n < cap) out[n++] = 1;
  return n;
}

static void SResult(const IUC* state, ISC state_words, IUC action, IUC* out) {
  (void)state_words;
  if (action == 0) {
    out[0] = state[0] + 1;  // +1
  } else {
    out[0] = state[0] - 2;  // -2
  }
}

static FPC SStepCost(const IUC* state, ISC state_words, IUC action) {
  (void)state;
  (void)state_words;
  (void)action;
  return FPC(1.0f);  // unit cost
}

/* Heuristic: distance to goal (admissible for this problem). */
static FPC SHeuristic(const IUC* state, ISC state_words) {
  (void)state_words;
  FPC dist = FPC(s_goal_val) - FPC(state[0]);
  return dist < FPC(0.0f) ? -dist : dist;
}

inline const CHA* Search(const CHA* args) {
#if SEAM >= IGEEK_CLASSICAL_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing Classical Search (BFS/UCS/IDS/DLS/RBFS)..."));

  IUC initial[1] = {0};
  Problem problem;
  problem.initial = initial;
  problem.state_words = 1;
  problem.GoalTest = SGoalTest;
  problem.Actions = SActions;
  problem.Result = SResult;
  problem.StepCost = SStepCost;
  problem.Heuristic = SHeuristic;

  // --- BFS: shortest path (unit cost) from 0 to 5.
  // Path: 0->1->2->3->4->5 (5 steps, all +1).
  {
    IUC path[32];
    ISC n = SearchBFS(&problem, path, 32);
    SCHECK("BFS: found path", n >= 0);
    SCHECK("BFS: path length 5", n == 5);
    if (n == 5) {
      // Verify the path reaches the goal.
      IUC state[1] = {0};
      for (ISC i = 0; i < n; ++i) {
        IUC next[1];
        problem.Result(state, 1, path[i], next);
        state[0] = next[0];
      }
      SCHECK("BFS: path reaches goal", SGoalTest(state, 1));
    }
  }

  // --- UCS: same as BFS for unit cost.
  {
    IUC path[32];
    ISC n = SearchUCS(&problem, path, 32);
    SCHECK("UCS: found path", n >= 0);
    SCHECK("UCS: path length 5", n == 5);
  }

  // --- IDS: iterative deepening, max_limit 10 (plenty).
  {
    IUC path[32];
    ISC n = SearchIDS(&problem, path, 32, 10);
    SCHECK("IDS: found path", n >= 0);
    SCHECK("IDS: path length 5", n == 5);
  }

  // --- DLS: depth-limited, limit 5 (exactly the path length).
  {
    IUC path[32];
    ISC n = SearchDLS(&problem, path, 32, 5);
    SCHECK("DLS(limit=5): found path", n >= 0);
    SCHECK("DLS(limit=5): path length 5", n == 5);
  }

  // --- DLS: depth-limited, limit 3 (too short -> no solution).
  {
    IUC path[32];
    ISC n = SearchDLS(&problem, path, 32, 3);
    SCHECK("DLS(limit=3): no solution within bound", n < 0);
  }

  // --- RBFS: recursive backtracking with heuristic.
  {
    IUC path[32];
    ISC n = SearchRBFS(&problem, path, 32);
    SCHECK("RBFS: found path", n >= 0);
    if (n >= 0) {
      IUC state[1] = {0};
      for (ISC i = 0; i < n; ++i) {
        IUC next[1];
        problem.Result(state, 1, path[i], next);
        state[0] = next[0];
      }
      SCHECK("RBFS: path reaches goal", SGoalTest(state, 1));
    }
  }

  // --- No solution: goal unreachable (goal_val = -1, can't reach from 0
  //     with +1/-2).
  {
    s_goal_val = -1;  // unreachable (state is always >= 0 with +1/-2 from 0).
    // Actually, -2 can go below 0... let's use a goal that's truly
    // unreachable. With +1 and -2, from 0 we can reach any integer
    // (0, 1, -1, 2, 0, -2, 3, ...). So every integer is reachable.
    // Instead, test path_cap too small.
    s_goal_val = 5;  // restore.
    IUC path[2];  // cap = 2, but the path needs 5.
    ISC n = SearchBFS(&problem, path, 2);
    SCHECK("BFS: path_cap too small -> no solution", n < 0);
  }

  D_COUT("s_pass=" << s_pass << " s_fail=" << s_fail << '\n');
  return s_fail ? "CLASSICAL_SEARCH_FAIL: " : NILP;

#endif
  return NILP;
}

}  //< namespace iGeek
