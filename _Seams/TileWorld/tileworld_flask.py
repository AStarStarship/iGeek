#!/usr/bin/env python3
"""TileWorld — tile-grid env + Flask/WebSocket browser demo.

Faithful Python port of `TileEnv` (Worlds/Tile/TileEnv.hxx) and the
`OcclusionMap` DDA raycast (ASCIICrabs/Occlusion.h) + `TileMap` cell
semantics (Worlds/Tile/TileMap.hxx). One change vs the bare C++ env: the
demo loads a seeded wall level via the map's own AddTile equivalent, so the
"keep enemies in sight" reward has actual occlusion to work with. Rules
(step cost, block penalty, seen bonuses, DDA visibility) are unmodified.

Rules (TileEnv::Step): 4-way move (up/down/left/right, no stay).
reward = -0.01 per step; -0.05 when the move is blocked; +0.5 the first time
each enemy enters the player's DDA field of view; +1.0 and done when all 3
enemies have been seen; MaxSteps=512 terminates.
"""

import os
import threading

from flask import Flask, send_from_directory, jsonify
from flask_socketio import SocketIO, emit

# --- OcclusionMap port (ASCIICrabs/Occlusion.h, line-by-line) -------------
ABS64 = (1 << 64) - 1
_LCG_A = 6364136223846793005
_LCG_C = 1442695040888963407

GRID_WIDTH = 24
GRID_HEIGHT = 18
ENEMY_COUNT = 3
ACTION_COUNT = 4   # up, down, left, right (no stay, per TileEnv)
MAX_STEPS = 512
CELL_SIZE = 16.0
# Action dirs (TileEnv::Step): 0=up(-y) 1=down(+y) 2=left(-x) 3=right(+x)
_DIRS = {0: (0, -1), 1: (0, 1), 2: (-1, 0), 3: (1, 0)}


def _trunc(v):
    """C-style (ISC) cast: truncate toward zero."""
    return int(v)


class OcclusionMap:
    """Port of _::OcclusionMap: occluder grid + per-observer DDA visibility."""

    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.occluded = [0] * (width * height)
        self.obs_pos = []
        self.vis = []

    def in_bounds(self, x, y):
        return 0 <= x < self.width and 0 <= y < self.height

    def set_occluded(self, x, y, on):
        if self.in_bounds(x, y):
            self.occluded[y * self.width + x] = 1 if on else 0

    def occluded_at(self, x, y):
        if not self.in_bounds(x, y):
            return True  # out of bounds = solid (can't see)
        return self.occluded[y * self.width + x] != 0

    def add_observer(self, px, py):
        self.obs_pos.append((px, py))
        return len(self.obs_pos) - 1

    @staticmethod
    def _start_cell(v):
        # C: (v >= 0) ? (ISC)v : ((ISC)v - 1)
        return _trunc(v) if v >= 0 else _trunc(v) - 1

    def line_of_sight(self, x0, y0, x1, y1):
        # DDA (Amanatides & Woo grid traversal), ported verbatim from
        # Occlusion.h LineOfSight.
        x = self._start_cell(x0)
        y = self._start_cell(y0)
        x_end = self._start_cell(x1)
        y_end = self._start_cell(y1)
        dx = x1 - x0
        dy = y1 - y0
        step_x = 1 if dx > 0 else (-1 if dx < 0 else 0)
        step_y = 1 if dy > 0 else (-1 if dy < 0 else 0)
        adx = -dx if dx < 0 else dx
        ady = -dy if dy < 0 else dy
        tdelta_x = 1.0 / adx if adx > 0 else 1e30
        tdelta_y = 1.0 / ady if ady > 0 else 1e30
        if step_x > 0:
            tmax_x = ((x + 1) - x0) * tdelta_x
        elif step_x < 0:
            tmax_x = (x0 - x) * tdelta_x
        else:
            tmax_x = 1e30
        if step_y > 0:
            tmax_y = ((y + 1) - y0) * tdelta_y
        elif step_y < 0:
            tmax_y = (y0 - y) * tdelta_y
        else:
            tmax_y = 1e30
        guard = 0
        max_steps = self.width + self.height + 2
        sx0, sy0 = self._start_cell(x0), self._start_cell(y0)
        while guard < max_steps:
            guard += 1
            if x == x_end and y == y_end:
                return True  # reached dest, unblocked
            # The observer's own starting cell never blocks sight out of itself.
            if not (x == sx0 and y == sy0):
                if self.occluded_at(x, y):
                    return False
            if tmax_x < tmax_y:
                x += step_x
                tmax_x += tdelta_x
            else:
                y += step_y
                tmax_y += tdelta_y
            if not self.in_bounds(x, y):
                return False  # ray left the map
        return False

    def recompute(self):
        self.vis = []
        for (ox, oy) in self.obs_pos:
            row = [0] * (self.width * self.height)
            for cy in range(self.height):
                for cx in range(self.width):
                    row[cy * self.width + cx] = 1 if self.line_of_sight(
                        ox, oy, cx + 0.5, cy + 0.5) else 0
            self.vis.append(row)

    def observer_sees(self, o, cx, cy):
        if o < 0 or o >= len(self.obs_pos):
            return False
        if not self.in_bounds(cx, cy):
            return False
        return self.vis[o][cy * self.width + cx] != 0


class TileMap:
    """Port of TileMap's env-relevant surface: collision cells + occlusion.
    The C++ Update() also runs per-face tile collision resolution for
    continuous motion; TileEnv actors are point-sized and move cell-by-cell
    (Step() itself does the block check), so only bounds clamp + occlusion
    rebuild are exercised here."""

    def __init__(self):
        self.occlusion = OcclusionMap(GRID_WIDTH, GRID_HEIGHT)
        self.cells = {}  # (x, y) -> collision flag

    def add_tile(self, x, y, collision):
        if 0 <= x < GRID_WIDTH and 0 <= y < GRID_HEIGHT:
            self.cells[(x, y)] = collision

    def cell_blocked(self, x, y):
        # Out of bounds is solid (TileMap::CellBlocked).
        if x < 0 or x >= GRID_WIDTH or y < 0 or y >= GRID_HEIGHT:
            return True
        return self.cells.get((x, y), False)

    def update(self, player_xy, enemy_xys):
        # Rebuild occluder grid (layer A = collision tiles; layer B =
        # occluding entities — TileActor has occlude_=false, so none).
        for y in range(GRID_HEIGHT):
            for x in range(GRID_WIDTH):
                self.occlusion.set_occluded(x, y, (x, y) in self.cells and
                                            self.cells[(x, y)])
        # Observers at cell-corner positions (Center()/grid == pos/cell since
        # TileActor size is 0), player first, then enemies (C++ caps at 7).
        self.occlusion.obs_pos = [player_xy] + list(enemy_xys[:7])
        self.occlusion.recompute()

    def player_sees_enemy(self, ex, ey):
        return self.occlusion.observer_sees(0, ex, ey)


class TileEnv:
    """Port of iGeek TileEnv (Worlds/Tile/TileEnv.hxx)."""

    def __init__(self, seed=7):
        self.seed = seed if seed else 7
        self.map_ = TileMap()
        self.player_x, self.player_y = 1, 1
        self.enemy_x, self.enemy_y = [], []
        self.steps = 0
        self.done = False
        self.seen_mask = 0
        self.reward_total = 0.0
        self.reset()

    def place_enemies(self):
        # Deterministic placement (TileEnv::Reset formula).
        self.enemy_x, self.enemy_y = [], []
        for i in range(ENEMY_COUNT):
            ex = 3 + (self.seed + i * 7) % (GRID_WIDTH - 6)
            ey = 3 + (self.seed + i * 13) % (GRID_HEIGHT - 6)
            self.enemy_x.append(ex)
            self.enemy_y.append(ey)

    def reset(self):
        self.player_x, self.player_y = 1, 1
        self.steps = 0
        self.done = False
        self.seen_mask = 0
        self.reward_total = 0.0
        self.place_enemies()
        self.map_.update((float(self.player_x), float(self.player_y)),
                         [(float(x), float(y))
                          for x, y in zip(self.enemy_x, self.enemy_y)])

    def step(self, action):
        d = min(max(action, 0), ACTION_COUNT - 1)
        dx, dy = _DIRS[d]
        nx, ny = self.player_x + dx, self.player_y + dy
        self.steps += 1
        reward = -0.01
        if self.map_.cell_blocked(nx, ny):
            reward = -0.05  # blocked: penalty, stay put
        else:
            self.player_x, self.player_y = nx, ny
        self.map_.update((float(self.player_x), float(self.player_y)),
                         [(float(x), float(y))
                          for x, y in zip(self.enemy_x, self.enemy_y)])
        # Reward the first time each enemy comes into view.
        newly_seen = 0
        for i in range(ENEMY_COUNT):
            if (self.seen_mask & (1 << i)) == 0 and \
                    self.map_.player_sees_enemy(self.enemy_x[i], self.enemy_y[i]):
                self.seen_mask |= 1 << i
                newly_seen += 1
        if newly_seen > 0:
            reward += 0.5 * newly_seen
        if self.seen_mask == ((1 << ENEMY_COUNT) - 1):
            self.done = True
            reward += 1.0
        if self.steps >= MAX_STEPS:
            self.done = True
        self.reward_total += reward
        return reward

    @property
    def done_state(self):
        return self.done or self.steps >= MAX_STEPS


def make_demo_level(seed=7):
    """Seeded wall layout via the map's add_tile API (the C++ env ships an
    empty map; this is a level, not a rule change). Guarantees an open start
    and reachable enemy cells (carving corridors when needed)."""
    rng = seed if seed else 1

    def rnd():
        nonlocal rng
        rng = (rng * _LCG_A + _LCG_C) & ABS64
        return (rng >> 11) & 0xFFFFFFFF

    walls = set()
    for y in range(GRID_HEIGHT):
        for x in range(GRID_WIDTH):
            border = x == 0 or y == 0 or x == GRID_WIDTH - 1 or y == GRID_HEIGHT - 1
            if border:
                walls.add((x, y))
                continue
            if x == 1 and y == 1:
                continue
            if rnd() < 0x3A000000:  # ~23% interior walls
                walls.add((x, y))
    # Keep enemy spawn cells open (C++ placement formula).
    enemy_cells = []
    for i in range(ENEMY_COUNT):
        ex = 3 + (seed + i * 7) % (GRID_WIDTH - 6)
        ey = 3 + (seed + i * 13) % (GRID_HEIGHT - 6)
        enemy_cells.append((ex, ey))
        walls.discard((ex, ey))

    def reachable():
        seen = {(1, 1)}
        stack = [(1, 1)]
        while stack:
            x, y = stack.pop()
            for dx, dy in ((0, -1), (0, 1), (-1, 0), (1, 0)):
                n = (x + dx, y + dy)
                if 0 <= n[0] < GRID_WIDTH and 0 <= n[1] < GRID_HEIGHT \
                        and n not in seen and n not in walls:
                    seen.add(n)
                    stack.append(n)
        return seen

    # Carve a corridor (straight L-path) from the start to any unreachable
    # enemy so all three are always solvable.
    for (ex, ey) in enemy_cells:
        if (ex, ey) not in reachable():
            x, y = 1, 1
            while x != ex:
                walls.discard((x, y))
                x += 1 if ex > x else -1
            while y != ey:
                walls.discard((x, y))
                y += 1 if ey > y else -1
    return walls, enemy_cells


def _seek_action(env):
    """Bot: BFS to the nearest unseen enemy; wander once all are seen."""
    import collections
    if env.seen_mask == ((1 << ENEMY_COUNT) - 1):
        import random
        return random.randint(0, 3)
    targets = [(env.enemy_x[i], env.enemy_y[i])
               for i in range(ENEMY_COUNT) if not (env.seen_mask & (1 << i))]
    if not targets:
        import random
        return random.randint(0, 3)
    sx, sy = env.player_x, env.player_y
    prev = {(sx, sy): None}
    q = collections.deque([(sx, sy)])
    while q:
        x, y = q.popleft()
        for d in range(4):
            nx, ny = x + _DIRS[d][0], y + _DIRS[d][1]
            if 0 <= nx < GRID_WIDTH and 0 <= ny < GRID_HEIGHT \
                    and (nx, ny) not in prev and not env.map_.cell_blocked(nx, ny):
                prev[(nx, ny)] = (x, y)
                q.append((nx, ny))
    best = None
    for (tx, ty) in targets:
        if (tx, ty) in prev and (tx, ty) != (sx, sy):
            node = (tx, ty)
            while prev[node] != (sx, sy):
                node = prev[node]
            key = abs(tx - sx) + abs(ty - sy)
            if best is None or key < best[0]:
                best = (key, node)
    if best is None:
        import random
        return random.randint(0, 3)
    tx, ty = best[1]
    for d in range(4):
        if (env.player_x + _DIRS[d][0], env.player_y + _DIRS[d][1]) == (tx, ty):
            return d
    import random
    return random.randint(0, 3)


# --- Flask app -------------------------------------------------------------
app = Flask(__name__)
app.config["SECRET_KEY"] = "tileworld-secret-key"
socketio = SocketIO(app, cors_allowed_origins="*", async_mode="threading")


class Game:
    def __init__(self, seed=7):
        self.seed = seed
        self.env = TileEnv(seed)
        walls, _ = make_demo_level(seed)
        for (x, y) in walls:
            self.env.map_.add_tile(x, y, True)
        self.env.reset()  # re-runs occlusion with walls present
        self.mode = "human"  # human | seek | random
        self.lock = threading.Lock()

    def state(self):
        e = self.env
        occ = e.map_.occlusion
        vis = occ.vis[0] if occ.vis else [0] * (GRID_WIDTH * GRID_HEIGHT)
        return {
            "width": GRID_WIDTH, "height": GRID_HEIGHT,
            "walls": [[1 if e.map_.cell_blocked(x, y) else 0
                       for x in range(GRID_WIDTH)] for y in range(GRID_HEIGHT)],
            "visible": [1 if vis[y * GRID_WIDTH + x] else 0
                        for y in range(GRID_HEIGHT) for x in range(GRID_WIDTH)],
            "player": [e.player_x, e.player_y],
            "enemies": [[e.enemy_x[i], e.enemy_y[i],
                         1 if (e.seen_mask & (1 << i)) else 0,
                         1 if e.map_.player_sees_enemy(e.enemy_x[i], e.enemy_y[i]) else 0]
                        for i in range(ENEMY_COUNT)],
            "seen": e.seen_mask, "steps": e.steps,
            "done": e.done, "reward": round(e.reward_total, 4),
            "mode": self.mode,
        }

    def do_step(self, action):
        with self.lock:
            if self.env.done:
                self.env.reset()
            self.env.step(action)
            return self.state()


game = Game(seed=7)
_stop = threading.Event()


@app.route("/")
def index():
    return send_from_directory(os.path.dirname(os.path.abspath(__file__)), "tileworld.html")


@app.route("/api/state")
def api_state():
    return jsonify(game.state())


@app.route("/move/<d>")
def move(d):
    try:
        a = int(d)
    except ValueError:
        return jsonify({"error": "bad dir"}), 400
    st = game.do_step(a)
    socketio.emit("state", st)
    return jsonify({"status": "ok", "dir": a})


@app.route("/setmode/<mode>")
def setmode(mode):
    if mode not in ("human", "seek", "random"):
        return jsonify({"error": "unknown mode"}), 400
    with game.lock:
        game.mode = mode
    return jsonify({"status": "ok", "mode": mode})


@app.route("/reset")
def reset():
    with game.lock:
        game.env.reset()
    st = game.state()
    socketio.emit("state", st)
    return jsonify({"status": "ok"})


@socketio.on("connect")
def on_connect():
    emit("state", game.state())


@socketio.on("move")
def on_move(data):
    try:
        a = int(data.get("dir", 0)) % 4
    except (TypeError, ValueError):
        a = 0
    st = game.do_step(a)
    emit("state", st)


def bot_loop():
    import time, random
    while not _stop.is_set():
        if game.mode in ("seek", "random"):
            a = _seek_action(game.env) if game.mode == "seek" else random.randint(0, 3)
            st = game.do_step(a)
            socketio.emit("state", st)
        time.sleep(0.3)


if __name__ == "__main__":
    t = threading.Thread(target=bot_loop, daemon=True)
    t.start()
    print("TileWorld demo: http://<host-ip>:5002", flush=True)
    socketio.run(app, host="0.0.0.0", port=5002,
                 debug=False, use_reloader=False, allow_unsafe_werkzeug=True)
