// Copyright AStarship <https://astarship.net>.
// Agent.hxx — implementations of the three generic iGeek agent tiers
// (AgentBase contract + TTableAgent / TReflexAgent / TModelAgent).
//
// Puffer/plan-informed (iGeek AGENT_PLAN.md 3.3): these are the world-agnostic
// Percept->Action + state tiers. No world-specific stimulus knowledge lives
// here — a world binds percepts/actions to its own tokens.
#include "Agent.h"
#include "AgentState.hxx"
#ifndef IGEEK_AGENT_IMPL
#define IGEEK_AGENT_IMPL 1
namespace _ {

// --- TTableAgent ----------------------------------------------------------

TTableAgent::TTableAgent(ISC action_count, ISC history_len, ISC percept_len)
    : action_count_(action_count < 1 ? 1 : action_count),
      history_len_(history_len < 1 ? 1 : history_len),
      percept_len_(percept_len < 1 ? 1 : percept_len),
      key_len_(0), key_count_(0), keys_(NILP), actions_(NILP),
      history_(NILP), hist_pos_(0), hist_len_(0) {
  key_len_ = history_len_ * percept_len_;
  // Reserve a small key table; the world binds entries via SetEntry, which
  // grows the table by realloc (new[] / delete[] are compiler builtins).
  key_count_ = 0;
  keys_ = NILP;
  actions_ = NILP;
  history_ = new IUC[key_len_]();
}

TTableAgent::~TTableAgent() {
  delete[] keys_;
  delete[] actions_;
  delete[] history_;
}

void TTableAgent::OnPercept(const IUC* percept, ISC percept_len) {
  if (percept == NILP) {
    return;
  }
  // Shift the rolling window left by one percept, append the new one at the
  // end. The window holds key_len_ = history_len_ * percept_len_ words.
  for (ISC i = 0; i + percept_len_ < key_len_; ++i) {
    history_[i] = history_[i + percept_len_];
  }
  for (ISC i = 0; i < percept_len_; ++i) {
    history_[key_len_ - percept_len_ + i] = percept[i];
  }
  if (hist_len_ < key_len_) {
    hist_len_ += percept_len_;
  }
}

IUC TTableAgent::Action() {
  // Exact-match lookup over the bound keys (last key_len_ history words).
  for (ISC k = 0; k < key_count_; ++k) {
    BOL match = true;
    for (ISC i = 0; i < key_len_; ++i) {
      if (keys_[k * key_len_ + i] != history_[i]) {
        match = false;
        break;
      }
    }
    if (match) {
      return actions_[k];
    }
  }
  return 0;  // no bound entry: the world's action 0 is the fallback.
}

const AgentState* TTableAgent::State() const {
  return NILP;  // the table agent keeps no internal model.
}

void TTableAgent::Reset() {
  if (history_ != NILP) {
    for (ISC i = 0; i < key_len_; ++i) {
      history_[i] = 0;
    }
  }
  hist_pos_ = 0;
  hist_len_ = 0;
}

ISC TTableAgent::ActionCount() const {
  return action_count_;
}

void TTableAgent::SetEntry(const IUC* key, IUC action) {
  // Grow the table by one slot (realloc). keys_/actions_ are owned here.
  ISC new_count = key_count_ + 1;
  IUC* new_keys = new IUC[(IUD)new_count * (IUD)key_len_]();
  IUC* new_actions = new IUC[new_count]();
  if (keys_ != NILP) {
    for (ISC i = 0; i < key_count_ * key_len_; ++i) {
      new_keys[i] = keys_[i];
    }
    for (ISC i = 0; i < key_count_; ++i) {
      new_actions[i] = actions_[i];
    }
    delete[] keys_;
    delete[] actions_;
  }
  for (ISC i = 0; i < key_len_; ++i) {
    new_keys[(IUD)key_count_ * key_len_ + i] = key[i];
  }
  new_actions[key_count_] = action;
  keys_ = new_keys;
  actions_ = new_actions;
  key_count_ = new_count;
}

// --- TReflexAgent ---------------------------------------------------------

TReflexAgent::TReflexAgent(ISC action_count, ISC percept_len, ISC rule_cap)
    : action_count_(action_count < 1 ? 1 : action_count),
      percept_len_(percept_len < 1 ? 1 : percept_len),
      rule_cap_(rule_cap < 1 ? 1 : rule_cap), rule_count_(0),
      conditions_(NILP), rule_actions_(NILP), current_(NILP),
      default_action_(0) {
  conditions_ = new IUC[(IUD)rule_cap_ * (IUD)percept_len_]();
  rule_actions_ = new IUC[rule_cap_]();
  current_ = new IUC[percept_len_]();
}

TReflexAgent::~TReflexAgent() {
  delete[] conditions_;
  delete[] rule_actions_;
  delete[] current_;
}

void TReflexAgent::OnPercept(const IUC* percept, ISC percept_len) {
  if (percept == NILP) {
    return;
  }
  for (ISC i = 0; i < percept_len_; ++i) {
    current_[i] = percept[i];
  }
}

IUC TReflexAgent::Action() {
  for (ISC r = 0; r < rule_count_; ++r) {
    BOL match = true;
    for (ISC i = 0; i < percept_len_; ++i) {
      if (conditions_[r * percept_len_ + i] != current_[i]) {
        match = false;
        break;
      }
    }
    if (match) {
      return rule_actions_[r];
    }
  }
  return default_action_;
}

void TReflexAgent::Reset() {
  for (ISC i = 0; i < percept_len_; ++i) {
    current_[i] = 0;
  }
}

ISC TReflexAgent::ActionCount() const {
  return action_count_;
}

BOL TReflexAgent::AddRule(const IUC* condition, IUC action) {
  if (condition == NILP || rule_count_ >= rule_cap_) {
    return false;
  }
  for (ISC i = 0; i < percept_len_; ++i) {
    conditions_[rule_count_ * percept_len_ + i] = condition[i];
  }
  rule_actions_[rule_count_] = action;
  ++rule_count_;
  return true;
}

void TReflexAgent::SetDefaultAction(IUC action) {
  default_action_ = action;
}

// --- TModelAgent ----------------------------------------------------------

TModelAgent::TModelAgent(ISC action_count, ISC state_slots)
    : action_count_(action_count < 1 ? 1 : action_count),
      state_slots_(state_slots < 1 ? 1 : state_slots),
      state_(state_slots_) {}

TModelAgent::~TModelAgent() {}

void TModelAgent::OnPercept(const IUC* percept, ISC percept_len) {
  if (percept == NILP) {
    return;
  }
  // Generic linear model update: accumulate percept words into the model's
  // slots (slot i += percept[i % state_slots_]). This is a deliberately simple
  // world-agnostic transform so the tier is testable in isolation; a world can
  // later specialize the update via a master-agent review card. The point of
  // the tier is that an internal state EXISTS and is probeable, not that this
  // particular transform is the "right" one.
  for (ISC i = 0; i < percept_len; ++i) {
    ISC slot = i % state_slots_;
    state_.Set(slot, state_.Get(slot) + percept[i]);
  }
}

IUC TModelAgent::Action() {
  // Generic read: the action is the index of the slot holding the largest
  // accumulated value (ties -> lowest index). Bounded by action_count_.
  ISC best = 0;
  IUC best_val = state_.Get(0);
  for (ISC i = 1; i < state_slots_; ++i) {
    IUC v = state_.Get(i);
    if (v > best_val) {
      best_val = v;
      best = i;
    }
  }
  if (best >= action_count_) {
    best = action_count_ - 1;
  }
  return (IUC)best;
}

const AgentState* TModelAgent::State() const {
  return &state_;
}

void TModelAgent::Reset() {
  state_.Clear();
}

ISC TModelAgent::ActionCount() const {
  return action_count_;
}

}  // namespace _
#endif
