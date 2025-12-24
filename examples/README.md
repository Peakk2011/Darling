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
npx electron --experimentalFeatures ./examples/electron-main.mjs
```

Notes
- The example assumes `darling.getHWND()` is available and returns a valid HWND (unsigned 64-bit). The `js/darling-bridge.cjs` will fall back to `bindings/native_stub.js` if the native addon is not built.
- Production usage: build the native addon and expose synchronous N-API functions: `getHWND`, `setParent`, `setWindowStyles`, `setWindowPos`, `showWindow`.
- Test both recommended method (SetParent + WS_CHILD) and DWM thumbnail alternative if necessary.
