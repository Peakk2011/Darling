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
    }
}