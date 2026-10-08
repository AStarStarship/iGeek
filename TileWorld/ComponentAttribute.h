// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef TILEWORLD_COMPONENTATTRIBUTE_DECL
#define TILEWORLD_COMPONENTATTRIBUTE_DECL
#include <_Config.h>
namespace TileWorld {

class AttributeComponent {
 public:
  ISN level,             //<
      exp,               //<
      exp_next,          //<
      attribute_points,  //<
      vitality,          //<
      strength,          //<
      dexterity,         //<
      agility,           //<
      intelligence,      //<
      hp,                //<
      hp_max,            //<
      damage_min,        //<
      damage_max,        //<
      accuracy,          //<
      defence,           //<
      luck;

  AttributeComponent(ISN level);
  virtual ~AttributeComponent();

  _::AString DebugPrint() const;

  void LoseHP(const ISN hp);
  void GainHP(const ISN hp);
  void LoseEXP(const ISN exp);
  void GainExp(const ISN exp);

  void UpdateStats(const bool reset);
  void UpdateLevel();

  void Update();
};
}  //< namespace TileWorld
#endif
