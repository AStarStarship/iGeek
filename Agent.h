// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekAgentH
#define _iGeekAgentH 1
#include "../ASCIICrabs/_Config.h"
#include "AgentState.h"
namespace iGeek {

/* Minimal agent contract shared by all three tiers. Generic, no stimulus. */
class AgentBase {
 public:
  virtual ~AgentBase() {}

  /* Observe a percept (IUC word array, length percept_len); update state. */
  virtual void OnPercept(const IUC* percept, ISC percept_len) = 0;

  /* Choose the next action (a single IUC word). */
  virtual IUC Action() = 0;

  /* Internal state for inspection/snapshot. _NilP for stateless tiers. */
  virtual AgentState* State() { return 0; }

  /* Reset internal state to a fresh episode. */
  virtual void Reset() = 0;

  /* Number of distinct actions the tier can emit (the world's action count). */
  virtual ISC ActionCount() = 0;
};

/* Tier 1 — table-driven: maps the last history_len percepts (concatenated
IUC words) to an action via a fixed lookup table. Shows whether the task is
even learnable. */
class TableAgent : public AgentBase {
 public:
  TableAgent(ISC action_count, ISC history_len, ISC percept_len);
  ~TableAgent() override;
  void OnPercept(const IUC* percept, ISC percept_len) override;
  IUC Action() override;
  AgentState* State() override;
  void Reset() override;
  ISC ActionCount() override;
  /* Bind a key (history_len*percept_len IUC words) to an action. The world
  fills the table; the agent just looks up. */
  void SetEntry(const IUC* key, IUC action);

 private:
  ISC action_count_;
  ISC history_len_;
  ISC percept_len_;
  ISC key_len_;
  ISC key_count_;
  const IUC* keys_;
  IUC* actions_;
  IUC* history_;
  ISC hist_pos_;
  ISC hist_len_;
};

/* Tier 2 — simple reflex: condition-action rules on the CURRENT percept only
(no history). The world registers rules; the agent matches the first exact
word-equal rule and takes its action, else the default. */
class ReflexAgent : public AgentBase {
 public:
  ReflexAgent(ISC action_count, ISC percept_len, ISC rule_cap);
  ~ReflexAgent() override;
  void OnPercept(const IUC* percept, ISC percept_len) override;
  IUC Action() override;
  void Reset() override;
  ISC ActionCount() override;
  /* If current percept == condition (percept_len words), take `action`. */
  BOL AddRule(const IUC* condition, IUC action);
  void SetDefaultAction(IUC action);

 private:
  ISC action_count_;
  ISC percept_len_;
  ISC rule_cap_;
  ISC rule_count_;
  IUC* conditions_;
  IUC* rule_actions_;
  IUC* current_;
  IUC default_action_;
};

/* Tier 3 — model-based reflex: maintains an internal AgentState world model.
On each percept it updates the model; on Action() it reads the model. This
is the tier whose internal state the qualia metrics probe (plan 3.4). */
class ModelAgent : public AgentBase {
 public:
  ModelAgent(ISC action_count, ISC state_slots);
  ~ModelAgent() override;
  void OnPercept(const IUC* percept, ISC percept_len) override;
  IUC Action() override;
  AgentState* State() override;
  void Reset() override;
  ISC ActionCount() override;

 private:
  ISC action_count_;
  ISC state_slots_;
  AgentState state_;
};

} //< namespace iGeek
#endif
