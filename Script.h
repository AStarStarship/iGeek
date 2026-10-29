// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_SCRIPT_H
#define IGEEK_SCRIPT_H
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
namespace iGeek {

/* SCRIPT: Serial Chinese Room, Interprocess, and Telemetry.
  The script is the Room's serial log: a fixed-size ring of tuples
  recording every event (frame, join, leave, reward, episode_end,
  error, heartbeat) in the order it happened. The script IS the
  telemetry — reading the script is reading the telemetry.
  No allocation, no I/O. The ring overwrites the oldest tuple when
  full. Stored in a Room Wall (contiguous memory). */

/* Script event types. */
enum {
  ScrFrame = 0,
  ScrJoin,
  ScrLeave,
  ScrReward,
  ScrEpisodeEnd,
  ScrError,
  ScrHeartbeat,
  ScrEventCount,
};

/* A single script tuple: one event, one frame, one agent. */
struct TScrTuple {
  IUD frame;
  IUC event;
  IUC agent_id;
  FPD value;
  ISC detail;
};

/* The script: a fixed-size ring of tuples in a Room Wall. */
struct TScript {
  TScrTuple tuples[256];
  IUD write_idx;
  IUD count;
  IUD max_count;
};

/* Initializes the script to empty. */
void TScriptInit(TScript* script, IUD max_count);

/* Records one tuple to the script. Overwrites the oldest when full. */
BOL TScriptRecord(TScript* script, IUD frame, IUC event, IUC agent_id,
                 FPD value, ISC detail);

/* Reads the most recent tuple for (event, agent). */
BOL TScriptQuery(const TScript* script, IUC event, IUC agent_id,
                FPD* out_value, IUD* out_frame);

/* Aggregates over all tuples for an event (all agents). */
BOL TScriptSummary(const TScript* script, IUC event, FPD* out_sum,
                  IUD* out_count, FPD* out_min, FPD* out_max);

/* Returns the number of tuples in the script. */
IUD TScriptCount(const TScript* script);

/* Clears the script. */
void TScriptClear(TScript* script);

/* Iterates the script from oldest to newest, calling fn for each tuple.
Stops early if fn returns false. */
void TScriptIterate(const TScript* script,
                    BOL (*fn)(const TScrTuple* tuple, void* user),
                    void* user);

}  //< namespace iGeek
#endif
#endif
