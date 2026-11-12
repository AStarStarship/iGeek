# ExoArena — ASCIICrabs memory model x Exo / WebGL

Proof of concept: CrabsTK's Exo (NanoVG-derived) vector graphics running on
the ASCIICrabs memory model, compiled to WebAssembly + WebGL2 for embedding
as a widget in the Python websites.

## What it demonstrates
- No C heap. Every allocation goes through the ASCIICrabs arena.
- A 64x64 RGBA8 texture whose pixels live in a **Room -> Wall -> root List**
  slab (TListAlloc index + TListValue pointer), uploaded with
  ExoCreateImageRGBA and drawn with ExoImagePattern.
- A vector crab drawn with Exo paths (rounded rect, bezier claws, circles).

## Layout
- `ExoArena.hpp`    — `AExoArena<LST_A>`: Room + Wall + root List + procedural
  texture fill. GL-free, so it can be unit-tested on the CPU.
- `ExoArenaWeb.cpp`— Emscripten + GLFW + GLES3 entry. Uploads the texture,
  draws the scene, runs the frame loop.

## Include order (required)
`AExoArena` uses the canonical `LST` / `WAL_P` / `LST_P` aliases, which need
`using namespace ::_;`. Include `ExoArena.hpp` only after the ASCIICrabs
package and that using-directive, exactly as `ExoArenaWeb.cpp` does:

```
#include <ASCIICrabs/_Config.h>
#define _Seam _CrabsRelease
#include <ASCIICrabs/_Package.hxx>
#include <Exo/VG.h>
using namespace CTK::Exo;
using namespace ::_;
#include <Exo/VG.hxx>
... GL header ...
#define NANOVG_GLES3_IMPLEMENTATION
#include <Exo/nanovg_gl.h>
#include "ExoArena.hpp"
```

## Build (Emscripten, wasm64)
```
. /path/to/emsdk/emsdk_env.sh
em++ -std=c++23 -O2 -sMEMORY64=1 \
  -I<CrabsTK> -I<ASCIICrabs> -I<AStarStarship> -I<CrabsTK>/Exo \
  ExoArenaWeb.cpp -o exoarena.html \
  -sUSE_GLFW=3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 \
  -sALLOW_MEMORY_GROWTH=1 \
  --preload-file=<Roboto-Regular.ttf>@Roboto-Regular.ttf
```

## Run
```
python3 -m http.server 8099
# open http://127.0.0.1:8099/exoarena.html
```

## Why wasm64
`-sMEMORY64=1` makes Emscripten's ABI LP64 (pointer=8, long=8), so
`_CPUSize` is `_CPU8Byte` and the tested 64-bit ASCIICrabs branches are used.
wasm32 requires finishing the 32-bit branch port in ASCIICrabs.
