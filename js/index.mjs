import { createRequire } from 'module'
const require = createRequire(import.meta.url)

const native = require('../bindings/build/Release/darling.node')

export function createWindow(w, h) {
    return native.createWindow(w, h)
}

export function showWindow(win) {
    native.showWindow(win)
}

export function pollEvents() {
    native.pollEvents()
}
