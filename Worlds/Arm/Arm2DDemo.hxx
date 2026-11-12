// Copyright AStarship <https://astarship.net>.
// Arm2D demo, sim side.
#include "Arm2DDemo.h"
#include "../../CrabsTK/Plot/Plot.hxx"

namespace iGeek {
using namespace CTK;

Arm2DDemo::Arm2DDemo(IUD seed) {
  /* Default-construct the params (the SI-unit 2-link reacher), then hand them
   * to the env. Reading env_.Params() before env_ is constructed would read
   * uninitialized memory, so the params are built first. */
  ArmParams params;
  env_.ResetParams(params);
  env_.ResetSeed(seed);
}

ISC Arm2DDemo::PolicyAction() {
  /* Two perceptrons over the tip error. Each perceptron is a weighted sum
   * of the x/y error; the sign of each decides the joint torque direction.
   * This is the deterministic stand-in policy — the same two weights become
   * the trained parameters once the RL loop lands. */
  FPD ex = env_.TargetX() - env_.State().tip_x;
  FPD ey = env_.TargetY() - env_.State().tip_y;
  const FPD w1 = 1.0, w2 = 0.5;
  FPD p1 = w1 * ex + w2 * ey;   //< drives joint 1
  FPD p2 = -w2 * ex + w1 * ey;  //< drives joint 2
  ISC a1 = (p1 > 0.001) ? ArmInc1 : (p1 < -0.001 ? ArmDec1 : ArmHold);
  ISC a2 = (p2 > 0.001) ? ArmInc2 : (p2 < -0.001 ? ArmDec2 : ArmHold);
  switch (a1) {
    case ArmInc1: return (a2 == ArmInc2) ? ArmInc12 : ArmInc1;
    case ArmDec1: return (a2 == ArmDec2) ? ArmDec12 : ArmDec1;
    default:      return (a2 == ArmInc2) ? ArmCross : ArmHold;
  }
}

FPC Arm2DDemo::Step() {
  FPC r = env_.Step(PolicyAction());
  if (frames_ < Arm2DTrajMax) {
    tip_x_[frames_] = env_.State().tip_x;
    tip_y_[frames_] = env_.State().tip_y;
    reward_[frames_] = FPD(r);
    tip_error_[frames_] = env_.TipError();
    frame_t_[frames_] = env_.State().time;
    ++frames_;
  }
  return r;
}

void Arm2DDemo::Reset() {
  env_.Reset();
  frames_ = 0;
}

void Arm2DDemo::RenderVector(CTK::Exo::AVCanvas& cv, FPD pad) {
  using namespace CTK::Exo;
  ISN W = cv.Image().width, H = cv.Image().height;
  /* The workspace is [0, 0.5]^2 m (both links 0.2, quarter plane). Map it
   * into the canvas with a margin, y-flipped (world y up, screen y down). */
  FPD lo = -0.05, hi = 0.50;
  FPD scale = (FPD)W - 2.0 * pad;
  FPD sy = (FPD)H - 2.0 * pad;
  auto SX = [&](FPD x) -> FPD { return pad + (x - lo) * (scale / (hi - lo)); };
  auto SY = [&](FPD y) -> FPD { return H - pad - (y - lo) * (sy / (hi - lo)); };

  cv.Clear(IWhite);
  cv.BeginFrame();

  /* Workspace frame. */
  cv.BeginPath();
  cv.MoveTo(SX(0), SY(0));
  cv.LineTo(SX(0.5), SY(0));
  cv.LineTo(SX(0.5), SY(0.5));
  cv.LineTo(SX(0), SY(0.5));
  cv.ClosePath();
  cv.Stroke(ISlateGray, 1.0);

  const ArmState& s = env_.State();
  FPD l1 = env_.Params().length_1, l2 = env_.Params().length_2;
  FPD elbow_x = l1 * cos(s.j1);
  FPD elbow_y = l1 * sin(s.j1);

  /* Tip trajectory (thin cyan polyline). */
  if (frames_ > 1) {
    cv.BeginPath();
    cv.MoveTo(SX(tip_x_[0]), SY(tip_y_[0]));
    for (ISC i = 1; i < (ISC)frames_; ++i) cv.LineTo(SX(tip_x_[i]), SY(tip_y_[i]));
    cv.Stroke(ICyan, 1.0);
  }

  /* The two links (thick steel-blue strokes). */
  cv.BeginPath();
  cv.MoveTo(SX(0), SY(0));
  cv.LineTo(SX(elbow_x), SY(elbow_y));
  cv.LineTo(SX(s.tip_x), SY(s.tip_y));
  cv.Stroke(IViolet, 4.0);

  /* Base + elbow joints. */
  cv.BeginPath(); cv.Circle(SX(0), SY(0), 5.0); cv.Fill(ISlateGray);
  cv.BeginPath(); cv.Circle(SX(elbow_x), SY(elbow_y), 4.0); cv.Fill(IViolet);
  /* Tip. */
  cv.BeginPath(); cv.Circle(SX(s.tip_x), SY(s.tip_y), 4.5); cv.Fill(IGreen);

  /* Target crosshair. */
  FPD tx = env_.TargetX(), ty = env_.TargetY();
  cv.BeginPath();
  cv.MoveTo(SX(tx) - 8, SY(ty)); cv.LineTo(SX(tx) + 8, SY(ty));
  cv.MoveTo(SX(tx), SY(ty) - 8); cv.LineTo(SX(tx), SY(ty) + 8);
  cv.Stroke(IAmber, 1.5);
  cv.BeginPath(); cv.Circle(SX(tx), SY(ty), 3.0); cv.Stroke(IAmber, 1.0);

  cv.EndFrame();
}

void Arm2DDemo::RenderFigure(CTK::Plot::Figure& fig) {
  using namespace CTK::Plot;
  /* Axis 0: the workspace (arm pose + trajectory + target). */
  Axis& ws = fig.AxisAt(0);
  ws.SetXRange(-0.1, 0.5);
  ws.SetYRange(-0.1, 0.5);
  ws.SetTitle("Arm2D workspace");
  ws.SetXLabel("tip_x (m)");
  ws.SetYLabel("tip_y (m)");
  ws.SetGrid(true);
  ws.SetFrame(true);
  /* Axis 1: the reward and tip-error curves over the episode. */
  Axis& curves = fig.AxisAt(1);
  curves.SetXRange(0, FPD(Arm2DTrajMax));
  curves.SetYRange(-1.0, 1.0);
  curves.SetTitle("reward / tip error");
  curves.SetXLabel("frame");
  curves.SetYLabel("value");
  curves.SetGrid(true);
  curves.SetFrame(true);

  /* The trajectory the tip has traced so far. */
  if (frames_ > 1) {
    Series2D traj;
    traj.x = tip_x_;
    traj.y = tip_y_;
    traj.count = frames_;
    traj.color = Exo::ICyan;
    PlotLine(fig, ws, traj, 1);
  }
  /* The target, as a single scatter marker. */
  {
    FPD tx[1] = {env_.TargetX()};
    FPD ty[1] = {env_.TargetY()};
    Series2D tgt;
    tgt.x = tx; tgt.y = ty; tgt.count = 1;
    tgt.color = Exo::IYellow;
    PlotScatter(fig, ws, tgt, 3);
  }
  /* The current tip. */
  {
    FPD cx[1] = {env_.State().tip_x};
    FPD cy[1] = {env_.State().tip_y};
    Series2D tip;
    tip.x = cx; tip.y = cy; tip.count = 1;
    tip.color = Exo::IGreen;
    PlotScatter(fig, ws, tip, 4);
  }
  /* Reward and tip-error over time. */
  if (frames_ > 1) {
    Series2D rw;
    rw.x = frame_t_;
    rw.y = reward_;
    rw.count = frames_;
    rw.color = Exo::IGreen;
    PlotLine(fig, curves, rw, 1);
    Series2D err;
    err.x = frame_t_;
    err.y = tip_error_;
    err.count = frames_;
    err.color = Exo::IRed;
    PlotLine(fig, curves, err, 1);
  }
}

}  //< namespace iGeek
