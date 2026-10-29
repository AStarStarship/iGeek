// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_SEARCH_DECL
#define IGEEK_CLASSICAL_SEARCH_DECL
#include "../ASCIICrabs/_Config.h"
namespace iGeek {

/* Figure 3.12 SEARCH-PROBLEM — a generic search problem over word-array
 * states. A state is `state_words` IUC words. All pointers and callbacks
 * are world-owned and must outlive the call. The searchers are planners
 * and reference drivers, not tick-path code: they allocate a node pool
 * on the heap and free it on return. */
struct Problem {
  const IUC* initial;  //< Initial state (state_words words).
  ISC state_words;     //< State width in IUC words.
  /* True when state is a goal state. */
  BOL (*GoalTest)(const IUC* state, ISC state_words);
  /* Writes up to cap actions into out; returns the count written. */
  ISC (*Actions)(const IUC* state, ISC state_words, IUC* out, ISC cap);
  /* Writes the successor of state under action into out. */
  void (*Result)(const IUC* state, ISC state_words, IUC action, IUC* out);
  /* Step cost of action from state (NILP => unit cost, UCS falls back). */
  FPC (*StepCost)(const IUC* state, ISC state_words, IUC action);
  /* Heuristic h(state), admissible for RBFS (NILP => RBFS unavailable). */
  FPC (*Heuristic)(const IUC* state, ISC state_words);
};

/* Searcher return values:
 *   n >= 0 — n actions written to path[0..n-1] (0: initial is a goal).
 *   -1     — no solution within the bound (or the path is longer than
 *            path_cap).
 *   -2     — cutoff at the maximum depth (DLS/IDS only).
 * Paths are written as the action sequence from the initial state to a
 * goal; applying them through Result reaches the goal state. */
ISC SearchBFS(const Problem* problem, IUC* path, ISC path_cap);
ISC SearchUCS(const Problem* problem, IUC* path, ISC path_cap);
ISC SearchDLS(const Problem* problem, IUC* path, ISC path_cap, ISC limit);
ISC SearchIDS(const Problem* problem, IUC* path, ISC path_cap, ISC max_limit);
ISC SearchRBFS(const Problem* problem, IUC* path, ISC path_cap);

} //< namespace iGeek
#endif
