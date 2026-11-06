// Copyright AStarship <https://astarship.net>.
//
// TerminalWorld — the agent-comparison terminal environment.
//
// A mini-Linux terminal where EACH Bash command is a Star reducer function
// (ASCIICrabs Crabs Op pattern, _Seams/16.CrabsOp.hxx). The Crab Stack
// (kernel) is the Crabs device's BIn/BOut ring + operand stack; a reducer
// reads its command from the BIn ring, mutates the terminal state, and
// writes the screen output to the BOut ring. The prompt->output loop is
// the agent-comparison surface: agent types a command (BIn), the kernel
// applies the command's Star reducer, the new screen state is read back
// (BOut) as the observation.
//
// This is NOT the Chinese Room IPC/transport code. The Room is just the
// state container; the reducers are the terminal's semantics. The
// TerminalEnvironment wraps a TerminalKernel (a Crabs Operand) as an
// iGeek::Gym so the PPO loop can compare agents headlessly.
//
// No C/C++ standard library: I/O via the Crabs BIn/BOut ring.
//
#pragma once
#ifndef _iGeekTerminalWorldTerminalH
#define _iGeekTerminalWorldTerminalH
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Binary.hpp"
#include "../ASCIICrabs/BIn.h"
#include "../ASCIICrabs/Crabs.h"
#include "../ASCIICrabs/Operand.h"
#include "../Gym.h"
#include "../EnvironmentManifest.h"

namespace TerminalWorld {
using namespace ::_;

/* Screen / session limits (fixed-capacity, no VLA, no heap growth). */
enum {
  TermScreenRows   = 24,    // 24x80 terminal
  TermScreenCols   = 80,
  TermScreenBytes  = TermScreenRows * TermScreenCols,
  TermLineMax      = 128,   // max command line length
  TermOutputMax    = 1024,  // max output bytes per command
  TermCwdMax       = 64,
  TermHistoryMax   = 16,    // command history depth
  TermStateMax     = 8,     // kernel operand-stack depth
};

/* The terminal state — the object the reducers push on the Crab Stack
   (the kernel operand stack). POD: screen, cursor, cwd, command line,
   history, running flag. */
struct TTerminalState {
  CHA  screen[TermScreenBytes];   //< Visible screen (row-major).
  ISC  cursor_row;                //< Cursor row (0..TermScreenRows-1).
  ISC  cursor_col;                //< Cursor col (0..TermScreenCols-1).
  CHA  cwd[TermCwdMax];           //< Current working directory.
  CHA  line[TermLineMax];         //< Command line being typed.
  ISC  line_len;                  //< Bytes in line.
  ISC  history[TermHistoryMax];   //< Ring of past command lengths.
  ISC  history_count;             //< Number of history entries.
  BOL  running;                   //< false when the session exits.
};

/* A Star reducer: reads the command from the Crabs BIn ring, mutates the
   terminal state (pushed on the operand stack), writes output to the BOut
   ring. Returns _NilP on success, an error Op on Read-Write failure. The
   index char selects the command (the manual dispatch — no Table, no
   hash, just the Op's First/Last index range). */
typedef const ::Op* (*TStarReducer)(::CHC index, ::Crabs* crabs,
                                    TTerminalState* state);

/* The command vocabulary: one ASCII index char per Bash command. The
   reducer table is indexed by these chars. Adding a command = add a char
   + a reducer fn + a slot in the table. */
enum {
  CmdCharPwd   = 'p',   // pwd  -> print cwd
  CmdCharLs    = 'l',   // ls   -> list / (stub: print cwd contents)
  CmdCharEcho  = 'e',   // echo <text> -> print text
  CmdCharCd    = 'c',   // cd <dir>  -> change cwd
  CmdCharClear = 'k',   // clear -> wipe screen
  CmdCharExit  = 'x',   // exit  -> terminate the session
};

/* The TerminalKernel — a Crabs Operand whose Star() dispatches each
   command char to its Star reducer over the terminal state. The Crab
   Stack (kernel) is the Crabs BIn/BOut ring + operand stack. */
class TerminalKernel : public ::Operand {
 public:
  TerminalKernel();
  ~TerminalKernel();

  /* The Crabs Op interface: the Star reducer dispatcher. index is the
     ASCII command char; crabs gives the BIn/BOut ring; the terminal state
     is the top of the operand stack. Returns _NilP on success. */
  const ::Op* Star(::CHC index, ::Crabs* crabs) override;

  /* The initial terminal state (the prompt). */
  TTerminalState state_;

  /* The Star reducer table: 256 slots, one per ASCII char. Empty slots
     are _NilP (unhandled command). Manual dispatch — no Table, no hash. */
  TStarReducer reducer_[128];
};

/* Build the Crabs device (the kernel) over a preallocated boofer. Returns
   _NilP on allocation failure. The boofer must be at least
   TermKernelBooferBytes; it holds the BIn/BOut ring + operand stack. */
::Crabs* TermKernelInit(IUW* boofer, ISC boofer_bytes,
                        TerminalKernel* kernel);

/* The prompt->output loop: write `command` to the BIn ring, scan it
   (which dispatches to the command's Star reducer), and read the output
   from the BOut ring into `out` (<= out_len bytes). Returns the number of
   output bytes, or -1 on error. This is one terminal step. */
ISC TermStep(::Crabs* crabs, const CHA* command, CHA* out, ISC out_len);

/* Initialize the terminal state to the login prompt. */
void TermStateInit(TTerminalState* state);

}  //< namespace TerminalWorld
#endif  //< _iGeekTerminalWorldTerminalH
