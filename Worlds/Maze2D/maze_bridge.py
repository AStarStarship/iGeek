# Copyright AStarship <https://astarship.net>.
"""ctypes bridge to the native MazeWorld generator + A* solver.

Compiles a verbatim slice of the pure MazeWorld struct (generation + A*) from
MazeWorld.h / MazeWorld.hxx into a shared library, exactly like the Sudoku
world's ANativeBoard bridge: full native header inclusion fails in Gym/ASCIICrabs
(Room.hpp path, CSizeMin, etc.), so we select the pure-board section with
explicit structural markers and compile that, unmodified.

NOT compiled: the MazeEnv / Environment layer (Step, Observe, reward). The
browser demo drives the player with the native IsWall/Tile/Solve and mirrors the
env reward semantics in Python; see README.md "native vs port" note.

Build products live exclusively in TMPDIR (Hermes scratch) or the configured
MAZE_SCRATCH directory.
"""
import ctypes
import os
import subprocess
import tempfile
from pathlib import Path

Directory = Path(__file__).resolve().parent

# Structural markers (verbatim, must remain unique).
StructStart = "struct MazeWorld {"
StructEnd = "/* The maze AS an Environment"
HxxNamespace = "namespace iGeek {"
HxxEnvMarker = "/* ------------------------------------------------------------------- MazeEnv */"

# Minimal type shim: the exact widths the native code expects (see
# ASCIICrabs/_ConfigHeader.h: CHA=char, BOL=bool, ISC=int32_t, IUD=uint64_t,
# FPC=float). Namespaced iGeek so the verbatim slice compiles unchanged.
TypeHeader = """\
// Copyright AStarship <https://astarship.net>.
#pragma once
#include <cstdint>
namespace iGeek {
typedef char CHA;
typedef bool BOL;
typedef int32_t ISC;
typedef uint64_t IUD;
typedef float FPC;

// Console sink for MazeWorld::Print(). The browser never calls Print(); this
// stub only exists so the verbatim slice links without the ASCIICrabs ROut.
struct ROut {
  ROut(const char*) {}
  ROut& operator<<(char) { return *this; }
  ROut& operator<<(int) { return *this; }
  ROut& operator<<(const char*) { return *this; }
  void NL() {}
};
}
"""

BridgeSource = """\
// Copyright AStarship <https://astarship.net>.
#include "MazeSlice.h"
extern "C" {

void* MazeBridgeCreate(int cols, int rows, unsigned long long seed) {
  return new iGeek::MazeWorld(cols, rows, seed);
}
void MazeBridgeDestroy(void* m) { delete static_cast<iGeek::MazeWorld*>(m); }
int MazeBridgeCols(void* m) { return static_cast<iGeek::MazeWorld*>(m)->Cols(); }
int MazeBridgeRows(void* m) { return static_cast<iGeek::MazeWorld*>(m)->Rows(); }
int MazeBridgeTile(void* m, int x, int y) {
  return (int)(unsigned char)static_cast<iGeek::MazeWorld*>(m)->Tile(x, y);
}
int MazeBridgeIsWall(void* m, int x, int y) {
  return static_cast<iGeek::MazeWorld*>(m)->IsWall(x, y) ? 1 : 0;
}
int MazeBridgeSolve(void* m, int sx, int sy, int gx, int gy) {
  return static_cast<iGeek::MazeWorld*>(m)->Solve(sx, sy, gx, gy);
}
}
"""


class ANativeMaze:
  def __init__(self):
    header = (Directory / "MazeWorld.h").read_text()
    body = (Directory / "MazeWorld.hxx").read_text()
    start = header.index(StructStart)
    end = header.index(StructEnd)
    struct_slice = header[start:end]
    ns_open = body.index(HxxNamespace) + len(HxxNamespace)
    ns_close = body.index(HxxEnvMarker, ns_open)
    method_slice = body[ns_open:ns_close]
    if StructStart not in struct_slice or "MazeWorld(" not in struct_slice:
      raise RuntimeError("MazeWorld.h slice did not contain the expected struct.")
    if "MazeWorld::Generate" not in method_slice:
      raise RuntimeError("MazeWorld.hxx slice did not contain the solver.")

    root = Path(os.environ.get("MAZE_SCRATCH", os.environ.get(
        "TMPDIR", str(Path.home() / ".cache" / "maze"))))
    root.mkdir(parents=True, exist_ok=True)
    self.build_dir = Path(tempfile.mkdtemp(prefix="maze-native-", dir=root))

    combined = (
        TypeHeader
        + "\nnamespace iGeek {\n" + struct_slice
        + "\n" + method_slice + "\n}\n"
    )
    (self.build_dir / "MazeSlice.h").write_text(combined)
    (self.build_dir / "maze_bridge.cpp").write_text(BridgeSource)

    self.library_path = self.build_dir / "maze_bridge.so"
    subprocess.run(
        ["g++", "-std=c++17", "-O2", "-shared", "-fPIC",
         str(self.build_dir / "maze_bridge.cpp"),
         "-I", str(self.build_dir),
         "-o", str(self.library_path)],
        check=True, capture_output=True, text=True)
    self.library = ctypes.CDLL(str(self.library_path))

    self.library.MazeBridgeCreate.argtypes = [
        ctypes.c_int, ctypes.c_int, ctypes.c_ulonglong]
    self.library.MazeBridgeCreate.restype = ctypes.c_void_p
    self.library.MazeBridgeDestroy.argtypes = [ctypes.c_void_p]
    self.library.MazeBridgeCols.argtypes = [ctypes.c_void_p]
    self.library.MazeBridgeCols.restype = ctypes.c_int
    self.library.MazeBridgeRows.argtypes = [ctypes.c_void_p]
    self.library.MazeBridgeRows.restype = ctypes.c_int
    self.library.MazeBridgeTile.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
    self.library.MazeBridgeTile.restype = ctypes.c_int
    self.library.MazeBridgeIsWall.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
    self.library.MazeBridgeIsWall.restype = ctypes.c_int
    self.library.MazeBridgeSolve.argtypes = [
        ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int]
    self.library.MazeBridgeSolve.restype = ctypes.c_int


Engine = ANativeMaze()
