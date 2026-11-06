// Copyright AStarship <https://astarship.net>.
#include "../../Gym.h"
#include "../../Gym.hxx"
#include "../../Tensor.h"
#include "../../PPO.h"
#include "../../LinearPolicy.hxx"
#include "../../Agent.h"
#include "../../Agent.hxx"
#include "../../Metrics.hxx"
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace iGeekTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* CoreMain(const CHA* args) {
  return TTestTree<iGeekTest::Core>(args);
}

}  //< namespace iGeekTest
