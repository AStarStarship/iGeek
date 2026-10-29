// Copyright AStarship <https://astarship.net>.
// TypingWorld.hxx — implementation of the deterministic typing-mine engine.
// No-stdlib: fixed arrays, a seeded 64-bit LCG, no heap, no std containers.
// The only "system" surface is COut for Print() (the ASCIICrabs console
// primitive, matching MazeWorld's no-stdlib convention).
#include "TypingWorld.h"
#include "../../ASCIICrabs/COut.h"
#if SEAM >= IGEEK_TYPINGWORLD
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

ISC TypingWorld::TargetIndex() const {
  return target_;
}

const TypingWorldBlock* TypingWorld::Target() const {
  if (target_ < 0) return NILP;
  return &blocks_[target_];
}

ISC TypingWorld::TargetTyped(CHA* out) const {
  if (out == NILP) return 0;
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

BOL TypingWorld::Done() const {
  return target_ < 0;
}

ISC TypingWorld::MinedCount() const {
  return mined_;
}

ISC TypingWorld::Remaining() const {
  return BlockCount - mined_;
}

ISC TypingWorld::KeysPressed() const {
  return keys_;
}

ISC TypingWorld::KeysCorrect() const {
  return correct_;
}

ISC TypingWorld::KeysMissed() const {
  return missed_;
}

ISC TypingWorld::NextUnmined(ISC i) const {
  for (ISC step = 0; step < BlockCount; ++step) {
    ISC idx = (i + step) % BlockCount;
    if (!blocks_[idx].mined) return idx;
  }
  return -1;
}

void TypingWorld::Print() const {
  ::_::StdOut() << Headingf("TypingWorld (typing mine)");
  for (ISC y = 0; y < GridHeight; ++y) {
    ::_::StdOut() << ' ';
    for (ISC x = 0; x < GridWidth; ++x) {
      ISC b = y * GridWidth + x;
      CHA c = blocks_[b].mined ? '.' : '#';
      if (b == target_) c = '@';
      ::_::StdOut() << c << ' ';
    }
    ::_::StdOut() << "\n";
  }
  ::_::StdOut() << "mined: " << mined_ << "/" << BlockCount
                << "  keys: " << keys_
                << "  ok: " << correct_
                << "  miss: " << missed_;
  if (target_ >= 0) {
    CHA buf[TypingWorldBlock::BlockWordMax + 1];
    TargetTyped(buf);
    ::_::StdOut() << "  target: " << buf;
  }
  ::_::StdOut() << "\n";
}

} //< namespace iGeek
#endif
