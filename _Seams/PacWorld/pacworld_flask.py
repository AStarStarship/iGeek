#!/usr/bin/env python3
"""PacWorld — headless Pac-Man env + Flask/WebSocket browser demo.

Faithful Python port of the no-stdlib ASCIICrabs `PacWorldEnv`
(Worlds/Pac/PacWorld.hxx) for visualization: same 64-bit LCG, same maze
generation, same ghost chase rules, same rewards. Serves a canvas demo that
streams live state over WebSocket (flask-socketio).

Port: deterministic LCG `rng = rng*A + C (mod 2^64)`, NextRand = (rng>>11) &
0xFFFFFFFF. See PacWorld.hxx GenerateMaze/PlaceGhosts/MoveGhost/Step.
"""

import math
import os
import threading

from flask import Flask, send_from_directory, jsonify
from flask_socketio import SocketIO, emit

# --- deterministic LCG (matches PacWorldEnv::NextRand exactly) ------------
_LCG_A = 6364136223846793005
_LCG_C = 1442695040888963407
_M64 = (1 << 64) - 1

# Cell codes (match PacWorldEnv::Cell*).
WALL, EMPTY, DOT, SUPERDOT, BONUS = 0, 1, 2, 3, 4
# Actions (match PacWorldEnv::Action*).
STAY, UP, DOWN, LEFT, RIGHT = 0, 1, 2, 3, 4
_ACTION_DIRS = {STAY: (0, 0), UP: (0, -1), DOWN: (0, 1), LEFT: (-1, 0), RIGHT: (1, 0)}
_OPPOSITE = {1: 2, 2: 1, 3: 4, 4: 3}

MAX_STEPS = 200
LIFES = 3
WEAK_STEPS = 12
STEP_PENALTY = -0.1
DOT_REWARD = 5
SUPERDOT_REWARD = 25
BONUS_REWARD = 500
GHOST_EAT_REWARD = 100
COLLISION_PENALTY = -10
WIN_BONUS = 50


class PacWorldEnv:
    """Python port of iGeek PacWorldEnv (grid, discrete, deterministic)."""

    def __init__(self, columns, rows, seed):
        self.cols = max(7, columns)
        self.rows = max(7, rows)
        self.seed = seed
        self.rng = seed if seed else 1
        self.grid = [EMPTY] * (self.cols * self.rows)
        self.pac_x, self.pac_y = 1, 1
        self.pac_dir = STAY
        self.ghost_x, self.ghost_y = [], []
        self.ghost_dir, self.ghost_weak = [], []
        self.ghost_count = 0
        self.steps = 0
        self.lives = LIFES
        self.score = 0
        self.done = False
        self.won = False
        self._generate_maze()
        self.reset()

    def _idx(self, x, y):
        return y * self.cols + x

    def _next_rand(self):
        self.rng = (self.rng * _LCG_A + _LCG_C) & _M64
        return (self.rng >> 11) & 0xFFFFFFFF

    def _is_wall(self, x, y):
        if x < 0 or y < 0 or x >= self.cols or y >= self.rows:
            return True
        return self.grid[y * self.cols + x] == WALL

    def _generate_maze(self):
        self.rng = self.seed if self.seed else 1
        for y in range(self.rows):
            for x in range(self.cols):
                border = x == 0 or y == 0 or x == self.cols - 1 or y == self.rows - 1
                if border:
                    self.grid[y * self.cols + x] = WALL
                    continue
                r = self._next_rand()
                self.grid[y * self.cols + x] = WALL if r < 0x40000000 else EMPTY
        # Carve guaranteed-open start + neighbours.
        self.pac_x, self.pac_y = 1, 1
        self.grid[self._idx(1, 1)] = EMPTY
        self.grid[self._idx(2, 1)] = EMPTY
        self.grid[self._idx(1, 2)] = EMPTY
        # Scatter dots in ~56% of open interior cells.
        for y in range(1, self.rows - 1):
            for x in range(1, self.cols - 1):
                i = self._idx(x, y)
                if self.grid[i] != EMPTY:
                    continue
                if x == self.pac_x and y == self.pac_y:
                    continue
                if self._next_rand() < 0x90000000:
                    self.grid[i] = DOT
        # A few superdots + a bonus (consumes RNG in scan order, as in C++).
        placed_super = placed_bonus = 0
        y = 1
        while y < self.rows - 1 and (placed_super < 2 or placed_bonus < 1):
            x = 1
            while x < self.cols - 1 and (placed_super < 2 or placed_bonus < 1):
                i = self._idx(x, y)
                if self.grid[i] != DOT:
                    x += 1
                    continue
                r = self._next_rand()
                if placed_super < 2 and r < 0x10000000:
                    self.grid[i] = SUPERDOT
                    placed_super += 1
                elif placed_bonus < 1 and r > 0xF0000000:
                    self.grid[i] = BONUS
                    placed_bonus += 1
                x += 1
            y += 1

    def _place_ghosts(self):
        self.ghost_x, self.ghost_y = [], []
        self.ghost_dir, self.ghost_weak = [], []
        self.ghost_count = 0
        for y in range(1, self.rows - 1):
            for x in range(1, self.cols - 1):
                if self.ghost_count >= 3:
                    break
                i = self._idx(x, y)
                if self.grid[i] != EMPTY:
                    continue
                if x == self.pac_x and y == self.pac_y:
                    continue
                if abs(x - self.pac_x) + abs(y - self.pac_y) < 4:
                    continue
                self.ghost_x.append(x)
                self.ghost_y.append(y)
                self.ghost_dir.append(STAY)
                self.ghost_weak.append(0)
                self.grid[i] = EMPTY
                self.ghost_count += 1
            if self.ghost_count >= 3:
                break
        if self.ghost_count == 0:  # tiny maps: guarantee one
            for y in range(1, self.rows - 1):
                for x in range(1, self.cols - 1):
                    i = self._idx(x, y)
                    if self.grid[i] == EMPTY and not (x == self.pac_x and y == self.pac_y):
                        self.ghost_x.append(x)
                        self.ghost_y.append(y)
                        self.ghost_dir.append(STAY)
                        self.ghost_weak.append(0)
                        self.ghost_count = 1
                        break

    def reset(self):
        self.rng = self.seed
        self._generate_maze()
        self.pac_x, self.pac_y = 1, 1
        self.pac_dir = STAY
        self.steps = 0
        self.lives = LIFES
        self.score = 0
        self.done = False
        self.won = False
        self._place_ghosts()

    def _move_ghost(self, g):
        if self.ghost_weak[g] > 0:
            self.ghost_weak[g] -= 1
        best_dir = self.ghost_dir[g]
        best_dist = 0x7FFFFFFF
        have_best = False
        for d in (1, 2, 3, 4):  # Up, Down, Left, Right (skip Stay)
            if d == _OPPOSITE.get(self.ghost_dir[g]):
                continue
            nx = self.ghost_x[g] + _ACTION_DIRS[d][0]
            ny = self.ghost_y[g] + _ACTION_DIRS[d][1]
            if self._is_wall(nx, ny):
                continue
            dist = abs(nx - self.pac_x) + abs(ny - self.pac_y)
            if not have_best or dist < best_dist:
                best_dist, best_dir, have_best = dist, d, True
        if not have_best:
            for d in (1, 2, 3, 4):
                if d == _OPPOSITE.get(self.ghost_dir[g]):
                    continue
                if not self._is_wall(self.ghost_x[g] + _ACTION_DIRS[d][0],
                                     self.ghost_y[g] + _ACTION_DIRS[d][1]):
                    best_dir = d
                    break
        self.ghost_dir[g] = best_dir
        self.ghost_x[g] += _ACTION_DIRS[best_dir][0]
        self.ghost_y[g] += _ACTION_DIRS[best_dir][1]

    def step(self, action):
        if self.done:
            return 0.0
        action = min(4, max(0, action))
        self.pac_dir = action
        if action != STAY:
            dx, dy = _ACTION_DIRS[action]
            nx, ny = self.pac_x + dx, self.pac_y + dy
            if not self._is_wall(nx, ny):
                self.pac_x, self.pac_y = nx, ny
        reward = STEP_PENALTY
        i = self._idx(self.pac_x, self.pac_y)
        cell = self.grid[i]
        if cell == DOT:
            reward += DOT_REWARD
            self.grid[i] = EMPTY
        elif cell == SUPERDOT:
            reward += SUPERDOT_REWARD
            self.grid[i] = EMPTY
            for g in range(self.ghost_count):
                self.ghost_weak[g] = WEAK_STEPS
        elif cell == BONUS:
            reward += BONUS_REWARD
            self.grid[i] = EMPTY
        for g in range(self.ghost_count):
            self._move_ghost(g)
        for g in range(self.ghost_count):
            if self.ghost_x[g] == self.pac_x and self.ghost_y[g] == self.pac_y:
                if self.ghost_weak[g] > 0:
                    reward += GHOST_EAT_REWARD
                    self.score += GHOST_EAT_REWARD
                    for _ in range(64):
                        rx = 1 + self._next_rand() % (self.cols - 2)
                        ry = 1 + self._next_rand() % (self.rows - 2)
                        if not self._is_wall(rx, ry) and not (rx == self.pac_x and ry == self.pac_y):
                            self.ghost_x[g], self.ghost_y[g] = rx, ry
                            break
                    self.ghost_weak[g] = 0
                else:
                    reward += COLLISION_PENALTY
                    self.lives -= 1
                    self.pac_x, self.pac_y = 1, 1
                    self.pac_dir = STAY
                    for gg in range(self.ghost_count):
                        self.ghost_weak[gg] = 0
                        self.ghost_dir[gg] = STAY
                    self._place_ghosts()
                    if self.lives <= 0:
                        self.done = True
                        self.won = False
                        return reward
        self.steps += 1
        any_dot = any(c in (DOT, SUPERDOT, BONUS) for c in self.grid)
        if not any_dot:
            self.done = True
            self.won = True
            reward += WIN_BONUS
        if self.steps >= MAX_STEPS:
            self.done = True
        return reward


class Game:
    """Single env + bot modes, guarded for cross-thread access."""

    def __init__(self, cols=11, rows=11, seed=7):
        self.env = PacWorldEnv(cols, rows, seed)
        self.mode = "human"  # human | greedy | random
        self.episode_score = 0
        self.lock = threading.Lock()

    def state(self):
        e = self.env
        return {
            "cols": e.cols, "rows": e.rows,
            "grid": list(e.grid),
            "pac": [e.pac_x, e.pac_y],
            "ghosts": [[e.ghost_x[g], e.ghost_y[g], e.ghost_weak[g] > 0]
                       for g in range(e.ghost_count)],
            "lives": e.lives, "score": e.score, "steps": e.steps,
            "done": e.done, "won": e.won,
            "mode": self.mode, "episode_score": self.episode_score,
        }

    def do_step(self, action):
        with self.lock:
            if self.env.done:
                self.env.reset()
                self.episode_score = 0
            r = self.env.step(action)
            self.episode_score += r
            return self.state()


def _greedy_action(env):
    """Demo bot: eat nearest edible thing, avoid strong ghosts, else wander.
    (Not the trained PPO policy — a readable heuristic for the viewer.)"""
    import collections
    e = env
    # If a weak ghost is reachable soon, head to it; else nearest dot/super/bonus.
    targets = []
    for g in range(e.ghost_count):
        if e.ghost_weak[g] > 0:
            targets.append((e.ghost_x[g], e.ghost_y[g], 3))
    for y in range(e.rows):
        for x in range(e.cols):
            c = e.grid[y * e.cols + x]
            if c in (DOT, SUPERDOT, BONUS):
                targets.append((x, y, c))
    # Strong-ghost cells to avoid as immediate moves.
    danger = {(e.ghost_x[g], e.ghost_y[g]) for g in range(e.ghost_count)
              if e.ghost_weak[g] == 0}

    def bfs(sx, sy):
        prev = {(sx, sy): None}
        q = collections.deque([(sx, sy)])
        while q:
            x, y = q.popleft()
            for d in (UP, DOWN, LEFT, RIGHT):
                nx, ny = x + _ACTION_DIRS[d][0], y + _ACTION_DIRS[d][1]
                if 0 <= nx < e.cols and 0 <= ny < e.rows and (nx, ny) not in prev \
                        and e.grid[ny * e.cols + nx] != WALL:
                    prev[(nx, ny)] = (x, y)
                    q.append((nx, ny))
        return prev

    best_key = None
    best_next = None  # first cell to step onto
    if targets:
        prev_map = bfs(e.pac_x, e.pac_y)
        for (tx, ty, prio) in targets:
            if (tx, ty) not in prev_map or (tx, ty) == (e.pac_x, e.pac_y):
                continue
            key = (-prio, abs(tx - e.pac_x) + abs(ty - e.pac_y))
            if best_key is None or key < best_key:
                node = (tx, ty)
                while prev_map[node] is not None and \
                        prev_map[node] != (e.pac_x, e.pac_y):
                    node = prev_map[node]
                # Only count connected targets (unconnected node ends at the
                # BFS root whose prev is None).
                if prev_map[node] == (e.pac_x, e.pac_y):
                    best_key, best_next = key, node
    # Decide the move toward the best target (or STAY).
    action = STAY
    if best_next is not None:
        tx, ty = best_next
        for d in (UP, DOWN, LEFT, RIGHT):
            nx, ny = e.pac_x + _ACTION_DIRS[d][0], e.pac_y + _ACTION_DIRS[d][1]
            if (nx, ny) == (tx, ty) and (nx, ny) not in danger:
                action = d
                break
    # Safety: if the chosen move walks into a strong ghost, prefer a safe one.
    if action != STAY:
        nx = e.pac_x + _ACTION_DIRS[action][0]
        ny = e.pac_y + _ACTION_DIRS[action][1]
        if (nx, ny) in danger:
            for d in (UP, DOWN, LEFT, RIGHT):
                mx, my = e.pac_x + _ACTION_DIRS[d][0], e.pac_y + _ACTION_DIRS[d][1]
                if 0 <= mx < e.cols and 0 <= my < e.rows \
                        and e.grid[my * e.cols + mx] != WALL and (mx, my) not in danger:
                    action = d
                    break
            else:
                action = STAY
    return action


def _random_action(env):
    import random
    return random.randint(0, 4)


# --- Flask app -------------------------------------------------------------
app = Flask(__name__)
app.config["SECRET_KEY"] = "pacworld-secret-key"
socketio = SocketIO(app, cors_allowed_origins="*", async_mode="threading")

game = Game(cols=11, rows=11, seed=7)
_stop = threading.Event()


@app.route("/")
def index():
    return send_from_directory(os.path.dirname(os.path.abspath(__file__)), "pacworld.html")


@app.route("/api/state")
def api_state():
    return jsonify(game.state())


@app.route("/action/<cmd>")
def action(cmd):
    mapping = {"stay": STAY, "up": UP, "down": DOWN, "left": LEFT, "right": RIGHT}
    if cmd not in mapping:
        return jsonify({"error": "unknown cmd"}), 400
    st = game.do_step(mapping[cmd])
    socketio.emit("state", st)
    return jsonify({"status": "ok", "action": cmd})


@app.route("/setmode/<mode>")
def setmode(mode):
    if mode not in ("human", "greedy", "random"):
        return jsonify({"error": "unknown mode"}), 400
    with game.lock:
        game.mode = mode
    return jsonify({"status": "ok", "mode": mode})


@app.route("/reset")
def reset():
    with game.lock:
        game.env.reset()
        game.episode_score = 0
    st = game.state()
    socketio.emit("state", st)
    return jsonify({"status": "ok"})


@socketio.on("connect")
def on_connect():
    emit("state", game.state())


@socketio.on("move")
def on_move(data):
    try:
        a = int(data.get("action", STAY))
    except (TypeError, ValueError):
        a = STAY
    st = game.do_step(a)
    emit("state", st)


@socketio.on("setmode")
def on_setmode(data):
    mode = data.get("mode", "human")
    if mode in ("human", "greedy", "random"):
        with game.lock:
            game.mode = mode


def bot_loop():
    import time
    while not _stop.is_set():
        if game.mode in ("greedy", "random"):
            act = _greedy_action(game.env) if game.mode == "greedy" else _random_action(game.env)
            st = game.do_step(act)
            socketio.emit("state", st)
        time.sleep(0.25)


if __name__ == "__main__":
    t = threading.Thread(target=bot_loop, daemon=True)
    t.start()
    print("PacWorld demo: http://<host-ip>:5001", flush=True)
    socketio.run(app, host="0.0.0.0", port=5001,
                 debug=False, use_reloader=False, allow_unsafe_werkzeug=True)
