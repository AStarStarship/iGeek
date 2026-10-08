// Copyright AStarship <https://astarship.net>.
#include "ghost.h"

#include <map>

using namespace kabuki::arcade;

Ghost::Ghost(sf::Texture& texture)
    : visual_(texture), is_weak_(false), weakness_duration_(sf::Time::Zero) {
  setOrigin(20, 20);

  strong_animator_.AddFrame(sf::IntRect(40, 32, 40, 40));
  // strong_animator_.AddFrame(sf::IntRect(80, 32, 40, 40));

  weak_animator_.AddFrame(sf::IntRect(40, 72, 40, 40));
  // weak_animator_.AddFrame(sf::IntRect(80, 72, 40, 40));

  strong_animator_.Play(sf::seconds(0.25), true);
  weak_animator_.Play(sf::seconds(1), true);
}

void Ghost::SetWeak(sf::Time duration) {
  is_weak_ = true;
  weakness_duration_ = duration;
}

bool Ghost::IsWeak() const { return is_weak_; }

void Ghost::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  states.transform *= getTransform();

  target.draw(visual_, states);
}

void Ghost::Update(sf::Time delta) {
  if (is_weak_) {
    weakness_duration_ -= delta;

    if (weakness_duration_ <= sf::Time::Zero) {
      is_weak_ = false;
      strong_animator_.Play(sf::seconds(0.25), true);
    }
  }

  if (is_weak_) {
    strong_animator_.Update(delta);
    strong_animator_.Animate(visual_);
  } else {
    weak_animator_.Update(delta);
    weak_animator_.Animate(visual_);
  }
  Character::Update(delta);
}

void Ghost::ChangeDirection() {
  static sf::Vector2i directions[4] = {
      sf::Vector2i(1, 0),
      sf::Vector2i(0, 1),
      sf::Vector2i(-1, 0),
      sf::Vector2i(0, -1),
  };

  std::map<float, sf::Vector2i> directions_prob;

  float target_angle;

  sf::Vector2f distance = pac_->getPosition() - getPosition();

  target_angle = std::atan2(distance.x, distance.y) * 180.0 / 3.14;

  for (auto direction : directions) {
    float direction_angle = std::atan2(direction.x, direction.y) * (180 / 3.14);

    // Noarmalize the angle.

    float diff = 180 - std::abs(std::abs(direction_angle - target_angle) - 180);

    direction_prob[diff] = direction;
  }
  SetDirection(direction_prob.begin()->second);

  auto it = direction_prob.begin();

  do {
    SetDirection(it->second);
    ++i;
  } while (!WillMove());
}
