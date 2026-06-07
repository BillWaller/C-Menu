# Copilot Instructions for C-Menu

## Build, test, and lint

- **Preferred full build:** `cd src && make`
- **Show active Makefile configuration:** `cd src && make config`
- **CMake/scripted build path:** `cd build && ./build.sh`, then `sudo ./install.sh`
- **Raw CMake configure step:** `cmake -S src -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_INSTALL_PREFIX="$HOME/menuapp"`
- **CMake caveat:** the current `src/CMakeLists.txt` does not link PAM for `rsh`, so `cmake --build build` currently fails at the `rsh` target. Use `cd src && make` for a complete build unless you are fixing the CMake build itself.
- **Single explicit test target:** `cd src && make -B test && ./test_strings`
- **Why `-B` matters:** the repo already contains `src/test/`, so plain `make test` can be treated as up to date without running the recipe.
- **Diagnostics:** `cd src && make valgrind` or `cd src && make helgrind`
- **Linting:** there is no repo-wide lint target; use `clang-tidy -p src src/menu.c --quiet` and swap in the source file you are changing. The repo config lives in `src/.clang-tidy`, and `src/compile_commands.json` is already checked in.

## High-level architecture

- `src/menu.c` is the shared entrypoint for `menu`, `form`, `pick`, `view`, and `ckeys`. The installed commands are mostly symlinks to the same executable, and runtime dispatch is based on `argv[0]`.
- `src/include/common.h` defines `Init`, the central runtime/config structure shared by Menu, Form, Pick, and View. `src/init.c` fills it from defaults, environment, the active `.minitrc`, and then CLI options.
- Runtime defaults assume a **menuapp tree** rooted at `CMENU_HOME` (default `~/menuapp`). `menuapp/` is therefore part of the product, not just sample data: `msrc/` holds menu/form descriptions, `help/` holds default help screens, `data/` stores sample/runtime data, and `scripts/`/`bin/` provide helper commands invoked from menus.
- The UI engines are split by component:
  - `src/menu_engine.c` + `src/parse_menu_desc.c` for hierarchical menu parsing and dispatch
  - `src/form_engine.c` for form parsing, field navigation, calculation/query/post-processing
  - `src/pick_engine.c` for list selection from files, stdin, or provider commands
  - `src/view_engine.c` for the read-only viewer
- `pick` and `form` are designed as pipeline components. `pick` can fork a provider command and read its stdout as the selectable dataset; `form` can load data, process it, write it back out, and hand results to a receiver command.
- `view` is performance-oriented and maps its input with `mmap(2)` instead of buffering line-by-line. Its navigation logic depends on that model and maintains its own line table for reverse/forward movement.
- `libcm` (`dwin.c`, `exec.c`, `futil.c`, `scriou.c`, `sig.c`) is the shared utility layer used by the ncurses-facing programs and standalone helpers such as `lf`, `enterchr`, `enterstr`, `stripansi`, and `whence`.

## Key conventions

- **Menu description DSL:** in `.m` files, the first `:` line becomes the title, later `:` lines become visible menu text, and each actionable entry must be paired with a following `!` command line. `#` starts a comment.
- **Menu hotkeys:** prefix a choice label with `-X` or `_X` to reserve `X` as the hotkey letter; otherwise `parse_menu_desc.c` auto-picks the first unused character from the rendered label.
- **Form description DSL:** `.f` files use single-letter directives such as `H:` (header/title), `T:` (static text), `F:` (field), `?` (help file), plus processing directives like `G`, `Q`, `C`, and command directives. Use the existing forms under `menuapp/msrc/` as the source of truth for the format.
- **Config/CLI mirroring is intentional:** `.minitrc` keys, `Init` fields, and most long option names are meant to line up. When adding or changing an option, update the struct definition, argp option parsing in `src/init.c`, and config-file handling together.
- **Precedence matters:** `provider_cmd` takes precedence over `in_spec` in `pick`, and `form` uses `out_spec` plus `receiver_cmd` as explicit post-processing hooks. Preserve that composition model instead of hardcoding one-off flows into the engines.
- **Default help lookup is runtime-data driven:** if a component does not receive `help_spec`, it falls back to files under `menuapp/help/` such as `menu.help`, `form.help`, `pick.help`, and `view.help`.
- **Wide-character support is not optional:** the core headers enable `ncursesw`, and the engines use wide-char APIs and UTF-8-aware logic for rendering, hotkey highlighting, and width calculations. Keep that intact when changing screen/input code.
- **GNU/C23 features are used deliberately:** the codebase relies on C23 plus GNU extensions such as statement-expression macros and `typeof`, so match the existing style instead of trying to backport changes to strict ISO C89/C99 patterns.
