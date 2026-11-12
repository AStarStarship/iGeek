// Copyright AStarship <https://astarship.net>.
//
// ArmWorld release seam = the Arm2D demo. The release seam is a module's
// official output, and for ArmWorld that is the windowed demo: run the 2-link
// reacher under the policy, draw it with the CrabsTK/Exo vector canvas
// (nanovg), and present it in the CrabsTK/Exo window.
//
// This unit is headless: it verifies the sim side (the policy moves the arm)
// and that the vector render produces the arm's pixels (links, tip, target,
// trajectory all land on the canvas). The windowed presentation
// (Arm2DWebGL.hxx) is the same demo with the CrabsTK/Exo window + GL texture
// upload; it is built as the Arm2DGui target and runs where a display exists.
#if _Seam >= _iGeekArmWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#include "../../CrabsTK/Exo/_Package.hxx"
#include "../../Worlds/Arm/Arm2DDemo.hxx"
namespace ArmWorldTest {
using namespace ::_;
using namespace ::iGeek;
using namespace ::CTK;

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekArmWorldRelease
  _ATestBegin;
  _DROut(Headingf("ArmWorld release seam (Arm2D demo)..."));

  // --- the policy actually drives the arm --------------------------------
  {
    Arm2DDemo demo(7);
    FPD tip0 = demo.Env().State().tip_x;
    for (ISC i = 0; i < 120 && !demo.Done(); ++i) demo.Step();
    _DCheck(demo.Frames() > 10, "policy advanced the arm (frames recorded)");
    _DCheck(demo.Env().State().tip_x != tip0, "tip position changed");
    _DCheck(demo.Frames() > 1, "trajectory arrays populated");
  }

  // --- the vector render draws the arm (nanovg) ---------------------------
  {
    Arm2DDemo demo(7);
    for (ISC i = 0; i < 120 && !demo.Done(); ++i) demo.Step();
    Exo::AVCanvas cv;
    _DCheck(cv.Init(*demo.Env().Room(), Arm2DFigWidth, Arm2DFigHeight),
            "canvas uses the environment Room");
    if (!cv.Ready()) return "canvas initialization failed";
    demo.RenderVector(cv);
    Exo::CImage& img = cv.Image();
    /* ISC, not IUA: a 640x480 canvas has far more than 255 of these, and an
       8-bit counter wraps (140506 non-white pixels wrap to 218). */
    ISC nonwhite = 0, violet = 0, green = 0, amber = 0, cyan = 0;
    for (ISN y = 0; y < img.height; ++y) {
      for (ISN x = 0; x < img.width; ++x) {
        IUA r = img.data[((IUN)y * (IUN)img.width + (IUN)x) * 4];
        IUA g = img.data[((IUN)y * (IUN)img.width + (IUN)x) * 4 + 1];
        IUA b = img.data[((IUN)y * (IUN)img.width + (IUN)x) * 4 + 2];
        if (!(r > 240 && g > 240 && b > 240)) ++nonwhite;
        if (r > 100 && r < 150 && g > 50 && g < 100 && b > 200) ++violet;
        if (r < 100 && g > 200 && b < 100) ++green;
        if (r > 200 && g > 150 && b < 100) ++amber;
        if (r < 100 && g > 200 && b > 200) ++cyan;
      }
    }
    _DCheck(nonwhite > 500, "vector canvas is non-trivial (arm drawn)");
    _DCheck(violet > 20, "arm links drawn (violet strokes)");
    _DCheck(green > 5, "tip drawn (green circle)");
    _DCheck(amber > 5, "target crosshair drawn (amber)");
    _DCheck(cyan > 5, "trajectory drawn (cyan polyline)");
    /* TEMP: dump the canvas to PPM for visual inspection. */
    {
      FILE* f = fopen("/home/astar/.hermes/cache/scratch/arm_release.ppm", "wb");
      if (f != nullptr) {
        fprintf(f, "P6\n%ld %ld\n255\n", (long)img.width, (long)img.height);
        for (long i = 0; i < (long)img.width * img.height * 4; i += 4)
          fputc(img.data[i], f), fputc(img.data[i+1], f), fputc(img.data[i+2], f);
        fclose(f);
      }
    }
  }

  _DROut("  Arm2D demo (release) clean\n");
#endif
  return _NilP;
}

}  //< namespace ArmWorldTest
