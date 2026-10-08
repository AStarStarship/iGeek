// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef VIREWWORLD_VIRUSPOPULATION_H
#define VIREWWORLD_VIRUSPOPULATION_H

#include <_Config.h>

namespace _ {

class Virus;
class Host;

/* VirusPopulation owns the viruses in a Host. It handles step, reproduce,
and render. Reproduction: a virus whose age >= kReproduceEvery spawns a
child with a mutated copy of its DNA, incrementing the generation.
*/
class VirusPopulation {
 public:
  static const ISN kDefaultSize = 8;   //< Starting population.
  static const ISN kMaxSize = 64;      //< Max viruses before culling.

  VirusPopulation();
  explicit VirusPopulation(ISN size);

  void Init(ISN width, ISN height, ISN count);
  ISN Count() const { return num_viruses; }
  Virus* Get(ISN index) const;

  /* Steps all viruses one frame. */
  void Step(Host* host, IUC seed);

  /* Renders all viruses onto the host grid. */
  void Render(Host* host) const;

  /* Attempts reproduction: each virus that has lived kReproduceEvery frames
  spawns a child (if under kMaxSize). Returns the number of children born. */
  ISN Reproduce(IUC seed);

  /* Culls the oldest viruses if over kMaxSize. Returns number culled. */
  ISN Cull();

  ISN width;    //< Host width at init time.
  ISN height;   //< Host height at init time.
  ISN num_viruses;  //< Current population count.

 private:
  Virus viruses[kMaxSize];  //< Virus array (value semantics).
  VirusPopulation(const VirusPopulation&);        //< No copy.
  VirusPopulation& operator=(const VirusPopulation&);  //< No assign.
};

}  //< namespace _

#endif  // VIREWWORLD_VIRUSPOPULATION_H
