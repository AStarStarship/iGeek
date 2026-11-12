// Copyright AStarship <https://astarship.net>.
// ExoArena WebGL entry — Emscripten + GLFW + GLES3. Builds the arena on the
// ASCIICrabs memory model (ExoArena.hpp) and draws it with Exo. The texture's
// pixels live in a Room -> Wall -> root List slab; ExoCreateImageRGBA uploads
// them, ExoImagePattern draws them. No C heap, no stdio, no std::.
//
// Build (from this folder):
//   em++ -std=c++23 -O2 -sMEMORY64=1 \
//     -I<CrabsTK> -I<ASCIICrabs> -I<AStarStarship> \
//     ExoArenaWeb.cpp -o exoarena.html \
//     -sUSE_GLFW=3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 \
//     -sALLOW_MEMORY_GROWTH=1 \
//     --preload-file=<Roboto-Regular.ttf>@Roboto-Regular.ttf
//   python3 -m http.server 8099   # open http://127.0.0.1:8099/exoarena.html

#define _CrabsCore 1
#define _CrabsIToS 2
#define _CrabsFToS 3
#define _CrabsUniprinter 4
#define _CrabsClock 5
#define _CrabsArray 6
#define _CrabsStack 7
#define _CrabsString 8
#define _CrabsRegEx 9
#define _CrabsLoom 10
#define _CrabsList 11
#define _CrabsBook 12
#define _CrabsMap 13
#define _CrabsTable 14
#define _CrabsDictionary 15
#define _CrabsOp 16
#define _CrabsRoom 17
#define _CrabsMathCore 18
#define _CrabsRandom 19
#define _CrabsFile 20
#define _CrabsPromise 21
#define _CrabsScript 22
#define _CrabsPassword 23
#define _CrabsRelease 24
#include <ASCIICrabs/_Config.h>
#define _Seam _CrabsRelease
#include <ASCIICrabs/_Package.hxx>
#include <Exo/VG.h>
using namespace CTK::Exo;
using namespace ::_;
#include <Exo/VG.hxx>
#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>
#define NANOVG_GLES3_IMPLEMENTATION
#include <Exo/nanovg_gl.h>
#include <emscripten/emscripten.h>
#include "ExoArena.hpp"

static RenderContext* g_ctx = nullptr;
static ISC g_image = -1;

static void Frame() {
  GLFWwindow* w = glfwGetCurrentContext();
  int fw = 0, fh = 0;
  glfwGetFramebufferSize(w, &fw, &fh);
  if (fw <= 0 || fh <= 0) return;
  glViewport(0, 0, fw, fh);
  glClearColor(0.04f, 0.05f, 0.08f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  FPC W = FPC(fw);
  FPC H = FPC(fh);
  ExoBeginFrame(g_ctx, W, H, 1.0f);
  FPC t = FPC(emscripten_get_now() * 0.001);
  FPC fr = t - FPC(ISC(t));
  FPC osc = fr < 0.5f ? fr * 2.0f : 2.0f - fr * 2.0f;  // triangle 0..1

  VectorPaint bg = ExoLinearGradient(g_ctx, 0, 0, W, H,
                                     ExoRGB(26, 31, 48), ExoRGB(6, 8, 14));
  ExoBeginPath(g_ctx);
  ExoRect(g_ctx, 0, 0, W, H);
  ExoFillPaint(g_ctx, bg);
  ExoFill(g_ctx);

  /* Arena plate: the Room -> Wall -> List texture, drawn as a tile. */
  FPC plate = (W < H ? W : H) * 0.46f;
  FPC px = W * 0.5f - plate * 0.5f;
  FPC py = H * 0.58f - plate * 0.5f;
  VectorPaint tex = ExoImagePattern(g_ctx, px, py, plate, plate, 0.0f,
                                    g_image, 1.0f);
  ExoBeginPath(g_ctx);
  ExoRoundedRect(g_ctx, px, py, plate, plate, 20.0f);
  ExoFillPaint(g_ctx, tex);
  ExoFill(g_ctx);

  FPC cx = W * 0.5f;
  FPC cy = H * 0.42f;
  FPC br = (W < H ? W : H) * 0.115f;
  FPC wave = osc * 20.0f;
  ExoLineCap(g_ctx, LCRound);
  ExoLineJoin(g_ctx, LCMiter);

  /* claws */
  ExoStrokeWidth(g_ctx, 11.0f);
  ExoStrokeColor(g_ctx, ExoRGBA(236, 112, 80, 255));
  ExoBeginPath(g_ctx);
  ExoMoveTo(g_ctx, cx - br + 4, cy - 8);
  ExoBezierTo(g_ctx, cx - br - 56, cy - 44 - wave, cx - br - 96, cy - 8 - wave,
              cx - br - 66, cy + 34);
  ExoStroke(g_ctx);
  ExoBeginPath(g_ctx);
  ExoMoveTo(g_ctx, cx + br - 4, cy - 8);
  ExoBezierTo(g_ctx, cx + br + 56, cy - 44 - wave, cx + br + 96, cy - 8 - wave,
              cx + br + 66, cy + 34);
  ExoStroke(g_ctx);

  /* legs */
  ExoStrokeWidth(g_ctx, 6.0f);
  for (int i = 0; i < 3; ++i) {
    FPC y = cy + 18 + i * 16;
    ExoBeginPath(g_ctx);
    ExoMoveTo(g_ctx, cx - 42, y);
    ExoLineTo(g_ctx, cx - 104, y + 28);
    ExoStroke(g_ctx);
    ExoBeginPath(g_ctx);
    ExoMoveTo(g_ctx, cx + 42, y);
    ExoLineTo(g_ctx, cx + 104, y + 28);
    ExoStroke(g_ctx);
  }

  /* body */
  VectorPaint bodyP = ExoLinearGradient(g_ctx, cx, cy - br, cx, cy + br,
                                        ExoRGB(240, 120, 90),
                                        ExoRGB(196, 58, 44));
  ExoBeginPath(g_ctx);
  ExoRoundedRect(g_ctx, cx - br, cy - br * 0.8f, br * 2, br * 1.6f,
                 br * 0.7f);
  ExoFillPaint(g_ctx, bodyP);
  ExoFill(g_ctx);

  /* eyes */
  ExoBeginPath(g_ctx);
  ExoCircle(g_ctx, cx - 26, cy - 20, 13);
  ExoFillColor(g_ctx, ExoRGBA(255, 255, 255, 255));
  ExoFill(g_ctx);
  ExoBeginPath(g_ctx);
  ExoCircle(g_ctx, cx + 26, cy - 20, 13);
  ExoFillColor(g_ctx, ExoRGBA(255, 255, 255, 255));
  ExoFill(g_ctx);
  ExoBeginPath(g_ctx);
  ExoCircle(g_ctx, cx - 26, cy - 20, 6);
  ExoFillColor(g_ctx, ExoRGBA(18, 18, 22, 255));
  ExoFill(g_ctx);
  ExoBeginPath(g_ctx);
  ExoCircle(g_ctx, cx + 26, cy - 20, 6);
  ExoFillColor(g_ctx, ExoRGBA(18, 18, 22, 255));
  ExoFill(g_ctx);

  /* text */
  ExoFontFace(g_ctx, "sans");
  ExoFontSize(g_ctx, 34.0f);
  ExoFillColor(g_ctx, ExoRGBA(130, 205, 255, 255));
  ExoText(g_ctx, 28.0f, 52.0f, "CrabsTK  ExoArena", nullptr);
  ExoFontSize(g_ctx, 17.0f);
  ExoFillColor(g_ctx, ExoRGBA(170, 190, 215, 255));
  ExoText(g_ctx, 28.0f, 78.0f,
          "vector graphics on the ASCIICrabs memory model  ·  WebGL2",
          nullptr);
  ExoFontSize(g_ctx, 14.0f);
  ExoFillColor(g_ctx, ExoRGBA(120, 140, 170, 255));
  ExoText(g_ctx, 28.0f, H - 26.0f,
          "texture pixels live in a Room -> Wall -> root List slab  ·  no C heap",
          nullptr);

  ExoEndFrame(g_ctx);
  glfwSwapBuffers(w);
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  GLFWwindow* win = glfwCreateWindow(900, 600, "CrabsTK ExoArena", nullptr,
                                      nullptr);
  if (!win) return -1;
  glfwMakeContextCurrent(win);
  g_ctx = ExoCreateGLES3(CFAntialias | CFStencilStrokes);
  if (!g_ctx) return -1;

  static iGeek::AExoArena<CHR, CHC, ISS, ISR, DTB, IUC> arena;
  if (!arena.Init(64, 64)) return -2;
  g_image = ExoCreateImageRGBA(g_ctx, arena.Width(), arena.Height(), 0,
                               arena.Pixels());

  ExoCreateFont(g_ctx, "sans", "Roboto-Regular.ttf");
  emscripten_set_main_loop(Frame, 0, 1);
  return 0;
}
