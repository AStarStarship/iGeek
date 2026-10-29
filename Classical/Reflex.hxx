// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_REFLEX_IMPL
#define IGEEK_CLASSICAL_REFLEX_IMPL
#include "Reflex.h"
#include "../ASCIICrabs/Binary.hpp"
#include "../ASCIICrabs/Autoject.hxx"
namespace iGeek {

// --- Internal machinery (internal linkage; not module API) ----------------
//
// All RAM is created and destroyed through the RAMFactory
// (ObjectFactoryHeap — the same factory BigInt and AgentState use):
//   alloc: ObjectFactoryHeap(NILP, bytes)  free: ObjectFactoryHeap(origin,
//   RAMFactoryDelete). No naked new/delete/malloc in this folder.

static inline IUW* RfxFree(IUW* origin) {
  return ObjectFactoryHeap(origin, RAMFactoryDelete);
}

static inline void RfxWipe(IUW* origin, IUD bytes) {
  IUD aligned = bytes + ((-bytes) & IUD(ACPUWordMask));
  IUW* p = TPtr<IUW>(origin);
  for (IUD i = 0; i < aligned >> ACPUBytesLog2; ++i) {
    p[i] = 0;
  }
}

/* Sentinel for an unpinned table entry (vacuum actions are 0..2). */
static const IUC kRfxUnpinned = 0xFFFE;

/* Pack one vacuum percept: bit 1 = dirty, bit 0 = location. */
static inline IUC RfxPackPercept(IUC location, BOL dirty) {
  return IUC(location & 1) | IUC(dirty ? 2 : 0);
}

// --- Fig 2.8 ----------------------------------------------------------------

IUC ReflexVacuumAction(IUC location, BOL dirty) {
  if (dirty) {
    return kVacSuck;
  }
  if (location == kVacLocA) {
    return kVacRight;
  }
  return kVacLeft;
}

// --- Fig 2.7 TableVacuumAgent -----------------------------------------------

TableVacuumAgent::TableVacuumAgent()
    : table_mem_(NILP, &ObjectFactoryHeap), history_len_(0), hist_count_(0),
      history_(NILP) {}

TableVacuumAgent::~TableVacuumAgent() {
  table_mem_.origin = RfxFree(table_mem_.origin);
  history_ = NILP;
}

BOL TableVacuumAgent::Init(ISC history_len) {
  if (history_len < 1 || history_len > 15) {
    return false;
  }
  IUD entries = IUD(1) << IUD(2 * history_len);
  table_mem_.origin =
      ObjectFactoryHeap(NILP, entries * IUD(sizeof(IUC)) +
                                  IUD(history_len) * IUD(sizeof(IUC)));
  if (IsError(table_mem_.origin)) {
    table_mem_.origin = NILP;
    return false;
  }
  IUC* table = TPtr<IUC>(table_mem_.origin);
  for (IUD i = 0; i < entries; ++i) {
    table[i] = kRfxUnpinned;
  }
  history_ = TPtr<IUC>(table_mem_.origin) + entries;
  RfxWipe(table_mem_.origin, IUD(history_len) * IUD(sizeof(IUC)));
  history_len_ = history_len;
  hist_count_ = 0;
  return true;
}

void TableVacuumAgent::Percept(IUC location, BOL dirty) {
  // append percept to the end of percepts (circular: keep the latest
  // history_len)
  IUC packed = RfxPackPercept(location, dirty);
  history_[hist_count_ % IUD(history_len_)] = packed;
  if (hist_count_ < history_len_) {
    hist_count_++;
  }
}

IUC TableVacuumAgent::Action() {
  // action <- LOOKUP (percepts, table)
  if (hist_count_ == 0) {
    return kVacSuck;
  }
  IUC key = 0;
  ISC start = hist_count_ - history_len_;
  if (start < 0) {
    start = 0;
  }
  ISC n = hist_count_ - start;
  for (ISC i = 0; i < n; ++i) {
    key = (key << 2) | history_[(start + i) % IUD(history_len_)];
  }
  key <<= IUC(2 * (history_len_ - n));  // left-align to the full width
  IUC* table = TPtr<IUC>(table_mem_.origin);
  IUC entry = table[key];
  if (entry != kRfxUnpinned) {
    return entry;
  }
  // The book's table: the action depends only on the latest percept.
  return ReflexVacuumAction(history_[hist_count_ % IUD(history_len_)] & 1,
                           (history_[hist_count_ % IUD(history_len_)] & 2) != 0);
}

void TableVacuumAgent::PinEntry(IUC packed_history, IUC action) {
  TPtr<IUC>(table_mem_.origin)[packed_history] = action;
}

void TableVacuumAgent::Reset() {
  if (history_ != NILP) {
    RfxWipe(TPtr<IUW>(history_), IUD(history_len_) * IUD(sizeof(IUC)));
  }
  hist_count_ = 0;
}

// --- Fig 2.12 SimpleReflexAgent ---------------------------------------------

SimpleReflexAgent::SimpleReflexAgent()
    : rules_mem_(NILP, &ObjectFactoryHeap), state_mem_(NILP, &ObjectFactoryHeap),
      state_words_(0), rule_count_(0), rule_cap_(0), seen_(false) {}

SimpleReflexAgent::~SimpleReflexAgent() {
  rules_mem_.origin = RfxFree(rules_mem_.origin);
  state_mem_.origin = RfxFree(state_mem_.origin);
}

BOL SimpleReflexAgent::Init(ISC state_words, ISC rule_cap) {
  if (state_words < 1 || rule_cap < 1) {
    return false;
  }
  rules_mem_.origin =
      ObjectFactoryHeap(NILP, IUD(rule_cap) * IUD(state_words + 1) *
                                  IUD(sizeof(IUC)));
  state_mem_.origin =
      ObjectFactoryHeap(NILP, IUD(state_words) * IUD(sizeof(IUC)));
  if (IsError(rules_mem_.origin) || IsError(state_mem_.origin)) {
    rules_mem_.origin = RfxFree(rules_mem_.origin);
    state_mem_.origin = RfxFree(state_mem_.origin);
    return false;
  }
  state_words_ = state_words;
  rule_cap_ = rule_cap;
  rule_count_ = 0;
  seen_ = false;
  return true;
}

BOL SimpleReflexAgent::AddRule(const IUC* condition, IUC action) {
  if (rule_count_ >= rule_cap_) {
    return false;
  }
  IUC* slot = TPtr<IUC>(rules_mem_.origin) +
              IUD(rule_count_) * IUD(state_words_ + 1);
  for (ISC i = 0; i < state_words_; ++i) {
    slot[i] = condition[i];
  }
  slot[state_words_] = action;
  rule_count_++;
  return true;
}

IUC SimpleReflexAgent::Percept(const IUC* state) {
  // state <- INTERPRET-INPUT(percept)
  IUC* s = TPtr<IUC>(state_mem_.origin);
  for (ISC i = 0; i < state_words_; ++i) {
    s[i] = state[i];
  }
  // rule <- RULE-MATCH(state, rules); first match wins
  for (ISC r = 0; r < rule_count_; ++r) {
    const IUC* slot = TPtr<IUC>(rules_mem_.origin) +
                      IUD(r) * IUD(state_words_ + 1);
    BOL match = true;
    for (ISC i = 0; i < state_words_; ++i) {
      if (slot[i] != s[i]) {
        match = false;
        break;
      }
    }
    if (match) {
      seen_ = true;
      return slot[state_words_];
    }
  }
  // action <- rule.ACTION; no match: no rule (world default)
  seen_ = false;
  return kVacSuck;
}

IUC SimpleReflexAgent::LastAction() {
  if (!seen_) {
    return kVacSuck;
  }
  // The last matched action is recomputed lazily by the world; expose the
  // stored state so the world can replay RULE-MATCH if needed.
  return Percept(TPtr<IUC>(state_mem_.origin));
}

void SimpleReflexAgent::Reset() {
  if (state_mem_.origin != NILP) {
    RfxWipe(state_mem_.origin, IUD(state_words_) * IUD(sizeof(IUC)));
  }
  seen_ = false;
}

} //< namespace iGeek
#endif
