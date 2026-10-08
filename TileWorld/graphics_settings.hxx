// Copyright AStarship <https://astarship.net>.
#include "settings_graphics.h"

GraphicsSettings::GraphicsSettings() {
  title = "kDefault";
  resolution = sf::VideoMode::getDesktopMode();
  fullscreen = false;
  verticalSync = false;
  frameRateLimit = 120;
  contextSettings.antialiasingLevel = 0;
  videoModes = sf::VideoMode::getFullscreenModes();
}

// Functions
void GraphicsSettings::SaveToFile(const _::AString path) {
  std::ofstream ofs(path);

  if (ofs.is_open()) {
    ofs << title << resolution.width << " " << resolution.height << fullscreen
        << frameRateLimit << verticalSync << contextSettings.antialiasingLevel;
  }

  ofs.close();
}

void GraphicsSettings::LoadFromFile(const _::AString path) {
  std::ifstream ifs(path);

  if (ifs.is_open()) {
    std::getline(ifs, title);
    ifs >> resolution.width >> resolution.height >> fullscreen >>
        frameRateLimit >> verticalSync >> contextSettings.antialiasingLevel;
  }

  ifs.close();
}
