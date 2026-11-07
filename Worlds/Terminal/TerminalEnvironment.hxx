// Copyright AStarship <https://astarship.net>.
//
// TerminalEnvironment implementation: the Room (the Environment) + the
// manifest fill + placeholder dynamics. Unguarded .hxx — pulled ONCE by
// the seam rollup (_Seams/TerminalWorld/_Seams.hxx); the seam units rely
// on that.
//
// The five batch arrays are carved out of the Room's Floor (the inline
// TDic's values List) via TListAlloc — no heap new/delete. The Floor is
// static (2048 bytes, embedded in the Room); the arrays are a subset of
// it; everything destructs with the Room.
#include "TerminalEnvironment.h"

namespace TerminalWorld {
using namespace _;
using namespace iGeek;

/* Template helper so LST_P (ISZ, ISY, DT) resolves: the constructor is
   not a template, so the Floor-carving calls live here. */
template<LST_A>
static void CarveBatchArrays(TList<LST_P>* floor_list,
                             ISZ env_count, ISZ observation_len, ISZ action_count,
                             FPC*& observations, ISC*& actions,
                             FPC*& rewards, FPC*& terminals, FPC*& action_mask) {
  ISY observation_index  = TListAlloc<LST_P>(floor_list,
    VTSCKBits | CATypePOD<FPC>(), ISZ(env_count) * observation_len * ISZ(sizeof(FPC)));
  ISY act_index  = TListAlloc<LST_P>(floor_list,
    VTSCKBits | CATypePOD<ISC>(), ISZ(env_count) * ISZ(sizeof(ISC)));
  ISY rwd_index  = TListAlloc<LST_P>(floor_list,
    VTSCKBits | CATypePOD<FPC>(), ISZ(env_count) * ISZ(sizeof(FPC)));
  ISY term_index = TListAlloc<LST_P>(floor_list,
    VTSCKBits | CATypePOD<FPC>(), ISZ(env_count) * ISZ(sizeof(FPC)));
  ISY mask_index = TListAlloc<LST_P>(floor_list,
    VTSCKBits | CATypePOD<FPC>(), ISZ(env_count) * action_count * ISZ(sizeof(FPC)));
  if (observation_index < 0 || act_index < 0 || rwd_index < 0 || term_index < 0 || mask_index < 0)
    return;
  observations  = TListValue<LST_P, FPC>(floor_list, observation_index);
  actions       = TListValue<LST_P, ISC>(floor_list, act_index);
  rewards       = TListValue<LST_P, FPC>(floor_list, rwd_index);
  terminals     = TListValue<LST_P, FPC>(floor_list, term_index);
  action_mask   = TListValue<LST_P, FPC>(floor_list, mask_index);
}

inline TerminalEnvironment::TerminalEnvironment()
    : iGeek::Gym("terminal_world") {
  Room()->SetRoomName("terminal_world_room");
  // Batch shape: one terminal session, observation features, action_count actions
  // — the five batch arrays the PPO loop indexes [env][feature].
  env_count_ = TermWorldEnvironmentCount;
  observation_len_ = TermWorldObservationCount;
  action_count_ = TermWorldActionCount;

  // Carve the five flat batch arrays out of the Room's Floor (the inline
  // TDic's values List). No heap: the Floor is static (2048 bytes,
  // embedded in the Room); each block is a TArray<T, ISZ> header (8 bytes)
  // followed by the data, allocated via TListAlloc.
  //
  // Budget (1 env slot, 16 observation, 5 actions):
  //   observations_ : 8 + 16*4  = 72 bytes
  //   actions_      : 8 +  1*4  = 12 bytes
  //   rewards_      : 8 +  1*4  = 12 bytes
  //   terminals_    : 8 +  1*4  = 12 bytes
  //   action_mask_  : 8 +  5*4  = 28 bytes
  //   total         = 136 bytes  (well under the Floor's 2048)
  auto* floor_list = &room_.Floor()->values;
  CarveBatchArrays<LST_D>(floor_list, env_count_, observation_len_, action_count_,
                   observations_, actions_, rewards_, terminals_, action_mask_);

  /* Bring the terminal kernel up over its boofer: this is the Crabs device the
     agent drives. One command char per step executes one Star reducer. */
  crabs_ = TermKernelInit(term_boofer_, ISC(sizeof(term_boofer_)), &kernel_);
  TermStateInit(&kernel_.state_);

  /* The Room IS the environment; give it the name the door/manifest refer to.
     Must fit _CrabsRoomNameLengthMax (16 incl. NUL), or it is truncated. */
  room_.SetRoomName("terminal_room");

  // The POD manifest (the in-Room contract). String fields are written
  // with the built-in TSPrint (Stringf.hpp) — no CopyStr wrapper.
  iGeek::EnvironmentManifestInit(&manifest_);
  ::_::TSPrint(manifest_.name, ISW(iGeek::ManifestNameLen), "terminal_world");
  ::_::TSPrint(manifest_.version, ISW(iGeek::ManifestVersionLen), "0.1.0");
  ::_::TSPrint(manifest_.description, ISW(iGeek::ManifestDescriptionLen),
               "Terminal World TUI agent environment");
  ::_::TSPrint(manifest_.modality, ISW(iGeek::ManifestModalityLen),
               "terminal_text");
  ::_::TSPrint(manifest_.world_name, ISW(iGeek::ManifestWorldLen),
               "TerminalWorld");
  manifest_.kind = iGeek::ManifestKindEnv;
  manifest_.domain = iGeek::ManifestDomainGame;
  manifest_.determinism = iGeek::ManifestDeterminismSeeded;
  manifest_.reward_low = FPC(-1.0);
  manifest_.reward_high = FPC(1.0);
  manifest_.reward_higher_is_better = true;
  manifest_.observation_count = TermWorldObservationCount;
  manifest_.action_count = TermWorldActionCount;
  manifest_.max_episode_steps = TermWorldMaxSteps;
  manifest_.environment_count_max = iGeek::GymEnvMax;
#ifdef _iGeekTerminalWorldCore
  manifest_.seam_number = _iGeekTerminalWorldCore;
#endif
  // The JSON (dictionary) form of the manifest is NOT built here. It is
  // bridged from this POD into a TDic that the Chinese Room places on
  // wall 0 (the manifest/config wall) using the Wall's dynamic memory —
  // see the seam test 01.Core.hxx (Room + wall-0 manifest dic).
  ResetBatch();
}

inline TerminalEnvironment::~TerminalEnvironment() {
  // No delete: the batch arrays are carved out of the Room's Floor (the
  // inline TDic's static 2048-byte memory). The Room's destructor
  // (TRoom::~TRoom) frees nothing — the Floor is static and embedded in
  // the Room's origin_ socket. Everything destructs with the Room.
}

/* One real step per env slot: the action IS the command. The kernel's Star
   reducer executes it against the terminal state (pwd/ls/echo/cd/clear/exit)
   and the resulting screen is the observation. Quitting ends the episode. */
inline void TerminalEnvironment::StepBatch() {
  const CHC cmds[TermWorldActionCount] = {CmdCharPwd, CmdCharLs, CmdCharEcho,
                                         CmdCharCd, CmdCharClear, CmdCharExit};
  for (ISC e = ISC(0); e < env_count_; ++e) {
    const ISC a = actions_[e];
    FPC reward = FPC(-0.01f);
    BOL terminal = false;
    if (a >= ISC(0) && a < ISC(TermWorldActionCount) && crabs_ != _NilP) {
      CHA cmd[2];
      cmd[0] = CHA(cmds[a]);
      cmd[1] = 0;
      const ISC n = TermStep(crabs_, cmd, out_, ISC(TermOutputMax));
      if (n > ISC(0)) {
        reward = FPC(0.2f);
        ++executed_;
      }
    }
    /* `exit` clears the running flag in the kernel; that is the episode end. */
    if (!kernel_.state_.running) {
      terminal = true;
      reward = executed_ > ISC(0) ? FPC(1.0f) : FPC(-1.0f);
    }
    ++steps_;
    rewards_[e] = reward;
    terminals_[e] = terminal ? FPC(1.0f) : FPC(0.0f);

    /* The observation is the terminal screen: cursor, line, cwd and how much
       of the screen carries text. Index 0..7 is the cursor/line block, 8..15
       is the screen/session block. */
    FPC* o = &observations_[e * observation_len_];
    ISC text_cells = ISC(0);
    for (ISC i = ISC(0); i < ISC(TermScreenBytes); ++i)
      if (kernel_.state_.screen[i] != 0 && kernel_.state_.screen[i] != ' ')
        ++text_cells;
    ISC cwd_len = ISC(0);
    while (cwd_len < ISC(TermCwdMax) && kernel_.state_.cwd[cwd_len] != 0) ++cwd_len;
    for (ISC i = ISC(0); i < observation_len_; ++i) o[i] = FPC(0.0f);
    o[0] = FPC(kernel_.state_.cursor_row) / FPC(TermScreenRows);
    o[1] = FPC(kernel_.state_.cursor_col) / FPC(TermScreenCols);
    o[2] = FPC(kernel_.state_.line_len) / FPC(TermLineMax);
    o[3] = kernel_.state_.running ? FPC(1.0f) : FPC(0.0f);
    o[4] = FPC(cwd_len) / FPC(TermCwdMax);
    o[5] = FPC(kernel_.state_.history_count) / FPC(TermHistoryMax);
    o[6] = FPC(text_cells) / FPC(TermScreenBytes);
    o[7] = FPC(executed_) / FPC(TermWorldMaxSteps);
    o[8] = terminal ? FPC(1.0f) : FPC(0.0f);
    o[9] = FPC(steps_) / FPC(TermWorldMaxSteps);
    for (ISC i = ISC(10); i < observation_len_; ++i) o[i] = FPC(0.0f);
    for (ISC i = ISC(0); i < action_count_; ++i)
      action_mask_[e * action_count_ + i] = FPC(1.0f);
  }
}

inline void TerminalEnvironment::ResetBatch() {
  TermStateInit(&kernel_.state_);
  executed_ = 0;
  steps_ = 0;
  for (ISC e = ISC(0); e < env_count_; ++e) {
    rewards_[e] = FPC(0.0f);
    terminals_[e] = FPC(0.0f);
    for (ISC i = ISC(0); i < observation_len_; ++i)
      observations_[e * observation_len_ + i] = FPC(0.0f);
    for (ISC i = ISC(0); i < action_count_; ++i)
      action_mask_[e * action_count_ + i] = FPC(1.0f);
  }
}

}  //< namespace TerminalWorld
