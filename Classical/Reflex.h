// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_REFLEX_DECL
#define IGEEK_CLASSICAL_REFLEX_DECL
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// The implementations below follow these figures.

/* Invoked for each new percept and returns an action each time.
Figure 2.7 The TABLE-DRIVEN-AGENT program is invoked for each new percept and
returns an action each time. It retains the complete percept sequence in memory.

Figure 2.2 A vacuum-cleaner world with just two locations.
Percept sequence Action
[A, Clean] Right
[A, Dirty] Suck
[B, Clean] Left
[B, Dirty] Suck
[A, Clean], [A, Clean] Right
[A, Clean], [A, Dirty] Suck
...
...
[A, Clean], [A, Clean], [A, Clean] Right
[A, Clean], [A, Clean], [A, Dirty] Suck
...
...

Figure 2.3 Partial tabulation of a simple agent function for the vacuum-cleaner
world shown in Figure 2.2.

@group TableDrivenAgent

@return An Action
@const percepts A sequence, initially empty
@const table, a table of actions, indexed by percept sequences, initially fully
specified.

@code
append percept to the end of percepts
action <- LOOKUP (percepts, table)
return action
@endcode
*/

/* Figure 2.8 The agent program for a simple reflex agent in the two-state
vacuum environment. This program implements the agent function tabulated in
Figure 2.3. function REFLEX-VACUUM-AGENT([location,status]) returns an action

@code
if (status == Dirty then return Suck
else if location = A then return Right
else if location = B then return Left
@endcode
*/

/* Figure 2.12 A model-based reflex agent. It keeps track of the current state
of the world, using an internal model. It then chooses an action in the same way
as the reflex agent. function SIMPLE-REFLEX-AGENT(percept ) returns an action
persistent: rules, a set of condition–action rules
@code
state←INTERPRET-INPUT(percept )
rule←RULE-MATCH(state, rules)
action ←rule.ACTION
return action
@endcode
*/


/* Vacuum-cleaner world (Fig 2.2): two locations A/B, each Clean or Dirty.
 * Actions: Suck, move A<->B. The book's table-driven agent (Fig 2.7)
 * retains the complete percept sequence and looks up the action; the
 * reflex agent (Fig 2.8) reads only the current [location, status]. */

/* Action codes for the vacuum world. */
static const IUC kVacSuck = 0;
static const IUC kVacRight = 1;  // move A -> B
static const IUC kVacLeft = 2;   // move B -> A

/* Location codes. */
static const IUC kVacLocA = 0;
static const IUC kVacLocB = 1;

/* Fig 2.8 REFLEX-VACUUM-AGENT([location, status]) — pure function, no state.
 * location: kVacLocA/kVacLocB, dirty: current square's dirt flag. */
IUC ReflexVacuumAction(IUC location, BOL dirty);

/* Fig 2.7 TABLE-DRIVEN-AGENT over the vacuum world. The book's table maps
 * each percept sequence to an action; for the vacuum environment the action
 * depends only on the latest percept, so the table is a lookup over packed
 * percept histories of length history_len (percept = (dirty, location),
 * 2 bits each; history_len <= 31). Unpinned entries default to the reflex
 * action of the latest percept (the book's tabulated values). */
struct TTableVacuumAgent {
  Autoject table_mem_;
  ISC history_len_;
  ISC hist_count_;
  IUC* history_;
  TTableVacuumAgent();
  ~TTableVacuumAgent();
  BOL Init(ISC history_len);
  /* Append a percept to the sequence; pin an explicit table entry if the
   * world overrides the book's defaults. */
  void Percept(IUC location, BOL dirty);
  /* LOOKUP percepts in the table; returns the book's action for the
   * sequence (falls back to the reflex action of the last percept). */
  IUC Action();
  /* Override one table entry (packed history) with a fixed action. */
  void PinEntry(IUC packed_history, IUC action);
  void Reset();
};

/* Fig 2.12 SIMPLE-REFLEX-AGENT: state <- INTERPRET-INPUT(percept);
 * rule <- RULE-MATCH(state, rules); return rule.ACTION. `state` is a flat
 * IUC word computed by the world's INTERPRET-INPUT (the world provides it
 * as the percept itself when identity is the right interpretation). Rules
 * are exact word-equal condition->action pairs, first match wins. */
struct TSimpleReflexAgent {
  Autoject rules_mem_;
  Autoject state_mem_;
  ISC state_words_;
  ISC rule_count_;
  ISC rule_cap_;
  BOL seen_;
  TSimpleReflexAgent();
  ~TSimpleReflexAgent();
  BOL Init(ISC state_words, ISC rule_cap);
  /* condition: state_words IUC words (the INTERPRET-INPUT output). */
  BOL AddRule(const IUC* condition, IUC action);
  /* PERCEPT -> action in one call (the book's function). */
  IUC Percept(const IUC* state);
  /* Last matched action, or kVacSuck as the no-rule sentinel. */
  IUC LastAction();
  void Reset();
};

} //< namespace iGeek
#endif
