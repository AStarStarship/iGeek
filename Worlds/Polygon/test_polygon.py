# Copyright AStarship <https://astarship.net>.
"""Retained tests for the PolygonWorld browser demo.

Run: python -m pytest Worlds/Polygon/test_polygon.py -v
Covers the port GA, the Flask HTTP contract, session isolation, input
validation, and the measured inert behaviour of the compiled native GA.
"""
from pathlib import Path
import sys

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parent))

from polygon_ga import AGaPopulation, MaxGenerations, Points, Population  # noqa: E402
from polygon_server import CreateApp  # noqa: E402


@pytest.fixture()
def client():
  app = CreateApp()
  app.config.update(TESTING=True)
  with app.test_client() as test_client:
    yield test_client


def State(client):
  response = client.get("/api/state")
  assert response.status_code == 200
  return response.get_json()


def test_manifest_labels_the_port(client):
  payload = client.get("/api/manifest").get_json()
  assert payload["name"] == "PolygonWorld browser demo"
  assert payload["engine"] == "python-port"
  assert "native_difference" in payload
  assert payload["constants"] == {"points": 16, "population": 32, "elitism": 2,
                                  "tournament": 3, "mutate_rate": 0.2,
                                  "mutate_step": 0.15, "converge": 0.99,
                                  "max_generations": 200}


def test_initial_state_shape(client):
  state = State(client)
  assert state["generation"] == 0
  assert state["max_generations"] == MaxGenerations
  assert state["seed"] == 1
  assert state["target_name"] == "circle"
  assert 0.0 <= state["best_fitness"] <= 1.0
  assert len(state["best"]) == Points * 2
  assert len(state["target"]) == Points * 2
  assert len(state["population"]) == Population
  assert all(len(genome) == Points * 2 for genome in state["population"])
  assert state["done"] is False


def test_evolve_increases_best_fitness(client):
  before = State(client)
  after = client.post("/api/action", json={"action": "evolve", "generations": 20}).get_json()
  assert after["generation"] == 20
  assert after["best_fitness"] > before["best_fitness"]
  assert after["last_reward"] == pytest.approx(after["best_fitness"] - before["best_fitness"],
                                               abs=1e-5)


def test_ga_port_is_seed_deterministic():
  first = AGaPopulation(seed=5, target="triangle")
  second = AGaPopulation(seed=5, target="triangle")
  first.Evolve(25)
  second.Evolve(25)
  assert first.best == second.best
  assert first.best_fitness == second.best_fitness


def test_same_seed_same_result_over_http(client):
  other = CreateApp().test_client()
  for test_client in (client, other):
    test_client.post("/api/new", json={"target": "star", "seed": 11})
    test_client.post("/api/action", json={"action": "evolve", "generations": 10})
  assert client.get("/api/state").get_json()["best"] == \
      other.get("/api/state").get_json()["best"]


def test_reset_restores_generation_zero(client):
  client.post("/api/action", json={"action": "evolve", "generations": 5})
  reset = client.post("/api/action", json={"action": "reset"}).get_json()
  assert reset["generation"] == 0
  assert reset["last_reward"] == 0.0
  assert reset["done"] is False


def test_generation_budget_terminates(client):
  state = State(client)
  for _ in range(4):
    state = client.post("/api/action",
                        json={"action": "evolve", "generations": 50}).get_json()
  assert state["generation"] == MaxGenerations
  assert state["done"] is True
  assert state["best_fitness"] >= 0.95, "200 generations should nearly match the target"
  # done is terminal: further evolve calls do not advance.
  again = client.post("/api/action", json={"action": "evolve", "generations": 1}).get_json()
  assert again["generation"] == MaxGenerations


def test_new_target_changes_shape(client):
  state = client.post("/api/new", json={"target": "square", "seed": 2}).get_json()
  assert state["target_name"] == "square"
  assert state["generation"] == 0
  circle = client.post("/api/new", json={"target": "circle", "seed": 2}).get_json()
  assert circle["target"] != state["target"]


def test_native_ga_probe_documents_the_inert_defect(client):
  probe = client.get("/api/native-probe").get_json()
  assert probe["generations"] == 10
  assert probe["best_fitness_after"] == 1.0
  assert probe["population_changed"] is False


@pytest.mark.parametrize("path,payload", [
    ("/api/action", {"action": "evolve"}),
    ("/api/action", {"action": "evolve", "generations": 0}),
    ("/api/action", {"action": "evolve", "generations": 51}),
    ("/api/action", {"action": "evolve", "generations": True}),
    ("/api/action", {"action": "evolve", "generations": 1, "seed": 2}),
    ("/api/action", {"action": "mutate"}),
    ("/api/action", {}),
    ("/api/new", {"target": "hexagon", "seed": 1}),
    ("/api/new", {"target": "circle", "seed": -1}),
    ("/api/new", {"target": "circle", "seed": 1, "cols": 2}),
    ("/api/new", {"target": 3, "seed": 1}),
])
def test_invalid_requests_rejected_without_mutation(client, path, payload):
  before = State(client)
  response = client.post(path, json=payload)
  assert response.status_code == 400, response.get_json()
  assert "error" in response.get_json()
  assert State(client) == before


def test_sessions_are_isolated(client):
  other = CreateApp().test_client()
  client.post("/api/new", json={"target": "triangle", "seed": 9})
  client.post("/api/action", json={"action": "evolve", "generations": 3})
  mine = client.get("/api/state").get_json()
  theirs = other.get("/api/state").get_json()
  assert mine["target_name"] == "triangle" and mine["generation"] == 3
  assert theirs["target_name"] == "circle" and theirs["generation"] == 0
