# logos-tictactoe-app

Multiplayer tic-tac-toe built as a single Logos `ui_qml` module with a process-isolated C++ backend. Game logic and UI live in the same module; `delivery_module` carries moves over the wire.

Built by following [Tutorial Part 3 (C++ UI Module, Process-Isolated)](../../logos-co/logos-tutorial/tutorial-cpp-ui-app.md), using `lgs basecamp` in place of raw `nix run` for install/launch where possible.

## Design (differs from the tutorial reference)

The tutorial's reference case (`calc_ui_cpp`) has two modules — a core `calc_module` with business logic and a `calc_ui_cpp` UI module that calls it. Here we collapse that into a single module:

- **tictactoe** (`ui_qml`) — QML view + C++ backend that owns the board state, turn management, win/draw detection, **and** (later) multiplayer wiring via `delivery_module`. No separate core module.

Rationale: the game rules are tiny (a 3×3 board and a winner check), not worth a process boundary on their own. The C++ backend is already process-isolated from the host app; that's enough.

## Status

See [slice plan](#slice-plan) for what's built and what's next.

## Slice plan

| # | Slice | State |
|---|-------|-------|
| 1 | Project skeleton (README, CLAUDE.md, scaffold, `lgs init`, `lgs basecamp setup`) | ✅ done |
| 2 | Configure module (`metadata.json`, `.rep`, interface header, CMakeLists) | in progress |
| 3 | C++ backend with local game logic | pending |
| 4 | QML view (3×3 board, status, new game) | pending |
| 5 | Local checkpoint: `lgs basecamp install` + `launch alice` | pending |
| 6 | Multiplayer via `delivery_module` | pending |
| 7 | Multiplayer checkpoint: `launch alice` + `launch bob` | pending |

## Deviations from the tutorial

Captured the moment they happen — not at the end.

| Slice | Tutorial says | We do | Why |
|-------|---------------|-------|-----|
| 1 | `calc_ui_cpp` with a separate `calc_module` dependency | Single `tictactoe` module containing both UI and game logic | Game rules are trivial; no second process boundary needed. See [Design](#design-differs-from-the-tutorial-reference). |
| 1 | Scaffolded `flake.nix` uses `logos-module-builder.url = "github:logos-co/logos-module-builder"` (no ref → default branch) | Pinned to `github:logos-co/logos-module-builder/tutorial-v1` | `lgs basecamp docs` warns that `main` is incompatible with basecamp v0.1.1's wire format. The reference tictactoe project pins the same tag. Required by the `lgs basecamp` choice, not by the tutorial. |

## `lgs` usage (replacing tutorial commands)

Where `lgs basecamp` supersedes a raw `nix` command from the tutorial.

| Slice | Tutorial command | `lgs` equivalent used | Notes |
|-------|------------------|-----------------------|-------|
| 1 | `nix flake init -t github:logos-co/logos-module-builder#ui-qml-backend` | (none — kept tutorial command) | `lgs` has no template-init subcommand; `lgs basecamp` is for dogfooding, not scaffolding. |
| 1 | (tutorial doesn't cover `scaffold.toml`) | `lgs init` | Required by `lgs basecamp *` commands. Creates `scaffold.toml` + `.scaffold/{state,logs}`, appends `.scaffold` to `.gitignore`. |
| 1 | (tutorial doesn't cover this) | `lgs basecamp setup` | One-time bootstrap: pins basecamp repo, builds `basecamp` + `lgpm` binaries, seeds `alice` / `bob` profile dirs under `.scaffold/basecamp/profiles/`. Prerequisite for `basecamp modules/install/launch`. |

_Rows for `lgs basecamp modules`, `lgs basecamp install`, `lgs basecamp launch alice|bob` will be added when those slices run (tutorial Step 8 equivalent)._

## Build + run (populated as slices complete)

_TBD — filled in as each slice lands._
