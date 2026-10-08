// Copyright AStarship <https://astarship.net>.
// Metrics.hxx — implementations of the qualia measurement protocol
// primitives (BehaviorMetrics, StateRecord(Set), LinearProbe).
#include "Metrics.h"
#include <math.h>
#ifndef IGEEK_METRICS_IMPL
#define IGEEK_METRICS_IMPL 1
namespace _ {

// --- BehaviorMetrics ------------------------------------------------------

void BehaviorMetrics::RecordTrial(BOL was_present, BOL did_report,
                                  ISC steps_to_report) {
  ++trials;
  if (was_present && did_report) {
    ++correct;
    if (steps_to_report >= 0) {
      first_correct_step += (IUD)steps_to_report;
    }
  } else if (!was_present && did_report) {
    ++false_positives;
  } else if (was_present && !did_report) {
    ++false_negatives;
  }
  // !was_present && !did_report: correct rejection, no counter.
}

FPC BehaviorMetrics::Accuracy() const {
  if (trials == 0) {
    return 0.0f;
  }
  return (FPC)correct / (FPC)trials;
}

FPC BehaviorMetrics::FalsePositiveRate() const {
  if (trials == 0) {
    return 0.0f;
  }
  return (FPC)false_positives / (FPC)trials;
}

FPC BehaviorMetrics::FalseNegativeRate() const {
  if (trials == 0) {
    return 0.0f;
  }
  return (FPC)false_negatives / (FPC)trials;
}

// --- StateRecord ----------------------------------------------------------

StateRecord::~StateRecord() {
  delete[] words;
}

void StateRecord::Capture(const AgentState& state, ISC label) {
  delete[] words;
  words = NILP;
  slots = state.SlotCount();
  this->label = label;
  if (slots <= 0) {
    return;
  }
  words = new IUC[slots]();
  state.Snapshot(words);
}

void StateRecord::Capture(const IUC* words_in, ISC slots_in, ISC label) {
  delete[] words;
  words = NILP;
  slots = slots_in < 0 ? 0 : slots_in;
  this->label = label;
  if (slots <= 0 || words_in == NILP) {
    return;
  }
  words = new IUC[slots]();
  for (ISC i = 0; i < slots; ++i) {
    words[i] = words_in[i];
  }
}

// --- StateRecordSet -------------------------------------------------------

StateRecordSet::StateRecordSet(ISC cap)
    : cap_(cap < 1 ? 1 : cap), count_(0),
      records_(new StateRecord[cap_]) {}

StateRecordSet::~StateRecordSet() {
  delete[] records_;
}

BOL StateRecordSet::Push(const AgentState& state, ISC label) {
  if (count_ >= cap_) {
    return false;
  }
  records_[count_].Capture(state, label);
  ++count_;
  return true;
}

// --- LinearProbe ----------------------------------------------------------

LinearProbe::LinearProbe(ISC feature_dim)
    : feature_dim_(feature_dim < 1 ? 1 : feature_dim),
      weights_(new FPC[feature_dim_]()), bias_(0.0f) {}

LinearProbe::~LinearProbe() {
  delete[] weights_;
}

FPC LinearProbe::Train(const StateRecordSet& data, ISC epochs, FPC lr) {
  ISC n = data.Count();
  if (n == 0) {
    return 0.0f;
  }
  // Full-batch logistic regression: z = w.x + b; p = sigmoid(z);
  // loss = -[y log p + (1-y) log(1-p)]; dw = (p-y)/n * x; db = (p-y)/n.
  // Feature scaling: IUC state words can be large; normalize by the max abs
  // word across the set so the sigmoid stays in a trainable range.
  FPC max_abs = 1e-6f;
  for (ISC i = 0; i < n; ++i) {
    const IUC* w = data.At(i).words;
    for (ISC f = 0; f < feature_dim_; ++f) {
      FPC a = (FPC)w[f];
      if (a < 0.0f) {
        a = -a;
      }
      if (a > max_abs) {
        max_abs = a;
      }
    }
  }
  FPC inv_scale = 1.0f / max_abs;
  for (ISC e = 0; e < epochs; ++e) {
    FPC dw[64];  // feature_dim_ is small for a probe; guard below.
    if (feature_dim_ > 64) {
      // Fall back to per-sample accumulation (no fixed buffer needed).
      for (ISC f = 0; f < feature_dim_; ++f) {
        FPC grad = 0.0f;
        for (ISC i = 0; i < n; ++i) {
          FPC z = bias_;
          for (ISC k = 0; k < feature_dim_; ++k) {
            z += weights_[k] * (FPC)data.At(i).words[k] * inv_scale;
          }
          FPC p = 1.0f / (1.0f + (FPC)exp(-z));
          grad += (p - (FPC)data.At(i).label) * (FPC)data.At(i).words[f] *
                  inv_scale;
        }
        weights_[f] -= lr * grad / (FPC)n;
      }
    } else {
      for (ISC f = 0; f < feature_dim_; ++f) {
        dw[f] = 0.0f;
      }
      FPC db = 0.0f;
      for (ISC i = 0; i < n; ++i) {
        FPC z = bias_;
        for (ISC k = 0; k < feature_dim_; ++k) {
          z += weights_[k] * (FPC)data.At(i).words[k] * inv_scale;
        }
        FPC p = 1.0f / (1.0f + (FPC)exp(-z));
        FPC err = p - (FPC)data.At(i).label;
        db += err;
        for (ISC k = 0; k < feature_dim_; ++k) {
          dw[k] += err * (FPC)data.At(i).words[k] * inv_scale;
        }
      }
      for (ISC f = 0; f < feature_dim_; ++f) {
        weights_[f] -= lr * dw[f] / (FPC)n;
      }
      bias_ -= lr * db / (FPC)n;
    }
  }
  return Accuracy(data);
}

ISC LinearProbe::Predict(const IUC* words, ISC feature_dim) const {
  if (words == NILP) {
    return 0;
  }
  FPC z = bias_;
  ISC n = feature_dim < feature_dim_ ? feature_dim : feature_dim_;
  for (ISC f = 0; f < n; ++f) {
    z += weights_[f] * (FPC)words[f];
  }
  return z > 0.0f ? 1 : 0;
}

FPC LinearProbe::Accuracy(const StateRecordSet& data) const {
  ISC n = data.Count();
  if (n == 0) {
    return 0.0f;
  }
  ISC correct = 0;
  for (ISC i = 0; i < n; ++i) {
    ISC pred = Predict(data.At(i).words, feature_dim_);
    if (pred == data.At(i).label) {
      ++correct;
    }
  }
  return (FPC)correct / (FPC)n;
}

FPC LinearProbe::Weight(ISC i) const {
  if (i < 0 || i >= feature_dim_) {
    return 0.0f;
  }
  return weights_[i];
}

}  // namespace _
#endif
