// Copyright AStarship <https://astarship.net>.
// Temporary notes:
// The Room + the manifest dic live HERE (the seam test), not in the Gym
// and not in the autogen _Seams.cpp (Hatch regenerates that from
// _Seams.yaml). The Chinese Room is created programmatically; wall 0 is
// the standardized manifest/config wall. The manifest dic is placed in
// wall 0's dynamic memory (the Wall is the dynamic-memory mechanism — a
// gym env's size is not known at compile time).
#if _Seam >= _iGeekTerminalWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#include "../../Worlds/Terminal/Terminal.hxx"
#include "../../../ASCIICrabs/Room.hpp"
#include "../../../ASCIICrabs/Dic.hpp"
namespace TerminalWorldTest {
using namespace ::_;
using namespace iGeek;
using namespace TerminalWorld;

/* The manifest dic's initial stack-buffer size (the ADic's BytesInit_
template param). Sized for the 17-field manifest schema; the ADic
grows via Autoject copy-up if the schema overflows this. */
enum { ManifestDicBytesInit = 1024 };

/* String equality uses the built-in TSEquals (Stringf.hpp): returns a
pointer to the stop of the match or _NilP upon mismatch. */

inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekTerminalWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing TerminalWorld (world env + JSON manifest)..."));

  TerminalEnvironment world;

  // --- 1. The POD manifest (the in-Room contract). --------------------
  EnvironmentManifest& m = world.Manifest();
  _DCheck(EnvironmentManifestValidate(&m), "POD manifest validates");
  _DCheck(TSEquals(m.name, "terminal_world") != _NilP, "name == terminal_world");
  _DCheck(TSEquals(m.version, "0.1.0") != _NilP, "version == 0.1.0");
  _DCheck(TSEquals(m.modality, "terminal_text") != _NilP, "modality == terminal_text");
  _DCheck(TSEquals(m.world_name, "TerminalWorld") != _NilP, "world_name == TerminalWorld");
  _DCheck(m.kind == ManifestKindEnv, "kind == Env");
  _DCheck(m.domain == ManifestDomainGame, "domain == Game");
  _DCheck(m.determinism == ManifestDeterminismSeeded, "determinism == Seeded");
  _DCheck(m.reward_low < m.reward_high, "reward_low < reward_high");
  _DCheck(m.reward_higher_is_better, "reward higher is better");
  _DCheck(m.observation_count == TermWorldObservationCount, "observation_count == 16");
  _DCheck(m.action_count == TermWorldActionCount, "action_count == 6");
  _DCheck(m.max_episode_steps == TermWorldMaxSteps, "max_episode_steps == 100");
  _DCheck(m.environment_count_max == GymEnvMax, "environment_count_max == GymEnvMax");
#ifdef _iGeekTerminalWorldCore
  _DCheck(m.seam_number == _iGeekTerminalWorldCore,
         "seam_number == _iGeekTerminalWorldCore");
#endif

  // --- 2. The Room + the JSON manifest on wall 0 (the standardized
  //     manifest/config wall). The Room IS the Environment (AStarCale) —
  //     it is owned by the Gym (embedded by value, see
  //     TerminalEnvironment::room_), NOT created here. The manifest dic is
  //     placed on wall 0 (the dynamic-memory wall) using the Wall's
  //     dynamic memory, because a gym env's size is not known at compile
  //     time. The manifest is a fixed 17-field schema (a well-defined,
  //     SQL-like row of named columns) — the POD EnvironmentManifest is the packed
  //     contiguous representation, and the dic is the named view over it.
  //     The dic's memory is managed by the ADic (stack-backed by default;
  //     grows via Autoject copy-up to a bigger block when the schema
  //     overflows the initial buffer) — no raw new here. ----
  TRoom<ROM_D>* room = world.Room();
  _DCheck(room != _NilP, "Gym owns a Room (the Environment)");
  _DCheck(TSEquals(room->GetRoomName(), "terminal_room") != _NilP,
         "Room name == terminal_room");

  // The batch arrays are carved out of the Room's Floor (the inline
  // TDic's values List). Verify the Floor budget: the five TArray blocks
  // (136 bytes total for 1 env slot) must fit in the Floor's 2048 bytes
  // alongside the keys Table (512 bytes) and the manifest dic's own
  // tuples if it is ever placed on the Floor (it is not — it goes on a
  // wall).
  {
    auto* floor_list = &room->Floor()->values;
    _DCheck(floor_list->map.count >= 5,
           "Floor has the 5 batch-array blocks (plus keys)");
  }

  // The manifest dic: an ADic with the DIC_D type params (same list as
  // ManifestDic) and a 1024-byte initial stack buffer (sized for the
  // 17-field manifest). Stack-backed (no new); grows via Autoject copy-up
  // if the schema overflows. This() returns the ManifestDic* the triples
  // land in.
  ::ADic<DIC_D, ManifestDicBytesInit> manifest_adic;
  ManifestDic* manifest_dic = manifest_adic.This();
  _DCheck(!IsError(manifest_dic), "manifest dic (ADic) allocated");

  // Bridge the POD manifest into the dic (the 17 canonical
  // type-key-value triples via TDicInsert). String values point into the
  // POD — the dic must not outlive `world`.
  _DCheck(EnvironmentManifestToDic(&world.Manifest(), manifest_dic),
         "POD -> manifest dic bridge ok");

  ManifestDic* d = manifest_dic;
  _DCheck(d != _NilP, "manifest dic non-null");
  {
    CHA buf[ManifestDescriptionLen];
    ISC n = ISC(-1);
    FPC f = FPC(-1.0);
    _DCheck(ManifestGetString(d, "name", buf, ManifestNameLen) &&
             TSEquals(buf, "terminal_world") != _NilP,
           "JSON name == terminal_world");
    _DCheck(ManifestGetInt(d, "observation_count", &n) && n == TermWorldObservationCount,
           "JSON observation_count == 16");
    _DCheck(ManifestGetInt(d, "action_count", &n) && n == TermWorldActionCount,
           "JSON action_count == 6");
    _DCheck(ManifestGetFloat(d, "reward_low", &f) && f == FPC(-1.0),
           "JSON reward_low == -1.0");
    _DCheck(ManifestGetInt(d, "manifest_version", &n) && n == ISC(1),
           "JSON manifest_version == 1");
  }

  // --- 3. Round-trip: JSON Dic back to a fresh POD. --------------------
  {
    EnvironmentManifest rt;
    _DCheck(ManifestDicToEnvironmentManifest(d, &rt), "Dic -> POD round-trip ok");
    _DCheck(EnvironmentManifestValidate(&rt), "round-tripped POD validates");
    _DCheck(TSEquals(rt.name, m.name) != _NilP, "rt.name matches");
    _DCheck(TSEquals(rt.version, m.version) != _NilP, "rt.version matches");
    _DCheck(TSEquals(rt.description, m.description) != _NilP, "rt.description matches");
    _DCheck(TSEquals(rt.modality, m.modality) != _NilP, "rt.modality matches");
    _DCheck(TSEquals(rt.world_name, m.world_name) != _NilP, "rt.world_name matches");
    _DCheck(rt.kind == m.kind && rt.domain == m.domain &&
             rt.determinism == m.determinism && rt.observation_count == m.observation_count &&
             rt.action_count == m.action_count &&
             rt.max_episode_steps == m.max_episode_steps &&
             rt.environment_count_max == m.environment_count_max &&
             rt.seam_number == m.seam_number &&
             rt.manifest_version == m.manifest_version,
           "rt int fields match");
    _DCheck(rt.reward_low == m.reward_low &&
             rt.reward_high == m.reward_high &&
             rt.reward_higher_is_better == m.reward_higher_is_better,
           "rt float/bool fields match");
  }

  // --- 4. The JSON dump (human-inspectable). ---------------------------
  {
    ::_::RefOut() << "\n  JSON manifest: ";
    _DCheck(EnvironmentManifestPrintJson(d, ::_::RefOut()), "EnvironmentManifestPrintJson ok");
    ::_::RefOut() << '\n';
  }

  // --- 5. Deliberate failures (the negative contract). -----------------
  {
    EnvironmentManifest bad;
    EnvironmentManifestInit(&bad);
    _DCheck(!EnvironmentManifestValidate(&bad), "zeroed POD fails validation");
    ISC n = ISC(0);
    FPC f = FPC(0.0);
    CHA buf[ManifestNameLen];
    _DCheck(!ManifestGetInt(d, "no_such_key", &n),
           "missing key fails ManifestGetInt");
    _DCheck(!ManifestGetString(d, "observation_count", buf, ManifestNameLen),
           "type mismatch (INT field) fails ManifestGetString");
    _DCheck(!ManifestGetFloat(d, "name", &f),
           "type mismatch (STR field) fails ManifestGetFloat");
  }

  // --- 6. The Gym batch interface (the PPO contract). ------------------
  {
    _DCheck(TSEquals(world.Name(), "terminal_world") != _NilP, "Gym name");
    _DCheck(world.EnvironmentCount() == TermWorldEnvironmentCount, "EnvironmentCount == 1");
    _DCheck(world.Observations() != _NilP && world.Actions() != _NilP &&
             world.Rewards() != _NilP && world.Terminals() != _NilP &&
             world.ActionMask() != _NilP,
           "all five batch buffers allocated");
    world.Actions()[0] = TermWorldActionCount - 1;  // quit
    world.StepBatch();
    _DCheck(world.Terminals()[0] == FPC(1.0), "quit action terminates");
    _DCheck(world.Rewards()[0] == FPC(-1.0), "quit action gets worst reward");
    world.Actions()[0] = ISC(0);                  // type
    world.StepBatch();
    _DCheck(world.Terminals()[0] == FPC(0.0), "non-quit action does not terminate");
    _DCheck(world.Rewards()[0] == FPC(0.1), "progress reward on non-quit");
    world.ResetBatch();
    _DCheck(world.Terminals()[0] == FPC(0.0) && world.Rewards()[0] == FPC(0.0),
           "reset clears terminals/rewards");
  }

  // The manifest ADic is stack-backed and test-local; it is destroyed
  // automatically when the seam unit ends (no manual free).

  // --- the kernel is wired: an action really executes a shell command ---
  {
    TerminalWorld::TerminalEnvironment env;
    env.ResetBatch();
    env.Actions()[0] = ISC(0);          //< CmdCharPwd
    env.StepBatch();
    _DCheck(env.Rewards()[0] > FPC(0.0f), "pwd produced output");
    _DCheck(env.Observations()[0] != FPC(0.0f), "observation reflects the screen");
    _DCheck(env.Terminals()[0] == FPC(0.0f), "pwd does not end the episode");
    env.Actions()[0] = ISC(5);          //< CmdCharExit
    env.StepBatch();
    _DCheck(env.Terminals()[0] == FPC(1.0f), "exit ends the episode");
  }

  return _NilP;
#else
  return _NilP;
#endif
}

}  //< namespace TerminalWorldTest
