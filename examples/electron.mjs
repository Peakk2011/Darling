
import { app, BrowserWindow } from 'electron'
import path from 'path'
import { fileURLToPath } from 'url'

// Our native addon API, bridged for ESM
import * as darling from '../js/index.mjs'

const __dirname = path.dirname(fileURLToPath(import.meta.url))

// Keep references to our windows to prevent them from being garbage collected.
let darlingWindow
let electronWindow

async function main() {
  await app.whenReady()

  // --- Step 1: Create the native parent window ---
  // This is the external, top-level window controlled by our C library.
  darlingWindow = darling.createWindow(800, 600)
  const parentHwnd = darling.getHWND()
  console.log(`Darling parent HWND: ${parentHwnd}`)

  // --- Step 2: Create the Electron child window ---
  // This is a frameless window that will draw our web content.
  electronWindow = new BrowserWindow({
    frame: false,
    width: 800,
    height: 600,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js')
    }
  })

  // Get the HWND of our Electron window.
  const childHwnd = electronWindow.getNativeWindowHandle()
  console.log(`Electron child HWND: ${childHwnd.readBigUInt64LE()}`)

  // --- Step 3: Embed the Electron window into the native window ---
  // This is the magic step. We tell Windows to make the Electron window a child
  // of our native window.
  darling.setParent(childHwnd.readBigUInt64LE(), parentHwnd)
  
  // Also important: ensure the child window's style is correct for being a child.
  const WS_CHILD = 0x40000000
  darling.setWindowStyles(childHwnd.readBigUInt64LE(), WS_CHILD, 0)

  // Move the child window to cover the parent's client area.
  const SWP_NOSIZE = 0x0001
  const SWP_NOMOVE = 0x0002
  const SWP_NOZORDER = 0x0004
  darling.setWindowPos(childHwnd.readBigUInt64LE(), 0, 0, 800, 600, SWP_NOZORDER)


  // --- Step 4: Set up the graceful shutdown callback ---
  // This is the most critical part to prevent DWM crashes.
  darling.onCloseRequested(() => {
    console.log('Native window close requested. Starting graceful shutdown...')

    // 1. Detach the Electron window.
    // The second argument '0' represents a NULL parent.
    console.log('Detaching child window...')
    darling.setParent(childHwnd.readBigUInt64LE(), 0)

    // 2. Destroy the Electron window.
    console.log('Destroying Electron window...')
    electronWindow.close() // or .destroy()

    // 3. Destroy the native window.
    console.log('Destroying native window...')
    darling.destroyWindow(darlingWindow)

    // 4. Quit the app.
    app.quit()
  })
  
  // --- Step 5: Finalize and Show ---
  electronWindow.loadFile('examples/index.html')
  darling.showDarlingWindow(darlingWindow)

  // Start polling for native window events.
  setInterval(() => {
    darling.pollEvents()
  }, 16) // ~60 FPS
}

main().catch(console.error)

app.on('window-all-closed', () => {
  // Don't quit automatically. The close callback handles it.
})

// --- Helper files needed for this example ---
// You will need to create:
// 1. examples/index.html (a simple HTML file)
// 2. examples/preload.js (can be an empty file for now)
