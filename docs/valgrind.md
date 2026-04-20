# Running Valgrind Against C-Menu

This document explains how to run [Valgrind](https://valgrind.org/) against
C-Menu and what to expect in its output.

---

## Background

Valgrind's `--show-leak-kinds=all` mode reports four categories of memory at
program exit:

| Category            | Meaning                                                                                                         |
| ------------------- | --------------------------------------------------------------------------------------------------------------- |
| **definitely lost** | Memory that can no longer be reached — a true leak.                                                             |
| **indirectly lost** | Memory reachable only through a definitely-lost block.                                                          |
| **possibly lost**   | Heuristic: may or may not be reachable.                                                                         |
| **still reachable** | Memory still pointed to by a live pointer at exit. Often intentional (library caches, global singletons, etc.). |

C-Menu aims for **zero** `definitely lost` / `indirectly lost` /
`possibly lost` bytes, and suppresses the known ncurses `still reachable`
allocations that the ncurses library intentionally retains as global caches.

---

## Suppression File

`src/valgrind.supp` suppresses the following known-harmless ncurses
`still reachable` records (all rooted in frames such as `_nc_first_db`,
`_nc_setupterm`, `_nc_acs_map`, and `update_getenv` inside `libncursesw` /
`libtinfo`). These allocations are reachable through ncurses global state and
are freed by the OS on process exit; suppressing them keeps Valgrind output
actionable.

---

## How to Run

### Using the Makefile target (recommended)

```bash
cd C-Menu/src
make valgrind
```

This builds `menu` (if needed) and then runs:

```bash
valgrind --leak-check=full --show-leak-kinds=all \
         --leak-resolution=high --track-origins=yes \
         --suppressions=valgrind.supp \
         ./menu
```

### Running manually

```bash
cd C-Menu/src
valgrind --leak-check=full --show-leak-kinds=all \
         --leak-resolution=high --track-origins=yes \
         --suppressions=valgrind.supp \
         ./menu [your-menu-args]
```

You can substitute `form`, `pick`, `view`, or `ckeys` in place of `menu`.

---

## Expected Output

With the suppression file applied, Valgrind should report:

```
LEAK SUMMARY:
   definitely lost: 0 bytes in 0 blocks
   indirectly lost: 0 bytes in 0 blocks
     possibly lost: 0 bytes in 0 blocks
   still reachable: 0 bytes in 0 blocks   (suppressed)
        suppressed: N bytes in N blocks
ERROR SUMMARY: 0 errors from 0 contexts (suppressed: N from N)
```

Any non-zero `definitely lost`, `indirectly lost`, or `possibly lost` values
indicate a real leak that should be investigated.

---

## Reference

- Suppression file: [`src/valgrind.supp`](../src/valgrind.supp)
- Sample Valgrind output (before fixes): [`src/valgrind.out`](../src/valgrind.out)
