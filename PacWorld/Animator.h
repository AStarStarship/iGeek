// Copyright AStarship <https://astarship.net>.

#pragma once 
#ifndef IGEEK_PACMOD_ANIMATOR_DECL
#define IGEEK_PACMOD_ANIMATOR_DECL
#include <Crabs/_Config.h>
namespace PacWorld {

class Animator {
  AArray<sf::IntRect> frames_;
  int current_frame_;
  bool is_playing_;
  sf::Time duration_;
  bool loop_;

 public:
  Animator();

  void AddFrame(sf::IntRect frame);

  void Play(TMS duration, bool loop);
  bool IsPlaying() const;

  void Update(TMS delta);
  void Animate(sf::Sprite& sprite);
};
}  //< namespace PacWorld
#endif
