// Copyright AStarship <https://astarship.net>.
//
// Classical Csp seam unit: constraint satisfaction (AC-3, Backtrack,
// MinConflicts, Tree). Gated on IGEEK_CLASSICAL_CSP.
#if SEAM >= IGEEK_CLASSICAL_CORE
#if SEAM >= IGEEK_CLASSICAL_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
#include "Csp.hxx"
#include <cstdio>
namespace iGeek {

static int c_pass = 0;
static int c_fail = 0;

static void CCHECK(const char* name, BOL ok) {
  if (ok) {
    c_pass++;
    D_COUT("PASS " << name << '\n');
  } else {
    c_fail++;
    D_COUT("FAIL " << name << '\n');
  }
}

/* Constraint: values must differ (x != y). */
static BOL CDiff(IUC x, IUC y) { return x != y; }

/* Build a 3-variable, 3-domain CSP: A!=B, B!=C (a path graph).
   Each variable has domain {0, 1, 2} (all 3 values). */
static CSP BuildPathCsp() {
  CSP csp;
  csp.var_count = 3;
  csp.domain_count = 3;
  // initial_domains: var_count array of IUC bitmasks.
  // All 3 values available for each var: bits 0,1,2 set = 0b111 = 7.
  static IUC domains[3] = {7, 7, 7};
  csp.initial_domains = domains;
  static CspConstraint constraints[2];
  constraints[0].var_a = 0;
  constraints[0].var_b = 1;
  constraints[0].Compatible = CDiff;
  constraints[1].var_a = 1;
  constraints[1].var_b = 2;
  constraints[1].Compatible = CDiff;
  csp.constraints = constraints;
  csp.constraint_count = 2;
  return csp;
}

/* A 2-variable, 2-domain UNSAT CSP: A!=B with domain {0} for both.
   No valid assignment exists. */
static CSP BuildUnsatCsp() {
  CSP csp;
  csp.var_count = 2;
  csp.domain_count = 2;
  static IUC domains[2] = {1, 1};  // only value 0 available for both.
  csp.initial_domains = domains;
  static CspConstraint constraints[1];
  constraints[0].var_a = 0;
  constraints[0].var_b = 1;
  constraints[0].Compatible = CDiff;
  csp.constraints = constraints;
  csp.constraint_count = 1;
  return csp;
}

/* Verify a solution: every constraint is satisfied. */
static BOL VerifySolution(const CSP* csp, const IUC* out) {
  for (ISC i = 0; i < csp->constraint_count; ++i) {
    const CspConstraint* c = &csp->constraints[i];
    if (!c->Compatible(out[c->var_a], out[c->var_b])) {
      return false;
    }
  }
  // Every value must be in the domain.
  for (ISC v = 0; v < csp->var_count; ++v) {
    if (!(csp->initial_domains[v] & (IUC(1) << out[v]))) {
      return false;
    }
  }
  return true;
}

inline const CHA* Csp(const CHA* args) {
#if SEAM >= IGEEK_CLASSICAL_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing Classical Csp (AC-3/Backtrack/MinConflicts/Tree)..."));

  CSP csp = BuildPathCsp();

  // --- AC-3: arc consistency on the path CSP (should succeed).
  {
    IUC domains[3] = {7, 7, 7};  // copy of initial_domains.
    BOL ok = CspAc3(&csp, domains);
    CCHECK("AC-3: path CSP is consistent", ok);
    // After AC-3, no domain should be empty.
    CCHECK("AC-3: no empty domains",
           (domains[0] != 0) && (domains[1] != 0) && (domains[2] != 0));
  }

  // --- AC-3: unsat CSP (should fail — a domain gets wiped).
  {
    CSP unsat = BuildUnsatCsp();
    IUC domains[2] = {1, 1};
    BOL ok = CspAc3(&unsat, domains);
    CCHECK("AC-3: unsat CSP detected", !ok);
  }

  // --- Backtrack: find a solution for the path CSP.
  {
    IUC domains[3] = {7, 7, 7};
    IUC out[3];
    BOL ok = CspBacktrack(&csp, domains, out);
    CCHECK("Backtrack: found solution", ok);
    if (ok) {
      CCHECK("Backtrack: solution is valid", VerifySolution(&csp, out));
      // A!=B!=C: out[0]!=out[1], out[1]!=out[2].
      CCHECK("Backtrack: A!=B", out[0] != out[1]);
      CCHECK("Backtrack: B!=C", out[1] != out[2]);
    }
  }

  // --- Backtrack: unsat CSP (should fail).
  {
    CSP unsat = BuildUnsatCsp();
    IUC domains[2] = {1, 1};
    IUC out[2];
    BOL ok = CspBacktrack(&unsat, domains, out);
    CCHECK("Backtrack: unsat CSP fails", !ok);
  }

  // --- MinConflicts: find a solution (seeded, deterministic).
  {
    IUC domains[3] = {7, 7, 7};
    IUC out[3];
    BOL ok = CspMinConflicts(&csp, domains, out, 1000, 42);
    CCHECK("MinConflicts: found solution", ok);
    if (ok) {
      CCHECK("MinConflicts: solution is valid", VerifySolution(&csp, out));
    }
  }

  // --- Tree: the path CSP is a tree (should succeed).
  {
    IUC domains[3] = {7, 7, 7};
    IUC out[3];
    BOL ok = CspTree(&csp, domains, out);
    CCHECK("Tree: path CSP solved", ok);
    if (ok) {
      CCHECK("Tree: solution is valid", VerifySolution(&csp, out));
    }
  }

  D_COUT("c_pass=" << c_pass << " c_fail=" << c_fail << '\n');
  return c_fail ? "CLASSICAL_CSP_FAIL: " : NILP;

#endif
  return NILP;
}

}  //< namespace iGeek
