// Copyright AStarship <https://astarship.net>.

// The single translation-unit entrypoint for the iGeek Cards World,
// mimicking ASCIICrabs/_Seams/_Main.cpp. It includes the config, the test
// aggregator (which pulls in the whole card game + seam units), and then
// dispatches: when SEAM == SEAM_N run the Release demo, otherwise run the
// unit test tree.

#include <_Config.h>
#include "_Tests.hxx"
using namespace ::_;

ISN main(ISN arg_count, CHA** args) {
  const CHA* argss = ArgsToString(arg_count, args);
#if SEAM == SEAM_N
  return SeamResult(Release(argss));
#else
  return TTestTree<CardsWorldTests>(arg_count, args);
#endif
}
