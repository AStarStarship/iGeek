// Copyright AStarship <https://astarship.net>.
#include "Host.h"

namespace _ {

Host::Host() : width(0), height(0), grid(NILP) {}

Host::Host(ISN w, ISN h) : width(w), height(h), grid(NILP) {
  if (width <= 0 || height <= 0) return;
  grid = new CHA[width * height];
  Clear();
}

void Host::Clear() {
  if (IsError(grid)) return;
  for (ISN i = 0; i < width * height; i++) grid[i] = '.';
}

BOL Host::InBounds(ISN x, ISN y) const {
  return x >= 0 && y >= 0 && x < width && y < height;
}

void Host::SetCell(ISN x, ISN y, CHA value) {
  if (!InBounds(x, y)) return;
  grid[y * width + x] = value;
}

CHA Host::GetCell(ISN x, ISN y) const {
  if (!InBounds(x, y)) return '.';
  return grid[y * width + x];
}

void Host::Render() const {
  if (IsError(grid)) return;
  for (ISN y = 0; y < height; y++) {
    for (ISN x = 0; x < width; x++) {
      StdOut() << grid[y * width + x];
    }
    StdOut() << '\n';
  }
}

}  //< namespace _
