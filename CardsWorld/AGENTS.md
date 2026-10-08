# iGeekCardsWorld

Playing cards game environment for iGeek (Blackjack, card combos, deck management).

## Kanban board

This project belongs to the **AStarship** organization; its Kanban board slug is `astarship` (see `~/AStarStarship/AGENTS.md`). Always target `astarship` explicitly (`--board astarship` / `board="astarship"`). Never rely on the current-board pointer. If a task's assigned board differs, report the mismatch before acting — do not silently switch.

## Unit Testing — Use the ASCIICrabs Seam Test API (NOT CTest/GoogleTest/Catch)

This project has **no CMake CTest, no GoogleTest, no Catch2, no pytest**. The unit-test
API is the **ASCIICrabs seam test harness** (`../ASCIICrabs/Test.h`, `Test.hpp`, `Test.hxx`).
When a task asks for "unit tests" or "tests", that means adding seam test units to this
world's `_Seams/`. **Do not add a CTest/`add_test`/`ctest` target** — the seam binary's
exit code is the CI signal (see below).

### Where tests live
Tests are **seam-gated `.hxx` units** in `_Seams/`, mimicking `ASCIICrabs/_Seams/`:
- `00.Core.hxx` — the default/first seam's unit tests (gated `#if SEAM >= CARDSWORLD_CORE`).
  This is where new unit tests go.
- `01.Release.hxx` — the release/demo seam (gated `#if SEAM >= CARDSWORLD_RELEASE`).
- `_Tests.hxx` — aggregates the units into `CardsWorldTests(args)` =
  `TTestTree<CWTest::Core, CWTest::Release>(args)`.
- `_Main.cpp` — `#if SEAM == SEAM_N` → run `Release(argss)`; `#else` →
  `TTestTree<CardsWorldTests>(arg_count, args)`. The default `SEAM`/`SEAM_N` is
  `CARDSWORLD_CORE`, so the normal build runs the unit tests.

### A test unit
A unit is an `inline const CHA* Name(const CHA* args)` in `namespace CWTest` that starts
with `A_TEST_BEGIN;`, runs the asserts, and ends with `return NILP;` on success.

### Assertions — and the exit-code trap (verified 2026-10-07)
The stock macros `A_ASSERT(cond)`, `A_AVOW(a, b)`, `A_ASSERT_PTR(p)` **print-and-continue**:
on failure they call `TestFail` (prints `FAILURE ... at line:NN`) but do **not** set a flag
the exit code reads. A unit that only uses those and then `return NILP;` **always exits 0
even when assertions fail.** This was confirmed empirically (injected a bad assert → exit 0).

To make a failure actually fail the build (exit 1, CI-visible), the unit must return a
non-nil `const CHA*`. `00.Core.hxx` does this with local fail-tracking macros:
- `CORE_CHECK(cond)` — like `A_ASSERT`, also bumps a fail counter.
- `CORE_EQ(a, b)` — like `A_AVOW` (both must be a type `TestEq` overloads: `ISC`, `CHA*`,
  etc. — **cast `enum class` values to `ISC`** first, e.g.
  `CORE_EQ(ISC(x), ISC(Enum::kVal))`).
- `CORE_PTR(p)` — nil-check (uses `IsError`).
At the end of `Core()`: `if (CWFails() != 0) return "cards_world_core_test_failure";`.
`SeamResult` maps that to `APP_EXIT_FAILURE`. **Use this pattern for new test seams** —
plain `A_ASSERT` alone is not CI-safe.

### Build & run
- Unit tests (default): `cmake --build build --clean-first && ./iGeekCardsWorld`
  → exit 0 = pass, exit 1 = an assertion failed (output shows which line).
- Release/demo seam: compile with `-DSEAM=CARDSWORLD_RELEASE -DSEAM_N=CARDSWORLD_RELEASE`
  (a separate `-DCMAKE_CXX_FLAGS` dir did not reliably pick up the seam macros; direct
  g++/`-D` does).
- `D_COUT` is a **no-op in Release seams** — use `StdOut() <<` for output you need there;
  in test seams `D_COUT` works.

### Pitfalls (verified 2026-10-07)
- `ISC` is in `namespace _`, **not** `_::CardsWorld` — write `ISC`, not `CardsWorld::ISC`.
- `Card` 5-arg ctor is `Card(pip, suit, face_value, point_value, SuitCulture)`;
  `PointValue()` returns the `point_value` you pass. For value tests pass
  `point_value = (pip >= 11) ? 10 : pip`, else every non-ace card is worth 0 and
  `HandValue` is wrong (the 2-arg `Card(pip, suit)` computes it for you).
- `TRandom` (SEAM >= CRABS_RANDOM) draws a **half-normal, not uniform** — `Random(0, i)`
  can return > `i`. Clamp the index in `Shuffle()`/`TakeRandomCard()` (upstream bug, not
  fixed in core).

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