// Copyright AStarship.

#pragma once
#include <_Config.h>

#include <Crabs/global.h"

/*
#include <kabuki/hal/global.h"

using namespace _;

int main(int arg_count, char** args) {
  enum { kSize = 1024 };
  char _log[kSize];
  return SeamTreeTest<_0_0_0_F2>(arg_count, args, _log, kSize); */

int main(int arg_count, char** args) {
  enum { kSize = 1024 };
  char _log[kSize];
  return ::_::TestTree<TestKabukiGraph>(arg_count, args, _log, kSize);
}
