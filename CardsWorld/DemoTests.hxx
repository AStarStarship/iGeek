// Copyright AStarship <https://astarship.net>.
#include "BlackjackDemo.hxx"
#include "../../ASCIICrabs/Test.hpp"
#include "_Seams/07.Demo.hxx"
using namespace _;
ISC main(ISC arg_count, CHA** args) {
  return TTestTree<CWTest::Demo>(arg_count, args);
}
