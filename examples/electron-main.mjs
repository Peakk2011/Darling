import { app, BrowserWindow } from 'electron'
import { createRequire } from 'module'
const require = createRequire(import.meta.url)
const darling = require('../js/darling-bridge.cjs')

app.whenReady().then(async () => {
    let dariHWND
    try {
        // create and show the Darling native window first
        const dw = darling.createWindow(800, 600)
        try { darling.showDarlingWindow(dw) } catch (e) { /* best-effort */ }

        // now read its HWND
        dariHWND = darling.getHWND()
        console.log('Darling HWND:', dariHWND)
    } catch (err) {
        console.warn('darling window creation/getHWND failed:', err)
    }

    const bw = new BrowserWindow({ width: 800, height: 600, show: false })
    const buf = bw.getNativeWindowHandle()
    let eleHWND = null
    try {
        eleHWND = Number(buf.readBigUInt64LE(0))
        console.log('Electron HWND:', eleHWND)
    } catch (e) {
        console.warn('Failed to read BrowserWindow native handle:', e)
    }

    if (eleHWND && dariHWND) {
        try {
            const WS_CHILD = 0x40000000
            const WS_POPUP = 0x80000000
            const SWP_SHOWWINDOW = 0x0040

            // Set window styles to child and remove popup style
            darling.setWindowStyles(eleHWND, WS_CHILD, WS_POPUP)

            // Make Chromium window a child of Darling's HWND
            darling.setParent(eleHWND, dariHWND)

            // Position it to fill the Darling window
            darling.setWindowPos(eleHWND, 0, 0, 800, 600, SWP_SHOWWINDOW)
        } catch (err) {
            console.error('Failed reparenting Electron window:', err)
        }
    }

    await bw.loadURL('about:blank')
    bw.show()

    // keep BrowserWindow in sync on resize
    bw.on('resize', () => {
        const r = bw.getContentBounds()
        if (eleHWND) {
            try {
                darling.setWindowPos(eleHWND, 0, 0, r.width, r.height, 0)
            } catch (e) {
                console.warn('Failed to update child pos:', e)
            }
        }
    })
})
