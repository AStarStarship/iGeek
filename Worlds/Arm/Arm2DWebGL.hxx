// Copyright AStarship <https://astarship.net>.
#pragma once
// Arm2D demo, render side.
#include "Arm2DWebGL.h"
#include "Arm2DDemo.hxx"
#include "../../CrabsTK/Exo/Canvas.hxx"
#include "../../CrabsTK/Exo/Window.hxx"
#include "../../CrabsTK/Plot/Plot.hxx"

namespace iGeek {
using namespace CTK;

Arm2DWebGL::Arm2DWebGL() = default;

BOL Arm2DWebGL::Open(ISN width, ISN height, const CHA* title) {
  if (!window_.Init(width, height, title)) return false;
  /* The canvas draws inside the environment's Room — the only Room in the
  process. On-screen pixels and the sim's state are the same allocation, so
  nothing is copied between a "sim world" and a "render world". */
  if (!canvas_.Init(*demo_.Env().Room(), figure_w_, figure_h_, Exo::IWhite)) {
    window_.Close();
    return false;
  }
  running_ = true;
  return true;
}

void Arm2DWebGL::Close() {
  running_ = false;
  /* Detach the Wall from the Room before the GL context goes away, and before
  the sim (which owns the Room) is destroyed. */
  canvas_.Release();
  texture_.Release();
  window_.Close();
}

void Arm2DWebGL::Frame() {
  /* 1. Sim: advance one control frame under the policy. */
  if (!demo_.Done()) demo_.Step();

  /* 2. Rasterize into the Room canvas. The Figure borrows those pixels, so
  there is no second image allocation anywhere in the frame. */
  Exo::CImage& img = canvas_.Image();
  Plot::Figure fig(img);
  fig.AddAxis(0.05, 0.40, 0.55, 0.95);  //< workspace (top-left)
  fig.AddAxis(0.58, 0.40, 0.97, 0.95);  //< reward / tip error (top-right)
  fig.SetCurrentAxis(0);
  Exo::Fill(img, Exo::IWhite);
  Plot::RenderFrame(fig);    //< frame, grid, ticks
  demo_.RenderFigure(fig);   //< series on top

  /* 3. Upload the same pixels and blit them into the window. */
  texture_.Upload(img);
  window_.BeginFrame();
  if (!window_.Poll()) {
    running_ = false;
    return;
  }
  window_.EndFrame(18, 18, 22, 255);
  texture_.Draw(window_.Width(), window_.Height(), 0, 0, img.width, img.height);
  window_.Swap();
}

void Arm2DWebGL::Run(BOL auto_reset) {
  while (running_) {
    Frame();
    if (running_ && demo_.Done() && auto_reset) demo_.Reset();
  }
}

}  //< namespace iGeek
