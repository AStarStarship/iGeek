# iGeek

Data-driven RL AI gym for building, measuring, and learning agents in
massively parallel fuzzed environments using the Chinese Room.

iGeek is a no-stdlib C++23 headless engine: deterministic simulation worlds
(plants, grids, cards, mazes) plus a vectorized-batch RL core (Gym/Env/PPO/
Policy/Tensor) and a generic agent + metrics framework for studying how
internal agent state changes under learning.

## Repository map

- **Root** — the engine core: `Env`/`Gym`/`Multiverse` (environment
  contract), `Tensor` (row-major float kernel), `Policy` (trainable policy
  interface + transformer policy), `PPO` (clipped policy loss + GAE +
  value-clip + entropy over a vectorized Gym batch), `Agent`/`AgentState`
  (three generic agent tiers), `Metrics` (behavior metrics + linear probe),
  `School*` (course/degree bookkeeping).
- **`BikeWorld/`** — continuous planar bicycle plant, headless, 4-tire
  friction model, classical reference driver + Gym/PPO smoke.
- **`CarWorld/`** — continuous 2D car plant, four friction-limited tire
  contacts, route planning, classical driver evidence + Gym/PPO smoke.
- **`CardsWorld/`** — blackjack environment, the reference PPO training
  loop (linear policy learns hit/stand).
- **`PacWorld/`** — classic maze-chase world, the PPO learning probe
  (reward climbs from one TrainStep).
- **`TileWorld/`** — 2D tile ECS (headless spine: entities, components,
  tile map, occlusion/visibility, enemies).
- **`MazeWorld/`**, **`PolygonWorld/`**, **`TypingWorld/`**,
  **`PrisonerWorld/`**, **`SudokuWorld/`**, **`HBM/`**, **`Ulator/`**,
  **`WikiWorld/`** — additional worlds in various states of completion
  (see each world's README).
- **`Cookbook/`** — worked examples and getting-started notes.
- **`_Seams/`** — the seam (test/demo) build tree, selected by the `SEAM`
  macro.

The engine depends on two sibling repositories at the same level:

- **`../ASCIICrabs/`** — the Crabs core: no-stdlib ASCII data types,
  containers, I/O, the Chinese Room stack machine.
- **`../CrabsTK/`** — the Crabs toolkit: reusable modules (Code, Data,
  Image, Math, Pro, Touch, Who, ...).

## Building

Each world with a `CMakeLists.txt` builds standalone, e.g.:

```sh
cmake -S BikeWorld -B BikeWorld/build -DCMAKE_BUILD_TYPE=Release
cmake --build BikeWorld/build -j
ctest --test-dir BikeWorld/build --output-on-failure
```

Sanitizer build:

```sh
cmake -S BikeWorld -B BikeWorld/build/sanitized \
  -DCMAKE_BUILD_TYPE=Debug -DBIKEWORLD_SANITIZE=ON
cmake --build BikeWorld/build/sanitized -j
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir BikeWorld/build/sanitized \
  --output-on-failure
```

Requirements: GCC 13+ (C++23/`-std=c++2b`), CMake 3.10+. CarWorld/BikeWorld
optionally use MKL or OpenBLAS via `CrabsTK/Math`.

## License

Copyright [AStarship](https://astarship.net)™; most rights reserved,
third-party commercialization prohibited, mandatory improvement donations,
licensed under the AStartup Strong Source-available License that YOU MUST
CONSENT TO at <https://github.com/AStarStarship/iGeek>.

### Legal Agreement

This source form is source-available content, the Writings and Discoveries,
written by and containing intellectual property belonging to the IP Owner.
The Writings and Discoveries consist of the documents, files, source code,
technology design files, art, trademarks, and other content contained in
this file, folder, and the GitHub repository, the Repo, located at
<https://github.com/AStarStarship/iGeek>. The Writings and Discoveries are
published under the AStartup Strong Source-available License, the License,
which is a non-commercial open-source license and is for educational and
demonstration purposes only. To use the Writings and Discoveries for
commercial purposes, you must download and purchase a license for
commercial use of the Writings and Discoveries from
<https://astarship.net>, and you will be bound by the license agreed upon
before downloading the Writings and Discoveries. You may use, reproduce,
publicly display, and modify the Writings and Discoveries so long as you
submit and donate fixes and derived intellectual property, the Donated
Ideas, to the Repo as an Issue ticket to become part of the Writings and
Discoveries. You may not sell the Writings and Discoveries or otherwise
profit from derivative works created from the Writings and Discoveries,
referred to as Third-party commercialization, without the expressed written
permission of the copyright holder. Unless required by applicable law or
agreed to in writing, the Writings and Discoveries distributed under the
License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
OF ANY KIND, either express or implied.
