// Copyright AStarship <https://astarship.net>.
//
// Classical Mdp seam unit: value iteration + policy iteration.
// Gated on IGEEK_CLASSICAL_MDP.
#if SEAM >= IGEEK_CLASSICAL_CORE
#if SEAM >= IGEEK_CLASSICAL_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
#include "Mdp.hxx"
#include <cstdio>
#include <cmath>
namespace iGeek {

static int m_pass = 0;
static int m_fail = 0;

static void MCHECK(const char* name, BOL ok) {
  if (ok) {
    m_pass++;
    D_COUT("PASS " << name << '\n');
  } else {
    m_fail++;
    D_COUT("FAIL " << name << '\n');
  }
}

static void MFLOAT(const char* name, FPC got, FPC want, FPC tol) {
  if (std::abs(got - want) <= tol) {
    m_pass++;
    D_COUT("PASS " << name << " got=" << got << '\n');
  } else {
    m_fail++;
    D_COUT("FAIL " << name << " got=" << got << " want=" << want << '\n');
  }
}

/* A simple 3-state MDP:
   State 0: action 0 -> state 1 (reward 1), action 1 -> state 1 (reward 0).
   State 1: action 0 -> state 2 (reward 1), action 1 -> state 1 (reward -1).
   State 2: terminal.
   gamma = 0.9.
   Optimal policy: 0: action 0 (reward 1 > 0), 1: action 0 (reward 1 > -1).
   Optimal values: V(2)=0, V(1)=1+0.9*0=1, V(0)=1+0.9*1=1.9. */

static BOL MIsTerminal(ISC state) { return state == 2; }

/* Next state for (state, action) — deterministic. */
static ISC MNext(ISC state, IUC action) {
  if (state == 0) return 1;  // both actions go to state 1.
  if (state == 1) {
    return action == 0 ? 2 : 1;  // action 0 -> terminal, action 1 -> stay.
  }
  return 2;  // terminal.
}

/* Reward for (state, action). */
static FPC MReward2(ISC state, IUC action) {
  if (state == 0) return action == 0 ? FPC(1.0f) : FPC(0.0f);
  if (state == 1) return action == 0 ? FPC(1.0f) : FPC(-1.0f);
  return FPC(0.0f);  // terminal.
}

/* Transition: deterministic. Writes 1 successor with prob 1.0. */
static ISC MTransitions(ISC state, IUC action, ISC* out_states, ISC cap,
                        FPC* out_probs, ISC prob_cap) {
  (void)cap;
  (void)prob_cap;
  if (state == 2) return 0;  // terminal: no transitions.
  out_states[0] = MNext(state, action);
  out_probs[0] = FPC(1.0f);
  return 1;
}

inline const CHA* Mdp(const CHA* args) {
#if SEAM >= IGEEK_CLASSICAL_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing Classical Mdp (ValueIteration/PolicyIteration)..."));

  MDP mdp;
  mdp.state_count = 3;
  mdp.action_count = 2;
  mdp.gamma = FPC(0.9f);
  mdp.tol = FPC(1e-4f);
  mdp.IsTerminal = MIsTerminal;
  mdp.Reward = MReward2;
  mdp.Transitions = MTransitions;

  // --- ValueIteration: should converge to V(0)=1.9, V(1)=1.0, V(2)=0.
  {
    FPC V[3] = {0, 0, 0};
    FPC policy[3] = {0, 0, 0};
    ISC iters = ValueIteration(&mdp, V, policy, 100);
    MCHECK("ValueIteration: converged within 100 iters", iters < 100);
    MFLOAT("ValueIteration: V(0)=1.9", V[0], FPC(1.9f), FPC(0.01f));
    MFLOAT("ValueIteration: V(1)=1.0", V[1], FPC(1.0f), FPC(0.01f));
    MFLOAT("ValueIteration: V(2)=0.0", V[2], FPC(0.0f), FPC(0.001f));
    // Policy: state 0 -> action 0, state 1 -> action 0.
    MCHECK("ValueIteration: policy(0)=0", IUC(policy[0]) == 0);
    MCHECK("ValueIteration: policy(1)=0", IUC(policy[1]) == 0);
  }

  // --- PolicyIteration: should converge to the same values/policy.
  {
    FPC V[3] = {0, 0, 0};
    FPC policy[3] = {0, 0, 0};
    ISC iters = PolicyIteration(&mdp, V, policy, 100);
    MCHECK("PolicyIteration: converged within 100 iters", iters < 100);
    MFLOAT("PolicyIteration: V(0)=1.9", V[0], FPC(1.9f), FPC(0.01f));
    MFLOAT("PolicyIteration: V(1)=1.0", V[1], FPC(1.0f), FPC(0.01f));
    MFLOAT("PolicyIteration: V(2)=0.0", V[2], FPC(0.0f), FPC(0.001f));
    MCHECK("PolicyIteration: policy(0)=0", IUC(policy[0]) == 0);
    MCHECK("PolicyIteration: policy(1)=0", IUC(policy[1]) == 0);
  }

  D_COUT("m_pass=" << m_pass << " m_fail=" << m_fail << '\n');
  return m_fail ? "CLASSICAL_MDP_FAIL: " : NILP;

#endif
  return NILP;
}

}  //< namespace iGeek
