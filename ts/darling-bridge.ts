import { createRequire } from "module";

const require = createRequire(import.meta.url);
let native: any;

try {
  native = require("../bindings/build/Release/darling.node");
} catch (err) {
  try {
    native = require("../bindings/native_stub.js");
  } catch (e) {
    native = {
      getHWND: () => {
        throw new Error("Darling native addon not loaded");
      },
      setParent: () => {
        throw new Error("Darling native addon not loaded");
      },
      setWindowStyles: () => {
        throw new Error("Darling native addon not loaded");
      },
      setWindowPos: () => {
        throw new Error("Darling native addon not loaded");
      },
      showWindow: () => {
        throw new Error("Darling native addon not loaded");
      },
      showDarlingWindow: () => {
        throw new Error("Darling native addon not loaded");
      },
      hideDarlingWindow: () => {
        throw new Error("Darling native addon not loaded");
      },
      focusDarlingWindow: () => {
        throw new Error("Darling native addon not loaded");
      },
      isVisible: () => {
        throw new Error("Darling native addon not loaded");
      },
      isFocused: () => {
        throw new Error("Darling native addon not loaded");
      },
      setWindowOpacity: () => {
        throw new Error("Darling native addon not loaded");
      },
      setAlwaysOnTop: () => {
        throw new Error("Darling native addon not loaded");
      },
      setTitlebarColor: () => {
        throw new Error("Darling native addon not loaded");
      },
      setCornerPreference: () => {
        throw new Error("Darling native addon not loaded");
      },
      flashWindow: () => {
        throw new Error("Darling native addon not loaded");
      },
      getDpi: () => {
        throw new Error("Darling native addon not loaded");
      },
      getScaleFactor: () => {
        throw new Error("Darling native addon not loaded");
      },
    };
  }
}

export const createWindow = (...args: any[]) => native.createWindow(...args);
export const destroyWindow = (win: any) => native.destroyWindow(win);
export const onCloseRequested = (cb: () => void) => native.onCloseRequested(cb);
export const showDarlingWindow = (win: any) => native.showDarlingWindow(win);
export const hideDarlingWindow = (win: any) => native.hideDarlingWindow(win);
export const focusDarlingWindow = (win: any) => native.focusDarlingWindow(win);
export const setChildWindow = (win: any, childHwnd: any) =>
  native.setChildWindow(win, childHwnd);
export const setWindowTitle = (win: any, title: string) =>
  native.setWindowTitle(win, title);
export const setWindowIconVisible = (win: any, visible: boolean) =>
  native.setWindowIconVisible(win, visible);
export const setWindowOpacity = (win: any, opacity: number) =>
  native.setWindowOpacity(win, opacity);
export const setAlwaysOnTop = (win: any, enable: boolean) =>
  native.setAlwaysOnTop(win, enable);
export const pollEvents = () => native.pollEvents();
export const getHWND = () => native.getHWND();
export const paintFrame = (buffer: Buffer, w: number, h: number) =>
  native.paintFrame(buffer, w, h);
export const setParent = (child: any, parent: any) =>
  native.setParent(child, parent);
export const setWindowStyles = (hwnd: any, add: number, remove: number) =>
  native.setWindowStyles(hwnd, add, remove);
export const setWindowExStyles = (hwnd: any, add: number, remove: number) =>
  native.setWindowExStyles(hwnd, add, remove);
export const setWindowPos = (
  hwnd: any,
  x: number,
  y: number,
  w: number,
  h: number,
  flags: number,
) => native.setWindowPos(hwnd, x, y, w, h, flags);
export const showWindow = (hwnd: any, cmd: number) =>
  native.showWindow(hwnd, cmd);
export const isVisible = (win: any) => native.isVisible(win);
export const isFocused = (win: any) => native.isFocused(win);
export const isDarkMode = () => native.isDarkMode();
export const setDarkMode = (win: any, enable: boolean) =>
  native.setDarkMode(win, enable);
export const setAutoDarkMode = (win: any) => native.setAutoDarkMode(win);
export const setTitlebarColors = (win: any, bg: number, text: number) =>
  native.setTitlebarColors(win, bg, text);
export const setTitlebarColor = (win: any, color: number) =>
  native.setTitlebarColor(win, color);
export const setCornerPreference = (win: any, pref: number) =>
  native.setCornerPreference(win, pref);
export const flashWindow = (win: any, continuous: boolean) =>
  native.flashWindow(win, continuous);
export const getDpi = (win: any) => native.getDpi(win);
export const getScaleFactor = (win: any) => native.getScaleFactor(win);
