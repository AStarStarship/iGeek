// Copyright AStarship.

#pragma once
#include <_Config.h>

#include <Crabs/_Package.hxx>

int main(int arg_count, char** args) {
  enum { cSize = 1024 };
  char log[cSize];
  return ::_::TestTree<iGeek::WikiWorld::Core>(arg_count, args, log, cSize);
}
