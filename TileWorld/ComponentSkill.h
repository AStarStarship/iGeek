// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef TILEWORLD_SKILLCOMPONENT
#define TILEWORLD_SKILLCOMPONENT
#include <_Config.h>
namespace TileWorld {

enum SKILLS { kConstitution = 0, kCombatMelee, kCombatRanged, kEndurance };

class SkillComponent {
  class Skill {
   private:
    ISN type_, level_, level_max_, exp_, next_exp_;

   public:
    Skill(ISN type) {
      type = type;
      level_ = 1;
      level_max_ = 99;
      exp_ = 0;
      next_exp_ = 100;
    }

    ~Skill() {}

    inline const ISN& getType() const { return type_; }
    inline const ISN& getLevel() const { return level_; }
    inline const ISN& getExp() const { return exp_; }
    inline const ISN& getExpNext() const { return next_exp_; }

    // Modifiers
    void setLevel(const ISN level) { level_ = level; }
    void setLevelCap(const ISN level_cap) { level_max_ = level_cap; }

    void GainExp(const ISN exp) {
      exp_ += exp;
      UpdateLevel();
    }

    void LoseExp(const ISN exp) { exp_ -= exp; }

    void UpdateLevel(const bool up = true) {
      // Changes the skill depending on if there is a deficit in the exp or not.

      if (up) {
        if (level_ < level_max_) {
          while (exp_ >= next_exp_) {
            if (level_ < level_max_) {
              level_++;
              next_exp_ = static_cast<ISN>(std::pow(level_, 2)) + level_ * 10 +
                          level_ * 2;
            }
          }
        }
      } else {
        if (level_ > 0) {
          while (exp_ < 0) {
            if (level_ > 0) {
              level_--;
              next_exp_ = static_cast<ISN>(std::pow(level_, 2)) + level_ * 10 +
                          level_ * 2;
            }
          }
        }
      }
    }

    void Update() {}
  };

  _::Array<Skill> skills;

 public:
  SkillComponent();
  virtual ~SkillComponent();

  const ISN GetSkill(const ISN skill) const;
  const void GainExp(const ISN skill, const ISN exp);
};
}  //< namespace TileWorld
#endif
