// Copyright AStarship <https://astarship.net>.
// iGeek core framework self-test entrypoint (single TU).
//
// Pulls in the whole core layer via the .hxx impl chain and runs the checks
// from core_test.cpp. Plain main: returns non-zero on any failure; the exit
// code is the CI signal. Run: ./igeeK_core
#include "_Config.h"
#include <stdio.h>
// Core impl chain (each .hxx includes its own .h + deps):
#include "../Tensor.hxx"      // TTensor ops
#include "../Agent.hxx"       // Agent tiers (-> AgentState.hxx)
#include "../Metrics.hxx"     // BehaviorMetrics, StateRecordSet, LinearProbe
#include "../PPO.hxx"         // TPPO (-> LinearPolicy.hxx, PolicyNet.hxx)
#include "../Gym.hxx"         // Gym base impl
#include "core_test.cpp"      // the checks (defines ::_::Core)

int main(int argc, char** argv) {
  (void)argc;
  const CHA* result = ::_::Core(argv[0] ? argv[0] : "igeeK_core");
  if (result != 0) {
    printf("\niGeek core test FAILED: %s\n", result);
    return 1;
  }
  printf("\niGeek core test PASSED\n");
  return 0;
}
