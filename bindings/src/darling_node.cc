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
#include "darling.h"

using namespace Napi;

static ThreadSafeFunction tsfn_on_close;

static void c_callback_on_close() {
    if (tsfn_on_close) {
        tsfn_on_close.BlockingCall();
    }
}

// Function exposed to JS to set the close callback.
Napi::Value SetOnCloseCallback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!info[0].IsFunction()) {
        Napi::TypeError::New(env, "Expected a function for the callback").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    tsfn_on_close = ThreadSafeFunction::New(
        env,
        info[0].As<Function>(),     // JS function to call
        "DarlingOnClose",           // Resource name
        0,                          // Max queue size (0 = unlimited)
        1                           // Initial thread count
    );

    // Register our C-function intermediary.
    darling_set_close_callback(c_callback_on_close);
    return env.Undefined();
}


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

// Allow JS to explicitly destroy the window after cleanup.
void DestroyDarlingWindow(const Napi::CallbackInfo& info) {
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    darling_destroy_window(win);

    // Release the thread-safe function so the process can exit.
    if (tsfn_on_close) {
        tsfn_on_close.Release();
    }
}

Napi::Value ShowWindowWrapped(const Napi::CallbackInfo& info) {
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    darling_show_window(win);
    return info.Env().Undefined();
}

Napi::Value SetChildWindowWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    uint64_t child = value_to_u64(info[1]);
    darling_set_child_hwnd(win, (uintptr_t)child);
    return env.Undefined();
}

Napi::Value SetWindowTitleWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    std::u16string title = info[1].As<Napi::String>().Utf16Value();
    darling_set_window_title(win, (const wchar_t*)title.c_str());
    return env.Undefined();
}

Napi::Value SetWindowIconVisibleWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    bool visible = info[1].As<Napi::Boolean>().Value();
    darling_set_window_icon_visible(win, visible ? 1 : 0);
    return env.Undefined();
}

Napi::Value PollEvents(const Napi::CallbackInfo& info) {
    darling_poll_events();
    return info.Env().Undefined();
}

Napi::Value GetHWND(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    uintptr_t h = darling_get_main_hwnd();
    return Napi::BigInt::New(env, (uint64_t)h);
}

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

Napi::Value IsDarkModeWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int isDark = darling_is_dark_mode();
    return Napi::Boolean::New(env, isDark ? true : false);
}

Napi::Value SetDarkModeWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    bool enable = info[1].As<Napi::Boolean>().Value();
    darling_set_dark_mode(win, enable ? 1 : 0);
    return env.Undefined();
}

Napi::Value SetAutoDarkModeWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    darling_set_auto_dark_mode(win);
    return env.Undefined();
}

Napi::Value SetTitlebarColorsWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto win = info[0].As<Napi::External<DarlingWindow>>().Data();
    uint32_t bg = info[1].As<Napi::Number>().Uint32Value();
    uint32_t text = info[2].As<Napi::Number>().Uint32Value();
    darling_set_titlebar_colors(win, bg, text);
    return env.Undefined();
}

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

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("createWindow", Napi::Function::New(env, CreateDarlingWindow));
    exports.Set("destroyWindow", Napi::Function::New(env, DestroyDarlingWindow));
    exports.Set("onCloseRequested", Napi::Function::New(env, SetOnCloseCallback));
    exports.Set("showDarlingWindow", Napi::Function::New(env, ShowWindowWrapped));
    exports.Set("setChildWindow", Napi::Function::New(env, SetChildWindowWrapped));
    exports.Set("setWindowTitle", Napi::Function::New(env, SetWindowTitleWrapped));
    exports.Set("setWindowIconVisible", Napi::Function::New(env, SetWindowIconVisibleWrapped));
    exports.Set("pollEvents", Napi::Function::New(env, PollEvents));
    exports.Set("getHWND", Napi::Function::New(env, GetHWND));
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
    return exports;
}

NODE_API_MODULE(darling, Init)
