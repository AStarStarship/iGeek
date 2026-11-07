// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekEnvironmentManifestHPP
#define _iGeekEnvironmentManifestHPP
#include "EnvironmentManifest.h"
#if _Seam >= _iGeekEnvironmentRolloutCompare
namespace iGeek {
using namespace ::_;

/* Writes a CHA array into the List as individual _CHA tuples, starting
   at the given index.
   @return The next free index, or -1 if the Floor filled up. */
template<LST_A>
static ISC WriteStr(TList<LST_P>* floor, const CHA* s, ISN len, ISC start) {
  for (ISN i = 0; i < len; ++i) {
    if (_::TListInsert<LST_P>(floor, s[i], start + ISC(i)) != start + ISC(i))
      return -1;
  }
  return start + ISC(len);
}

/* Reads a CHA array from the List as individual _CHA tuples.
   @return True when all len tuples were present and _CHA-typed. */
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

/* The defaults live in the EnvironmentManifest.h declaration, so the
   definition here repeats the parameter list without them. */
template<typename ISZ, typename ISY, typename DT>
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
  if (_::TListInsert<LST_P>(floor, m->observation_count) != ISC(6)) return false;
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

template<typename ISZ, typename ISY, typename DT>
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
  m->observation_count = ISC(*_::TRef<const ISC>(_::TListValue<LST_P>(floor, ISC(6))));
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

}  //< namespace iGeek
#endif  //< _Seam >= _iGeekEnvironmentRolloutCompare
#endif  //< _iGeekEnvironmentManifestHPP
