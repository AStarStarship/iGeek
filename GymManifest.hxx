// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_GYM_MANIFEST_HXX
#define IGEEK_GYM_MANIFEST_HXX 1
#include "GymManifest.h"
#include "List.hpp"
#include "Stringf.hpp"
namespace iGeek {

void TManifestInit(TManifest* m) {
  if (!m) return;
  for (ISN i = 0; i < ManifestNameLen; ++i) m->name[i] = 0;
  for (ISN i = 0; i < ManifestVersionLen; ++i) m->version[i] = 0;
  for (ISN i = 0; i < ManifestDescriptionLen; ++i) m->description[i] = 0;
  for (ISN i = 0; i < ManifestModalityLen; ++i) m->modality[i] = 0;
  for (ISN i = 0; i < ManifestWorldLen; ++i) m->world_name[i] = 0;
  m->kind = ManifestKindEnv;
  m->domain = ManifestDomainInvalid;
  m->determinism = ManifestDeterminismUnknown;
  m->reward_low = FPC(0.0f);
  m->reward_high = FPC(1.0f);
  m->reward_higher_is_better = true;
  m->obs_count = 0;
  m->action_count = 0;
  m->max_episode_steps = 0;
  m->env_count_max = 0;
  m->seam_number = ISC(0);
  m->manifest_version = ISC(1);
}

void CopyStr(CHA* dst, ISN dst_len, const CHA* src) {
  if (!dst || !src || dst_len < 1) return;
  *dst = 0;
  ::_::TSPrint(dst, dst_len, src);
}

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
static ISC WriteStr(TList<ISC, ISC, DTB>* floor, const CHA* s, ISN len,
                    ISC start) {
  for (ISN i = 0; i < len; ++i) {
    if (TListInsert(floor, s[i], start + ISC(i)) != start + ISC(i))
      return -1;
  }
  return start + ISC(len);
}

/* Reads a CHA array from the List as individual _CHA tuples. */
static BOL ReadStr(TList<ISC, ISC, DTB>* floor, CHA* s, ISN len, ISC start) {
  for (ISN i = 0; i < len; ++i) {
    const void* p = TListValue(floor, start + ISC(i));
    if (!p) return false;
    if (TListType(floor, start + ISC(i)) != _CHA) return false;
    s[i] = CHA(*TPtr<const CHA>(p));
  }
  return true;
}

BOL TManifestDump(const TManifest* m, TList<ISC, ISC, DTB>* floor) {
  if (!m || !floor) return false;
  TListClear(floor);
  /* 12 scalars at indices 0-11. */
  if (TListInsert(floor, m->kind) != ISC(0)) return false;
  if (TListInsert(floor, m->domain) != ISC(1)) return false;
  if (TListInsert(floor, m->determinism) != ISC(2)) return false;
  if (TListInsert(floor, m->reward_low) != ISC(3)) return false;
  if (TListInsert(floor, m->reward_high) != ISC(4)) return false;
  if (TListInsert(floor, IUC(m->reward_higher_is_better)) != ISC(5))
    return false;
  if (TListInsert(floor, m->obs_count) != ISC(6)) return false;
  if (TListInsert(floor, m->action_count) != ISC(7)) return false;
  if (TListInsert(floor, m->max_episode_steps) != ISC(8)) return false;
  if (TListInsert(floor, m->env_count_max) != ISC(9)) return false;
  if (TListInsert(floor, m->seam_number) != ISC(10)) return false;
  if (TListInsert(floor, m->manifest_version) != ISC(11)) return false;
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

BOL TManifestLoad(TList<ISC, ISC, DTB>* floor, TManifest* m) {
  if (!floor || !m) return false;
  if (ISC(floor->map.count) != ManifestTupleTotal) return false;
  /* Verify scalar types. */
  if (TListType(floor, ISC(0)) != _ISC) return false;
  if (TListType(floor, ISC(1)) != _ISC) return false;
  if (TListType(floor, ISC(2)) != _ISC) return false;
  if (TListType(floor, ISC(3)) != _FPC) return false;
  if (TListType(floor, ISC(4)) != _FPC) return false;
  if (TListType(floor, ISC(5)) != _IUC) return false;
  if (TListType(floor, ISC(6)) != _ISC) return false;
  if (TListType(floor, ISC(7)) != _ISC) return false;
  if (TListType(floor, ISC(8)) != _ISC) return false;
  if (TListType(floor, ISC(9)) != _ISC) return false;
  if (TListType(floor, ISC(10)) != _ISC) return false;
  if (TListType(floor, ISC(11)) != _ISC) return false;
  /* Read scalars. */
  m->kind = ISC(*TPtr<const ISC>(TListValue(floor, ISC(0))));
  m->domain = ISC(*TPtr<const ISC>(TListValue(floor, ISC(1))));
  m->determinism = ISC(*TPtr<const ISC>(TListValue(floor, ISC(2))));
  m->reward_low = FPC(*TPtr<const FPC>(TListValue(floor, ISC(3))));
  m->reward_high = FPC(*TPtr<const FPC>(TListValue(floor, ISC(4))));
  m->reward_higher_is_better =
      IUC(*TPtr<const IUC>(TListValue(floor, ISC(5)))) != IUC(0);
  m->obs_count = ISC(*TPtr<const ISC>(TListValue(floor, ISC(6))));
  m->action_count = ISC(*TPtr<const ISC>(TListValue(floor, ISC(7))));
  m->max_episode_steps = ISC(*TPtr<const ISC>(TListValue(floor, ISC(8))));
  m->env_count_max = ISC(*TPtr<const ISC>(TListValue(floor, ISC(9))));
  m->seam_number = ISC(*TPtr<const ISC>(TListValue(floor, ISC(10))));
  m->manifest_version = ISC(*TPtr<const ISC>(TListValue(floor, ISC(11))));
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

BOL TManifestValidate(const TManifest* m) {
  if (!m) return false;
  if (!::_::TSIsntEmpty(m->name)) return false;
  if (!::_::TSIsntEmpty(m->version)) return false;
  if (m->kind != ManifestKindEnv && m->kind != ManifestKindBenchmark)
    return false;
  if (m->domain == ManifestDomainInvalid) return false;
  if (m->determinism != ManifestDeterminismSeeded &&
      m->determinism != ManifestDeterminismStochastic &&
      m->determinism != ManifestDeterminismUnknown)
    return false;
  if (m->reward_low > m->reward_high) return false;
  if (m->obs_count <= 0) return false;
  if (m->action_count <= 0) return false;
  if (m->max_episode_steps <= 0) return false;
  if (m->env_count_max <= 0) return false;
  return true;
}

}  //< namespace iGeek
#endif
