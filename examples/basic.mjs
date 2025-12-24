import { createWindow, showWindow, pollEvents } from '../js/index.mjs'

const win = createWindow(800, 600)
showWindow(win)

setInterval(() => {
    pollEvents()
}, 16)