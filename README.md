# logos-tictactoe-app

Multiplayer tic-tac-toe built as a single Logos C++ UI module (`type: "ui"`). Game logic lives in a `TictactoeBackend` QObject; the UI is a QML view loaded into the plugin via `QQuickWidget` (Tutorial Part 3, **Option A**). Later slices add `delivery_module` for multiplayer.

Built by following **the `tutorial-v1` branch** of [`logos-tutorial`'s Part 3](https://github.com/logos-co/logos-tutorial/blob/tutorial-v1/tutorial-cpp-ui-app.md), using `lgs basecamp` in place of raw `nix run` for install/launch where possible. Both `logos-module-builder` and the tutorial repo are pinned to `tutorial-v1` for consistency with what `lgs basecamp`'s source-built basecamp expects.

## Design (differs from the tutorial reference)

The tutorial's reference case (`calc_ui_cpp`) has two modules — a core `calc_module` with business logic and a `calc_ui_cpp` UI module that calls it through a generated SDK. Here we collapse that into a single module:

- **tictactoe** (`type: "ui"`) — C++ Qt plugin. `createWidget(LogosAPI*)` returns a `QQuickWidget` loading `Main.qml`; a `TictactoeBackend` QObject is set as the QML `backend` context property. The backend owns board state, turn management, win/draw detection, and (later) multiplayer wiring via `delivery_module`.

Rationale: the game rules are tiny (a 3×3 board and a winner check), not worth a second process boundary on their own. With no external module to call, we also skip the generated `logos_sdk.h` path — the backend never needs `LogosModules`.

## Status

See [slice plan](#slice-plan) for what's built and what's next.

## Slice plan

| # | Slice | State |
|---|-------|-------|
| 1 | Project skeleton (README, CLAUDE.md, scaffold, `lgs init`, `lgs basecamp setup`) | ✅ done |
| 2 | Configure module (`metadata.json`, interfaces/, CMakeLists) | ✅ done |
| 3 | C++ backend with local game logic + plugin shell | ✅ done |
| 4 | QML view (3×3 board, status, new game) | ✅ done |
| 5 | Local checkpoint: `lgs basecamp install` + `launch alice` | in progress |
| 6 | Multiplayer via `delivery_module` | pending |
| 7 | Multiplayer checkpoint: `launch alice` + `launch bob` | pending |

## Deviations from the tutorial

Captured the moment they happen — not at the end.

| Slice | Tutorial says | We do | Why |
|-------|---------------|-------|-----|
| 1 | `calc_ui_cpp` with a separate `calc_module` dependency | Single `tictactoe` module containing both UI and game logic | Game rules are trivial; no second process boundary needed. See [Design](#design-differs-from-the-tutorial-reference). |
| 1 | Tutorial Step 1 says `nix flake init -t github:logos-co/logos-module-builder/tutorial-v1#ui-module` | Scaffolded with `nix flake init -t github:logos-co/logos-module-builder#ui-qml-backend` (no ref, wrong template name) | Mistake: I was reading the **`master`-branch tutorial** (`tutorial-cpp-ui-app.md` at HEAD), which describes a newer process-isolated ui_qml+backend pattern (`#ui-qml-backend` template, `mkLogosQmlModule` compiling C++, `.rep` files, `LogosViewPluginBase`). That pattern only exists on `logos-module-builder`'s `master`. The **`tutorial-v1` branch** of the tutorial describes the older `ui-module` template + `mkLogosModule` + `IComponent` interface + direct `LogosAPI*` calls. The `lgs basecamp` / basecamp v0.1.1 ecosystem needs `tutorial-v1`, so that's the tutorial we actually follow. After discovering this we kept the files we already had but rewrote them to match the `tutorial-v1` shape (see row below). |
| 1 | Step 9 flake.nix pins `logos-module-builder.url = "github:logos-co/logos-module-builder/tutorial-v1"` (with a Step 1 note saying the scaffold's unpinned URL should be replaced). | Same — pinned to `tutorial-v1`. | No deviation in the end; matches tutorial-v1 Step 9. |
| 2–4 | Step 2 sets `"type": "ui"`; Step 5 plugin inherits `QObject, TictactoeInterface, IComponent` with `Q_PLUGIN_METADATA(IID IComponent_iid …)`; Step 6 backend is a separate `QObject` with `Q_INVOKABLE` methods; Step 7 uses `QQuickWidget` + QML context property. | Same, but backend is `TictactoeBackend` (game logic — no `LogosModules` since `dependencies: []`) and QML is a 3×3 board rather than a calculator. | No structural deviation — domain swap only. |
| — | Tutorial's `CalcBackend` constructor takes `LogosAPI* api` and stores a `LogosModules*` wrapper. | `TictactoeBackend(LogosAPI* api, …)` — takes but **ignores** `LogosAPI*` for slices 1–5 (no module dependencies). | `LogosAPI*` is needed in slice 6 to call `delivery_module` for multiplayer; passing it through now avoids constructor churn later. |

### Earlier deviations we since walked back

These are recorded for history — they reflect wrong turns before we realized the `tutorial-v1` branch of the tutorial existed:

- Briefly swapped `mkLogosQmlModule` → `mkLogosModule` while assuming we were following the `master`-branch Tutorial Part 3 (ui_qml + backend). That swap is effectively subsumed by the full pivot to `tutorial-v1`'s Part 3, which is a different architecture (widget plugin + QML loaded via `QQuickWidget`, not a process-isolated ui-host).
- Briefly considered vendoring a `lib/libtictactoe.c` + `preConfigure` hook. Walked back — that pattern is for Tutorial Part 1's external-C-library case, not Part 3.

## `lgs` usage (replacing tutorial commands)

Where `lgs basecamp` supersedes a raw `nix` command from the tutorial.

| Slice | Tutorial command | `lgs` equivalent used | Notes |
|-------|------------------|-----------------------|-------|
| 1 | `nix flake init -t github:logos-co/logos-module-builder/tutorial-v1#ui-module` | (none — kept tutorial command, but initially scaffolded with the wrong template name; see deviation row) | `lgs` has no template-init subcommand; `lgs basecamp` is for dogfooding, not scaffolding. |
| 1 | (tutorial doesn't cover `scaffold.toml`) | `lgs init` | Required by `lgs basecamp *` commands. Creates `scaffold.toml` + `.scaffold/{state,logs}`, appends `.scaffold` to `.gitignore`. |
| 1 | (tutorial doesn't cover this) | `lgs basecamp setup` | One-time bootstrap: pins basecamp repo, builds `basecamp` + `lgpm` binaries, seeds `alice` / `bob` profile dirs under `.scaffold/basecamp/profiles/`. Prerequisite for `basecamp modules/install/launch`. |
| 5 | Tutorial Step 10 (`nix run .`) launches `logos-standalone-app` with this plugin | `lgs basecamp install` + `lgs basecamp launch alice` | Installs `.lgx` into the alice profile, then launches basecamp as alice. Matches the "install into basecamp" workflow rather than the tutorial's standalone runner. |

_Row for `lgs basecamp modules` added when it's next run against the pivoted code._

## Build + run (populated as slices complete)

_TBD — filled in once slice 5's `lgs basecamp install` + `launch alice` checkpoint is green._
