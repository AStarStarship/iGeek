// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CHARACTER_HXX
#define IGEEK_CHARACTER_HXX
#include "Character.h"

using namespace PacWorld;

Character::Character()
    : speed_(1.f),
      position_(0.f, 0.f),
      direction_(0, 0),
      next_direction_(0, 0),
      previous_intersection_(-1, -1),
      maze_(NILP) {
  for (ISC i = 0; i < 4; ++i) available_directions_[i] = true;
}

void Character::Update(FPC delta) {
  FPC2 pixel_position = position_;

  FPC pixel_traveled = Speed() * delta;

  // Advance along the current direction.
  FPC2 next_pixel_position =
      pixel_position + FPC2(FPC(direction_.x), FPC(direction_.y)) * pixel_traveled;
  position_ = next_pixel_position;

  ISC2 cell_position = maze_->PixelToCell(pixel_position);

  // Offset from cell center (was std::fmod, now explicit).
  FPC offset_x = (pixel_position.x - FPC(ISC(pixel_position.x) / 32) * 32.f) - 16.f;
  FPC offset_y = (pixel_position.y - FPC(ISC(pixel_position.y) / 32) * 32.f) - 16.f;

  // Wall ahead: snap back to the cell center if we're past it.
  if (maze_->IsWall(cell_position + direction_)) {
    if ((direction_.x == 1 && offset_x > 0) ||
        (direction_.x == -1 && offset_x < 0) ||
        (direction_.y == 1 && offset_y > 0) ||
        (direction_.y == -1 && offset_y < 0)) {
      position_ = maze_->CellToPixel(cell_position);
    }
  }

  // At an intersection and turning: snap to center and change direction.
  // Only fires when both the current and next directions are valid
  // (non-zero) and differ — prevents a spurious turn when next_direction_
  // is still the default (0,0).
  BOL valid_dir = (direction_.x != 0 || direction_.y != 0);
  BOL valid_next = (next_direction_.x != 0 || next_direction_.y != 0);
  if (valid_dir && valid_next &&
      !maze_->IsWall(cell_position + direction_) &&
      direction_ != next_direction_) {
    if ((!direction_.y && (offset_x > -2.f && offset_x < 2.f)) ||
        (!direction_.x && (offset_y > -2.f && offset_y < 2.f))) {
      position_ = maze_->CellToPixel(cell_position);
      direction_ = next_direction_;
      // (rotation/scale are render concerns — stubbed for headless)
    }
  }

  // Detect entering a new intersection and check for available-direction
  // changes (triggers ChangeDirection() for Ghost AI).
  static const ISC2 kDirections[4] = {
      ISC2(1, 0), ISC2(0, 1), ISC2(-1, 0), ISC2(0, -1)};

  if (cell_position != previous_intersection_) {
    if ((!direction_.y && (offset_x > -2.f && offset_x < 2.f)) ||
        (!direction_.x && (offset_y > -2.f && offset_y < 2.f))) {
      BOL new_available[4];
      for (ISC i = 0; i < 4; ++i) {
        new_available[i] = maze_->IsWall(cell_position + kDirections[i]);
      }
      BOL changed = false;
      for (ISC i = 0; i < 4; ++i) {
        if (new_available[i] != available_directions_[i]) {
          changed = true;
          break;
        }
      }
      if (changed) {
        previous_intersection_ = cell_position;
        for (ISC i = 0; i < 4; ++i) available_directions_[i] = new_available[i];
        ChangeDirection();
      }
    }
  }
}

void Character::SetDirection(ISC2 direction) { direction_ = direction; }

ISC2 Character::Direction() const { return direction_; }

void Character::SetSpeed(FPC speed) { speed_ = speed; }

FPC Character::Speed() const { return speed_; }

void Character::SetPosition(FPC2 position) { position_ = position; }

FPC2 Character::Position() const { return position_; }

FPC4 Character::CollisionBox() const {
  // 34x34 box centered on the character (was sf::Transformable::transformRect).
  return FPC4(position_.x - 17.f, position_.y - 17.f, 34.f, 34.f);
}

void Character::SetMaze(Maze* maze) {
  if (NILP == maze) return;
  maze_ = maze;
}

BOL Character::WillMove() const {
  return !maze_->IsWall(previous_intersection_ + next_direction_);
}

#endif  // IGEEK_CHARACTER_HXX
