// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_RL_IMPL
#define IGEEK_CLASSICAL_RL_IMPL
#include "RL.h"
#include "../ASCIICrabs/Binary.hpp"
#include "../ASCIICrabs/Autoject.hxx"
namespace iGeek {

// --- Internal machinery (internal linkage; not module API) ----------------
//
// All RAM is created and destroyed through the RAMFactory
// (ObjectFactoryHeap — the same factory BigInt and AgentState use):
//   alloc: ObjectFactoryHeap(NILP, bytes)  free: ObjectFactoryHeap(origin,
//   RAMFactoryDelete). No naked new/delete/malloc in this folder.
//
// "s_prev_ == -1" plays the book's role for s = null (no previous state).

static inline IUW* RlFree(IUW* origin) {
  return ObjectFactoryHeap(origin, RAMFactoryDelete);
}

static inline void RlWipe(IUW* origin, IUD bytes) {
  IUD aligned = bytes + ((-bytes) & IUD(ACPUWordMask));
  IUW* p = TPtr<IUW>(origin);
  for (IUD i = 0; i < aligned >> ACPUBytesLog2; ++i) {
    p[i] = 0;
  }
}

/* Deterministic LCG for the Q-agent's exploration. */
static inline IUD RlLcgStep(IUD s) {
  return s * 6364136223846793005ULL + 1442695040888963407ULL;
}

// --- PassiveTDAgent (Fig 21.4) --------------------------------------------

PassiveTDAgent::PassiveTDAgent()
    : u_mem_(NILP, &ObjectFactoryHeap),
      n_mem_(NILP, &ObjectFactoryHeap), state_count_(0), action_count_(0),
      gamma_(FPC(0.0f)), alpha_(FPC(0.0f)), IsTerminal_(NILP),
      Policy_(NILP), s_prev_(-1), a_prev_(0), r_prev_(FPC(0.0f)) {}

PassiveTDAgent::~PassiveTDAgent() {
  u_mem_.origin = RlFree(u_mem_.origin);
  n_mem_.origin = RlFree(n_mem_.origin);
}

BOL PassiveTDAgent::Init(ISC state_count, ISC action_count, FPC gamma,
                          FPC alpha, BOL (*IsTerminal)(ISC state),
                          IUC (*Policy)(ISC state)) {
  if (state_count < 1 || action_count < 1 || IsTerminal == NILP ||
      Policy == NILP) {
    return false;
  }
  u_mem_.origin = ObjectFactoryHeap(NILP, IUD(state_count) * IUD(sizeof(FPC)));
  n_mem_.origin = ObjectFactoryHeap(NILP, IUD(state_count) * IUD(sizeof(IUD)));
  if (IsError(u_mem_.origin) || IsError(n_mem_.origin)) {
    u_mem_.origin = RlFree(u_mem_.origin);
    n_mem_.origin = RlFree(n_mem_.origin);
    return false;
  }
  RlWipe(u_mem_.origin, IUD(state_count) * IUD(sizeof(FPC)));
  RlWipe(n_mem_.origin, IUD(state_count) * IUD(sizeof(IUD)));
  state_count_ = state_count;
  action_count_ = action_count;
  gamma_ = gamma;
  alpha_ = alpha;
  IsTerminal_ = IsTerminal;
  Policy_ = Policy;
  s_prev_ = -1;
  a_prev_ = 0;
  r_prev_ = FPC(0.0f);
  return true;
}

FPC PassiveTDAgent::Utility(ISC state) const {
  return TPtr<FPC>(u_mem_.origin)[state];
}

IUC PassiveTDAgent::Percept(ISC s_next, FPC r) {
  FPC* u = TPtr<FPC>(u_mem_.origin);
  IUD* n = TPtr<IUD>(n_mem_.origin);
  if (n[s_next] == 0) {
    u[s_next] = r;  // s' is new
  }
  if (s_prev_ != -1) {
    n[s_prev_]++;
    // Book's TD term uses r, the reward received for the previous (s, a).
    u[s_prev_] += alpha_ * (r_prev_ + gamma_ * u[s_next] - u[s_prev_]);
  }
  if (IsTerminal_(s_next)) {
    s_prev_ = -1;  // s, a, r <- null
    return IUC(action_count_);  // the book's null action
  }
  s_prev_ = s_next;
  a_prev_ = Policy_(s_next);
  r_prev_ = r;
  return a_prev_;
}

void PassiveTDAgent::Reset() {
  RlWipe(u_mem_.origin, IUD(state_count_) * IUD(sizeof(FPC)));
  RlWipe(n_mem_.origin, IUD(state_count_) * IUD(sizeof(IUD)));
  s_prev_ = -1;
  a_prev_ = 0;
  r_prev_ = FPC(0.0f);
}

// --- QAgent (Fig 21.8) -----------------------------------------------------

QAgent::QAgent()
    : q_mem_(NILP, &ObjectFactoryHeap), n_mem_(NILP, &ObjectFactoryHeap),
      state_count_(0), action_count_(0), gamma_(FPC(0.0f)),
      alpha_(FPC(0.0f)), eps_(FPC(0.0f)), IsTerminal_(NILP),
      rnd_state_(1442695040888963407ULL), s_prev_(-1), a_prev_(0),
      r_prev_(FPC(0.0f)) {}

QAgent::~QAgent() {
  q_mem_.origin = RlFree(q_mem_.origin);
  n_mem_.origin = RlFree(n_mem_.origin);
}

BOL QAgent::Init(ISC state_count, ISC action_count, FPC gamma, FPC alpha,
                  FPC eps, BOL (*IsTerminal)(ISC state), IUD seed) {
  if (state_count < 1 || action_count < 1 || IsTerminal == NILP) {
    return false;
  }
  IUD cells = IUD(state_count) * IUD(action_count);
  q_mem_.origin = ObjectFactoryHeap(NILP, cells * IUD(sizeof(FPC)));
  n_mem_.origin = ObjectFactoryHeap(NILP, cells * IUD(sizeof(IUD)));
  if (IsError(q_mem_.origin) || IsError(n_mem_.origin)) {
    q_mem_.origin = RlFree(q_mem_.origin);
    n_mem_.origin = RlFree(n_mem_.origin);
    return false;
  }
  RlWipe(q_mem_.origin, cells * IUD(sizeof(FPC)));
  RlWipe(n_mem_.origin, cells * IUD(sizeof(IUD)));
  state_count_ = state_count;
  action_count_ = action_count;
  gamma_ = gamma;
  alpha_ = alpha;
  eps_ = eps;
  IsTerminal_ = IsTerminal;
  rnd_state_ = seed == 0 ? 1442695040888963407ULL : seed;
  s_prev_ = -1;
  a_prev_ = 0;
  r_prev_ = FPC(0.0f);
  return true;
}

FPC QAgent::Q(ISC state, IUC action) const {
  return TPtr<FPC>(q_mem_.origin)[IUD(state) * IUD(action_count_) + action];
}

IUC QAgent::Percept(ISC s_next, FPC r) {
  FPC* q = TPtr<FPC>(q_mem_.origin);
  IUD* n = TPtr<IUD>(n_mem_.origin);
  if (s_prev_ != -1) {
    IUD cell = IUD(s_prev_) * IUD(action_count_) + IUD(a_prev_);
    n[cell]++;
    // Book's Q term uses r, the reward received for the previous (s, a).
    FPC target = r_prev_;
    if (!IsTerminal_(s_next)) {
      // The book's Q[s, None] sentinel for terminal outcomes: bootstrap
      // with 0 instead, so target = r + gamma * max Q(s', .).
      FPC best = q[IUD(s_next) * IUD(action_count_)];
      for (ISC a = 1; a < action_count_; ++a) {
        FPC v = q[IUD(s_next) * IUD(action_count_) + a];
        if (v > best) {
          best = v;
        }
      }
      target += gamma_ * best;
    }
    q[cell] += alpha_ * (target - q[cell]);
  }
  if (IsTerminal_(s_next)) {
    s_prev_ = -1;  // s, a, r <- null
    return IUC(action_count_);  // the book's null action
  }
  IUC action;
  if (eps_ > FPC(0.0f)) {
    rnd_state_ = RlLcgStep(rnd_state_);
    FPC roll = FPC(rnd_state_ >> 32) * FPC(2.3283064365386963e-10f);  // /2^32
    if (roll < eps_) {
      rnd_state_ = RlLcgStep(rnd_state_);
      action = IUC(rnd_state_ % IUD(action_count_));
    } else {
      action = 0;
      FPC best = q[IUD(s_next) * IUD(action_count_)];
      for (ISC a = 1; a < action_count_; ++a) {
        FPC v = q[IUD(s_next) * IUD(action_count_) + a];
        if (v > best) {
          best = v;
          action = IUC(a);
        }
      }
    }
  } else {
    action = 0;
    FPC best = q[IUD(s_next) * IUD(action_count_)];
    for (ISC a = 1; a < action_count_; ++a) {
      FPC v = q[IUD(s_next) * IUD(action_count_) + a];
      if (v > best) {
        best = v;
        action = IUC(a);
      }
    }
  }
  s_prev_ = s_next;
  a_prev_ = action;
  r_prev_ = r;
  return action;
}

void QAgent::Reset() {
  IUD cells = IUD(state_count_) * IUD(action_count_);
  RlWipe(q_mem_.origin, cells * IUD(sizeof(FPC)));
  RlWipe(n_mem_.origin, cells * IUD(sizeof(IUD)));
  s_prev_ = -1;
  a_prev_ = 0;
  r_prev_ = FPC(0.0f);
}

// --- PassiveADPAgent (Fig 21.2) --------------------------------------------

PassiveADPAgent::PassiveADPAgent()
    : u_mem_(NILP, &ObjectFactoryHeap), r_mem_(NILP, &ObjectFactoryHeap),
      nsa_mem_(NILP, &ObjectFactoryHeap), np_mem_(NILP, &ObjectFactoryHeap),
      p_mem_(NILP, &ObjectFactoryHeap), u_work_(NILP, &ObjectFactoryHeap),
      state_count_(0), action_count_(0), gamma_(FPC(0.0f)),
      tol_(FPC(0.0f)), IsTerminal_(NILP), Policy_(NILP), s_prev_(-1),
      a_prev_(0), r_prev_(FPC(0.0f)) {}

PassiveADPAgent::~PassiveADPAgent() {
  u_mem_.origin = RlFree(u_mem_.origin);
  r_mem_.origin = RlFree(r_mem_.origin);
  nsa_mem_.origin = RlFree(nsa_mem_.origin);
  np_mem_.origin = RlFree(np_mem_.origin);
  p_mem_.origin = RlFree(p_mem_.origin);
  u_work_.origin = RlFree(u_work_.origin);
}

BOL PassiveADPAgent::Init(ISC state_count, ISC action_count, FPC gamma,
                           FPC tol, BOL (*IsTerminal)(ISC state),
                           IUC (*Policy)(ISC state)) {
  if (state_count < 1 || action_count < 1 || IsTerminal == NILP ||
      Policy == NILP) {
    return false;
  }
  IUD cells = IUD(state_count) * IUD(action_count);
  IUD tcells = cells * IUD(state_count);
  u_mem_.origin =
      ObjectFactoryHeap(NILP, IUD(state_count) * IUD(sizeof(FPC)));
  r_mem_.origin =
      ObjectFactoryHeap(NILP, IUD(state_count) * IUD(sizeof(FPC)));
  nsa_mem_.origin = ObjectFactoryHeap(NILP, cells * IUD(sizeof(IUD)));
  np_mem_.origin = ObjectFactoryHeap(NILP, tcells * IUD(sizeof(IUD)));
  p_mem_.origin = ObjectFactoryHeap(NILP, tcells * IUD(sizeof(FPC)));
  u_work_.origin =
      ObjectFactoryHeap(NILP, IUD(state_count) * IUD(sizeof(FPC)));
  if (IsError(u_mem_.origin) || IsError(r_mem_.origin) ||
      IsError(nsa_mem_.origin) || IsError(np_mem_.origin) ||
      IsError(p_mem_.origin) || IsError(u_work_.origin)) {
    u_mem_.origin = RlFree(u_mem_.origin);
    r_mem_.origin = RlFree(r_mem_.origin);
    nsa_mem_.origin = RlFree(nsa_mem_.origin);
    np_mem_.origin = RlFree(np_mem_.origin);
    p_mem_.origin = RlFree(p_mem_.origin);
    u_work_.origin = RlFree(u_work_.origin);
    return false;
  }
  RlWipe(u_mem_.origin, IUD(state_count) * IUD(sizeof(FPC)));
  RlWipe(r_mem_.origin, IUD(state_count) * IUD(sizeof(FPC)));
  RlWipe(nsa_mem_.origin, cells * IUD(sizeof(IUD)));
  RlWipe(np_mem_.origin, tcells * IUD(sizeof(IUD)));
  RlWipe(p_mem_.origin, tcells * IUD(sizeof(FPC)));
  state_count_ = state_count;
  action_count_ = action_count;
  gamma_ = gamma;
  tol_ = tol;
  IsTerminal_ = IsTerminal;
  Policy_ = Policy;
  s_prev_ = -1;
  a_prev_ = 0;
  r_prev_ = FPC(0.0f);
  return true;
}

FPC PassiveADPAgent::Utility(ISC state) const {
  return TPtr<FPC>(u_mem_.origin)[state];
}

/* POLICY-EVALUATION(π, U, mdp): sweep the fixed-policy Bellman equations
U(s) = R(s) + γ Σ_t P(t | s, π(s)) U(t) until the maximum change over a
sweep drops below tol (terminals keep R). */
static void RlPolicyEvalNC(const PassiveADPAgent* agent, FPC* u) {
  FPC* un = TPtr<FPC>(agent->u_work_.origin);
  const FPC* p = TPtr<FPC>(agent->p_mem_.origin);
  const FPC* r = TPtr<FPC>(agent->r_mem_.origin);
  for (;;) {
    FPC delta = FPC(0.0f);
    for (ISC s = 0; s < agent->state_count_; ++s) {
      FPC v;
      if (agent->IsTerminal_(s)) {
        v = r[s];
      } else {
        IUC a = agent->Policy_(s);
        FPC acc = r[s];
        const FPC* row =
            p + ((IUD(s) * IUD(agent->action_count_) + a) *
                 IUD(agent->state_count_));
        for (ISC t = 0; t < agent->state_count_; ++t) {
          acc += agent->gamma_ * row[t] * u[t];
        }
        v = acc;
      }
      un[s] = v;
      FPC d = un[s] - u[s];
      if (d < 0) {
        d = -d;
      }
      if (d > delta) {
        delta = d;
      }
    }
    for (ISC s = 0; s < agent->state_count_; ++s) {
      u[s] = un[s];
    }
    if (delta < agent->tol_) {
      return;
    }
  }
}

IUC PassiveADPAgent::Percept(ISC s_next, FPC r) {
  FPC* u = TPtr<FPC>(u_mem_.origin);
  FPC* rr = TPtr<FPC>(r_mem_.origin);
  IUD* nsa = TPtr<IUD>(nsa_mem_.origin);
  IUD* np = TPtr<IUD>(np_mem_.origin);
  {
    // "s' is new" detection: Nsa[s', a] == 0 for every action. The book
    // lists no per-state count for ADP; this is equivalent on first sight
    // (every non-terminal state is acted in before the episode ends).
    BOL new_state = true;
    for (ISC a = 0; a < action_count_; ++a) {
      if (nsa[IUD(s_next) * IUD(action_count_) + a] != 0) {
        new_state = false;
        break;
      }
    }
    if (new_state) {
      u[s_next] = r;
      rr[s_next] = r;
    }
  }
  if (s_prev_ != -1) {
    IUD cell = IUD(s_prev_) * IUD(action_count_) + IUD(a_prev_);
    nsa[cell]++;
    // Book's R update as a per-state sample mean: R += (r - R)/Nsa[s, a],
    // r = the reward received for the previous (s, a).
    rr[s_prev_] += (r_prev_ - rr[s_prev_]) / FPC(nsa[cell]);
    np[cell * IUD(state_count_) + s_next]++;
    // Re-derive P(t | s_prev_, a_prev_) from the counts.
    FPC* prow = TPtr<FPC>(p_mem_.origin) + cell * IUD(state_count_);
    for (ISC t = 0; t < state_count_; ++t) {
      IUD cnt = np[cell * IUD(state_count_) + t];
      prow[t] = cnt == 0 ? FPC(0.0f) : FPC(cnt) / FPC(nsa[cell]);
    }
  }
  RlPolicyEvalNC(this, u);
  if (IsTerminal_(s_next)) {
    s_prev_ = -1;  // s, a <- null
    r_prev_ = r;
    return IUC(action_count_);  // the book's null action
  }
  s_prev_ = s_next;
  a_prev_ = Policy_(s_next);
  r_prev_ = r;
  return a_prev_;
}

void PassiveADPAgent::Reset() {
  IUD cells = IUD(state_count_) * IUD(action_count_);
  IUD tcells = cells * IUD(state_count_);
  RlWipe(u_mem_.origin, IUD(state_count_) * IUD(sizeof(FPC)));
  RlWipe(r_mem_.origin, IUD(state_count_) * IUD(sizeof(FPC)));
  RlWipe(nsa_mem_.origin, cells * IUD(sizeof(IUD)));
  RlWipe(np_mem_.origin, tcells * IUD(sizeof(IUD)));
  RlWipe(p_mem_.origin, tcells * IUD(sizeof(FPC)));
  s_prev_ = -1;
  a_prev_ = 0;
  r_prev_ = FPC(0.0f);
}

} //< namespace iGeek
#endif
