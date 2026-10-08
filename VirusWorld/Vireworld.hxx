// Copyright AStarship <https://astarship.net>.
#include "Vireworld.h"
#include "Host.h"
#include "VirusPopulation.h"
#include "Virus.h"
#include "Random.h"

namespace _ {

Vireworld::Vireworld()
  : host(NILP), population(NILP), frame(0),
    total_born(0), max_generation(0) {}

ISN Vireworld::Init(ISN w, ISN h, ISN pop) {
  if (w <= 0 || h <= 0 || pop <= 0) return 1;
  host = new Host(w, h);
  population = new VirusPopulation(pop);
  population->Init(w, h, pop);
  frame = 0;
  total_born = pop;
  max_generation = 0;
  return 0;
}

void Vireworld::Step() {
  if (IsError(host) || IsError(population)) return;
  frame++;
  IUC seed = IUC(frame * 7 + 13);
  population->Step(host, seed);
  ISN born = population->Reproduce(seed);
  total_born += born;
  population->Cull();
  // Track max generation.
  for (ISN i = 0; i < population->num_viruses; i++) {
    if (population->Get(i)->generation > max_generation) {
      max_generation = population->Get(i)->generation;
    }
  }
  // Clear and re-render.
  host->Clear();
  population->Render(host);
}

void Vireworld::Render() const {
  if (IsError(host)) return;
  host->Render();
}

void Vireworld::Summary() const {
  if (IsError(population)) return;
  ISN total_vertices = 0;
  for (ISN i = 0; i < population->num_viruses; i++) {
    total_vertices += population->Get(i)->VertexCount();
  }
  StdOut() << "\nFrame: " << frame
    << "  Population: " << population->Count()
    << "  Born: " << total_born
    << "  Gen: " << max_generation
    << "  Vertices: " << total_vertices;
}

}  //< namespace _
