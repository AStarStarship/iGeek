// Copyright AStarship <https://astarship.net>.
#include "stdafx.h"
//
#include "component_attribute.h"

AttributeComponent::AttributeComponent(int level) {
  level = level;
  exp = 0;
  next_exp =
      static_cast<int>((50 / 3) * (pow(level + 1, 3) - 6 * pow(level + 1, 2) +
                                   ((level + 1) * 17) - 12));
  attribute_points = 2;

  vitality = 1;
  strength = 1;
  dexterity = 1;
  agility = 1;
  intelligence = 1;

  UpdateLevel();
  UpdateStats(true);
}

AttributeComponent::~AttributeComponent() {}

_::AString AttributeComponent::DebugPrint() const {
  std::stringstream ss;

  ss << "Level: " << level << "\n"
     << "Exp: " << exp << "\n"
     << "Exp Next: " << next_exp << "\n"
     << "Attp: " << attribute_points << "\n";

  return ss.str();
}

void AttributeComponent::LoseHP(const int hp) {
  hp_ -= hp;

  if (hp < 0) hp_ = 0;
}

void AttributeComponent::GainHP(const int hp) {
  hp += hp;

  if (hp > hp_max) hp = hp_max;
}

void AttributeComponent::LoseEXP(const int exp) {
  exp -= exp;

  if (exp < 0) exp = 0;
}

void AttributeComponent::GainExp(const int exp) {
  exp += exp;

  UpdateLevel();
}

void AttributeComponent::UpdateStats(const bool reset) {
  hp_max = vitality * 5 + vitality + strength / 2 + intelligence / 5;
  damage_min = strength * 2 + strength / 4 + intelligence / 5;
  damage_max = strength * 2 + strength / 2 + intelligence / 5;
  accuracy = dexterity * 5 + dexterity / 2 + intelligence / 5;
  defence = agility * 2 + agility / 4 + intelligence / 5;
  luck = intelligence * 2 + intelligence / 5;

  if (reset) {
    hp = hp_max;
  }
}

void AttributeComponent::UpdateLevel() {
  while (exp >= next_exp) {
    ++level;
    exp -= next_exp;
    next_exp = static_cast<int>(
        (50 / 3) * (pow(level, 3) - 6 * pow(level, 2) + (level * 17) - 12));
    ++attribute_points;
  }
}

void AttributeComponent::Update() { UpdateLevel(); }
