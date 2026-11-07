// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekEnvironmentManifestH
#define _iGeekEnvironmentManifestH
#include <_Config.h>
#if _Seam >= _iGeekEnvironmentRolloutCompare
#include "ROut.h"
#include "AType.hpp"
#include "List.hpp"
#include "Dic.hpp"
namespace iGeek {
using namespace ::_;

/* The manifest as a dictionary: CHS keys, value types in the DTB byte,
   hash in IUN. Same shape the JSON accessors use to read/write it.
   Written as TDic<> so it uses DIC_A's default parameter list, which is
   the same list DIC_D supplies. */
typedef _::TDic<> ManifestDic;

/* Fixed-length string capacity for manifest fields. */
enum {
  ManifestNameLen = 32,
  ManifestVersionLen = 16,
  ManifestDescriptionLen = 128,
  ManifestModalityLen = 32,
  ManifestWorldLen = 32,
};

/* Environment kind: what this Gym is. */
enum {
  ManifestKindEnv = 0,
  ManifestKindBenchmark,
  ManifestKindInvalid,
};

/* Domain: what the environment models. */
enum {
  ManifestDomainRobotics = 0,
  ManifestDomainGame,
  ManifestDomainScience,
  ManifestDomainClassical,
  ManifestDomainInvalid,
};

/* Determinism guarantee. */
enum {
  ManifestDeterminismSeeded = 0,
  ManifestDeterminismStochastic,
  ManifestDeterminismUnknown,
};

/* The Gym manifest: a fixed-size POD describing what is in the Room.
   Stored in the Room's Floor (the ASCII List), written by the World at
   init, read by the agent on join. NOT a YAML file — a contiguous block
   of typed data in the Room's memory. */
struct EnvironmentManifest {
  CHA name[ManifestNameLen];
  CHA version[ManifestVersionLen];
  ISC kind;
  ISC domain;
  CHA description[ManifestDescriptionLen];
  CHA modality[ManifestModalityLen];
  FPC reward_low;
  FPC reward_high;
  BOL reward_higher_is_better;
  ISC determinism;
  ISC observation_count;
  ISC action_count;
  ISC max_episode_steps;
  ISC environment_count_max;
  CHA world_name[ManifestWorldLen];
  ISC seam_number;
  ISC manifest_version;
};

/* Total tuples: 12 scalars + 5 strings (each field stored as _CHA chars). */
enum {
  ManifestScalarCount = 12,
  ManifestTupleTotal =
      ManifestScalarCount +
      ManifestNameLen + ManifestVersionLen + ManifestDescriptionLen +
      ManifestModalityLen + ManifestWorldLen,
};

/* ---- Non-template API (defined in EnvironmentManifest.hxx) ----------- */

/* Initializes a manifest to safe defaults (zeroed, kind=Env, domain=Invalid,
   determinism=Unknown, manifest_version=1). */
void EnvironmentManifestInit(EnvironmentManifest* m);

/* Validates a manifest: non-empty name/version, valid kind/domain/
   determinism, reward_low <= reward_high, observation_count > 0,
   action_count > 0, max_episode_steps > 0, environment_count_max > 0.
   @return        True if valid, false otherwise. */
BOL EnvironmentManifestValidate(const EnvironmentManifest* m);

/* ---- ManifestDic (JSON) accessors ---------------------------------- */

/* Typed value getters: return false on missing key or type mismatch. */
BOL ManifestGetString(const ManifestDic* dic, const CHS* key,
                      CHA* out, ISN out_len);
BOL ManifestGetInt(const ManifestDic* dic, const CHS* key, ISC* out);
BOL ManifestGetFloat(const ManifestDic* dic, const CHS* key, FPC* out);

/* Typed value setters: insert or overwrite; return false on bad args. */
BOL ManifestSetString(ManifestDic* dic, const CHS* key, const CHA* value);
BOL ManifestSetInt(ManifestDic* dic, const CHS* key, ISC value);
BOL ManifestSetFloat(ManifestDic* dic, const CHS* key, FPC value);

/* Serializes a ManifestDic to a JSON object on the ROut stream. */
BOL EnvironmentManifestPrintJson(const ManifestDic* dic, ::_::ROut& out);

/* POD <-> Dic bridge: all 17 fields, canonical key names, field order. */
BOL EnvironmentManifestToDic(const EnvironmentManifest* m, ManifestDic* dic);
BOL ManifestDicToEnvironmentManifest(const ManifestDic* dic, EnvironmentManifest* m);

/* ---- Template API (defined in EnvironmentManifest.hpp) --------------- */

/* Serializes the manifest into the Floor as ASCII tuples.
   @param m       The manifest to write.
   @param floor   The Room's Floor (ASCII List) to write into.
   @return        True on success, false if the Floor is full. */
template<LST_A>
BOL EnvironmentManifestDump(const EnvironmentManifest* m, TList<LST_P>* floor);

/* Deserializes a manifest from the Floor.
   @param floor   The Room's Floor to read from.
   @param m       The manifest to fill.
   @return        True on success, false if the Floor is empty or corrupt. */
template<LST_A>
BOL EnvironmentManifestLoad(TList<LST_P>* floor, EnvironmentManifest* m);

}  //< namespace iGeek
#endif
#endif
