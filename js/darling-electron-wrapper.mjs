import { app, BrowserWindow } from 'electron'
import { createRequire } from 'module'
const require = createRequire(import.meta.url)
const darling = require('./darling-bridge.cjs')

// This module encapsulates the complexity of setting up a Darling native window
// with an offscreen Electron renderer. It provides a simplified API.

export function CreateWindow(options) {
    const {
        width = 800,
        height = 600,
        url = 'about:blank',
        onClose = () => app.quit(),
        frameRate = 60
    } = options || {};

    let darlingWindowHandle = null;
    let pollInterval = null;

    // 1. Create the native host window.
    const dw = darling.createWindow(width, height)
    darling.showDarlingWindow(dw)
    darlingWindowHandle = dw; // Store for cleanup

    // 2. Create the offscreen Electron window.
    const bw = new BrowserWindow({
        width: width,
        height: height,
        show: false, // Never show this window.
        webPreferences: {
            offscreen: true, // Crucial for this approach!
            nodeIntegration: false,
            contextIsolation: true,
        }
    })

    // 3. Listen for the 'paint' event to get new frames.
    bw.webContents.on('paint', (event, dirty, image) => {
        const size = image.getSize()
        if (size.width === 0 || size.height === 0) {
            return
        }
        // Get the raw BGRA bitmap data.
        const buffer = image.toBitmap()
        
        // Send the frame to the native window to be painted.
        try {
            darling.paintFrame(buffer, size.width, size.height)
        } catch (e) {
            console.error('Failed to paint frame:', e)
        }
    })

    // Set a desired frame rate.
    bw.webContents.setFrameRate(frameRate)

    // 4. Load content into the offscreen window.
    bw.loadURL(url)

    // 5. Set up a message loop poller.
    // This is crucial to keep the native window responsive and processing
    // WM_PAINT and WM_CLOSE messages.
    pollInterval = setInterval(() => {
        try {
            darling.pollEvents()
        } catch (e) {
            console.error('Failed polling events:', e)
            if (pollInterval) clearInterval(pollInterval)
            if (onClose) onClose(); // Trigger close if polling fails
        }
    }, 1000 / frameRate)

    // 6. Set up cleanup logic.
    const cleanup = () => {
        console.log('Cleaning up Darling Electron Window...')
        if (pollInterval) {
            clearInterval(pollInterval)
            pollInterval = null;
        }
        if (darlingWindowHandle) {
            try {
                darling.destroyWindow(darlingWindowHandle)
                darlingWindowHandle = null;
            } catch (e) {
                console.error('Failed to destroy darling window:', e)
            }
        }
        // Destroy the offscreen BrowserWindow as well
        if (!bw.isDestroyed()) {
            bw.destroy();
        }
    }
    
    // Ensure cleanup happens when Electron quits.
    app.on('will-quit', cleanup)

    // Also handle the native window being closed by the user (e.g., Alt+F4).
    darling.onCloseRequested(() => {
        console.log('Darling window close requested.')
        if (onClose) onClose(); // Use the provided callback
    })

    // Return the offscreen BrowserWindow instance in case it's needed for further control
    return bw;
}

// NOTE: This implementation only handles rendering. It does NOT forward
// input (mouse, keyboard) from the Darling window to the Electron window.
// That is a much more complex task requiring more native code to capture
// window messages (e.g., WM_MOUSEMOVE, WM_KEYDOWN) and forward them to JS
// using `webContents.sendInputEvent()`.
