// Copyright AStarship <https://astarship.net>.
//
// Gym seam test tree: wires all 6 Gym units.
#pragma once
#include "01.Manifest.hxx"
#include "02.AgentSlot.hxx"
#include "03.Script.hxx"
#include "04.JobQueue.hxx"
#include "05.RoomSandbox.hxx"
#include "06.RolloutCompare.hxx"
using namespace ::_;
namespace iGeekGymTest {

inline const CHA* Gym(const CHA* args) {
  return TTestTree<iGeekGymTest::Manifest, iGeekGymTest::AgentSlot,
                   iGeekGymTest::Script, iGeekGymTest::JobQueue,
                   iGeekGymTest::Sandbox, iGeekGymTest::RolloutCompare>(args);
}

}  //< namespace iGeekGymTest
