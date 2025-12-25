# Examples

This folder contains runnable examples and a small startup benchmark for the Darling embedding
prototype.

Files
- `startup-benchmark.mjs`: measures Darling native-window create + embed vs plain Electron startup.
- `electron-main.mjs`: sample that creates a Darling window and reparents an Electron BrowserWindow into it.

Quick benchmark result (single run on developer machine):
- Darling embed: darlingCreate 11 ms, embed 882 ms, total 894 ms
- Plain Electron: create 8 ms, loadShow 514 ms, total 522 ms

Why the embed run is slower
- Most of the time is spent creating and loading the Chromium `BrowserWindow` and showing content. The embedding steps (set styles, SetParent, SetWindowPos) add extra Win32/DWM work and synchronous round-trips from Node -> native which increase wall-clock time.

Practical ways to reduce startup latency
1) Pre-create work in parallel — create the Darling native window earlier (app startup) so its creation overlaps other initialization.
2) Batch native calls — add a single N-API call that performs styles/parent/position in one native function to avoid multiple sync round-trips.
3) Reuse windows — reuse a `BrowserWindow` or use `BrowserView` instead of creating many top-level windows.
4) Reduce web load — minimize what the renderer loads on startup (fewer resources, local files, smaller bundle).
5) Build production artifacts — run the app with production build flags and ensure native addon is Release-built (node-gyp rebuild in Release is already used here).
6) Disable unnecessary visual effects during startup — experiment with DWM/animation settings or hide until fully positioned.

How to reproduce the benchmark
Run the script under Electron (from repo root):

```bash
npx electron examples/startup-benchmark.mjs
```

To get a stable number, run several iterations and average the `total` value.

Suggested next steps
- Automate N runs and compute mean/median in `startup-benchmark.mjs`.
- Implement a combined native API to set styles+parent+pos in one call.
- Try `BrowserView`-based embedding to avoid creating a second top-level window.
This example shows how to embed an Electron BrowserWindow into a Darling native HWND (Windows).

Prerequisites
- Node.js (v16+)
- Electron installed as a dev dependency
- Build the Darling native addon (`darling.node`) if you need real native calls; a stub exists under `bindings/native_stub.js` for development.

Quick run (from repository root):

1. Install electron (example):

```bash
npm install --save-dev electron
```

2. Start Electron using the example main:

```bash
npx electron --experimentalFeatures ./examples/basic.mjs
```

Notes
- The example assumes `darling.getHWND()` is available and returns a valid HWND (unsigned 64-bit). The `js/darling-bridge.cjs` will fall back to `bindings/native_stub.js` if the native addon is not built.
- Production usage: build the native addon and expose synchronous N-API functions: `getHWND`, `setParent`, `setWindowStyles`, `setWindowPos`, `showWindow`.
- Test both recommended method (SetParent + WS_CHILD) and DWM thumbnail alternative if necessary.
