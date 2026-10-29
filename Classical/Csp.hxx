// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_CSP_IMPL
#define IGEEK_CLASSICAL_CSP_IMPL
#include "Csp.h"
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
// Domains are IUC bitmasks, so var_count and domain_count are both <= 32.

// Sentinel for "unassigned" (0xFFFFFFFF is never a real value).
static const IUC kCspUnassigned = 0xFFFFFFFF;

static inline IUW* CspFree(IUW* origin) {
  return ObjectFactoryHeap(origin, RAMFactoryDelete);
}

static inline void CspWipe(IUW* origin, IUD bytes) {
  IUD aligned = bytes + ((-bytes) & IUD(ACPUWordMask));
  IUW* p = TPtr<IUW>(origin);
  for (IUD i = 0; i < aligned >> ACPUBytesLog2; ++i) {
    p[i] = 0;
  }
}

/* Popcount of a domain bitmask (bits above domain_count are never set). */
static ISC CspDomainSizeNC(IUC mask) {
  ISC n = 0;
  IUC m = mask;
  while (m != 0) {
    n += ISC(m & 1);
    m >>= 1;
  }
  return n;
}

/* Compatibility of (i = vi, j = vj) under c, in c's argument order. */
static BOL CspPairOK(const CspConstraint* c, ISC i, ISC j, IUC vi, IUC vj) {
  if (c->var_a == i) {
    return c->Compatible(vi, vj);
  }
  return c->Compatible(vj, vi);
}

/* Fig 6.3 REVISE: delete values of variable i with no support in j. Returns
 * true when the domain of i changed. */
static BOL CspReviseNC(const CSP* csp, ISC i, ISC j, IUC* d) {
  BOL revised = false;
  IUC di = d[i];
  IUC dj = d[j];
  for (IUC x = 0; x < IUC(csp->domain_count); ++x) {
    if ((di & (IUC(1) << x)) == 0) {
      continue;
    }
    BOL support = false;
    for (IUC y = 0; y < IUC(csp->domain_count) && !support; ++y) {
      if ((dj & (IUC(1) << y)) == 0) {
        continue;
      }
      for (ISC c = 0; c < csp->constraint_count; ++c) {
        const CspConstraint* cc = &csp->constraints[c];
        if ((cc->var_a == i && cc->var_b == j) ||
            (cc->var_a == j && cc->var_b == i)) {
          if (CspPairOK(cc, i, j, x, y)) {
            support = true;
            break;
          }
        }
      }
    }
    if (!support) {
      d[i] &= ~(IUC(1) << x);
      revised = true;
    }
  }
  return revised;
}

/* FIFO arc queue over a RAMFactory block, growing x2. Arcs pack (i << 32) | j. */
struct CspArcQueue {
  IUW* mem;
  ISC cap;
  ISC count;
  ISC head;
  CspArcQueue() : mem(NILP), cap(0), count(0), head(0) {}
  CspArcQueue(const CspArcQueue&) = delete;
  CspArcQueue& operator=(const CspArcQueue&) = delete;
  ~CspArcQueue() {
    Free();
  }
  BOL Init(ISC expected) {
    cap = 16;
    while (cap < expected) {
      cap <<= 1;
    }
    mem = ObjectFactoryHeap(NILP, IUD(cap) * IUD(sizeof(IUD)));
    if (IsError(mem)) {
      mem = NILP;
      cap = 0;
      return false;
    }
    CspWipe(mem, IUD(cap) * IUD(sizeof(IUD)));
    count = 0;
    head = 0;
    return true;
  }
  IUD* Items() const {
    return TPtr<IUD>(mem);
  }
  BOL Push(IUD arc) {
    if (count + 1 > cap) {
      ISC nc = cap * 2;
      IUW* nm = ObjectFactoryHeap(NILP, IUD(nc) * IUD(sizeof(IUD)));
      if (IsError(nm)) {
        return false;
      }
      IUD* ni = TPtr<IUD>(nm);
      for (ISC i = 0; i < count - head; ++i) {
        ni[i] = TPtr<IUD>(mem)[head + i];
      }
      mem = CspFree(mem);
      mem = nm;
      cap = nc;
      head = 0;
    }
    TPtr<IUD>(mem)[count++] = arc;
    return true;
  }
  BOL Empty() const {
    return count == head;
  }
  IUD Pop() {
    return TPtr<IUD>(mem)[head++];
  }
  void Free() {
    mem = CspFree(mem);
  }
};

/* Fig 6.3 AC-3: queue every arc of every constraint (both directions) and
 * REVISE until the queue drains. Returns false when a domain is wiped. */
BOL CspAc3(CSP* csp, IUC* d) {
  CspArcQueue q;
  if (!q.Init(csp->constraint_count * 2)) {
    return false;
  }
  for (ISC c = 0; c < csp->constraint_count; ++c) {
    const CspConstraint* cc = &csp->constraints[c];
    if (!q.Push(IUD(IUC(cc->var_a)) << 32 | IUD(IUC(cc->var_b))) ||
        !q.Push(IUD(IUC(cc->var_b)) << 32 | IUD(IUC(cc->var_a)))) {
      return false;
    }
  }
  while (!q.Empty()) {
    IUD arc = q.Pop();
    ISC i = ISC(arc >> 32);
    ISC j = ISC(arc & 0xFFFFFFFF);
    if (CspReviseNC(csp, i, j, d)) {
      if (d[i] == 0) {
        return false;
      }
      for (ISC c = 0; c < csp->constraint_count; ++c) {
        const CspConstraint* cc = &csp->constraints[c];
        ISC k = -1;
        if (cc->var_a == i) {
          k = cc->var_b;
        } else if (cc->var_b == i) {
          k = cc->var_a;
        }
        if (k == j || k == -1) {
          continue;
        }
        if (!q.Push(IUD(IUC(k)) << 32 | IUD(IUC(i)))) {
          return false;
        }
      }
    }
  }
  return true;
}

/* Undo log for the backtracker: every domain write is logged (variable,
 * previous mask). PopTo restores all entries above save_top. */
struct CspTrail {
  struct Entry {
    ISC var;
    IUC old_mask;
  };
  IUW* mem;
  ISC cap;
  ISC top;
  CspTrail() : mem(NILP), cap(0), top(0) {}
  CspTrail(const CspTrail&) = delete;
  CspTrail& operator=(const CspTrail&) = delete;
  ~CspTrail() {
    Free();
  }
  Entry* Entries() const {
    return TPtr<Entry>(mem);
  }
  BOL Init(ISC expected) {
    cap = 16;
    while (cap < expected) {
      cap <<= 1;
    }
    mem = ObjectFactoryHeap(NILP, IUD(cap) * IUD(sizeof(Entry)));
    if (IsError(mem)) {
      mem = NILP;
      cap = 0;
      return false;
    }
    top = 0;
    return true;
  }
  BOL Push(ISC var, IUC old_mask) {
    if (top + 1 > cap) {
      ISC nc = cap * 2;
      IUW* nm = ObjectFactoryHeap(NILP, IUD(nc) * IUD(sizeof(Entry)));
      if (IsError(nm)) {
        return false;
      }
      Entry* ne = TPtr<Entry>(nm);
      for (ISC i = 0; i < top; ++i) {
        ne[i] = TPtr<Entry>(mem)[i];
      }
      mem = CspFree(mem);
      mem = nm;
      cap = nc;
    }
    Entry* e = &TPtr<Entry>(mem)[top++];
    e->var = var;
    e->old_mask = old_mask;
    return true;
  }
  void PopTo(IUC* d, ISC save_top) {
    for (ISC i = top - 1; i >= save_top; --i) {
      d[Entries()[i].var] = Entries()[i].old_mask;
    }
    top = save_top;
  }
  void Free() {
    mem = CspFree(mem);
  }
};

/* Consistency of value x for var against the already-assigned variables. */
static BOL CspValueConsistent(const CSP* csp, const IUC* asg, ISC var, IUC x) {
  for (ISC c = 0; c < csp->constraint_count; ++c) {
    const CspConstraint* cc = &csp->constraints[c];
    ISC other = -1;
    if (cc->var_a == var) {
      other = cc->var_b;
    } else if (cc->var_b == var) {
      other = cc->var_a;
    } else {
      continue;
    }
    if (asg[other] == kCspUnassigned) {
      continue;
    }
    if (!CspPairOK(cc, var, other, x, asg[other])) {
      return false;
    }
  }
  return true;
}

/* Conflict count of value x for var against the assigned variables. */
static ISC CspConflictsNC(const CSP* csp, const IUC* asg, ISC var, IUC x) {
  ISC n = 0;
  for (ISC c = 0; c < csp->constraint_count; ++c) {
    const CspConstraint* cc = &csp->constraints[c];
    ISC other = -1;
    if (cc->var_a == var) {
      other = cc->var_b;
    } else if (cc->var_b == var) {
      other = cc->var_a;
    } else {
      continue;
    }
    if (asg[other] == kCspUnassigned) {
      continue;
    }
    if (!CspPairOK(cc, var, other, x, asg[other])) {
      ++n;
    }
  }
  return n;
}

/* Fig 6.5 BACKTRACK: MRV variable selection (fewest remaining values, ties
 * by index), ascending value order, forward domain filtering as INFERENCE.
 * Every domain write goes to the trail; on failure the trail restores. */
static BOL CspBacktrackRec(const CSP* csp, IUC* d, IUC* asg, CspTrail* trail,
                           IUC* out) {
  ISC best = -1;
  ISC best_n = 0;
  for (ISC v = 0; v < csp->var_count; ++v) {
    if (asg[v] != kCspUnassigned) {
      continue;
    }
    ISC n = CspDomainSizeNC(d[v]);
    if (n == 0) {
      return false;
    }
    if (best == -1 || n < best_n) {
      best = v;
      best_n = n;
      if (n == 1) {
        break;
      }
    }
  }
  if (best == -1) {
    for (ISC v = 0; v < csp->var_count; ++v) {
      out[v] = asg[v];
    }
    return true;
  }
  for (IUC x = 0; x < IUC(csp->domain_count); ++x) {
    if ((d[best] & (IUC(1) << x)) == 0) {
      continue;
    }
    if (!CspValueConsistent(csp, asg, best, x)) {
      continue;
    }
    ISC save = trail->top;
    if (!trail->Push(best, d[best])) {
      return false;
    }
    d[best] = IUC(1) << x;
    asg[best] = x;
    BOL fail = false;
    for (ISC c = 0; c < csp->constraint_count && !fail; ++c) {
      const CspConstraint* cc = &csp->constraints[c];
      ISC other = -1;
      if (cc->var_a == best) {
        other = cc->var_b;
      } else if (cc->var_b == best) {
        other = cc->var_a;
      }
      if (other == -1 || asg[other] != kCspUnassigned) {
        continue;
      }
      IUC nm = 0;
      for (IUC y = 0; y < IUC(csp->domain_count); ++y) {
        if ((d[other] & (IUC(1) << y)) == 0) {
          continue;
        }
        if (CspPairOK(cc, best, other, x, y)) {
          nm |= IUC(1) << y;
        }
      }
      if (nm != d[other]) {
        if (!trail->Push(other, d[other])) {
          return false;
        }
        d[other] = nm;
        if (nm == 0) {
          fail = true;
        }
      }
    }
    if (!fail) {
      fail = !CspBacktrackRec(csp, d, asg, trail, out);
    }
    trail->PopTo(d, save);
    asg[best] = kCspUnassigned;
    if (!fail) {
      return true;
    }
  }
  return false;
}

BOL CspBacktrack(CSP* csp, const IUC* domains, IUC* out) {
  ISC words = csp->var_count;
  IUW* mem = ObjectFactoryHeap(NILP, IUD(words) * IUD(sizeof(IUC)) * 2);
  if (IsError(mem)) {
    return false;
  }
  IUC* d = TPtr<IUC>(mem);
  IUC* asg = d + words;
  for (ISC v = 0; v < words; ++v) {
    d[v] = domains[v];
    asg[v] = kCspUnassigned;
  }
  CspTrail trail;
  if (!trail.Init(words * 2)) {
    mem = CspFree(mem);
    return false;
  }
  BOL ok = CspBacktrackRec(csp, d, asg, &trail, out);
  mem = CspFree(mem);
  return ok;
}

/* Deterministic LCG for the min-conflicts random choices. */
static inline IUD CspLcgStep(IUD s) {
  return s * 6364136223846793005ULL + 1442695040888963407ULL;
}

BOL CspMinConflicts(CSP* csp, const IUC* domains, IUC* out, ISC max_steps,
                    IUD seed) {
  ISC words = csp->var_count;
  IUW* mem = ObjectFactoryHeap(NILP, IUD(words) * IUD(sizeof(IUC)) * 2);
  if (IsError(mem)) {
    return false;
  }
  IUC* asg = TPtr<IUC>(mem);
  IUC* cv = asg + words;
  // Greedy initial assignment: each variable takes the value with the fewest
  // conflicts against the already-assigned earlier variables.
  for (ISC v = 0; v < words; ++v) {
    IUC best = kCspUnassigned;
    ISC best_c = 0;
    for (IUC x = 0; x < IUC(csp->domain_count); ++x) {
      if ((domains[v] & (IUC(1) << x)) == 0) {
        continue;
      }
      ISC cc = CspConflictsNC(csp, asg, v, x);
      if (best == kCspUnassigned || cc < best_c) {
        best = x;
        best_c = cc;
      }
    }
    asg[v] = best;
  }
  IUD rnd = seed == 0 ? 1442695040888963407ULL : seed;
  for (ISC step = 0; step < max_steps; ++step) {
    ISC nc = 0;
    for (ISC v = 0; v < words; ++v) {
      if (asg[v] == kCspUnassigned) {
        continue;
      }
      if (CspConflictsNC(csp, asg, v, asg[v]) > 0) {
        cv[nc++] = IUC(v);
      }
    }
    if (nc == 0) {
      for (ISC v = 0; v < words; ++v) {
        out[v] = asg[v];
      }
      mem = CspFree(mem);
      return true;
    }
    ISC var = ISC(cv[IUC(rnd % IUD(nc))]);
    rnd = CspLcgStep(rnd);
    IUC best = asg[var];
    ISC best_c = CspConflictsNC(csp, asg, var, best);
    for (IUC x = 0; x < IUC(csp->domain_count); ++x) {
      if ((domains[var] & (IUC(1) << x)) == 0) {
        continue;
      }
      ISC cc = CspConflictsNC(csp, asg, var, x);
      if (cc < best_c) {
        best = x;
        best_c = cc;
      }
    }
    asg[var] = best;
  }
  for (ISC v = 0; v < words; ++v) {
    if (asg[v] == kCspUnassigned ||
        CspConflictsNC(csp, asg, v, asg[v]) > 0) {
      mem = CspFree(mem);
      return false;
    }
  }
  for (ISC v = 0; v < words; ++v) {
    out[v] = asg[v];
  }
  mem = CspFree(mem);
  return true;
}

/* DFS over the adjacency bitmasks: preorder into order[], parent[] set.
 * Returns false on a cycle (back edge) — the constraint graph is not a tree.
 * Depth is bounded by var_count <= 32. */
static BOL CspTreeDfsNC(const IUC* adj, IUC* parent, IUC* order, ISC* order_n,
                        ISC var) {
  order[(*order_n)++] = IUC(var);
  IUC m = adj[var];
  for (ISC k = 0; k < 32; ++k) {
    if ((m & (IUC(1) << k)) == 0) {
      continue;
    }
    if (IUC(k) == parent[var]) {
      continue;
    }
    if (parent[k] != kCspUnassigned) {
      return false;  // back edge: cycle
    }
    parent[k] = IUC(var);
    if (!CspTreeDfsNC(adj, parent, order, order_n, k)) {
      return false;
    }
  }
  return true;
}

BOL CspTree(CSP* csp, const IUC* domains, IUC* out) {
  ISC words = csp->var_count;
  IUW* mem = ObjectFactoryHeap(NILP, IUD(words) * IUD(sizeof(IUC)) * 5);
  if (IsError(mem)) {
    return false;
  }
  IUC* d = TPtr<IUC>(mem);
  IUC* adj = d + words;
  IUC* parent = adj + words;
  IUC* order = parent + words;
  for (ISC v = 0; v < words; ++v) {
    d[v] = domains[v];
    adj[v] = 0;
    parent[v] = kCspUnassigned;
  }
  for (ISC c = 0; c < csp->constraint_count; ++c) {
    const CspConstraint* cc = &csp->constraints[c];
    adj[cc->var_a] |= IUC(1) << cc->var_b;
    adj[cc->var_b] |= IUC(1) << cc->var_a;
  }
  ISC order_n = 0;
  if (!CspTreeDfsNC(adj, parent, order, &order_n, 0) || order_n != words) {
    mem = CspFree(mem);
    return false;  // cycle or disconnected
  }
  // Fig 6.11: leaf-to-root, MAKE-ARC-CONSISTENT(PARENT(Xj), Xj) — one REVISE
  // per arc, deeper arcs first (each parent domain is final when its own arc
  // is processed).
  for (ISC idx = words - 1; idx >= 1; --idx) {
    ISC j = ISC(order[idx]);
    ISC pi = ISC(parent[j]);
    CspReviseNC(csp, pi, j, d);
    if (d[pi] == 0) {
      mem = CspFree(mem);
      return false;
    }
  }
  // Forward pass: any value consistent with the assigned parent.
  for (ISC idx = 0; idx < words; ++idx) {
    ISC i = ISC(order[idx]);
    IUC chosen = kCspUnassigned;
    for (IUC x = 0; x < IUC(csp->domain_count); ++x) {
      if ((d[i] & (IUC(1) << x)) == 0) {
        continue;
      }
      BOL ok = true;
      for (ISC c = 0; c < csp->constraint_count && ok; ++c) {
        const CspConstraint* cc = &csp->constraints[c];
        if (cc->var_a == i) {
          if (cc->var_b != parent[i]) {
            continue;
          }
          ok = CspPairOK(cc, i, parent[i], x, out[cc->var_b]);
        } else if (cc->var_b == i) {
          if (cc->var_a != parent[i]) {
            continue;
          }
          ok = CspPairOK(cc, parent[i], i, out[cc->var_a], x);
        }
      }
      if (parent[i] == kCspUnassigned || ok) {
        chosen = x;
        break;
      }
    }
    if (chosen == kCspUnassigned) {
      mem = CspFree(mem);
      return false;
    }
    out[i] = chosen;
  }
  mem = CspFree(mem);
  return true;
}

} //< namespace iGeek
#endif
