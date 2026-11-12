# Copyright AStarship <https://astarship.net>.
"""PolygonWorld GA — Python port used by the browser demo.

WHY A PORT: the surviving native AGenomePopulation (GeneticPolygon.h/.hxx)
compiles through polygon_bridge.py but cannot evolve. Measured, not assumed:

  1. rand01() never advances rand_state_ (only rand32() does), so Randomize()
     fills every control point of every genome with the same value; the whole
     population is one degenerate point at seed-dependent coordinates.
  2. Evolve() calls Fitness(best_), passing the best genome as the *target*, so
     every fitness value is a self-comparison and is always 1.0.
  3. SelectIndex() calls Fitness(target) for every tournament candidate, so all
     candidates compare equal and the "tournament" is a uniform random pick.

Net effect measured on the compiled native bridge: best_fitness == 1.0 from
generation 1 onward and the population is byte-identical after any number of
generations. That is documented by test_polygon.py::test_native_ga_is_inert.

This module is a faithful port of the *documented* GA semantics in
GeneticPolygon.h (16 control points in [-1,1]^2, 32 genomes, elitism 2,
tournament 3, point-wise crossover, bounded mutation step 0.15, 20% mutation
rate, fitness = 1 - mean point distance / sqrt(8), converge at 0.99 / 200
generations, reward = delta of best fitness) with exactly those three defects
corrected. It is NOT native code and the demo labels it as a port.
"""
import math
import random

Points = 16
Population = 32
Elitism = 2
Tournament = 3
MaxGenerations = 200
MutateRate = 0.2
MutateStep = 0.15
Converge = 0.99
MaxDistance = math.sqrt(8.0)

TargetNames = ("circle", "square", "triangle", "star")


def TargetPoints(name):
  """Target polygon as 16 (x, y) points in [-1, 1]^2, in winding order."""
  if name == "circle":
    return [(0.8 * math.cos(2 * math.pi * i / Points),
             0.8 * math.sin(2 * math.pi * i / Points)) for i in range(Points)]
  if name == "star":
    return [((0.9 if i % 2 == 0 else 0.4) * math.cos(2 * math.pi * i / Points),
             (0.9 if i % 2 == 0 else 0.4) * math.sin(2 * math.pi * i / Points))
            for i in range(Points)]
  if name == "square":
    return _Perimeter(4, 0.82)
  if name == "triangle":
    return _Perimeter(3, 0.85)
  raise ValueError("Unknown target shape.")


def _Perimeter(sides, radius):
  """Sample `sides` corners evenly around the unit circle, `Points` samples."""
  corners = [(radius * math.cos(2 * math.pi * k / sides + math.pi / 2),
              radius * math.sin(2 * math.pi * k / sides + math.pi / 2))
             for k in range(sides)]
  out = []
  for i in range(Points):
    t = i / Points * sides
    k = int(t)
    f = t - k
    x0, y0 = corners[k % sides]
    x1, y1 = corners[(k + 1) % sides]
    out.append((x0 + (x1 - x0) * f, y0 + (y1 - y0) * f))
  return out


def GenomeFitness(genome, target):
  total = 0.0
  for (x, y), (tx, ty) in zip(genome, target):
    total += math.hypot(tx - x, ty - y)
  norm = total / Points / MaxDistance
  return max(0.0, min(1.0, 1.0 - norm))


class AGaPopulation:
  def __init__(self, seed=1, target="circle"):
    self.seed = seed
    self.rng = random.Random(seed)
    self.target_name = target
    self.target = TargetPoints(target)
    self.Reset(seed, target)

  def Reset(self, seed=None, target=None):
    if seed is not None:
      self.seed = seed
      self.rng = random.Random(seed)
    if target is not None:
      self.target_name = target
      self.target = TargetPoints(target)
    self.generation = 0
    self.done = False
    self.population = [self._RandomGenome() for _ in range(Population)]
    self.best = list(self.population[0])
    self.best_fitness = GenomeFitness(self.best, self.target)
    self.last_reward = 0.0
    self._RefreshBest()
    self.done = self.best_fitness >= Converge

  def _RandomGenome(self):
    return [(self.rng.uniform(-1.0, 1.0), self.rng.uniform(-1.0, 1.0))
            for _ in range(Points)]

  def _RefreshBest(self):
    scored = sorted(((GenomeFitness(g, self.target), i)
                     for i, g in enumerate(self.population)), reverse=True)
    top_fitness, top_index = scored[0]
    if top_fitness > self.best_fitness or self.generation == 0:
      self.best_fitness = top_fitness
      self.best = list(self.population[top_index])

  def _Tournament(self):
    best_index = self.rng.randrange(Population)
    best_fit = GenomeFitness(self.population[best_index], self.target)
    for _ in range(Tournament - 1):
      index = self.rng.randrange(Population)
      fit = GenomeFitness(self.population[index], self.target)
      if fit > best_fit:
        best_index, best_fit = index, fit
    return best_index

  def _Crossover(self, mother, father):
    return [(mother[i] if self.rng.random() < 0.5 else father[i])
            for i in range(Points)]

  def _Mutate(self, genome):
    out = []
    for x, y in genome:
      if self.rng.random() < MutateRate:
        x += self.rng.uniform(-MutateStep, MutateStep)
        y += self.rng.uniform(-MutateStep, MutateStep)
      out.append((max(-1.0, min(1.0, x)), max(-1.0, min(1.0, y))))
    return out

  def Evolve(self, generations=1):
    if self.done:
      return 0.0
    previous = self.best_fitness
    for _ in range(generations):
      if self.done:
        break
      self._RefreshBest()
      ranked = sorted(range(Population),
                      key=lambda i: GenomeFitness(self.population[i], self.target),
                      reverse=True)
      next_population = [list(self.population[i]) for i in ranked[:Elitism]]
      while len(next_population) < Population:
        mother = self.population[self._Tournament()]
        father = self.population[self._Tournament()]
        next_population.append(self._Mutate(self._Crossover(mother, father)))
      self.population = next_population
      self.generation += 1
      self._RefreshBest()
      if self.best_fitness >= Converge or self.generation >= MaxGenerations:
        self.done = True
    self.last_reward = self.best_fitness - previous
    return self.last_reward

  def State(self):
    return {
        "generation": self.generation,
        "max_generations": MaxGenerations,
        "done": self.done,
        "seed": self.seed,
        "target_name": self.target_name,
        "best_fitness": round(self.best_fitness, 6),
        "last_reward": round(self.last_reward, 6),
        "best": [round(v, 5) for point in self.best for v in point],
        "target": [round(v, 5) for point in self.target for v in point],
        "population": [[round(v, 5) for point in genome for v in point]
                       for genome in self.population],
        "message": self._Message(),
    }

  def _Message(self):
    if self.best_fitness >= Converge:
      return "Converged: best fitness reached 0.99 (port GA)."
    if self.generation >= MaxGenerations:
      return "Generation budget exhausted (port GA)."
    return "Port GA generation %d of %d." % (self.generation, MaxGenerations)
