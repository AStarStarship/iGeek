// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekCarWorldRoomHXX
#define _iGeekCarWorldRoomHXX
#include "CarWorld2D.hxx"
#include "../../ASCIICrabs/Math.hxx"
#include "CarRoom.h"
#if _Seam >= _iGeekCarWorldCore
namespace iGeek {
ACarRoom::ACarRoom(ACarEnv& env) : TCarRoomBase("carworld"), env_(env) {
  TCarRoomCommand empty;
    SetCommand(empty);
  Snapshot(CarRoomAutopilot, 0);
}
BOL ACarRoom::Valid(TCarRoomCommand& command) {
  return command.version == 1 &&
      (command.mode == CarRoomManual || command.mode == CarRoomAutopilot) &&
      command.action >= 0 && command.action < CarActionCount &&
      iGeek::IsFinite(command.control.steer) && Absolute(command.control.steer) <= 0.55 &&
      iGeek::IsFinite(command.control.throttle) && command.control.throttle >= 0 &&
      command.control.throttle <= 1 && iGeek::IsFinite(command.control.brake) &&
      command.control.brake >= 0 && command.control.brake <= 1;
}
BOL ACarRoom::SetCommand(TCarRoomCommand& command) {
  // Validate before changing the prior command.
  if (!Valid(command)) return false;
  ClearFloor();
  auto* floor = Floor();
  // Insert key-value pairs: the Floor is a TDic (keys Table at index 0).
  CHA k_ver[]   = {'v','e','r',0};
  CHA k_mode[]  = {'m','o','d','e',0};
  CHA k_act[]   = {'a','c','t',0};
  CHA k_steer[] = {'s','t','e','e','r',0};
  CHA k_thr[]   = {'t','h','r',0};
  CHA k_brk[]   = {'b','r','k',0};
  return TDicInsert<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_ver,   _::_ISC, IUW(ISC(command.version))) >= 0 &&
         TDicInsert<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_mode,  _::_ISC, IUW(ISC(command.mode))) >= 0 &&
         TDicInsert<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_act,   _::_ISC, IUW(ISC(command.action))) >= 0 &&
         TDicInsert<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_steer, _::_FPD, IUW(*TRef<IUW>(&command.control.steer))) >= 0 &&
         TDicInsert<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_thr,   _::_FPD, IUW(*TRef<IUW>(&command.control.throttle))) >= 0 &&
         TDicInsert<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_brk,   _::_FPD, IUW(*TRef<IUW>(&command.control.brake))) >= 0;
}
BOL ACarRoom::ReadCommand(TCarRoomCommand& command) {
  const auto* floor = Floor();
  CHA k_ver[]   = {'v','e','r',0};
  CHA k_mode[]  = {'m','o','d','e',0};
  CHA k_act[]   = {'a','c','t',0};
  CHA k_steer[] = {'s','t','e','e','r',0};
  CHA k_thr[]   = {'t','h','r',0};
  CHA k_brk[]   = {'b','r','k',0};
  // Validate the Floor Dic type schema before reading any values.
  // The values List stores type tags after the data (TListTypes).
  // Index 0 = keys Table, indices 1..N = KV values in insertion order.
  // Expected: ver=ISC, mode=ISC, act=ISC, steer=FPD, thr=FPD, brk=FPD.
  const DTB* types = TListTypes(&floor->values, floor->values.map.total);
  if (types[1] != _ISC || types[2] != _ISC || types[3] != _ISC ||
      types[4] != _FPD || types[5] != _FPD || types[6] != _FPD)
    return false;
  ISC idx;
  idx = TDicFind<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_ver);
  if (idx < 0) return false;
  command.version = *TRef<const ISC>(TDicValue_NC<CHA, CHC, ISC, ISC, DTB, IUC>(floor, idx));
  idx = TDicFind<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_mode);
  if (idx < 0) return false;
  command.mode = *TRef<const ISC>(TDicValue_NC<CHA, CHC, ISC, ISC, DTB, IUC>(floor, idx));
  idx = TDicFind<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_act);
  if (idx < 0) return false;
  command.action = *TRef<const ISC>(TDicValue_NC<CHA, CHC, ISC, ISC, DTB, IUC>(floor, idx));
  idx = TDicFind<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_steer);
  if (idx < 0) return false;
  command.control.steer = *TRef<const FPD>(TDicValue_NC<CHA, CHC, ISC, ISC, DTB, IUC>(floor, idx));
  idx = TDicFind<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_thr);
  if (idx < 0) return false;
  command.control.throttle = *TRef<const FPD>(TDicValue_NC<CHA, CHC, ISC, ISC, DTB, IUC>(floor, idx));
  idx = TDicFind<CHA, CHC, ISC, ISC, DTB, IUC>(floor, k_brk);
  if (idx < 0) return false;
  command.control.brake = *TRef<const FPD>(TDicValue_NC<CHA, CHC, ISC, ISC, DTB, IUC>(floor, idx));
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
      ';', '}', 0, _NilP};
  // This native endpoint does not yet consume interpreter/Slot arguments.
  if (crabs != _NilP) return OpError(AErrorInvalidInput);
  if (index == '?') return &Metadata;
  if (index == 'r') {
    env_.Reset();
    TCarRoomCommand empty;
    SetCommand(empty);
    Snapshot(CarRoomAutopilot, 0);
    return _NilP;
  }
  if (index != 't') return OpError(AErrorInvalidOp);
  TCarRoomCommand command;
  if (!ReadCommand(command)) return OpError(AErrorInvalidInput);
  FPC reward = command.mode == CarRoomManual ?
      env_.StepControl(command.control) : env_.Step(command.action);
  Snapshot(command.mode, reward);
  return _NilP;
}
} //< namespace iGeek
#endif
#endif
