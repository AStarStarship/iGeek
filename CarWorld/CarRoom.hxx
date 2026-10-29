// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_ROOM_HXX
#define CRABS_CARWORLD_ROOM_HXX
#include "CarWorld2D.hxx"
#include "CarRoom.h"
#if SEAM >= CRABS_CARWORLD
namespace iGeek {
ACarRoom::ACarRoom(ACarEnv& env) : TCarRoomBase("carworld"), env_(env) {
  SetCommand(TCarRoomCommand());
  Snapshot(CarRoomAutopilot, 0);
}
BOL ACarRoom::Valid(const TCarRoomCommand& command) {
  return command.version == 1 &&
      (command.mode == CarRoomManual || command.mode == CarRoomAutopilot) &&
      command.action >= 0 && command.action < CarActionCount &&
      isfinite(command.control.steer) && fabs(command.control.steer) <= 0.55 &&
      isfinite(command.control.throttle) && command.control.throttle >= 0 &&
      command.control.throttle <= 1 && isfinite(command.control.brake) &&
      command.control.brake >= 0 && command.control.brake <= 1;
}
BOL ACarRoom::SetCommand(const TCarRoomCommand& command) {
  // Validate before changing the prior command. Six scalars fit the fixed Floor.
  if (!Valid(command)) return false;
  ClearFloor();
  auto* floor = Floor();
  return TListInsert(floor, command.version) == 0 &&
      TListInsert(floor, command.mode) == 1 &&
      TListInsert(floor, command.action) == 2 &&
      TListInsert(floor, command.control.steer) == 3 &&
      TListInsert(floor, command.control.throttle) == 4 &&
      TListInsert(floor, command.control.brake) == 5;
}
BOL ACarRoom::ReadCommand(TCarRoomCommand& command) const {
  const auto* floor = Floor();
  if (floor->bytes != FloorSize || floor->map.total != FloorTupleMax ||
      floor->map.count != 6) return false;
  const DTB types[6] = {_ISC, _ISC, _ISC, _FPD, _FPD, _FPD};
  ISC cursor = ISC(sizeof(*floor)) + FloorTupleMax * ISC(sizeof(ISC) + sizeof(DTB));
  const ISC* offsets = TListValuesMap(floor);
  // Check the canonical packed schema before dereferencing any stored offset.
  for (ISC i = 0; i < 6; ++i) {
    ISC bytes = i < 3 ? ISC(sizeof(ISC)) : ISC(sizeof(FPD));
    cursor = (cursor + bytes - 1) & ~(bytes - 1);
    if (TListType(floor, i) != types[i] || offsets[i] != cursor ||
        cursor > FloorSize - bytes) return false;
    cursor += bytes;
  }
  if (floor->top != cursor) return false;
  command.version = *TPtr<const ISC>(TListValue(floor, ISC(0)));
  command.mode = *TPtr<const ISC>(TListValue(floor, ISC(1)));
  command.action = *TPtr<const ISC>(TListValue(floor, ISC(2)));
  command.control.steer = *TPtr<const FPD>(TListValue(floor, ISC(3)));
  command.control.throttle = *TPtr<const FPD>(TListValue(floor, ISC(4)));
  command.control.brake = *TPtr<const FPD>(TListValue(floor, ISC(5)));
  return Valid(command);
}
void ACarRoom::Snapshot(ISC mode, FPC reward) {
  reply_.mode = mode;
  reply_.status = env_.Status();
  reply_.frames = env_.Metrics().frames;
  reply_.reward = reward;
  reply_.state = env_.State();
}
const Op* ACarRoom::Star(CHC index, Crabs* crabs) {
  static const Op Metadata = {"CarWorld", OpFirst('r'), OpLast('t'),
      "Native vehicle Room: reset or tick validated Floor schema v1.",
      ';', '}', 0, NILP};
  // This native endpoint does not yet consume interpreter/Slot arguments.
  if (crabs != NILP) return OpError(AErrorInvalidInput);
  if (index == '?') return &Metadata;
  if (index == 'r') {
    env_.Reset();
    SetCommand(TCarRoomCommand());
    Snapshot(CarRoomAutopilot, 0);
    return NILP;
  }
  if (index != 't') return OpError(AErrorInvalidOp);
  TCarRoomCommand command;
  if (!ReadCommand(command)) return OpError(AErrorInvalidInput);
  FPC reward = command.mode == CarRoomManual ?
      env_.StepControl(command.control) : env_.Step(command.action);
  Snapshot(command.mode, reward);
  return NILP;
}
} //< namespace iGeek
#endif
#endif
