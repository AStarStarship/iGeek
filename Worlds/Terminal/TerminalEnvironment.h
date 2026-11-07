// Copyright AStarship <https://astarship.net>.
//
// TerminalWorld environment — the iGeek Gym for the Terminal World TUI
// agent framework (our Hermes replacement, built on CrabsTK + ASCIICrabs).
//
// The Environment IS the Room (AStarCale). The Room is the Chinese Room
// (ASCIICrabs/Room.hpp): a state machine with a static Floor (an inline
// TDic, 2048 bytes) and optional Walls (heap). This world embeds one Room
// by value in the Gym; the Room owns the world's memory. The five batch
// arrays (observations/actions/rewards/terminals/action_mask) are
// TArray blocks written into the Room's Floor via TListAlloc — no heap
// new/delete at all (the Floor is static, the arrays are a subset of it,
// and everything destructs with the Room).
//
// The manifest is the fixed POD EnvironmentManifest held by the Gym base (the
// in-Room contract). Its JSON (dictionary) form is bridged from the POD
// into a TDic on wall 0 in the seam test (_Seams/TerminalWorld/01.Core.hxx)
// using the Wall's dynamic memory, because a gym env's size is not known
// at compile time.
//
// No C/C++ standard library: I/O via ASCIICrabs ROut.

#pragma once
#ifndef _iGeekTerminalEnvironmentH
#define _iGeekTerminalEnvironmentH
#include "../../../ASCIICrabs/_Config.h"
#include "../../../ASCIICrabs/Room.hpp"
#include "../../Gym.h"
#include "../../EnvironmentManifest.h"
#include "Terminal.h"
namespace TerminalWorld {
using namespace _;
using namespace CTK;
using namespace iGeek;

/* Terminal World dynamics constants (the TUI action vocabulary + screen
state vector). Placeholder scale for the first seam step; the real
screen/tool dynamics land with the TUI loop (see _Agent/IllyaRequest.md). */
enum {
  TermWorldObservationCount = 16,     // screen state: cursor row/col, scroll, exit...
  TermWorldActionCount      = 6,      // pwd, ls, echo, cd, clear, exit
  TermWorldMaxSteps         = 100,    // episode cap
  TermWorldEnvironmentCount = 1,      // one terminal session per batch env
};

/* The Terminal World environment. One terminal session per env slot; the
PPO loop indexes the five batch arrays [env][feature] as with any Gym.
The Room (the Environment) is embedded by value; the batch arrays are
carved out of the Room's Floor in the constructor and live for the
Room's lifetime. */
class TerminalEnvironment : public iGeek::Gym {
 public:
  TerminalEnvironment();
  ~TerminalEnvironment() override;

  /* Placeholder deterministic step: every non-quit action earns a small
  progress reward; the quit action (action_count-1) ends the episode.
  Verified by the seam test 01.Core.hxx; replaced by the real TUI
  screen/tool dynamics in a later seam. */
  void StepBatch() override;
  void ResetBatch() override;

 protected:
  /* The kernel the agent drives: a Crabs Operand whose Star reducer executes
     one shell command per step. The boofer holds the BIn/BOut ring + operand
     stack; it is a member, not heap (the Room's Floor stays static). */
  TerminalKernel kernel_;
  alignas(64) IUW term_boofer_[2048];
  ::Crabs* crabs_ = _NilP;
  CHA out_[TermOutputMax] = {};   //< screen text after the last command
  ISC executed_ = 0;              //< commands that produced output this episode
  ISC steps_ = 0;

  /* The Room IS the Environment. It is the Chinese Room (the state
  machine that receives messages through Door slots); its Floor is the
  inline TDic (the static 2048-byte memory) and the batch arrays are
  carved out of it. The manifest POD (manifest_) and the Room (room_)
  are held by the Environment base; the JSON form is bridged into a
  TDic on wall 0 in the seam test. See _Seams/TerminalWorld/01.Core.hxx. */
};

}  //< namespace TerminalWorld
#endif  //< _iGeekTerminalEnvironmentH
