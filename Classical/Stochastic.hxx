// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_STOCHASTIC_IMPL
#define IGEEK_CLASSICAL_STOCHASTIC_IMPL
#include "Stochastic.h"
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
// No libm either: the SA acceptance ratio uses a local exp (StoExpNC) via
// range reduction + IEEE exponent bits.

static inline IUW* StoFree(IUW* origin) {
  return ObjectFactoryHeap(origin, RAMFactoryDelete);
}

static inline void StoWipe(IUW* origin, IUD bytes) {
  IUD aligned = bytes + ((-bytes) & IUD(ACPUWordMask));
  IUW* p = TPtr<IUW>(origin);
  for (IUD i = 0; i < aligned >> ACPUBytesLog2; ++i) {
    p[i] = 0;
  }
}

/* Deterministic LCG shared by the stochastic samplers. */
static inline IUD StoLcgStep(IUD s) {
  return s * 6364136223846793005ULL + 1442695040888963407ULL;
}

static inline IUD StoSeedNC(IUD seed) {
  return seed == 0 ? 1442695040888963407ULL : seed;
}

/* 1/2^32 as an FPC constant (the LCG's top half is a uniform [0,1) roll). */
static const FPC kStoUnit = FPC(2.3283064365386963e-10f);

/* exp(x) for x <= 0 (the SA acceptance case): range reduction to r = x + k
 * in (0, 1] with a 7th-degree Taylor, scaled by 2^k through the IEEE
 * exponent bits. Max relative error ~3e-5 — ample for an acceptance roll. */
static FPC StoExpNC(FPC x) {
  if (x < FPC(-80.0f)) {
    return FPC(0.0f);
  }
  if (x >= FPC(0.0f)) {
    return FPC(1.0f);
  }
  FPC neg = -x;
  ISC k = ISC(neg);
  if (FPC(k) < neg) {
    k++;  // ceil(neg)
  }
  FPC r = x + FPC(k);
  FPC e = FPC(1.0f) + r * (FPC(1.0f) + r * (FPC(0.5f) +
             r * (FPC(0.16666667f) + r * (FPC(0.041666668f) +
                 r * (FPC(0.008333334f) + r * (FPC(0.0013888889f) +
                     r * FPC(0.0001984127f)))))));
  union {
    FPC f;
    IUD u;
  } bits;
  bits.u = (IUD(k) + 127) << 23;  // 2^k
  return e * bits.f;
}

static const ISC kStoStateWords = 8;      //< state width bound (IUC words)
static const ISC kStoMaxActions = 32;     //< per-state action bound
static const IUD kNoState = ~IUD(0);      //< "undefined" state id

static IUD StoStateHashNC(const IUC* state, ISC state_words) {
  IUD h = 1469598103934665603ULL;  // FNV-1a 64-bit offset basis
  for (ISC i = 0; i < state_words; ++i) {
    h ^= IUD(state[i]);
    h *= 1099511628211ULL;
  }
  return h;
}

// --- Fig 4.5 SearchSA ------------------------------------------------------

BOL SearchSA(const Problem* problem, ISC iters, FPC T0, IUD seed,
             FPC (*Value)(const IUC* state, ISC state_words), IUC* out_state) {
  if (problem == NILP || problem->Actions == NILP || problem->Result == NILP ||
      Value == NILP || iters < 1 || problem->state_words < 1 ||
      problem->state_words > kStoStateWords) {
    return false;
  }
  ISC w = problem->state_words;
  IUW* mem = ObjectFactoryHeap(NILP, 2 * IUD(w) * IUD(sizeof(IUC)));
  if (IsError(mem)) {
    return false;
  }
  IUC* cur = TPtr<IUC>(mem);
  IUC* nxt = cur + w;
  for (ISC i = 0; i < w; ++i) {
    cur[i] = problem->initial[i];
  }
  IUD rnd = StoSeedNC(seed);
  IUC acts[kStoMaxActions];
  for (ISC t = 1; t <= iters; ++t) {
    // Internal schedule: linear cool T(t) = T0 * (1 - (t-1)/iters).
    FPC T = T0 * (FPC(1.0f) - FPC(t - 1) / FPC(IUD(iters)));
    if (T <= FPC(0.0f)) {
      break;  // T = 0 then return current
    }
    rnd = StoLcgStep(rnd);
    ISC na = problem->Actions(cur, w, acts, kStoMaxActions);
    if (na <= 0) {
      break;
    }
    IUC a = acts[IUC(rnd % IUD(na))];
    problem->Result(cur, w, a, nxt);
    rnd = StoLcgStep(rnd);
    FPC dE = Value(nxt, w) - Value(cur, w);
    BOL accept = dE > FPC(0.0f);
    if (!accept) {
      FPC roll = FPC(rnd >> 32) * kStoUnit;
      accept = roll < StoExpNC(dE / T);
    }
    if (accept) {
      for (ISC i = 0; i < w; ++i) {
        cur[i] = nxt[i];
      }
    }
  }
  for (ISC i = 0; i < w; ++i) {
    out_state[i] = cur[i];
  }
  mem = StoFree(mem);
  return true;
}

// --- Fig 4.8 SearchGA -------------------------------------------------------

BOL SearchGA(const Problem* problem, const IUC* population, ISC pop_size,
             ISC generations, FPC p_crossover, FPC p_mutate, IUD seed,
             FPC (*Fitness)(const IUC* state, ISC state_words),
             IUC* out_state) {
  if (problem == NILP || population == NILP || Fitness == NILP ||
      pop_size < 2 || generations < 1 || problem->state_words < 1 ||
      problem->state_words > kStoStateWords) {
    return false;
  }
  ISC w = problem->state_words;
  IUD bytes = IUD(pop_size) * IUD(w) * IUD(sizeof(IUC));
  IUW* mem_a = ObjectFactoryHeap(NILP, bytes);
  IUW* mem_b = ObjectFactoryHeap(NILP, bytes);
  if (IsError(mem_a) || IsError(mem_b)) {
    mem_a = StoFree(mem_a);
    mem_b = StoFree(mem_b);
    return false;
  }
  IUC* pop = TPtr<IUC>(mem_a);
  IUC* fresh = TPtr<IUC>(mem_b);
  for (ISC i = 0; i < pop_size * w; ++i) {
    pop[i] = population[i];
  }
  IUD rnd = StoSeedNC(seed);
  for (ISC gen = 0; gen < generations; ++gen) {
    for (ISC i = 0; i < pop_size; ++i) {
      // RANDOM-SELECTION: 2-way fitness tournament.
      rnd = StoLcgStep(rnd);
      ISC x = IUC(rnd % IUD(pop_size));
      rnd = StoLcgStep(rnd);
      ISC y = IUC(rnd % IUD(pop_size));
      const IUC* px = pop + IUD(x) * IUD(w);
      const IUC* py = pop + IUD(y) * IUD(w);
      if (Fitness(py, w) > Fitness(px, w)) {
        px = py;
      }
      IUC* child = fresh + IUD(i) * IUD(w);
      // REPRODUCE: one-point crossover over the word stream (probability
      // p_crossover; otherwise the child copies the selected parent).
      rnd = StoLcgStep(rnd);
      FPC roll = FPC(rnd >> 32) * kStoUnit;
      if (roll < p_crossover) {
        ISC c = IUC((rnd >> 48) % IUD(w));  // keep x[0..c-1], take y[c..]
        for (ISC j = 0; j < w; ++j) {
          child[j] = j < c ? px[j] : py[j];
        }
      } else {
        for (ISC j = 0; j < w; ++j) {
          child[j] = px[j];
        }
      }
      // MUTATE with small random probability: flip one bit of one word.
      rnd = StoLcgStep(rnd);
      if (FPC(rnd >> 32) * kStoUnit < p_mutate) {
        ISC widx = IUC((rnd >> 48) % IUD(w));
        child[widx] ^= IUC(1) << IUC(rnd % 32);
      }
    }
    // population <- new population
    IUC* tmp = pop;
    pop = fresh;
    fresh = tmp;
  }
  ISC best = 0;
  FPC best_f = Fitness(pop, w);
  for (ISC i = 1; i < pop_size; ++i) {
    FPC f = Fitness(pop + IUD(i) * IUD(w), w);
    if (f > best_f) {
      best_f = f;
      best = i;
    }
  }
  for (ISC i = 0; i < w; ++i) {
    out_state[i] = pop[IUD(best) * IUD(w) + i];
  }
  mem_a = StoFree(mem_a);
  mem_b = StoFree(mem_b);
  return true;
}

// --- Fig 4.11 AND–OR graph search ------------------------------------------

SearchAndOrPlan::~SearchAndOrPlan() {
  node_mem_.origin = StoFree(node_mem_.origin);
  state_mem_.origin = StoFree(state_mem_.origin);
}

/* Grow the plan node pool so that `need` nodes fit (plan ids run
 * 1..node_count; 0 = kPlanEmpty). */
static BOL StoPlanEnsureNodes(SearchAndOrPlan* plan, ISC need) {
  if (plan->node_mem_.origin != NILP && need <= plan->node_cap_) {
    return true;
  }
  ISC cap = plan->node_cap_;
  if (cap == 0) {
    cap = 128;
  }
  while (cap < need) {
    cap *= 2;
  }
  IUW* nm = ObjectFactoryHeap(NILP, IUD(cap) * IUD(sizeof(AndOrNode)));
  if (IsError(nm)) {
    return false;
  }
  if (plan->node_mem_.origin != NILP) {
    AndOrNode* nn = TPtr<AndOrNode>(nm);
    const AndOrNode* on = TPtr<AndOrNode>(plan->node_mem_.origin);
    for (ISC i = 0; i < plan->node_count; ++i) {
      nn[i] = on[i];
    }
    plan->node_mem_.origin = StoFree(plan->node_mem_.origin);
  }
  plan->node_mem_.origin = nm;
  plan->node_cap_ = cap;
  return true;
}

static BOL StoPoolGrowStatesNC(SearchAndOrPlan* plan) {
  ISC cap = plan->state_cap_;
  if (cap == 0) {
    cap = 64;
  }
  while (cap <= plan->state_count) {
    cap *= 2;
  }
  IUD bytes = IUD(cap) * IUD(plan->state_words) * IUD(sizeof(IUC));
  IUW* nm = ObjectFactoryHeap(NILP, bytes);
  if (IsError(nm)) {
    return false;
  }
  if (plan->state_mem_.origin != NILP) {
    IUC* ns = TPtr<IUC>(nm);
    const IUC* os = TPtr<IUC>(plan->state_mem_.origin);
    for (IUD i = 0; i < IUD(plan->state_count) * IUD(plan->state_words); ++i) {
      ns[i] = os[i];
    }
    plan->state_mem_.origin = StoFree(plan->state_mem_.origin);
  }
  plan->state_mem_.origin = nm;
  plan->state_cap_ = cap;
  return true;
}

/* Find a state already in the plan's state pool, or add a copy of it;
 * returns the pool id. */
static BOL StoPoolStateNC(const NondetProblem* problem, const IUC* state,
                          SearchAndOrPlan* plan) {
  ISC w = plan->state_words;
  for (ISC i = 0; i < plan->state_count; ++i) {
    const IUC* s = SearchAndOrState(plan, IUD(i));
    BOL same = true;
    for (ISC j = 0; j < w; ++j) {
      if (s[j] != state[j]) {
        same = false;
        break;
      }
    }
    if (same) {
      return true;
    }
  }
  if (IUD(plan->state_count + 1) > IUD(plan->state_cap_) &&
      !StoPoolGrowStatesNC(plan)) {
    return false;
  }
  IUC* s = TPtr<IUC>(plan->state_mem_.origin) +
           IUD(plan->state_count) * IUD(w);
  for (ISC i = 0; i < w; ++i) {
    s[i] = state[i];
  }
  plan->state_count++;
  return true;
}

/* OR-SEARCH(state, path): goal test, cycle check along path, then one
 * AND-SEARCH per action; the first action whose outcome plans all succeed
 * becomes the returned plan node. Path entries are pool state ids. */
static IUD StoOrSearchNC(const NondetProblem* problem, const IUC* state,
                         IUD* path, ISC path_len, SearchAndOrPlan* plan) {
  if (problem->GoalTest(state, problem->state_words)) {
    return kPlanEmpty;  // the empty plan
  }
  if (!StoPoolStateNC(problem, state, plan)) {
    return kPlanFail;
  }
  IUD sid = IUD(plan->state_count - 1);
  // if state is on path then return failure
  for (ISC i = 0; i < path_len; ++i) {
    if (path[i] == sid) {
      return kPlanFail;
    }
  }
  path[path_len] = sid;
  IUC acts[kStoMaxActions];
  ISC na = problem->Actions(state, problem->state_words, acts,
                            kStoMaxActions);
  IUC outbuf[kAndOrMaxOutcomes * kStoStateWords];
  for (ISC a = 0; a < na; ++a) {
    ISC no = problem->Outcomes(state, problem->state_words, acts[a], outbuf,
                               kAndOrMaxOutcomes);
    if (no <= 0 || no > kAndOrMaxOutcomes) {
      continue;
    }
    // AND-SEARCH over the outcome set
    BOL all_ok = true;
    AndOrNode node;
    node.action = acts[a];
    node.out_count = IUC(no);
    for (ISC i = 0; i < kAndOrMaxOutcomes; ++i) {
      node.outcome_id[i] = kNoState;
      node.child[i] = kPlanFail;
    }
    for (ISC i = 0; i < no; ++i) {
      const IUC* oc = outbuf + IUD(i) * IUD(problem->state_words);
      if (!StoPoolStateNC(problem, oc, plan)) {
        all_ok = false;
        break;
      }
      node.outcome_id[i] = IUD(plan->state_count - 1);
      IUD child = StoOrSearchNC(problem, oc, path, path_len + 1, plan);
      if (child == kPlanFail) {
        all_ok = false;
        break;
      }
      node.child[i] = child;
    }
    if (!all_ok) {
      continue;
    }
    if (!StoPlanEnsureNodes(plan, plan->node_count + 1)) {
      return kPlanFail;
    }
    TPtr<AndOrNode>(plan->node_mem_.origin)[plan->node_count] = node;
    plan->node_count++;
    return IUD(plan->node_count);
  }
  return kPlanFail;
}

BOL SearchAndOrGraph(const NondetProblem* problem, ISC max_depth,
                     SearchAndOrPlan* plan, IUD* out_root) {
  if (problem == NILP || problem->GoalTest == NILP || problem->Actions == NILP ||
      problem->Outcomes == NILP || max_depth < 1 ||
      problem->state_words < 1 || problem->state_words > kStoStateWords ||
      plan == NILP) {
    return false;
  }
  plan->node_mem_.origin = NILP;
  plan->state_mem_.origin = NILP;
  plan->node_count = 0;
  plan->node_cap_ = 0;
  plan->state_count = 0;
  plan->state_cap_ = 0;
  plan->state_words = problem->state_words;
  IUW* path_mem =
      ObjectFactoryHeap(NILP, IUD(max_depth) * IUD(sizeof(IUD)));
  if (IsError(path_mem)) {
    return false;
  }
  IUD* path = TPtr<IUD>(path_mem);
  IUD root = StoOrSearchNC(problem, problem->initial, path, 0, plan);
  path_mem = StoFree(path_mem);
  if (root == kPlanFail) {
    plan->node_mem_.origin = StoFree(plan->node_mem_.origin);
    plan->state_mem_.origin = StoFree(plan->state_mem_.origin);
    plan->node_count = 0;
    plan->node_cap_ = 0;
    plan->state_count = 0;
    plan->state_cap_ = 0;
    return false;
  }
  *out_root = root;
  return true;
}

// --- Per-state hash table layout (shared by the online agents) -------------

struct StoStateEntry {
  IUD hash;
  BOL used;
  IUC state[kStoStateWords];
};

/* Open-addressed state table with linear probing; the entry array is a
 * RAMFactory block that grows x2 at 70% load. */
static BOL StoTableEnsureNC(IUW** mem, ISC* cap, ISC* count,
                            const IUC* state, ISC w, ISC* slot_out) {
  const StoStateEntry* entries = TPtr<StoStateEntry>(*mem);
  IUD h = StoStateHashNC(state, w);
  ISC empty = -1;
  for (ISC i = 0; i < *cap; ++i) {
    ISC idx = (ISC)((h + IUD(i)) & IUD(*cap - 1));
    if (!entries[idx].used) {
      if (empty == -1) {
        empty = idx;
      }
      continue;
    }
    if (entries[idx].hash != h) {
      continue;
    }
    BOL same = true;
    for (ISC j = 0; j < w; ++j) {
      if (entries[idx].state[j] != state[j]) {
        same = false;
        break;
      }
    }
    if (same) {
      *slot_out = idx;
      return true;  // found (not new)
    }
  }
  if (empty == -1) {
    return false;
  }
  *slot_out = empty;
  return true;
}

static BOL StoTableInsertNC(IUW** mem, ISC* cap, ISC* count,
                            const IUC* state, ISC w, ISC slot) {
  if (IUD(*count + 1) * 10 >= IUD(*cap) * 7) {
    ISC nc = *cap * 2;
    IUW* nm = ObjectFactoryHeap(NILP, IUD(nc) * IUD(sizeof(StoStateEntry)));
    if (IsError(nm)) {
      return false;
    }
    StoStateEntry* ne = TPtr<StoStateEntry>(nm);
    StoWipe(nm, IUD(nc) * IUD(sizeof(StoStateEntry)));
    const StoStateEntry* oe = TPtr<StoStateEntry>(*mem);
    for (ISC i = 0; i < *cap; ++i) {
      if (!oe[i].used) {
        continue;
      }
      for (ISC k = 0; k < nc; ++k) {
        ISC idx = (ISC)((oe[i].hash + IUD(k)) & IUD(nc - 1));
        if (!ne[idx].used) {
          ne[idx] = oe[i];
          break;
        }
      }
    }
    *mem = StoFree(*mem);
    *mem = nm;
    *cap = nc;
  }
  StoStateEntry* e = &TPtr<StoStateEntry>(*mem)[slot];
  e->used = true;
  e->hash = StoStateHashNC(state, w);
  for (ISC j = 0; j < w; ++j) {
    e->state[j] = state[j];
  }
  for (ISC j = w; j < kStoStateWords; ++j) {
    e->state[j] = 0;
  }
  (*count)++;
  return true;
}

static BOL StoDataGrowNC(IUW** mem, ISC* cap, ISC stride) {
  ISC nc = *cap == 0 ? 16 : *cap * 2;
  IUW* nm = ObjectFactoryHeap(NILP, IUD(nc) * IUD(stride));
  if (IsError(nm)) {
    return false;
  }
  if (*mem != NILP) {
    IUD old_bytes = IUD(*cap) * IUD(stride);
    IUD new_bytes = IUD(nc) * IUD(stride);
    IUW* np = TPtr<IUW>(nm);
    IUW* op = TPtr<IUW>(*mem);
    for (IUD i = 0; i < old_bytes >> ACPUBytesLog2; ++i) {
      np[i] = op[i];
    }
    *mem = StoFree(*mem);
    StoWipe(nm, new_bytes - old_bytes);
  }
  *mem = nm;
  *cap = nc;
  return true;
}

// --- Fig 4.21 OnlineDfsAgent ------------------------------------------------

struct OnlineData {
  IUC untried[kStoMaxActions];
  ISC untried_top;
  BOL init_done;
  IUD unback[kStoMaxActions];
  ISC unback_top;
  IUD result_state[kStoMaxActions];  // kNoState when unseen
};

OnlineDfsAgent::OnlineDfsAgent()
    : table_mem_(NILP, &ObjectFactoryHeap),
      data_mem_(NILP, &ObjectFactoryHeap), problem_(NILP), cap_(0),
      state_count_(0), data_cap_(0), s_id_(-1), a_prev_(0) {}

OnlineDfsAgent::~OnlineDfsAgent() {
  table_mem_.origin = StoFree(table_mem_.origin);
  data_mem_.origin = StoFree(data_mem_.origin);
}

BOL OnlineDfsAgent::Init(const Problem* problem) {
  problem_ = problem;
  cap_ = 0;
  state_count_ = 0;
  data_cap_ = 0;
  s_id_ = -1;
  a_prev_ = 0;
  return problem != NILP && problem->state_words >= 1 &&
         problem->state_words <= kStoStateWords;
}

IUC OnlineDfsAgent::Percept(const IUC* s_next) {
  const Problem* p = problem_;
  ISC w = p->state_words;
  // Lazy RAMFactory allocation of the tables on first percept.
  if (table_mem_.origin == NILP) {
    table_mem_.origin =
        ObjectFactoryHeap(NILP, IUD(16) * IUD(sizeof(StoStateEntry)));
    if (IsError(table_mem_.origin)) {
      return kStochasticStop;
    }
    StoWipe(table_mem_.origin, IUD(16) * IUD(sizeof(StoStateEntry)));
    cap_ = 16;
    data_mem_.origin =
        ObjectFactoryHeap(NILP, IUD(16) * IUD(sizeof(OnlineData)));
    if (IsError(data_mem_.origin)) {
      table_mem_.origin = StoFree(table_mem_.origin);
      table_mem_.origin = NILP;
      cap_ = 0;
      return kStochasticStop;
    }
    StoWipe(data_mem_.origin, IUD(16) * IUD(sizeof(OnlineData)));
    data_cap_ = 16;
  }
  if (p->GoalTest(s_next, w)) {
    return kStochasticStop;  // GOAL-TEST(s')
  }
  ISC slot = 0;
  BOL found = StoTableEnsureNC(&table_mem_.origin, &cap_, &state_count_,
                               s_next, w, &slot);
  if (!found) {
    return kStochasticStop;
  }
  BOL is_new = (TPtr<StoStateEntry>(table_mem_.origin)[slot].used == false);
  if (is_new) {
    if (state_count_ >= data_cap_) {
      if (!StoDataGrowNC(&data_mem_.origin, &data_cap_,
                         ISC(sizeof(OnlineData)))) {
        return kStochasticStop;
      }
    }
    if (!StoTableInsertNC(&table_mem_.origin, &cap_, &state_count_, s_next,
                          w, slot)) {
      return kStochasticStop;
    }
  }
  OnlineData* d = TPtr<OnlineData>(data_mem_.origin) + slot;
  // if s' is a new state (not in untried) then untried[s'] <- ACTIONS(s')
  if (!d->init_done) {
    d->init_done = true;
    IUC acts[kStoMaxActions];
    ISC na = p->Actions(s_next, w, acts, kStoMaxActions);
    if (na > kStoMaxActions) {
      na = kStoMaxActions;
    }
    for (ISC i = 0; i < na; ++i) {
      d->untried[i] = acts[i];
    }
    d->untried_top = na;
  }
  if (s_id_ != -1) {
    // result[s, a] <- s'
    OnlineData* ds = TPtr<OnlineData>(data_mem_.origin) + s_id_;
    if (a_prev_ < kStoMaxActions) {
      ds->result_state[a_prev_] = IUD(slot);
    }
    // add s to the front of unbacktracked[s']
    if (d->unback_top < kStoMaxActions) {
      d->unback[d->unback_top++] = IUD(s_id_);
    }
  }
  IUC action = kStochasticStop;
  if (d->untried_top == 0) {
    if (d->unback_top == 0) {
      return kStochasticStop;
    }
    IUD target = d->unback[--d->unback_top];
    for (ISC b = 0; b < kStoMaxActions; ++b) {
      if (d->result_state[b] == target) {
        action = IUC(b);
        break;
      }
    }
  } else {
    action = d->untried[--d->untried_top];
  }
  s_id_ = slot;
  a_prev_ = action;
  return action;
}

void OnlineDfsAgent::Reset() {
  if (table_mem_.origin != NILP) {
    StoWipe(table_mem_.origin, IUD(cap_) * IUD(sizeof(StoStateEntry)));
  }
  if (data_mem_.origin != NILP) {
    StoWipe(data_mem_.origin, IUD(data_cap_) * IUD(sizeof(OnlineData)));
  }
  state_count_ = 0;
  s_id_ = -1;
  a_prev_ = 0;
}

// --- Fig 4.24 LrtaStarAgent --------------------------------------------------

struct LrtaData {
  BOL seen;
  FPC h;
  IUD result_state[kStoMaxActions];  // kNoState when unseen
};

LrtaStarAgent::LrtaStarAgent()
    : table_mem_(NILP, &ObjectFactoryHeap),
      data_mem_(NILP, &ObjectFactoryHeap), problem_(NILP), cap_(0),
      state_count_(0), data_cap_(0), s_id_(-1), a_prev_(0) {}

LrtaStarAgent::~LrtaStarAgent() {
  table_mem_.origin = StoFree(table_mem_.origin);
  data_mem_.origin = StoFree(data_mem_.origin);
}

BOL LrtaStarAgent::Init(const Problem* problem) {
  problem_ = problem;
  cap_ = 0;
  state_count_ = 0;
  data_cap_ = 0;
  s_id_ = -1;
  a_prev_ = 0;
  return problem != NILP && problem->state_words >= 1 &&
         problem->state_words <= kStoStateWords;
}

static FPC StoStepCostNC(const Problem* p, const IUC* s, ISC w, IUC a) {
  if (p->StepCost == NILP) {
    return FPC(1.0f);
  }
  return p->StepCost(s, w, a);
}

static FPC StoHeuristicNC(const Problem* p, const IUC* s, ISC w) {
  if (p->Heuristic == NILP) {
    return FPC(1.0f);
  }
  return p->Heuristic(s, w);
}

IUC LrtaStarAgent::Percept(const IUC* s_next) {
  const Problem* p = problem_;
  ISC w = p->state_words;
  if (table_mem_.origin == NILP) {
    table_mem_.origin =
        ObjectFactoryHeap(NILP, IUD(16) * IUD(sizeof(StoStateEntry)));
    if (IsError(table_mem_.origin)) {
      return kStochasticStop;
    }
    StoWipe(table_mem_.origin, IUD(16) * IUD(sizeof(StoStateEntry)));
    cap_ = 16;
    data_mem_.origin =
        ObjectFactoryHeap(NILP, IUD(16) * IUD(sizeof(LrtaData)));
    if (IsError(data_mem_.origin)) {
      table_mem_.origin = StoFree(table_mem_.origin);
      table_mem_.origin = NILP;
      cap_ = 0;
      return kStochasticStop;
    }
    StoWipe(data_mem_.origin, IUD(16) * IUD(sizeof(LrtaData)));
    data_cap_ = 16;
  }
  if (p->GoalTest(s_next, w)) {
    return kStochasticStop;
  }
  ISC slot = 0;
  BOL found = StoTableEnsureNC(&table_mem_.origin, &cap_, &state_count_,
                               s_next, w, &slot);
  if (!found) {
    return kStochasticStop;
  }
  BOL is_new = (TPtr<StoStateEntry>(table_mem_.origin)[slot].used == false);
  if (is_new) {
    if (state_count_ >= data_cap_) {
      if (!StoDataGrowNC(&data_mem_.origin, &data_cap_,
                         ISC(sizeof(LrtaData)))) {
        return kStochasticStop;
      }
    }
    if (!StoTableInsertNC(&table_mem_.origin, &cap_, &state_count_, s_next,
                          w, slot)) {
      return kStochasticStop;
    }
  }
  const StoStateEntry* entries = TPtr<StoStateEntry>(table_mem_.origin);
  LrtaData* data = TPtr<LrtaData>(data_mem_.origin);
  LrtaData* d = data + slot;
  // if s' is a new state (not in H) then H[s'] <- h(s')
  if (!d->seen) {
    d->seen = true;
    d->h = StoHeuristicNC(p, s_next, w);
  }
  if (s_id_ != -1) {
    LrtaData* ds = data + s_id_;
    if (a_prev_ < kStoMaxActions) {
      ds->result_state[a_prev_] = IUD(slot);
    }
    // H[s] <- min_b LRTA*-COST(s, b, result[s, b], H)
    const IUC* s = entries[s_id_].state;
    IUC acts[kStoMaxActions];
    ISC na = p->Actions(s, w, acts, kStoMaxActions);
    FPC best = FPC(1e30f);
    for (ISC i = 0; i < na && i < kStoMaxActions; ++i) {
      IUD succ = ds->result_state[acts[i]];
      FPC c;
      if (succ == kNoState) {
        c = StoHeuristicNC(p, s, w);
      } else {
        c = StoStepCostNC(p, s, w, acts[i]) + data[succ].h;
      }
      if (c < best) {
        best = c;
      }
    }
    ds->h = best;
  }
  // a <- argmin_b LRTA*-COST(s', b, result[s', b], H)
  IUC acts2[kStoMaxActions];
  ISC na2 = p->Actions(s_next, w, acts2, kStoMaxActions);
  IUC action = kStochasticStop;
  FPC best2 = FPC(1e30f);
  for (ISC i = 0; i < na2 && i < kStoMaxActions; ++i) {
    IUD succ = d->result_state[acts2[i]];
    FPC c;
    if (succ == kNoState) {
      c = StoHeuristicNC(p, s_next, w);
    } else {
      c = StoStepCostNC(p, s_next, w, acts2[i]) + data[succ].h;
    }
    if (c < best2) {
      best2 = c;
      action = acts2[i];
    }
  }
  s_id_ = slot;
  a_prev_ = action;
  return action;
}

FPC LrtaStarAgent::Cost(ISC state_id) const {
  if (data_mem_.origin == NILP || state_id < 0 ||
      state_id >= state_count_) {
    return FPC(-1.0f);  // sentinel: unseen
  }
  return TPtr<LrtaData>(data_mem_.origin)[state_id].h;
}

void LrtaStarAgent::Reset() {
  if (table_mem_.origin != NILP) {
    StoWipe(table_mem_.origin, IUD(cap_) * IUD(sizeof(StoStateEntry)));
  }
  if (data_mem_.origin != NILP) {
    StoWipe(data_mem_.origin, IUD(data_cap_) * IUD(sizeof(LrtaData)));
  }
  state_count_ = 0;
  s_id_ = -1;
  a_prev_ = 0;
}

} //< namespace iGeek
#endif
