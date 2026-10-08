// Copyright AStarship <https://astarship.net>.

#include "rat.h"

void Rat::InitVariables() {}

void Rat::InitAnimation() {
  animation_->AddAnimation("IDLE", 25.f, 0, 0, 3, 0, 60, 64);
  animation_->AddAnimation("WALK_DOWN", 11.f, 0, 1, 3, 1, 60, 64);
  animation_->AddAnimation("WALK_LEFT", 11.f, 0, 2, 3, 2, 60, 64);
  animation_->AddAnimation("WALK_RIGHT", 11.f, 0, 3, 3, 3, 60, 64);
  animation_->AddAnimation("WALK_UP", 11.f, 0, 4, 3, 4, 60, 64);
  animation_->AddAnimation("ATTACK", 5.f, 0, 2, 1, 2, 60, 64);
}

Rat::Rat(float x, float y, sf::Texture& texture_sheet) : Enemy() {
  InitVariables();

  CreateHitboxComponent(sprite, 13.f, 39.f, 30.f, 30.f);
  CreateMovementComponent(50.f, 1600.f, 1000.f);
  CreateAnimation(texture_sheet);

  PositionSet(x, y);
  InitAnimation();
}

Rat::~Rat() {}

void Rat::UpdateAnimation(const float& dt) {
  if (movement_->GetState(IDLE)) {
    animation_->Play("IDLE", dt);
  } else if (movement_->GetState(kMovingLeft)) {
    animation_->Play("WALK_LEFT", dt, movement_->Velocity().x,
                     movement_->VelocityMax());
  } else if (movement_->GetState(kMovingRight)) {
    animation_->Play("WALK_RIGHT", dt, movement_->Velocity().x,
                     movement_->VelocityMax());
  } else if (movement_->GetState(kMovingUp)) {
    animation_->Play("WALK_UP", dt, movement_->Velocity().y,
                     movement_->VelocityMax());
  } else if (movement_->GetState(kMovingDown)) {
    animation_->Play("WALK_DOWN", dt, movement_->Velocity().y,
                     movement_->VelocityMax());
  }
}

void Rat::Update(const float& dt, sf::Vector2f& mouse_pos_view) {
  movement_->Update(dt);

  // UpdateAttack();

  UpdateAnimation(dt);

  hitbox_->Update();
}

void Rat::Render(sf::RenderTarget& target, sf::Shader* shader,
                 const sf::Vector2f light_position, const bool show_hitbox) {
  if (shader) {
    shader->setUniform("HasTexture", true);
    shader->setUniform("LightPos", light_position);
    target.draw(sprite, shader);
  } else {
    target.draw(sprite);
  }

  if (show_hitbox) hitbox_->Render(target);
}
