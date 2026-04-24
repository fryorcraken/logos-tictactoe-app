# tictactoe-app slice 1→5: learn Tutorial Part 3 via lgs basecamp

## Problem Statement
How might we walk Tutorial Part 3 end-to-end by swapping its calculator for a
tictactoe game, using `lgs basecamp` as the daily install/launch loop, and
stopping at the first real checkpoint (local single-player via `launch alice`)?

## Recommended Direction
Single `tictactoe` ui_qml module with a process-isolated C++ backend that
holds both game state and (later) multiplayer wiring. Follow the tutorial
step-for-step; substitute `lgs basecamp {setup,modules,install,launch}` for
`nix run` at the install/launch phase. Game-only `.rep` day 1. First compile
check after slice 3 (backend alone); first end-to-end run at slice 5.

Why this over the reference's two-module split: the user asked for it, the
game is trivial, and one fewer process boundary makes the tutorial's
`setBackend(this)` + `enableRemoting(host)` + replica-factory path the
single thing we're learning.

## Key Assumptions to Validate
- [ ] Scaffolded `flake.nix` pins `logos-module-builder` to a basecamp-v0.1.1-
      compatible rev (likely `tutorial-v1`). → Inspect after `nix flake init`,
      STOP if `main`.
- [ ] `lgs basecamp setup` completes on this machine. → Kick off early in
      background; failure blocks slice 5.
- [ ] Scaffolded filenames match tutorial text (`ui_example_interface.h`). →
      `ls src/` after init, adjust rename step if drifted.
- [ ] Scaffolded `.rep` template is compatible with a game interface shape
      (SLOTs + PROP). → Read whatever it scaffolds, mirror the calc tutorial's
      shape.

## MVP Scope
Slices 1→5:
  1. README + CLAUDE.md skeletons + `nix flake init` + `lgs init` + `git init`
  2. metadata.json (no delivery dep), tictactoe.rep (game-only),
     tictactoe_interface.h rename, CMakeLists update
  3. tictactoe_plugin.{h,cpp} with local game logic; first compile check
  4. qml/Main.qml with 3×3 board + status + new game
  5. `lgs basecamp setup` (if not done) + `modules` + `install` + `launch alice`
     with working single-player tictactoe. → STOP here; multiplayer in a later
     session.

## Not Doing (and Why)
- Multiplayer / delivery_module — slice 6, separate session
- Tests (tutorial Step 10) — optional, not needed for the checkpoint
- `lgx-portable` — only needed for AppImage basecamp, not dev basecamp
- Reference-project's two-module split — user locked single module
- Preemptively applying reference's known-issue workarounds — would violate
  the STOP-before-deviating rule
- Mac / Windows — Linux only, matches tutorial

## Resolved Decisions
- `lgs basecamp setup` runs in the background during slice 2/3. Confirmed.
- If `nix build` fails mid-slice-3 on a non-typo issue: STOP and report.
  Confirmed.
