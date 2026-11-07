// Copyright AStarship <https://astarship.net>.
#include "01.Core.hxx"
#include "02.JobQueue.hxx"
#include "03.RolloutCompare.hxx"
#include "04.Release.hxx"
namespace iGeekGymTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Gym(const CHA* args) {
  return TTestTree<Core, JobQueue, RolloutCompare, GymTest::Release>(args);
}

}  //< namespace iGeekGymTest
