// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekEnvironmentManifestH
#define _iGeekEnvironmentManifestH
#include <_Config.h>
#if _Seam >= _iGeekEnvironmentManifest
#include "Environment.h"
#include "ROut.h"
#include "AType.hpp"
#include "List.hpp"
#include "Dic.hpp"
namespace iGeek {
using namespace ::_;

/* The manifest as a dictionary: CHS keys, value types in the DTB byte,
   hash in IUN. Same shape the JSON accessors use to read/write it. */
typedef _::TDic<CHR, CHR, ISR, ISR, DTB, IUN> ManifestDic;

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
  ISC obs_count;
  ISC action_count;
  ISC max_episode_steps;
  ISC environment_count_max;
  CHA world_name[ManifestWorldLen];
  ISC seam_number;
  ISC manifest_version;
};

/* Initializes a manifest to safe defaults (zeroed, kind=Env, domain=Invalid,
   determinism=Unknown, manifest_version=1). */
void EnvironmentManifestInit(EnvironmentManifest* m);

/* Total tuples: 12 scalars + 5 strings (each field stored as _CHA chars). */
enum {
  ManifestScalarCount = 12,
  ManifestTupleTotal =
      ManifestScalarCount +
      ManifestNameLen + ManifestVersionLen + ManifestDescriptionLen +
      ManifestModalityLen + ManifestWorldLen,
};

/* Writes a CHA array into the List as individual _CHA tuples, starting
   at the given index. Returns the next free index. */
template<LST_A>
static ISC WriteStr(TList<LST_P>* floor, const CHA* s, ISN len, ISC start) {
  for (ISN i = 0; i < len; ++i) {
    if (_::TListInsert<LST_P>(floor, s[i], start + ISC(i)) != start + ISC(i))
      return -1;
  }
  return start + ISC(len);
}

/* Reads a CHA array from the List as individual _CHA tuples. */
template<LST_A>
static BOL ReadStr(TList<LST_P>* floor, CHA* s, ISN len, ISC start) {
  for (ISN i = 0; i < len; ++i) {
    const void* p = _::TListValue<LST_P>(floor, start + ISC(i));
    if (!p) return false;
    if (_::TListType<LST_P>(floor, start + ISC(i)) != _CHA) return false;
    s[i] = CHA(*TRef<const CHA>(p));
  }
  return true;
}

/* Serializes the manifest into the Floor as ASCII tuples.
   @param m       The manifest to write.
   @param floor   The Room's Floor (ASCII List) to write into.
   @return        True on success, false if the Floor is full. */
template<LST_A>
BOL EnvironmentManifestDump(const EnvironmentManifest* m, TList<LST_P>* floor) {
  if (!m || !floor) return false;
  _::TListClear<LST_P>(floor);
  /* 12 scalars at indices 0-11. */
  if (_::TListInsert<LST_P>(floor, m->kind) != ISC(0)) return false;
  if (_::TListInsert<LST_P>(floor, m->domain) != ISC(1)) return false;
  if (_::TListInsert<LST_P>(floor, m->determinism) != ISC(2)) return false;
  if (_::TListInsert<LST_P>(floor, m->reward_low) != ISC(3)) return false;
  if (_::TListInsert<LST_P>(floor, m->reward_high) != ISC(4)) return false;
  if (_::TListInsert<LST_P>(floor, IUC(m->reward_higher_is_better)) != ISC(5))
    return false;
  if (_::TListInsert<LST_P>(floor, m->obs_count) != ISC(6)) return false;
  if (_::TListInsert<LST_P>(floor, m->action_count) != ISC(7)) return false;
  if (_::TListInsert<LST_P>(floor, m->max_episode_steps) != ISC(8)) return false;
  if (_::TListInsert<LST_P>(floor, m->environment_count_max) != ISC(9)) return false;
  if (_::TListInsert<LST_P>(floor, m->seam_number) != ISC(10)) return false;
  if (_::TListInsert<LST_P>(floor, m->manifest_version) != ISC(11)) return false;
  /* 5 strings as _CHA tuples at indices 12+. */
  ISC idx = ManifestScalarCount;
  if (WriteStr(floor, m->name, ManifestNameLen, idx) < 0) return false;
  idx += ManifestNameLen;
  if (WriteStr(floor, m->version, ManifestVersionLen, idx) < 0) return false;
  idx += ManifestVersionLen;
  if (WriteStr(floor, m->description, ManifestDescriptionLen, idx) < 0)
    return false;
  idx += ManifestDescriptionLen;
  if (WriteStr(floor, m->modality, ManifestModalityLen, idx) < 0) return false;
  idx += ManifestModalityLen;
  if (WriteStr(floor, m->world_name, ManifestWorldLen, idx) < 0) return false;
  return true;
}

/* Deserializes a manifest from the Floor.
   @param floor   The Room's Floor to read from.
   @param m       The manifest to fill.
   @return        True on success, false if the Floor is empty or corrupt. */
template<LST_A>
BOL EnvironmentManifestLoad(TList<LST_P>* floor, EnvironmentManifest* m) {
  if (!floor || !m) return false;
  if (ISC(floor->map.count) != ManifestTupleTotal) return false;
  /* Verify scalar types. */
  if (_::TListType<LST_P>(floor, ISC(0)) != _ISC) return false;
  if (_::TListType<LST_P>(floor, ISC(1)) != _ISC) return false;
  if (_::TListType<LST_P>(floor, ISC(2)) != _ISC) return false;
  if (_::TListType<LST_P>(floor, ISC(3)) != _FPC) return false;
  if (_::TListType<LST_P>(floor, ISC(4)) != _FPC) return false;
  if (_::TListType<LST_P>(floor, ISC(5)) != _IUC) return false;
  if (_::TListType<LST_P>(floor, ISC(6)) != _ISC) return false;
  if (_::TListType<LST_P>(floor, ISC(7)) != _ISC) return false;
  if (_::TListType<LST_P>(floor, ISC(8)) != _ISC) return false;
  if (_::TListType<LST_P>(floor, ISC(9)) != _ISC) return false;
  if (_::TListType<LST_P>(floor, ISC(10)) != _ISC) return false;
  if (_::TListType<LST_P>(floor, ISC(11)) != _ISC) return false;
  /* Read scalars. */
  m->kind = ISC(*_::TRef<const ISC>(_::TListValue<LST_P>(floor, ISC(0))));
  m->domain = ISC(*_::TRef<const ISC>(_::TListValue<LST_P>(floor, ISC(1))));
  m->determinism = ISC(*_::TRef<const ISC>(_::TListValue<LST_P>(floor, ISC(2))));
  m->reward_low = FPC(*_::TRef<const FPC>(_::TListValue<LST_P>(floor, ISC(3))));
  m->reward_high = FPC(*_::TRef<const FPC>(_::TListValue<LST_P>(floor, ISC(4))));
  m->reward_higher_is_better =
      IUC(*_::TRef<const IUC>(_::TListValue<LST_P>(floor, ISC(5)))) != IUC(0);
  m->obs_count = ISC(*_::TRef<const ISC>(_::TListValue<LST_P>(floor, ISC(6))));
  m->action_count = ISC(*_::TRef<const ISC>(_::TListValue<LST_P>(floor, ISC(7))));
  m->max_episode_steps = ISC(*_::TRef<const ISC>(_::TListValue<LST_P>(floor, ISC(8))));
  m->environment_count_max = ISC(*_::TRef<const ISC>(_::TListValue<LST_P>(floor, ISC(9))));
  m->seam_number = ISC(*_::TRef<const ISC>(_::TListValue<LST_P>(floor, ISC(10))));
  m->manifest_version = ISC(*_::TRef<const ISC>(_::TListValue<LST_P>(floor, ISC(11))));
  /* Read strings. */
  ISC idx = ManifestScalarCount;
  if (!ReadStr(floor, m->name, ManifestNameLen, idx)) return false;
  idx += ManifestNameLen;
  if (!ReadStr(floor, m->version, ManifestVersionLen, idx)) return false;
  idx += ManifestVersionLen;
  if (!ReadStr(floor, m->description, ManifestDescriptionLen, idx)) return false;
  idx += ManifestDescriptionLen;
  if (!ReadStr(floor, m->modality, ManifestModalityLen, idx)) return false;
  idx += ManifestModalityLen;
  if (!ReadStr(floor, m->world_name, ManifestWorldLen, idx)) return false;
  return true;
}

/* Validates a manifest: non-empty name/version, valid kind/domain/
   determinism, reward_low <= reward_high, obs_count > 0,
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

}  //< namespace iGeek
#endif
#endif
