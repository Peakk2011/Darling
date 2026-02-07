# Examples

Examples of using Darling and a script to measure the startup time of Electron embedding.

Important Files

- `basic.mjs`: Basic example for launching Electron via Darling
- `electron-embed-child.mjs`: Embeds BrowserWindow as a child of Darling HWND
- `startup-benchmark.mjs`: Measures startup time (Darling embed vs. regular Electron)

Run the example (from root)

```bash
npm install
npx electron --experimentalFeatures ./examples/basic.mjs
```

Run the benchmark

```bash
npx electron ./examples/startup-benchmark.mjs
```

## Usage (Win32/Electron)

```js
import { app } from 'electron'
import { CreateWindow } from './js/darling-electron-wrapper.mjs'

app.whenReady().then(() => {
  CreateWindow({
    width: 800,
    height: 600,
    url: 'https://example.com',
    title: 'My Darling Window',
    showIcon: true,                 // false will hide the icon and close WS_SYSMENU
    frameRate: 60,                  // used with polling/renderer
    nativeStylesAdd: 0,             // add GWL_STYLE (e.g., WS_BORDER)
    nativeStylesRemove: 0,          // remove GWL_STYLE (e.g., WS_CAPTION)
    nativeExStylesAdd: 0,           // add GWL_EXSTYLE (e.g., WS_EX_TOPMOST)
    nativeExStylesRemove: 0,        // remove GWL_EXSTYLE
    
    theme: {
        titlebar: 'dark | light',
        content: 'light | light',
    },
    
    onClose: () => app.quit(),      // callback to close
  })
})
```

Note
- If the addon isn't built, it will fallback to `bindings/native_stub.js`
- For addon build instructions, see `README.md` in root.