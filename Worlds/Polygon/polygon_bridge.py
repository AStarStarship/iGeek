# Copyright AStarship <https://astarship.net>.
"""ctypes bridge to the surviving PolygonWorld genetic-algorithm code.

Only GeneticPolygon.h / GeneticPolygon.hxx survive in Worlds/Polygon/ (the
Agent/Virus/Bezier/Host/etc. files were intentionally deleted and are NOT
reconstructed). The pure GA (TGenome + AGenomePopulation) is self-contained, so
this bridge compiles that verbatim slice into a shared library, mirroring the
Maze/Sudoku bridge pattern.

Not compiled: APolygonEnv / Environment (the env layer depends on the deleted
Gym/Manifest scaffolding). The target-shape handling and generation stepping are
driven from Python against the native Evolve()/Fitness()/Observe() primitives.

IMPORTANT (documented, not "fixed"): the surviving AGenomePopulation::Fitness()
scores the stored best_ genome, and SelectIndex() calls Fitness(target) for
every tournament candidate, so candidate fitness is constant and tournament
selection is effectively random. The result is that BestFitness() freezes after
generation 0 while the population drifts. This demo surfaces that real native
behaviour faithfully; it does not modify the surviving logic (see README.md).

The bridge reads the population for rendering via a `#define private public`
introspection shim applied only inside the generated bridge TU — the shipped
headers are untouched.

Build products live exclusively in TMPDIR (Hermes scratch) or POLYGON_SCRATCH.
"""
import ctypes
import os
import subprocess
import tempfile
from pathlib import Path

Directory = Path(__file__).resolve().parent

StructStart = "enum {\n  PolygonWorldPoints = 16,"
StructEnd = "/* The Environment: owns the population"
HxxStart = "ISC AGenomePopulation::rand32() {"
HxxEnd = "/* --- APolygonEnv"

TypeHeader = """\
// Copyright AStarship <https://astarship.net>.
#pragma once
#include <cstdint>
namespace _ {
typedef char CHA;
typedef bool BOL;
typedef int32_t ISC;
typedef uint64_t IUD;
typedef float FPC;
typedef double FPD;
}
namespace iGeek {}
"""

BridgeSource = """\
// Copyright AStarship <https://astarship.net>.
#include <cstdint>
// Introspection shim ONLY for reading the population for rendering; the
// shipped native headers are not modified.
#define private public
#include "PolygonSlice.h"
#undef private
#include <cstddef>
extern "C" {

void* PolyBridgeCreate(unsigned long long seed) {
  return new PolygonWorld::AGenomePopulation(seed);
}
void PolyBridgeDestroy(void* p) {
  delete static_cast<PolygonWorld::AGenomePopulation*>(p);
}
void PolyBridgeReset(void* p, unsigned long long seed) {
  static_cast<PolygonWorld::AGenomePopulation*>(p)->Reset(seed);
}
double PolyBridgeEvolve(void* p) {
  return static_cast<PolygonWorld::AGenomePopulation*>(p)->Evolve();
}
double PolyBridgeBestFitness(void* p) {
  return static_cast<PolygonWorld::AGenomePopulation*>(p)->BestFitness();
}
double PolyBridgeLastFitness(void* p) {
  return static_cast<PolygonWorld::AGenomePopulation*>(p)->LastGenerationFitness();
}
int PolyBridgeGeneration(void* p) {
  return static_cast<PolygonWorld::AGenomePopulation*>(p)->Generation();
}
int PolyBridgePoints(void) { return PolygonWorld::PolygonWorldPoints; }
int PolyBridgePopulation(void) { return PolygonWorld::PolygonWorldPopulation; }
// Best genome (x,y interleaved), points*2 floats.
void PolyBridgeObserve(void* p, float* out) {
  static_cast<PolygonWorld::AGenomePopulation*>(p)->Observe(out);
}
// Entire population (population*points*2 floats) for rendering.
void PolyBridgeDump(void* p, float* out) {
  auto* pop = static_cast<PolygonWorld::AGenomePopulation*>(p);
  int n = PolygonWorld::PolygonWorldPoints;
  for (int i = 0; i < PolygonWorld::PolygonWorldPopulation; ++i) {
    for (int j = 0; j < n; ++j) {
      out[(i * n + j) * 2] = pop->pop_[i].x_[j];
      out[(i * n + j) * 2 + 1] = pop->pop_[i].y_[j];
    }
  }
}
// Fitness of the stored best genome against the supplied target polygon.
double PolyBridgeFitness(void* p, const double* xs, const double* ys) {
  PolygonWorld::TGenome target;
  int n = PolygonWorld::PolygonWorldPoints;
  for (int j = 0; j < n; ++j) {
    target.x_[j] = (float)xs[j];
    target.y_[j] = (float)ys[j];
  }
  return static_cast<PolygonWorld::AGenomePopulation*>(p)->Fitness(target);
}
}
"""


class ANativePolygon:
  def __init__(self):
    header = (Directory / "GeneticPolygon.h").read_text()
    body = (Directory / "GeneticPolygon.hxx").read_text()
    start = header.index(StructStart)
    end = header.index(StructEnd)
    struct_slice = header[start:end]
    method_start = body.index(HxxStart)
    method_end = body.index(HxxEnd, method_start)
    method_slice = body[method_start:method_end]
    if "class AGenomePopulation" not in struct_slice:
      raise RuntimeError("GeneticPolygon.h slice missing the population class.")
    if "AGenomePopulation::Evolve" not in method_slice:
      raise RuntimeError("GeneticPolygon.hxx slice missing Evolve().")

    root = Path(os.environ.get("POLYGON_SCRATCH", os.environ.get(
        "TMPDIR", str(Path.home() / ".cache" / "polygon"))))
    root.mkdir(parents=True, exist_ok=True)
    self.build_dir = Path(tempfile.mkdtemp(prefix="polygon-native-", dir=root))

    combined = (
        TypeHeader
        + "\nnamespace PolygonWorld {\nusing namespace ::_;\nusing namespace ::iGeek;\n"
        + struct_slice
        + "\n" + method_slice + "\n}\n"
    )
    (self.build_dir / "PolygonSlice.h").write_text(combined)
    (self.build_dir / "polygon_bridge.cpp").write_text(BridgeSource)

    self.library_path = self.build_dir / "polygon_bridge.so"
    subprocess.run(
        ["g++", "-std=c++17", "-O2", "-shared", "-fPIC",
         str(self.build_dir / "polygon_bridge.cpp"),
         "-I", str(self.build_dir),
         "-o", str(self.library_path)],
        check=True, capture_output=True, text=True)
    self.library = ctypes.CDLL(str(self.library_path))

    self.points = self.library.PolyBridgePoints()
    self.population = self.library.PolyBridgePopulation()
    self.library.PolyBridgeCreate.argtypes = [ctypes.c_ulonglong]
    self.library.PolyBridgeCreate.restype = ctypes.c_void_p
    self.library.PolyBridgeDestroy.argtypes = [ctypes.c_void_p]
    self.library.PolyBridgeReset.argtypes = [ctypes.c_void_p, ctypes.c_ulonglong]
    self.library.PolyBridgeEvolve.argtypes = [ctypes.c_void_p]
    self.library.PolyBridgeEvolve.restype = ctypes.c_double
    self.library.PolyBridgeBestFitness.argtypes = [ctypes.c_void_p]
    self.library.PolyBridgeBestFitness.restype = ctypes.c_double
    self.library.PolyBridgeLastFitness.argtypes = [ctypes.c_void_p]
    self.library.PolyBridgeLastFitness.restype = ctypes.c_double
    self.library.PolyBridgeGeneration.argtypes = [ctypes.c_void_p]
    self.library.PolyBridgeGeneration.restype = ctypes.c_int
    self.library.PolyBridgeObserve.argtypes = [
        ctypes.c_void_p, ctypes.POINTER(ctypes.c_float)]
    self.library.PolyBridgeDump.argtypes = [
        ctypes.c_void_p, ctypes.POINTER(ctypes.c_float)]
    self.library.PolyBridgeFitness.argtypes = [
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double)]
    self.library.PolyBridgeFitness.restype = ctypes.c_double

  def Best(self, handle):
    out = (ctypes.c_float * (self.points * 2))()
    self.library.PolyBridgeObserve(handle, out)
    return list(out)

  def Dump(self, handle):
    count = self.population * self.points * 2
    out = (ctypes.c_float * count)()
    self.library.PolyBridgeDump(handle, out)
    return list(out)

  def Fitness(self, handle, xs, ys):
    xs_array = (ctypes.c_double * self.points)(*xs)
    ys_array = (ctypes.c_double * self.points)(*ys)
    return self.library.PolyBridgeFitness(handle, xs_array, ys_array)


Engine = ANativePolygon()
