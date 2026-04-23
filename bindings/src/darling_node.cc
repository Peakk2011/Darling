#include <napi.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include <mutex>
#include <unordered_map>
#include "darling.h"

using namespace Napi;

static ThreadSafeFunction tsfn_on_close;
static std::unordered_map<uint64_t, ThreadSafeFunction> tsfn_on_close_by_hwnd;
static std::mutex g_close_callbacks_mutex;
static bool g_close_hook_registered = false;

static void c_callback_on_close() {
    if (tsfn_on_close) {
        tsfn_on_close.BlockingCall();
    }
}

// C-side close callback trampoline.
static void c_callback_on_close_hwnd(uintptr_t hwnd) {
    ThreadSafeFunction hwnd_tsfn;

    {
        std::lock_guard<std::mutex> lock(g_close_callbacks_mutex);
        auto it = tsfn_on_close_by_hwnd.find((uint64_t)hwnd);
        if (it != tsfn_on_close_by_hwnd.end()) {
            hwnd_tsfn = it->second;
        }
    }

    if (hwnd_tsfn) {
        hwnd_tsfn.BlockingCall();
    }
}

static void ensure_close_hook_registered() {
    if (!g_close_hook_registered) {
        darling_set_close_callback_hwnd(c_callback_on_close_hwnd);
        g_close_hook_registered = true;
    }
}

// Bind a JS close callback through a ThreadSafeFunction.
Napi::Value SetOnCloseCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!info[0].IsFunction()) {
        Napi::TypeError::New(env, "Expected a function for the callback").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    if (tsfn_on_close) {
        tsfn_on_close.Release();
    }

    tsfn_on_close = ThreadSafeFunction::New(
        env,
        info[0].As<Function>(),     // JS function to call
        "DarlingOnClose",           // Resource name
        0,                          // Max queue size (0 = unlimited)
        1                           // Initial thread count
    );

    // Register legacy callback and ensure HWND callback is also active.
    darling_set_close_callback(c_callback_on_close);
    ensure_close_hook_registered();
    return env.Undefined();
}


// Convert JS Number/BigInt to uint64.
static uint64_t value_to_u64(const Napi::Value& v) {
    if (v.IsBigInt()) {
        bool lossless = false;
        return v.As<Napi::BigInt>().Uint64Value(&lossless);
    }
    if (v.IsNumber()) {
        return (uint64_t)v.As<Napi::Number>().Int64Value();
    }
    return 0;
}

// Create a native Darling window.
Napi::Value CreateDarlingWindow(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    uint32_t w = info[0].As<Napi::Number>().Uint32Value();
    uint32_t h = info[1].As<Napi::Number>().Uint32Value();
    uintptr_t parent_hwnd = 0;
    if (info.Length() >= 3) {
        parent_hwnd = (uintptr_t)value_to_u64(info[2]);
    }

    DarlingWindow* win = darling_create_window(w, h, parent_hwnd);
    return Napi::External<DarlingWindow>::New(env, win);
}

Napi::Value GetWindowHWND(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsExternal()) {
        Napi::TypeError::New(env, "Expected a Darling window handle").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    uintptr_t hwnd = darling_get_window_hwnd(win);
    return Napi::BigInt::New(env, (uint64_t)hwnd);
}

Napi::Value SetOnCloseCallbackForWindow(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsExternal()) {
        Napi::TypeError::New(env, "Expected window handle and callback").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    if (!info[1].IsFunction()) {
        Napi::TypeError::New(env, "Expected a function for the callback").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    uint64_t hwnd = (uint64_t)darling_get_window_hwnd(win);
    if (hwnd == 0) {
        Napi::TypeError::New(env, "Invalid window handle").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    ThreadSafeFunction tsfn = ThreadSafeFunction::New(
        env,
        info[1].As<Function>(),
        "DarlingOnCloseByWindow",
        0,
        1
    );

    {
        std::lock_guard<std::mutex> lock(g_close_callbacks_mutex);
        auto it = tsfn_on_close_by_hwnd.find(hwnd);
        if (it != tsfn_on_close_by_hwnd.end() && it->second) {
            it->second.Release();
        }
        tsfn_on_close_by_hwnd[hwnd] = tsfn;
    }

    ensure_close_hook_registered();
    return env.Undefined();
}

// Destroy the window and release resources.
void DestroyDarlingWindow(const Napi::CallbackInfo& info) {
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    uint64_t hwnd = (uint64_t)darling_get_window_hwnd(win);
    darling_destroy_window(win);

    if (hwnd != 0) {
        std::lock_guard<std::mutex> lock(g_close_callbacks_mutex);
        auto it = tsfn_on_close_by_hwnd.find(hwnd);
        if (it != tsfn_on_close_by_hwnd.end()) {
            if (it->second) {
                it->second.Release();
            }
            tsfn_on_close_by_hwnd.erase(it);
        }
    }

}

// Show a Darling window.
Napi::Value ShowWindowWrapped(const Napi::CallbackInfo& info) {
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    darling_show_window(win);
    return info.Env().Undefined();
}

// Hide a Darling window.
Napi::Value HideWindowWrapped(const Napi::CallbackInfo& info) {
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    darling_hide_window(win);
    return info.Env().Undefined();
}

// Focus a Darling window.
Napi::Value FocusWindowWrapped(const Napi::CallbackInfo& info) {
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    darling_focus_window(win);
    return info.Env().Undefined();
}

// Check if a Darling window is visible.
Napi::Value IsVisibleWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    int visible = darling_is_visible(win);
    return Napi::Boolean::New(env, visible ? true : false);
}

// Check if a Darling window is focused.
Napi::Value IsFocusedWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    int focused = darling_is_focused(win);
    return Napi::Boolean::New(env, focused ? true : false);
}

// Set child HWND used for resize parenting.
Napi::Value SetChildWindowWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    uint64_t child = value_to_u64(info[1]);
    darling_set_child_hwnd(win, (uintptr_t)child);
    return env.Undefined();
}

// Set the Win32 window title.
Napi::Value SetWindowTitleWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    std::u16string title = info[1].As<Napi::String>().Utf16Value();
    darling_set_window_title(win, (const wchar_t*)title.c_str());
    return env.Undefined();
}

// Show or hide the titlebar icon.
Napi::Value SetWindowIconVisibleWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    bool visible = info[1].As<Napi::Boolean>().Value();
    darling_set_window_icon_visible(win, visible ? 1 : 0);
    return env.Undefined();
}

// Set window opacity (0-255).
Napi::Value SetWindowOpacityWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    uint32_t opacity = info[1].As<Napi::Number>().Uint32Value();
    if (opacity > 255) {
        opacity = 255;
    }
    darling_set_window_opacity(win, (uint8_t)opacity);
    return env.Undefined();
}

// Toggle always-on-top for the window.
Napi::Value SetAlwaysOnTopWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    bool enable = info[1].As<Napi::Boolean>().Value();
    darling_set_always_on_top(win, enable ? 1 : 0);
    return env.Undefined();
}

// Process pending Win32 messages.
Napi::Value PollEvents(const Napi::CallbackInfo& info) {
    darling_poll_events();
    return info.Env().Undefined();
}

// Get HWND of the main Darling window.
Napi::Value GetHWND(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    uintptr_t h = darling_get_main_hwnd();
    return Napi::BigInt::New(env, (uint64_t)h);
}

// Set a raw HWND parent (Win32 SetParent).
Napi::Value SetParentWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    uint64_t child = value_to_u64(info[0]);
    uint64_t parent = value_to_u64(info[1]);
#ifdef _WIN32
    HWND res = SetParent((HWND)(uintptr_t)child, (HWND)(uintptr_t)parent);
    return Napi::Boolean::New(env, res != NULL);
#else
    return Napi::Boolean::New(env, false);
#endif
}

// Update GWL_STYLE bits on an HWND.
Napi::Value SetWindowStylesWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    uint64_t hwnd_v = value_to_u64(info[0]);
    uint64_t add = value_to_u64(info[1]);
    uint64_t remove = value_to_u64(info[2]);
#ifdef _WIN32
    HWND hwnd = (HWND)(uintptr_t)hwnd_v;
    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    style = (style | (LONG_PTR)add) & ~((LONG_PTR)remove);
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);
    return Napi::Boolean::New(env, true);
#else
    return Napi::Boolean::New(env, false);
#endif
}

// Update GWL_EXSTYLE bits on an HWND.
Napi::Value SetWindowExStylesWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    uint64_t hwnd_v = value_to_u64(info[0]);
    uint64_t add = value_to_u64(info[1]);
    uint64_t remove = value_to_u64(info[2]);
#ifdef _WIN32
    HWND hwnd = (HWND)(uintptr_t)hwnd_v;
    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    
    style = (style | (LONG_PTR)add) & ~((LONG_PTR)remove);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, style);
    return Napi::Boolean::New(env, true);
#else
    return Napi::Boolean::New(env, false);
#endif
}

// Query system dark mode.
Napi::Value IsDarkModeWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int isDark = darling_is_dark_mode();
    return Napi::Boolean::New(env, isDark ? true : false);
}

// Set dark mode on a Darling window.
Napi::Value SetDarkModeWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    bool enable = info[1].As<Napi::Boolean>().Value();
    darling_set_dark_mode(win, enable ? 1 : 0);
    return env.Undefined();
}

// Apply system theme to a Darling window.
Napi::Value SetAutoDarkModeWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    darling_set_auto_dark_mode(win);
    return env.Undefined();
}

// Set titlebar background and text colors.
Napi::Value SetTitlebarColorsWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    uint32_t bg = info[1].As<Napi::Number>().Uint32Value();
    uint32_t text = info[2].As<Napi::Number>().Uint32Value();
    darling_set_titlebar_colors(win, bg, text);
    return env.Undefined();
}

// Set titlebar background color only.
Napi::Value SetTitlebarColorWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    uint32_t color = info[1].As<Napi::Number>().Uint32Value();
    darling_set_titlebar_color(win, color);
    return env.Undefined();
}

// Set rounded corner preference (Win11+).
Napi::Value SetCornerPreferenceWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    int pref = info[1].As<Napi::Number>().Int32Value();
    darling_set_corner_preference(win, (DarlingCornerPreference)pref);
    return env.Undefined();
}

// Flash the window/taskbar.
Napi::Value FlashWindowWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    bool continuous = info[1].As<Napi::Boolean>().Value();
    darling_flash_window(win, continuous ? 1 : 0);
    return env.Undefined();
}

// Get window DPI (fallback to 96 if unsupported).
Napi::Value GetDpiWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    uint32_t dpi = darling_get_dpi(win);
    return Napi::Number::New(env, dpi);
}

// Get DPI scale factor (dpi / 96).
Napi::Value GetScaleFactorWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    float scale = darling_get_scale_factor(win);
    return Napi::Number::New(env, scale);
}

// Call SetWindowPos on a raw HWND.
Napi::Value SetWindowPosWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    uint64_t hwnd_v = value_to_u64(info[0]);
    int x = info[1].As<Napi::Number>().Int32Value();
    int y = info[2].As<Napi::Number>().Int32Value();
    int w = info[3].As<Napi::Number>().Int32Value();
    int h = info[4].As<Napi::Number>().Int32Value();
    uint32_t flags = (uint32_t)value_to_u64(info[5]);
#ifdef _WIN32
    BOOL ok = SetWindowPos((HWND)(uintptr_t)hwnd_v, NULL, x, y, w, h, flags);
    return Napi::Boolean::New(env, ok != FALSE);
#else
    return Napi::Boolean::New(env, false);
#endif
}

// Call ShowWindow on a raw HWND.
Napi::Value ShowWindowWrappedHWND(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    uint64_t hwnd_v = value_to_u64(info[0]);
    int cmd = info[1].As<Napi::Number>().Int32Value();
#ifdef _WIN32
    BOOL ok = ShowWindow((HWND)(uintptr_t)hwnd_v, cmd);
    return Napi::Boolean::New(env, ok != FALSE);
#else
    return Napi::Boolean::New(env, false);
#endif
}

// Paint a BGRA buffer to the main Darling window.
Napi::Value PaintFrameWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Expected a Buffer for the frame data").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    auto buffer = info[0].As<Napi::Buffer<unsigned char>>();
    uint32_t w = info[1].As<Napi::Number>().Uint32Value();
    uint32_t h = info[2].As<Napi::Number>().Uint32Value();

    darling_paint_frame(buffer.Data(), w, h);

    return env.Undefined();
}

// Export all native bindings.
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("createWindow", Napi::Function::New(env, CreateDarlingWindow));
    exports.Set("destroyWindow", Napi::Function::New(env, DestroyDarlingWindow));
    exports.Set("onCloseRequested", Napi::Function::New(env, SetOnCloseCallback));
    exports.Set("onCloseRequestedForWindow", Napi::Function::New(env, SetOnCloseCallbackForWindow));
    exports.Set("showDarlingWindow", Napi::Function::New(env, ShowWindowWrapped));
    exports.Set("hideDarlingWindow", Napi::Function::New(env, HideWindowWrapped));
    exports.Set("focusDarlingWindow", Napi::Function::New(env, FocusWindowWrapped));
    exports.Set("isVisible", Napi::Function::New(env, IsVisibleWrapped));
    exports.Set("isFocused", Napi::Function::New(env, IsFocusedWrapped));
    exports.Set("setChildWindow", Napi::Function::New(env, SetChildWindowWrapped));
    exports.Set("setWindowTitle", Napi::Function::New(env, SetWindowTitleWrapped));
    exports.Set("setWindowIconVisible", Napi::Function::New(env, SetWindowIconVisibleWrapped));
    exports.Set("setWindowOpacity", Napi::Function::New(env, SetWindowOpacityWrapped));
    exports.Set("setAlwaysOnTop", Napi::Function::New(env, SetAlwaysOnTopWrapped));
    exports.Set("pollEvents", Napi::Function::New(env, PollEvents));
    exports.Set("getHWND", Napi::Function::New(env, GetHWND));
    exports.Set("getWindowHWND", Napi::Function::New(env, GetWindowHWND));
    exports.Set("paintFrame", Napi::Function::New(env, PaintFrameWrapped));
    exports.Set("setParent", Napi::Function::New(env, SetParentWrapped));
    exports.Set("setWindowStyles", Napi::Function::New(env, SetWindowStylesWrapped));
    exports.Set("setWindowExStyles", Napi::Function::New(env, SetWindowExStylesWrapped));
    exports.Set("setWindowPos", Napi::Function::New(env, SetWindowPosWrapped));
    exports.Set("showWindow", Napi::Function::New(env, ShowWindowWrappedHWND));
    exports.Set("isDarkMode", Napi::Function::New(env, IsDarkModeWrapped));
    exports.Set("setDarkMode", Napi::Function::New(env, SetDarkModeWrapped));
    exports.Set("setAutoDarkMode", Napi::Function::New(env, SetAutoDarkModeWrapped));
    exports.Set("setTitlebarColors", Napi::Function::New(env, SetTitlebarColorsWrapped));
    exports.Set("setTitlebarColor", Napi::Function::New(env, SetTitlebarColorWrapped));
    exports.Set("setCornerPreference", Napi::Function::New(env, SetCornerPreferenceWrapped));
    exports.Set("flashWindow", Napi::Function::New(env, FlashWindowWrapped));
    exports.Set("getDpi", Napi::Function::New(env, GetDpiWrapped));
    exports.Set("getScaleFactor", Napi::Function::New(env, GetScaleFactorWrapped));
    return exports;
}

NODE_API_MODULE(darling, Init)