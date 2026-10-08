// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef VIREWWORLD_HOST_H
#define VIREWWORLD_HOST_H

#include <_Config.h>

namespace _ {

/* Host is the simulation grid. It owns the cells that viruses swim through
and provides the ASCII render surface. A Host is a terminal-only concept:
there is no GUI, no window, no pixel buffer. The host IS the terminal.
*/
class Host {
 public:
  Host();
  explicit Host(ISN width, ISN height);

  ISN width;     //< Grid width in cells (columns).
  ISN height;    //< Grid height in cells (rows).
  CHA* grid;     //< Cell buffer, width * height CHA. Row-major.

  void SetCell(ISN x, ISN y, CHA value);
  CHA GetCell(ISN x, ISN y) const;
  BOL InBounds(ISN x, ISN y) const;
  void Clear();
  void Render() const;

 private:
  Host(const Host&);              //< No copy.
  Host& operator=(const Host&);   //< No assign.
};

}  //< namespace _

#endif  // VIREWWORLD_HOST_H
