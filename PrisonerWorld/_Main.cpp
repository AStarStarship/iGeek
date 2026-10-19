// Copyright AStarship <https://astarship.net>.
#include <iostream>
#include "PrisonerWorld.h"
using _::PrisonerEnv;
using _::PrisonerEnvPtr;
#if SEAM >= CRABS_PRISONERWORLD
/* Helper: run the simulation loop.
 * Kept in _ namespace to avoid symbol conflicts.
 */
namespace _ {
ISC RunSimulation(ISC episodes, PrisonerEnv* env) {
  // Simple reflex agents: always Cooperate
  IUD agent_actions[2] = {0, 0};  // Both default to Cooperate

  // Allocate reward/terminal arrays (Step writes to these)
  FPC rewards[2];
  BOL terminals[2];

  // Run episodes
  std::cout << "\nPrisoner's Dilemma Simulation";
  for (ISC ep = 0; ep < episodes; ++ep) {
    // Check if episode is done (history limit reached)
    if (env->Done()) {
      std::cout << "Episode " << ep << " done (history limit reached)\n";
      env->Reset();
    }
    // Step the environment
    env->Step(agent_actions, rewards, terminals);

    // Agents always Cooperate for this demo
    // In a full simulation, agents would learn from rewards

    // Print episode summary every 10 episodes
    if (ep % 10 == 0) {
      std::cout << "Episode " << ep << ", Round " << env->Round() << "\n";
    }
  }

  std::cout << "\nDone. Total episodes: " << episodes << "\n";
  return 0;
}
}  // namespace _

/* Global entry point - must be at global namespace for C runtime. */
int main(int args_count, CHA** args) {
  ISC episodes = 100;

  // Parse CLI args (simple opt parsing)
  for (ISC i = 1; i < args_count; ++i) {
    if (args[i][0] == '-' && args[i][1] == '-') {
      // --episodes N
      if (i + 1 < args_count) {
        episodes = ISC(args[++i][0] - '0');  // simplified
        ++i;
      }
      // --payoffs T R P S
      else if (i + 4 < args_count) {
        IUD T = IUD(args[++i][0] - '0');
        IUD R = IUD(args[++i][0] - '0');
        IUD P = IUD(args[++i][0] - '0');
        IUD S = IUD(args[++i][0] - '0');
        // Skip the remaining args
        i += 3;
      }
      // --seed N
      else if (i + 1 < args_count) {
        IUD seed = IUD(args[++i][0] - '0');
        ++i;
      }
    }
  }

  // Initialize the environment via the exported pointer
  PrisonerEnv* env = PrisonerEnvPtr;
  env->SetPayoffs(5, 3, 1, 0);  // default payoffs: T=5, R=3, P=1, S=0
  env->Reset();

  // Run the simulation
  return _::RunSimulation(episodes, env);
}
#endif  // SEAM >= CRABS_PRISONERWORLD