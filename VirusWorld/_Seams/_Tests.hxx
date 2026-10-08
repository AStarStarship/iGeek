// Copyright AStarship <https://astarship.net>.
#include <_Config.h>
//
#include "../_Package.hxx"
//
#include "01.Core.hxx"
#include "02.Release.hxx"
//
#include <Test.hpp>
using namespace ::_;

inline const CHA* VireworldTests(const CHA* args) {
  return TTestTree<VireworldTest::Core, VireworldTest::Release>(args);
}
