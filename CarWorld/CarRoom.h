// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_ROOM_H
#define CRABS_CARWORLD_ROOM_H
#include "CarWorld2D.h"
#include "../../ASCIICrabs/Room.hpp"
#if SEAM >= CRABS_CARWORLD
namespace iGeek {

enum { CarRoomAutopilot, CarRoomManual };
/** Native request schema v1: ISC version/mode/action, FPD steer/throttle/brake.
 * Autopilot actions select the existing classical driver; manual controls
 * bypass it. This is not a wire struct and contains no executable pointers. */
struct TCarRoomCommand
{
  ISC version = 1, mode = CarRoomAutopilot, action = CarCruise;
  TCarControl control;
};
/** Contiguous native reply snapshot; serialize fields, never struct padding. */
struct TCarRoomReply
{
  ISC version = 1, mode = CarRoomAutopilot, status = CarRunning;
  IUD frames = 0;
  FPC reward = 0;
  TCarState state;
};
using TCarRoomBase = TRoom<CHA, CHC, ISC, ISC, DTB, IUC>;

/** Native application binding of abstract Room -> C++ vehicle methods.
 * The Room owns its Floor request; the environment/map/route are borrowed
 * and must outlive it. Single-owner stepping only, no transport or locks.
 * A future Slot reducer must decode a message into SetCommand, not cast
 * incoming bytes to a C++ object or pass untrusted pointers to Star.
 * Operations: '?' metadata, 't' one 20ms frame, 'r' seeded episode reset.
 * Manual/autopilot takeover does not reset pose, time or trajectory.
 */
class ACarRoom : public TCarRoomBase
{
 public:
  explicit ACarRoom(ACarEnv& env);
  BOL SetCommand(const TCarRoomCommand& command);
  const Op* Star(CHC index, Crabs* crabs) override;
  const TCarRoomReply& Reply() const { return reply_; }
 private:
  ACarEnv& env_;
  TCarRoomReply reply_;
  static BOL Valid(const TCarRoomCommand& command);
  BOL ReadCommand(TCarRoomCommand& command) const;
  void Snapshot(ISC mode, FPC reward);
};
} //< namespace iGeek
#endif
#endif
