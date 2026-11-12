// Copyright AStarship <https://astarship.net>.
// Headless verification of the Arm2D demo sim side. Runs the policy for a
// fixed number of frames, rasterizes the figure, and checks: (a) the env
// actually moves, (b) the figure canvas is non-trivial (pixels changed from
// the background), (c) the trajectory arrays are populated. Writes the
// figure to a PPM so the pixels can be inspected. No GL, no SDL.
#include "Arm2DDemo.hxx"
#include <ASCIICrabs/_Config.h>
#include <ASCIICrabs/Math.hxx>
#include <ASCIICrabs/_Package.hxx>
#include <cstdio>

using namespace _;
using namespace iGeek;
using namespace CTK;

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, what) do { \
  if (cond) { ++g_pass; printf("PASS %s\n", what); } \
  else { ++g_fail; printf("FAIL %s\n", what); } \
} while (0)

int main(int argc, char** argv) {
  Arm2DDemo demo(7);
  {
    Exo::AVCanvas canvas;
    CHECK(canvas.Init(*demo.Env().Room(), 128, 128),
          "canvas binds to the environment Room");
    CHECK(demo.Env().Room()->WallCount() == 1,
          "renderer Wall is registered in the existing Room");
    CHECK(canvas.Room() == demo.Env().Room(), "no second Room for graphics");
    if (!canvas.Ready()) return 1;
    demo.RenderVector(canvas);
    CHECK(canvas.Image().data != _NilP, "Room-backed pixels exist");
    auto* root = TWallRoot<>(demo.Env().Room()->GetWall(0));
    IUW begin = reinterpret_cast<IUW>(root);
    IUW pixels = reinterpret_cast<IUW>(canvas.Image().data);
    CHECK(pixels >= begin && pixels + 128 * 128 * 4 <= begin + root->bytes,
          "pixels are inside the registered Wall");
    auto blocks = root->map.count;
    auto top = root->top;
    for (ISC i = 0; i < 20; ++i) demo.RenderVector(canvas);
    CHECK(root->map.count == blocks && root->top == top,
          "repeated vector frames do not allocate Wall blocks");
    CHECK(!canvas.Init(*demo.Env().Room(), 64, 64),
          "live canvas cannot be rebound");
  }
  CHECK(demo.Env().Room()->WallCount() == 0,
        "canvas destruction detaches its Wall");
  {
    Exo::ARoom room("graphics_app");
    Exo::AVCanvas first, second, invalid;
    CHECK(!invalid.Init(room, 0, 16) && room.WallCount() == 0,
          "invalid dimensions leave the Room unchanged");
    CHECK(first.Init(room, 32, 32) && second.Init(room, 32, 32),
          "non-iGeek application shares one Room between canvases");
    first.Release();
    CHECK(room.WallCount() == 1 && second.Ready(),
          "releasing one canvas preserves another");
    second.Clear(Exo::IRed);
    CHECK(second.Image().data[0] == 255 && second.Image().data[1] == 0,
          "remaining canvas still draws");
    first.Release();
    CHECK(room.WallCount() == 1, "release is idempotent");
    CHECK(first.Init(room, 16, 16), "released canvas can be initialized again");
  }
  {
    /* One Room, a hard Wall budget: the sixth canvas is refused rather than
       overrunning the Room's Wall slots. */
    Exo::ARoom room("graphics_budget");
    Exo::AVCanvas walled[5];
    ISC opened = 0;
    for (ISC i = 0; i < 5; ++i)
      if (walled[i].Init(room, 16, 16)) ++opened;
    CHECK(opened == 5, "a Room hosts WallCountMax canvases");
    Exo::AVCanvas overflow;
    CHECK(!overflow.Init(room, 16, 16) && room.WallCount() == 5,
          "a full Room refuses another canvas");
  }
  FPD t0_tip = demo.Env().State().tip_x;
  for (int i = 0; i < 120 && !demo.Done(); ++i) demo.Step();
  CHECK(demo.Frames() > 10, "policy advanced the arm (frames recorded)");
  CHECK(demo.Env().State().tip_x != t0_tip, "tip position changed");
  CHECK(demo.Frames() > 1, "trajectory arrays populated");

  /* Rasterize and confirm the canvas is non-trivial. */
  Exo::AVCanvas plot_canvas;
  CHECK(plot_canvas.Init(*demo.Env().Room(), Arm2DFigWidth, Arm2DFigHeight),
        "plot pixels use the same environment Room");
  if (!plot_canvas.Ready()) return 1;
  Plot::Figure fig(plot_canvas.Image());
  CHECK(fig.Canvas().data == plot_canvas.Image().data,
        "Figure borrows Room pixels without a second allocation");
  fig.AddAxis(0.05, 0.40, 0.55, 0.95);
  fig.AddAxis(0.58, 0.40, 0.97, 0.95);
  fig.AddAxis(0.05, 0.05, 0.97, 0.35);
  fig.SetCurrentAxis(0);
  Plot::RenderFrame(fig);   // frame first
  demo.RenderFigure(fig);   // series on top
  Exo::CImage& cv = fig.Canvas();
  ISW nonzero = 0;
  for (ISN i = 0; i < (ISN)cv.width * (ISN)cv.height * 4; ++i)
    nonzero += (cv.data[i] != 255 && cv.data[i] != 0) ? 1 : 0;
  CHECK(nonzero > 100, "figure canvas is non-trivial (pixels drawn)");

  /* Dump the figure as a PPM (RGB) for visual inspection. */
  FILE* f = argc > 1 ? fopen(argv[1], "wb") : _NilP;
  if (f) {
    fprintf(f, "P6\n%d %d\n255\n", cv.width, cv.height);
    for (ISN i = 0; i < (ISN)cv.width * (ISN)cv.height; ++i) {
      fputc(cv.data[i * 4], f);
      fputc(cv.data[i * 4 + 1], f);
      fputc(cv.data[i * 4 + 2], f);
    }
    fclose(f);
    printf("figure written to %s\n", argv[1]);
  }

  printf("--- arm2d headless: %d passed, %d failed ---\n", g_pass, g_fail);
  return g_fail ? 1 : 0;
}
