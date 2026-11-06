# iGeek

AI sandbox and game engine for building, measuring, and learning AI agents in different environments running on parallel systems. Portable robotics simulator also usable for games and scientific computing.

## Kanban board

This project belongs to the **AStarship** organization; its Kanban board slug is `astarship` (see `~/AStarStarship/AGENTS.md`). Always target `astarship` explicitly (`--board astarship` / `board="astarship"`). Never rely on the current-board pointer. If a task's assigned board differs, report the mismatch before acting — do not silently switch.

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
| Namespaces | `namespace iGeek { ... }` | single shared underscore namespace |
| DB names / tables / columns | lower_snake_case (mutable) | `user_sessions`, `order_items` |
| Client-facing API / headers | CamelCase (immutable contract) | `OrderService`, `PaymentGateway` |
| Filenames (client contract) | CamelCase | `OrderService.h` |
| Filenames (host/storage) | lower_snake_case | `user_sessions.py` |

### Header Boilerplate (C/C++)
```
// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeek<NAME><H|HPP>
#define _iGeek<NAME><H|HPP>
#include "..."
#if SEAM >= _iGeek<NAME>
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
starts with AStarship copyright, `#pragma once`, a `_iGeek*` guard, and `#if SEAM >= _iGeek*`
gating.** The name tells you the kind, width, access level, and mutability — before you
read the body.

## Git Protocol

- **Do NOT undelete files from git.** A deleted or renamed file was done so on
  purpose — it wasn't supposed to be there, it got merged into another file, or
  it wasn't named correctly. Fix the current files in place; do not try to
  rescue or restore stale files from git history (no `git checkout -- <path>`,
  `git restore <path>`, `git show <ref>:<path>` written back to disk, or
  `git revert` to bring a deleted path back).
- **Only `master-agent` and the Captain may modify git.** No other agent or
  profile may run git write operations on a tracked repo — no `git commit`,
  `git add`, `git push`, `git reset`, `git rebase`, `git merge`, `git rm`,
  branch creation, force-push, or PR changes. Read-only inspection
  (`git status`, `git log`, `git diff`, `git show`, `git ls-files`) is allowed
  for anyone.
