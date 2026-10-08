// Copyright AStarship <https://astarship.net>.
#include "Virus.h"
#include "Host.h"
#include "Random.h"

namespace _ {

/* Nucleotide radial values: . = 0, - = 1, | = 2, + = 3, * = 4. */
static const CHA kNucleotides[] = {'.', '-', '|', '+', '*'};
static const ISN kNucleotideCount = 5;

Virus::Virus()
  : x(0), y(0), phase(0), speed(kMinSpeed), age(0), generation(0) {
  for (ISN i = 0; i < kDnaLength; i++) dna[i] = '.';
}

Virus::Virus(ISN init_x, ISN init_y)
  : x(init_x), y(init_y), phase(Random(0, 360)),
    speed(Random(kMinSpeed, kMaxSpeed)), age(0), generation(0) {
  for (ISN i = 0; i < kDnaLength; i++) dna[i] = '.';
}

ISN Virus::Radial(CHA nucleotide) const {
  for (ISN i = 0; i < kNucleotideCount; i++) {
    if (nucleotide == kNucleotides[i]) return i;
  }
  return 0;
}

ISN Virus::VertexCount() const {
  ISN count = 0;
  for (ISN i = 0; i < kDnaLength; i++) {
    if (Radial(dna[i]) >= 2) count++;
  }
  return count;
}

ISN Virus::Radius() const {
  ISN max_radial = 0;
  for (ISN i = 0; i < kDnaLength; i++) {
    ISN r = Radial(dna[i]);
    if (r > max_radial) max_radial = r;
  }
  // Radius 1 for a plain circle, 2 for spiky, 3 for very spiky.
  if (max_radial >= 3) return 3;
  if (max_radial >= 2) return 2;
  return 1;
}

ISN Virus::Mutate(IUC seed) {
  if (kDnaLength <= 0) return -1;
  ISN index = ISN(seed % kDnaLength);
  CHA old = dna[index];
  ISN old_radial = Radial(old);
  // Mutate to an adjacent radial (up or down), not the same.
  ISN delta = (seed % 2 == 0) ? 1 : -1;
  ISN new_radial = old_radial + delta;
  if (new_radial < 0) new_radial = 0;
  if (new_radial >= kNucleotideCount) new_radial = kNucleotideCount - 1;
  dna[index] = kNucleotides[new_radial];
  return index;
}

void Virus::Step(ISN host_width, ISN host_height, IUC seed) {
  age++;
  phase++;
  if (phase >= 360) phase -= 360;

  // Lissajous drift: x oscillates at 1x, y at 2x.
  ISN amplitude_x = (host_width / 4) - 1;
  ISN amplitude_y = (host_height / 4) - 1;
  if (amplitude_x < 1) amplitude_x = 1;
  if (amplitude_y < 1) amplitude_y = 1;

  // Phase in degrees -> coarse sine lookup (4 quadrants).
  ISN px = phase % 180;
  if (px > 90) px = 180 - px;
  ISN py = (phase * 2) % 180;
  if (py > 90) py = 180 - py;

  // Sine approximation: 0,25,50,75,100 (percentage of amplitude).
  ISN sine_table[] = {0, 25, 50, 75, 100};
  ISN sx = sine_table[px / 45];
  ISN sy = sine_table[py / 45];

  ISN new_x = host_width / 2 + sx * amplitude_x / 100 - speed / 2;
  ISN new_y = host_height / 2 + sy * amplitude_y / 100 - speed / 2;

  // Bounce off walls.
  if (new_x < 0) { new_x = 0; phase = -phase; }
  if (new_x >= host_width) { new_x = host_width - 1; phase = -phase; }
  if (new_y < 0) { new_y = 0; phase = -phase; }
  if (new_y >= host_height) { new_y = host_height - 1; phase = -phase; }

  x = new_x;
  y = new_y;
}

void Virus::Render(Host* host) const {
  if (IsError(host)) return;
  ISN r = Radius();
  if (r <= 1) {
    // Single cell organism.
    host->SetCell(x, y, 'o');
    return;
  }
  // Multi-cell organism: draw a plus/cross shape.
  host->SetCell(x, y, 'O');  // Core cell.
  for (ISN i = 0; i < kDnaLength; i++) {
    ISN radial = Radial(dna[i]);
    if (radial < 2) continue;
    // Each spike nucleotide gets a direction (i / 4 quadrants).
    ISN angle = i * 22;  // 16 nucleotides * 22 = 352 ~ 360.
    // Coarse direction: 8-way.
    ISN octant = (angle / 45) % 8;
    ISN dx = 0, dy = 0;
    switch (octant) {
      case 0: dx =  r; dy = 0;  break;  // East
      case 1: dx =  r; dy = -r; break;  // NE
      case 2: dx = 0; dy = -r;  break;  // North
      case 3: dx = -r; dy = -r; break;  // NW
      case 4: dx = -r; dy = 0;  break;  // West
      case 5: dx = -r; dy =  r; break;  // SW
      case 6: dx = 0; dy =  r;  break;  // South
      case 7: dx =  r; dy =  r; break;  // SE
    }
    CHA spike_char = (radial >= 4) ? '*' : '+';
    host->SetCell(x + dx, y + dy, spike_char);
    // Intermediate cell for larger radii.
    if (r >= 2) {
      host->SetCell(x + dx / 2, y + dy / 2, '|');
    }
  }
}

}  //< namespace _
