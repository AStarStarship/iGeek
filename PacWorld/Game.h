// Copyright AStarship <https://astarship.net>.
#ifndef PACWORLD_GAME
#define PACWORLD_GAME

#include "GameState.h"
namespace PacWorld {

template <typename T>
void CenterOrigin(T& drawable) {
  fl::FloatRect bound = drawable.getLocalBounds();
  drawable.setOrigin(bound.width / 2, bound.height / 2);
}

class Game {
  sf::RenderWindow window_;  //< Window to render too.
  GameState& state_;
  sf::Font font_;
  sf::Texture logo_, texture_;

 public:
  StateInsertCoin state_insert_coin;
  StatePlaying state_playing;
  StateWon state_won;
  StateLost state_lost;

  enum State {
    kStateInsertCoin = 0,  //< Init game state waits for user to insert coin.
    kStateGetReady,        //< Game 3. 2, 1 countdown state.
    kStatePlaying,         //< Playing game state.
    kStateWon,             //< Won game state.
    kStateLost,            //< Lost game state.
  };

  Game();

  int Run();

  void ChangeState(GameState& next_state);

  sf::Font& Font();

  sf::Texture& Logo();

  sf::Texture& Texture();
};
}  //< namespace PacWorld
#endif
