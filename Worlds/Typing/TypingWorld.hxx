// Copyright AStarship <https://astarship.net>.
// TypingWorld.hxx — implementation of the deterministic typing-mine engine.
// No-stdlib: fixed arrays, a seeded 64-bit LCG, no heap, no std containers.
// The only "system" surface is ROut for Print() (the ASCIICrabs console
// primitive, matching MazeWorld's no-stdlib convention).
#include "Stringf.hpp"
#include "../../Gym.h"
#include "TypingWorld.h"
#include "../../../ASCIICrabs/ROut.h"
#if _Seam >= _iGeekTypingWorldCore
namespace iGeek {

/* Small fixed vocabulary for block words. Kept short and printable so the
   engine is deterministic and testable without a real keyboard layout.
   Indexed by the LCG -> each block gets a reproducible word per seed. */
static const CHA* const kWords[] = {
    "the", "and", "for", "you", "are", "min", "ing", "typ",
    "ing", "block", "stone", "dirt", "coal", "iron", "gold", "tree",
};
static const ISC kWordCount =
    sizeof(kWords) / sizeof(kWords[0]);

/* 64-bit LCG (Numerical Recipes constants). Deterministic per seed — the
   world generator must not depend on std::random. */
static IUD LcgNext(IUD* state) {
  *state = *state * 6364136223846793005ULL + 1442695040888963407ULL;
  return *state;
}

/* True for printable ASCII we accept as a valid keystroke. */
static BOL IsTypeChar(CHA ch) {
  return ch >= ' ' && ch <= '~';
}

TypingWorld::TypingWorld(IUD seed)
    : rng_(0), seed_(seed ? seed : 1),
      target_(-1), mined_(0), keys_(0), correct_(0), missed_(0),
      inv_count_(0) {
  Reset();
}

void TypingWorld::Reset() {
  Generate(seed_);
}

void TypingWorld::Generate(IUD seed) {
  rng_ = seed ? seed : 1;
  for (ISC b = 0; b < BlockCount; ++b) {
    blocks_[b].word_len = 0;
    blocks_[b].hits = 0;
    blocks_[b].mined = false;
    AssignWord(b);
  }
  inv_count_ = 0;
  mined_ = 0;
  keys_ = 0;
  correct_ = 0;
  missed_ = 0;
  target_ = NextUnmined(0);
}

void TypingWorld::AssignWord(ISC b) {
  const CHA* w = kWords[LcgNext(&rng_) % kWordCount];
  ISC n = 0;
  while (w[n] != '\0' && n < TypingWorldBlock::BlockWordMax - 1) {
    blocks_[b].word[n] = w[n];
    ++n;
  }
  blocks_[b].word_len = n;
}

BOL TypingWorld::PressKey(CHA ch) {
  if (target_ < 0) return false;
  ++keys_;
  TypingWorldBlock* blk = &blocks_[target_];
  if (!IsTypeChar(ch)) {
    ++missed_;
    return false;
  }
  /* Correct only if it matches the next expected character. */
  if (blk->hits < blk->word_len && ch == blk->word[blk->hits]) {
    ++blk->hits;
    ++correct_;
    if (blk->hits == blk->word_len) {
      blk->mined = true;
      ++mined_;
      /* Store the mined word in the inventory (bounded). */
      if (inv_count_ < InvMax) {
        ISC n = blk->word_len;
        for (ISC i = 0; i < n; ++i) inv_[inv_count_][i] = blk->word[i];
        inv_[inv_count_][n] = '\0';
        ++inv_count_;
      }
      target_ = NextUnmined(target_ + 1);
    }
    return true;
  }
  ++missed_;
  return false;
}

ISC TypingWorld::TargetIndex() {
  return target_;
}

const TypingWorldBlock* TypingWorld::Target() {
  if (target_ < 0) return _NilP;
  return &blocks_[target_];
}

ISC TypingWorld::TargetWord(CHA* out) {
  if (out == _NilP) return 0;
  if (target_ < 0) {
    out[0] = '\0';
    return 0;
  }
  const TypingWorldBlock* blk = &blocks_[target_];
  for (ISC i = 0; i < blk->word_len; ++i) out[i] = blk->word[i];
  out[blk->word_len] = '\0';
  return blk->word_len;
}

ISC TypingWorld::TargetTyped(CHA* out) {
  if (out == _NilP) return 0;
  if (target_ < 0) {
    out[0] = '\0';
    return 0;
  }
  const TypingWorldBlock* blk = &blocks_[target_];
  ISC n = blk->hits;
  for (ISC i = 0; i < n; ++i) out[i] = blk->word[i];
  out[n] = '\0';
  return n;
}

BOL TypingWorld::Done() {
  return target_ < 0;
}

ISC TypingWorld::MinedCount() {
  return mined_;
}

ISC TypingWorld::Remaining() {
  return BlockCount - mined_;
}

ISC TypingWorld::KeysPressed() {
  return keys_;
}

ISC TypingWorld::KeysCorrect() {
  return correct_;
}

ISC TypingWorld::KeysMissed() {
  return missed_;
}

ISC TypingWorld::NextUnmined(ISC i) {
  for (ISC step = 0; step < BlockCount; ++step) {
    ISC idx = (i + step) % BlockCount;
    if (!blocks_[idx].mined) return idx;
  }
  return -1;
}

void TypingWorld::Print() {
  ::_::RefOut() << Headingf("TypingWorld (typing mine)");
  for (ISC y = 0; y < GridHeight; ++y) {
    ::_::RefOut() << ' ';
    for (ISC x = 0; x < GridWidth; ++x) {
      ISC b = y * GridWidth + x;
      CHA c = blocks_[b].mined ? '.' : '#';
      if (b == target_) c = '@';
      ::_::RefOut() << c << ' ';
    }
    ::_::RefOut() << "\n";
  }
  ::_::RefOut() << "mined: " << mined_ << "/" << BlockCount
                << "  keys: " << keys_
                << "  ok: " << correct_
                << "  miss: " << missed_;
  if (target_ >= 0) {
    CHA buf[TypingWorldBlock::BlockWordMax + 1];
    TargetTyped(buf);
    ::_::RefOut() << "  target: " << buf;
  }
  ::_::RefOut() << "\n";
}


/* ------------------------------------------------------------------ TypingEnv */

TypingEnv::TypingEnv(IUD seed)
    : Environment("typingworld", 2), seed_(seed ? seed : IUD(7)) {
  /* The env IS the Environment: it owns the Room and the manifest a joining
     agent reads before it sends its first keystroke. */
  TSPrint(manifest_.name, ISW(ManifestNameLen), "typingworld");
  TSPrint(manifest_.version, ISW(ManifestVersionLen), "1.0");
  manifest_.kind = ManifestKindEnv;
  manifest_.domain = ManifestDomainGame;
  TSPrint(manifest_.description, ISW(ManifestDescriptionLen),
          "Typing tutor: mine blocks by typing their words");
  TSPrint(manifest_.modality, ISW(ManifestModalityLen), "vector");
  manifest_.reward_low = FPC(-0.1);
  manifest_.reward_high = FPC(1.0);
  manifest_.reward_higher_is_better = true;
  manifest_.determinism = ManifestDeterminismSeeded;
  manifest_.observation_count = ISC(ObservationLength);
  manifest_.action_count = ISC(ActionCount);
  manifest_.max_episode_steps = ISC(MaxSteps);
  manifest_.environment_count_max = ISC(GymEnvMax);
  TSPrint(manifest_.world_name, ISW(ManifestWorldLen), "TypingWorld");
  manifest_.seam_number = _iGeekTypingWorldCore;
  manifest_.manifest_version = ISC(1);
  Reset();
}

void TypingEnv::Reset() {
  world_.Reset();
  steps_ = 0;
  Observe();
}

FPC TypingEnv::Step(ISC action) {
  ISC a = action;
  if (a < 0) a = 0;
  if (a >= ISC(ActionCount)) a = ISC(ActionCount) - 1;
  const CHA ch = CHA('a' + a);
  const ISC correct_before = world_.KeysCorrect();
  const ISC mined_before = world_.MinedCount();
  world_.PressKey(ch);
  ++steps_;
  Observe();
  if (world_.MinedCount() > mined_before) return FPC(1.0f);   //< block mined
  return FPC(world_.KeysCorrect() > correct_before ? 0.1f : -0.1f);
}

void TypingEnv::Observe() {
  for (ISC i = 0; i < ISC(ObservationLength); ++i) observation_[i] = 0.0f;
  CHA word[24];
  const ISC n = world_.TargetWord(word);
  const ISC word_max = ISC(ObservationLength) - 4;
  for (ISC i = 0; i < n && i < word_max; ++i)
    observation_[i] = FPC(word[i] - 'a' + 1) / 27.0f;
  CHA typed[24];
  const ISC t = world_.TargetTyped(typed);
  observation_[word_max] = n > 0 ? FPC(t) / FPC(n) : 0.0f;
  observation_[word_max + 1] =
      FPC(world_.Remaining()) / FPC(TypingWorld::BlockCount);
  observation_[word_max + 2] =
      FPC(world_.MinedCount()) / FPC(TypingWorld::BlockCount);
  observation_[word_max + 3] = world_.Done() ? 1.0f : 0.0f;
}

} //< namespace iGeek
#endif
