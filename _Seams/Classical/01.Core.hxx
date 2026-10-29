// Copyright AStarship <https://astarship.net>.
//
// AUTO-GENERATED from __Seams.yaml (iGeek CLASSICAL).
// CLASSICAL core seam unit: the unit tests for the deterministic parts of the
// CLASSICAL engine. Gated on IGEEK_CLASSICAL_CORE (the first / default seam).
#if SEAM >= IGEEK_CLASSICAL_CORE
#if SEAM == IGEEK_CLASSICAL_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
#include "Bayes.hxx"
#include <cstdio>
#include <cmath>
namespace iGeek {

static int g_pass = 0;
static int g_fail = 0;

static void Check(const char* name, FPC got, FPC want, FPC tol) {
  if (std::abs(got - want) <= tol) {
    g_pass++;
    D_COUT("name=" << name <<);
  } else {
    g_fail++;
    D_COUT("name=" << name <<);
  }
}

static BOL BuildUmbrella(TBayesNet* bn) {
  if (!bn->Init(3, 4, 16)) {
    return false;
  }
  {
    FPC ppt[1] = {FPC(0.5f)};  // U
    if (!bn->TellVar(0, NILP, 0, ppt)) return false;
  }
  {
    FPC ppt[2] = {FPC(0.2f), FPC(0.9f)};  // R | U=0, R | U=1
    ISC parents[1] = {0};
    if (!bn->TellVar(1, parents, 1, ppt)) return false;
  }
  {
    FPC ppt[2] = {FPC(0.3f), FPC(0.8f)};  // W | R=0, W | R=1
    ISC parents[1] = {1};
    if (!bn->TellVar(2, parents, 1, ppt)) return false;
  }
  return true;
}

inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_CLASSICAL_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf());


  TBayesNet bn;
  if (!BuildUmbrella(&bn)) {
    D_COUT("BuildUmbrella failed\n");
    return "CLASSICAL_FAIL: BuildUmbrella failed";
  }

  // P-table sanity.
  {
    IUC pv0[1] = {0};
    IUC pv1[1] = {1};
    Check("P(R=1|U=0)", bn.P(1, 0, pv0), FPC(0.2f), FPC(1e-6f));
    Check("P(R=1|U=1)", bn.P(1, 1, pv1), FPC(0.9f), FPC(1e-6f));
    IUC wv[1] = {1};
    Check("P(W=1|R=1)", bn.P(2, 1, wv), FPC(0.8f), FPC(1e-6f));
  }

  // --- Fig 14.9 ENUMERATION-ASK (exact) -----------------------------------
  {
    Check("Enum P(R=1)", TBayesEnumerate(bn, 1, NILP, 0), FPC(0.55f),
          FPC(1e-4f));
    TBayesEv ev1[1] = {{2, 1}};  // W = 1
    Check("Enum P(R=1|W=1)", TBayesEnumerate(bn, 1, ev1, 1),
          FPC(0.44f / 0.575f), FPC(1e-4f));
    Check("Enum P(U=1|W=1)", TBayesEnumerate(bn, 0, ev1, 1),
          FPC(0.375f / 0.575f), FPC(1e-4f));
    // Contradictory evidence -> 0.
    TBayesEv ev2[2] = {{2, 1}, {2, 0}};
    Check("Enum contradictory", TBayesEnumerate(bn, 1, ev2, 2), FPC(0.0f),
          FPC(0.0f));
  }

  // --- Fig 14.11 ELIMINATION-ASK (exact) -----------------------------------
  {
    TBayesInfer inf;
    if (!inf.Init(3, 512)) {
      D_COUT("VE Init failed\n");
      return "CLASSICAL_FAIL: VE Init failed";
    }
    Check("VE P(R=1)", inf.Ask(bn, 1, NILP, 0), FPC(0.55f), FPC(1e-4f));
    TBayesEv ev1[1] = {{2, 1}};
    Check("VE P(R=1|W=1)", inf.Ask(bn, 1, ev1, 1), FPC(0.44f / 0.575f),
          FPC(1e-4f));
    Check("VE P(U=1|W=1)", inf.Ask(bn, 0, ev1, 1), FPC(0.375f / 0.575f),
          FPC(1e-4f));
    TBayesEv ev2[1] = {{1, 0}};  // R = 0
    // P(U=1 | R=0) = 0.5 (U independent of R... no: R depends on U, so
    // P(U=1|R=0) = P(R=0|U=1)P(U=1) / P(R=0) = 0.1*0.5/0.45 = 0.05/0.45
    Check("VE P(U=1|R=0)", inf.Ask(bn, 0, ev2, 1), FPC(0.05f / 0.45f),
          FPC(1e-4f));
  }

  // --- Fig 14.13 PRIOR-SAMPLE ----------------------------------------------
  {
    IUC x[3];
    FPC ok = TBayesSample(bn, 42, x);
    if (ok != FPC(1.0f) || (x[0] != 0 && x[0] != 1) ||
        (x[1] != 0 && x[1] != 1) || (x[2] != 0 && x[2] != 1)) {
      D_COUT("(label)");
      g_fail++;
    } else {
      g_pass++;
      D_COUT("IUC=" << IUC(x[0]) <<  << IUC(x[1]) <<  << IUC(x[2]) <<);
    }
    // 20k samples: P(R=1) should land near 0.55.
    IUD c1 = 0;
    for (IUD j = 0; j < 20000; ++j) {
      TBayesSample(bn, 1000 + j, x);
      c1 += IUD(x[1]);
    }
    Check("Sample P(R=1)~", FPC(c1) / FPC(20000), FPC(0.55f), FPC(0.02f));
  }

  // --- Fig 14.14 REJECTION-SAMPLING ----------------------------------------
  {
    TBayesEv ev1[1] = {{2, 1}};  // W = 1 (P=0.575, ~57% accepted)
    Check("Reject P(R=1|W=1)",
          TBayesReject(bn, 1, ev1, 1, 100000, 7), FPC(0.44f / 0.575f),
          FPC(0.02f));
  }

  // --- Fig 14.15 LIKELIHOOD-WEIGHTING --------------------------------------
  {
    TBayesEv ev1[1] = {{2, 1}};
    Check("LW P(R=1|W=1)", TBayesLw(bn, 1, ev1, 1, 100000, 7),
          FPC(0.44f / 0.575f), FPC(0.02f));
    Check("LW P(U=1|W=1)", TBayesLw(bn, 0, ev1, 1, 100000, 7),
          FPC(0.375f / 0.575f), FPC(0.02f));
  }

  // --- Fig 14.16 GIBBS-ASK -------------------------------------------------
  {
    TBayesEv ev1[1] = {{2, 1}};
    // Hidden vars = {U, R} (W is evidence). 2000 cycles * 2 vars = 4000 draws.
    Check("Gibbs P(R=1|W=1)", TBayesGibbs(bn, 1, ev1, 1, 20000, 7),
          FPC(0.44f / 0.575f), FPC(0.03f));
    Check("Gibbs P(U=1|W=1)", TBayesGibbs(bn, 0, ev1, 1, 20000, 7),
          FPC(0.375f / 0.575f), FPC(0.03f));
  }

  D_COUT("g_pass=" << g_pass << " g_fail=" << g_fail << '\n');
  return g_fail ? "CLASSICAL_FAIL: " : NILP;

#endif
  return NILP;
}

}  //< namespace ClassicalTest
