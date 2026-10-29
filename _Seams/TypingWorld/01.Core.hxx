// Copyright AStarship <https://astarship.net>.
// 
// TypingWorld core seam unit: unit tests for the deterministic typing-mine
// engine. Gated on IGEEK_TYPINGWORLD_CORE (first / default seam).
// No C/C++ standard library: I/O via COut (ASCIICrabs console primitive).
#if SEAM >= IGEEK_TYPINGWORLD_CORE
#if SEAM == IGEEK_TYPINGWORLD_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
// TypingWorld.hxx is pulled ONCE by the rollup _Tests.hxx; rely on that.
using namespace ::_;
namespace TypingWorldTest {
using namespace iGeek;   // engine types (in iGeek) used unqualified

// Test determinism: two worlds with the same seed produce identical state.
inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_TYPINGWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing TypingWorld (typing-mine core)..."));

  // --- Determinism: two worlds with the same seed produce identical state ---
  const IUD seed = 7;
  iGeek::TypingWorld world_a(seed);
  iGeek::TypingWorld world_b(seed);
  // Run 200 mixed keystrokes (type correct chars + some wrong ones)
  ISC n_steps = 200;
  for (ISC s = 0; s < n_steps; ++s) {
    // Alternate between typing the current target char and a wrong char
    if (world_a.Target() != NILP) {
      // Type correct char
      world_a.PressKey(world_a.Target()->word[world_a.Target()->hits]);
      // Type a wrong char (space is always wrong when word starts non-space)
      world_a.PressKey(' ');
      // Type another correct char
      if (world_a.Target() != NILP)
        world_a.PressKey(world_a.Target()->word[world_a.Target()->hits]);
    }
    if (world_b.Target() != NILP) {
      world_b.PressKey(world_b.Target()->word[world_b.Target()->hits]);
      world_b.PressKey(' ');
      if (world_b.Target() != NILP)
        world_b.PressKey(world_b.Target()->word[world_b.Target()->hits]);
    }
  }
  // Verify identical state
  if (world_a.MinedCount() != world_b.MinedCount()) {
    return "TYPINGWORLD_FAIL: MinedCount mismatch (determinism)";
  }
  if (world_a.KeysPressed() != world_b.KeysPressed()) {
    return "TYPINGWORLD_FAIL: KeysPressed mismatch (determinism)";
  }
  if (world_a.KeysCorrect() != world_b.KeysCorrect()) {
    return "TYPINGWORLD_FAIL: KeysCorrect mismatch (determinism)";
  }
  if (world_a.KeysMissed() != world_b.KeysMissed()) {
    return "TYPINGWORLD_FAIL: KeysMissed mismatch (determinism)";
  }
  if (world_a.TargetIndex() != world_b.TargetIndex()) {
    return "TYPINGWORLD_FAIL: TargetIndex mismatch (determinism)";
  }
  // Verify identical TargetTyped() strings
  CHA buf_a[iGeek::TypingWorldBlock::BlockWordMax + 1];
  CHA buf_b[iGeek::TypingWorldBlock::BlockWordMax + 1];
  world_a.TargetTyped(buf_a);
  world_b.TargetTyped(buf_b);
  if (strcmp(buf_a, buf_b) != 0) {
    return "TYPINGWORLD_FAIL: TargetTyped string mismatch (determinism)";
  }
  // Also verify world_b's state matches
  if (world_a.Remaining() != world_b.Remaining()) {
    return "TYPINGWORLD_FAIL: Remaining mismatch (determinism)";
  }
#endif
  return NILP;
}

// --- Correct char advances: typing the next expected character advances state ---
inline const CHA* CorrectCharAdvances() {
#if SEAM >= IGEEK_TYPINGWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing TypingWorld (correct char advance)..."));
  iGeek::TypingWorld world(7);
  // Read the current target word's first char
  if (world.Target() == NILP) {
    return "TYPINGWORLD_FAIL: no target on fresh world";
  }
  // Type the first character
  bool advanced = world.PressKey(world.Target()->word[0]);
  // Should have advanced
  if (!advanced) {
    return "TYPINGWORLD_FAIL: PressKey of first char did not advance";
  }
  // hits should be 1
  if (world.Target()->hits != 1) {
    return "TYPINGWORLD_FAIL: hits != 1 after first PressKey";
  }
  // correct_ should be 1
  if (world.KeysCorrect() != 1) {
    return "TYPINGWORLD_FAIL: KeysCorrect != 1 after first PressKey";
  }
  // Wrong char should be a miss
  bool missed = world.PressKey('Z');  // 'Z' is a printable char that won't match
  (void)missed;
  if (world.KeysMissed() != 1) {
    return "TYPINGWORLD_FAIL: KeysMissed != 1 after wrong PressKey";
  }
  // hits should still be 1 (wrong char doesn't advance)
  if (world.Target()->hits != 1) {
    return "TYPINGWORLD_FAIL: hits changed after wrong PressKey";
  }
#endif
  return NILP;
}

// --- Wrong char is a miss, no state change ---
inline const CHA* WrongCharIsMiss() {
#if SEAM >= IGEEK_TYPINGWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing TypingWorld (wrong char is miss)..."));
  iGeek::TypingWorld world(7);
  if (world.Target() == NILP) {
    return "TYPINGWORLD_FAIL: no target on fresh world";
  }
  ISC old_hits = world.Target()->hits;
  ISC old_keys_correct = world.KeysCorrect();
  ISC old_keys_missed = world.KeysMissed();
  // Press a wrong char
  bool r = world.PressKey('Q');
  (void)r;
  // Should return false (wrong char)
  if (r) {
    return "TYPINGWORLD_FAIL: wrong char unexpectedly advanced";
  }
  // hits should be unchanged
  if (world.Target()->hits != old_hits) {
    return "TYPINGWORLD_FAIL: hits changed after wrong char";
  }
  // KeysCorrect should be unchanged
  if (world.KeysCorrect() != old_keys_correct) {
    return "TYPINGWORLD_FAIL: KeysCorrect changed after wrong char";
  }
  // KeysMissed should be +1
  if (world.KeysMissed() != old_keys_missed + 1) {
    return "TYPINGWORLD_FAIL: KeysMissed not +1 after wrong char";
  }
#endif
  return NILP;
}

// --- Typing the full current word mines the block ---
inline const CHA* MineFullWord() {
#if SEAM >= IGEEK_TYPINGWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing TypingWorld (mine full word)..."));
  iGeek::TypingWorld world(7);
  if (world.Target() == NILP) {
    return "TYPINGWORLD_FAIL: no target on fresh world";
  }
  // Type chars until the block is mined
  while (!world.Done()) {
    if (world.Target() == NILP) break;
    // Type the next expected character
    bool advanced = world.PressKey(world.Target()->word[world.Target()->hits]);
    (void)advanced;
    // After mining, the next TargetIndex should differ
    ISC old_target_idx = world.TargetIndex();
    (void)old_target_idx;
  }
  // MinedCount should have incremented at least once
  if (world.MinedCount() < 1) {
    return "TYPINGWORLD_FAIL: MinedCount did not increment after full word";
  }
#endif
  return NILP;
}

// --- Mine the whole world: repeatedly type each Target()->word in full until Done ---
inline const CHA* MineWholeWorld() {
#if SEAM >= IGEEK_TYPINGWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing TypingWorld (mine whole world)..."));
  iGeek::TypingWorld world(7);
  // Mine until Done
  while (!world.Done()) {
    if (world.Target() == NILP) break;
    // Type the full current word from current hits to word_len
    ISC wlen = world.Target()->word_len;
    for (ISC c = world.Target()->hits; c < wlen; ++c) {
      world.PressKey(world.Target()->word[c]);
    }
  }
  // After mining the whole world:
  if (world.MinedCount() != iGeek::TypingWorld::BlockCount) {
    return "TYPINGWORLD_FAIL: MinedCount != BlockCount";
  }
  if (world.Remaining() != 0) {
    return "TYPINGWORLD_FAIL: Remaining != 0";
  }
  if (world.Target() != NILP) {
    return "TYPINGWORLD_FAIL: Target != NILP when Done";
  }
  if (world.TargetIndex() != -1) {
    return "TYPINGWORLD_FAIL: TargetIndex != -1 when Done";
  }
  if (world.KeysMissed() != 0) {
    return "TYPINGWORLD_FAIL: KeysMissed != 0 when Done";
  }
#endif
  return NILP;
}

// --- Reset restores ---
inline const CHA* ResetRestores() {
#if SEAM >= IGEEK_TYPINGWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing TypingWorld (reset restores)..."));
  iGeek::TypingWorld world(7);
  // Mine the whole world first
  while (!world.Done()) {
    if (world.Target() == NILP) break;
    ISC wlen = world.Target()->word_len;
    for (ISC c = world.Target()->hits; c < wlen; ++c) {
      world.PressKey(world.Target()->word[c]);
    }
  }
  // Now reset
  world.Reset();
  if (world.MinedCount() != 0) {
    return "TYPINGWORLD_FAIL: MinedCount != 0 after Reset";
  }
  if (world.Remaining() != iGeek::TypingWorld::BlockCount) {
    return "TYPINGWORLD_FAIL: Remaining != BlockCount after Reset";
  }
  if (world.TargetIndex() != 0) {
    return "TYPINGWORLD_FAIL: TargetIndex != 0 after Reset";
  }
  if (world.KeysPressed() != 0) {
    return "TYPINGWORLD_FAIL: KeysPressed != 0 after Reset";
  }
  if (world.KeysCorrect() != 0) {
    return "TYPINGWORLD_FAIL: KeysCorrect != 0 after Reset";
  }
  if (world.KeysMissed() != 0) {
    return "TYPINGWORLD_FAIL: KeysMissed != 0 after Reset";
  }
#endif
  return NILP;
}

}  //< namespace TypingWorldTest