// Copyright AStarship <https://astarship.net>.
#include "AgentState.h"
#include "../ASCIICrabs/Autoject.hxx"
#ifndef IGEEK_AGENTSTATE_IMPL
#define IGEEK_AGENTSTATE_IMPL 1
namespace _ {

AgentState::AgentState(ISC slot_count)
    : autoject_{NILP, &ObjectFactoryHeap},
      words_(NILP), slot_count_(slot_count < 1 ? 1 : slot_count) {
  // Size the buffer in words. Each IUC slot lives in one IUW word, so the
  // byte size is slot_count * sizeof(IUW). ObjectFactoryHeap word-aligns up.
  ISW bytes = (ISW)(slot_count_ * (ISC)sizeof(IUW));
  autoject_.origin = ObjectFactoryStack(NILP, bytes);
  words_ = reinterpret_cast<IUC*>(autoject_.origin);
  if (words_ != NILP) {
    Clear();
  }
}

AgentState::~AgentState() {
  if (autoject_.origin != NILP) {
    // The heap factory deletes the buffer when called with a non-nil origin.
    autoject_.ram(autoject_.origin, (ISW)(slot_count_ * (ISC)sizeof(IUW)));
    autoject_.origin = NILP;
  }
  words_ = NILP;
}

IUC AgentState::Get(ISC i) const {
  if (i < 0 || i >= slot_count_) {
    return 0;
  }
  return words_[i];
}

void AgentState::Set(ISC i, IUC value) {
  if (i < 0 || i >= slot_count_) {
    return;
  }
  words_[i] = value;
}

void AgentState::Clear() {
  for (ISC i = 0; i < slot_count_; ++i) {
    words_[i] = 0;
  }
}

ISC AgentState::Snapshot(IUC* out) const {
  if (out == NILP || words_ == NILP) {
    return slot_count_;
  }
  for (ISC i = 0; i < slot_count_; ++i) {
    out[i] = words_[i];
  }
  return slot_count_;
}

void AgentState::Restore(const IUC* in) {
  if (in == NILP || words_ == NILP) {
    return;
  }
  for (ISC i = 0; i < slot_count_; ++i) {
    words_[i] = in[i];
  }
}

IUD AgentState::ByteSize() const {
  return (IUD)slot_count_ * (IUD)sizeof(IUW);
}

}  // namespace _
#endif
