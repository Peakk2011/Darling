import { app, BrowserWindow } from 'electron'
import { createRequire } from 'module'
const require = createRequire(import.meta.url)
const darling = require('../js/darling-bridge.cjs')

function now() { return performance.now() }

async function benchmarkDarling() {
    const results = {}
    const w = 800, h = 600

    const t0 = now()
    // measure Darling native window creation
    const tCreateStart = now()
    const dwin = darling.createWindow(w, h)
    const tCreateEnd = now()
    results.darlingCreateMs = tCreateEnd - tCreateStart

    try { darling.showDarlingWindow(dwin) } catch (e) { }

    // now create BrowserWindow and reparent
    const tEmbedStart = now()
    const bw = new BrowserWindow({ width: w, height: h, show: false })
    const buf = bw.getNativeWindowHandle()
    const eleHWND = Number(buf.readBigUInt64LE(0))

    // set styles + parent + pos
    const WS_CHILD = 0x40000000
    const WS_POPUP = 0x80000000
    const SWP_SHOWWINDOW = 0x0040
    try {
        darling.setWindowStyles(eleHWND, WS_CHILD, WS_POPUP)
        const dariHWND = darling.getHWND()
        darling.setParent(eleHWND, dariHWND)
        darling.setWindowPos(eleHWND, 0, 0, w, h, SWP_SHOWWINDOW)
    } catch (e) {
        console.warn('embed ops failed', e)
    }

    await bw.loadURL('about:blank')
    bw.show()
    const tEmbedEnd = now()

    results.embedMs = tEmbedEnd - tEmbedStart
    results.totalMs = tEmbedEnd - t0
    // clean up
    bw.close()
    return results
}

async function benchmarkPlain() {
    const results = {}
    const w = 800, h = 600
    const t0 = now()
    const tCreateStart = now()
    const bw = new BrowserWindow({ width: w, height: h, show: false })
    const tCreateEnd = now()
    results.browserCreateMs = tCreateEnd - tCreateStart

    const tLoadStart = now()
    await bw.loadURL('about:blank')
    bw.show()
    const tLoadEnd = now()
    results.loadShowMs = tLoadEnd - tLoadStart
    results.totalMs = tLoadEnd - t0
    bw.close()
    return results
}

app.whenReady().then(async () => {
    console.log('Running startup benchmark...')

    try {
        const d = await benchmarkDarling()
        console.log('Darling embed results (ms):', {
            darlingCreate: Math.round(d.darlingCreateMs),
            embed: Math.round(d.embedMs),
            total: Math.round(d.totalMs)
        })
    } catch (e) {
        console.error('Darling benchmark failed:', e)
    }

    try {
        const p = await benchmarkPlain()
        console.log('Plain Electron results (ms):', {
            create: Math.round(p.browserCreateMs),
            loadShow: Math.round(p.loadShowMs),
            total: Math.round(p.totalMs)
        })
    } catch (e) {
        console.error('Plain benchmark failed:', e)
    }

    app.quit()
})
