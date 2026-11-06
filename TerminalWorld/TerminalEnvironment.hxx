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
                             ISZ env_count, ISZ obs_len, ISZ action_count,
                             FPC*& observations, ISC*& actions,
                             FPC*& rewards, FPC*& terminals, FPC*& action_mask) {
  ISY obs_index  = TListAlloc<LST_P>(floor_list,
    VTSCKBits | CATypePOD<FPC>(), ISZ(env_count) * obs_len * ISZ(sizeof(FPC)));
  ISY act_index  = TListAlloc<LST_P>(floor_list,
    VTSCKBits | CATypePOD<ISC>(), ISZ(env_count) * ISZ(sizeof(ISC)));
  ISY rwd_index  = TListAlloc<LST_P>(floor_list,
    VTSCKBits | CATypePOD<FPC>(), ISZ(env_count) * ISZ(sizeof(FPC)));
  ISY term_index = TListAlloc<LST_P>(floor_list,
    VTSCKBits | CATypePOD<FPC>(), ISZ(env_count) * ISZ(sizeof(FPC)));
  ISY mask_index = TListAlloc<LST_P>(floor_list,
    VTSCKBits | CATypePOD<FPC>(), ISZ(env_count) * action_count * ISZ(sizeof(FPC)));
  if (obs_index < 0 || act_index < 0 || rwd_index < 0 || term_index < 0 || mask_index < 0)
    return;
  observations  = TListValue<LST_P, FPC>(floor_list, obs_index);
  actions       = TListValue<LST_P, ISC>(floor_list, act_index);
  rewards       = TListValue<LST_P, FPC>(floor_list, rwd_index);
  terminals     = TListValue<LST_P, FPC>(floor_list, term_index);
  action_mask   = TListValue<LST_P, FPC>(floor_list, mask_index);
}

inline TerminalEnvironment::TerminalEnvironment()
    : iGeek::Gym("terminal_world"), room_("terminal_world_room") {
  // Batch shape: one terminal session, obs features, action_count actions
  // — the five batch arrays the PPO loop indexes [env][feature].
  env_count_ = TermWorldEnvironmentCount;
  obs_len_ = TermWorldObsCount;
  action_count_ = TermWorldActionCount;

  // Carve the five flat batch arrays out of the Room's Floor (the inline
  // TDic's values List). No heap: the Floor is static (2048 bytes,
  // embedded in the Room); each block is a TArray<T, ISZ> header (8 bytes)
  // followed by the data, allocated via TListAlloc.
  //
  // Budget (1 env slot, 16 obs, 5 actions):
  //   observations_ : 8 + 16*4  = 72 bytes
  //   actions_      : 8 +  1*4  = 12 bytes
  //   rewards_      : 8 +  1*4  = 12 bytes
  //   terminals_    : 8 +  1*4  = 12 bytes
  //   action_mask_  : 8 +  5*4  = 28 bytes
  //   total         = 136 bytes  (well under the Floor's 2048)
  auto* floor_list = &room_.Floor()->values;
  CarveBatchArrays<ISR, ISR, DTB>(floor_list, env_count_, obs_len_, action_count_,
                   observations_, actions_, rewards_, terminals_, action_mask_);

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
  manifest_.obs_count = TermWorldObsCount;
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

inline void TerminalEnvironment::ResetBatch() {
  for (ISC e = ISC(0); e < env_count_; ++e) {
    for (ISC i = ISC(0); i < obs_len_; ++i)
      observations_[e * obs_len_ + i] = FPC(0.0);
    actions_[e] = ISC(0);
    rewards_[e] = FPC(0.0);
    terminals_[e] = FPC(0.0);
    for (ISC a = ISC(0); a < action_count_; ++a)
      action_mask_[e * action_count_ + a] = FPC(1.0);
  }
}

inline void TerminalEnvironment::StepBatch() {
  // Placeholder deterministic dynamics (verified by the seam test):
  // every action earns a small progress reward; the quit action ends the
  // episode with a terminal flag + the worst reward. Replaced by the real
  // TUI screen/tool dynamics in a later seam — the manifest/contract is
  // what this step verifies.
  const ISC quit = action_count_ - ISC(1);
  for (ISC e = ISC(0); e < env_count_; ++e) {
    const ISC a = actions_[e];
    rewards_[e] = (a == quit) ? FPC(-1.0) : FPC(0.1);
    terminals_[e] = (a == quit) ? FPC(1.0) : FPC(0.0);
    for (ISC i = ISC(0); i < obs_len_; ++i)
      observations_[e * obs_len_ + i] =
          FPC(FPC(a) + FPC(i) * FPC(0.01));
  }
}

}  //< namespace TerminalWorld
