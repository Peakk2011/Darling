// Fallback native stub for development when .node is not built
module.exports = {
    getHWND() {
        throw new Error('native addon not built — getHWND() not available')
    },
    setParent() {
        throw new Error('native addon not built — setParent() not available')
    },
    setWindowStyles() {
        throw new Error('native addon not built — setWindowStyles() not available')
    },
    setWindowPos() {
        throw new Error('native addon not built — setWindowPos() not available')
    },
    showWindow() {
        throw new Error('native addon not built — showWindow() not available')
    },
    setChildWindow() {
        throw new Error('native addon not built — setChildWindow() not available')
    },
    setWindowTitle() {
        throw new Error('native addon not built — setWindowTitle() not available')
    },
    setWindowIconVisible() {
        throw new Error('native addon not built — setWindowIconVisible() not available')
    },
    showDarlingWindow() {
        throw new Error('native addon not built — showDarlingWindow() not available')
    },
    hideDarlingWindow() {
        throw new Error('native addon not built — hideDarlingWindow() not available')
    },
    focusDarlingWindow() {
        throw new Error('native addon not built — focusDarlingWindow() not available')
    },
    isVisible() {
        throw new Error('native addon not built — isVisible() not available')
    },
    isFocused() {
        throw new Error('native addon not built — isFocused() not available')
    },
    setWindowOpacity() {
        throw new Error('native addon not built — setWindowOpacity() not available')
    },
    setAlwaysOnTop() {
        throw new Error('native addon not built — setAlwaysOnTop() not available')
    },
    setWindowExStyles() {
        throw new Error('native addon not built — setWindowExStyles() not available')
    },
    isDarkMode() {
        throw new Error('native addon not built — isDarkMode() not available')
    },
    setDarkMode() {
        throw new Error('native addon not built — setDarkMode() not available')
    },
    setAutoDarkMode() {
        throw new Error('native addon not built — setAutoDarkMode() not available')
    },
    setTitlebarColors() {
        throw new Error('native addon not built — setTitlebarColors() not available')
    },
    setTitlebarColor() {
        throw new Error('native addon not built — setTitlebarColor() not available')
    },
    setCornerPreference() {
        throw new Error('native addon not built — setCornerPreference() not available')
    },
    flashWindow() {
        throw new Error('native addon not built — flashWindow() not available')
    },
    getDpi() {
        throw new Error('native addon not built — getDpi() not available')
    },
    getScaleFactor() {
        throw new Error('native addon not built — getScaleFactor() not available')
    }
}