# PolygonWorld

Headless genetic-algorithm polygon shape-matching. The population is a set of
closed 16-point polygons in the unit square, evolved by tournament selection,
point-wise crossover, and bounded mutation toward a target shape (default: a
regular 16-gon, radius 0.8).

`APolygonEnv : public Environment` exposes the GA as an RL-style contract:
observation is the best genome's (x, y) points, the four actions are
{evolve, mutate, crossover, re-evaluate}, reward is the delta of best fitness,
and the episode ends on convergence (fitness >= 0.99) or the 200-step budget.
Deterministic given a seed, so a GA run is directly comparable to an RL policy
trained on the same objective — the intended baseline for the genetic-vs-RL
comparison.

The original 2014 Java/Swing implementation (JFrame, Color, Graphics,
BufferedImage, Bezier panels, virus/leukocyte host) was removed; this is the
headless kernel it was trying to be.

## Browser demo (WebGL, port 5007)

Interactive GA demo: pick `circle`/`square`/`triangle`/`star`, press Evolve, and
watch the population (faint), the best genome (cyan) and the target (amber
outline) on a raw WebGL canvas (no CDN, no framework).

**Native vs port — read this before citing convergence.** `polygon_bridge.py`
compiles the surviving `GeneticPolygon.h`/`.hxx` GA slice through ctypes, and it
builds cleanly, but the surviving implementation cannot evolve. Measured
(`GET /api/native-probe`, and `test_polygon.py::test_native_ga_probe_documents_the_inert_defect`):

1. `rand01()` never advances `rand_state_` (only `rand32()` does), so every
   control point of every genome gets the same value — the population is one
   degenerate point.
2. `Evolve()` calls `Fitness(best_)`, passing the best genome as the *target*, so
   fitness is always a self-comparison = `1.0`.
3. `SelectIndex()` calls `Fitness(target)` for every tournament candidate, so all
   candidates compare equal and selection is uniform random.

Result: `best_fitness` is `1.0` from generation 1 and the population is
byte-identical after any number of generations. The **interactive demo therefore
runs `polygon_ga.py`, a Python port** of the documented GA semantics (same 16
points / 32 genomes / elitism 2 / tournament 3 / mutation 0.15 / converge 0.99 /
200 generations) with those three defects corrected. It is labelled `port GA` in
the UI and `engine: python-port` in the manifest. The native bridge is kept and
exercised as evidence, not as the demo engine.

Files: `polygon_bridge.py` (native, inert), `polygon_ga.py` (port used by the
demo), `polygon_server.py` (Flask), `polygon.html` (WebGL), `test_polygon.py`
(21 retained tests).

```
cd iGeek
python3 -m venv .venv-polygon && . .venv-polygon/bin/activate
pip install flask pytest
python -m pytest Worlds/Polygon/test_polygon.py -v
python Worlds/Polygon/polygon_server.py     # http://127.0.0.1:5007/
```

Controls: `Space` evolve x1, `E` evolve x10, `R` reset, `N` new seed; the target
dropdown restarts the run with the same seed and the new target.

## License

Copyright [AStarship™](https://astarship.net).
