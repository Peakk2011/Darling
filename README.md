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