# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

A single Logos `ui_qml` module (`tictactoe`) with a process-isolated C++ backend. QML view + C++ backend + game logic all live in this one module; `delivery_module` handles multiplayer wire traffic.

This repo is built by **following Tutorial Part 3** verbatim except for documented deviations. The tutorial is the source of truth — do not invent steps, and do not reorder them.

- Tutorial: `~/src/logos-co/logos-tutorial/tutorial-cpp-ui-app.md` (Part 3: C++ UI Module, Process-Isolated)
- Example the tutorial builds: `logos-calc-ui-cpp/` in that same repo — consult it when the tutorial text is ambiguous

## Working rules (non-negotiable, from the user's instructions)

1. **Follow the tutorial.** Do not improvise around steps. Use `agent-skills` as the process layer.
2. **Prefer `lgs basecamp` over raw `nix` commands** whenever the tutorial's action has an `lgs` equivalent. `lgs basecamp {setup,modules,install,launch,reset,build-portable,doctor,docs}` replaces the tutorial's `nix run` / install flow. Initial scaffolding (`nix flake init -t ...#ui-qml-backend`) has **no** `lgs` equivalent — keep it.
3. **STOP on error before deviating.** If a tutorial command fails, do not try a different command or "fix" it without confirming with the user. Report the exact failure and wait.
4. **Capture every deviation in `README.md` immediately** — in the `Deviations` table. Not at the end of coding. Same for every `lgs` command used in place of a tutorial command — record it in the `lgs usage` table as it happens.
5. **Delivery module reference is hands-off until the multiplayer slice. This is absolute.** `~/src/fryorcraken/logos-module-tictactoe` is a separate reference project. You may consult it **only** for `delivery_module` usage patterns, and **only** when slice 6 (multiplayer) is the active slice. That means:
   - Do **not** read its README, flake.nix, CMakeLists, source, QML, metadata.json, scaffold.toml, or any other file outside of slice 6.
   - Do **not** cite it, mirror it, mimic it, or "cross-check against it" for API design, naming, layout, game logic, UI, build config, or anything other than delivery-module wiring.
   - Do **not** use phrases like "the reference project does X" as justification for any design decision outside slice 6. Justify from the tutorial (`~/src/logos-co/logos-tutorial`) and your own reasoning only.
   - Naming the project "tictactoe" is coincidence; that similarity is not a license to look at the reference's tictactoe. This project's design intentionally differs (single module vs multi-module).
   - If you catch yourself reaching for the reference outside slice 6, STOP, tell the user you almost broke the rule, and re-derive from the tutorial.

## Slice-by-slice build flow

Work in thin vertical slices (per `agent-skills:incremental-implementation`). The slice plan lives in `README.md`. Mark each slice's state there as it advances. Do not jump ahead.

Roughly:

1. Skeleton — README, CLAUDE.md, `nix flake init` template, `lgs init`.
2. Configure — `metadata.json`, `src/tictactoe.rep`, `src/tictactoe_interface.h`, `CMakeLists.txt`.
3. Backend — `src/tictactoe_plugin.{h,cpp}` with local game logic.
4. QML — `src/qml/Main.qml`.
5. Local checkpoint — `lgs basecamp install` + `launch alice`, single-player verified.
6. Multiplayer — `delivery_module` input + deps + `.rep` additions + backend wiring.
7. Multiplayer checkpoint — `launch alice` + `launch bob`.

## Key command mapping

| Purpose | Tutorial command | What we use |
|---------|------------------|-------------|
| Scaffold a new ui_qml+backend template | `nix flake init -t github:logos-co/logos-module-builder#ui-qml-backend` | same (no `lgs` alternative) |
| Enable `lgs basecamp` workflow | — (tutorial doesn't cover it) | `lgs init` (once, after scaffolding) |
| One-time basecamp bootstrap | — | `lgs basecamp setup` (slow: fetches basecamp, builds binaries, seeds alice/bob) |
| Capture modules into `scaffold.toml` | — | `lgs basecamp modules` (auto-discovers root flake's `.#lgx`) |
| Build + install `.lgx` into profiles | `nix run` | `lgs basecamp install` |
| Launch the host app | `nix run` | `lgs basecamp launch alice` (or `bob`) |
| Wipe + re-seed | (manual `rm -rf`) | `lgs basecamp reset` |
| Health check | — | `lgs basecamp doctor` |
| Print the requirements doc | — | `lgs basecamp docs` |

## Platform assumptions

- Linux (the tutorial's `.so` / `linux-amd64` variant paths are hardcoded in several places).
- Nix with flakes enabled.
- `lgs` (alias for `logos-scaffold`) on `PATH`. Installed via `cargo install --path .` from `~/src/logos-co/logos-scaffold`.

## Gotchas worth knowing up front

From `lgs basecamp docs`:

- Any flake we build must expose `packages.<system>.lgx`. `lgx-portable` alone is not enough and will fail explicitly.
- If `delivery_module` ends up as a flake input, its transitive `logos-module-builder` must `follows` our top-level pin, or builds through `lgs basecamp install` will pick up a newer incompatible version from `delivery_module`'s own lock.
- Sibling sub-flakes with `path:../<dir>` inputs are auto-rewritten by scaffold, but we're a single root flake so this doesn't apply yet.

From the tutorial:

- The scaffolded template may create `src/ui_example_interface.h`. Rename to match our module name (`tictactoe_interface.h`) and update the IID symbol in the `Q_PLUGIN_METADATA` line, or the plugin won't load.
- `nix flake init` output is expected to include `flake.nix`, `metadata.json`, `CMakeLists.txt`, and a starter plugin + QML file. Inspect what it actually writes before editing.
