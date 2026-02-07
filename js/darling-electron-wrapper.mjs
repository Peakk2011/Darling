import { app, BrowserWindow } from 'electron';
import { createRequire } from 'module';

const require = createRequire(import.meta.url);
const darling = require('./darling-bridge.cjs');

let windowAllClosedHandlerAttached = false

export const CreateWindow = async (options) => {
    if (!windowAllClosedHandlerAttached) {
        windowAllClosedHandlerAttached = true;

        app.on('window-all-closed', () => {
            app.quit();
        });
    }

    if (!app.isReady()) {
        await app.whenReady();
    }

    const {
        width = 800,
        height = 600,
        url = 'about:blank',
        onClose = () => app.quit(),
        frameRate = 60,
        title = '',
        showIcon = true,
        theme = null,
        nativeStylesAdd = 0,
        nativeStylesRemove = 0,
        nativeExStylesAdd = 0,
        nativeExStylesRemove = 0
    } = options || {};

    let darlingWindowHandle = null;
    let pollInterval = null;

    // Create the native host window.
    const dw = darling.createWindow(width, height);

    darling.showDarlingWindow(dw);
    darlingWindowHandle = dw; // Store for cleanup

    try {
        darling.setWindowTitle(dw, title);
    } catch (e) {
        console.warn('Failed to set window title:', e);
    }

    try {
        darling.setWindowIconVisible(dw, !!showIcon)
    } catch (e) {
        console.warn('Failed to set window icon visibility:', e);
    }

    if (theme) {
        const titlebarTheme = typeof theme === 'string' ? theme : theme.titlebar;
        
        if (titlebarTheme === 'dark' || titlebarTheme === 'light') {
            try {
                darling.setDarkMode(dw, titlebarTheme === 'dark');
            } catch (e) {
                console.warn('Failed to set titlebar theme:', e);
            }
        }
    }

    // Create the Electron window (embedded as child).
    const bw = new BrowserWindow({
        width: width,
        height: height,
        show: false,
        frame: false,
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true,
        }
    });

    // Embed the Electron window into the native Darling window.
    const buf = bw.getNativeWindowHandle();

    const eleHWND = BigInt.asUintN(
        64,
        buf.readBigUInt64LE(0)
    );

    const darlingHWND = BigInt.asUintN(64, BigInt(darling.getHWND()));

    const WS_CHILD = 0x40000000;
    const WS_POPUP = 0x80000000;
    const WS_OVERLAPPEDWINDOW = 0x00CF0000;
    const SWP_NOZORDER = 0x0004;
    const SWP_FRAMECHANGED = 0x0020;

    try {
        darling.setParent(
            eleHWND,
            darlingHWND
        );

        darling.setWindowStyles(
            eleHWND,
            WS_CHILD,
            WS_POPUP | WS_OVERLAPPEDWINDOW
        );

        darling.setWindowPos(
            eleHWND,
            0,
            0,
            width,
            height,
            SWP_NOZORDER | SWP_FRAMECHANGED
        );

        darling.setChildWindow(dw, eleHWND);
    } catch (e) {
        console.error('Failed to embed Electron window:', e);
    }

    // Apply native window style overrides if provided.
    if (nativeStylesAdd || nativeStylesRemove) {
        try {
            darling.setWindowStyles(
                darlingHWND,
                nativeStylesAdd,
                nativeStylesRemove
            );

            darling.setWindowPos(
                darlingHWND,
                0,
                0,
                width,
                height,
                SWP_NOZORDER | SWP_FRAMECHANGED
            );
        } catch (e) {
            console.warn('Failed to apply native styles:', e);
        }
    }
    if (nativeExStylesAdd || nativeExStylesRemove) {
        try {
            darling.setWindowExStyles(
                darlingHWND,
                nativeExStylesAdd,
                nativeExStylesRemove
            );

            darling.setWindowPos(
                darlingHWND,
                0,
                0,
                width,
                height,
                SWP_NOZORDER | SWP_FRAMECHANGED
            );
        } catch (e) {
            console.warn('Failed to apply native ex-styles:', e);
        }
    }

    // Load content into the Electron window.
    bw.loadURL(url)
        .then(() => bw.show())
        .catch((e) => console.error('Failed to load URL:', e));

    if (theme && typeof theme === 'object' && theme.content) {
        const contentTheme = theme.content;
        if (contentTheme === 'dark' || contentTheme === 'light') {
            bw.webContents.on('did-finish-load', () => {
                const scheme = contentTheme === 'dark' ? 'dark' : 'light';
                bw.webContents.insertCSS(`:root{color-scheme:${scheme};}`);
            });
        }
    }

    /*
        Set up a message loop poller.
        This is crucial to keep the native window responsive and processing
        WM_PAINT and WM_CLOSE messages.
    */

    pollInterval = setInterval(() => {
        try {
            darling.pollEvents();
        } catch (e) {
            console.error('Failed polling events:', e);

            if (pollInterval) clearInterval(pollInterval);
            if (onClose) onClose();
        }
    }, 1000 / frameRate);

    // Set up cleanup logic.
    const cleanup = () => {
        console.log('Cleaning up Darling Electron Window...');

        if (pollInterval) {
            clearInterval(pollInterval);
            pollInterval = null;
        }

        if (darlingWindowHandle) {
            try {
                darling.destroyWindow(darlingWindowHandle);
                darlingWindowHandle = null;
            } catch (e) {
                console.error('Failed to destroy darling window:', e);
            }
        }

        // Destroy the embedded BrowserWindow as well
        if (!bw.isDestroyed()) {
            bw.destroy();
        }
    }

    // Ensure cleanup happens when Electron quits.
    app.on('will-quit', cleanup);

    // Also handle the native window being closed by the user (e.g., Alt+F4).
    darling.onCloseRequested(() => {
        console.log('Darling window close requested.');

        cleanup();
        if (onClose) onClose(); // Use the provided callback
    });

    return bw;
}
