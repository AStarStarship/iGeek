// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef VIREWWORLD_VIREWWORLD_H
#define VIREWWORLD_VIREWWORLD_H

#include <_Config.h>

namespace _ {

class Host;
class VirusPopulation;

/* Vireworld is the top-level simulation: a Host grid with a
VirusPopulation swimming through it. Step advances one frame;
Render draws the current state to the terminal.
*/
class Vireworld {
 public:
  Vireworld();

  /* Initializes the world with a grid of the given size and a starting
  population. Returns an error code (0 = success). */
  ISN Init(ISN width, ISN height, ISN population);

  /* Advances the simulation one frame: step viruses, reproduce, cull. */
  void Step();

  /* Renders the current state to the terminal. */
  void Render() const;

  /* Prints a summary: population, generations, vertex counts. */
  void Summary() const;

  Host* host;          //< Grid surface.
  VirusPopulation* population;  //< Viruses.
  ISN frame;           //< Frame counter.
  ISN total_born;      //< Total viruses ever born.
  ISN max_generation;  //< Highest generation seen.

 private:
  Vireworld(const Vireworld&);        //< No copy.
  Vireworld& operator=(const Vireworld&);  //< No assign.
};

}  //< namespace _

#endif  // VIREWWORLD_VIREWWORLD_H
