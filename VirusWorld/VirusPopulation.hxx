// Copyright AStarship <https://astarship.net>.
#include "VirusPopulation.h"
#include "Virus.h"
#include "Host.h"
#include "Random.h"

namespace _ {

VirusPopulation::VirusPopulation()
  : width(0), height(0), num_viruses(0) {}

VirusPopulation::VirusPopulation(ISN size)
  : width(0), height(0), num_viruses(0) {
  (void)size;
}

void VirusPopulation::Init(ISN w, ISN h, ISN count) {
  width = w;
  height = h;
  num_viruses = 0;
  if (count > kMaxSize) count = kMaxSize;
  if (count < 1) count = 1;
  for (ISN i = 0; i < count; i++) {
    ISN x = Random(0, w - 1);
    ISN y = Random(0, h - 1);
    viruses[i] = Virus(x, y);
    num_viruses++;
  }
}

Virus* VirusPopulation::Get(ISN index) const {
  if (index < 0 || index >= num_viruses) return NILP;
  return const_cast<Virus*>(&viruses[index]);
}

void VirusPopulation::Step(Host* host, IUC seed) {
  for (ISN i = 0; i < num_viruses; i++) {
    viruses[i].Step(width, height, seed);
  }
}

void VirusPopulation::Render(Host* host) const {
  if (IsError(host)) return;
  for (ISN i = 0; i < num_viruses; i++) {
    viruses[i].Render(host);
  }
}

ISN VirusPopulation::Reproduce(IUC seed) {
  ISN born = 0;
  ISN i = 0;
  while (i < num_viruses) {
    if (viruses[i].age >= Virus::kReproduceEvery &&
        num_viruses < kMaxSize) {
      // Spawn a child with mutated DNA.
      ISN child_index = num_viruses;
      viruses[child_index] = viruses[i];  // Copy the parent.
      viruses[child_index].Mutate(seed);
      viruses[child_index].age = 0;
      viruses[child_index].generation = viruses[i].generation + 1;
      // Offset the child slightly.
      ISN dx = Random(-2, 2);
      ISN dy = Random(-2, 2);
      viruses[child_index].x = viruses[i].x + dx;
      viruses[child_index].y = viruses[i].y + dy;
      if (viruses[child_index].x < 0) viruses[child_index].x = 0;
      if (viruses[child_index].y < 0) viruses[child_index].y = 0;
      if (viruses[child_index].x >= width) viruses[child_index].x = width - 1;
      if (viruses[child_index].y >= height) viruses[child_index].y = height - 1;
      // Reset parent age.
      viruses[i].age = 0;
      num_viruses++;
      born++;
      // Don't re-check this parent; move on.
      i++;
    } else {
      i++;
    }
  }
  return born;
}

ISN VirusPopulation::Cull() {
  if (num_viruses <= kMaxSize) return 0;
  ISN culled = 0;
  while (num_viruses > kMaxSize) {
    // Find the oldest virus.
    ISN oldest = 0;
    for (ISN i = 1; i < num_viruses; i++) {
      if (viruses[i].age > viruses[oldest].age) oldest = i;
    }
    // Remove by shifting down.
    for (ISN i = oldest; i < num_viruses - 1; i++) {
      viruses[i] = viruses[i + 1];
    }
    num_viruses--;
    culled++;
  }
  return culled;
}

}  //< namespace _
