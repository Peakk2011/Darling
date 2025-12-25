// CommonJS bridge that wraps the native addon (fallback to stub)
let native;

try {
    // try to load a built native addon first (bindings build path)
    native = require('../bindings/build/Release/darling.node');
} catch (err) {
    try {
        native = require('../bindings/native_stub');
    } catch (e) {
        native = {
            getHWND: () => { throw new Error('Darling native addon not loaded') },
            setParent: () => { throw new Error('Darling native addon not loaded') },
            setWindowStyles: () => { throw new Error('Darling native addon not loaded') },
            setWindowPos: () => { throw new Error('Darling native addon not loaded') },
            showWindow: () => { throw new Error('Darling native addon not loaded') },
        }
    }
}

module.exports = {
    createWindow: (...args) => native.createWindow(...args),
    destroyWindow: (win) => native.destroyWindow(win),
    onCloseRequested: (cb) => native.onCloseRequested(cb),
    showDarlingWindow: (win) => native.showDarlingWindow(win),
    pollEvents: () => native.pollEvents(),
    getHWND: () => native.getHWND(),
    paintFrame: (buffer, w, h) => native.paintFrame(buffer, w, h),
    setParent: (child, parent) => native.setParent(child, parent),
    setWindowStyles: (hwnd, add, remove) => native.setWindowStyles(hwnd, add, remove),
    setWindowPos: (hwnd, x, y, w, h, flags) => native.setWindowPos(hwnd, x, y, w, h, flags),
    showWindow: (hwnd, cmd) => native.showWindow(hwnd, cmd),
}
