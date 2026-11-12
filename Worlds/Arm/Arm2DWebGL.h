// Copyright AStarship <https://astarship.net>.
// Arm2D demo, render side. An SDL3 window + GL context from CrabsTK/Exo, and
// the Room-rasterized figure blitted as a texture. The sim (Arm2DDemo) fills a
// Plot::Figure over the environment Room's canvas pixels; this file only
// uploads that canvas and presents it. One Room: the environment's. The sim
// never touches SDL/GL, the render never touches sim state beyond reading the
// finished figure.
#pragma once
#ifndef _iGeekArm2DWebGLH
#define _iGeekArm2DWebGLH
#include "Arm2DDemo.h"
#include "../../CrabsTK/Exo/Canvas.h"
#include "../../CrabsTK/Exo/Window.h"

namespace iGeek {

/* The windowed presentation. Owns the window, the texture, the sim and the
canvas that draws inside the sim's Room. */
class Arm2DWebGL {
 public:
  Arm2DWebGL();
  /* Open the window + GL context and bind a canvas to the environment Room.
  @return false on failure (the window is closed again). */
  BOL Open(ISN width = 1024, ISN height = 768,
           const CHA* title = "Arm2D  //  2-link reacher");
  /* Run until the window closes. Resets the episode when it ends and
  auto_reset is set. */
  void Run(BOL auto_reset = true);
  /* One frame: step the sim, rasterize the figure, upload, present. Public so
  an application can drive its own loop (and so a headless video driver can
  exercise the real window path). */
  void Frame();
  void Close();

  /* The Room-backed canvas the figure is rasterized into. */
  CTK::Exo::AVCanvas& Canvas() { return canvas_; }

 private:
  CTK::Exo::AWindow window_;
  CTK::Exo::ATexture texture_;
  Arm2DDemo demo_;             //< owns the one Room that canvas_ draws in.
  CTK::Exo::AVCanvas canvas_;  //< after demo_: destroyed before its Room.
  ISN figure_w_ = ISN(Arm2DFigWidth);
  ISN figure_h_ = ISN(Arm2DFigHeight);
  BOL running_ = false;
};

}  //< namespace iGeek
#endif  //< _iGeekArm2DWebGLH
