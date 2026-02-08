import { EventEmitter } from "events";
import { createRequire } from "module";
import * as darling from "./darling-bridge";

const require = createRequire(import.meta.url);
const electron = require("electron");
const { app, BrowserWindow } = electron;

let windowAllClosedHandlerAttached = false;

/**
 * Darling Window Instance
 * Wraps both the native Darling window and Electron BrowserWindow
 */
class DarlingWindowInstance extends EventEmitter {
  darlingWindow: any;
  browserWindow: BrowserWindow;
  options: any;
  closed: boolean;
  _pollInterval: NodeJS.Timeout | null;

  constructor(darlingWindow: any, browserWindow: BrowserWindow, options: any) {
    super();

    this.darlingWindow = darlingWindow;
    this.browserWindow = browserWindow;
    this.options = options;
    this.closed = false;
    this._pollInterval = null;

    this._setupEventForwarding();
  }

  _setupEventForwarding() {
    // Forward BrowserWindow events
    this.browserWindow.on("closed", () => {
      this.closed = true;
      this.emit("closed");
    });

    this.browserWindow.on("resize", () => {
      const [width, height] = this.browserWindow.getSize();
      this.emit("resize", width, height);
    });

    this.browserWindow.on("move", () => {
      const [x, y] = this.browserWindow.getPosition();
      this.emit("move", x, y);
    });

    this.browserWindow.on("focus", () => {
      this.emit("focus");
    });

    this.browserWindow.on("blur", () => {
      this.emit("blur");
    });

    this.browserWindow.webContents.on("did-finish-load", () => {
      this.emit("ready");
    });
  }

  // Window control methods
  close() {
    if (this.closed) return;

    if (this._pollInterval) {
      clearInterval(this._pollInterval);
      this._pollInterval = null;
    }

    try {
      if (this.darlingWindow) {
        darling.destroyWindow(this.darlingWindow);
        this.darlingWindow = null;
      }
    } catch (e) {
      console.error("Failed to destroy darling window:", e);
    }

    if (!this.browserWindow.isDestroyed()) {
      this.browserWindow.destroy();
    }

    this.closed = true;
    this.emit("close");
  }

  focus() {
    if (!this.closed) {
      this.browserWindow.focus();
    }
  }

  blur() {
    if (!this.closed) {
      this.browserWindow.blur();
    }
  }

  resize(width: number, height: number) {
    if (!this.closed) {
      this.browserWindow.setSize(width, height);

      try {
        const darlingHWND = BigInt.asUintN(64, BigInt(darling.getHWND()));
        const SWP_NOZORDER = 0x0004;
        const SWP_FRAMECHANGED = 0x0020;

        darling.setWindowPos(
          darlingHWND,
          0,
          0,
          width,
          height,
          SWP_NOZORDER | SWP_FRAMECHANGED,
        );
      } catch (e) {
        console.error("Failed to resize darling window:", e);
      }
    }
  }

  move(x: number, y: number) {
    if (!this.closed) {
      this.browserWindow.setPosition(x, y);
    }
  }

  center() {
    if (!this.closed) {
      this.browserWindow.center();
    }
  }

  setTitle(title: string) {
    if (!this.closed) {
      try {
        darling.setWindowTitle(this.darlingWindow, title);
      } catch (e) {
        console.error("Failed to set title:", e);
        throw e;
      }
    }
  }

  setDarkMode(enabled: boolean) {
    if (!this.closed) {
      try {
        darling.setDarkMode(this.darlingWindow, !!enabled);
      } catch (e) {
        console.error("Failed to set dark mode:", e);
        throw e;
      }
    }
  }

  showWindow() {
    if (!this.closed) {
      try {
        darling.showDarlingWindow(this.darlingWindow);
      } catch (e) {
        console.error("Failed to show darling window:", e);
        throw e;
      }
    }
  }

  hideWindow() {
    if (!this.closed) {
      try {
        darling.hideDarlingWindow(this.darlingWindow);
      } catch (e) {
        console.error("Failed to hide darling window:", e);
        throw e;
      }
    }
  }

  focusWindow() {
    if (!this.closed) {
      try {
        darling.focusDarlingWindow(this.darlingWindow);
      } catch (e) {
        console.error("Failed to focus darling window:", e);
        throw e;
      }
    }
  }

  isVisible() {
    if (this.closed) return false;
    try {
      return !!darling.isVisible(this.darlingWindow);
    } catch (e) {
      console.error("Failed to read window visibility:", e);
      throw e;
    }
  }

  isFocused() {
    if (this.closed) return false;
    try {
      return !!darling.isFocused(this.darlingWindow);
    } catch (e) {
      console.error("Failed to read window focus state:", e);
      throw e;
    }
  }

  setWindowOpacity(opacity: number) {
    if (!this.closed) {
      try {
        darling.setWindowOpacity(this.darlingWindow, opacity);
      } catch (e) {
        console.error("Failed to set window opacity:", e);
        throw e;
      }
    }
  }

  setAlwaysOnTop(enable: boolean) {
    if (!this.closed) {
      try {
        darling.setAlwaysOnTop(this.darlingWindow, !!enable);
      } catch (e) {
        console.error("Failed to set always-on-top:", e);
        throw e;
      }
    }
  }

  setTitlebarColor(color: number) {
    if (!this.closed) {
      try {
        darling.setTitlebarColor(this.darlingWindow, color);
      } catch (e) {
        console.error("Failed to set titlebar color:", e);
        throw e;
      }
    }
  }

  setCornerPreference(preference: number) {
    if (!this.closed) {
      try {
        darling.setCornerPreference(this.darlingWindow, preference);
      } catch (e) {
        console.error("Failed to set corner preference:", e);
        throw e;
      }
    }
  }

  flashWindow(continuous = false) {
    if (!this.closed) {
      try {
        darling.flashWindow(this.darlingWindow, !!continuous);
      } catch (e) {
        console.error("Failed to flash window:", e);
        throw e;
      }
    }
  }

  getDpi() {
    if (this.closed) return 96;
    try {
      return darling.getDpi(this.darlingWindow);
    } catch (e) {
      console.error("Failed to get DPI:", e);
      throw e;
    }
  }

  getScaleFactor() {
    if (this.closed) return 1;
    try {
      return darling.getScaleFactor(this.darlingWindow);
    } catch (e) {
      console.error("Failed to get scale factor:", e);
      throw e;
    }
  }

  minimize() {
    if (!this.closed) {
      this.browserWindow.minimize();
    }
  }

  maximize() {
    if (!this.closed) {
      this.browserWindow.maximize();
    }
  }

  restore() {
    if (!this.closed) {
      this.browserWindow.restore();
    }
  }

  // Getters
  get handle() {
    return this.darlingWindow;
  }

  get isDestroyed() {
    return this.closed;
  }

  get webContents() {
    return this.browserWindow.webContents;
  }
}

/**
 * Create a Darling window with embedded Electron BrowserWindow
 */
export const CreateWindow = async (options: any = {}) => {
  // Setup app-level handlers
  if (!windowAllClosedHandlerAttached) {
    windowAllClosedHandlerAttached = true;
    app.on("window-all-closed", () => {
      app.quit();
    });
  }

  if (!app.isReady()) {
    await app.whenReady();
  }

  // Extract and validate options
  const {
    width = 800,
    height = 600,
    x = undefined,
    y = undefined,
    center = false,
    url = "about:blank",
    title = "",
    showIcon = true,
    frameRate = 60,
    theme = null,

    // Native styles
    nativeStylesAdd = 0,
    nativeStylesRemove = 0,
    nativeExStylesAdd = 0,
    nativeExStylesRemove = 0,

    // Electron options
    webPreferences = {
      nodeIntegration: false,
      contextIsolation: true,
    },

    // Callbacks
    onClose = null,
    onReady = null,
    onError = null,
    electron = null,
  } = options;

  let darlingWindowHandle: any = null;
  let browserWindow: BrowserWindow | null = null;
  let instance: DarlingWindowInstance | null = null;

  try {
    // Create the native host window
    darlingWindowHandle = darling.createWindow(width, height);
    darling.showDarlingWindow(darlingWindowHandle);

    // Set window title
    if (title) {
      try {
        darling.setWindowTitle(darlingWindowHandle, title);
      } catch (e) {
        console.warn("Failed to set window title:", e);
      }
    }

    // Set icon visibility
    try {
      darling.setWindowIconVisible(darlingWindowHandle, !!showIcon);
    } catch (e) {
      console.warn("Failed to set window icon visibility:", e);
    }

    // Apply theme
    if (theme) {
      const titlebarTheme = typeof theme === "string" ? theme : theme.titlebar;

      if (titlebarTheme === "dark" || titlebarTheme === "light") {
        try {
          darling.setDarkMode(darlingWindowHandle, titlebarTheme === "dark");
        } catch (e) {
          console.warn("Failed to set titlebar theme:", e);
        }
      }
    }

    // Create the Electron BrowserWindow
    browserWindow = new BrowserWindow({
      width,
      height,
      x,
      y,
      show: false,
      frame: false,
      webPreferences,
    });

    // Embed the Electron window into the native Darling window
    const buf = browserWindow.getNativeWindowHandle();
    const eleHWND = BigInt.asUintN(64, buf.readBigUInt64LE(0));
    const darlingHWND = BigInt.asUintN(64, BigInt(darling.getHWND()));

    const WS_CHILD = 0x40000000;
    const WS_POPUP = 0x80000000;
    const WS_OVERLAPPEDWINDOW = 0x00cf0000;
    const SWP_NOZORDER = 0x0004;
    const SWP_FRAMECHANGED = 0x0020;

    darling.setParent(eleHWND, darlingHWND);
    darling.setWindowStyles(eleHWND, WS_CHILD, WS_POPUP | WS_OVERLAPPEDWINDOW);
    darling.setWindowPos(
      eleHWND,
      0,
      0,
      width,
      height,
      SWP_NOZORDER | SWP_FRAMECHANGED,
    );
    darling.setChildWindow(darlingWindowHandle, eleHWND);

    // Apply native window style overrides
    if (nativeStylesAdd || nativeStylesRemove) {
      darling.setWindowStyles(darlingHWND, nativeStylesAdd, nativeStylesRemove);
      darling.setWindowPos(
        darlingHWND,
        0,
        0,
        width,
        height,
        SWP_NOZORDER | SWP_FRAMECHANGED,
      );
    }

    if (nativeExStylesAdd || nativeExStylesRemove) {
      darling.setWindowExStyles(
        darlingHWND,
        nativeExStylesAdd,
        nativeExStylesRemove,
      );
      darling.setWindowPos(
        darlingHWND,
        0,
        0,
        width,
        height,
        SWP_NOZORDER | SWP_FRAMECHANGED,
      );
    }

    // Create window instance
    instance = new DarlingWindowInstance(
      darlingWindowHandle,
      browserWindow,
      options,
    );

    // Call electron callback if provided
    if (electron && typeof electron === "function") {
      try {
        electron(browserWindow, instance);
      } catch (e) {
        console.error("Error in electron callback:", e);
        if (onError) onError(e);
      }
    }

    // Apply content theme
    if (theme && typeof theme === "object" && theme.content) {
      const contentTheme = theme.content;
      if (contentTheme === "dark" || contentTheme === "light") {
        browserWindow.webContents.on("did-finish-load", () => {
          const scheme = contentTheme === "dark" ? "dark" : "light";
          browserWindow.webContents.insertCSS(`:root{color-scheme:${scheme};}`);
        });
      }
    }

    // Load URL
    await browserWindow.loadURL(url);
    browserWindow.show();

    // Center if requested
    if (center) {
      browserWindow.center();
    }

    // Call onReady callback
    if (onReady && typeof onReady === "function") {
      try {
        onReady(instance);
      } catch (e) {
        console.error("Error in onReady callback:", e);
        if (onError) onError(e);
      }
    }

    // Set up message loop poller
    instance._pollInterval = setInterval(() => {
      try {
        darling.pollEvents();
      } catch (e) {
        console.error("Failed polling events:", e);
        instance?.close();
        if (onError) onError(e);
      }
    }, 1000 / frameRate);

    // Handle native window close
    darling.onCloseRequested(() => {
      console.log("Darling window close requested.");
      instance?.close();
      if (onClose) onClose();
    });

    // Handle app quit
    const cleanupHandler = () => {
      if (!instance?.closed) {
        instance?.close();
      }
    };

    app.on("will-quit", cleanupHandler);

    // Handle window close event
    instance.on("close", () => {
      app.removeListener("will-quit", cleanupHandler);
      if (onClose) onClose();
    });

    return instance;
  } catch (error) {
    console.error("Failed to create Darling window:", error);

    // Cleanup on error
    if (darlingWindowHandle) {
      try {
        darling.destroyWindow(darlingWindowHandle);
      } catch (e) {
        console.error("Failed to cleanup darling window:", e);
      }
    }

    if (browserWindow && !browserWindow.isDestroyed()) {
      browserWindow.destroy();
    }

    if (onError) onError(error as Error);
    throw error;
  }
};

/**
 * Get the main Darling window (if any)
 */
export const GetMainWindow = () => {
  return null;
};

export default CreateWindow;
