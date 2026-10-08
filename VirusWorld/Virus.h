// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef VIREWWORLD_VIRUS_H
#define VIREWWORLD_VIRUS_H

#include <_Config.h>

namespace _ {

/* Virus is a genetic polygon virus. Its DNA is an ASCII string; its shape
is derived from that DNA. Each nucleotide is a character that contributes a
vertex offset to the polygon. The virus drifts on a bounded Lissajous path
and reproduces by copying its DNA with mutation.

DNA alphabet (each character encodes one radial segment):
  .  radial = 0      (flat, no protrusion)
  -  radial = 1/4
  |  radial = 1/2
  +  radial = 3/4
  *  radial = 1      (full protrusion)

A virus is the single character 'o' when its DNA is all '.' (a circle with
no spikes). As spikes emerge through mutation, the shape becomes polygonal.
*/
class Virus {
 public:
  static const ISN kDnaLength = 16;  //< Nucleotides per genome.
  static const ISN kMaxSpeed = 3;    //< Max drift cells/frame.
  static const ISN kMinSpeed = 1;    //< Min drift cells/frame.
  static const ISN kReproduceEvery = 20;  //< Frames between divisions.

  Virus();
  Virus(ISN x, ISN y);

  /* Mutates a random nucleotide in the DNA. Returns the index mutated,
  or -1 if the DNA is empty. */
  ISN Mutate(IUC seed);

  /* Computes the polygon vertex count from the DNA. Spikes (radial >= 1/2)
  contribute one vertex each; flat segments contribute none. */
  ISN VertexCount() const;

  /* Radius in cells (1 = single cell dot, larger = bigger organism). */
  ISN Radius() const;

  /* Drifts the virus one frame on its Lissajous path. */
  void Step(ISN host_width, ISN host_height, IUC seed);

  /* Renders the virus onto the host grid. */
  void Render(Host* host) const;

  ISN x;    //< Current x position (grid column).
  ISN y;    //< Current y position (grid row).
  ISN phase;  //< Lissajous phase (0..360, one degree per tick).
  ISN speed;  //< Drift speed (kMinSpeed..kMaxSpeed).
  ISN age;    //< Frames since birth.
  ISN generation;  //< Generation number (0 = founder).
  CHA dna[kDnaLength];  //< Genome: ASCII nucleotide string.

 private:
  ISN Radial(CHA nucleotide) const;
};

}  //< namespace _

#endif  // VIREWWORLD_VIRUS_H
