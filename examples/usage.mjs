import { CreateWindow } from 'darling';

/**
 * Full CreateWindow example with every prop set.
 * Clear JSDOC so each prop is easy to read.
 *
 * @typedef {Object} ExampleOptions
 * @property {number} width                     - Window width in pixels.
 * @property {number} height                    - Window height in pixels.
 * @property {number} x                         - Window left position (px).
 * @property {number} y                         - Window top position (px).
 * @property {boolean} center                   - Center after show (may override x/y).
 * @property {string} url                       - URL or file to load in the BrowserWindow.
 * @property {string} title                     - Window title (Win32 title bar).
 * @property {boolean} showIcon                 - Show or hide the window icon (affects WS_SYSMENU).
 * @property {number} frameRate                 - Polling rate (times per second).
 * @property {{titlebar?: 'dark'|'light', content?: 'dark'|'light'}|'dark'|'light'|null} theme
 * Theme for titlebar (Win32) and content (CSS color-scheme), or a single string.
 * @property {number} nativeStylesAdd           - GWL_STYLE bits to add on the Darling window.
 * @property {number} nativeStylesRemove        - GWL_STYLE bits to remove on the Darling window.
 * @property {number} nativeExStylesAdd         - GWL_EXSTYLE bits to add on the Darling window.
 * @property {number} nativeExStylesRemove      - GWL_EXSTYLE bits to remove on the Darling window.
 * @property {import('electron').BrowserWindowConstructorOptions['webPreferences']} webPreferences
 * Electron BrowserWindow webPreferences.
 * All known webPreferences keys (Electron):
 * - devTools
 * - nodeIntegration
 * - nodeIntegrationInWorker
 * - nodeIntegrationInSubFrames
 * - preload
 * - sandbox
 * - session
 * - partition
 * - zoomFactor
 * - javascript
 * - webSecurity
 * - allowRunningInsecureContent
 * - images
 * - imageAnimationPolicy
 * - textAreasAreResizable
 * - webgl
 * - plugins
 * - experimentalFeatures
 * - scrollBounce
 * - enableBlinkFeatures
 * - disableBlinkFeatures
 * - defaultFontFamily
 * - defaultFontSize
 * - defaultMonospaceFontSize
 * - minimumFontSize
 * - defaultEncoding
 * - backgroundThrottling
 * - offscreen
 * - contextIsolation
 * - webviewTag
 * - additionalArguments
 * - safeDialogs
 * - safeDialogsMessage
 * - disableDialogs
 * - navigateOnDragDrop
 * - autoplayPolicy
 * - disableHtmlFullscreenWindowResize
 * - accessibleTitle
 * - spellcheck
 * - enableWebSQL
 * - v8CacheOptions
 * @property {(() => void)|null} onClose Called when the window is closed (native or app).
 * @property {(window: import('../js/darling.d.ts').DarlingWindowInstance) => void|null} onReady
 * Called when the window is ready (after load).
 * @property {(error: Error) => void|null} onError Called on errors during create/run.
 * @property {(browserWindow: import('electron').BrowserWindow, instance: import('../js/darling.d.ts').DarlingWindowInstance) => void|null} electron
 * Callback to customize the BrowserWindow after instance creation.
 */

/**
 * Example options with every prop filled in.
 * @type {ExampleOptions}
 */
const options = {
    // Window dimensions
    width: 800,
    height: 600,

    // Window position
    x: 120,
    y: 80,
    center: true,           // Note: This will override x/y after window shows

    // Content
    url: 'https://example.com',
    title: 'Darling Usage Example',

    // Appearance
    showIcon: true,
    frameRate: 60,
    theme: {
        titlebar: 'dark',   // Dark title bar (Win32 DWM)
        content: 'light',   // Light content (CSS color-scheme)
    },

    // Native Win32 styles (advanced usage)
    // See: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles

    nativeStylesAdd: 0,
    nativeStylesRemove: 0,
    nativeExStylesAdd: 0,
    nativeExStylesRemove: 0,

    // Electron BrowserWindow preferences
    webPreferences: {
        nodeIntegration: false,    // Recommended: false for security
        contextIsolation: true,    // Recommended: true for security
        devTools: true,            // Enable DevTools
        sandbox: false,            // Sandbox mode

        // Add more as needed:
        
        // preload: path.join(__dirname, 'preload.js'),
        // webSecurity: true,
        // allowRunningInsecureContent: false,
    },

    // Lifecycle callbacks
    onClose: () => {
        console.log('Window closed');
    },

    onReady: (window) => {
        console.log('Window ready, handle:', window.handle);

        // Darling instance events
        window.on('close', () => {
            console.log('Event: close');
        });

        window.on('closed', () => {
            console.log('Event: closed');
        });

        window.on('ready', () => {
            console.log('Event: ready');
        });

        window.on('resize', (width, height) => {
            console.log('Event: resize', { width, height });
        });

        window.on('move', (x, y) => {
            console.log('Event: move', { x, y });
        });

        window.on('focus', () => {
            console.log('Event: focus');
        });

        window.on('blur', () => {
            console.log('Event: blur');
        });

        // Example: Control window after ready
        setTimeout(() => {
            console.log('Changing title');
            window.setTitle('Title Changed!');
        }, 2000);

        setTimeout(() => {
            console.log('Set opacity to 220');
            window.setWindowOpacity(220);
        }, 3000);

        setTimeout(() => {
            console.log('Always on top: true');
            window.setAlwaysOnTop(true);
        }, 4000);

        setTimeout(() => {
            console.log('Titlebar color + corner preference');
            window.setTitlebarColor(0x202020); // RGB
            window.setCornerPreference(3); // 0=default, 1=none, 2=small, 3=large
        }, 5000);

        setTimeout(() => {
            console.log('Flash window once');
            window.flashWindow(false);
        }, 6000);

        setTimeout(() => {
            console.log('DPI / scale:', window.getDpi(), window.getScaleFactor());
        }, 7000);

        setTimeout(() => {
            console.log('Hide window');
            window.hideWindow();
        }, 8000);

        setTimeout(() => {
            console.log('Show + focus window');
            window.showWindow();
            window.focusWindow();
        }, 9000);

        setTimeout(() => {
            console.log('Visible / focused:', window.isVisible(), window.isFocused());
            window.setAlwaysOnTop(false);
            window.setWindowOpacity(255);
        }, 10000);
    },

    onError: (error) => {
        console.error('Darling error:', error);
    },

    electron: (browserWindow, instance) => {
        console.log('Electron callback - customizing BrowserWindow');
        console.log('Instance available:', !!instance);

        // Customize BrowserWindow
        browserWindow.setAlwaysOnTop(false);
        
        // You can also use the title from Darling instance
        // browserWindow.setTitle('Electron Embedded');

        // More BrowserWindow customization examples:
        // browserWindow.setResizable(true);
        // browserWindow.setMinimumSize(640, 480);
        // browserWindow.setMaximumSize(1920, 1080);
        // browserWindow.setOpacity(0.95);
        // browserWindow.setMenuBarVisibility(false);
        // browserWindow.setBackgroundColor('#1e1e1e');

        // Setup IPC handlers
        const { ipcMain } = require('electron');
        
        ipcMain.on('change-title', (event, title) => {
            instance.setTitle(title);
        });

        ipcMain.on('toggle-visibility', () => {
            if (instance.isVisible()) {
                instance.hideWindow();
                return;
            }
            instance.showWindow();
        });

        // DevTools in development
        if (process.env.NODE_ENV === 'development') {
            browserWindow.webContents.openDevTools();
        }

        // Log when page finishes loading
        browserWindow.webContents.on('did-finish-load', () => {
            console.log('Page finished loading');
        });

        // Handle navigation
        browserWindow.webContents.on('will-navigate', (event, url) => {
            console.log('Navigating to:', url);
        });
    },
};

/**
 * Main function to create the window
 */
const main = async () => {
    try {
        const window = await CreateWindow(options);
        
        // console.log('Window created successfully');
        // console.log('Handle:', window.handle);
        // console.log('Browser window:', window.browserWindow);

        // You can also control the window here
        // window.setTitle('Main Title');
        // window.focus();

    } catch (error) {
        console.error('Failed to create window:', error);
        process.exit(1);
    }
};

// Run main function
main().catch((error) => {
    console.error('Unhandled error:', error);
    process.exit(1);
});
