# [iGeek Cookbook](../)

## [Getting Started](./)

### Quickstart Guide

**1.** Clone the iGeek repo (it expects its two sibling repos at the same
level: `ASCIICrabs` and `CrabsTK`):

```Bash
git clone https://github.com/AStarStarship/iGeek.git
cd iGeek
# siblings, if you have not cloned them yet:
git clone https://github.com/AStarStarship/ASCIICrabs.git ../ASCIICrabs
git clone https://github.com/AStarStarship/CrabsTK.git ../CrabsTK
```

**2.** Requirements: GCC 13+ (C++23 / `-std=c++2b`), CMake 3.16+.

**3.** Build and run the core framework self-test (verifies the whole
RL/agent layer: tensor kernels, policy, PPO, agent tiers, linear probe):

```Bash
cmake -S . -B build_core -DCMAKE_BUILD_TYPE=Release
cmake --build build_core -j
./build_core/igeeK_core        # exit 0 = pass
```

**4.** Build a world. BikeWorld is the fastest end-to-end example:

```Bash
cmake -S BikeWorld -B BikeWorld/build -DCMAKE_BUILD_TYPE=Release
cmake --build BikeWorld/build -j
./BikeWorld/build/bikeworld    # classical driver, prints goals + timing
```

**5.** Watch it learn. CardsWorld trains a PPO agent on blackjack (hit/stand)
in a few seconds on CPU:

```Bash
cmake -S CardsWorld -B CardsWorld/build -DCMAKE_BUILD_TYPE=Release
cmake --build CardsWorld/build -j
./CardsWorld/build/iGeekCardsWorld        # exit 0 = tests + training pass
```

**[<< Previous Section: Getting Started](./) | [Next Section: Command Line Usage >>](./CommandLineUsage.md)**

## License

Copyright [AStarship™](https://astarship.net).
