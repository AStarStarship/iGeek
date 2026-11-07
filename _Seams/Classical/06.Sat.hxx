// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekClassicalSat
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#include "../../Classical/Sat.hxx"
namespace iGeek {
using namespace ::_;
using namespace iGeek; 

static int sat_pass = 0;
static int sat_fail = 0;

static void STCHECK(const char* name, BOL ok) {
  if (ok) {
    sat_pass++;
    _DROut("PASS " << name << '\n');
  } else {
    sat_fail++;
    _DROut("FAIL " << name << '\n');
  }
}

inline const CHA* Sat(const CHA* args) {
#if _Seam >= _iGeekClassicalCore
  _ATestBegin;
  _DROut(Headingf("Testing Classical Sat (TT/FC/Resolution/DPLL/Walksat/Kb/Plan)..."));

  const IUD P = IUD(1) << 0;
  const IUD Q = IUD(1) << 1;
  const IUD R = IUD(1) << 2;

  // --- Fig 7.10 SatTtEntails: does the KB clause-set entail the query? -------
  {
    SatClause kb_p[1] = {{P, 0}};          // P
    SatClause q_p[1]  = {{P, 0}};          // P
    STCHECK("TT: P entails P", SatTtEntails(kb_p, 1, q_p, 1, P));
    SatClause q_q[1]  = {{Q, 0}};          // Q
    STCHECK("TT: P does not entail Q",
           !SatTtEntails(kb_p, 1, q_q, 1, P | Q));
    SatClause q_poq[1] = {{P | Q, 0}};      // P ∨ Q
    STCHECK("TT: P entails (P ∨ Q)",
           SatTtEntails(kb_p, 1, q_poq, 1, P | Q));
    SatClause kb_poq[1] = {{P | Q, 0}};     // P ∨ Q
    STCHECK("TT: (P ∨ Q) does not entail P",
           !SatTtEntails(kb_poq, 1, q_p, 1, P | Q));
  }

  // --- Fig 7.15 SatFcEntails: forward chaining over definite clauses. -------
  {
    SatDefinite one[1] = {{P, Q}};          // P => Q
    STCHECK("FC: P=>Q, know P, entail Q", SatFcEntails(one, 1, P, 1));
    STCHECK("FC: P=>Q, know Q, not entail P", !SatFcEntails(one, 1, Q, 0));
    SatDefinite chain[2] = {{P, Q}, {Q, R}};  // P=>Q, Q=>R
    STCHECK("FC: chain P=>Q=>R, know P, entail R",
           SatFcEntails(chain, 2, P, 2));
    STCHECK("FC: know R only, cannot infer P", !SatFcEntails(chain, 2, R, 0));
  }

  // --- Fig 7.12 SatResolution: iterate to the empty clause. ------------------
  {
    SatClause contra[2] = {{P, 0}, {0, P}};  // P, ¬P
    STCHECK("Resolution: {P, ¬P} derives empty clause",
           SatResolution(contra, 2, P));
    SatClause ok2[2] = {{P, 0}, {Q, 0}};     // P, Q (no contradiction)
    STCHECK("Resolution: {P, Q} closes without empty clause",
           !SatResolution(ok2, 2, P | Q));
  }

  // --- Fig 7.17 SatDpll: satisfiability. --------------------------------------
  {
    SatClause c1[1] = {{P, 0}};              // P
    STCHECK("DPLL: {P} satisfiable", SatDpll(c1, 1, P));
    SatClause c2[2] = {{P, 0}, {0, P}};      // P, ¬P
    STCHECK("DPLL: {P, ¬P} unsatisfiable", !SatDpll(c2, 2, P));
    // 2-SAT unsat block: (P∨Q)(¬P∨Q)(¬P∨¬Q)(P∨¬Q) — every assignment fails.
    SatClause c3[4] = {{P | Q, 0}, {Q, P}, {0, P | Q}, {P, Q}};
    STCHECK("DPLL: 2-SAT unsat block", !SatDpll(c3, 4, P | Q));
    // Satisfiable pair: (P∨Q)(P∨¬Q) is true whenever P is true.
    SatClause c4[2] = {{P | Q, 0}, {P, Q}};
    STCHECK("DPLL: (P∨Q)(P∨¬Q) satisfiable", SatDpll(c4, 2, P | Q));
  }

  // --- Fig 7.18 SatWalksat: seeded random walk over models. ------------------
  {
    IUD model = 0;
    SatClause c1[1] = {{P, 0}};              // P (satisfiable)
    STCHECK("Walksat: {P} finds a model",
           SatWalksat(c1, 1, P, FPC(0.5f), 100, 7, &model) &&
           (model & P) != 0);
    SatClause c2[2] = {{P, 0}, {0, P}};      // P, ¬P (unsatisfiable)
    STCHECK("Walksat: {P, ¬P} reports failure",
           !SatWalksat(c2, 2, P, FPC(0.5f), 100, 7, &model));
  }

  // --- Fig 7.1 KBAgent: TELL / Ask / Clauses. --------------------------------
  {
    KBAgent agent;
    if (agent.Init(4)) {
      SatClause kb[1] = {{P, 0}};            // tell P
      STCHECK("Kb: Tell stores the clause", agent.Tell(kb, 1));
      STCHECK("Kb: one clause stored, positive P",
             agent.Clauses()[0].positive == P &&
             agent.Clauses()[0].negative == 0);
      SatClause qneg_p[1] = {{0, P}};        // ¬P
      STCHECK("Kb: entails P", agent.Ask(qneg_p, 1, P));
      SatClause qneg_q[1] = {{0, Q}};        // ¬Q
      STCHECK("Kb: does not entail Q", !agent.Ask(qneg_q, 1, P | Q));
    } else {
      STCHECK("Kb: Init(4) succeeds", false);
    }
  }

  // --- Fig 7.22 SatPlan: translate to CNF, find the shortest plan. ----------
  {
    SatPlanAction acts[1];
    acts[0].pre = IUD(1) << 0;   // needs atom 0
    acts[0].add = IUD(1) << 1;   // adds atom 1
    acts[0].del = 0;
    SatPlanProblem prob;
    prob.atom_count = 2;
    prob.actions = acts;
    prob.action_count = 1;
    prob.init_atoms = IUD(1) << 0;   // atom 0 true at t = 0
    prob.goal_atoms = IUD(1) << 1;   // atom 1 must hold at the horizon
    IUD model = 0;
    IUC plan[4];
    BOL ok = SatPlan(&prob, 2, &model, plan, 4);
    STCHECK("Plan: finds a plan", ok);
    if (ok) {
      STCHECK("Plan: single step, action 0", plan[0] == 0);
    }
    // An unreachable goal: add a third atom that no action adds. atom 2 is
    // false at t = 0 (closed world) and the only-if axioms block it from
    // appearing without a cause, so it can never hold at the horizon.
    SatPlanProblem prob2;
    prob2.atom_count = 3;
    prob2.actions = acts;
    prob2.action_count = 1;
    prob2.init_atoms = IUD(1) << 0;
    prob2.goal_atoms = IUD(1) << 2;
    STCHECK("Plan: unreachable goal fails",
            !SatPlan(&prob2, 3, &model, plan, 4));
  }

  _DROut("sat_pass=" << sat_pass << " sat_fail=" << sat_fail << '\n');
  return sat_fail ? "CLASSICAL_SAT_FAIL: " : _NilP;

#endif
  return _NilP;
}

}  //< namespace iGeek
