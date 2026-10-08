// Copyright AStarship <https://astarship.net>.
#include "stdafx.h"
//
#include "skill_component.h"

SkillComponent::SkillComponent() {
  skills.Push(Skill(SKILLS::kConstitution));
  skills.Push(Skill(SKILLS::kCombatMelee));
  skills.Push(Skill(SKILLS::kCombatRanged));
  skills.Push(Skill(SKILLS::kEndurance));
}

SkillComponent::~SkillComponent() {}

const int SkillComponent::GetSkill(const int skill) const {
  if (skills.empty() || skill < 0 || skill >= skills.size())
    throw("ERROR::SKILLCOMPONENT::GETSKILL::SKILL DOES NOT EXIST: " + skill);
  else
    return skills[skill].getLevel();
}

const void SkillComponent::GainExp(const int skill, const int exp) {
  if (skills.empty() || skill < 0 || skill >= skills.size())
    throw("ERROR::SKILLCOMPONENT::GAINEXP::SKILL DOES NOT EXIST: " + skill);
  else {
    skills[skill].GainExp(exp);
  }
}
