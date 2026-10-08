// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_GRAPHICSSETTINGS
#define TILEWORLD_GRAPHICSSETTINGS

class GraphicsSettings {
 public:
  GraphicsSettings();

  _::AString title;
  sf::VideoMode resolution;
  bool fullscreen;
  bool vertical_sync;
  unsigned frame_rate_limit;
  sf::ContextSettings context_settings;
  _::Array<sf::VideoMode> video_modes;

  void SaveToFile(const _::AString path);
  void LoadFromFile(const _::AString path);
};

#endif
