# Copyright AStarship <https://astarship.net>.
"""Retained tests for the MazeWorld browser demo.

Run: python -m pytest Worlds/Maze2D/test_maze.py -v
The tests build and exercise the native bridge, then drive the real Flask app
through its HTTP contract with Flask's test client (no network listener).
"""
from collections import deque
from pathlib import Path
import sys

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parent))

from maze_server import CreateApp  # noqa: E402


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


def Cell(state, x, y):
  return state["grid"][y * state["cols"] + x]


def DirectionsBetween(x0, y0, x1, y1):
  if (x1, y1) == (x0, y0 - 1):
    return "up"
  if (x1, y1) == (x0, y0 + 1):
    return "down"
  if (x1, y1) == (x0 - 1, y0):
    return "left"
  if (x1, y1) == (x0 + 1, y0):
    return "right"
  raise AssertionError("cells are not adjacent")


def ShortestPath(state, start, goal):
  cols, rows = state["cols"], state["rows"]
  grid = state["grid"]
  queue = deque([start])
  parent: dict = {start: None}
  while queue:
    x, y = queue.popleft()
    if (x, y) == goal:
      break
    for nx, ny in ((x, y - 1), (x, y + 1), (x - 1, y), (x + 1, y)):
      if 0 <= nx < cols and 0 <= ny < rows and (nx, ny) not in parent:
        if grid[ny * cols + nx] != "#":
          parent[(nx, ny)] = (x, y)
          queue.append((nx, ny))
  assert goal in parent, "exit must be reachable"
  path = []
  cursor = goal
  while cursor is not None:
    path.append(cursor)
    cursor = parent[cursor]
  return list(reversed(path))


def test_manifest_contract(client):
  payload = client.get("/api/manifest").get_json()
  assert payload["name"] == "MazeWorld browser demo"
  assert payload["mode"] == "browser-human"
  assert "MazeEnv::Step/Observe are not compiled" in payload["native_gym_difference"]
  assert payload["routes"] == ["/", "/maze.html", "/api/state", "/api/action",
                               "/api/new", "/api/manifest"]


def test_initial_state(client):
  state = State(client)
  assert state["cols"] == 31 and state["rows"] == 21
  assert len(state["grid"]) == state["cols"] * state["rows"]
  assert state["player"] == [1, 1]
  assert state["goal"] == [state["cols"] - 2, state["rows"] - 2]
  assert Cell(state, 1, 1) == "S"
  assert Cell(state, state["cols"] - 2, state["rows"] - 2) == "E"
  assert state["done"] is False
  assert state["steps"] == 0
  assert state["path_len"] is None


def test_new_maze_is_seeded_and_deterministic(client):
  first = client.post("/api/new", json={"cols": 15, "rows": 11, "seed": 7}).get_json()
  second = client.post("/api/new", json={"cols": 15, "rows": 11, "seed": 7}).get_json()
  assert first["grid"] == second["grid"]
  assert first["seed"] == 7 and (first["cols"], first["rows"]) == (15, 11)
  other = client.post("/api/new", json={"cols": 15, "rows": 11, "seed": 8}).get_json()
  assert other["grid"] != first["grid"], "different seeds must differ"


def test_wall_bump_is_blocked_and_penalised(client):
  before = State(client)
  after = client.post("/api/action", json={"action": "move", "direction": "up"}).get_json()
  assert after["player"] == before["player"], "border wall must block"
  assert after["steps"] == before["steps"] + 1
  assert after["last_reward"] == -0.05


def test_open_move_advances_player(client):
  state = State(client)
  for direction, (dx, dy) in (("right", (1, 0)), ("down", (0, 1))):
    x, y = state["player"]
    if Cell(state, x + dx, y + dy) != "#":
      moved = client.post("/api/action",
                          json={"action": "move", "direction": direction}).get_json()
      assert moved["player"] == [x + dx, y + dy]
      assert moved["last_reward"] == -0.01
      return
  pytest.fail("start cell has no open neighbour")


def test_solve_stamps_native_path(client):
  solved = client.post("/api/action", json={"action": "solve"}).get_json()
  assert solved["path_len"] > 0
  assert "*" in solved["grid"]
  assert solved["grid"].count("*") == solved["path_len"] - 2  # S and E preserved


def test_walking_the_solved_path_reaches_exit(client):
  state = State(client)
  path = ShortestPath(state, tuple(state["player"]), tuple(state["goal"]))
  for (x0, y0), (x1, y1) in zip(path, path[1:]):
    state = client.post("/api/action", json={
        "action": "move", "direction": DirectionsBetween(x0, y0, x1, y1)}).get_json()
  assert state["done"] is True
  assert state["player"] == state["goal"]
  assert state["last_reward"] == 1.0


def test_reset_restores_start_and_clears_path(client):
  client.post("/api/action", json={"action": "solve"})
  reset = client.post("/api/action", json={"action": "reset"}).get_json()
  assert reset["player"] == [1, 1]
  assert reset["steps"] == 0
  assert reset["path_len"] is None
  assert "*" not in reset["grid"]
  assert reset["done"] is False


@pytest.mark.parametrize("path,payload", [
    ("/api/action", {"action": "move"}),
    ("/api/action", {"action": "move", "direction": "north"}),
    ("/api/action", {"action": "move", "direction": "up; rm -rf /"}),
    ("/api/action", {"action": "move", "direction": 1}),
    ("/api/action", {"action": "solve", "cell": 3}),
    ("/api/action", {"action": "teleport"}),
    ("/api/action", {}),
    ("/api/new", {"cols": 14, "rows": 11, "seed": 1}),
    ("/api/new", {"cols": 15, "rows": 11, "seed": -1}),
    ("/api/new", {"cols": 15, "rows": 11, "seed": True}),
    ("/api/new", {"cols": 15, "rows": 11}),
    ("/api/new", {"cols": 6, "rows": 11, "seed": 1}),
    ("/api/new", {"cols": 203, "rows": 11, "seed": 1}),
])
def test_invalid_requests_are_rejected_without_mutation(client, path, payload):
  before = State(client)
  response = client.post(path, json=payload)
  assert response.status_code == 400, response.get_json()
  assert "error" in response.get_json()
  assert State(client) == before


def test_non_json_body_is_rejected(client):
  response = client.post("/api/action", data="not json",
                         content_type="application/json")
  assert response.status_code == 400


def test_sessions_are_isolated(client):
  other = CreateApp().test_client()
  client.post("/api/new", json={"cols": 15, "rows": 11, "seed": 3})
  mine = client.get("/api/state").get_json()
  theirs = other.get("/api/state").get_json()
  assert mine["seed"] == 3 and (mine["cols"], mine["rows"]) == (15, 11)
  assert theirs["seed"] == 42 and (theirs["cols"], theirs["rows"]) == (31, 21)
  client.post("/api/action", json={"action": "solve"})
  assert other.get("/api/state").get_json()["path_len"] is None
