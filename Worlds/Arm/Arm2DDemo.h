// Copyright AStarship <https://astarship.net>.
// Arm2D demo, sim side. Runs the 2-link reacher with a small CPU policy,
// records the trajectory + reward into flat arrays, and rasterizes them with
// the CrabsTK/Plot API into a CImage. No GL, no SDL — this file is the part
// an agent reads to understand the demo, and it is the part that runs
// headless. The WebGL widget (Arm2DWebGL.hxx) only uploads the resulting
// texture. Data and code are separated: the trajectory lives in arrays, the
// Plot API is the only thing that turns them into pixels.
#pragma once
#ifndef _iGeekArm2DDemoH
#define _iGeekArm2DDemoH
#include "ArmWorld.h"
#include "../../CrabsTK/Plot/Plot.h"
#include "../../CrabsTK/Exo/Canvas.h"

namespace iGeek {

enum {
  Arm2DTrajMax = 512,      //< frames recorded per episode
  Arm2DFigWidth = 640,
  Arm2DFigHeight = 480
};

/* One recorded frame of the arm. */
struct Arm2DFrame {
  FPD j1 = 0, j2 = 0;      //< joint angles, rad
  FPD tip_x = 0, tip_y = 0;
  FPD reward = 0;
  FPD tip_error = 0;
};

/* The sim + recorder. Owns the env, the policy state, and the trajectory
 * arrays. Step() advances one control frame; the arrays are filled as a
 * side effect so the render side can plot them at any time. */
class Arm2DDemo {
 public:
  Arm2DDemo(IUD seed = 7);
  /* Advance one control frame under the policy. Returns the frame reward. */
  FPC Step();
  /* True when the episode is over (reach or timeout). */
  BOL Done() { return env_.Done(); }
  void Reset();
  /* How many frames are recorded so far. */
  ISN Frames() { return frames_; }

  /* --- the data (flat arrays, plotted by the render side) -------------- */
  FPD tip_x_[Arm2DTrajMax] = {};
  FPD tip_y_[Arm2DTrajMax] = {};
  FPD reward_[Arm2DTrajMax] = {};
  FPD tip_error_[Arm2DTrajMax] = {};
  FPD frame_t_[Arm2DTrajMax] = {};

  /* Rasterize the current state into fig: the arm pose (workspace view) on
   * axis 0 and the reward/error curves on axis 1. */
  void RenderFigure(CTK::Plot::Figure& fig);
  /* Vector-draw the current state into a nanovg canvas: the tip trajectory
   * as a stroked polyline, the two links as thick strokes, the joints and
   * tip as circles, and the target as a crosshair. This is the crisp
   * vector view the WebGL window shows. */
  void RenderVector(CTK::Exo::AVCanvas& cv, FPD pad = 12);
  AArmEnv& Env() { return env_; }
 private:
  AArmEnv env_;
  ISN frames_ = 0;
  /* A tiny CPU policy: two "perceptrons" (weighted sums of the tip error)
   * that push the joints toward the target. Deterministic, no training in
   * this pass — it is the stand-in until the trained weights land. */
  ISC PolicyAction();
};

}  //< namespace iGeek
#endif  //< _iGeekArm2DDemoH
