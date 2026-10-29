// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_SEARCH_IMPL
#define IGEEK_CLASSICAL_SEARCH_IMPL
#include "Search.h"
#include "../ASCIICrabs/Binary.hpp"
#include "../ASCIICrabs/Autoject.hxx"
#include "../ASCIICrabs/Array.hpp"
namespace iGeek {

// --- Internal machinery (internal linkage; not module API) ----------------
//
// All RAM is created and destroyed through the RAMFactory
// (ObjectFactoryHeap — the same factory BigInt and AgentState use):
//   alloc: ObjectFactoryHeap(NILP, bytes)  free: ObjectFactoryHeap(origin,
//   RAMFactoryDelete).  Fixed-capacity action scratch uses AArray
// (stack-first, RAMFactory-managed under its Autoboofer). No naked
// new/delete/malloc in this folder.

// Per-node action-list capacity for expansion (world branching above this
// is a contract violation; the excess actions are dropped).
static const ISC kSearchBranch = 256;

static IUD StateHashNC(const IUC* state, ISC state_words) {
  IUD h = 1469598103934665603ULL;  // FNV-1a 64-bit offset basis
  for (ISC i = 0; i < state_words; ++i) {
    h ^= IUD(state[i]);
    h *= 1099511628211ULL;
  }
  return h;
}

static BOL StatesEqualNC(const IUC* a, const IUC* b, ISC state_words) {
  for (ISC i = 0; i < state_words; ++i) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

/* Returns a RAMFactory block to the factory (a no-op for NILP origins). */
static inline IUW* SearchFree(IUW* origin) {
  return ObjectFactoryHeap(origin, RAMFactoryDelete);
}

/* Zeroes a freshly allocated RAMFactory block (word-aligned up). */
static inline void SearchWipe(IUW* origin, ISW bytes) {
  ISW aligned = bytes + ((-bytes) & ACPUWordMask);
  ISC words = ISC(aligned >> ACPUBytesLog2);
  for (ISC i = 0; i < words; ++i) {
    origin[i] = 0;
  }
}

/* Node pool: append-only arrays; node i's state is at
 * states + i * state_words. All five arrays are RAMFactory-owned. */
struct SearchPool {
  Autoject states_;
  Autoject parents_;
  Autoject actions_;
  Autoject depth_;
  Autoject g_;
  ISC count;
  ISC cap;
  SearchPool()
      : states_(NILP, &ObjectFactoryHeap),
        parents_(NILP, &ObjectFactoryHeap),
        actions_(NILP, &ObjectFactoryHeap),
        depth_(NILP, &ObjectFactoryHeap),
        g_(NILP, &ObjectFactoryHeap), count(0), cap(0) {}
  SearchPool(const SearchPool&) = delete;
  SearchPool& operator=(const SearchPool&) = delete;
  ~SearchPool() {
    Free();
  }
  BOL Alloc(ISC state_words, ISC cap0) {
    cap = cap0;
    count = 0;
    states_.origin =
        ObjectFactoryHeap(NILP, IUD(cap0) * IUD(state_words) * IUD(sizeof(IUC)));
    parents_.origin = ObjectFactoryHeap(NILP, IUD(cap0) * IUD(sizeof(ISC)));
    actions_.origin = ObjectFactoryHeap(NILP, IUD(cap0) * IUD(sizeof(IUC)));
    depth_.origin = ObjectFactoryHeap(NILP, IUD(cap0) * IUD(sizeof(ISC)));
    g_.origin = ObjectFactoryHeap(NILP, IUD(cap0) * IUD(sizeof(FPC)));
    return !IsError();
  }
  void Free() {
    states_.origin = SearchFree(states_.origin);
    parents_.origin = SearchFree(parents_.origin);
    actions_.origin = SearchFree(actions_.origin);
    depth_.origin = SearchFree(depth_.origin);
    g_.origin = SearchFree(g_.origin);
  }
  BOL IsError() const {
    return _::IsError(states_.origin) || _::IsError(parents_.origin) ||
           _::IsError(actions_.origin) || _::IsError(depth_.origin) ||
           _::IsError(g_.origin);
  }
  IUC* States() const {
    return TPtr<IUC>(states_.origin);
  }
  ISC* Parents() const {
    return TPtr<ISC>(parents_.origin);
  }
  IUC* ActionArr() const {
    return TPtr<IUC>(actions_.origin);
  }
  ISC* Depth() const {
    return TPtr<ISC>(depth_.origin);
  }
  FPC* G() const {
    return TPtr<FPC>(g_.origin);
  }
};

static void SearchPoolGrow(SearchPool* pool, ISC state_words) {
  ISC new_cap = pool->cap * 2;
  IUW* ns = ObjectFactoryHeap(
      NILP, IUD(new_cap) * IUD(state_words) * IUD(sizeof(IUC)));
  IUW* np = ObjectFactoryHeap(NILP, IUD(new_cap) * IUD(sizeof(ISC)));
  IUW* na = ObjectFactoryHeap(NILP, IUD(new_cap) * IUD(sizeof(IUC)));
  IUW* nd = ObjectFactoryHeap(NILP, IUD(new_cap) * IUD(sizeof(ISC)));
  IUW* ng = ObjectFactoryHeap(NILP, IUD(new_cap) * IUD(sizeof(FPC)));
  IUC* ns_p = TPtr<IUC>(ns);
  IUC* os = pool->States();
  for (ISC i = 0; i < pool->count; ++i) {
    for (ISC w = 0; w < state_words; ++w) {
      ns_p[IUD(i) * state_words + w] = os[IUD(i) * state_words + w];
    }
    TPtr<ISC>(np)[i] = pool->Parents()[i];
    TPtr<IUC>(na)[i] = pool->ActionArr()[i];
    TPtr<ISC>(nd)[i] = pool->Depth()[i];
    TPtr<FPC>(ng)[i] = pool->G()[i];
  }
  pool->Free();
  pool->states_.origin = ns;
  pool->parents_.origin = np;
  pool->actions_.origin = na;
  pool->depth_.origin = nd;
  pool->g_.origin = ng;
  pool->cap = new_cap;
}

static ISC SearchPoolAdd(SearchPool* pool, const IUC* state, ISC parent,
                         IUC action, ISC depth, FPC g, ISC state_words) {
  if (pool->count == pool->cap) {
    SearchPoolGrow(pool, state_words);
  }
  ISC n = pool->count;
  for (ISC w = 0; w < state_words; ++w) {
    pool->States()[IUD(n) * state_words + w] = state[w];
  }
  pool->Parents()[n] = parent;
  pool->ActionArr()[n] = action;
  pool->Depth()[n] = depth;
  pool->G()[n] = g;
  ++pool->count;
  return n;
}

/* Extract the action path from root to `node` (exclusive of the root's
 * action). Returns -1 when the path is longer than path_cap. */
static ISC SearchPoolPath(const SearchPool* pool, ISC node, IUC* path,
                          ISC path_cap) {
  ISC len = 0;
  ISC n = node;
  while (pool->Parents()[n] != -1) {
    ++len;
    n = pool->Parents()[n];
  }
  if (len > path_cap) {
    return -1;
  }
  n = node;
  for (ISC i = len - 1; i >= 0; --i) {
    path[i] = pool->ActionArr()[n];
    n = pool->Parents()[n];
  }
  return len;
}

/* Visited set: open addressing, power-of-two capacity. A slot is occupied
 * when vals != 0 (stored node index + 1); keys holds the raw FNV hash.
 * Hash collisions are resolved by re-probing on the full state words.
 * Both tables are RAMFactory-owned. */
struct SearchVisited {
  Autoject keys_;
  Autoject vals_;
  ISC cap;
  ISC count;
  SearchVisited()
      : keys_(NILP, &ObjectFactoryHeap), vals_(NILP, &ObjectFactoryHeap),
        cap(0), count(0) {}
  SearchVisited(const SearchVisited&) = delete;
  SearchVisited& operator=(const SearchVisited&) = delete;
  ~SearchVisited() {
    Free();
  }
  BOL Alloc(ISC expected) {
    ISC p = 16;
    while (p < expected * 4) {
      p <<= 1;
    }
    cap = p;
    count = 0;
    keys_.origin = ObjectFactoryHeap(NILP, IUD(p) * IUD(sizeof(IUD)));
    vals_.origin = ObjectFactoryHeap(NILP, IUD(p) * IUD(sizeof(ISC)));
    if (IsError(keys_.origin) || IsError(vals_.origin)) {
      Free();
      return false;
    }
    SearchWipe(keys_.origin, IUD(p) * IUD(sizeof(IUD)));
    SearchWipe(vals_.origin, IUD(p) * IUD(sizeof(ISC)));
    return true;
  }
  IUD* Keys() const {
    return TPtr<IUD>(keys_.origin);
  }
  ISC* Vals() const {
    return TPtr<ISC>(vals_.origin);
  }
  void Free() {
    keys_.origin = SearchFree(keys_.origin);
    vals_.origin = SearchFree(vals_.origin);
  }
};

static void SearchVisitedRehash(SearchVisited* vis, ISC new_cap) {
  ISC old_cap = vis->cap;
  IUD* ok = vis->Keys();
  ISC* ov = vis->Vals();
  vis->cap = new_cap;
  vis->count = 0;
  vis->keys_.origin = ObjectFactoryHeap(NILP, IUD(new_cap) * IUD(sizeof(IUD)));
  vis->vals_.origin = ObjectFactoryHeap(NILP, IUD(new_cap) * IUD(sizeof(ISC)));
  SearchWipe(vis->keys_.origin, IUD(new_cap) * IUD(sizeof(IUD)));
  SearchWipe(vis->vals_.origin, IUD(new_cap) * IUD(sizeof(ISC)));
  ISC mask = new_cap - 1;
  // Reinsert every occupied slot of the old table into the new one, then
  // return the old tables to the factory.
  for (ISC i = 0; i < old_cap; ++i) {
    if (ov[i] == 0) {
      continue;
    }
    ISC idx = ISC(ok[i] & IUD(mask));
    for (;;) {
      if (vis->Vals()[idx] == 0) {
        vis->Keys()[idx] = ok[i];
        vis->Vals()[idx] = ov[i];
        ++vis->count;
        break;
      }
      idx = (idx + 1) & mask;
    }
  }
  SearchFree(TPtr<IUW>(ok));
  SearchFree(TPtr<IUW>(ov));
}

/* Inserts the state of node `node` into the visited set.
 * Returns true when newly inserted, false when an equal state was already
 * present. */
static BOL SearchVisitedInsert(SearchVisited* vis, const SearchPool* pool,
                               ISC state_words, IUD hash, ISC node) {
  const IUC* state = pool->States() + IUD(node) * IUD(state_words);
  ISC mask = vis->cap - 1;
  ISC idx = ISC(hash & IUD(mask));
  for (;;) {
    if (vis->Vals()[idx] == 0) {
      vis->Keys()[idx] = hash;
      vis->Vals()[idx] = node + 1;
      ++vis->count;
      return true;
    }
    if (vis->Keys()[idx] == hash) {
      ISC n = vis->Vals()[idx] - 1;
      if (StatesEqualNC(pool->States() + IUD(n) * IUD(state_words), state,
                        state_words)) {
        return false;
      }
    }
    idx = (idx + 1) & mask;
  }
}

static FPC SearchStepCostNC(const Problem* problem, const SearchPool* pool,
                            ISC state_words, ISC parent, IUC action) {
  if (problem->StepCost == NILP) {
    return FPC(1.0f);
  }
  return problem->StepCost(pool->States() + IUD(parent) * IUD(state_words),
                           state_words, action);
}

// --- Figures 3.13/3.22/3.24/3.30/3.38: the searchers -----------------------

/* Figure 3.13 (breadth-first) with a visited set; unit-cost optimal. */
ISC SearchBFS(const Problem* problem, IUC* path, ISC path_cap) {
  ISC sw = problem->state_words < 1 ? 1 : problem->state_words;
  SearchPool pool;
  SearchVisited vis;
  AArray<IUC, ISN, kSearchBranch> acts;
  IUW* succ_mem = ObjectFactoryHeap(NILP, IUD(sw) * IUD(sizeof(IUC)));
  IUW* queue_mem = ObjectFactoryHeap(NILP, 64 * sizeof(ISC));
  if (!pool.Alloc(sw, 16) || !vis.Alloc(16) || IsError(succ_mem) ||
      IsError(queue_mem)) {
    succ_mem = SearchFree(succ_mem);
    queue_mem = SearchFree(queue_mem);
    return -1;
  }
  IUC* succ = TPtr<IUC>(succ_mem);
  ISC* queue = TPtr<ISC>(queue_mem);
  IUD h0 = StateHashNC(problem->initial, sw);
  SearchPoolAdd(&pool, problem->initial, -1, 0, 0, FPC(0.0f), sw);
  SearchVisitedInsert(&vis, &pool, sw, h0, 0);
  if (problem->GoalTest(problem->initial, sw)) {
    return 0;
  }
  ISC queue_cap = 64;
  ISC head = 0;
  ISC tail = 0;
  queue[tail++] = 0;
  ISC result = -1;
  BOL done = false;
  while (head < tail && !done) {
    ISC n = queue[head++];
    const IUC* ns = pool.States() + IUD(n) * IUD(sw);
    ISC act_count = problem->Actions(ns, sw, acts.Begin(), kSearchBranch);
    for (ISC k = 0; k < act_count && !done; ++k) {
      problem->Result(ns, sw, acts[k], succ);
      IUD hs = StateHashNC(succ, sw);
      FPC cost = SearchStepCostNC(problem, &pool, sw, n, acts[k]);
      ISC m = SearchPoolAdd(&pool, succ, n, acts[k], pool.Depth()[n] + 1,
                            pool.G()[n] + cost, sw);
      if (pool.IsError()) {
        result = -1;
        done = true;
        break;
      }
      if (!SearchVisitedInsert(&vis, &pool, sw, hs, m)) {
        continue;
      }
      if (vis.count * 4 >= vis.cap * 3) {
        SearchVisitedRehash(&vis, vis.cap * 2);
      }
      if (problem->GoalTest(pool.States() + IUD(m) * IUD(sw), sw)) {
        result = SearchPoolPath(&pool, m, path, path_cap);
        done = true;
        break;
      }
      if (tail == queue_cap) {
        queue_cap *= 2;
        IUW* nq =
            ObjectFactoryHeap(NILP, IUD(queue_cap) * IUD(sizeof(ISC)));
        for (ISC i = 0; i < tail; ++i) {
          TPtr<ISC>(nq)[i] = queue[i];
        }
        queue_mem = SearchFree(queue_mem);
        queue_mem = nq;
        queue = TPtr<ISC>(queue_mem);
      }
      queue[tail++] = m;
    }
  }
  succ_mem = SearchFree(succ_mem);
  queue_mem = SearchFree(queue_mem);
  return result;
}

/* Binary min-heap of node indices keyed by g (uniform-cost search). */
static ISC SearchHeapPop(ISC* heap, ISC* n, const SearchPool* pool) {
  ISC top = heap[0];
  --(*n);
  if (*n > 0) {
    heap[0] = heap[*n];
    ISC i = 0;
    for (;;) {
      ISC l = i * 2 + 1;
      ISC r = i * 2 + 2;
      ISC m = i;
      if (l < *n && pool->G()[heap[l]] < pool->G()[heap[m]]) {
        m = l;
      }
      if (r < *n && pool->G()[heap[r]] < pool->G()[heap[m]]) {
        m = r;
      }
      if (m == i) {
        break;
      }
      ISC t = heap[i];
      heap[i] = heap[m];
      heap[m] = t;
      i = m;
    }
  }
  return top;
}

static BOL SearchHeapPush(IUW** heap_mem, ISC* heap_cap, ISC* n, ISC node,
                          const SearchPool* pool) {
  if (*n == *heap_cap) {
    *heap_cap *= 2;
    IUW* nh =
        ObjectFactoryHeap(NILP, IUD(*heap_cap) * IUD(sizeof(ISC)));
    if (IsError(nh)) {
      return false;
    }
    ISC* heap = TPtr<ISC>(*heap_mem);
    for (ISC i = 0; i < *n; ++i) {
      TPtr<ISC>(nh)[i] = heap[i];
    }
    *heap_mem = SearchFree(*heap_mem);
    *heap_mem = nh;
  }
  ISC* heap = TPtr<ISC>(*heap_mem);
  heap[*n] = node;
  ++(*n);
  // Sift up.
  ISC i = *n - 1;
  while (i > 0) {
    ISC p = (i - 1) / 2;
    if (pool->G()[heap[p]] <= pool->G()[heap[i]]) {
      break;
    }
    ISC t = heap[p];
    heap[p] = heap[i];
    heap[i] = t;
    i = p;
  }
  return true;
}

ISC SearchUCS(const Problem* problem, IUC* path, ISC path_cap) {
  ISC sw = problem->state_words < 1 ? 1 : problem->state_words;
  SearchPool pool;
  SearchVisited vis;
  AArray<IUC, ISN, kSearchBranch> acts;
  IUW* succ_mem = ObjectFactoryHeap(NILP, IUD(sw) * IUD(sizeof(IUC)));
  IUW* heap_mem = ObjectFactoryHeap(NILP, 16 * sizeof(ISC));
  if (!pool.Alloc(sw, 16) || !vis.Alloc(16) || IsError(succ_mem) ||
      IsError(heap_mem)) {
    succ_mem = SearchFree(succ_mem);
    heap_mem = SearchFree(heap_mem);
    return -1;
  }
  IUC* succ = TPtr<IUC>(succ_mem);
  ISC* heap = TPtr<ISC>(heap_mem);
  IUD h0 = StateHashNC(problem->initial, sw);
  SearchPoolAdd(&pool, problem->initial, -1, 0, 0, FPC(0.0f), sw);
  SearchVisitedInsert(&vis, &pool, sw, h0, 0);
  if (problem->GoalTest(problem->initial, sw)) {
    return 0;
  }
  ISC heap_cap = 16;
  ISC hn = 0;
  heap[hn++] = 0;
  ISC result = -1;
  BOL done = false;
  while (hn > 0 && !done) {
    ISC n = SearchHeapPop(heap, &hn, &pool);
    const IUC* ns = pool.States() + IUD(n) * IUD(sw);
    if (problem->GoalTest(ns, sw)) {
      result = SearchPoolPath(&pool, n, path, path_cap);
      done = true;
      break;
    }
    ISC act_count = problem->Actions(ns, sw, acts.Begin(), kSearchBranch);
    for (ISC k = 0; k < act_count && !done; ++k) {
      problem->Result(ns, sw, acts[k], succ);
      IUD hs = StateHashNC(succ, sw);
      FPC cost = SearchStepCostNC(problem, &pool, sw, n, acts[k]);
      ISC m = SearchPoolAdd(&pool, succ, n, acts[k], pool.Depth()[n] + 1,
                            pool.G()[n] + cost, sw);
      if (pool.IsError()) {
        result = -1;
        done = true;
        break;
      }
      if (!SearchVisitedInsert(&vis, &pool, sw, hs, m)) {
        continue;
      }
      if (vis.count * 4 >= vis.cap * 3) {
        SearchVisitedRehash(&vis, vis.cap * 2);
      }
      if (!SearchHeapPush(&heap_mem, &heap_cap, &hn, m, &pool)) {
        result = -1;
        done = true;
        break;
      }
      heap = TPtr<ISC>(heap_mem);
    }
  }
  succ_mem = SearchFree(succ_mem);
  heap_mem = SearchFree(heap_mem);
  return result;
}

/* Figure 3.24 depth-limited search, iterative with an explicit stack.
 * Frame child_result: 0 = none, -1 = failure, -2 = cutoff. */
struct DlsFrame {
  ISC node;
  ISC cursor;
  BOL expanded;
  ISC child_result;
  BOL cutoff;
  IUW* acts;
  ISC act_count;
};

ISC SearchDLS(const Problem* problem, IUC* path, ISC path_cap, ISC limit) {
  if (limit < 0) {
    return -1;
  }
  ISC sw = problem->state_words < 1 ? 1 : problem->state_words;
  SearchPool pool;
  IUW* succ_mem = ObjectFactoryHeap(NILP, IUD(sw) * IUD(sizeof(IUC)));
  IUW* stack_mem =
      ObjectFactoryHeap(NILP, 16 * sizeof(DlsFrame));
  if (!pool.Alloc(sw, 16) || IsError(succ_mem) || IsError(stack_mem)) {
    succ_mem = SearchFree(succ_mem);
    stack_mem = SearchFree(stack_mem);
    return -1;
  }
  IUC* succ = TPtr<IUC>(succ_mem);
  DlsFrame* stack = TPtr<DlsFrame>(stack_mem);
  SearchPoolAdd(&pool, problem->initial, -1, 0, 0, FPC(0.0f), sw);
  if (problem->GoalTest(problem->initial, sw)) {
    return 0;
  }
  ISC stack_cap = 16;
  ISC top = 0;
  stack[0].node = 0;
  stack[0].cursor = 0;
  stack[0].expanded = false;
  stack[0].child_result = 0;
  stack[0].cutoff = false;
  stack[0].acts = NILP;
  stack[0].act_count = 0;
  ISC result = -1;
  BOL finished = false;
  while (top >= 0 && !finished) {
    DlsFrame* f = &stack[top];
    if (f->child_result != 0) {
      ISC r = f->child_result;
      f->acts = SearchFree(f->acts);
      --top;
      if (top < 0) {
        result = r;
        finished = true;
        break;
      }
      if (r == -2) {
        // CUTOFF: record it on the parent, then let the parent keep trying
        // its remaining siblings (the book's `cutoff occurred? <- true`).
        stack[top].cutoff = true;
      }
      stack[top].child_result = 0;
      continue;
    }
    ISC n = f->node;
    if (!f->expanded) {
      if (pool.Depth()[n] == limit) {
        f->child_result = -2;
        continue;
      }
      f->expanded = true;
      f->acts = ObjectFactoryHeap(NILP, kSearchBranch * sizeof(IUC));
      if (IsError(f->acts)) {
        f->child_result = -1;
        continue;
      }
      f->act_count = problem->Actions(pool.States() + IUD(n) * IUD(sw), sw,
                                      TPtr<IUC>(f->acts), kSearchBranch);
      if (f->act_count == 0) {
        f->child_result = -1;
        continue;
      }
    }
    if (f->cursor == f->act_count) {
      f->child_result = f->cutoff ? -2 : -1;
      continue;
    }
    IUC action = TPtr<IUC>(f->acts)[f->cursor++];
    problem->Result(pool.States() + IUD(n) * IUD(sw), sw, action, succ);
    ISC m = SearchPoolAdd(&pool, succ, n, action, pool.Depth()[n] + 1,
                          FPC(0.0f), sw);
    if (pool.IsError()) {
      result = -1;
      finished = true;
      break;
    }
    if (problem->GoalTest(pool.States() + IUD(m) * IUD(sw), sw)) {
      result = SearchPoolPath(&pool, m, path, path_cap);
      finished = true;
      break;
    }
    if (top + 1 == stack_cap) {
      stack_cap *= 2;
      IUW* ns =
          ObjectFactoryHeap(NILP, IUD(stack_cap) * IUD(sizeof(DlsFrame)));
      if (IsError(ns)) {
        result = -1;
        finished = true;
        break;
      }
      for (ISC i = 0; i <= top; ++i) {
        TPtr<DlsFrame>(ns)[i] = stack[i];
      }
      stack_mem = SearchFree(stack_mem);
      stack_mem = ns;
      stack = TPtr<DlsFrame>(stack_mem);
    }
    ++top;
    stack[top].node = m;
    stack[top].cursor = 0;
    stack[top].expanded = false;
    stack[top].child_result = 0;
    stack[top].cutoff = false;
    stack[top].acts = NILP;
    stack[top].act_count = 0;
  }
  for (ISC i = 0; i <= top; ++i) {
    stack[i].acts = SearchFree(stack[i].acts);
  }
  stack_mem = SearchFree(stack_mem);
  succ_mem = SearchFree(succ_mem);
  return result;
}

/* Figure 3.30 iterative-deepening search. DLS reports -1 only when the
 * whole tree below the limit was exhausted without a goal, so -1 is a
 * definitive no-solution; -2 means "deeper". */
ISC SearchIDS(const Problem* problem, IUC* path, ISC path_cap, ISC max_limit) {
  if (max_limit < 0) {
    return -1;
  }
  for (ISC limit = 0; limit <= max_limit; ++limit) {
    ISC r = SearchDLS(problem, path, path_cap, limit);
    if (r >= 0 || r == -1) {
      return r;
    }
  }
  return -2;
}

/* Figure 3.38 recursive best-first search, iterative with an explicit
 * stack. The heuristic must be admissible. */
struct RbfsFrame {
  ISC node;
  FPC alt;
  BOL expanded;
  IUW* children;
  ISC child_count;
};

static FPC RbfsFNC(const Problem* problem, const SearchPool* pool,
                   ISC state_words, ISC node) {
  return pool->G()[node] +
         problem->Heuristic(pool->States() + IUD(node) * IUD(state_words),
                            state_words);
}

ISC SearchRBFS(const Problem* problem, IUC* path, ISC path_cap) {
  if (problem->Heuristic == NILP) {
    return -1;
  }
  ISC sw = problem->state_words < 1 ? 1 : problem->state_words;
  SearchPool pool;
  AArray<IUC, ISN, kSearchBranch> acts;
  IUW* succ_mem = ObjectFactoryHeap(NILP, IUD(sw) * IUD(sizeof(IUC)));
  IUW* stack_mem =
      ObjectFactoryHeap(NILP, 16 * sizeof(RbfsFrame));
  if (!pool.Alloc(sw, 16) || IsError(succ_mem) || IsError(stack_mem)) {
    succ_mem = SearchFree(succ_mem);
    stack_mem = SearchFree(stack_mem);
    return -1;
  }
  IUC* succ = TPtr<IUC>(succ_mem);
  RbfsFrame* stack = TPtr<RbfsFrame>(stack_mem);
  SearchPoolAdd(&pool, problem->initial, -1, 0, 0, FPC(0.0f), sw);
  ISC stack_cap = 16;
  ISC top = 0;
  stack[0].node = 0;
  stack[0].alt = FPC(1e30f);
  stack[0].expanded = false;
  stack[0].children = NILP;
  stack[0].child_count = 0;
  ISC result = -1;
  BOL finished = false;
  while (top >= 0 && !finished) {
    RbfsFrame* f = &stack[top];
    ISC n = f->node;
    if (!f->expanded) {
      f->expanded = true;
      ISC act_count =
          problem->Actions(pool.States() + IUD(n) * IUD(sw), sw, acts.Begin(),
                           kSearchBranch);
      if (act_count == 0) {
        // Leaf node with no goal: the book's `RETURN FAILURE`.
        result = -1;
        finished = true;
        break;
      }
      f->children = ObjectFactoryHeap(NILP, IUD(act_count) * IUD(sizeof(ISC)));
      if (IsError(f->children)) {
        result = -1;
        finished = true;
        break;
      }
      f->child_count = 0;
      for (ISC k = 0; k < act_count; ++k) {
        problem->Result(pool.States() + IUD(n) * IUD(sw), sw, acts[k], succ);
        FPC cost = SearchStepCostNC(problem, &pool, sw, n, acts[k]);
        ISC m = SearchPoolAdd(&pool, succ, n, acts[k], pool.Depth()[n] + 1,
                              pool.G()[n] + cost, sw);
        if (pool.IsError()) {
          result = -1;
          finished = true;
          break;
        }
        TPtr<ISC>(f->children)[f->child_count++] = m;
      }
      if (finished) {
        break;
      }
      // Order children by f = g + h (insertion sort; small lists).
      for (ISC i = 1; i < f->child_count; ++i) {
        ISC key = TPtr<ISC>(f->children)[i];
        FPC kf = RbfsFNC(problem, &pool, sw, key);
        ISC j = i - 1;
        while (j >= 0 &&
               RbfsFNC(problem, &pool, sw, TPtr<ISC>(f->children)[j]) > kf) {
          TPtr<ISC>(f->children)[j + 1] = TPtr<ISC>(f->children)[j];
          --j;
        }
        TPtr<ISC>(f->children)[j + 1] = key;
      }
    }
    // alt <- MIN(alt, MIN-f(children[1..]))
    for (ISC i = 1; i < f->child_count; ++i) {
      FPC fv = RbfsFNC(problem, &pool, sw, TPtr<ISC>(f->children)[i]);
      if (fv < f->alt) {
        f->alt = fv;
      }
    }
    if (f->child_count >= 2 &&
        RbfsFNC(problem, &pool, sw, TPtr<ISC>(f->children)[1]) > f->alt) {
      // UNEXPAND(children[1..]) back to parent; node <- node.parent.
      f->children = SearchFree(f->children);
      f->child_count = 0;
      f->expanded = false;
      --top;
      if (top < 0) {
        // Root with an unusable bound: no path to report.
        result = -1;
        finished = true;
        break;
      }
      RbfsFrame* p = &stack[top];
      FPC fp = RbfsFNC(problem, &pool, sw, p->node);
      if (fp > p->alt) {
        p->alt = fp;
      }
      continue;
    }
    // Descend into children[0].
    ISC c = TPtr<ISC>(f->children)[0];
    if (problem->GoalTest(pool.States() + IUD(c) * IUD(sw), sw)) {
      result = SearchPoolPath(&pool, c, path, path_cap);
      finished = true;
      break;
    }
    if (top + 1 == stack_cap) {
      stack_cap *= 2;
      IUW* ns =
          ObjectFactoryHeap(NILP, IUD(stack_cap) * IUD(sizeof(RbfsFrame)));
      if (IsError(ns)) {
        result = -1;
        finished = true;
        break;
      }
      for (ISC i = 0; i <= top; ++i) {
        TPtr<RbfsFrame>(ns)[i] = stack[i];
      }
      stack_mem = SearchFree(stack_mem);
      stack_mem = ns;
      stack = TPtr<RbfsFrame>(stack_mem);
    }
    ++top;
    stack[top].node = c;
    stack[top].alt = FPC(1e30f);
    stack[top].expanded = false;
    stack[top].children = NILP;
    stack[top].child_count = 0;
  }
  for (ISC i = 0; i <= top; ++i) {
    stack[i].children = SearchFree(stack[i].children);
  }
  stack_mem = SearchFree(stack_mem);
  succ_mem = SearchFree(succ_mem);
  return result;
}

} //< namespace iGeek
#endif
