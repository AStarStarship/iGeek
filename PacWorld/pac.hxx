// Copyright AStarship <https://astarship.net>.
#include "pac.h"

using namespace kabuki::arcade;

Pac::Pac(sf::Texture& texture)
    : is_dead_(false), is_dying_(false), visual_(texture) {
  setOrigin(20, 20);

  // run_animator_.AddFrame(sf::IntRect(0, 32, 40, 40));
  run_animator_.AddFrame(sf::IntRect(0, 72, 40, 40));

  // die_animator_.AddFrame(sf::IntRect(0, 32, 40, 40));
  // die_animator_.AddFrame(sf::IntRect(0, 72, 40, 40));
  // die_animator_.AddFrame(sf::IntRect(0, 112, 40, 40));
  // die_animator_.AddFrame(sf::IntRect(40, 112, 40, 40));
  // die_animator_.AddFrame(sf::IntRect(80, 112, 40, 40));
  // die_animator_.AddFrame(sf::IntRect(120, 112, 40, 40));
  // die_animator_.AddFrame(sf::IntRect(160, 112, 40, 40));

  run_animator_.Play(sf::seconds(0.25), true);
}

void Pac::Die() {
  if (!is_dying_) {
    die_animator_.Play(sf::seconds(1), false);
    is_dying_ = true;
  }
}

void Pac::Reset() {
  is_dying_ = false;
  is_dead_ = false;
  run_animator_.Play(sf::seconds(0.25), true);
  run_animator_.Animate(visual_);
}

bool Pac::IsDying() { return is_dying_; }

bool Pac::IsDead() { return is_dead_; }

void Pac::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  states.transform *= getTransform();

  if (!is_dead_) target.draw(visual_, states);
}

void Pac::Update(sf::Time delta) {
  if (!is_dead_ && !is_dying_) {
    run_animator_.Update(delta);
    run_animator_.animate_visual_);
  } else {
    // die_animator_.Update(delta);
    die_animator.Animate(visual_);

    if (!die_animator_.IsPlaying()) {
      is_dying _ = false;
      is_dead_ = true;
    }
  }
}
