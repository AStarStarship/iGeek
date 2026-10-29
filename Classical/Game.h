// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_GAME_DECL
#define IGEEK_CLASSICAL_GAME_DECL
#include "../ASCIICrabs/_Config.h"
namespace iGeek {

/* Chapter 5 — a deterministic two-player game over word-array states.
 * The MAX player moves at the initial state and players alternate after
 * that. Utility is only read at terminal states (positive favors MAX).
 * All pointers and callbacks are world-owned and must outlive the call. */
struct Game {
  const IUC* initial;  //< Initial state (state_words words).
  ISC state_words;     //< State width in IUC words.
  /* True when state is a terminal state. */
  BOL (*IsTerminal)(const IUC* state, ISC state_words);
  /* Writes up to cap actions into out; returns the count written. */
  ISC (*Actions)(const IUC* state, ISC state_words, IUC* out, ISC cap);
  /* Writes the successor of state under action into out. */
  void (*Result)(const IUC* state, ISC state_words, IUC action, IUC* out);
  /* Terminal-state utility (positive favors MAX). */
  FPC (*Utility)(const IUC* state, ISC state_words);
};

/* Figures 5.3/5.7 minimax with alpha-beta pruning. Returns the best
 * action at the initial state (first best on ties; 0 when there are no
 * actions). out_nodes is an optional counter of evaluated states
 * (NILP is fine). MinimaxAction passes -inf/+inf bounds, which is
 * plain minimax; MinimaxAlphaBetaAction is the same routine with
 * pruning enabled — with utility values below kGameUtilityInf the
 * decisions are identical, only the node count differs. */
IUC MinimaxAction(const Game* game, ISC* out_nodes);
IUC MinimaxAlphaBetaAction(const Game* game, ISC* out_nodes);

} //< namespace iGeek
#endif
