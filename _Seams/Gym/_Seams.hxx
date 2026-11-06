// Copyright AStarship <https://astarship.net>.
#include "01.Core.hxx"
#include "02.Script.hxx"
#include "03.JobQueue.hxx"
#include "04.RoomSandbox.hxx"
#include "05.RolloutCompare.hxx"
#include "06.Release.hxx"
namespace iGeekGymTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Gym(const CHA* args) {
  return TTestTree<Core, Script, JobQueue, Sandbox, RolloutCompare, 
    GymTest::Release
  >(args);
}

}  //< namespace iGeekGymTest
