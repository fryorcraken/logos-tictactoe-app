# logos-tictactoe-app

[![build](https://github.com/fryorcraken/logos-tictactoe-app/actions/workflows/build.yml/badge.svg)](https://github.com/fryorcraken/logos-tictactoe-app/actions/workflows/build.yml)

Multiplayer tic-tac-toe built as a single Logos C++ UI module (`type: "ui"`). Game logic lives in a `TictactoeBackend` QObject; the UI is a QML view loaded into the plugin via `QQuickWidget` (Tutorial Part 3, **Option A**). Later slices add `delivery_module` for multiplayer.

Built by following **the `tutorial-v1` branch** of [`logos-tutorial`'s Part 3](https://github.com/logos-co/logos-tutorial/blob/tutorial-v1/tutorial-cpp-ui-app.md), using `lgs basecamp` in place of raw `nix run` for install/launch where possible. Both `logos-module-builder` and the tutorial repo are pinned to `tutorial-v1` for consistency with what `lgs basecamp`'s source-built basecamp expects.

## Status

**Slice 5 (local single-player checkpoint) ✅ green.** `lgs basecamp install` + `lgs basecamp launch alice` loads the tictactoe sidebar entry, renders the 3×3 board, plays X/O with win/draw detection, and resets via "New Game." Multiplayer (slice 6) is next.

## Design (differs from the tutorial reference)

The tutorial's reference case (`calc_ui_cpp`) has two modules — a core `calc_module` with business logic and a `calc_ui_cpp` UI module that calls it through a generated SDK. Here we collapse that into a single module:

- **tictactoe** (`type: "ui"`) — C++ Qt plugin. `createWidget(LogosAPI*)` returns a `QQuickWidget` loading `Main.qml`; a `TictactoeBackend` QObject is set as the QML `backend` context property. The backend owns board state, turn management, win/draw detection, and (later) multiplayer wiring via `delivery_module`.

Rationale: the game rules are tiny (a 3×3 board and a winner check), not worth a second process boundary on their own. With no external module to call in slices 1–5, we also skip the generated `logos_sdk.h` path — the backend never needs `LogosModules`. Slice 6 adds one dep (`delivery_module`) so the backend starts using `LogosAPI*`.

## Quick start

Prerequisites:

- [Nix](https://nixos.org/download.html) with flakes enabled.
- [`lgs`](https://github.com/logos-co/logos-scaffold) (alias for `logos-scaffold`) on `$PATH`. The `lgs basecamp` subcommand currently lives on the `feature/basecamp` branch, so install with `cargo install --git https://github.com/logos-co/logos-scaffold --branch feature/basecamp --locked`.

```bash
git clone https://github.com/fryorcraken/logos-tictactoe-app.git
cd logos-tictactoe-app

lgs basecamp setup          # one-time: pins + builds basecamp & lgpm, seeds alice/bob
lgs basecamp install        # builds the .lgx, installs into alice + bob
lgs basecamp launch alice   # opens basecamp; click the tictactoe sidebar entry
```

## Slice plan

| # | Slice | State |
|---|-------|-------|
| 1 | Project skeleton (README, CLAUDE.md, scaffold, `lgs init`, `lgs basecamp setup`) | ✅ done |
| 2 | Configure module (`metadata.json`, interfaces/, CMakeLists) | ✅ done |
| 3 | C++ backend with local game logic + plugin shell | ✅ done |
| 4 | QML view (3×3 board, status, new game) | ✅ done |
| 5 | Local checkpoint: `lgs basecamp install` + `launch alice` | ✅ done |
| 6 | Multiplayer via `delivery_module` | in progress |
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
| 5 | Tutorial `flake.nix` has no install hooks. | `postInstall` hook copies `$out/lib/tictactoe_plugin.so` → `$out/lib/tictactoe.so`. | Basecamp (the host app `lgs basecamp launch` runs) looks up UI plugins at `<plugin-dir>/<plugin-dir>.so` but `logos_module()`'s CMake target emits `<name>_plugin.so`, so after `lgs basecamp install` the plugin sat as `plugins/tictactoe/tictactoe_plugin.so` and basecamp silently skipped it — the sidebar showed no tictactoe entry. Tracked upstream as [`logos-co/logos-basecamp#136`](https://github.com/logos-co/logos-basecamp/issues/136). The hook ships both names so basecamp finds `tictactoe.so` next to the neighboring `counter/counter.so`, `webview_app/webview_app.so`, etc. |
| 6 | Tutorial-v1 Part 3 only covers a `calc_module` dependency (computation). | Depend on **`delivery_module`** (network broadcast). | Tutorial shows how to add a dep + consume the generated `logos_sdk.h`; we swap the target module but the mechanism is the same. `delivery_module` is how Logos ships an over-the-wire transport (libwaku under the hood); it's the natural way to add multiplayer without inventing our own protocol. Not in the tutorial; extends it. |
| 6 | Tutorial's `CalcBackend` uses the generated typed SDK for every call (`m_logos->calc_module.add(…)`). | Use the typed SDK **only for `.on()` event subscription**. RPCs go through `m_logosAPI->getClient("delivery_module")->invokeRemoteMethod(…)`. | At the pinned `delivery_module` rev (tag 1.0.0 tutorial-v1-compat), RPC slots return plain `bool` on the wire — the pre-LogosResult format basecamp v0.1.1 ships. The generated wrapper decodes replies as `LogosResult`, so `QVariant(bool).value<LogosResult>()` yields `{success=false}` on every RPC. Typed wrapper reports spurious failures. Bypass it for RPCs. [`logos-co/logos-delivery-module/pull/23`](https://github.com/logos-co/logos-delivery-module/pull/23) context. |
| 6 | No mention of flake-input `follows` wiring. | Add `delivery_module.inputs.logos-module-builder.follows = "logos-module-builder"` in `flake.nix`. | Without it, `delivery_module`'s own `flake.lock` pulls a second `logos-module-builder` (typically master), which can silently win in parts of the build graph and produce wire-format mismatches. [`logos-co/logos-module-builder#83`](https://github.com/logos-co/logos-module-builder/issues/83). |
| 6 | `createNode` expects a JSON config with real port numbers. Tutorial doesn't discuss port selection. | Pick free ephemeral ports ourselves via `socket()` + `bind(port=0)` + `getsockname()` + `close()`, then pass the picked numbers as `tcpPort` / `discv5UdpPort`. | `delivery_module` rejects `0` in its JSON config (`[logos-co/logos-delivery-module#24](https://github.com/logos-co/logos-delivery-module/issues/24)`) even though libwaku itself accepts port 0 as "OS-assigned." Without this, `createNode` fails silently. Inherent TOCTOU window between our `close()` and the module's `bind()`; acceptable for local dev multiplayer. |
| 6 | Tutorial RPCs use the SDK default reply timeout (20 s). | Every delivery RPC passes `Timeout(90000)` — 90 s. | `createNode` spins up libwaku (20–25 s cold). The default 20 s race-aborts the replica mid-handshake; that makes `delivery_module`'s `logos_host` child exit and basecamp segfaults dispatching a socket-read into the torn-down replica. 90 s gives libwaku room without surprising the user. |
| 6 | Tutorial doesn't cover synchronous-vs-async RPC choice. | Use the **synchronous** `invokeRemoteMethod`. The UI freezes "connecting…" while `createNode` + `start` + `subscribe` complete (several hundred ms plus the libwaku start). | `invokeRemoteMethodAsync` only exists on `logos-module-builder` master; we're pinned to `tutorial-v1` for wire-format compat. Sync is intentional here, not an oversight. When basecamp ships a build that bundles the newer `delivery_module` wrapper this can become async. |
| 6 | N/A — tutorial doesn't use events. | `delivery_module.on(…)` handlers registered once per `TictactoeBackend` lifetime; guarded by `m_mpHandlersRegistered`. | `LogosAPIConsumer::onEvent` appends without dedup and exposes no `.off()`. A naive enable → disable → enable cycle would stack duplicate handlers, so each event arrival would fire N times. The lambdas gate on `m_mpEnabled` so they're inert when multiplayer is off. |
| 6 | N/A | `messageReceived` payload is **double-base64-decoded** before protobuf parsing. | `delivery_module` base64-encodes the payload on the wire (`data[2]` is base64). We also base64-encode our payload before `send()` so it survives the typed-wrapper `QString` transport. Full receive path: `decode(delivery base64)` → `decode(our base64)` → protobuf `ParseFromArray`. Mirrors reference; will simplify once delivery_module's wire path is unified. |
| 6 | N/A | Naive, known-broken "self-echo" semantics for remote moves. | `delivery_module` broadcasts to **all** subscribers including the sender, so every own move echoes back. We apply it as the current local player (same as remote plays). Own echoes always land on an occupied cell and are rejected by `play()`'s existing guard → benign. Accepted limitation; real multiplayer needs per-session ID + player assignment, not in scope. |

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
| 5 | (tutorial doesn't cover it — scaffold.toml is `lgs`-specific) | `lgs basecamp modules` | Auto-discovers the root flake's `#lgx` and writes it into `scaffold.toml`'s `[basecamp.modules]` table (with the module name resolved from `metadata.json`). Captures the install set. |
| 5 | Tutorial Step 10: `nix run .` launches `logos-standalone-app` with this plugin | `lgs basecamp install` | Builds `.#lgx` and installs into every seeded profile (alice, bob) via `lgpm`. Exercises the basecamp-facing toolchain, not a standalone runner. |
| 5 | Tutorial Step 10: `nix run .` launches the host | `lgs basecamp launch alice` | Launches basecamp (the Logos host app, built from source during `lgs basecamp setup`) in the alice profile with clean-slate semantics. Implicitly re-runs `install` before launching. |

## CI

[`.github/workflows/build.yml`](.github/workflows/build.yml) runs on every push to `master`, every pull request, and every tag matching `v*` or `slice-*`. It:

1. Installs Nix with flakes.
2. Installs `lgs` from source (`cargo install --git logos-co/logos-scaffold`).
3. Runs `lgs basecamp setup` then `lgs basecamp install` — same commands developers use locally.
4. Uploads the resulting `.lgx` as a per-run artifact.
5. On `v*` / `slice-*` tags, attaches the `.lgx` to the corresponding GitHub Release.

To publish a slice checkpoint:

```bash
git tag slice-5
git push origin slice-5
# CI builds and attaches logos-tictactoe-module-lib.lgx to the slice-5 Release
```

## Build + run

Local dev, after `lgs basecamp setup` has been run once:

```bash
lgs basecamp install               # build + install into alice + bob
lgs basecamp launch alice          # open basecamp as alice
# — click the tictactoe icon in the sidebar to open the board —
lgs basecamp launch bob            # (after slice 6) second peer for multiplayer
```

QML-only edits (layout, styling, JS logic in `src/qml/Main.qml`) can be iterated without rebuilding by setting `QML_PATH`:

```bash
# In the basecamp process's environment (set before `lgs basecamp launch`):
export QML_PATH="$PWD/src/qml"
```

Per [Tutorial Step 7.4](https://github.com/logos-co/logos-tutorial/blob/tutorial-v1/tutorial-cpp-ui-app.md), when `QML_PATH` is set the plugin loads `Main.qml` from disk instead of the embedded Qt resource — no Nix rebuild needed. Any `.cpp`/`.h`/CMakeLists/metadata change still requires `lgs basecamp install`.

Reset a profile if something gets stuck:

```bash
lgs basecamp reset
```
