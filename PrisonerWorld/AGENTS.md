# PrisonerWorld_

## Kanban board

This project belongs to the **AStarship** organization; its Kanban board slug is `astarship` (see `~/AStarStarship/AGENTS.md`). Always target `astarship` explicitly (`--board astarship` / `board="astarship"`). Never rely on the current-board pointer. If a task's assigned board differs, report the mismatch before acting — do not silently switch.

Greenfield [iGeek](https://github.com/AStarStarship/iGeek) world for evaluating
**agentic game theory** (prisoner's-dilemma-style repeated games between AI
agents) on the [Crabs](https://github.com/AStarStarship/ASCIICrabs) ASCII
Data Specification. No build system, code, or tests exist yet — only this file
and an empty `_Seams/`.

## Sibling references (the real sources of truth)

- `../iGeek/` — the iGeek framework itself: `Env` (a `Room` with pure-virtual
  `ComputeReward(Crabs*, EnvGoal, EnvGoal, STA*)`), `Agent`, `Gym`,
  `Multiverse`, reward/goal types. Read these before designing the world.
- `../iGeekCarWorld/`, `../iGeekPacWorld/` — closest working world examples:
  `_Main.cpp` entrypoint, `_Seams/` tree, `.h`/`.inl` split, `_Config.h` /
  `_Debug.inl` / `_Release.inl` / `_Undef.inl` build-mode pattern.
- `../ASCIICrabs/` — upstream core. Treat as **read-only**; never edit in place.

## Conventions (follow the root `AGENTS.md` + siblings exactly)

- Files: `Thing.h` decl + `Thing.inl` implementation (siblings use `.inl`,
  not the `.hxx` the root file mentions — match siblings).
- Entrypoint: `_Main.cpp` (or `Main.cpp`) with a `namespace _` and
  `ISC main(ISC argc_count, CHA** args)`.
- Tests: seam tree under `_Seams/` (`00.Core.inl`, `_Debug.inl`,
  `_Release.inl`, `_Undef.inl`); selected via the `SEAM` macro, run by
  executing the built binary. No test framework.
- **No C++ std library.** Do not include `<string>`, `<vector>`, `<iostream>`,
  `<map>`, `<set>`, `<algorithm>`, or any other C++ standard library header.
  Use ASCII Crabs data types (`ISN`, `ISC`, `CHA`, `CHR`, `IUW`, `STA`, `IUD`,
  `BOL`, `A8`, `B16`, `C32`, `D64`, `E128`, `CHA*`, `CHA**`, etc.) instead of
  `int`, `char`, `std::string`, and all other std types. ASCII Crabs provide the
  equivalent containers, strings, and I/O.
- New C++ files start with `// Copyright AStarship <https://astarship.net>.`

## Gotchas

- `README.md` exists but is empty — fill it when the world has content.
- Directory name ends with `_`; the `_` suffix here means a new/working
  project, not "non-code content" (unlike `Campaigns_` etc.).
- iGeek `Env` inherits `Crabs::Room`, so the world's state must be
  representable in ASCII Crabs data; rewards come from
  `ComputeReward` comparing achieved vs desired `EnvGoal`.

## Code Style — Chimera+ (AStarship Standard)

All **code and filenames** in the AStarship ecosystem follow the **Chimera+** style guide
(`~/AStarStarship/ASCIICrabs/__ChimeraPlus.md`). The naming philosophy applies to code
identifiers and filenames fleet-wide.

### Core Rule: immutable vs mutable
- **Immutable** (can't change after init) → **CamelCase**
- **Mutable** (changes at runtime) → **lower_snake_case**

### Naming Summary
| Element | Convention | Example |
|---|---|---|
| Type aliases (fixed-width) | 3-letter CAPS codes | `CHA` char8, `ISC` int32, `IUD` uint64, `BOL` bool, `FPC` float32, `FPD` double64 |
| Structs / classes | CamelCase + T(POD)/A(utoject) prefix | `TArray`, `AMap`, `Crabs` |
| Struct members (mutable) | lower_snake_case | `socket_bytes`, `header_bytes` |
| Struct members (immutable) | CamelCase | `StackTotalMin`, `ColumnWidth` |
| Free functions / methods | CamelCase, type-prefixed | `CrabsInit`, `TArrayBytes`, `TMapFind` |
| Function suffixes | `_NC` (no-check), `C` (const/count), `T` (POD), `A` (autoject) | `TArrayInsert_NC`, `CSizeMin` |
| Local variables | lower_snake_case (always) | `bytes_data`, `read_cursor` |
| Macros / #define | UPPER_SNAKE_CASE | `CRABS_RUN_TESTS`, `CPU_X64` |
| Private/protected members | trailing underscore | `aobj_`, `array_` |
| Namespaces | `namespace _ { ... }` | single shared underscore namespace |
| DB names / tables / columns | lower_snake_case (mutable) | `user_sessions`, `order_items` |
| Client-facing API / headers | CamelCase (immutable contract) | `OrderService`, `PaymentGateway` |
| Filenames (client contract) | CamelCase | `OrderService.h` |
| Filenames (host/storage) | lower_snake_case | `user_sessions.py` |

### Header Boilerplate (C/C++)
```
// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_<NAME>_<H|HPP>
#define CRABS_<NAME>_<H|HPP>
#include "..."
#if SEAM >= CRABS_<NAME>
... body ...
#endif
#endif
```

### Formatting
- 2-space indent
- Opening brace same line (functions/control), next line (structs/classes)
- `D_ASSERT()`, `D_COUT()`, `D_RETURNT(type, val)` for debug/assert/return
- `NILP` = nullptr
- `alignas(ACPUCacheLineSize)` for hot structs
- Doxygen `/** @param @return @pre @link @see @code */` comments

### The One-Sentence Version
**Immutable → CamelCase, mutable → lower_snake_case; macros UPPER_SNAKE; private/protected
get a trailing `_`; types are short CAPS width-codes (CHA/ISC/IUD/BOL); structs are
CamelCase with T(POD)/A(utoject) prefix; functions are CamelCase with T/A kind prefixes
and _NC/C markers; locals are snake_case; everything lives in `namespace _`; every file
starts with AStarship copyright, `#pragma once`, a `CRABS_*` guard, and `#if SEAM >= CRABS_*`
gating.** The name tells you the kind, width, access level, and mutability — before you
read the body.