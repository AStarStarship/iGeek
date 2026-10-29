// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_GAME_IMPL
#define IGEEK_CLASSICAL_GAME_IMPL
#include "Game.h"
#include "../ASCIICrabs/Binary.hpp"
#include "../ASCIICrabs/Autoject.hxx"
#include "../ASCIICrabs/Array.hpp"
namespace iGeek {

// --- Internal machinery (internal linkage; not module API) ----------------
//
// All RAM is created and destroyed through the RAMFactory
// (ObjectFactoryHeap — the same factory BigInt and AgentState use):
//   alloc: ObjectFactoryHeap(NILP, bytes)  free: ObjectFactoryHeap(origin,
//   RAMFactoryDelete).  Fixed-capacity action scratch uses AArray
// (stack-first, RAMFactory-managed under its Autoboofer). No naked
// new/delete/malloc in this folder.

// Per-node action-list capacity for expansion (world branching above this
// is a contract violation; the excess actions are dropped).
static const ISC kGameBranch = 256;

// Finite infinity for the minimax bounds (the book's -inf/+inf). Utility
// magnitudes above this saturate the pruning bounds.
static const FPC kGameUtilityInf = FPC(1e30f);

/* Returns a RAMFactory block to the factory (a no-op for NILP origins). */
static inline IUW* GameFree(IUW* origin) {
  return ObjectFactoryHeap(origin, RAMFactoryDelete);
}

/* One state row per depth level: the state currently being examined at that depth. The row at level L+1 is written by Result before the recursion descends; siblings overwrite it on backtracking. */
struct GameStateStack {
  IUW* mem;
  ISC cap;
  ISC state_words;
};

static BOL GameStackEnsure(GameStateStack* stk, ISC level) {
  if (level < stk->cap) {
    return true;
  }
  ISC new_cap = stk->cap < 4 ? 4 : stk->cap * 2;
  if (new_cap <= level) {
    new_cap = level + 1;
  }
  IUW* nm = ObjectFactoryHeap(
      NILP, IUD(new_cap) * IUD(stk->state_words) * IUD(sizeof(IUC)));
  if (IsError(nm)) {
    return false;
  }
  IUC* nd = TPtr<IUC>(nm);
  const IUC* od = TPtr<IUC>(stk->mem);
  for (ISC i = 0; i < stk->cap; ++i) {
    for (ISC w = 0; w < stk->state_words; ++w) {
      nd[IUD(i) * stk->state_words + w] = od[IUD(i) * stk->state_words + w];
    }
  }
  stk->mem = GameFree(stk->mem);
  stk->mem = nm;
  stk->cap = new_cap;
  return true;
}

/* Figures 5.3/5.7 minimax with alpha-beta pruning. `prune` false passes
-inf/+inf bounds and never cuts off (plain minimax); `prune` true cuts
off on beta <= alpha. Best action on ties is the first one encountered.
A non-terminal state with no actions scores 0.0 (documented). */
static IUC GameMinimaxRec(const Game* game, ISC level, GameStateStack* stk,
                          const IUC* state, FPC alpha, FPC beta,
                          ISC* out_nodes, FPC* value_out, BOL prune,
                          IUC* best_action_out) {
  if (out_nodes != NILP) {
    ++(*out_nodes);
  }
  if (game->IsTerminal(state, game->state_words)) {
    *value_out = game->Utility(state, game->state_words);
    *best_action_out = 0;
    return 0;
  }
  AArray<IUC, ISN, kGameBranch> acts;
  ISC act_count =
      game->Actions(state, game->state_words, acts.Begin(), kGameBranch);
  if (act_count <= 0) {
    *value_out = FPC(0.0f);
    *best_action_out = 0;
    return 0;
  }
  BOL is_max = (level % 2) == 0;
  FPC value = is_max ? FPC(-kGameUtilityInf) : FPC(kGameUtilityInf);
  IUC best = 0;
  for (ISC k = 0; k < act_count; ++k) {
    IUC action = acts[k];
    if (!GameStackEnsure(stk, level + 1)) {
      // OOM: report the value accumulated so far.
      break;
    }
    IUC* child =
        TPtr<IUC>(stk->mem) + IUD(level + 1) * IUD(stk->state_words);
    game->Result(state, game->state_words, action, child);
    FPC child_value = FPC(0.0f);
    IUC child_best = 0;
    GameMinimaxRec(game, level + 1, stk, child, alpha, beta, out_nodes,
                   &child_value, prune, &child_best);
    if (is_max) {
      if (child_value > value) {
        value = child_value;
        best = action;
      }
      if (prune && value > alpha) {
        alpha = value;
      }
      if (prune && beta <= alpha) {
        break;
      }
    } else {
      if (child_value < value) {
        value = child_value;
        best = action;
      }
      if (prune && value < beta) {
        beta = value;
      }
      if (prune && beta <= alpha) {
        break;
      }
    }
  }
  *value_out = value;
  *best_action_out = best;
  return best;
}

static IUC MinimaxRoot(const Game* game, ISC* out_nodes, BOL prune) {
  if (out_nodes != NILP) {
    *out_nodes = 0;
  }
  GameStateStack stk;
  stk.mem = NILP;
  stk.cap = 0;
  stk.state_words = game->state_words < 1 ? 1 : game->state_words;
  if (!GameStackEnsure(&stk, 0)) {
    return 0;
  }
  FPC value = FPC(0.0f);
  IUC best = 0;
  GameMinimaxRec(game, 0, &stk, game->initial, FPC(-kGameUtilityInf),
                 FPC(kGameUtilityInf), out_nodes, &value, prune, &best);
  stk.mem = GameFree(stk.mem);
  return best;
}

IUC MinimaxAction(const Game* game, ISC* out_nodes) {
  return MinimaxRoot(game, out_nodes, false);
}

IUC MinimaxAlphaBetaAction(const Game* game, ISC* out_nodes) {
  return MinimaxRoot(game, out_nodes, true);
}

} //< namespace iGeek
#endif
