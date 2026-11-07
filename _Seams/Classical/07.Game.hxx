// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekClassicalGame
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#include "../../Classical/Game.hxx"
namespace iGeek {

static int game_pass = 0;
static int game_fail = 0;

static void GTCHECK(const char* name, BOL ok) {
  if (ok) {
    game_pass++;
    _DROut("PASS " << name << '\n');
  } else {
    game_fail++;
    _DROut("FAIL " << name << '\n');
  }
}

/* A tiny game tree: node id is the single IUC word of the state. Each node
   is either terminal (utility known) or has up to 4 actions with fixed
   successors. The callbacks read the file-static g_nodes table, which each
   test repopulates. */
struct GNode {
  BOL terminal;
  FPC utility;
  ISC action_count;
  IUC action[4];
  IUC successor[4];
};

static GNode g_nodes[16];

static BOL GIsTerminal(const IUC* state, ISC state_words) {
  (void)state_words;
  return g_nodes[state[0]].terminal;
}
static ISC GActions(const IUC* state, ISC state_words, IUC* out, ISC cap) {
  (void)state_words;
  GNode& n = g_nodes[state[0]];
  ISC k = 0;
  for (ISC i = 0; i < n.action_count && k < cap; ++i) {
    out[k++] = n.action[i];
  }
  return k;
}
static void GResult(const IUC* state, ISC state_words, IUC action, IUC* out) {
  (void)state_words;
  GNode& n = g_nodes[state[0]];
  out[0] = state[0];
  for (ISC i = 0; i < n.action_count; ++i) {
    if (n.action[i] == action) {
      out[0] = n.successor[i];
      return;
    }
  }
}
static FPC GUtility(const IUC* state, ISC state_words) {
  (void)state_words;
  return g_nodes[state[0]].utility;
}

static void GSetNode(ISC id, BOL terminal, FPC utility, const IUC* acts,
                     ISC act_count, const IUC* succ, ISC succ_count) {
  (void)succ_count;
  GNode& n = g_nodes[id];
  n.terminal = terminal;
  n.utility = utility;
  n.action_count = act_count;
  for (ISC i = 0; i < act_count; ++i) {
    n.action[i] = acts[i];
    n.successor[i] = succ[i];
  }
}

inline const CHA* GameTest(const CHA* args) {
#if _Seam >= _iGeekClassicalGame
  _ATestBegin;
  _DROut(Headingf("Testing Classical Game (Minimax / Alpha-Beta)..."));

  IUC initial[1] = {0};
  Game game;
  game.initial = initial;
  game.state_words = 1;
  game.IsTerminal = GIsTerminal;
  game.Actions = GActions;
  game.Result = GResult;
  game.Utility = GUtility;

  // --- Tree 1: optimal is action 0; alpha-beta prunes the sibling.
  // root(MAX) -> {0: node1(terminal +10), 1: node2(MIN)}
  //            node2 -> {0: node3(terminal +5), 1: node4(terminal +20)}
  {
    IUC acts0[2] = {0, 1};
    IUC succ0[2] = {1, 2};
    IUC acts2[2] = {0, 1};
    IUC succ2[2] = {3, 4};
    GSetNode(0, false, 0.0f, acts0, 2, succ0, 2);
    GSetNode(1, true, 10.0f, _NilP, 0, _NilP, 0);
    GSetNode(2, false, 0.0f, acts2, 2, succ2, 2);
    GSetNode(3, true, 5.0f, _NilP, 0, _NilP, 0);
    GSetNode(4, true, 20.0f, _NilP, 0, _NilP, 0);

    ISC nodes_mm = 0;
    IUC best_mm = MinimaxAction(&game, &nodes_mm);
    GTCHECK("Minimax: picks action 0", best_mm == 0);
    GTCHECK("Minimax: evaluates all 5 nodes", nodes_mm == 5);

    ISC nodes_ab = 0;
    IUC best_ab = MinimaxAlphaBetaAction(&game, &nodes_ab);
    GTCHECK("AlphaBeta: picks action 0", best_ab == 0);
    GTCHECK("AlphaBeta: prunes to 4 nodes", nodes_ab == 4);
    GTCHECK("AlphaBeta: evaluates fewer nodes than minimax",
           nodes_ab < nodes_mm);
  }

  // --- Tree 2: optimal is action 1 (guards against a hardcoded 0).
  // root(MAX) -> {0: node1(terminal +3), 1: node2(terminal +7)}
  {
    IUC acts0[2] = {0, 1};
    IUC succ0[2] = {1, 2};
    GSetNode(0, false, 0.0f, acts0, 2, succ0, 2);
    GSetNode(1, true, 3.0f, _NilP, 0, _NilP, 0);
    GSetNode(2, true, 7.0f, _NilP, 0, _NilP, 0);

    ISC nodes_mm = 0;
    IUC best_mm = MinimaxAction(&game, &nodes_mm);
    GTCHECK("Minimax: picks action 1 when it is better", best_mm == 1);
    ISC nodes_ab = 0;
    IUC best_ab = MinimaxAlphaBetaAction(&game, &nodes_ab);
    GTCHECK("AlphaBeta: picks action 1 when it is better", best_ab == 1);
  }

  // --- Tree 3: root has no actions -> value 0, action 0 (documented).
  {
    GSetNode(0, false, 0.0f, _NilP, 0, _NilP, 0);
    ISC nodes_mm = 0;
    IUC best_mm = MinimaxAction(&game, &nodes_mm);
    GTCHECK("Minimax: no actions -> action 0", best_mm == 0);
    GTCHECK("Minimax: no actions -> exactly 1 node evaluated", nodes_mm == 1);
  }

  // --- Tree 4: tie -> the first action encountered wins.
  // root(MAX) -> {0: node1(+5), 1: node2(+5)}
  {
    IUC acts0[2] = {0, 1};
    IUC succ0[2] = {1, 2};
    GSetNode(0, false, 0.0f, acts0, 2, succ0, 2);
    GSetNode(1, true, 5.0f, _NilP, 0, _NilP, 0);
    GSetNode(2, true, 5.0f, _NilP, 0, _NilP, 0);
    ISC nodes_mm = 0;
    IUC best_mm = MinimaxAction(&game, &nodes_mm);
    GTCHECK("Minimax: tie -> first action (0)", best_mm == 0);
  }

  _DROut("game_pass=" << game_pass << " game_fail=" << game_fail << '\n');
  return game_fail ? "CLASSICAL_GAME_FAIL: " : _NilP;

#endif
  return _NilP;
}

}  //< namespace iGeek
