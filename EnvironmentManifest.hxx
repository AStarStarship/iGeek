// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekEnvironmentManifest_HXX
#define _iGeekEnvironmentManifest_HXX 1
#include "EnvironmentManifest.h"
#include "EnvironmentManifest.hpp"
#include "List.hpp"
#include "Stringf.hpp"
#include "Environment.h"
namespace iGeek {
using namespace ::_;

void EnvironmentManifestInit(EnvironmentManifest* m) {
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
  m->observation_count = 0;
  m->action_count = 0;
  m->max_episode_steps = 0;
  m->environment_count_max = 0;
  m->seam_number = ISC(0);
  m->manifest_version = ISC(1);
}

/* Non-template helpers: see EnvironmentManifest.h for the inlined templates. */

BOL EnvironmentManifestValidate(const EnvironmentManifest* m) {
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
  if (m->observation_count <= 0) return false;
  if (m->action_count <= 0) return false;
  if (m->max_episode_steps <= 0) return false;
  if (m->environment_count_max <= 0) return false;
  return true;
}

/* ---- ManifestDic (JSON) accessors: implementations ----------------- */

BOL ManifestGetString(const ManifestDic* dic, const CHS* key,
                      CHA* out, ISN out_len) {
  if (IsError(dic) || IsError(key) || IsError(out) || out_len < 1) return false;
  ISR index = _::TDicFind<DIC_D>(
      CRef<ManifestDic>(dic), key);
  if (index < 0) return false;
  DTB type = _::TDicType_NC<DIC_D>(dic,
      dic->values.map.total, index);
  if (type != _::_STA) return false;
  const void* val = _::TDicValue_NC<DIC_D>(dic, index);
  if (IsError(val)) return false;
  const CHA* str = _::CRef<const CHA>(_::TRef<IUW>(val));
  if (IsError(str)) return false;
  _::TSPrint(out, out_len, str);
  return true;
}

BOL ManifestGetInt(const ManifestDic* dic, const CHS* key, ISC* out) {
  if (IsError(dic) || IsError(key) || IsError(out)) return false;
  ISR index = _::TDicFind<DIC_D>(
      CRef<ManifestDic>(dic), key);
  if (index < 0) return false;
  DTB type = _::TDicType_NC<DIC_D>(dic,
      dic->values.map.total, index);
  if (type != _::_ISC) return false;
  *out = ISC(*_::TRef<const ISC>(
      _::TDicValue_NC<DIC_D>(dic, index)));
  return true;
}

BOL ManifestGetFloat(const ManifestDic* dic, const CHS* key, FPC* out) {
  if (IsError(dic) || IsError(key) || IsError(out)) return false;
  ISR index = _::TDicFind<DIC_D>(
      CRef<ManifestDic>(dic), key);
  if (index < 0) return false;
  DTB type = _::TDicType_NC<DIC_D>(dic,
      dic->values.map.total, index);
  if (type != _::_FPC) return false;
  *out = FPC(*_::TRef<const FPC>(
      _::TDicValue_NC<DIC_D>(dic, index)));
  return true;
}

BOL ManifestSetString(ManifestDic* dic, const CHS* key, const CHA* value) {
  if (!dic || !key || !value) return false;
  ISR index = _::TDicInsert<DIC_D>(dic, key, _::_STA, IUW(value));
  return index >= 0;
}

BOL ManifestSetInt(ManifestDic* dic, const CHS* key, ISC value) {
  if (!dic || !key) return false;
  ISR index = _::TDicInsert<DIC_D>(dic, key, _::_ISC, IUW(value));
  return index >= 0;
}

BOL ManifestSetFloat(ManifestDic* dic, const CHS* key, FPC value) {
  if (!dic || !key) return false;
  ISR index = _::TDicInsert<DIC_D>(dic, key, _::_FPC, IUW(value));
  return index >= 0;
}

static void PrintJsonKey(const CHS* key, ::_::ROut& out) {
  out << '"';
  while (key && *key) {
    if (*key == CHS('"')) out << "\\\"";
    else if (*key == CHS('\\')) out << "\\\\";
    else out << CHA(*key);
    ++key;
  }
  out << '"';
}

BOL EnvironmentManifestPrintJson(const ManifestDic* dic, ::_::ROut& out) {
  if (!dic) return false;
  out << '{';
  typedef _::TTable<CHR, CHR, ISR, ISR, IUN> KeysTable;
  const KeysTable* keys = _::CRef<const KeysTable>(
      _::TDicKeys<DIC_D>(
          _::CRef<ManifestDic>(dic)));
  ISC count = ISC(dic->values.map.count);
  BOL first = true;
  for (ISC i = ISC(1); i < count; ++i) {
    const CHS* key = _::TTableGet_NC<CHR, CHR, ISR, ISR, IUN>(
        keys, ISR(keys->map.total), ISR(i));
    if (!key) continue;
    if (!first) out << ',';
    first = false;
    PrintJsonKey(key, out);
    out << ':';
    DTB type = _::TDicType_NC<DIC_D>(dic,
        dic->values.map.total, i);
    const void* val = _::TDicValue_NC<DIC_D>(dic, i);
    if (!val) { out << "null"; continue; }
    switch (type) {
      case _::_ISC: out << ISC(*_::TRef<const ISC>(val)); break;
      case _::_IUC: out << IUC(*_::TRef<const IUC>(val)); break;
      case _::_ISD: out << ISD(*_::TRef<const ISD>(val)); break;
      case _::_IUD: out << IUD(*_::TRef<const IUD>(val)); break;
      case _::_FPC: out << FPC(*_::TRef<const FPC>(val)); break;
      case _::_FPD: out << FPD(*_::TRef<const FPD>(val)); break;
      case _::_STA: {
        const CHA* s = _::CRef<const CHA>(_::TRef<IUW>(val));
        out << '"';
        while (s && *s) {
          if (*s == '"') out << "\\\"";
          else if (*s == '\\') out << "\\\\";
          else out << *s;
          ++s;
        }
        out << '"';
        break;
      }
      case _::_IUA: out << IUA(*_::TRef<const IUA>(val)); break;
      case _::_IUB: out << IUB(*_::TRef<const IUB>(val)); break;
      case _::_ISB: out << ISB(*_::TRef<const ISB>(val)); break;
      case _::_ISA: out << ISA(*_::TRef<const ISA>(val)); break;
      case _::_CHA: out << CHA(*_::TRef<const CHA>(val)); break;
      default:   out << "null"; break;
    }
  }
  out << '}';
  return true;
}

BOL EnvironmentManifestToDic(const EnvironmentManifest* m, ManifestDic* dic) {
  if (!m || !dic) return false;
  BOL ok = true;
  ok = ok && ManifestSetString(dic, "name", m->name);
  ok = ok && ManifestSetString(dic, "version", m->version);
  ok = ok && ManifestSetInt(dic, "kind", m->kind);
  ok = ok && ManifestSetInt(dic, "domain", m->domain);
  ok = ok && ManifestSetString(dic, "description", m->description);
  ok = ok && ManifestSetString(dic, "modality", m->modality);
  ok = ok && ManifestSetFloat(dic, "reward_low", m->reward_low);
  ok = ok && ManifestSetFloat(dic, "reward_high", m->reward_high);
  ok = ok && ManifestSetInt(dic, "reward_higher_is_better",
                            m->reward_higher_is_better ? ISC(1) : ISC(0));
  ok = ok && ManifestSetInt(dic, "determinism", m->determinism);
  ok = ok && ManifestSetInt(dic, "observation_count", m->observation_count);
  ok = ok && ManifestSetInt(dic, "action_count", m->action_count);
  ok = ok && ManifestSetInt(dic, "max_episode_steps", m->max_episode_steps);
  ok = ok && ManifestSetInt(dic, "environment_count_max", m->environment_count_max);
  ok = ok && ManifestSetString(dic, "world_name", m->world_name);
  ok = ok && ManifestSetInt(dic, "seam_number", m->seam_number);
  ok = ok && ManifestSetInt(dic, "manifest_version", m->manifest_version);
  return ok;
}

BOL ManifestDicToEnvironmentManifest(const ManifestDic* dic, EnvironmentManifest* m) {
  if (!dic || !m) return false;
  EnvironmentManifestInit(m);
  ISC n = ISC(0);
  FPC f = FPC(0.0);
  BOL ok = true;
  ok = ok && ManifestGetString(dic, "name", m->name, ManifestNameLen);
  ok = ok && ManifestGetString(dic, "version", m->version, ManifestVersionLen);
  ok = ok && ManifestGetInt(dic, "kind", &n);
  m->kind = n;
  ok = ok && ManifestGetInt(dic, "domain", &n);
  m->domain = n;
  ok = ok && ManifestGetString(dic, "description", m->description,
                               ManifestDescriptionLen);
  ok = ok && ManifestGetString(dic, "modality", m->modality,
                               ManifestModalityLen);
  ok = ok && ManifestGetFloat(dic, "reward_low", &f);
  m->reward_low = f;
  ok = ok && ManifestGetFloat(dic, "reward_high", &f);
  m->reward_high = f;
  ok = ok && ManifestGetInt(dic, "reward_higher_is_better", &n);
  m->reward_higher_is_better = n != ISC(0);
  ok = ok && ManifestGetInt(dic, "determinism", &n);
  m->determinism = n;
  ok = ok && ManifestGetInt(dic, "observation_count", &n);
  m->observation_count = n;
  ok = ok && ManifestGetInt(dic, "action_count", &n);
  m->action_count = n;
  ok = ok && ManifestGetInt(dic, "max_episode_steps", &n);
  m->max_episode_steps = n;
  ok = ok && ManifestGetInt(dic, "environment_count_max", &n);
  m->environment_count_max = n;
  ok = ok && ManifestGetString(dic, "world_name", m->world_name,
                               ManifestWorldLen);
  ok = ok && ManifestGetInt(dic, "seam_number", &n);
  m->seam_number = n;
  ok = ok && ManifestGetInt(dic, "manifest_version", &n);
  m->manifest_version = n;
  return ok;
}

}  //< namespace iGeek
#endif
