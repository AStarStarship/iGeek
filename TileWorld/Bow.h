// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _TileWorldBowH
#define _TileWorldBowH
#include <_Config.h>
//
#include "WeaponRanged.h"
namespace TileWorld {

class WeaponRanged;

class Bow : public WeaponRanged {
 public:
  Bow();

  virtual ~Bow();
};
}  //< namespace TileWorld
#endif
