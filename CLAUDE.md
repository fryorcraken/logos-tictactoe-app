# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

A single Logos C++ UI module (`type: "ui"`, `main: "tictactoe_plugin"`) named `tictactoe`. `createWidget(LogosAPI*)` returns a `QQuickWidget` loading `src/qml/Main.qml` with a `TictactoeBackend` QObject set as the QML `backend` context property. The backend holds game state (3×3 board, turn, outcome) and exposes `Q_INVOKABLE newGame()`, `play(r, c)` plus `Q_PROPERTY board/status/outcome/currentPlayer`. QML calls backend methods directly — no `logos.module`, no `logos.watch`, no `.rep`, no Qt Remote Objects. Multiplayer (slice 6) will add `delivery_module` as a flake input + metadata dep and wire outbound moves through `LogosAPI*`.

This repo follows **the `tutorial-v1` branch** of `logos-co/logos-tutorial`. Tutorial Part 3 on that branch has two sub-options; we use **Option A** (QML loaded via `QQuickWidget` inside a C++ Qt widget plugin).

- Tutorial (authoritative): `~/src/logos-co/logos-tutorial/tutorial-cpp-ui-app.md` (checked out on the `tutorial-v1` branch — verify with `git -C ~/src/logos-co/logos-tutorial status`).
- Online: https://github.com/logos-co/logos-tutorial/blob/tutorial-v1/tutorial-cpp-ui-app.md
- Tutorial's working example to consult when ambiguous: `logos-co/logos-tutorial`'s `logos-calc-ui-cpp/` directory on `tutorial-v1`.

Do **not** read the `master`-branch tutorial for this project — it describes a different process-isolated ui_qml+backend pattern that doesn't exist at `tutorial-v1` and will mislead you. (Earlier slices wasted cycles on this.)

## Working rules (non-negotiable, from the user's instructions)

1. **Follow the tutorial** (the `tutorial-v1` branch). Do not improvise around steps. Use `agent-skills` as the process layer.
2. **Prefer `lgs basecamp` over raw `nix` commands** whenever the tutorial's action has an `lgs` equivalent. `lgs basecamp {setup,modules,install,launch,reset,build-portable,doctor,docs}` replaces the tutorial's `nix run`/install flow. Initial scaffolding (`nix flake init -t ...`) has **no** `lgs` equivalent — keep it.
3. **STOP on error before deviating.** If a tutorial command fails, do not try a different command or "fix" it without confirming with the user. Report the exact failure and wait.
4. **Capture every deviation in `README.md` immediately** — in the `Deviations` table. Not at the end of coding. Same for every `lgs` command used in place of a tutorial command — record it in the `lgs usage` table as it happens.
5. **Delivery module reference is hands-off until the multiplayer slice. This is absolute.** `~/src/fryorcraken/logos-module-tictactoe` is a separate reference project. You may consult it **only** for `delivery_module` usage patterns, and **only** when slice 6 (multiplayer) is the active slice. That means:
   - Do **not** read its README, flake.nix, CMakeLists, source, QML, metadata.json, scaffold.toml, or any other file outside of slice 6.
   - Do **not** cite it, mirror it, mimic it, or "cross-check against it" for API design, naming, layout, game logic, UI, build config, or anything other than delivery-module wiring.
   - Do **not** use phrases like "the reference project does X" as justification for any design decision outside slice 6. Justify from the tutorial and your own reasoning only.
   - Naming the project "tictactoe" is coincidence; that similarity is not a license to look at the reference's tictactoe. This project's design intentionally differs (single module vs multi-module).
   - If you catch yourself reaching for the reference outside slice 6, STOP, tell the user you almost broke the rule, and re-derive from the tutorial.

## Slice-by-slice build flow

Work in thin vertical slices (per `agent-skills:incremental-implementation`). The slice plan lives in `README.md`. Mark each slice's state there as it advances. Do not jump ahead.

Roughly:

1. Skeleton — README, CLAUDE.md, `nix flake init`, `lgs init`, `lgs basecamp setup`.
2. Configure — `metadata.json` (`type: "ui"`), `interfaces/IComponent.h`, `CMakeLists.txt`.
3. Backend — `src/tictactoe_backend.{h,cpp}` holds game state + `Q_PROPERTY`/`Q_INVOKABLE` surface.
4. Plugin shell + QML — `src/tictactoe_plugin.{h,cpp}` (QQuickWidget loader), `src/qml/Main.qml` (direct `backend.*` calls).
5. Local checkpoint — `lgs basecamp install` + `launch alice`, single-player verified. ✅
6. Multiplayer — `delivery_module` input + `metadata.json.dependencies` + backend wiring (uses `LogosAPI*`).
7. Multiplayer checkpoint — `launch alice` + `launch bob`.

## Key command mapping

| Purpose | Tutorial command | What we use |
|---------|------------------|-------------|
| Scaffold | `nix flake init -t github:logos-co/logos-module-builder/tutorial-v1#ui-module` | same (no `lgs` alternative) |
| Enable `lgs basecamp` workflow | — | `lgs init` (once, after scaffolding) |
| One-time basecamp bootstrap | — | `lgs basecamp setup` |
| Capture modules into `scaffold.toml` | — | `lgs basecamp modules` |
| Build + install `.lgx` into profiles | `nix run` (tutorial's standalone-app runner) | `lgs basecamp install` |
| Launch the host app | `nix run` | `lgs basecamp launch alice` (or `bob`) |
| Wipe + re-seed | (manual `rm -rf`) | `lgs basecamp reset` |
| Health check | — | `lgs basecamp doctor` |
| Print the requirements doc | — | `lgs basecamp docs` |

## Platform assumptions

- Linux (`.so`, `linux-amd64` / `linux-amd64-dev` variants).
- Nix with flakes enabled.
- `lgs` (alias for `logos-scaffold`) on `PATH`. The `lgs basecamp` subcommand currently lives on the `feature/basecamp` branch of `logos-scaffold`. Install via `cargo install --path .` from `~/src/logos-co/logos-scaffold` (after `git checkout feature/basecamp` there), or `cargo install --git https://github.com/logos-co/logos-scaffold --branch feature/basecamp --locked`.

## Gotchas worth knowing up front

### From `lgs basecamp docs`
- Any flake we build must expose `packages.<system>.lgx`. `lgx-portable` alone is not enough.
- If `delivery_module` ends up as a flake input, its transitive `logos-module-builder` must `follows` our top-level pin (see `logos-co/logos-module-builder#83`). Slice 6 will hit this.

### From experience in this repo
- The scaffolded `#ui-qml-backend` template (on `master`-branch module-builder) is **wrong** for `tutorial-v1`. Use `#ui-module`, which maps to tutorial-v1's Part 3. If you find yourself in this project with `.rep` files, `LogosViewPluginBase`, `mkLogosQmlModule`, or `type: "ui_qml"`, you are on the wrong tutorial.
- `logos_module()`'s CMake target emits `<name>_plugin.so`, but basecamp looks up UI plugins at `<dir>/<dir>.so`. Our `flake.nix` has a `postInstall` hook that also writes `<name>.so`. Upstream bug: `logos-co/logos-basecamp#136`.
- The scaffolded `src/ui_example_*` filenames need to be renamed to match the module name, and the IID in `Q_PLUGIN_METADATA` must be `IComponent_iid` (not the module-specific interface IID — that threw us earlier).
