// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_METRICS_DECL
#define IGEEK_METRICS_DECL 1
#include "../ASCIICrabs/_Config.h"
#include "AgentState.h"

namespace _ {

/* Metrics — the qualia measurement protocol primitives
 * (iGeek AGENT_PLAN.md section 3.4, the "actual research contribution").
 *
 * These are world-agnostic accumulators: a world feeds them per-episode
 * outcomes (correct/incorrect/false-positive/false-negative) and internal
 * state snapshots; the protocol produces the curves and probes the plan
 * specifies. No red/blue/green knowledge lives here — a world maps its
 * outcomes onto these counters.
 *
 * No C++ standard library: fixed-size IUC/IUD buffers, new/delete[] builtins.
 */

/* Behavior metrics for one stimulus across training. All counters are IUD so
 * they never overflow for long runs. (Decision: single scalar reward per step;
 * these are the per-stimulus tallies the plan's "Behavior metrics" list names.)
 *
 * Fields (plan 3.4):
 *   correct            — correct identifications (e.g. red reported when red).
 *   false_positives    — reported the target when it was not present.
 *   false_negatives    — failed to report the target when it was present.
 *   trials             — total decision trials (denominator for accuracy).
 *   first_correct_step — latency in steps to first correct report per episode,
 *                        summed; divide by episodes for mean latency.
 */
struct BehaviorMetrics {
  IUD correct;
  IUD false_positives;
  IUD false_negatives;
  IUD trials;
  IUD first_correct_step;

  BehaviorMetrics()
      : correct(0), false_positives(0), false_negatives(0), trials(0),
        first_correct_step(0) {}

  /* Record one decision trial.
  @param was_present  True if the target stimulus was present this trial.
  @param did_report   True if the agent reported the target.
  @param steps_to_report  Steps taken before the (first) correct report, if
                          any this episode; pass -1 for no correct report. */
  void RecordTrial(BOL was_present, BOL did_report, ISC steps_to_report);

  /* Accuracy = correct / trials (FPC in [0,1]; 0 if no trials). */
  FPC Accuracy() const;
  /* False-positive rate = false_positives / trials. */
  FPC FalsePositiveRate() const;
  /* False-negative rate = false_negatives / trials. */
  FPC FalseNegativeRate() const;
};

/* A single internal-state snapshot record: one agent's state vector at one
 * step, tagged with a label (0 = non-target context, 1 = target context, e.g.
 * "red present") so the protocol can later cluster/probe by context. */
struct StateRecord {
  ISC slots;       // number of IUC slots in this snapshot.
  ISC label;       // context label (world-defined; 0/1 for presence).
  IUC* words;      // slots IUC words (owned).

  StateRecord() : slots(0), label(0), words(NILP) {}
  ~StateRecord();
  /* Deep-copy a snapshot from an AgentState into this record. */
  void Capture(const AgentState& state, ISC label);
  /* Deep-copy a raw word array. */
  void Capture(const IUC* words_in, ISC slots_in, ISC label);
};

/* A fixed-capacity collection of state records (no std::vector). The qualia
 * state-space metrics (3.4) walk these: cluster by label, check stability,
 * and run the linear probe. */
class StateRecordSet {
 public:
  explicit StateRecordSet(ISC cap);
  ~StateRecordSet();
  StateRecordSet(const StateRecordSet&) = delete;
  StateRecordSet& operator=(const StateRecordSet&) = delete;

  /* Append a captured record. Returns false if at capacity. */
  BOL Push(const AgentState& state, ISC label);
  ISC Count() const { return count_; }
  const StateRecord& At(ISC i) const { return records_[i]; }
  ISC Capacity() const { return cap_; }

 private:
  ISC cap_;
  ISC count_;
  StateRecord* records_;
};

/* Linear probe: can a LINEAR function of the state words predict the label
 * (e.g. "target present")? A high accuracy is the strongest operational
 * evidence for a target-specific representation (plan 3.4 "Probing").
 *
 * Trained by full-batch gradient descent on logistic loss over the fixed
 * record set. World-agnostic: labels are ISC (0/1), features are the IUC
 * state words (cast to FPC).
 */
class LinearProbe {
 public:
  /* @param feature_dim Number of feature words (== state slots). */
  explicit LinearProbe(ISC feature_dim);
  ~LinearProbe();
  LinearProbe(const LinearProbe&) = delete;
  LinearProbe& operator=(const LinearProbe&) = delete;

  /* Train for `epochs` full-batch gradient steps at learning rate `lr`.
  Returns the training accuracy after the final epoch (FPC in [0,1]). */
  FPC Train(const StateRecordSet& data, ISC epochs, FPC lr);

  /* Predict the label (0/1) for a single state word array. */
  ISC Predict(const IUC* words, ISC feature_dim) const;

  /* Accuracy over the (training) record set. */
  FPC Accuracy(const StateRecordSet& data) const;

  /* The learned weight vector (feature_dim FPC) + bias, for inspection. */
  FPC Weight(ISC i) const;
  FPC Bias() const { return bias_; }

 private:
  ISC feature_dim_;
  FPC* weights_;  // feature_dim_ (owned).
  FPC bias_;
};

}  // namespace _
#endif
