// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_MDP_IMPL
#define IGEEK_CLASSICAL_MDP_IMPL
#include "Mdp.h"
#include "../ASCIICrabs/Binary.hpp"
#include "../ASCIICrabs/Autoject.hxx"
namespace iGeek {

// --- Internal machinery (internal linkage; not module API) ----------------
//
// All RAM is created and destroyed through the RAMFactory
// (ObjectFactoryHeap — the same factory BigInt and AgentState use):
//   alloc: ObjectFactoryHeap(NILP, bytes)  free: ObjectFactoryHeap(origin,
//   RAMFactoryDelete).  No naked new/delete/malloc in this folder.

/* Equation (17.8): delta < eps * (1 - gamma) / (1 + gamma). */
static FPC MdpThresholdNC(const MDP* mdp, FPC eps) {
  FPC one = FPC(1.0f);
  return eps * (one - mdp->gamma) / (one + mdp->gamma);
}

/* Sum over successors of (state, action) of prob * (R(s,a) + gamma * V[s']). */
static FPC MdpBackupNC(const MDP* mdp, ISC state, IUC action, FPC* V,
                       ISC* succ, FPC* prob, ISC cap) {
  ISC n = mdp->Transitions(state, action, succ, cap, prob, cap);
  if (n <= 0) {
    return FPC(0.0f);
  }
  FPC acc = FPC(0.0f);
  FPC reward = mdp->Reward(state, action);
  for (ISC i = 0; i < n; ++i) {
    acc += prob[i] * (reward + mdp->gamma * V[succ[i]]);
  }
  return acc;
}

/* Runtime-sized (state_count) successor/probability scratch, both buffers
 * RAMFactory-owned; the destructor returns them to the factory. */
struct MdpScratch {
  Autoject succ_;
  Autoject prob_;
  MdpScratch()
      : succ_(NILP, &ObjectFactoryHeap), prob_(NILP, &ObjectFactoryHeap) {}
  MdpScratch(const MdpScratch&) = delete;
  MdpScratch& operator=(const MdpScratch&) = delete;
  ~MdpScratch() {
    Free();
  }
  /* Allocates room for s_count successors/probabilities; false on OOM. */
  BOL Init(ISC s_count) {
    Free();
    ISW bytes = IUD(s_count) * IUD(sizeof(ISC));
    succ_.origin = ObjectFactoryHeap(NILP, bytes);
    prob_.origin = ObjectFactoryHeap(NILP, bytes);
    return !IsError(succ_.origin) && !IsError(prob_.origin);
  }
  void Free() {
    succ_.origin = ObjectFactoryHeap(succ_.origin, RAMFactoryDelete);
    prob_.origin = ObjectFactoryHeap(prob_.origin, RAMFactoryDelete);
  }
  ISC* Succ() const {
    return TPtr<ISC>(succ_.origin);
  }
  FPC* Prob() const {
    return TPtr<FPC>(prob_.origin);
  }
};

// --- Figure 17.4: value iteration ------------------------------------------

ISC ValueIteration(const MDP* mdp, FPC* V, FPC* policy, ISC max_iters) {
  ISC s_count = mdp->state_count < 1 ? 1 : mdp->state_count;
  ISC a_count = mdp->action_count < 1 ? 1 : mdp->action_count;
  ISC iters = max_iters < 1 ? 1 : max_iters;
  for (ISC s = 0; s < s_count; ++s) {
    V[s] = FPC(0.0f);
    policy[s] = 0;
  }
  MdpScratch scratch;
  if (!scratch.Init(s_count)) {
    return -1;
  }
  FPC threshold = MdpThresholdNC(mdp, mdp->tol);
  ISC ran = iters;
  for (ISC it = 0; it < iters; ++it) {
    FPC delta = FPC(0.0f);
    for (ISC s = 0; s < s_count; ++s) {
      if (mdp->IsTerminal(s)) {
        V[s] = FPC(0.0f);
        policy[s] = 0;
        continue;
      }
      FPC best = FPC(-1e30f);
      IUC best_a = 0;
      for (ISC a = 0; a < a_count; ++a) {
        FPC v = MdpBackupNC(mdp, s, IUC(a), V, scratch.Succ(),
                            scratch.Prob(), s_count);
        if (v > best) {
          best = v;
          best_a = IUC(a);
        }
      }
      FPC d = best - V[s];
      if (d < FPC(0.0f)) {
        d = -d;
      }
      if (d > delta) {
        delta = d;
      }
      V[s] = best;
      policy[s] = best_a;
    }
    ran = it + 1;
    if (delta < threshold) {
      break;
    }
  }
  return ran;
}

// --- Figure 17.7: policy iteration -----------------------------------------

ISC PolicyIteration(const MDP* mdp, FPC* V, FPC* policy, ISC max_iters) {
  ISC s_count = mdp->state_count < 1 ? 1 : mdp->state_count;
  ISC a_count = mdp->action_count < 1 ? 1 : mdp->action_count;
  ISC iters = max_iters < 1 ? 1 : max_iters;
  for (ISC s = 0; s < s_count; ++s) {
    V[s] = FPC(0.0f);
    policy[s] = 0;
  }
  MdpScratch scratch;
  if (!scratch.Init(s_count)) {
    return -1;
  }
  FPC threshold = MdpThresholdNC(mdp, mdp->tol);
  ISC ran = iters;
  for (ISC outer = 0; outer < iters; ++outer) {
    // Policy evaluation: successive approximation on the fixed policy.
    for (ISC inner = 0; inner < iters; ++inner) {
      FPC delta = FPC(0.0f);
      for (ISC s = 0; s < s_count; ++s) {
        if (mdp->IsTerminal(s)) {
          V[s] = FPC(0.0f);
          continue;
        }
        FPC v = MdpBackupNC(mdp, s, policy[s], V, scratch.Succ(),
                            scratch.Prob(), s_count);
        FPC d = v - V[s];
        if (d < FPC(0.0f)) {
          d = -d;
        }
        if (d > delta) {
          delta = d;
        }
        V[s] = v;
      }
      if (delta < threshold) {
        break;
      }
    }
    // Policy improvement.
    BOL changed = false;
    for (ISC s = 0; s < s_count; ++s) {
      if (mdp->IsTerminal(s)) {
        policy[s] = 0;
        continue;
      }
      FPC best = FPC(-1e30f);
      IUC best_a = 0;
      for (ISC a = 0; a < a_count; ++a) {
        FPC v = MdpBackupNC(mdp, s, IUC(a), V, scratch.Succ(),
                            scratch.Prob(), s_count);
        if (v > best) {
          best = v;
          best_a = IUC(a);
        }
      }
      if (best_a != policy[s]) {
        policy[s] = best_a;
        changed = true;
      }
    }
    ran = outer + 1;
    if (!changed) {
      break;
    }
  }
  return ran;
}

} //< namespace iGeek
#endif
