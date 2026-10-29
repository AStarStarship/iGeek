// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_FOL_IMPL
#define IGEEK_CLASSICAL_FOL_IMPL 1
#include "Fol.h"
#include "../ASCIICrabs/Autoject.hxx"
#include "Sat.hxx"
namespace iGeek {

// --- Internal machinery (internal linkage; not module API) ----------------
//
// All RAM is created and destroyed through the RAMFactory
// (ObjectFactoryHeap — the same factory BigInt and AgentState use):
//   alloc: ObjectFactoryHeap(NILP, bytes)  free: ObjectFactoryHeap(origin,
//   RAMFactoryDelete). No naked new/delete/malloc in this folder.

static inline IUW* FolFree(IUW* origin) {
  return ObjectFactoryHeap(origin, RAMFactoryDelete);
}

static inline void FolWipe(IUW* origin, IUD bytes) {
  IUD aligned = bytes + ((-bytes) & IUD(ACPUWordMask));
  IUW* p = TPtr<IUW>(origin);
  for (IUD i = 0; i < aligned >> ACPUBytesLog2; ++i) {
    p[i] = 0;
  }
}

// --- Fig 9.1 UNIFY ---------------------------------------------------------

/* Resolve a node through the substitution (follow var bindings until the
 * node is not a bound var; bounded by theta_n steps — cyclic bindings
 * resolve to a node already seen). */
static IUD FolApplyNC(const TFolTerm* pool, IUD node, const TFolSubst* theta,
                      ISC theta_n) {
  ISC guard = theta_n + 1;
  for (;;) {
    const TFolTerm* t = &pool[node];
    if (t->kind != 1) {
      return node;
    }
    BOL bound = false;
    IUD target = 0;
    for (ISC i = 0; i < theta_n; ++i) {
      if (theta[i].var == ISC(t->value)) {
        target = theta[i].node;
        bound = true;
        break;
      }
    }
    if (!bound) {
      return node;  // unbound var
    }
    if (target == node || guard-- <= 0) {
      return node;  // self-binding or cyclic
    }
    node = target;
  }
}

/* Does var (a pool node id of a var) occur in node, after applying θ at
 * each step (the book's OCCUR-CHECK). */
static BOL FolOccursNC(const TFolTerm* pool, IUD var_node, IUD node,
                       const TFolSubst* theta, ISC theta_n) {
  node = FolApplyNC(pool, node, theta, theta_n);
  const TFolTerm* t = &pool[node];
  if (t->kind == 1) {
    return t->value == pool[var_node].value;
  }
  if (t->kind == 2) {
    return FolOccursNC(pool, var_node, t->left, theta, theta_n) ||
           FolOccursNC(pool, var_node, t->right, theta, theta_n);
  }
  return false;
}

static BOL FolUnifyRecNC(const TFolTerm* pool, IUD x, IUD y,
                         const TFolSubst* theta, ISC theta_n,
                         TFolSubst* out_theta, ISC out_cap, ISC* out_n) {
  // θ grows as the book's: the first child unifies against the input θ; the
  // second child sees the bindings the first added (out_theta, *out_n).
  x = FolApplyNC(pool, x, theta, theta_n);
  y = FolApplyNC(pool, y, theta, theta_n);
  const TFolTerm* tx = &pool[x];
  const TFolTerm* ty = &pool[y];
  if (tx->kind == ty->kind &&
      (tx->kind == 3 ||
       (tx->kind == 0 && tx->value == ty->value) ||
       (tx->kind == 1 && tx->value == ty->value))) {
    return true;
  }
  if (tx->kind == 1) {
    if (FolOccursNC(pool, x, y, theta, theta_n)) {
      return false;
    }
    if (*out_n >= out_cap) {
      return false;
    }
    out_theta[(*out_n)++] = (TFolSubst){ISC(tx->value), y};
    return true;
  }
  if (ty->kind == 1) {
    if (FolOccursNC(pool, y, x, theta, theta_n)) {
      return false;
    }
    if (*out_n >= out_cap) {
      return false;
    }
    out_theta[(*out_n)++] = (TFolSubst){ISC(ty->value), x};
    return true;
  }
  if (tx->kind == 2 && ty->kind == 2) {
    // x.FIRST/y.FIRST then x.REST/y.REST (the book's LIST branch).
    return FolUnifyRecNC(pool, tx->left, ty->left, theta, theta_n, out_theta,
                         out_cap, out_n) &&
           FolUnifyRecNC(pool, tx->right, ty->right, out_theta, *out_n,
                         out_theta, out_cap, out_n);
  }
  return false;
}

BOL FolUnify(const TFolTerm* pool, IUD x, IUD y, const TFolSubst* theta,
             ISC theta_n, ISC var_count, TFolSubst* out_theta, ISC out_cap,
             ISC* out_n) {
  if (pool == NILP || var_count < 0 || out_cap < 0 || theta_n < 0) {
    return false;
  }
  for (ISC i = 0; i < theta_n; ++i) {
    out_theta[i] = theta[i];
  }
  *out_n = theta_n;
  return FolUnifyRecNC(pool, x, y, theta, theta_n, out_theta, out_cap, out_n);
}

// --- TFolKb (Fig 9.6) ------------------------------------------------------

/* Clause record: conclusion atom id + premises slice into the term pool.
 * The pool stores TFolTerm nodes. */
struct TFolClauseRec {
  IUD conclusion;
  ISC prem_start;
  ISC prem_count;
};

static TFolClauseRec* FolTabNC(const TFolKb* kb) {
  return TPtr<TFolClauseRec>(kb->tab_.origin);
}

TFolKb::TFolKb()
    : tab_(NILP, &ObjectFactoryHeap), pool_(NILP, &ObjectFactoryHeap),
      var_count_(0), clause_cap_(0), clause_count_(0), pool_count_(0) {}

TFolKb::~TFolKb() {
  tab_.origin = FolFree(tab_.origin);
  pool_.origin = FolFree(pool_.origin);
}

BOL TFolKb::Init(ISC clause_cap, ISC var_count) {
  if (clause_cap < 1 || var_count < 1) {
    return false;
  }
  tab_.origin = ObjectFactoryHeap(
      NILP, IUD(clause_cap) * IUD(sizeof(TFolClauseRec)));
  pool_.origin =
      ObjectFactoryHeap(NILP, IUD(clause_cap * 32) * IUD(sizeof(TFolTerm)));
  if (IsError(tab_.origin) || IsError(pool_.origin)) {
    tab_.origin = FolFree(tab_.origin);
    pool_.origin = FolFree(pool_.origin);
    return false;
  }
  FolWipe(tab_.origin, IUD(clause_cap) * IUD(sizeof(TFolClauseRec)));
  FolWipe(pool_.origin, IUD(clause_cap * 32) * IUD(sizeof(TFolTerm)));
  var_count_ = var_count;
  clause_cap_ = clause_cap;
  clause_count_ = 0;
  pool_count_ = 0;
  return true;
}

BOL TFolKb::Tell(const TFolTerm* premises, ISC premise_count,
                 IUD conclusion_atom) {
  if (premises == NILP || premise_count > 32 || clause_count_ >= clause_cap_) {
    return false;
  }
  ISC start = pool_count_;
  TFolTerm* pool = TPtr<TFolTerm>(pool_.origin);
  for (ISC i = 0; i < premise_count; ++i) {
    if (pool_count_ >= clause_cap_ * 32) {
      return false;
    }
    pool[pool_count_++] = premises[i];
  }
  TFolClauseRec* tab = TPtr<TFolClauseRec>(tab_.origin);
  tab[clause_count_++] = (TFolClauseRec){conclusion_atom, start, premise_count};
  return true;
}

/* Goal-directed backward chaining. Premise vars are existential wildcards
 * (documented deviation: θ is not carried through premise bindings — the
 * module's queries are ground atoms, so no binding information is needed
 * downstream). The path mask is the visited set: a goal already on the
 * current chain fails (loop avoidance). */
static BOL FolAskRecNC(const TFolKb* kb, IUD goal, ISC depth, IUD path) {
  if (depth <= 0) {
    return false;
  }
  if (goal >= 64) {  // visited mask bound (documented)
    return false;
  }
  if ((path >> goal) & 1) {
    return false;
  }
  TFolClauseRec* tab = FolTabNC(kb);
  TFolTerm* pool = TPtr<TFolTerm>(kb->pool_.origin);
  path |= IUD(1) << goal;
  for (ISC c = 0; c < kb->clause_count_; ++c) {
    if (tab[c].conclusion != goal) {
      continue;
    }
    BOL all = true;
    for (ISC i = tab[c].prem_start;
         i < tab[c].prem_start + tab[c].prem_count && all; ++i) {
      const TFolTerm* p = &pool[i];
      if (p->kind == 1) {
        continue;  // existential wildcard: any atom satisfies
      }
      if (p->kind != 0) {
        all = false;  // lists as premises: out of scope (documented)
        break;
      }
      all = FolAskRecNC(kb, p->value, depth - 1, path);
    }
    if (all) {
      return true;
    }
  }
  return false;
}

BOL TFolKb::Ask(IUD goal_atom, ISC max_depth) const {
  if (max_depth < 1) {
    return false;
  }
  return FolAskRecNC(this, goal_atom, max_depth, 0);
}

// --- Fig 9.3 NAIVE-ENTAILS -------------------------------------------------

BOL TFolKb::NaiveEntails(const TFolTerm* goal, const IUD* constants,
                         ISC domain_size) const {
  if (goal == NILP || constants == NILP || domain_size < 1 || domain_size > 8 ||
      goal->kind != 0 && goal->kind != 1) {
    return false;
  }
  // 1) Ground every clause over the constant domain.
  static const IUD kMaxGroundings = 4096;
  IUD total_groundings = 1;
  for (ISC c = 0; c < clause_count_; ++c) {
    ISC distinct = 0;
    TFolClauseRec* tab = TPtr<TFolClauseRec>(tab_.origin);
    TFolTerm* pool = TPtr<TFolTerm>(pool_.origin);
    IUD seen = 0;
    for (ISC i = tab[c].prem_start; i < tab[c].prem_start + tab[c].prem_count;
         ++i) {
      if (pool[i].kind == 1) {
        IUD bit = IUD(1) << pool[i].value;
        if (!(seen & bit)) {
          seen |= bit;
          distinct++;
        }
      } else if (pool[i].kind != 0) {
        return false;  // non-ground-able premises: out of scope (documented)
      }
    }
    IUD g = 1;
    for (ISC i = 0; i < distinct; ++i) {
      g *= IUD(domain_size);
    }
    if (g > kMaxGroundings || total_groundings > kMaxGroundings / g) {
      return false;  // grounding bound exceeded (documented)
    }
    total_groundings += g;
  }
  // Temporary RAMFactory allocation for the ground clause table.
  Autoject tmp(NILP, &ObjectFactoryHeap);
  IUD tmp_bytes =
      IUD(total_groundings) * IUD(sizeof(TFolClauseRec)) +
      IUD(goal->kind == 1 ? domain_size : 1) * IUD(sizeof(SatClause));
  tmp.origin = ObjectFactoryHeap(NILP, tmp_bytes);
  if (IsError(tmp.origin)) {
    return false;
  }
  FolWipe(tmp.origin, tmp_bytes);
  TFolClauseRec* ground = TPtr<TFolClauseRec>(tmp.origin);
  ISC ground_n = 0;
  TFolClauseRec* tab = TPtr<TFolClauseRec>(tab_.origin);
  TFolTerm* pool = TPtr<TFolTerm>(pool_.origin);
  for (ISC c = 0; c < clause_count_; ++c) {
    // Enumerate assignments of the clause's distinct premise vars.
    ISC var_ids[8];
    ISC var_n = 0;
    IUD seen = 0;
    for (ISC i = tab[c].prem_start; i < tab[c].prem_start + tab[c].prem_count;
         ++i) {
      if (pool[i].kind == 1) {
        IUD bit = IUD(1) << pool[i].value;
        if (!(seen & bit)) {
          seen |= bit;
          var_ids[var_n++] = ISC(pool[i].value);
        }
      }
    }
    IUD count = 1;
    for (ISC i = 0; i < var_n; ++i) {
      count *= IUD(domain_size);
    }
    for (IUD assign = 0; assign < count; ++assign) {
      IUD rem = assign;
      IUD prem_mask = 0;
      BOL bad = false;
      for (ISC i = 0; i < var_n; ++i) {
        IUD atom = constants[ISC(rem % IUD(domain_size))];
        rem /= IUD(domain_size);
        if (atom >= 64) {
          bad = true;
        } else {
          prem_mask |= IUD(1) << atom;
        }
      }
      if (bad) {
        continue;
      }
      // Premise atoms not in the assignment (already ground) add to the mask.
      for (ISC i = tab[c].prem_start; i < tab[c].prem_start + tab[c].prem_count;
           ++i) {
        if (pool[i].kind == 0) {
          prem_mask |= IUD(1) << pool[i].value;
        }
      }
      TFolClauseRec g = {tab[c].conclusion, ISC(prem_mask), 0};
      // Deduplicate.
      BOL dup = false;
      for (ISC i = 0; i < ground_n; ++i) {
        if (ground[i].conclusion == g.conclusion &&
            ground[i].prem_start == g.prem_start) {
          dup = true;
          break;
        }
      }
      if (!dup) {
        ground[ground_n++] = g;
      }
    }
  }
  // 2) For each grounding of the goal: KB ∪ {¬g} must be unsatisfiable.
  SatClause* sat = TPtr<SatClause>(tmp.origin) + ground_n;
  IUD goal_atoms[8];
  ISC goal_atom_n = 0;
  if (goal->kind == 1) {
    for (ISC i = 0; i < domain_size; ++i) {
      goal_atoms[goal_atom_n++] = constants[i];
    }
  } else {
    goal_atoms[goal_atom_n++] = goal->value;
  }
  for (ISC gi = 0; gi < goal_atom_n; ++gi) {
    if (goal_atoms[gi] >= 64) {
      return false;
    }
    ISC clause_n = ground_n + 1;
    // Rebuild the SatClause table in place (ground recs are 24B, SatClause
    // is 16B; write over the same RAM).
    SatClause* clauses = TPtr<SatClause>(tmp.origin);
    for (ISC i = 0; i < ground_n; ++i) {
      // definite premise ⇒ conclusion  ==  (¬p1 ∧ ... ∨ q)
      clauses[i].negative = ground[i].prem_start;
      clauses[i].positive = (ground[i].conclusion < 64)
                               ? IUD(1) << ground[i].conclusion
                               : 0;
    }
    clauses[ground_n] =
        (SatClause){0, (goal_atoms[gi] < 64) ? IUD(1) << goal_atoms[gi] : 0};
    if (SatDpll(clauses, clause_n, ~IUD(0))) {
      FolFree(tmp.origin);
      return false;  // a model exists where the KB holds and g is false
    }
  }
  FolFree(tmp.origin);
  return true;
}

// --- Fig 9.8 compiled Append -----------------------------------------------

BOL FolAppend(const IUD* ax, ISC ax_n, const IUD* y, ISC y_n, IUD* az,
              ISC az_cap, ISC* az_n) {
  if (ax_n < 0 || y_n < 0) {
    return false;
  }
  if (az == NILP || az_cap < ax_n + y_n) {
    return false;
  }
  // ax = [a | x]: the compiled scheme's base case ax = [] and UNIFY(y, az)
  // is the prefix part; the recursive case APPEND(x, y, z) with
  // UNIFY(ax, [a|x]) and UNIFY(az, [a|z]) is the tail part, run at bounded
  // depth ax_n + y_n (the iteration bound of the compiled scheme).
  for (ISC i = 0; i < ax_n; ++i) {
    az[i] = ax[i];
  }
  for (ISC i = 0; i < y_n; ++i) {
    az[ax_n + i] = y[i];
  }
  *az_n = ax_n + y_n;
  return true;
}

} //< namespace iGeek
#endif
