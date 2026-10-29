// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_BAYES_IMPL
#define IGEEK_CLASSICAL_BAYES_IMPL 1
#include "Bayes.h"
#include "../ASCIICrabs/Autoject.hxx"
namespace iGeek {

// --- Internal machinery (internal linkage; not module API) ----------------
//
// All RAM is created and destroyed through the RAMFactory
// (ObjectFactoryHeap — the same factory BigInt and AgentState use):
//   alloc: ObjectFactoryHeap(NILP, bytes)  free: ObjectFactoryHeap(origin,
//   RAMFactoryDelete). No naked new/delete/malloc in this folder.

static inline IUW* BayesFree(IUW* origin) {
  return ObjectFactoryHeap(origin, RAMFactoryDelete);
}

static inline void BayesWipe(IUW* origin, IUD bytes) {
  IUD aligned = bytes + ((-bytes) & IUD(ACPUWordMask));
  IUW* p = TPtr<IUW>(origin);
  for (IUD i = 0; i < aligned >> ACPUBytesLog2; ++i) {
    p[i] = 0;
  }
}

/* Deterministic LCG (same pattern as RL.hxx RlLcgStep). */
static inline IUD BayesLcgStep(IUD s) {
  return s * 6364136223846793005ULL + 1442695040888963407ULL;
}

static inline FPC BayesRoll(IUD* rnd) {
  *rnd = BayesLcgStep(*rnd);
  return FPC(*rnd >> 32) * FPC(2.3283064365386963e-10f);  // /2^32
}

// --- BaysNet -------------------------------------------------------------

/* Per-var record: parent slice start/count into the parent pool, PPT slice start into the ppt pool. 16 bytes per var (word-aligned tail padding). */
struct BaysVarRec {
  ISC parent_start;
  ISC parent_count;
  ISC ppt_start;
};

static char* BayesRecEndNC(const BaysNet* bn) {
  return TPtr<char>(bn->mem_.origin) +
         IUD(bn->var_count_) * IUD(sizeof(BaysVarRec));
}

static ISC* BayesParentsNC(const BaysNet* bn) {
  return TPtr<ISC>(BayesRecEndNC(bn));
}

static FPC* BayesPptsNC(const BaysNet* bn) {
  return TPtr<FPC>(
      BayesRecEndNC(bn) + IUD(bn->parent_cap_) * IUD(sizeof(ISC)));
}

BaysNet::BaysNet()
    : mem_(NILP, &ObjectFactoryHeap), var_count_(0), parent_cap_(0),
      ppt_cap_(0), parent_count_(0), ppt_count_(0) {}

BaysNet::~BaysNet() {
  mem_.origin = BayesFree(mem_.origin);
}

BOL BaysNet::Init(ISC var_count, ISC parent_cap, ISC ppt_cap) {
  if (var_count < 1 || var_count > 16 || parent_cap < 0 || ppt_cap < 0) {
    return false;
  }
  IUD bytes = IUD(var_count) * IUD(sizeof(BaysVarRec)) +
              IUD(parent_cap) * IUD(sizeof(ISC)) + IUD(ppt_cap) * IUD(sizeof(FPC));
  mem_.origin = ObjectFactoryHeap(NILP, bytes);
  if (IsError(mem_.origin)) {
    return false;
  }
  BayesWipe(mem_.origin, bytes);
  var_count_ = var_count;
  parent_cap_ = parent_cap;
  ppt_cap_ = ppt_cap;
  parent_count_ = 0;
  ppt_count_ = 0;
  return true;
}

BOL BaysNet::TellVar(ISC var, const ISC* parents, ISC parent_count,
                       const FPC* ppt) {
  if (var < 0 || var >= var_count_ || ppt == NILP || parent_count < 0 ||
      parent_count > 8 || (parents == NILP && parent_count > 0)) {
    return false;
  }
  IUD rows = IUD(1) << IUD(parent_count);
  if (parent_count_ + parent_count > parent_cap_ ||
      ppt_count_ + rows > IUD(ppt_cap_)) {
    return false;
  }
  BaysVarRec* recs = TPtr<BaysVarRec>(mem_.origin);
  ISC* parents_pool = BayesParentsNC(this);
  FPC* ppt_pool = BayesPptsNC(this);
  recs[var].parent_start = parent_count_;
  recs[var].parent_count = parent_count;
  recs[var].ppt_start = ISC(ppt_count_);
  for (ISC i = 0; i < parent_count; ++i) {
    if (parents[i] < 0 || parents[i] >= var_count_ || parents[i] >= var) {
      return false;  // topological order required (documented)
    }
    parents_pool[parent_count_ + i] = parents[i];
  }
  for (IUD r = 0; r < rows; ++r) {
    ppt_pool[ppt_count_ + ISC(r)] = ppt[r];
  }
  parent_count_ += parent_count;
  ppt_count_ += ISC(rows);
  return true;
}

FPC BaysNet::P(ISC var, IUC value, const IUC* parent_values) const {
  BaysVarRec* recs = TPtr<BaysVarRec>(mem_.origin);
  const BaysVarRec* r = &recs[var];
  FPC* ppt = BayesPptsNC(this);
  IUD row = 0;
  for (ISC i = 0; i < r->parent_count; ++i) {
    row |= IUD(parent_values[i] & 1) << IUD(i);
  }
  FPC p = ppt[r->ppt_start + ISC(row)];
  return value ? p : FPC(1.0f) - p;
}

// --- evidence helpers ------------------------------------------------------

/* ev_map[var] = 0 unseen, 1 evidence value 0, 2 evidence value 1. */
static BOL BayesEvMapNC(const BaysNet& bn, const BaysEv* ev, ISC ev_n,
                        IUC* ev_map, ISC ev_map_cap) {
  for (ISC i = 0; i < ev_map_cap; ++i) {
    ev_map[i] = 0;
  }
  for (ISC i = 0; i < ev_n; ++i) {
    if (ev[i].var < 0 || ev[i].var >= bn.var_count_ || ev[i].value > 1) {
      return false;
    }
    IUC code = IUC(ev[i].value) + 1;
    if (ev_map[ev[i].var] != 0 && ev_map[ev[i].var] != code) {
      return false;  // contradictory evidence
    }
    ev_map[ev[i].var] = code;
  }
  return true;
}

/* Scope bits of a mask into bits[0..n-1], ascending; returns n. */
static ISC BayesScopeBitsNC(IUD scope, ISC* bits) {
  ISC n = 0;
  for (IUD s = scope; s != 0; s &= s - 1) {
    ISC w = 0;
    IUD x = s;
    while ((x & 1) == 0) {
      w++;
      x >>= 1;
    }
    bits[n++] = w;
  }
  return n;
}

/* Rank of assignment a over scope bits (bit i of the rank = bit bits[i]). */
static IUD BayesRankNC(const ISC* bits, ISC n, IUD a) {
  IUD r = 0;
  for (ISC i = 0; i < n; ++i) {
    r |= ((a >> bits[i]) & 1) << IUD(i);
  }
  return r;
}

static IUD BayesExpandNC(const ISC* bits, ISC n, IUD r) {
  IUD a = 0;
  for (ISC i = 0; i < n; ++i) {
    a |= ((r >> i) & 1) << IUD(bits[i]);
  }
  return a;
}

// --- Fig 14.9 ENUMERATION-ASK ----------------------------------------------

static FPC BayesEnumerateAllNC(const BaysNet& bn, ISC idx, IUC* ev_map) {
  if (idx == bn.var_count_) {
    return FPC(1.0f);
  }
  BaysVarRec* recs = TPtr<BaysVarRec>(bn.mem_.origin);
  const BaysVarRec* r = &recs[idx];
  const ISC* parents = BayesParentsNC(&bn) + r->parent_start;
  IUC pv[8];
  for (ISC i = 0; i < r->parent_count; ++i) {
    IUC code = ev_map[parents[i]];
    pv[i] = code ? IUC(code - 1) : IUC(0);
  }
  IUC code = ev_map[idx];
  if (code != 0) {
    // Evidence value y: factor P(y | parents) times the rest.
    return bn.P(idx, IUC(code - 1), pv) *
           BayesEnumerateAllNC(bn, idx + 1, ev_map);
  }
  // Hidden: sum over both values, extending e with Y = y for the
  // recursion (the figure's ey).
  FPC acc = FPC(0.0f);
  for (ISC y = 0; y < 2; ++y) {
    ev_map[idx] = IUC(y) + 1;
    acc += bn.P(idx, IUC(y), pv) * BayesEnumerateAllNC(bn, idx + 1, ev_map);
  }
  ev_map[idx] = 0;
  return acc;
}

FPC BaysEnumerate(const BaysNet& bn, ISC query_var, const BaysEv* ev,
                    ISC ev_n) {
  if (query_var < 0 || query_var >= bn.var_count_) {
    return FPC(0.0f);
  }
  IUC ev_map[16];
  if (!BayesEvMapNC(bn, ev, ev_n, ev_map, 16)) {
    return FPC(0.0f);
  }
  IUC base = ev_map[query_var];
  ev_map[query_var] = 2;  // e' extended with X = 1
  FPC q1 = BayesEnumerateAllNC(bn, 0, ev_map);
  ev_map[query_var] = 1;  // e' extended with X = 0
  FPC q0 = BayesEnumerateAllNC(bn, 0, ev_map);
  ev_map[query_var] = base;
  FPC total = q0 + q1;
  return total > FPC(0.0f) ? q1 / total : FPC(0.0f);
}

// --- Fig 14.11 ELIMINATION-ASK ---------------------------------------------

BayesInfer::BayesInfer()
    : scope_mem_(NILP, &ObjectFactoryHeap), val_mem_(NILP, &ObjectFactoryHeap),
      cells_used_(0), cells_cap_(0), var_count_(0) {}

BayesInfer::~BayesInfer() {
  scope_mem_.origin = BayesFree(scope_mem_.origin);
  val_mem_.origin = BayesFree(val_mem_.origin);
}

BOL BayesInfer::Init(ISC var_count, IUD cells_cap) {
  if (var_count < 1 || var_count > 16 || cells_cap < 1) {
    return false;
  }
  var_count_ = var_count;
  val_mem_.origin = ObjectFactoryHeap(NILP, cells_cap * IUD(sizeof(FPC)));
  if (IsError(val_mem_.origin)) {
    val_mem_.origin = BayesFree(val_mem_.origin);
    return false;
  }
  BayesWipe(val_mem_.origin, cells_cap * IUD(sizeof(FPC)));
  cells_cap_ = cells_cap;
  cells_used_ = 0;
  return true;
}

FPC BayesInfer::Ask(const BaysNet& bn, ISC query_var, const BaysEv* ev,
                     ISC ev_n) {
  if (query_var < 0 || query_var >= var_count_) {
    return FPC(0.0f);
  }
  IUC ev_map[16];
  if (!BayesEvMapNC(bn, ev, ev_n, ev_map, 16)) {
    return FPC(0.0f);
  }
  BaysVarRec* recs = TPtr<BaysVarRec>(bn.mem_.origin);
  const ISC* parents_pool = BayesParentsNC(&bn);
  const FPC* ppt_pool = BayesPptsNC(&bn);

  // Running factor: unit. The pool cursor only ever moves forward: every
  // factor (m, j, so) gets a fresh non-aliasing block, and live factors
  // stay intact until the destructor (documented).
  FPC* pool = TPtr<FPC>(val_mem_.origin);
  IUD off = 1;
  IUD run_scope = 0;
  IUD run_cells = 1;
  FPC* run = pool;
  run[0] = FPC(1.0f);
  ISC rbits[16];
  ISC rn = 0;

  // Reverse topological order (children before parents): a variable may be
  // summed out only after every factor that still references it — its
  // children's factors — has been joined. In this order each variable's
  // parents are still present, so MAKE-FACTOR keeps the full
  // {v} + parents(v) scope.
  for (ISC v = var_count_ - 1; v >= 0; --v) {
    // MAKE-FACTOR(v, e): scope {v} + parents(v).
    const BaysVarRec* r = &recs[v];
    IUD m_scope = IUD(1) << IUD(v);
    for (ISC i = 0; i < r->parent_count; ++i) {
      m_scope |= IUD(1) << IUD(parents_pool[r->parent_start + i]);
    }
    ISC mbits[16];
    ISC mn = BayesScopeBitsNC(m_scope, mbits);
    IUD mc = IUD(1) << IUD(mn);
    if (off + mc > cells_cap_) {
      return FPC(0.0f);  // pool exhausted (caller chose the cap)
    }
    FPC* m = pool + off;
    off += mc;
    for (IUD rk = 0; rk < mc; ++rk) {
      IUD a = BayesExpandNC(mbits, mn, rk);
      BOL ok = true;
      for (ISC i = 0; i < mn && ok; ++i) {
        IUC code = ev_map[mbits[i]];
        if (code != 0 && ((a >> mbits[i]) & 1) != IUC(code - 1)) {
          ok = false;
        }
      }
      IUD row = 0;
      for (ISC i = 0; i < r->parent_count; ++i) {
        row |= ((a >> parents_pool[r->parent_start + i]) & 1) << IUD(i);
      }
      FPC p1 = ppt_pool[r->ppt_start + ISC(row)];
      m[rk] = ok ? (((a >> v) & 1) ? p1 : FPC(1.0f) - p1) : FPC(0.0f);
    }
    // JOIN: product over the union of scopes.
    IUD j_scope = run_scope | m_scope;
    ISC jbits[16];
    ISC jn = BayesScopeBitsNC(j_scope, jbits);
    IUD jc = IUD(1) << IUD(jn);
    if (off + jc > cells_cap_) {
      return FPC(0.0f);
    }
    FPC* j = pool + off;
    off += jc;
    for (IUD rk = 0; rk < jc; ++rk) {
      IUD a = BayesExpandNC(jbits, jn, rk);
      j[rk] = run[BayesRankNC(rbits, rn, a)] * m[BayesRankNC(mbits, mn, a)];
    }
    run = j;
    run_scope = j_scope;
    run_cells = jc;
    rn = BayesScopeBitsNC(run_scope, rbits);

    // SUM-OUT hidden variables (not query, not evidence).
    IUC code = ev_map[v];
    if (v != query_var && code == 0) {
      IUD so_scope = run_scope & ~(IUD(1) << IUD(v));
      ISC sbits[16];
      ISC sn = BayesScopeBitsNC(so_scope, sbits);
      IUD sc = IUD(1) << IUD(sn);
      if (off + sc > cells_cap_) {
        return FPC(0.0f);
      }
      FPC* so = pool + off;
      off += sc;
      for (IUD rk = 0; rk < sc; ++rk) {
        IUD a = BayesExpandNC(sbits, sn, rk);
        so[rk] = run[BayesRankNC(rbits, rn, a)] +
                 run[BayesRankNC(rbits, rn, a | (IUD(1) << IUD(v)))];
      }
      run = so;
      run_scope = so_scope;
      run_cells = sc;
      rn = BayesScopeBitsNC(run_scope, rbits);
    }
  }
  cells_used_ = off;

  // NORMALIZE(POINTWISE-PRODUCT(factors)) over the query value. The final
  // scope is {query} ∪ evidence vars (every other var was summed out);
  // pin each evidence var to its observed value.
  ISC fb[16];
  ISC fn = BayesScopeBitsNC(run_scope, fb);
  IUD a1 = IUD(1) << IUD(query_var);
  for (ISC i = 0; i < fn; ++i) {
    if (fb[i] != query_var) {
      IUC code = ev_map[fb[i]];
      a1 |= IUD(code ? code - 1 : 0) << IUD(fb[i]);
    }
  }
  FPC q1 = run[BayesRankNC(fb, fn, a1)];
  FPC q0 = run[BayesRankNC(fb, fn, a1 & ~(IUD(1) << IUD(query_var)))];
  FPC total = q0 + q1;
  return total > FPC(0.0f) ? q1 / total : FPC(0.0f);
}

// --- Fig 14.13 PRIOR-SAMPLE ------------------------------------------------

static BOL BayesSampleNC(const BaysNet& bn, IUD* rnd, IUC* x) {
  BaysVarRec* recs = TPtr<BaysVarRec>(bn.mem_.origin);
  const ISC* parents_pool = BayesParentsNC(&bn);
  for (ISC i = 0; i < bn.var_count_; ++i) {
    const BaysVarRec* r = &recs[i];
    IUD row = 0;
    for (ISC j = 0; j < r->parent_count; ++j) {
      row |= IUD(x[parents_pool[r->parent_start + j]] & 1) << IUD(j);
    }
    FPC p = BayesPptsNC(&bn)[r->ppt_start + ISC(row)];
    x[i] = (BayesRoll(rnd) < p) ? IUC(1) : IUC(0);
  }
  return true;
}

FPC BaysSample(const BaysNet& bn, IUD seed, IUC* out_event) {
  if (out_event == NILP || seed == 0) {
    return FPC(0.0f);
  }
  IUD rnd = seed;
  BayesSampleNC(bn, &rnd, out_event);
  return FPC(1.0f);  // a sample is not a number; 1.0 signals success
}

static BOL BayesConsistentNC(const IUC* x, const BaysEv* ev, ISC ev_n) {
  for (ISC i = 0; i < ev_n; ++i) {
    if (x[ev[i].var] != ev[i].value) {
      return false;
    }
  }
  return true;
}

// --- Fig 14.14 REJECTION-SAMPLING ------------------------------------------

FPC BaysReject(const BaysNet& bn, ISC query_var, const BaysEv* ev,
                 ISC ev_n, ISC n_samples, IUD seed) {
  if (query_var < 0 || query_var >= bn.var_count_ || n_samples < 1 ||
      seed == 0) {
    return FPC(0.0f);
  }
  IUD c0 = 0, c1 = 0;
  IUC x[16];
  IUD rnd = seed;
  for (ISC j = 0; j < n_samples; ++j) {
    BayesSampleNC(bn, &rnd, x);
    if (BayesConsistentNC(x, ev, ev_n)) {
      if (x[query_var]) {
        c1++;
      } else {
        c0++;
      }
    }
  }
  IUD total = c0 + c1;
  return total ? FPC(c1) / FPC(total) : FPC(0.0f);
}

// --- Fig 14.15 LIKELIHOOD-WEIGHTING ----------------------------------------

FPC BaysLw(const BaysNet& bn, ISC query_var, const BaysEv* ev,
             ISC ev_n, ISC n_samples, IUD seed) {
  if (query_var < 0 || query_var >= bn.var_count_ || n_samples < 1 ||
      seed == 0) {
    return FPC(0.0f);
  }
  IUC ev_map[16];
  if (!BayesEvMapNC(bn, ev, ev_n, ev_map, 16)) {
    return FPC(0.0f);
  }
  FPC w0 = FPC(0.0f), w1 = FPC(0.0f);
  IUC x[16];
  IUD rnd = seed;
  BaysVarRec* recs = TPtr<BaysVarRec>(bn.mem_.origin);
  const ISC* parents_pool = BayesParentsNC(&bn);
  for (ISC j = 0; j < n_samples; ++j) {
    FPC w = FPC(1.0f);
    for (ISC i = 0; i < bn.var_count_; ++i) {
      const BaysVarRec* r = &recs[i];
      IUC code = ev_map[i];
      if (code != 0) {
        // Evidence variable: weight by its likelihood, keep its value.
        x[i] = IUC(code - 1);
        IUD prow = 0;
        for (ISC k = 0; k < r->parent_count; ++k) {
          prow |= IUD(x[parents_pool[r->parent_start + k]] & 1) << IUD(k);
        }
        FPC p = BayesPptsNC(&bn)[r->ppt_start + ISC(prow)];
        w *= x[i] ? p : FPC(1.0f) - p;
      } else {
        IUD row = 0;
        for (ISC k = 0; k < r->parent_count; ++k) {
          row |= IUD(x[parents_pool[r->parent_start + k]] & 1) << IUD(k);
        }
        FPC p = BayesPptsNC(&bn)[r->ppt_start + ISC(row)];
        x[i] = (BayesRoll(&rnd) < p) ? IUC(1) : IUC(0);
      }
    }
    if (w == FPC(0.0f)) {
      continue;
    }
    if (x[query_var]) {
      w1 += w;
    } else {
      w0 += w;
    }
  }
  FPC total = w0 + w1;
  return total > FPC(0.0f) ? w1 / total : FPC(0.0f);
}

// --- Fig 14.16 GIBBS-ASK ---------------------------------------------------

/* P(z = value | mb) given the current assignment x: P(z | parents) times
 * the product of the children's likelihoods at their current values.
 * Children are found by scanning the parent lists (fine for the documented
 * small-net scope). */
static FPC BayesGibbsCondNC(const BaysNet& bn, ISC z, IUC value,
                            const IUC* x) {
  BaysVarRec* recs = TPtr<BaysVarRec>(bn.mem_.origin);
  const ISC* parents_pool = BayesParentsNC(&bn);
  const FPC* ppt_pool = BayesPptsNC(&bn);
  const BaysVarRec* rz = &recs[z];
  IUD row = 0;
  for (ISC k = 0; k < rz->parent_count; ++k) {
    row |= IUD(x[parents_pool[rz->parent_start + k]] & 1) << IUD(k);
  }
  FPC p = ppt_pool[rz->ppt_start + ISC(row)];
  FPC acc = value ? p : FPC(1.0f) - p;
  for (ISC c = z + 1; c < bn.var_count_; ++c) {
    const BaysVarRec* rc = &recs[c];
    BOL is_child = false;
    for (ISC k = 0; k < rc->parent_count; ++k) {
      if (parents_pool[rc->parent_start + k] == z) {
        is_child = true;
        break;
      }
    }
    if (!is_child) {
      continue;
    }
    IUD crow = 0;
    for (ISC k = 0; k < rc->parent_count; ++k) {
      IUC xv = value;
      if (parents_pool[rc->parent_start + k] == z) {
        crow |= IUD(xv & 1) << IUD(k);
      } else {
        crow |= IUD(x[parents_pool[rc->parent_start + k]] & 1) << IUD(k);
      }
    }
    FPC cp = ppt_pool[rc->ppt_start + ISC(crow)];
    acc *= x[c] ? cp : FPC(1.0f) - cp;
  }
  return acc;
}

FPC BaysGibbs(const BaysNet& bn, ISC query_var, const BaysEv* ev,
                ISC ev_n, ISC n_cycles, IUD seed) {
  if (query_var < 0 || query_var >= bn.var_count_ || n_cycles < 1 ||
      seed == 0) {
    return FPC(0.0f);
  }
  IUC ev_map[16];
  if (!BayesEvMapNC(bn, ev, ev_n, ev_map, 16)) {
    return FPC(0.0f);
  }
  IUC x[16];
  IUD rnd = seed;
  for (ISC i = 0; i < bn.var_count_; ++i) {
    IUC code = ev_map[i];
    if (code != 0) {
      x[i] = IUC(code - 1);
    } else {
      x[i] = (BayesRoll(&rnd) < FPC(0.5f)) ? IUC(1) : IUC(0);
    }
  }
  IUD d0 = 0, d1 = 0;
  for (ISC j = 0; j < n_cycles; ++j) {
    for (ISC z = 0; z < bn.var_count_; ++z) {
      if (ev_map[z] != 0) {
        continue;  // the book's Z: all nonevidence variables, query included
      }
      FPC m0 = BayesGibbsCondNC(bn, z, IUC(0), x);
      FPC m1 = BayesGibbsCondNC(bn, z, IUC(1), x);
      FPC total = m0 + m1;
      if (total <= FPC(0.0f)) {
        continue;  // degenerate conditional (documented)
      }
      x[z] = (BayesRoll(&rnd) < m1 / total) ? IUC(1) : IUC(0);
      if (x[query_var]) {
        d1++;
      } else {
        d0++;
      }
    }
  }
  IUD total = d0 + d1;
  return total ? FPC(d1) / FPC(total) : FPC(0.0f);
}

} //< namespace iGeek
#endif
