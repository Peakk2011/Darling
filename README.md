## Darling

Darling is an experimental library for creating native Windows windows and embedding Electron windows via a native addon.

Brief Features:

- Creates native (Win32) windows.
- Includes Electron embedding examples in `examples/`.
- Includes a native addon (Node) for connecting JavaScript with Win32.

Why it matters:

- Native window control, no compromises: Titlebar, styles, and ex‑styles are yours.
- Embed Electron as a real child window, so input/resize/focus feels native.
- Win32 titlebar theming (dark/light) and caption colors (Win11).
- Keep web UI, gain native‑grade window behavior.

Performance vs standard Electron:

- Chromium cost is unchanged; this is not a magic speed boost.
- The native host window is fast and can be created early or in parallel.
- Embedding adds overhead (SetParent/SetWindowPos/SetWindowStyles + JS→native).
- Net: feels faster when you overlap work, not when you do everything serially.

Installation and Build (Windows):

1. Build core (CMake):

- `cmake -S core -B build`
- `cmake --build build --config Debug`

2. Build native addon:

- `cd bindings`
- `npm install`
- `npx node-gyp rebuild --release`

3. Run examples (Electron):

- Return to root and run:
- `npm install`
- `npx electron --experimentalFeatures ./examples/basic.mjs`

Note:

- If the addon is not built, it will fallback to the stub. `bindings/native_stub.js`
- See `examples/README.md` for more details and other examples.

## Development Usage (Extend/Customize)

This section is for contributors who want to continue development or add features.

What you run:
- Build core (CMake) and the native addon (node-gyp) to produce `bindings/build/Release/darling.node`.
- Use Electron examples in `examples/` to verify changes end-to-end.

Where to change code:
- Win32 core: `core/src/platform/win32/impl/window.c`
- Public C API: `core/include/darling.h`
- Node addon: `bindings/src/darling_node.cc`
- JS bridge: `js/darling-bridge.cjs`
- Electron wrapper: `js/darling-electron-wrapper.mjs`
- Type defs: `js/darling.d.ts`

Recommended workflow:
- Make a change in C (`window.c` / `darling.h`)
- Rebuild addon (`bindings/`)
- Update bindings + JS wrapper as needed
- Run example: `npx electron --experimentalFeatures ./examples/basic.mjs`

Packaging note:
- The `.node` file must be shipped outside ASAR.
