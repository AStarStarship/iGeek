# Copyright AStarship <https://astarship.net>.
"""MazeWorld browser demo (Flask, port 5006).

The maze layout, wall collision and A* solve come from the native MazeWorld
slice compiled by maze_bridge.py (verbatim MazeWorld.h/.hxx pure-struct code).
The MazeEnv reward/observation layer is NOT compiled; this server mirrors its
reward arithmetic in Python and says so in /api/manifest.

Run: python Worlds/Maze2D/maze_server.py
Sessions are in-memory, single-process, one native MazeWorld per cookie.
"""
from collections import OrderedDict
from pathlib import Path
import secrets
import threading

from flask import Flask, g, jsonify, request, send_from_directory

try:
  from .maze_bridge import Engine
except ImportError:
  from maze_bridge import Engine

Directory = Path(__file__).resolve().parent

# Native MazeWorld::Dx / Dy order: Up, Down, Left, Right.
Directions = {
    "up": (0, -1),
    "down": (0, 1),
    "left": (-1, 0),
    "right": (1, 0),
}

MinCols, MaxCols = 7, 201
MinRows, MaxRows = 7, 61
MaxSeed = 2 ** 53 - 1
MaxSessions = 512


def CreateMaze(cols, rows, seed):
  handle = Engine.library.MazeBridgeCreate(cols, rows, seed)
  if not handle:
    raise RuntimeError("Native maze allocation failed.")
  return handle


class AMazeSession:
  """One browser's maze: native grid handle plus mirrored env bookkeeping."""

  def __init__(self, cols, rows, seed):
    self.cols = cols
    self.rows = rows
    self.seed = seed
    self.handle = CreateMaze(cols, rows, seed)
    self.player_x = 1
    self.player_y = 1
    self.goal_x = cols - 2
    self.goal_y = rows - 2
    self.steps = 0
    self.path_len = None
    self.last_reward = 0.0
    self.message = "Reach the exit. Arrow keys or WASD to move."

  def Close(self):
    if self.handle:
      Engine.library.MazeBridgeDestroy(self.handle)
      self.handle = None

  def Regenerate(self):
    """Recreate the native maze from the same seed (clears any A* path)."""
    self.Close()
    self.handle = CreateMaze(self.cols, self.rows, self.seed)

  def Tile(self, x, y):
    return chr(Engine.library.MazeBridgeTile(self.handle, x, y))

  def IsWall(self, x, y):
    return bool(Engine.library.MazeBridgeIsWall(self.handle, x, y))

  def Grid(self):
    return "".join(
        chr(Engine.library.MazeBridgeTile(self.handle, x, y))
        for y in range(self.rows) for x in range(self.cols))

  def Move(self, direction):
    dx, dy = Directions[direction]
    nx, ny = self.player_x + dx, self.player_y + dy
    self.steps += 1
    if nx < 0 or ny < 0 or nx >= self.cols or ny >= self.rows or self.IsWall(nx, ny):
      # Native MazeEnv::Step wall branch: reward -0.05, position unchanged.
      self.last_reward = -0.05
      self.message = "Blocked by a wall (native reward -0.05)."
      return
    self.player_x, self.player_y = nx, ny
    if nx == self.goal_x and ny == self.goal_y:
      self.last_reward = 1.0
      self.message = "Exit reached (native reward +1.0)."
    else:
      self.last_reward = -0.01
      self.message = "Moved (native step reward -0.01)."

  def Solve(self):
    length = Engine.library.MazeBridgeSolve(
        self.handle, self.player_x, self.player_y, self.goal_x, self.goal_y)
    if length <= 0:
      raise ValueError("No path from the player to the exit.")
    self.path_len = int(length)
    self.message = "A* shortest path from the player (native solver, not learned)."

  def Reset(self):
    self.Regenerate()
    self.player_x = self.player_y = 1
    self.steps = 0
    self.path_len = None
    self.last_reward = 0.0
    self.message = "Maze reset. Same seed, same layout."

  def State(self):
    return {
        "cols": self.cols,
        "rows": self.rows,
        "seed": self.seed,
        "grid": self.Grid(),
        "player": [self.player_x, self.player_y],
        "goal": [self.goal_x, self.goal_y],
        "done": self.player_x == self.goal_x and self.player_y == self.goal_y,
        "steps": self.steps,
        "path_len": self.path_len,
        "last_reward": self.last_reward,
        "message": self.message,
    }


def CreateApp():
  app = Flask(__name__, static_folder=None)
  app.config["MAX_CONTENT_LENGTH"] = 4096
  sessions = OrderedDict()
  lock = threading.RLock()

  @app.after_request
  def Headers(response):
    response.headers["Cache-Control"] = "no-store"
    response.headers["X-Content-Type-Options"] = "nosniff"
    return response

  def Dispatch():
    """Resolve (or create) this browser's session under the session lock."""
    token = request.cookies.get("maze_session")
    if token not in sessions:
      token = secrets.token_hex(16)
      sessions[token] = AMazeSession(31, 21, 42)
      while len(sessions) > MaxSessions:
        _, evicted = sessions.popitem(last=False)
        evicted.Close()
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
  @app.get("/maze.html")
  def Index():
    return send_from_directory(Directory, "maze.html")

  @app.get("/api/state")
  def State():
    with lock:
      token, maze = Dispatch()
      response = jsonify(maze.State())
      response.set_cookie("maze_session", token, httponly=True,
                          samesite="Strict", secure=request.is_secure)
      return response

  @app.post("/api/new")
  def New():
    with lock:
      token, maze = Dispatch()
      payload = Payload()
      if set(payload) != {"cols", "rows", "seed"}:
        raise ValueError("Expected exactly cols, rows and seed.")
      cols = Integer(payload["cols"], MinCols, MaxCols, "cols")
      rows = Integer(payload["rows"], MinRows, MaxRows, "rows")
      seed = Integer(payload["seed"], 0, MaxSeed, "seed")
      if cols % 2 == 0 or rows % 2 == 0:
        raise ValueError("cols and rows must be odd (the native grid requires it).")
      fresh = AMazeSession(cols, rows, seed)
      fresh.message = "New maze generated by the native recursive backtracker."
      sessions[token] = fresh
      maze.Close()
      response = jsonify(fresh.State())
      response.set_cookie("maze_session", token, httponly=True,
                          samesite="Strict", secure=request.is_secure)
      return response

  @app.post("/api/action")
  def Action():
    with lock:
      token, maze = Dispatch()
      payload = Payload()
      action = payload.get("action")
      if type(action) is not str:
        raise ValueError("Expected a string action.")
      if action == "move":
        if set(payload) != {"action", "direction"} or type(payload["direction"]) is not str:
          raise ValueError("move requires exactly action and direction.")
        if payload["direction"] not in Directions:
          raise ValueError("direction must be up, down, left or right.")
        maze.Move(payload["direction"])
      elif action == "solve":
        if set(payload) != {"action"}:
          raise ValueError("solve takes no extra fields.")
        maze.Solve()
      elif action == "reset":
        if set(payload) != {"action"}:
          raise ValueError("reset takes no extra fields.")
        maze.Reset()
      else:
        raise ValueError("Unknown action.")
      response = jsonify(maze.State())
      response.set_cookie("maze_session", token, httponly=True,
                          samesite="Strict", secure=request.is_secure)
      return response

  @app.get("/api/manifest")
  def Manifest():
    return jsonify({
        "name": "MazeWorld browser demo",
        "version": "1.0",
        "mode": "browser-human",
        "engine": "C ABI compiled from verbatim MazeWorld.h/.hxx pure-struct slice "
                  "(seeded recursive-backtracker generation + A*).",
        "policy": "Human movement; solve runs the deterministic native A*, not learning.",
        "native_gym_difference":
            "MazeEnv::Step/Observe are not compiled. Collision and A* are native; "
            "reward (-0.05 wall, -0.01 step, +1.0 exit) and step counting are "
            "mirrored in Python and are not a native reward equivalence proof.",
        "observation": {
            "cols": "odd integer 7..201",
            "rows": "odd integer 7..61",
            "seed": "generation seed, 0..2^53-1; same seed reproduces the layout",
            "grid": "cols*rows row-major string; # wall, . floor, S start, E exit, * A* path",
            "player": "[x, y]; starts at [1, 1]",
            "goal": "[cols-2, rows-2]",
            "done": "boolean: player stands on the exit",
            "steps": "move attempts since new/reset, including blocked bumps",
            "path_len": "null until solve; A* path length in cells including start and exit",
            "last_reward": "float mirroring native MazeEnv step reward",
            "message": "human-readable last action status"},
        "actions": {
            "GET /api/state": "Returns observation",
            "POST /api/action": [
                {"action": "move", "direction": "up|down|left|right"},
                {"action": "solve", "effect": "stamps the native A* path from the player"},
                {"action": "reset", "effect": "same seed, player back at start, path cleared"}],
            "POST /api/new": {"cols": "odd 7..201", "rows": "odd 7..61",
                              "seed": "integer 0..2^53-1"},
            "GET /api/manifest": "This contract"},
        "responses": "Successful mutations return observation. Invalid JSON, types, "
                     "bounds, extra fields or unknown actions: HTTP 400 {error:string} "
                     "and no state mutation. Booleans are not integers.",
        "sessions": "Opaque HttpOnly SameSite=Strict cookie; in-memory single-process; "
                    "restart clears mazes; up to 512 sessions, oldest evicted.",
        "routes": ["/", "/maze.html", "/api/state", "/api/action", "/api/new",
                   "/api/manifest"]})

  return app


app = CreateApp()

if __name__ == "__main__":
  app.run(host="0.0.0.0", port=5006, debug=False, threaded=True)
