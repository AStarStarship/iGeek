// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekTerminalHXX
#define _iGeekTerminalHXX
// TerminalWorld kernel + Star reducers. Each Bash command is a Star
// reducer fn (CHC index, Crabs* crabs, TTerminalState*) -> const Op*.
// The Crab Stack (kernel) is the Crabs BIn/BOut ring; a reducer reads its
// command from BIn, mutates the terminal state, writes output to BOut.
// Manual dispatch: the reducer table is indexed by ASCII char, no Table.

#include "Terminal.h"
#include "../ASCIICrabs/_Package.hxx"
#include "../ASCIICrabs/BIn.hpp"
#include "../ASCIICrabs/BOut.hpp"
#include "../ASCIICrabs/BSeq.hpp"
#include "../ASCIICrabs/Args.h"
#include "../ASCIICrabs/ROut.h"
#include "../ASCIICrabs/Stringf.hpp"

namespace TerminalWorld {
using namespace ::_;
using namespace ::_iGeek;

static void TermScreenClear(TTerminalState* s) {
  for (ISC i = 0; i < TermScreenBytes; ++i) s->screen[i] = 0;
  s->cursor_row = 0;
  s->cursor_col = 0;
}

/* Print a line at the cursor, advancing the cursor. Truncates at the row
edge; scrolls the screen up one row when the cursor passes the bottom. */
static void TermScreenPrint(TTerminalState* s, const CHA* text) {
  while (*text) {
    if (s->cursor_row >= TermScreenRows) {
      // Scroll up one row.
      for (ISC r = 0; r < TermScreenRows - 1; ++r)
        for (ISC c = 0; c < TermScreenCols; ++c)
          s->screen[r * TermScreenCols + c] =
              s->screen[(r + 1) * TermScreenCols + c];
      s->cursor_row = TermScreenRows - 1;
      s->cursor_col = 0;
    }
    if (s->cursor_col >= TermScreenCols) {
      s->cursor_row += 1;
      s->cursor_col = 0;
      continue;
    }
    s->screen[s->cursor_row * TermScreenCols + s->cursor_col] = *text;
    ++s->cursor_col;
    ++text;
  }
  // Newline after a printed line.
  s->cursor_row += 1;
  s->cursor_col = 0;
}

/* The prompt line: "user@host:cwd$" — printed before each command echo. */
static void TermScreenPrompt(TTerminalState* s) {
  CHA line[TermLineMax];
  ISC n = 0;
  const CHA* host = "astar";
  while (*host && n < TermLineMax - 1) line[n++] = *host++;
  if (n < TermLineMax - 1) line[n++] = ':';
  const CHA* cwd = s->cwd;
  while (*cwd && n < TermLineMax - 1) line[n++] = *cwd++;
  if (n < TermLineMax - 1) line[n++] = '$';
  line[n] = 0;
  TermScreenPrint(s, line);
}

void TermStateInit(TTerminalState* s) {
  TermScreenClear(s);
  TSPrint(s->cwd, ISW(TermCwdMax), "/home/astar");
  s->line_len = 0;
  s->history_count = 0;
  s->running = true;
  TermScreenPrint(s, "TerminalWorld mini-Linux (Star-reducer kernel)\n");
  TermScreenPrompt(s);
}

/* --- The Star reducers -------------------------------------------------- */
/* Each reads its command from the Crabs BIn ring, mutates the state,
   writes output to the BOut ring. Returns _NilP on success. The index char
   selects the command (manual dispatch). */

/* pwd: print the cwd. No args. */
static const Op* ReducerPwd(CHC, Crabs* crabs, TTerminalState* s) {
  (void)crabs;
  TermScreenPrint(s, s->cwd);
  return _NilP;
}

/* ls: list the cwd (stub: prints the cwd + a placeholder entry). */
static const Op* ReducerLs(CHC, Crabs* crabs, TTerminalState* s) {
  (void)crabs;
  TermScreenPrint(s, s->cwd);
  TermScreenPrint(s, "  (stub: no file table yet)");
  return _NilP;
}

/* echo <text>: print the text from the BIn ring. Reads a STR_ arg. */
static const Op* ReducerEcho(CHC, Crabs* crabs, TTerminalState* s) {
  TBIn<ISC>* bin = CrabsBIn(crabs);
  const DTB* params = TTSQ<STR_, TermLineMax>();
  CHA text[TermLineMax] = {0};
  void* args[] = {text};
  const Op* err = BInRead(bin, params, args, 0);
  if (err) return err;
  TermScreenPrint(s, text);
  return _NilP;
}

/* cd <dir>: change the cwd. Reads a STR_ arg. */
static const Op* ReducerCd(CHC, Crabs* crabs, TTerminalState* s) {
  TBIn<ISC>* bin = CrabsBIn(crabs);
  const DTB* params = TTSQ<STR_, TermCwdMax>();
  CHA dir[TermCwdMax] = {0};
  void* args[] = {dir};
  const Op* err = BInRead(bin, params, args, 0);
  if (err) return err;
  TSPrint(s->cwd, ISW(TermCwdMax), dir);
  return _NilP;
}

/* clear: wipe the screen + cursor. No args. */
static const Op* ReducerClear(CHC, Crabs* crabs, TTerminalState* s) {
  (void)crabs;
  TermScreenClear(s);
  return _NilP;
}

/* exit: terminate the session. No args. */
static const Op* ReducerExit(CHC, Crabs* crabs, TTerminalState* s) {
  (void)crabs;
  s->running = false;
  TermScreenPrint(s, "exit");
  return _NilP;
}

/* --- The Op header (the signature) ------------------------------------- */
static const Op* TermOpHeader() {
  static const Op op = {
      "terminal_world",
      TBSeq<>().CParams() /*in: no fixed params (per-reducer BSeq)*/,
      TBSeq<>().CParams() /*out*/,
      "mini-Linux terminal (Star-reducer kernel)",
      CmdCharClear,  //< pop: index of the Pop Operation.
      CmdCharExit,   //< close: index of the Close Operation.
      CmdCharPwd,    //< default_op: index of the Default Operation.
      _NilP           //< Evaluated expression Slot.
  };
  return &op;
}

TerminalKernel::TerminalKernel() {
  for (ISC i = 0; i < 128; ++i) reducer_[i] = _NilP;
  reducer_[CmdCharPwd]   = ReducerPwd;
  reducer_[CmdCharLs]    = ReducerLs;
  reducer_[CmdCharEcho]  = ReducerEcho;
  reducer_[CmdCharCd]    = ReducerCd;
  reducer_[CmdCharClear] = ReducerClear;
  reducer_[CmdCharExit]  = ReducerExit;
  TermStateInit(&state_);
}

TerminalKernel::~TerminalKernel() {}

const Op* TerminalKernel::Star(CHC index, Crabs* crabs) {
  if (!crabs) return TermOpHeader();
  if (index < 0 || index >= 128) return _NilP;
  TStarReducer r = reducer_[index];
  if (!r) return _NilP;  //< unhandled command char: no-op.
  return r(index, crabs, &state_);
}

/* --- Crabs device init + the prompt->output step loop ------------------ */

Crabs* TermKernelInit(IUW* boofer, ISC boofer_bytes, TerminalKernel* kernel) {
  if (IsError(boofer) || boofer_bytes < 1024 || !kernel) return _NilP;
  enum { UnpackedBytes = 1024 };
  static IUW unpacked[UnpackedBytes / sizeof(IUW)];
  IUD pc_ctx = 0;
  Crabs* crabs = CrabsInit(boofer, boofer_bytes, TermStateMax, kernel,
                           unpacked, UnpackedBytes, pc_ctx);
  if (crabs) Push(crabs, kernel);  //< Select the kernel as the current operand.
  return crabs;
}

ISC TermStep(Crabs* crabs, const CHA* command, CHA* out, ISC out_len) {
  if (IsError(crabs) || IsError(command) || IsError(out) || out_len < 1)
    return -1;
  TerminalKernel* kernel = TRef<TerminalKernel>(crabs->operand);
  // The prompt->output loop. The agent's typed command is the first char
  // (the command index) + optional args. The BOut ring is the agent's
  // input channel; write the command there (the kernel's I/O substrate),
  // then dispatch to the command's Star reducer directly (manual dispatch
  // on the command char — no Table, no hash). The reducer mutates the
  // terminal state; the observation is the resulting screen.
  // Dispatch: the command char selects the Star reducer (manual dispatch —
  // no Table, no hash, just the char). The reducer mutates the terminal
  // state. (The BOut ring is the agent's typed-input channel; the agent
  // writes command bytes there and the kernel reads command[0] to
  // dispatch. Wiring the ring<->reducer arg flow is the next step.)
  CHC idx = CHC(command[0]);
  if (idx >= 0 && idx < 128 && kernel->reducer_[idx])
    kernel->reducer_[idx](idx, crabs, &kernel->state_);
  // The observation is the screen state (the agent reads the terminal).
  TTerminalState* s = &kernel->state_;
  ISC n = 0;
  for (ISC r = 0; r < TermScreenRows && n < out_len - 1; ++r) {
    for (ISC c = 0; c < TermScreenCols && n < out_len - 1; ++c)
      out[n++] = s->screen[r * TermScreenCols + c];
    out[n++] = '\n';
  }
  out[n] = 0;
  return n;
}

}  //< namespace TerminalWorld
#endif