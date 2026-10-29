// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_SCRIPT_HXX
#define IGEEK_SCRIPT_HXX 1
#include "Script.h"
namespace iGeek {

void TScriptInit(TScript* script, IUD max_count) {
  if (!script) return;
  for (IUD i = 0; i < 256; ++i) {
    script->tuples[i].frame = IUD(0);
    script->tuples[i].event = IUC(0);
    script->tuples[i].agent_id = IUC(0);
    script->tuples[i].value = FPD(0.0);
    script->tuples[i].detail = ISC(0);
  }
  script->write_idx = IUD(0);
  script->count = IUD(0);
  script->max_count = max_count;
}

BOL TScriptRecord(TScript* script, IUD frame, IUC event, IUC agent_id,
                 FPD value, ISC detail) {
  if (!script || script->max_count == IUD(0)) return false;
  IUD idx = script->write_idx % script->max_count;
  script->tuples[idx].frame = frame;
  script->tuples[idx].event = event;
  script->tuples[idx].agent_id = agent_id;
  script->tuples[idx].value = value;
  script->tuples[idx].detail = detail;
  ++script->write_idx;
  if (script->count < script->max_count) ++script->count;
  return true;
}

BOL TScriptQuery(const TScript* script, IUC event, IUC agent_id,
                FPD* out_value, IUD* out_frame) {
  if (!script || script->count == IUD(0)) return false;
  IUD latest = IUD(0);
  IUD latest_frame = IUD(0);
  BOL found = false;
  for (IUD i = IUD(0); i < script->count; ++i) {
    IUD idx = (script->write_idx - script->count + i) % script->max_count;
    const TScrTuple* t = &script->tuples[idx];
    if (t->event == event && t->agent_id == agent_id) {
      if (!found || t->frame >= latest_frame) {
        latest_frame = t->frame;
        if (out_value) *out_value = t->value;
        found = true;
      }
    }
  }
  if (found && out_frame) *out_frame = latest_frame;
  return found;
}

BOL TScriptSummary(const TScript* script, IUC event, FPD* out_sum,
                  IUD* out_count, FPD* out_min, FPD* out_max) {
  if (!script) return false;
  FPD sum = FPD(0.0);
  FPD min_v = FPD(1e30);
  FPD max_v = FPD(-1e30);
  IUD count = IUD(0);
  for (IUD i = IUD(0); i < script->count; ++i) {
    IUD idx = (script->write_idx - script->count + i) % script->max_count;
    const TScrTuple* t = &script->tuples[idx];
    if (t->event == event) {
      sum += t->value;
      if (t->value < min_v) min_v = t->value;
      if (t->value > max_v) max_v = t->value;
      ++count;
    }
  }
  if (count == IUD(0)) return false;
  if (out_sum) *out_sum = sum;
  if (out_count) *out_count = count;
  if (out_min) *out_min = min_v;
  if (out_max) *out_max = max_v;
  return true;
}

IUD TScriptCount(const TScript* script) {
  return script ? script->count : IUD(0);
}

void TScriptClear(TScript* script) {
  if (!script) return;
  script->write_idx = IUD(0);
  script->count = IUD(0);
}

void TScriptIterate(const TScript* script,
                    BOL (*fn)(const TScrTuple* tuple, void* user),
                    void* user) {
  if (!script || !fn) return;
  for (IUD i = IUD(0); i < script->count; ++i) {
    IUD idx = (script->write_idx - script->count + i) % script->max_count;
    if (!fn(&script->tuples[idx], user)) return;
  }
}

}  //< namespace iGeek
#endif
