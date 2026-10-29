// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_SAT_IMPL
#define IGEEK_CLASSICAL_SAT_IMPL
#include "Sat.h"
#include "../ASCIICrabs/Binary.hpp"
#include "../ASCIICrabs/Autoject.hxx"
#include "Search.hxx"
namespace iGeek {

// --- Internal machinery (internal linkage; not module API) ----------------
//
// All RAM is created and destroyed through the RAMFactory
// (ObjectFactoryHeap — the same factory BigInt and AgentState use):
//   alloc: ObjectFactoryHeap(NILP, bytes)  free: ObjectFactoryHeap(origin,
//   RAMFactoryDelete). No naked new/delete/malloc in this folder.

static inline IUW* SatFree(IUW* origin) {
  return ObjectFactoryHeap(origin, RAMFactoryDelete);
}

static inline void SatWipe(IUW* origin, IUD bytes) {
  IUD aligned = bytes + ((-bytes) & IUD(ACPUWordMask));
  IUW* p = TPtr<IUW>(origin);
  for (IUD i = 0; i < aligned >> ACPUBytesLog2; ++i) {
    p[i] = 0;
  }
}

static inline IUD SatLcgStep(IUD s) {
  return s * 6364136223846793005ULL + 1442695040888963407ULL;
}

static inline IUD SatSeedNC(IUD seed) {
  return seed == 0 ? 1442695040888963407ULL : seed;
}

static const FPC kSatUnit = FPC(2.3283064365386963e-10f);  // 1/2^32

/* A clause holds in the partial model (t, f) iff some literal is true:
 * a positive literal s is true when s in t; a negative one when s in f. */
static inline BOL SatClauseTrueNC(const SatClause* c, IUD t, IUD f) {
  return (c->positive & t) != 0 || (c->negative & f) != 0;
}

/* A clause is falsified iff every literal is assigned and false:
 * a positive literal is false when its symbol is in f; a negative one
 * when its symbol is in t. (The empty clause is falsified by any model.) */
static inline BOL SatClauseFalseNC(const SatClause* c, IUD t, IUD f) {
  return (c->positive & ~f) == 0 && (c->negative & ~t) == 0;
}

// --- Fig 7.10 SatTtEntails --------------------------------------------------

BOL SatTtEntails(const SatClause* kb, ISC kb_count, const SatClause* query,
                 ISC query_count, IUD universe) {
  if (kb == NILP || query == NILP) {
    return false;
  }
  // Enumerate every submask model of the symbol universe.
  IUD m = universe;
  for (;;) {
    IUD f = universe & ~m;
    BOL kb_true = true;
    for (ISC i = 0; i < kb_count; ++i) {
      if (!SatClauseTrueNC(&kb[i], m, f)) {
        kb_true = false;
        break;
      }
    }
    if (kb_true) {
      for (ISC i = 0; i < query_count; ++i) {
        if (!SatClauseTrueNC(&query[i], m, f)) {
          return false;  // a model of KB where the query fails
        }
      }
    }
    if (m == 0) {
      break;
    }
    m = (m - 1) & universe;
  }
  return true;
}

// --- Fig 7.15 SatFcEntails --------------------------------------------------

BOL SatFcEntails(const SatDefinite* kb, ISC kb_count, IUD known, ISC q) {
  if (kb == NILP || q > 63) {
    return false;
  }
  IUD inferred = 0;  // processed symbols
  // count table: premises still unmet per clause
  IUW* mem = ObjectFactoryHeap(NILP, IUD(kb_count + 64) * IUD(sizeof(ISC)));
  if (IsError(mem)) {
    return false;
  }
  ISC* count = TPtr<ISC>(mem);
  for (ISC i = 0; i < kb_count; ++i) {
    ISC n = 0;
    IUD m = kb[i].premise;
    while (m != 0) {
      n += ISC(m & 1);
      m >>= 1;
    }
    count[i] = n;
  }
  // agenda: a ring of symbols stored after the count table; a clause fires
  // at most once, so total pushes <= kb_count + 64 < cap.
  ISC cap = kb_count + 65;
  ISC* a = count + kb_count;
  ISC head = 0;
  ISC tail = 0;
  IUD facts = known;  // seed with the facts known to be true in KB
  while (facts != 0) {
    ISC s = 0;
    IUD t = facts;
    while ((t & 1) == 0) {
      s++;
      t >>= 1;
    }
    a[tail % cap] = s;
    tail++;
    facts &= facts - 1;
  }
  while (tail != head) {
    ISC p = a[head % cap];
    head++;
    if (p == q) {
      mem = SatFree(mem);
      return true;
    }
    if ((inferred >> p) & 1) {
      continue;
    }
    inferred |= IUD(1) << p;
    for (ISC i = 0; i < kb_count; ++i) {
      if ((kb[i].premise & (IUD(1) << p)) == 0) {
        continue;
      }
      count[i]--;
      if (count[i] == 0) {
        IUD cc = kb[i].conclusion;
        if (cc == 0) {
          continue;
        }
        ISC cs = 0;
        IUD t = cc;
        while ((t & 1) == 0) {
          cs++;
          t >>= 1;
        }
        a[tail % cap] = cs;
        tail++;
      }
    }
  }
  mem = SatFree(mem);
  return false;
}

// --- Fig 7.12 SatResolution --------------------------------------------------

/* Growing clause set with linear duplicate check (reference driver).
 * Add returns 1 when the clause is newly added, 0 when it is already
 * present, -1 on allocation failure. */
struct SatClauseSet {
  IUW* mem;
  ISC count;
  ISC cap;
  SatClauseSet() : mem(NILP), count(0), cap(0) {}
  ISC Add(const SatClause* c) {
    for (ISC i = 0; i < count; ++i) {
      const SatClause* s = &TPtr<SatClause>(mem)[i];
      if (s->positive == c->positive && s->negative == c->negative) {
        return 0;  // already present
      }
    }
    if (count == cap) {
      ISC nc = cap == 0 ? 64 : cap * 2;
      IUW* nm = ObjectFactoryHeap(NILP, IUD(nc) * IUD(sizeof(SatClause)));
      if (IsError(nm)) {
        return -1;
      }
      if (mem != NILP) {
        SatClause* ns = TPtr<SatClause>(nm);
        const SatClause* os = TPtr<SatClause>(mem);
        for (ISC i = 0; i < count; ++i) {
          ns[i] = os[i];
        }
        mem = SatFree(mem);
      }
      mem = nm;
      cap = nc;
    }
    TPtr<SatClause>(mem)[count++] = *c;
    return 1;
  }
};

/* PL-RESOLVE(Ci, Cj): one positive literal of Ci against one negative
 * literal of Cj (and vice versa); returns the resolvent clauses written
 * into out (up to out_cap). Empty clause => *out_empty true. */
static ISC SatResolveNC(const SatClause* ci, const SatClause* cj,
                        SatClause* out, ISC out_cap, BOL* out_empty) {
  *out_empty = false;
  ISC n = 0;
  // Ci's positive literal vs Cj's negative literal (and the reverse)
  IUD pairs[2];
  pairs[0] = ci->positive & cj->negative;
  pairs[1] = cj->positive & ci->negative;
  for (ISC k = 0; k < 2; ++k) {
    IUD s = pairs[k];
    while (s != 0) {
      ISC bit = 0;
      IUD t = s;
      while ((t & 1) == 0) {
        bit++;
        t >>= 1;
      }
      IUD sym = IUD(1) << bit;
      SatClause r;
      r.positive = (ci->positive | cj->positive) & ~sym;
      r.negative = (ci->negative | cj->negative) & ~sym;
      if (r.positive == 0 && r.negative == 0) {
        *out_empty = true;
      }
      if (n < out_cap) {
        out[n] = r;
      }
      n++;
      s &= s - 1;
    }
  }
  return n;
}

BOL SatResolution(const SatClause* clauses, ISC clause_count, IUD universe) {
  (void)universe;
  if (clauses == NILP || clause_count < 1) {
    return false;
  }
  SatClauseSet cs;
  for (ISC i = 0; i < clause_count; ++i) {
    if (cs.Add(&clauses[i]) == -1) {
      cs.mem = SatFree(cs.mem);
      return false;
    }
  }
  // If the empty clause is already present (unit contradiction), done.
  for (ISC i = 0; i < cs.count; ++i) {
    const SatClause* c = &TPtr<SatClause>(cs.mem)[i];
    if (c->positive == 0 && c->negative == 0) {
      cs.mem = SatFree(cs.mem);
      return true;
    }
  }
  for (;;) {
    SatClause new_clauses[64];
    ISC new_count = 0;
    BOL empty = false;
    for (ISC i = 0; i < cs.count && !empty; ++i) {
      for (ISC j = i + 1; j < cs.count; ++j) {
        SatClause scratch[64];
        ISC n = SatResolveNC(&TPtr<SatClause>(cs.mem)[i],
                             &TPtr<SatClause>(cs.mem)[j], scratch, 64, &empty);
        if (empty) {
          cs.mem = SatFree(cs.mem);
          return true;
        }
        for (ISC k = 0; k < n && new_count < 64; ++k) {
          new_clauses[new_count++] = scratch[k];
        }
      }
    }
    ISC added = 0;
    for (ISC i = 0; i < new_count; ++i) {
      if (cs.Add(&new_clauses[i]) == 1) {
        if (new_clauses[i].positive == 0 && new_clauses[i].negative == 0) {
          cs.mem = SatFree(cs.mem);
          return true;
        }
        added++;
      }
    }
    if (added == 0) {
      cs.mem = SatFree(cs.mem);
      return false;  // new ⊆ clauses
    }
  }
}

// --- Fig 7.17 SatDpll --------------------------------------------------------

static BOL SatDpllRecNC(const SatClause* clauses, ISC clause_count,
                        IUD universe, IUD* model, IUD t, IUD f) {
  for (ISC i = 0; i < clause_count; ++i) {
    if (SatClauseFalseNC(&clauses[i], t, f)) {
      return false;
    }
  }
  // All clauses true? (the book's first success test)
  {
    BOL all_true = true;
    for (ISC i = 0; i < clause_count; ++i) {
      if (!SatClauseTrueNC(&clauses[i], t, f)) {
        all_true = false;
        break;
      }
    }
    if (all_true) {
      if (model != NILP) {
        *model = t;
      }
      return true;
    }
  }
  IUD unassigned = universe & ~t & ~f;
  // FIND-PURE-SYMBOL: a symbol occurring in only one polarity (and
  // unassigned) — assign it so no literal falsifies it.
  for (IUD s = unassigned; s != 0; s &= s - 1) {
    ISC bit = 0;
    IUD x = s;
    while ((x & 1) == 0) {
      bit++;
      x >>= 1;
    }
    IUD sym = IUD(1) << bit;
    BOL pos = false;
    BOL neg = false;
    for (ISC i = 0; i < clause_count; ++i) {
      if (clauses[i].positive & sym) {
        pos = true;
      }
      if (clauses[i].negative & sym) {
        neg = true;
      }
    }
    if (pos && !neg) {
      if (SatDpllRecNC(clauses, clause_count, universe, model, t | sym, f)) {
        return true;
      }
    } else if (neg && !pos) {
      if (SatDpllRecNC(clauses, clause_count, universe, model, t, f | sym)) {
        return true;
      }
    }
  }
  // FIND-UNIT-CLAUSE: exactly one unassigned literal — assign it true.
  for (ISC i = 0; i < clause_count; ++i) {
    IUD open = (clauses[i].positive | clauses[i].negative) & unassigned;
    if (open == 0) {
      continue;
    }
    if ((open & (open - 1)) == 0) {  // single bit
      if (clauses[i].positive & open) {
        if (SatDpllRecNC(clauses, clause_count, universe, model, t | open, f)) {
          return true;
        }
      } else {
        if (SatDpllRecNC(clauses, clause_count, universe, model, t, f | open)) {
          return true;
        }
      }
    }
  }
  // Branch on the first unassigned symbol.
  IUD first = unassigned;
  first &= -first;
  if (SatDpllRecNC(clauses, clause_count, universe, model, t | first, f)) {
    return true;
  }
  return SatDpllRecNC(clauses, clause_count, universe, model, t, f | first);
}

BOL SatDpll(const SatClause* clauses, ISC clause_count, IUD universe) {
  if (clauses == NILP || clause_count < 1) {
    return false;
  }
  return SatDpllRecNC(clauses, clause_count, universe, NILP, 0, 0);
}

// --- Fig 7.18 SatWalksat -----------------------------------------------------

static IUD SatSatisfiedCountNC(const SatClause* clauses, ISC clause_count,
                               IUD m, IUD universe) {
  IUD f = universe & ~m;
  ISC n = 0;
  for (ISC i = 0; i < clause_count; ++i) {
    if (SatClauseTrueNC(&clauses[i], m, f)) {
      n++;
    }
  }
  return n;
}

BOL SatWalksat(const SatClause* clauses, ISC clause_count, IUD universe, FPC p,
               ISC max_flips, IUD seed, IUD* out_model) {
  if (clauses == NILP || clause_count < 1 || universe == 0) {
    return false;
  }
  IUD rnd = SatSeedNC(seed);
  // model <- a random assignment of true/false to the symbols
  rnd = SatLcgStep(rnd);
  rnd = SatLcgStep(rnd);
  IUD m = (rnd | SatLcgStep(rnd)) & universe;
  for (ISC i = 1; i <= max_flips; ++i) {
    if (SatSatisfiedCountNC(clauses, clause_count, m, universe) ==
        IUD(clause_count)) {
      if (out_model != NILP) {
        *out_model = m;
      }
      return true;
    }
    // a randomly selected clause that is false in model
    rnd = SatLcgStep(rnd);
    ISC picked = -1;
    for (ISC tries = 0; tries < 64; ++tries) {
      ISC c = IUC(rnd % IUD(clause_count));
      if (!SatClauseTrueNC(&clauses[c], m, universe & ~m)) {
        picked = c;
        break;
      }
      rnd = SatLcgStep(rnd);
    }
    if (picked == -1) {
      picked = 0;
      for (ISC c = 0; c < clause_count; ++c) {
        if (!SatClauseTrueNC(&clauses[c], m, universe & ~m)) {
          picked = c;
          break;
        }
      }
    }
    const SatClause* cl = &clauses[picked];
    IUD lits = cl->positive | cl->negative;
    if (lits == 0) {
      continue;
    }
    // with probability p: flip a randomly selected symbol from the clause
    rnd = SatLcgStep(rnd);
    if (FPC(rnd >> 32) * kSatUnit < p) {
      IUD s = lits;
      IUD pick = (rnd & -rnd) & lits;  // random bit of lits
      if (pick == 0) {
        pick = s & -s;
      }
      m ^= pick;
    } else {
      // flip the symbol in the clause maximizing the satisfied-clause count
      IUD best_sym = lits & -lits;
      IUD best_n =
          SatSatisfiedCountNC(clauses, clause_count, m ^ best_sym, universe);
      IUD s = lits;
      while (s != 0) {
        IUD pick = s & -s;
        s &= s - 1;
        if (pick == best_sym) {
          continue;
        }
        IUD n = SatSatisfiedCountNC(clauses, clause_count, m ^ pick,
                                    universe);
        if (n > best_n) {
          best_n = n;
          best_sym = pick;
        }
      }
      m ^= best_sym;
    }
  }
  if (SatSatisfiedCountNC(clauses, clause_count, m, universe) ==
      IUD(clause_count)) {
    if (out_model != NILP) {
      *out_model = m;
    }
    return true;
  }
  return false;
}

// --- Fig 7.1 TKBAgent ---------------------------------------------------------

TKBAgent::TKBAgent() : kb_mem_(NILP, &ObjectFactoryHeap), kb_count_(0),
                       kb_cap_(0) {}

TKBAgent::~TKBAgent() {
  kb_mem_.origin = SatFree(kb_mem_.origin);
}

BOL TKBAgent::Init(ISC kb_cap) {
  if (kb_cap < 1) {
    return false;
  }
  kb_cap_ = kb_cap;
  kb_count_ = 0;
  kb_mem_.origin =
      ObjectFactoryHeap(NILP, IUD(kb_cap) * IUD(sizeof(SatClause)));
  if (IsError(kb_mem_.origin)) {
    kb_mem_.origin = NILP;
    kb_cap_ = 0;
    return false;
  }
  SatWipe(kb_mem_.origin, IUD(kb_cap) * IUD(sizeof(SatClause)));
  return true;
}

const SatClause* TKBAgent::Clauses() const {
  return TPtr<SatClause>(kb_mem_.origin);
}

BOL TKBAgent::Tell(const SatClause* clauses, ISC count) {
  if (clauses == NILP || count < 0) {
    return false;
  }
  for (ISC i = 0; i < count; ++i) {
    if (kb_count_ >= kb_cap_) {
      ISC nc = kb_cap_ * 2;
      IUW* nm = ObjectFactoryHeap(NILP, IUD(nc) * IUD(sizeof(SatClause)));
      if (IsError(nm)) {
        return false;
      }
      SatClause* ns = TPtr<SatClause>(nm);
      const SatClause* os = TPtr<SatClause>(kb_mem_.origin);
      for (ISC j = 0; j < kb_count_; ++j) {
        ns[j] = os[j];
      }
      kb_mem_.origin = SatFree(kb_mem_.origin);
      kb_mem_.origin = nm;
      kb_cap_ = nc;
    }
    TPtr<SatClause>(kb_mem_.origin)[kb_count_++] = clauses[i];
  }
  return true;
}

BOL TKBAgent::Ask(const SatClause* query_neg, ISC count, IUD universe) const {
  // KB entails the query iff KB ∧ ¬query is unsatisfiable (DPLL).
  ISC total = kb_count_ + count;
  if (total < 1) {
    return false;
  }
  IUW* mem = ObjectFactoryHeap(NILP, IUD(total) * IUD(sizeof(SatClause)));
  if (IsError(mem)) {
    return false;
  }
  SatClause* all = TPtr<SatClause>(mem);
  const SatClause* kb = TPtr<SatClause>(kb_mem_.origin);
  for (ISC i = 0; i < kb_count_; ++i) {
    all[i] = kb[i];
  }
  for (ISC i = 0; i < count; ++i) {
    all[kb_count_ + i] = query_neg[i];
  }
  BOL ok = !SatDpllRecNC(all, total, universe, NILP, 0, 0);
  mem = SatFree(mem);
  return ok;
}

// --- Fig 7.22 SatPlan ----------------------------------------------------------

/* Time-indexed atom id: atom a at time t of horizon t (0..t). */
static inline IUD SatPlanAtomId(ISC atom, ISC t, ISC horizon) {
  return IUD(atom) * IUD(horizon + 1) + IUD(t);
}

/* Index of the single set bit in m (m has exactly one bit). */
static inline ISC SatPlanIndexNC(IUD m) {
  ISC i = 0;
  while ((m & 1) == 0) {
    m >>= 1;
    ++i;
  }
  return i;
}

BOL SatPlan(const SatPlanProblem* problem, ISC t_max, IUD* model_out,
           IUC* out_plan, ISC plan_cap) {
  if (problem == NILP || problem->actions == NILP ||
      problem->atom_count < 1 || problem->atom_count > 6 ||
      problem->action_count < 1 || t_max < 0 || plan_cap < 0) {
    return false;
  }
  IUD atom_mask = (IUD(1) << problem->atom_count) - 1;
  // The book runs t upward from 0; the first satisfiable t is the plan.
  for (ISC t = 0; t <= t_max; ++t) {
    SatClauseSet cs;
    BOL ok = true;
    // Time 0: closed world over the initial atoms.
    for (ISC a = 0; a < problem->atom_count && ok; ++a) {
      SatClause c;
      c.positive = 0;
      c.negative = 0;
      IUD bit = IUD(1) << SatPlanAtomId(a, 0, t);
      if (problem->init_atoms & (IUD(1) << a)) {
        c.positive = bit;
      } else {
        c.negative = bit;
      }
      ok = cs.Add(&c) != -1;
    }
    // Action axioms for each step tau = 0..t-1 and each action i.
    for (ISC tau = 0; tau < t && ok; ++tau) {
      for (ISC i = 0; i < problem->action_count && ok; ++i) {
        const SatPlanAction* act = &problem->actions[i];
        IUD a_bit = IUD(1) << SatPlanAtomId(i, tau, t);
        // Preconditions: a_i(tau) -> p(tau)
        IUD pre = act->pre & atom_mask;
        while (pre != 0) {
          IUD pick = pre & -pre;
          pre &= pre - 1;
          SatClause c;
          c.positive = 0;
          c.negative = a_bit;
          c.positive = IUD(1) << SatPlanAtomId(SatPlanIndexNC(pick), tau, t);
          ok = cs.Add(&c) != -1;
        }
        // Effects: a_i(tau) -> r(tau+1)
        IUD add = act->add & atom_mask;
        while (add != 0) {
          IUD pick = add & -add;
          add &= add - 1;
          SatClause c;
          c.positive = 0;
          c.negative = a_bit;
          c.positive =
              IUD(1) << SatPlanAtomId(SatPlanIndexNC(pick), tau + 1, t);
          ok = cs.Add(&c) != -1;
        }
        // Deletion axioms: a_i(tau) -> ¬r(tau+1)
        IUD del = act->del & atom_mask;
        while (del != 0) {
          IUD pick = del & -del;
          del &= del - 1;
          SatClause c;
          c.positive = 0;
          c.negative = a_bit |
                       (IUD(1) << SatPlanAtomId(SatPlanIndexNC(pick), tau + 1,
                                                t));
          ok = cs.Add(&c) != -1;
        }
        // Persistence: (r(tau) ∧ ¬a_i(tau)) -> r(tau+1) for r not in del.
        IUD persist = atom_mask & ~act->del;
        while (persist != 0) {
          IUD pick = persist & -persist;
          persist &= persist - 1;
          SatClause c;
          c.positive =
              IUD(1) << SatPlanAtomId(SatPlanIndexNC(pick), tau + 1, t);
          c.negative =
              a_bit | (IUD(1) << SatPlanAtomId(SatPlanIndexNC(pick), tau, t));
          ok = cs.Add(&c) != -1;
        }
      }
      // Only-if effects: r(tau+1) -> r(tau) ∨ (some action adding r at tau).
      // This blocks atoms from appearing without a cause.
      for (ISC r = 0; r < problem->atom_count && ok; ++r) {
        IUD r_bit = IUD(1) << r;
        IUD adds = 0;
        for (ISC i = 0; i < problem->action_count; ++i) {
          if (problem->actions[i].add & r_bit) {
            adds |= IUD(1) << SatPlanAtomId(i, tau, t);
          }
        }
        SatClause c;
        c.positive =
            (IUD(1) << SatPlanAtomId(r, tau, t)) | adds;
        c.negative = IUD(1) << SatPlanAtomId(r, tau + 1, t);
        ok = cs.Add(&c) != -1;
      }
    }
    // The goal at the final step.
    IUD goal = problem->goal_atoms & atom_mask;
    while (goal != 0 && ok) {
      IUD pick = goal & -goal;
      goal &= goal - 1;
      SatClause c;
      c.positive = 0;
      c.negative = 0;
      c.positive = IUD(1) << SatPlanAtomId(SatPlanIndexNC(pick), t, t);
      ok = cs.Add(&c) != -1;
    }
    if (!ok) {
      cs.mem = SatFree(cs.mem);
      return false;
    }
    // SAT-SOLVER: DPLL with model extraction.
    IUD model = 0;
    const SatClause* cl = TPtr<SatClause>(cs.mem);
    BOL sat = SatDpllRecNC(cl, cs.count, ~IUD(0), &model, 0, 0);
    if (!sat) {
      cs.mem = SatFree(cs.mem);
      continue;
    }
    // EXTRACT-SOLUTION: the action symbols true in the model, step by step.
    BOL good = true;
    for (ISC tau = 0; tau < t; ++tau) {
      if (tau >= plan_cap) {
        good = false;
        break;
      }
      BOL found = false;
      for (ISC i = 0; i < problem->action_count; ++i) {
        if (model & (IUD(1) << SatPlanAtomId(i, tau, t))) {
          out_plan[tau] = IUC(i);
          found = true;
          break;
        }
      }
      if (!found) {
        good = false;
        break;
      }
    }
    if (!good) {
      cs.mem = SatFree(cs.mem);
      continue;
    }
    if (model_out != NILP) {
      *model_out = model;
    }
    cs.mem = SatFree(cs.mem);
    return true;
  }
  return false;  // failure: no plan up to T max
}

// --- Fig 7.20 THybridWumpusAgent (2x2 grid) ----------------------------------
//
// Symbol universe (t = 0..5, 56 symbols, all IUD masks):
//   OK(x,y)      bit y*2+x            0..3
//   W(x,y)       bit 4 + y*2+x        4..7
//   Stench_t     bit 8 + t            8..13
//   Breeze_t     bit 14 + t           14..19
//   Glitter_t    bit 20 + t           20..25
//   HaveArrow_t  bit 26 + t           26..31
//   At(x,y,t)    bit 32 + t*4 + y*2+x 32..55
//
// The KB holds generalized definite (Horn) clauses: positive premises plus
// negative premises imply one conclusion literal. The book's atemporal
// "wumpus physics" is built at Init(); time-indexed facts are TELLed each
// Percept step, and a fixpoint chain drives the inference.

static const ISC kWmpTMax = 5;  // time steps supported by the symbol table

static inline IUD WmpOK(ISC x, ISC y) {
  return IUD(1) << (y * 2 + x);
}
static inline IUD WmpW(ISC x, ISC y) {
  return IUD(1) << (4 + y * 2 + x);
}
static inline IUD WmpStench(ISC t) {
  return IUD(1) << (8 + t);
}
static inline IUD WmpBreeze(ISC t) {
  return IUD(1) << (14 + t);
}
static inline IUD WmpGlitter(ISC t) {
  return IUD(1) << (20 + t);
}
static inline IUD WmpHaveArrow(ISC t) {
  return IUD(1) << (26 + t);
}
static inline IUD WmpAt(ISC x, ISC y, ISC t) {
  return IUD(1) << (32 + t * 4 + y * 2 + x);
}
static inline IUC WmpSquare(IUC x, IUC y) {
  return y * 2 + x;
}
static inline IUC WmpX(IUC sq) {
  return sq % 2;
}
static inline IUC WmpY(IUC sq) {
  return sq / 2;
}

/* Horn clause: pos ∧ ¬neg -> (con_neg ? ¬con : con). */
struct WmpHorn {
  IUD pos;
  IUD neg;
  IUD con;
  BOL con_neg;
};

/* PLAN-ROUTE callbacks for the Problem interface (SearchBFS). State is a
 * single packed word: heading * 4 + square (heading 0=N,1=E,2=S,3=W;
 * square y*2+x). The allowed/goals masks are per-call context — the
 * reference driver is single-threaded. */
static IUD wmp_allowed_ = 0;
static IUD wmp_goals_ = 0;

static BOL WmpGoalTestNC(const IUC* state, ISC state_words) {
  (void)state_words;
  return (wmp_goals_ & (IUD(1) << (*state % 4))) != 0;
}

static ISC WmpActionsNC(const IUC* state, ISC state_words, IUC* out, ISC cap) {
  (void)state_words;
  IUC h = *state / 4;
  IUC sq = *state % 4;
  ISC n = 0;
  if (n < cap) {
    out[n++] = kWmpLeft;
  }
  if (n < cap) {
    out[n++] = kWmpRight;
  }
  // Forward, only when the target square is in bounds and allowed.
  IUC x = WmpX(sq);
  IUC y = WmpY(sq);
  IUC ns = 0;
  BOL can = false;
  switch (h) {
    case 0:
      can = (y != 1);
      ns = sq + 2;
      break;
    case 1:
      can = (x != 1);
      ns = sq + 1;
      break;
    case 2:
      can = (y != 0);
      ns = sq - 2;
      break;
    default:
      can = (x != 0);
      ns = sq - 1;
      break;
  }
  if (can && (wmp_allowed_ & (IUD(1) << ns)) != 0 && n < cap) {
    out[n++] = kWmpForward;
  }
  return n;
}

static void WmpResultNC(const IUC* state, ISC state_words, IUC action,
                        IUC* out) {
  (void)state_words;
  IUC h = *state / 4;
  IUC sq = *state % 4;
  switch (action) {
    case kWmpLeft:
      h = (h + 3) % 4;
      break;
    case kWmpRight:
      h = (h + 1) % 4;
      break;
    case kWmpForward: {
      IUC x = WmpX(sq);
      IUC y = WmpY(sq);
      switch (h) {
        case 0:
          sq += 2;
          break;
        case 1:
          sq += 1;
          break;
        case 2:
          sq -= 2;
          break;
        default:
          sq -= 1;
          break;
      }
      (void)x;
      (void)y;
      break;
    }
    default:
      break;
  }
  *out = h * 4 + sq;
}

THybridWumpusAgent::THybridWumpusAgent()
    : kb_mem_(NILP, &ObjectFactoryHeap),
      kb_count_(0),
      kb_cap_(0),
      t_(0),
      x_(0),
      y_(0),
      heading_(0),
      visited_(1),
      known_true_(0),
      known_false_(0),
      arrow_(true),
      done_(false),
      plan_count_(0) {}

THybridWumpusAgent::~THybridWumpusAgent() {
  kb_mem_.origin = SatFree(kb_mem_.origin);
}

BOL THybridWumpusAgent::Init(ISC kb_cap) {
  if (kb_cap < 96) {  // 84 atemporal clauses (+ headroom)
    return false;
  }
  kb_cap_ = kb_cap;
  kb_count_ = 0;
  kb_mem_.origin = ObjectFactoryHeap(NILP, IUD(kb_cap) * IUD(sizeof(WmpHorn)));
  if (IsError(kb_mem_.origin)) {
    kb_mem_.origin = NILP;
    kb_cap_ = 0;
    return false;
  }
  SatWipe(kb_mem_.origin, IUD(kb_cap) * IUD(sizeof(WmpHorn)));
  WmpHorn* kb = TPtr<WmpHorn>(kb_mem_.origin);
  // (a,b) adjacent to (x,y) in the 2x2 grid
  for (ISC x = 0; x < 2; ++x) {
    for (ISC y = 0; y < 2; ++y) {
      // W(a,b) -> ¬OK(x,y) for each edge-adjacent (a,b)
      for (ISC ax = 0; ax < 2; ++ax) {
        for (ISC ay = 0; ay < 2; ++ay) {
          if ((ax != x) == (ay != y)) {
            continue;  // same square or diagonal: not adjacent
          }
          WmpHorn c;
          c.pos = WmpW(ax, ay);
          c.neg = 0;
          c.con = WmpOK(x, y);
          c.con_neg = true;
          kb[kb_count_++] = c;
        }
      }
      // ¬Stench_t ∧ At(x,y,t) -> OK(x,y): the current-square inference
      for (ISC t = 0; t <= kWmpTMax; ++t) {
        WmpHorn c;
        c.pos = WmpAt(x, y, t);
        c.neg = WmpStench(t);
        c.con = WmpOK(x, y);
        c.con_neg = false;
        kb[kb_count_++] = c;
      }
      // (¬W(a1,b1) ∧ ... ) -> OK(x,y) for adjacent squares: the
      // unvisited-square inference
      {
        WmpHorn c;
        c.pos = 0;
        c.neg = 0;
        for (ISC ax = 0; ax < 2; ++ax) {
          for (ISC ay = 0; ay < 2; ++ay) {
            if ((ax != x) == (ay != y)) {
              continue;  // same square or diagonal: not adjacent
            }
            c.neg |= WmpW(ax, ay);
          }
        }
        c.con = WmpOK(x, y);
        c.con_neg = false;
        kb[kb_count_++] = c;
      }
      // W(a,b) ∧ At(x,y,t) -> Stench_t for each edge-adjacent (a,b)
      for (ISC ax = 0; ax < 2; ++ax) {
        for (ISC ay = 0; ay < 2; ++ay) {
          if ((ax != x) == (ay != y)) {
            continue;  // same square or diagonal: not adjacent
          }
          for (ISC t = 0; t <= kWmpTMax; ++t) {
            WmpHorn c;
            c.pos = WmpW(ax, ay) | WmpAt(x, y, t);
            c.neg = 0;
            c.con = WmpStench(t);
            c.con_neg = false;
            kb[kb_count_++] = c;
          }
        }
      }
    }
  }
  t_ = 0;
  x_ = 0;
  y_ = 0;
  visited_ = 1;  // the start square (0,0) is lit
  arrow_ = true;
  done_ = false;
  return true;
}

/* Assert a symbol true (or false) if not already known; returns whether
 * it was new. */
static inline BOL WmpAssert(IUD* known_true, IUD* known_false, IUD sym,
                            BOL is_true) {
  if (is_true) {
    if (*known_true & sym) {
      return false;
    }
    *known_true |= sym;
    return true;
  }
  if (*known_false & sym) {
    return false;
  }
  *known_false |= sym;
  return true;
}

/* TELL the t_-th percept: the raw facts, then the fixpoint chain. */
void THybridWumpusAgent::TellPercept(const WumpusPercept* p, BOL moved,
                                     IUC x, IUC y) {
  IUD* kt = &known_true_;
  IUD* kf = &known_false_;
  (void)WmpAssert(kt, kf, WmpStench(t_), p->stench);
  (void)WmpAssert(kt, kf, WmpBreeze(t_), p->breeze);
  (void)WmpAssert(kt, kf, WmpGlitter(t_), p->glitter);
  (void)WmpAssert(kt, kf, WmpHaveArrow(t_), arrow_);
  IUC sq = WmpSquare(x, y);
  // The atemporal physics: the agent is not on top of the wumpus.
  (void)WmpAssert(kt, kf, WmpW(x, y), false);
  (void)WmpAssert(kt, kf, WmpAt(x, y, t_), true);
  for (ISC o = 0; o < 4; ++o) {
    if (IUC(o) != sq) {
      (void)WmpAssert(kt, kf,
                      WmpAt(WmpX(o), WmpY(o), t_), false);
    }
  }
  if (moved) {
    visited_ |= IUD(1) << sq;
  }
  x_ = x;
  y_ = y;
  // Fixpoint chain over the Horn KB.
  for (;;) {
    BOL fired = false;
    const WmpHorn* kb = TPtr<WmpHorn>(kb_mem_.origin);
    for (ISC i = 0; i < kb_count_; ++i) {
      const WmpHorn* c = &kb[i];
      if (c->con_neg ? *kf & c->con : *kt & c->con) {
        continue;  // conclusion already asserted
      }
      IUD open = (c->pos & ~*kt) | (c->neg & ~*kf);
      if (open == 0) {
        (void)WmpAssert(kt, kf, c->con, !c->con_neg);
        fired = true;
      }
    }
    if (!fired) {
      break;
    }
  }
}

BOL THybridWumpusAgent::Safe(IUC x, IUC y) const {
  return (known_true_ & WmpOK(x, y)) != 0;
}

BOL THybridWumpusAgent::NotWumpus(IUC x, IUC y) const {
  return (known_false_ & WmpW(x, y)) != 0;
}

/* PLAN-ROUTE: append the relative actions (Forward/Left/Right) that move the
 * agent from `from` to one of `goals` through `allowed | goals` squares
 * (SearchBFS over the packed (heading, square) state). */
BOL THybridWumpusAgent::Route(IUC from, IUD goals, IUD allowed) {
  IUC init = heading_ * 4 + from;
  Problem prob;
  prob.initial = &init;
  prob.state_words = 1;
  prob.GoalTest = WmpGoalTestNC;
  prob.Actions = WmpActionsNC;
  prob.Result = WmpResultNC;
  prob.StepCost = NILP;
  prob.Heuristic = NILP;
  wmp_allowed_ = allowed | goals;
  wmp_goals_ = goals;
  IUC acts[32];
  ISC n = SearchBFS(&prob, acts, 32);
  if (n < 0) {
    return false;
  }
  for (ISC i = 0; i < n && plan_count_ < 32; ++i) {
    plan_[plan_count_++] = acts[i];
  }
  return true;
}

static inline IUC WmpShootDir(ISC dx, ISC dy) {
  if (dy == 1) {
    return kWmpShootN;
  }
  if (dy == -1) {
    return kWmpShootS;
  }
  if (dx == 1) {
    return kWmpShootE;
  }
  return kWmpShootW;
}

/* PLAN-SHOT: append the actions to shoot a possible wumpus square: shoot it
 * directly when adjacent, or route to an adjacent safe square and shoot. */
BOL THybridWumpusAgent::PlanShot(IUC current, IUD possible, IUD safe) {
  while (possible != 0) {
    IUD pick = possible & -possible;
    possible &= possible - 1;
    IUC w = 0;
    IUD m = pick;
    while ((m & 1) == 0) {
      w++;
      m >>= 1;
    }
    if (w == current) {
      continue;  // the wumpus is not in the square we stand on
    }
    ISC dx = ISC(WmpX(w)) - ISC(WmpX(current));
    ISC dy = ISC(WmpY(w)) - ISC(WmpY(current));
    if (dx == 0 || dy == 0) {
      // adjacent: shoot now
      plan_[plan_count_++] = WmpShootDir(dx, dy);
      return true;
    }
    // diagonal: route to an intermediate square adjacent to the wumpus
    for (ISC k = 0; k < 2; ++k) {
      IUC sx = (k == 0) ? WmpX(w) : WmpX(current);
      IUC sy = (k == 0) ? WmpY(current) : WmpY(w);
      IUC mid = WmpSquare(sx, sy);
      if (mid == current || (safe & (IUD(1) << mid)) == 0) {
        continue;
      }
      if (!Route(current, IUD(1) << mid, safe)) {
        continue;
      }
      ISC mdx = ISC(WmpX(w)) - ISC(sx);
      ISC mdy = ISC(WmpY(w)) - ISC(sy);
      plan_[plan_count_++] = WmpShootDir(mdx, mdy);
      return true;
    }
  }
  return false;
}

IUC THybridWumpusAgent::Percept(const WumpusPercept* p, BOL moved, IUC x,
                                IUC y) {
  if (p == NILP || done_) {
    return kWmpClimb;
  }
  if (t_ > kWmpTMax) {
    done_ = true;
    return kWmpClimb;
  }
  // 1. TELL the percept (facts + chain).
  TellPercept(p, moved, x, y);
  // 2. Action selection (the book's plan logic).
  plan_count_ = 0;
  IUD safe = 0;
  for (ISC s = 0; s < 4; ++s) {
    if ((known_true_ & WmpOK(WmpX(s), WmpY(s))) != 0 ||
        IUC(s) == WmpSquare(x_, y_)) {
      safe |= IUD(1) << s;
    }
  }
  if ((known_true_ & WmpGlitter(t_)) != 0) {
    plan_[plan_count_++] = kWmpGrab;
    (void)Route(WmpSquare(x_, y_), IUD(1) << 3, safe);
    plan_[plan_count_++] = kWmpClimb;
  } else if (plan_count_ == 0) {
    IUD unvisited = ~visited_ & IUD(3);
    IUD cand = unvisited & safe;
    if (cand != 0) {
      Route(WmpSquare(x_, y_), cand, safe);
    }
    if (plan_count_ == 0 && arrow_) {
      // possible wumpus: ¬W not yet known
      IUD possible = 0;
      for (ISC s = 0; s < 4; ++s) {
        if ((known_false_ & WmpW(WmpX(s), WmpY(s))) == 0) {
          possible |= IUD(1) << s;
        }
      }
      possible &= ~IUD(1) << WmpSquare(x_, y_);
      PlanShot(WmpSquare(x_, y_), possible, safe);
      if (plan_count_ == 0) {
        // no choice but to take a risk
        IUD not_unsafe = 0;
        for (ISC s = 0; s < 4; ++s) {
          if ((known_true_ & WmpOK(WmpX(s), WmpY(s))) == 0 &&
              (known_false_ & WmpOK(WmpX(s), WmpY(s))) == 0) {
            not_unsafe |= IUD(1) << s;
          }
        }
        IUD cand2 = unvisited & not_unsafe;
        if (cand2 != 0) {
          Route(WmpSquare(x_, y_), cand2, safe);
        }
      }
    }
    if (plan_count_ == 0) {
      Route(WmpSquare(x_, y_), 1, safe);
      plan_[plan_count_++] = kWmpClimb;
      if (plan_count_ == 1) {  // route failed
        plan_[0] = (x_ == 0 && y_ == 0) ? kWmpClimb : kWmpForward;
      }
    }
  }
  // 3. POP(plan); record the action (MAKE-ACTION-SENTENCE).
  IUC action = plan_[0];
  for (ISC i = 0; i + 1 < plan_count_; ++i) {
    plan_[i] = plan_[i + 1];
  }
  plan_count_--;
  switch (action) {
    case kWmpLeft:
      heading_ = (heading_ + 3) % 4;
      break;
    case kWmpRight:
      heading_ = (heading_ + 1) % 4;
      break;
    case kWmpShootN:
    case kWmpShootS:
    case kWmpShootE:
    case kWmpShootW:
      arrow_ = false;
      break;
    case kWmpClimb:
      done_ = true;
      break;
    default:
      break;
  }
  t_++;
  return action;
}

} //< namespace iGeek
#endif
