// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekPolygonWorldGeneticPolygonHXX
#define _iGeekPolygonWorldGeneticPolygonHXX
#include "GeneticPolygon.h"

namespace PolygonWorld {
using namespace ::_;
using namespace ::iGeek;

/* --- seeded LCG (Numerical Recipes constants) --------------------------------
 * Deterministic given a seed, so a GA run is reproducible and comparable to
 * an RL policy on the same objective. No stdlib, no thread-local state. */
ISC AGenomePopulation::rand32() {
  rand_state_ = rand_state_ * 636413622326ULL + 1442695041ULL;
  return ISC((rand_state_ >> 32) & 0x7FFFFFFF);
}

FPC AGenomePopulation::rand01() {
  return FPC(ISC(rand_state_ >> 1) & 0x7FFFFFFF) / FPC(0x7FFFFFFF);
}

AGenomePopulation::AGenomePopulation(IUD seed) {
  Reset(seed);
}

void AGenomePopulation::Reset(IUD seed) {
  seed_ = seed;
  rand_state_ = seed ? seed : 1;
  generation_ = 0;
  best_fitness_ = 0.0f;
  last_fitness_ = 0.0f;
  best_index_ = 0;
  Randomize();
}

TGenome AGenomePopulation::RandomGenome() {
  TGenome g;
  for (ISC i = 0; i < PolygonWorldPoints; ++i) {
    g.x_[i] = FPC(rand01() * 2.0 - 1.0);
    g.y_[i] = FPC(rand01() * 2.0 - 1.0);
  }
  return g;
}

void AGenomePopulation::Randomize() {
  for (ISC i = 0; i < PolygonWorldPopulation; ++i) {
    pop_[i] = RandomGenome();
    fitness_[i] = 0.0f;
  }
  best_ = pop_[0];
  best_index_ = 0;
  best_fitness_ = 0.0f;
}

/* Inline Newton-Raphson sqrt — no libm. 6 iterations is float-precision. */
static inline FPC TGA_Sqrt(FPC v) {
  if (v <= FPC(0.0)) return FPC(0.0);
  FPC x = v;
  for (ISC i = 0; i < 6; ++i)
    x = FPC((x + v / x) * 0.5);
  return x;
}

/* Inline sin/cos via Taylor (8 terms — plenty for float). a is in radians,
 * assumed in [0, 2*pi). */
static inline FPC TGA_Cos(FPD a) {
  FPD a2 = a * a;
  return FPD(1.0) - a2 / FPD(2.0) + a2 * a2 / FPD(24.0) - a2 * a2 * a2 / FPD(720.0);
}
static inline FPC TGA_Sin(FPD a) {
  FPD a2 = a * a;
  return a - a * a2 / FPD(6.0) + a * a2 * a2 / FPD(120.0) - a * a2 * a2 * a2 / FPD(5040.0);
}

/* Fitness: 1.0 - normalized mean point distance to the target. The genome is
 * a closed polygon; we compare point-by-point in order (rotation is not
 * factored out; the target is presented in the same winding). */
FPC AGenomePopulation::Fitness(TGenome& target) const {
  FPD sum = 0.0;
  for (ISC i = 0; i < PolygonWorldPoints; ++i) {
    FPD dx = FPD(target.x_[i]) - FPD(best_.x_[i]);
    FPD dy = FPD(target.y_[i]) - FPD(best_.y_[i]);
    sum += FPD(TGA_Sqrt(FPC(dx * dx + dy * dy)));
  }
  /* Max mean distance across the square is sqrt(8); normalize to [0, 1]. */
  FPD norm = sum / FPD(PolygonWorldPoints) / FPD(2.8284271247461903);
  if (norm > 1.0) norm = 1.0;
  return FPC(1.0 - norm);
}

FPC AGenomePopulation::PopulationFitness(TGenome& target) const {
  FPD sum = 0.0;
  for (ISC i = 0; i < PolygonWorldPopulation; ++i)
    sum += FPD(fitness_[i] > 0.0f ? fitness_[i] : Fitness(target));
  return FPC(sum / FPD(PolygonWorldPopulation));
}

/* Tournament selection: pick PolygonWorldTournament random individuals,
 * return the one with the best fitness against the target. */
ISC AGenomePopulation::SelectIndex(TGenome& target) {
  ISC best = ISC((rand32() & 0xFFFF) % PolygonWorldPopulation);
  FPC best_fit = Fitness(target);
  for (ISC k = 1; k < PolygonWorldTournament; ++k) {
    ISC cand = ISC((rand32() & 0xFFFF) % PolygonWorldPopulation);
    FPC fit = Fitness(target);
    if (fit > best_fit) {
      best = cand;
      best_fit = fit;
    }
  }
  return best;
}

/* Point-wise crossover: each control point comes from the mother or the
 * father, chosen independently. */
TGenome AGenomePopulation::Crossover(TGenome& mother, TGenome& father) {
  TGenome child;
  for (ISC i = 0; i < PolygonWorldPoints; ++i) {
    BOL from_mother = (rand32() & 1) != 0;
    child.x_[i] = from_mother ? mother.x_[i] : father.x_[i];
    child.y_[i] = from_mother ? mother.y_[i] : father.y_[i];
  }
  return child;
}

/* Bounded mutation: nudge each point by a small random delta, clamped to
 * the unit square. */
TGenome AGenomePopulation::Mutate(TGenome& parent) {
  TGenome child = parent;
  const FPC step = FPC(0.15);
  for (ISC i = 0; i < PolygonWorldPoints; ++i) {
    child.x_[i] = child.x_[i] + FPC((rand01() * 2.0 - 1.0) * step);
    child.y_[i] = child.y_[i] + FPC((rand01() * 2.0 - 1.0) * step);
    if (child.x_[i] > 1.0f) child.x_[i] = 1.0f;
    if (child.x_[i] < -1.0f) child.x_[i] = -1.0f;
    if (child.y_[i] > 1.0f) child.y_[i] = 1.0f;
    if (child.y_[i] < -1.0f) child.y_[i] = -1.0f;
  }
  return child;
}

/* One generation: evaluate, track the best-so-far, keep the elite, fill the
 * rest with crossover+mutation children. */
FPC AGenomePopulation::Evolve() {
  for (ISC i = 0; i < PolygonWorldPopulation; ++i)
    fitness_[i] = Fitness(best_);
  ISC new_best = 0;
  FPC new_best_fit = fitness_[0];
  for (ISC i = 1; i < PolygonWorldPopulation; ++i) {
    if (fitness_[i] > new_best_fit) {
      new_best = i;
      new_best_fit = fitness_[i];
    }
  }
  if (new_best_fit > best_fitness_) {
    best_ = pop_[new_best];
    best_fitness_ = new_best_fit;
    best_index_ = new_best;
  }
  last_fitness_ = PopulationFitness(best_);
  ++generation_;
  TGenome next[PolygonWorldPopulation];
  next[0] = best_;
  next[1] = pop_[new_best == 0 ? 1 : 0];
  for (ISC i = PolygonWorldElitism; i < PolygonWorldPopulation; ++i) {
    ISC m = SelectIndex(best_);
    ISC f = SelectIndex(best_);
    TGenome child = Crossover(pop_[m], pop_[f]);
    if (rand01() < FPC(0.2)) child = Mutate(child);
    next[i] = child;
  }
  for (ISC i = 0; i < PolygonWorldPopulation; ++i) pop_[i] = next[i];
  for (ISC i = 0; i < PolygonWorldPopulation; ++i) fitness_[i] = 0.0f;
  return last_fitness_;
}

void AGenomePopulation::Observe(FPC* obs) const {
  for (ISC i = 0; i < PolygonWorldPoints; ++i) {
    obs[i * 2] = best_.x_[i];
    obs[i * 2 + 1] = best_.y_[i];
  }
}

/* --- APolygonEnv ------------------------------------------------------------- */
APolygonEnv::APolygonEnv(IUD seed)
    : Environment("polygonworld", 2), seed_(seed) {
  TSPrint(manifest_.name, ISW(ManifestNameLen), "polygonworld");
  TSPrint(manifest_.version, ISW(ManifestVersionLen), "1.0.0");
  manifest_.kind = ManifestKindEnv;
  manifest_.domain = ManifestDomainGame;
  TSPrint(manifest_.description, ISW(ManifestDescriptionLen),
          "Headless GA polygon shape-matching vs. a target");
  TSPrint(manifest_.modality, ISW(ManifestModalityLen), "vector");
  manifest_.reward_low = FPC(-1.0);
  manifest_.reward_high = FPC(1.0);
  manifest_.reward_higher_is_better = true;
  manifest_.determinism = ManifestDeterminismSeeded;
  manifest_.observation_count = ISC(PolygonWorldObservationCount);
  manifest_.action_count = ISC(PolygonWorldActionCount);
  manifest_.max_episode_steps = ISC(PolygonWorldMaxSteps);
  manifest_.environment_count_max = ISC(GymEnvMax);
  TSPrint(manifest_.world_name, ISW(ManifestWorldLen), "PolygonWorld");
  manifest_.seam_number = ISC(46);
  manifest_.manifest_version = ISC(1);
  pop_ = AGenomePopulation(seed);
  /* Default target: a regular 16-gon, radius 0.8, in the unit square. */
  {
    const FPD two_pi = FPD(6.283185307179586);
    for (ISC i = 0; i < PolygonWorldPoints; ++i) {
      FPD a = FPD(i) / FPD(PolygonWorldPoints) * two_pi;
      target_.x_[i] = FPC(FPD(0.8) * TGA_Cos(a));
      target_.y_[i] = FPC(FPD(0.8) * TGA_Sin(a));
    }
  }
  prev_best_fitness_ = 0.0f;
  steps_ = 0;
  done_ = false;
  Observe();
}

void APolygonEnv::Reset() {
  pop_.Reset(seed_);
  {
    const FPD two_pi = FPD(6.283185307179586);
    for (ISC i = 0; i < PolygonWorldPoints; ++i) {
      FPD a = FPD(i) / FPD(PolygonWorldPoints) * two_pi;
      target_.x_[i] = FPC(FPD(0.8) * TGA_Cos(a));
      target_.y_[i] = FPC(FPD(0.8) * TGA_Sin(a));
    }
  }
  prev_best_fitness_ = 0.0f;
  steps_ = 0;
  done_ = false;
  Observe();
}

FPC APolygonEnv::Step(ISC action) {
  if (done_) return FPC(0.0);
  switch (action) {
    case 1:
    case 2:
    default:
      /* Advance one generation (actions 1 and 2 are aliases of the default
         evolve step; a future seam can specialise them). */
      pop_.Evolve();
      break;
    case 3:
      /* Re-evaluate only; no generational advance. */
      pop_.PopulationFitness(target_);
      break;
  }
  ++steps_;
  FPC cur = pop_.BestFitness();
  FPC reward = cur - prev_best_fitness_;
  if (reward > 1.0f) reward = 1.0f;
  if (reward < -1.0f) reward = -1.0f;
  prev_best_fitness_ = cur;
  if (cur >= FPC(PolygonWorldConvergeNumerator) / FPC(PolygonWorldConvergeDenominator) || steps_ >= ISC(PolygonWorldMaxSteps))
    done_ = true;
  Observe();
  return reward;
}

void APolygonEnv::Observe() {
  pop_.Observe(observation_);
}

void APolygonEnv::SetTarget(TGenome& target) {
  target_ = target;
  pop_.Reset(seed_);
  prev_best_fitness_ = 0.0f;
  steps_ = 0;
  done_ = false;
  Observe();
}

}  //< namespace PolygonWorld
#endif  //< _iGeekPolygonWorldGeneticPolygonHXX
