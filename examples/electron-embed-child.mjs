import { app, BrowserWindow } from 'electron'
import { createRequire } from 'module'
const require = createRequire(import.meta.url)
const darling = require('../js/darling-bridge.cjs')

app.whenReady().then(async () => {
    const w = 800, h = 600

    // Create the Electron BrowserWindow first
    const bw = new BrowserWindow({ width: w, height: h, show: false })
    const buf = bw.getNativeWindowHandle()
    const eleHWND = Number(buf.readBigUInt64LE(0))
    console.log('Electron HWND:', eleHWND)

    // Create Darling as a child of the Electron window (WS_CHILD)
    const dwin = darling.createWindow(w, h, BigInt(eleHWND))
    try { darling.showDarlingWindow(dwin) } catch (e) {}
    try {
        console.log('Darling HWND:', darling.getHWND())
    } catch (e) {
        console.warn('getHWND failed', e)
    }

    await bw.loadURL('about:blank')
    bw.show()

    bw.on('resize', () => {
        const r = bw.getContentBounds()
        try {
            const SWP_NOZORDER = 0x0004
            darling.setWindowPos(Number(buf.readBigUInt64LE(0)), 0, 0, r.width, r.height, SWP_NOZORDER)
        } catch (e) {
            console.warn('Failed to resize child:', e)
        }
    })
})
