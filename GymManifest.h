// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_GYM_MANIFEST_H
#define IGEEK_GYM_MANIFEST_H
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "Env.h"
namespace iGeek {

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
struct TManifest {
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
  ISC obs_count;
  ISC action_count;
  ISC max_episode_steps;
  ISC env_count_max;
  CHA world_name[ManifestWorldLen];
  ISC seam_number;
  ISC manifest_version;
};

/* Initializes a manifest to safe defaults (zeroed, kind=Env, domain=Invalid,
   determinism=Unknown, manifest_version=1). */
void TManifestInit(TManifest* m);

/* Copies a C string into a fixed-size CHA buffer (truncates if too long). */
void CopyStr(CHA* dst, ISN dst_len, const CHA* src);

/* Serializes the manifest into the Floor as ASCII tuples.
   @param m       The manifest to write.
   @param floor   The Room's Floor (ASCII List) to write into.
   @return        True on success, false if the Floor is full. */
BOL TManifestDump(const TManifest* m, TList<ISC, ISC, DTB>* floor);

/* Deserializes a manifest from the Floor.
   @param floor   The Room's Floor to read from.
   @param m       The manifest to fill.
   @return        True on success, false if the Floor is empty or corrupt. */
BOL TManifestLoad(TList<ISC, ISC, DTB>* floor, TManifest* m);

/* Validates a manifest: non-empty name/version, valid kind/domain/
   determinism, reward_low <= reward_high, obs_count > 0,
   action_count > 0, max_episode_steps > 0, env_count_max > 0.
   @return        True if valid, false otherwise. */
BOL TManifestValidate(const TManifest* m);

}  //< namespace iGeek
#endif
#endif
