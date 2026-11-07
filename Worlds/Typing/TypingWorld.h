// Copyright AStarship <https://astarship.net>.
// TypingWorld — a deterministic typing-mine engine (TypingWorld core).
//
// The "advanced" TypingWorld/Minecraft scaffold in this folder is dead weight
// (duplicate enum values, missing files, half-defined methods — it does not
// compile). Per the project decision, the AI layer is dropped and TypingWorld
// becomes a headless typing tutor: you MINE BLOCKS BY TYPING.
//
// The loop (pure, deterministic, no I/O):
//   * The world holds a fixed grid of unmined blocks, each assigned a word.
//   * The player types one character at a time (PressKey).
//   * Each correct character chisels the current block one "hit" deeper.
//   * When the block's full word is typed, it drops: it is mined, the item is
//     added to the inventory, and the next block becomes the target.
//   * A wrong character is a miss: it does NOT advance the block and is
//     counted (accuracy tracking), but it never corrupts state.
//
// No-stdlib: fixed-size arrays, a seeded 64-bit LCG (same seed -> same world),
// no std containers, no exceptions, no heap. Rendering is a separate concern
// (Print via ROut) so the simulator never depends on a terminal.
#pragma once
#ifndef TYPINGWORLD_TYPINGWORLD_DECL
#define TYPINGWORLD_TYPINGWORLD_DECL
#include "Environment.h"
#include <_Config.h>
namespace iGeek {

/* A single mining target in the world. */
struct TypingWorldBlock {
  enum {
    BlockWordMax = 24,  //< Max characters in a block's word.
  };
  /* One unmined target.
     word        the string to type to mine this block.
     word_len    number of characters in word.
     hits        characters typed correctly so far (== word_len when mined).
     mined       true once the block has dropped. */
  CHA word[BlockWordMax];
  ISC word_len;
  ISC hits;
  BOL mined;
};

struct TypingWorld {
  enum {
    GridWidth = 8,    //< Blocks across.
    GridHeight = 6,   //< Blocks down.
    BlockCount = GridWidth * GridHeight,
    //< Total blocks in the world (GridWidth * GridHeight).
    InvMax = 64,      //< Max mined items the inventory holds.
  };

  /* Constructs a fresh world.
     @param seed World-generation seed (0 -> 1). Same seed -> same layout,
            same words, same mining order, so runs are reproducible. */
  TypingWorld(IUD seed = 7);
  ~TypingWorld() {}

  /* Resets the world to a freshly generated episode (same seed -> same
     world). Clears the inventory and all mining progress. */
  void Reset();

  /* Types one character against the current block.
     @param ch The keystroke (any value; non-printable is a miss).
     @return true if it advanced the block (correct char), false on a miss. */
  BOL PressKey(CHA ch);

  /* The index (0..BlockCount-1) of the block currently being typed. */
  ISC TargetIndex();

  /* The block currently being typed (_NilP when all blocks are mined). */
  const TypingWorldBlock* Target();

  /* The current block's word so far (what the player has typed correctly).
     @param out Buffer of at least BlockWordMax+1 (NUL-terminated on write).
     @return The number of correct characters typed so far. */
  ISC TargetTyped(CHA* out);

  /* Copies the current target WORD into out (NUL-terminated).
     @param out Destination buffer, at least TypingWordLenMax chars.
     @return The word length, 0 when there is no target block. */
  ISC TargetWord(CHA* out);

  /* True when every block in the world has been mined. */
  BOL Done();

  /* The number of blocks mined so far. */
  ISC MinedCount();

  /* The number of blocks still unmined. */
  ISC Remaining();

  /* Total keystrokes processed this episode. */
  ISC KeysPressed();

  /* Total correct keystrokes (each one advanced a block by one hit). */
  ISC KeysCorrect();

  /* Total misses (wrong or non-printable keystrokes). */
  ISC KeysMissed();

  /* The index (0..BlockCount-1) of the next unmined block after i, wrapping
     the grid; -1 when none remain. Used by the mining order. */
  ISC NextUnmined(ISC i);

  /* Renders the world to the console (ROut). Grid of blocks: '#' unmined,
     '.' mined, '@' the current target; the typed-so-far string and stats.
     Pure ROut — the engine itself does not require a terminal. */
  void Print();

 private:
  /* (Re)generates the world from seed_: assigns a word to every block and
     clears mining state. Deterministic per seed via the LCG. */
  void Generate(IUD seed);

  /* Picks a printable word for block b from a small fixed vocabulary,
     chosen by the LCG so it is deterministic per seed. */
  void AssignWord(ISC b);

  IUD rng_;
  IUD seed_;
  TypingWorldBlock blocks_[BlockCount];
  ISC target_;   //< Current target block index (-1 when Done()).
  ISC mined_;    //< Number of blocks mined.
  ISC keys_;     //< Total keystrokes.
  ISC correct_;  //< Correct keystrokes.
  ISC missed_;   //< Missed keystrokes.
  CHA inv_[InvMax][TypingWorldBlock::BlockWordMax + 1];
  ISC inv_count_;
};


/* The typing world AS an Environment: a Room/thread an agent can join.
   Observation is the target text plus mining progress; the action is one of
   the 26 letters. The env IS the Environment, so it owns the Room and the
   manifest a joining agent reads. */
class TypingEnv : public Environment {
 public:
  enum {
    ObservationLength = 40,
    ActionCount = 26,                       //< 'a'..'z'
    MaxSteps = TypingWorld::BlockCount * 8,
  };
  TypingEnv(IUD seed = 7);
  void Reset();
  FPC Step(ISC action);
  void Observe();
  const FPC* Observation() { return observation_; }
  BOL Done() { return world_.Done() || steps_ >= MaxSteps; }
  ISC Steps() { return steps_; }
  TypingWorld& World() { return world_; }
 private:
  TypingWorld world_;
  FPC observation_[ObservationLength] = {};
  ISC steps_ = 0;
  IUD seed_;
};

} //< namespace iGeek
#endif
