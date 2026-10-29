// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_RL_DECL
#define IGEEK_CLASSICAL_RL_DECL
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// The implementations below follow these figures.

/* Figure 21.2 A passive reinforcement learning agent based on adaptive dynamic
programming. The POLICY-EVALUATION function solves the fixed-policy Bellman
equations, as described on page 657.

@code
function PASSIVE-ADP-AGENT(percept ) returns an action
    inputs: percept , a percept indicating the current state s′ and reward
signal r ′ persistent: π, a fixed policy mdp, an MDP with model P, rewards R,
discount γ U, a table of utilities, initially empty Nsa , a table of frequencies
for state–action pairs, initially zero Ns′|sa , a table of outcome frequencies
given state–action pairs, initially zero s, a, the previous state and action,
initially null

    if s′ is new then U[s′]←r ′; R[s′]←r ′
    if s is not null then
        increment Nsa [s, a] and Ns′|sa [s′, s, a]
        for each t such that Ns′|sa [t , s, a] is nonzero do
            P(t | s, a)←Ns′|sa [t , s, a] / Nsa [s, a]
    U ←POLICY-EVALUATION(π,U,mdp)
    if s′.TERMINAL? then s, a←null else s, a←s′, π[s′]
    return a
@endcode
*/

/* Figure 21.4 A passive reinforcement learning agent that learns utility
estimates using temporal differences. The step-size function α(n) is chosen to
ensure convergence, as described in the text.

@code
function PASSIVE-TD-AGENT(percept ) returns an action
    inputs: percept , a percept indicating the current state s′ and reward
signal r ′ persistent: π, a fixed policy U, a table of utilities, initially
empty Ns, a table of frequencies for states, initially zero s, a, r , the
previous state, action, and reward, initially null

    if s′ is new then U[s′]←r ′
    if s is not null then
        increment Ns[s]
        U[s]←U[s] + α(Ns[s])(r + γ U[s′] − U[s])
    if s′.TERMINAL? then s, a, r ←null else s, a, r ←s′, π[s′], r ′
    return a
@endcode
*/

/* Figure 21.8 An exploratory Q-learning agent. It is an active learner that
learns the value Q(s, a) of each action in each situation. It uses the same
exploration function f as the exploratory ADP agent, but avoids having to learn
the transition model because the Q-value of a state can be related directly to
those of its neighbors.

@code
function Q-LEARNING-AGENT(percept ) returns an action
    inputs: percept , a percept indicating the current state s′ and reward
signal r ′ persistent: Q, a table of action values indexed by state and action,
initially zero Nsa , a table of frequencies for state–action pairs, initially
zero s, a, r , the previous state, action, and reward, initially null

    if TERMINAL?(s) then Q[s,None]←r ′
    if s is not null then
        increment Nsa [s, a]
        Q[s, a]←Q[s, a] + α(Nsa [s, a])(r + γ maxa′ Q[s′, a′] − Q[s, a])
    s, a, r ←s′, argmaxa′ f(Q[s′, a′],Nsa [s′, a′]), r ′
    return a
@endcode
*/


/* Tabular reinforcement-learning agents over discrete states
(ISC indices 0..state_count-1) and actions (IUC, 0..action_count-1).
A percept is (s_next, r): the state entered and the reward received.
Each agent's Percept() applies the book's update for the previous
(s, a, r) triple and returns the action to take in s_next; when s_next
is terminal it returns the sentinel action_count (the book's null
action — no action is taken in a terminal state).

All tables are RAMFactory-owned (ObjectFactoryHeap), allocated by Init
and returned by the destructor. The step size is a constant alpha
(the book's alpha(n) is any schedule satisfying the convergence
conditions; constant is standard).

The three agents (implemented in RL.hxx, following the relocated AIMA
figures):
  PassiveTDAgent  — Fig 21.4  fixed policy, utility table, TD update
  QAgent          — Fig 21.8  active learner, Q table, eps-greedy
   PassiveADPAgent — Fig 21.2  learns the transition model, then runs
                             POLICY-EVALUATION over it every step */

/* Fig 21.4: passive learner with a fixed policy. Persistent state: U
 (utilities), Ns (state visit counts), and the previous s, a, r triple. */
struct PassiveTDAgent {
  Autoject u_mem_;
  Autoject n_mem_;
  ISC state_count_;
  ISC action_count_;
  FPC gamma_;
  FPC alpha_;
  BOL (*IsTerminal_)(ISC state);
  IUC (*Policy_)(ISC state);
  ISC s_prev_;
  IUC a_prev_;
  FPC r_prev_;
  PassiveTDAgent();
  ~PassiveTDAgent();
  BOL Init(ISC state_count, ISC action_count, FPC gamma, FPC alpha,
           BOL (*IsTerminal)(ISC state), IUC (*Policy)(ISC state));
  IUC Percept(ISC s_next, FPC r);
  FPC Utility(ISC state) const;
  void Reset();
};

/* Fig 21.8: exploratory Q-learning agent. Q is indexed [state][action];
  Nsa counts state-action visits. Exploration is eps-greedy (seeded LCG):
  with probability eps a uniform random action, otherwise the first
  the book's exploration function f (Fig 21.10) is not in
  Agent.h; eps-greedy is the standard stand-in. Like the TD agent, the
  update uses r_prev_ (the reward received for the previous action); when
  s_next is terminal it bootstraps with 0 (target = r_prev_, no gamma
  term). */
struct QAgent {
  Autoject q_mem_;
  Autoject n_mem_;
  ISC state_count_;
  ISC action_count_;
  FPC gamma_;
  FPC alpha_;
  FPC eps_;
  BOL (*IsTerminal_)(ISC state);
  IUD rnd_state_;
  ISC s_prev_;
  IUC a_prev_;
  FPC r_prev_;
  QAgent();
  ~QAgent();
  BOL Init(ISC state_count, ISC action_count, FPC gamma, FPC alpha, FPC eps,
           BOL (*IsTerminal)(ISC state), IUD seed);
  IUC Percept(ISC s_next, FPC r);
  FPC Q(ISC state, IUC action) const;
  void Reset();
};

/* Fig 21.2: passive adaptive-dynamic-programming agent. Persistent state:
U (utilities), R (state rewards — the book's per-(a,s,s') R is condensed
to a per-state sample mean, updated on every departure from the state),
the fixed policy, Nsa (state-action counts), Ns'|sa (outcome counts), and
the learned model P(t | s, a). Every step re-derives P from the counts and
re-solves the fixed-policy Bellman equations (POLICY-EVALUATION) to tol. */
struct PassiveADPAgent {
  Autoject u_mem_;
  Autoject r_mem_;
  Autoject nsa_mem_;
  Autoject np_mem_;
  Autoject p_mem_;
  Autoject u_work_;
  ISC state_count_;
  ISC action_count_;
  FPC gamma_;
  FPC tol_;
  BOL (*IsTerminal_)(ISC state);
  IUC (*Policy_)(ISC state);
  ISC s_prev_;
  IUC a_prev_;
  FPC r_prev_;
  PassiveADPAgent();
  ~PassiveADPAgent();
  BOL Init(ISC state_count, ISC action_count, FPC gamma, FPC tol,
           BOL (*IsTerminal)(ISC state), IUC (*Policy)(ISC state));
  IUC Percept(ISC s_next, FPC r);
  FPC Utility(ISC state) const;
  void Reset();
};

} //< namespace iGeek
#endif
