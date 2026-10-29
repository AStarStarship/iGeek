// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_MDP_DECL
#define IGEEK_CLASSICAL_MDP_DECL
#include "../ASCIICrabs/_Config.h"
namespace iGeek {

/* Chapter 17 — a Markov decision process over indexed states (0..
 * state_count-1). R(s,a) is the reward earned by taking action a from
 * state s (the book's Figure 17.4 form uses a state-only reward; an
 * action-dependent reward subsumes it). States with IsTerminal true have
 * no transitions and zero value. */
struct MDP {
  ISC state_count;
  ISC action_count;
  FPC gamma;  //< Discount factor (0 <= gamma < 1).
  FPC tol;    //< eps — maximum allowed error in the utility of any state.
  /* True when state is terminal. */
  BOL (*IsTerminal)(ISC state);
  /* Reward earned taking action from state. */
  FPC (*Reward)(ISC state, IUC action);
  /* Transition model: writes the successors of (state, action) into
   * out_states with probabilities out_probs (count <= cap, summing to
   * 1); returns the count. */
  ISC (*Transitions)(ISC state, IUC action, ISC* out_states, ISC cap,
                     FPC* out_probs, ISC prob_cap);
};

/* Figure 17.4 value iteration (full max backup) and Figure 17.7 policy
 * iteration. V and policy are state_count words owned by the caller;
 * V is overwritten in place (its initial values are ignored) and policy
 * is filled greedily with respect to the final V. The termination test
 * is Equation (17.8): delta < eps * (1-gamma) / (1+gamma).
 * Returns the number of outer iterations run (max_iters when the
 * bound was exhausted without convergence). */
ISC ValueIteration(const MDP* mdp, FPC* V, FPC* policy, ISC max_iters);
ISC PolicyIteration(const MDP* mdp, FPC* V, FPC* policy, ISC max_iters);


// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// Figure 17.9. Reference only: no C++ implementation yet.

/* Figure 17.9 A high-level sketch of the value iteration algorithm for POMDPs.
The REMOVE-DOMINATED-PLANS step and MAX-DIFFERENCE test are typically
implemented as linear programs.

@code
function POMDP-VALUE-ITERATION(pomdp, ε) returns a utility function
    inputs: pomdp, a POMDP with states S, actions A(s), transition model P(s′ |
s, a), sensor model P(e | s), rewards R(s), discount γ ε, the maximum error
allowed in the utility of any state local variables: U, U′, sets of plans p with
associated utility vectors αp

    U′←a set containing just the empty plan [ ], with α[ ](s)= R(s)
    repeat
        U ←U′
        U′←the set of all plans consisting of an action and, for each possible
next percept, a plan in U with utility vectors computed according to Equation
(17.13) U′←REMOVE-DOMINATED-PLANS(U′) until MAX-DIFFERENCE(U,U′) < ε(1 − γ)/γ
    return U
@endcode
*/

} //< namespace iGeek
#endif
