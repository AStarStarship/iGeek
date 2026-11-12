# Copyright AStarship <https://astarship.net>.
"""PolygonWorld browser demo (Flask, port 5007).

IMPORTANT: the interactive GA is a Python port (polygon_ga.py), because the
surviving native AGenomePopulation is inert (see polygon_ga.py docstring and
GET /api/native-probe, which measures the compiled native bridge live). The
native bridge is still built and exercised here; the demo labels itself a port.

Run: python Worlds/Polygon/polygon_server.py
Sessions are in-memory, single-process GA populations, one per cookie.
"""
from collections import OrderedDict
from pathlib import Path
import secrets
import threading

from flask import Flask, jsonify, request, send_from_directory

try:
  from .polygon_ga import AGaPopulation, TargetNames, Points, Population
  from .polygon_bridge import Engine as NativeEngine
except ImportError:
  from polygon_ga import AGaPopulation, TargetNames, Points, Population
  from polygon_bridge import Engine as NativeEngine

Directory = Path(__file__).resolve().parent
MaxSeed = 2 ** 53 - 1
MaxSessions = 256


def NativeProbe():
  """Measured behaviour of the compiled surviving native GA (10 generations)."""
  handle = NativeEngine.library.PolyBridgeCreate(1)
  try:
    before = NativeEngine.Dump(handle)
    fitness_before = NativeEngine.library.PolyBridgeBestFitness(handle)
    for _ in range(10):
      NativeEngine.library.PolyBridgeEvolve(handle)
    after = NativeEngine.Dump(handle)
    fitness_after = NativeEngine.library.PolyBridgeBestFitness(handle)
    return {
        "generations": 10,
        "seed": 1,
        "best_fitness_before": round(fitness_before, 6),
        "best_fitness_after": round(fitness_after, 6),
        "population_changed": before != after,
        "note": "Surviving native GA: rand01 never advances the RNG, Fitness() "
                "self-compares against best_, and tournament candidates all score "
                "equal. Best fitness is 1.0 and the population is static.",
    }
  finally:
    NativeEngine.library.PolyBridgeDestroy(handle)


def CreateApp():
  app = Flask(__name__, static_folder=None)
  app.config["MAX_CONTENT_LENGTH"] = 4096
  sessions = OrderedDict()
  lock = threading.RLock()
  probe = NativeProbe()

  @app.after_request
  def Headers(response):
    response.headers["Cache-Control"] = "no-store"
    response.headers["X-Content-Type-Options"] = "nosniff"
    return response

  def Dispatch():
    token = request.cookies.get("polygon_session")
    if token not in sessions:
      token = secrets.token_hex(16)
      sessions[token] = AGaPopulation(seed=1, target="circle")
      while len(sessions) > MaxSessions:
        _, evicted = sessions.popitem(last=False)
        evicted.population = []
    sessions.move_to_end(token)
    return token, sessions[token]

  def Payload():
    payload = request.get_json(silent=True)
    if type(payload) is not dict:
      raise ValueError("Expected a JSON object.")
    return payload

  def Integer(value, low, high, name):
    if type(value) is not int or not low <= value <= high:
      raise ValueError(f"{name} must be an integer in {low}..{high}.")
    return value

  @app.errorhandler(ValueError)
  def Invalid(error):
    return jsonify(error=str(error)), 400

  @app.get("/")
  @app.get("/polygon.html")
  def Index():
    return send_from_directory(Directory, "polygon.html")

  @app.get("/api/state")
  def State():
    with lock:
      token, population = Dispatch()
      response = jsonify(population.State())
      response.set_cookie("polygon_session", token, httponly=True,
                          samesite="Strict", secure=request.is_secure)
      return response

  @app.post("/api/new")
  def New():
    with lock:
      token, population = Dispatch()
      payload = Payload()
      if set(payload) != {"target", "seed"} or type(payload.get("target")) is not str:
        raise ValueError("Expected exactly target and seed.")
      if payload["target"] not in TargetNames:
        raise ValueError("target must be one of: " + ", ".join(TargetNames) + ".")
      seed = Integer(payload["seed"], 0, MaxSeed, "seed")
      sessions[token] = AGaPopulation(seed=seed, target=payload["target"])
      fresh = sessions[token]
      response = jsonify(fresh.State())
      response.set_cookie("polygon_session", token, httponly=True,
                          samesite="Strict", secure=request.is_secure)
      return response

  @app.post("/api/action")
  def Action():
    with lock:
      token, population = Dispatch()
      payload = Payload()
      action = payload.get("action")
      if type(action) is not str:
        raise ValueError("Expected a string action.")
      if action == "evolve":
        if set(payload) != {"action", "generations"}:
          raise ValueError("evolve requires exactly action and generations.")
        generations = Integer(payload["generations"], 1, 50, "generations")
        population.Evolve(generations)
      elif action == "reset":
        if set(payload) != {"action"}:
          raise ValueError("reset takes no extra fields.")
        population.Reset()
      else:
        raise ValueError("Unknown action.")
      response = jsonify(population.State())
      response.set_cookie("polygon_session", token, httponly=True,
                          samesite="Strict", secure=request.is_secure)
      return response

  @app.get("/api/native-probe")
  def NativeProbeRoute():
    return jsonify(probe)

  @app.get("/api/manifest")
  def Manifest():
    return jsonify({
        "name": "PolygonWorld browser demo",
        "version": "1.0",
        "mode": "browser-human",
        "engine": "python-port",
        "engine_detail":
            "Surviving native AGenomePopulation is compiled and probed but inert; "
            "the interactive GA is polygon_ga.py, a port of the documented "
            "GeneticPolygon.h semantics with the three measured defects corrected.",
        "native_difference":
            "Native rand01() never advances its RNG, Fitness(best_) self-compares "
            "(always 1.0), and SelectIndex scores every candidate the same, so the "
            "native population never changes. /api/native-probe reports this live. "
            "The port is not native code and is labelled as a port in the UI.",
        "policy": "Human presses evolve/reset/new; selection is deterministic per seed.",
        "observation": {
            "generation": "integer 0..200",
            "max_generations": 200,
            "done": "boolean: fitness >= 0.99 or generation budget exhausted",
            "seed": "GA seed, 0..2^53-1",
            "target_name": "circle|square|triangle|star",
            "best_fitness": "0..1; port fitness = 1 - mean point distance / sqrt(8)",
            "last_reward": "delta of best fitness over the last evolve request",
            "best": "32 floats: 16 (x, y) control points",
            "target": "32 floats: 16 (x, y) target points",
            "population": "32 genomes x 32 floats for rendering the population",
            "message": "human-readable status"},
        "actions": {
            "GET /api/state": "Returns observation",
            "POST /api/action": [
                {"action": "evolve", "generations": "integer 1..50"},
                {"action": "reset", "effect": "same seed and target, generation 0"}],
            "POST /api/new": {"target": "circle|square|triangle|star",
                              "seed": "integer 0..2^53-1"},
            "GET /api/native-probe": "Live measurement of the inert native GA",
            "GET /api/manifest": "This contract"},
        "responses": "Successful mutations return observation. Invalid JSON, types, "
                     "bounds, extra fields or unknown actions: HTTP 400 {error:string} "
                     "and no state mutation. Booleans are not integers.",
        "sessions": "Opaque HttpOnly SameSite=Strict cookie; in-memory single-process; "
                    "restart clears populations; up to 256 sessions, oldest evicted.",
        "constants": {"points": Points, "population": Population, "elitism": 2,
                      "tournament": 3, "mutate_rate": 0.2, "mutate_step": 0.15,
                      "converge": 0.99, "max_generations": 200},
        "routes": ["/", "/polygon.html", "/api/state", "/api/action", "/api/new",
                   "/api/native-probe", "/api/manifest"]})

  return app


app = CreateApp()

if __name__ == "__main__":
  app.run(host="0.0.0.0", port=5007, debug=False, threaded=True)
