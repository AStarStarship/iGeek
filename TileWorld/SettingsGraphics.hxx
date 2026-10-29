#include "TVec.h"
// Copyright AStarship <https://astarship.net>.
#include "settings_graphics.h"

GraphicsSettings::GraphicsSettings() {
  title = "kDefault";
  resolution = _::TVideoMode::getDesktopMode();
  fullscreen = false;
  vertical_sync = false;
  frame_rate_limit = 120;
  context_settings.antialiasingLevel = 0;
  video_modes = _::TVideoMode::getFullscreenModes();
}

void GraphicsSettings::SaveToFile(const _::AString path) {
  std::ofstream ofs(path);

  if (ofs.is_open()) {
    ofs << title << resolution.width << " " << resolution.height << fullscreen
        << frame_rate_limit << vertical_sync
        << context_settings.antialiasingLevel;
  }

  ofs.close();
}

void GraphicsSettings::LoadFromFile(const _::AString path) {
  std::ifstream ifs(path);

  if (ifs.is_open()) {
    std::getline(ifs, title);
    ifs >> resolution.width >> resolution.height >> fullscreen >>
        frame_rate_limit >> vertical_sync >> context_settings.antialiasingLevel;
  }

  ifs.close();
}
