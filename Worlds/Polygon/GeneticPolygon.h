// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekPolygonWorldGeneticPolygonH
#define _iGeekPolygonWorldGeneticPolygonH
#include "../../Gym.h"
#include "../../EnvironmentManifest.h"

/* Headless genetic-algorithm polygon. No Java, no Graphics, no bitmap.
 *
 * A genetic polygon is a closed set of control points in the unit
 * [-1, 1]^2 square. The shape it traces is its genome: the population is
 * evolved by selection, crossover, and mutation, and fitness is measured
 * against a target shape. The whole thing is deterministic given a seed, so a
 * run is reproducible and comparable to an RL policy trained on the same
 * objective. */
namespace PolygonWorld {
using namespace ::_;
using namespace ::iGeek;

enum {
  PolygonWorldPoints = 16,          //< control points per polygon (closed loop)
  PolygonWorldPopulation = 32,      //< population size
  PolygonWorldMaxGenerations = 200,
  PolygonWorldElitism = 2,          //< top-N copied verbatim each generation
  PolygonWorldTournament = 3,       //< tournament size for selection
  PolygonWorldObservationCount = PolygonWorldPoints * 2,  //< x,y of every point
  PolygonWorldActionCount = 4,      //< 0 none, 1 mutate, 2 crossover, 3 evaluate
  PolygonWorldMaxSteps = PolygonWorldMaxGenerations,
  PolygonWorldConvergeNumerator = 99,  //< 0.99 as a fraction (avoids a float in the enum)
  PolygonWorldConvergeDenominator = 100
};

/* A single polygon genome: PolygonWorldPoints (x, y) control points. */
struct TGenome {
  FPC x_[PolygonWorldPoints] = {};
  FPC y_[PolygonWorldPoints] = {};
};

/* The GA population. Owns the genome slots and the fitness bookkeeping. */
class AGenomePopulation {
 public:
  AGenomePopulation(IUD seed = 1);
  void Reset(IUD seed);
  /* Fill the population with random polygons. */
  void Randomize();
  /* One generational step: select, crossover, mutate. Returns the
     generation's best fitness. */
  FPC Evolve();
  /* Fitness of one genome against a target polygon (1.0 = identical). */
  FPC Fitness(TGenome& target) const;
  /* Fitness of the whole population (mean of per-genome fitness). */
  FPC PopulationFitness(TGenome& target) const;
  /* The best genome so far. */
  const TGenome& Best() const { return best_; }
  ISC BestIndex() const { return best_index_; }
  FPC BestFitness() const { return best_fitness_; }
  FPC LastGenerationFitness() const { return last_fitness_; }
  ISC Generation() const { return generation_; }
  /* Write the best genome out as the observation vector (x,y interleaved). */
  void Observe(FPC* obs) const;
  IUD Seed() const { return seed_; }
 private:
  /* Seeded LCG; the only source of randomness so runs are reproducible. */
  IUD rand_state_;
  ISC rand32();
  FPC rand01();
  TGenome RandomGenome();
  TGenome Crossover(TGenome& mother, TGenome& father);
  TGenome Mutate(TGenome& parent);
  ISC SelectIndex(TGenome& target);
  IUD seed_;
  ISC generation_;
  TGenome pop_[PolygonWorldPopulation];
  FPC fitness_[PolygonWorldPopulation] = {};
  TGenome best_;
  FPC best_fitness_ = 0.0f;
  ISC best_index_ = 0;
  FPC last_fitness_ = 0.0f;
};

/* The Environment: owns the population and exposes the GA contract to
 * agents and the manifest. Step action semantics:
 *   0 = advance one generation (the default, what an agent does per frame)
 *   1 = force a mutation pass on the current generation
 *   2 = force a crossover pass (re-derive children from the top pairs)
 *   3 = re-evaluate fitness against the stored target
 * Reward: delta of best fitness between steps (positive = progress).
 * Terminal: best fitness reaches PolygonWorldConverge (0.99) or the step
 * budget is exhausted. */
class APolygonEnv : public Environment {
 public:
  APolygonEnv(IUD seed = 1);
  void Reset();
  FPC Step(ISC action);
  void Observe();
  BOL Done() { return done_; }
  ISC Steps() { return steps_; }
  const FPC* Observation() { return observation_; }
  AGenomePopulation& Population() { return pop_; }
  const TGenome& Target() const { return target_; }
  void SetTarget(TGenome& target);
 private:
  AGenomePopulation pop_;
  TGenome target_;
  FPC observation_[PolygonWorldObservationCount] = {};
  FPC prev_best_fitness_ = 0.0f;
  ISC steps_ = 0;
  BOL done_ = false;
  IUD seed_;
};
}  //< namespace PolygonWorld
#endif  //< _iGeekPolygonWorldGeneticPolygonH
