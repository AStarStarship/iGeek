# Copyright AStarship <https://astarship.net>.
"""Retained tests for the SpaceWorld browser demo.

Run: python -m pytest Worlds/Space/test_space.py -v
Drives the native physics bridge and the real Flask HTTP contract.
"""
from pathlib import Path
import math
import sys

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parent))

from space_server import CreateApp  # noqa: E402


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


def test_manifest(client):
  payload = client.get("/api/manifest").get_json()
  assert payload["name"] == "SpaceWorld browser demo"
  assert payload["engine"] == "native"
  assert "SpaceEnv is not compiled" in payload["native_gym_difference"]
  assert payload["routes"] == ["/", "/space.html", "/api/state", "/api/spec",
                               "/api/action", "/api/new", "/api/manifest"]


def test_initial_state_is_seeded_tumble_at_rest(client):
  state = State(client)
  assert state["position"] == [0.0, 0.0, 0.0]
  assert state["velocity"] == [0.0, 0.0, 0.0]
  assert state["quaternion"] == [0.0, 0.0, 0.0, 1.0]
  assert state["time"] == 0.0
  assert state["total_steps"] == 0
  assert state["spin"] > 0.0, "SpaceEnv::Reset mirror starts with a seeded tumble"
  assert state["seed"] == 1


def test_spec_is_native_and_consistent(client):
  spec = client.get("/api/spec").get_json()
  assert spec["mass"] == 1000.0
  assert spec["half"] == [0.5, 0.5, 1.0]
  assert spec["dt"] == pytest.approx(1.0 / 60.0)
  assert spec["inertia"][0] == pytest.approx(104.1667, abs=1e-3)
  assert spec["inertia"][2] == pytest.approx(41.6667, abs=1e-3)
  assert len(spec["thrusters"]) == 24
  assert all(len(t) == 6 for t in spec["thrusters"])
  assert all(v > 0 for v in spec["caps"]["lin_force"])
  assert all(v > 0 for v in spec["caps"]["ang_torque"])


def test_force_burn_integrates_native_physics(client):
  state = client.post("/api/action", json={
      "action": "burn", "force": [0, 0, 50], "torque": [0, 0, 0],
      "steps": 60}).get_json()
  assert state["total_steps"] == 60
  assert state["time"] == pytest.approx(1.0)
  assert state["velocity"][2] == pytest.approx(0.05, abs=1e-3)
  assert state["position"][2] > 0
  assert state["burns"] == 1


def test_torque_burn_changes_angular_velocity(client):
  before = State(client)
  after = client.post("/api/action", json={
      "action": "burn", "force": [0, 0, 0], "torque": [5, 0, 0],
      "steps": 30}).get_json()
  assert after["angular"][0] != before["angular"][0]
  assert after["quaternion"] != before["quaternion"]


def test_stabilizer_damps_the_tumble(client):
  before = State(client)
  after = client.post("/api/action", json={"action": "stabilize", "steps": 600}).get_json()
  assert after["spin"] < before["spin"]
  assert after["spin"] < 0.05
  assert after["total_steps"] == 600


def test_burn_then_stabilize_keeps_state_valid(client):
  state = client.post("/api/action", json={
      "action": "burn", "force": [50, -50, 50], "torque": [5, -5, 5],
      "steps": 120}).get_json()
  state = client.post("/api/action", json={"action": "stabilize", "steps": 600}).get_json()
  assert all(math.isfinite(v) for v in state["position"] + state["velocity"] +
             state["angular"] + state["quaternion"])
  assert state["time"] > 10.0


def test_reset_and_new(client):
  client.post("/api/action", json={
      "action": "burn", "force": [0, 0, 50], "torque": [0, 0, 0], "steps": 60})
  reset = client.post("/api/action", json={"action": "reset"}).get_json()
  assert reset["time"] == 0.0 and reset["total_steps"] == 0
  assert reset["position"] == [0.0, 0.0, 0.0]
  other = client.post("/api/new", json={"seed": 4}).get_json()
  assert other["seed"] == 4
  assert other["spin"] != reset["spin"], "different seeds must give different tumbles"


@pytest.mark.parametrize("payload", [
    {"action": "burn", "force": [0, 0, 50], "torque": [0, 0, 0]},
    {"action": "burn", "force": [0, 0], "torque": [0, 0, 0], "steps": 10},
    {"action": "burn", "force": [0, 0, 50], "torque": [0, 0, 0], "steps": 0},
    {"action": "burn", "force": [0, 0, 50], "torque": [0, 0, 0], "steps": 121},
    {"action": "burn", "force": [0, 0, 5000], "torque": [0, 0, 0], "steps": 10},
    {"action": "burn", "force": [0, 0, "nan"], "torque": [0, 0, 0], "steps": 10},
    {"action": "burn", "force": [0, 0, True], "torque": [0, 0, 0], "steps": 10},
    {"action": "burn", "force": [0, 0, 50], "torque": [0, 0, 0], "steps": 10, "x": 1},
    {"action": "burn", "force": "0,0,50", "torque": [0, 0, 0], "steps": 10},
    {"action": "warp", "factor": 9},
    {"action": "stabilize"},
    {"action": "stabilize", "steps": 601},
    {"action": "stabilize", "steps": True},
    {},
])
def test_invalid_requests_rejected_without_mutation(client, payload):
  before = State(client)
  response = client.post("/api/action", json=payload)
  assert response.status_code == 400, response.get_json()
  assert "error" in response.get_json()
  assert State(client) == before


def test_invalid_seed_rejected(client):
  before = State(client)
  for seed in (-1, True, "1", 2 ** 53):
    response = client.post("/api/new", json={"seed": seed})
    assert response.status_code == 400
  assert State(client) == before


def test_sessions_are_isolated(client):
  other = CreateApp().test_client()
  client.post("/api/new", json={"seed": 9})
  client.post("/api/action", json={
      "action": "burn", "force": [0, 0, 50], "torque": [0, 0, 0], "steps": 60})
  mine = client.get("/api/state").get_json()
  theirs = other.get("/api/state").get_json()
  assert mine["seed"] == 9 and mine["time"] == pytest.approx(1.0)
  assert theirs["seed"] == 1 and theirs["time"] == 0.0
