// Copyright AStarship <https://astarship.net>.
#include "AgentState.h"
#include "../ASCIICrabs/Autoject.hxx"
#ifndef _iGeekAgentStateHXX
#define _iGeekAgentStateHXX
namespace iGeek {

AgentState::AgentState(ISC slot_count)
    : autoject_{_NilP, &ObjectFactoryHeap},
      words_(_NilP), slot_count_(slot_count < 1 ? 1 : slot_count) {
  // Size the buffer in words. Each IUC slot lives in one IUW word, so the
  // byte size is slot_count * sizeofIUW(. ObjectFactoryHeap word-aligns up.
  ISW bytes = ISW(slot_count_ * ISC(sizeof(IUW)));
  autoject_.origin = ObjectFactoryStack(_NilP, bytes);
  words_ = reinterpret_cast<IUC*>(autoject_.origin);
  if (words_ != _NilP) {
    Clear();
  }
}

AgentState::~AgentState() {
  if (autoject_.origin != _NilP) {
    // The heap factory deletes the buffer when called with a non-nil origin.
    autoject_.ram(autoject_.origin, ISW(slot_count_ * ISC(sizeof(IUW))));
    autoject_.origin = _NilP;
  }
  words_ = _NilP;
}

IUC AgentState::Get(ISC i) {
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

ISC AgentState::Snapshot(IUC* out) {
  if (out == _NilP || words_ == _NilP) {
    return slot_count_;
  }
  for (ISC i = 0; i < slot_count_; ++i) {
    out[i] = words_[i];
  }
  return slot_count_;
}

void AgentState::Restore(const IUC* in) {
  if (in == _NilP || words_ == _NilP) {
    return;
  }
  for (ISC i = 0; i < slot_count_; ++i) {
    words_[i] = in[i];
  }
}

IUD AgentState::ByteSize() {
  return IUD(slot_count_) * IUD(sizeof(IUW));
}

}       //< namespace iGeek
#endif  //< _iGeekAgentStateHXX
