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

// Thread-safe function to call from the message loop thread back into Node's event loop.
static ThreadSafeFunction tsfn_on_close;

// This is the C-style function that will be passed to the darling library.
// It gets called on the message loop thread.
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
        info[0].As<Function>(), // JS function to call
        "DarlingOnClose",      // Resource name
        0,                     // Max queue size (0 = unlimited)
        1                      // Initial thread count
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

    DarlingWindow* win = darling_create_window(w, h);
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

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("createWindow", Napi::Function::New(env, CreateDarlingWindow));
    exports.Set("destroyWindow", Napi::Function::New(env, DestroyDarlingWindow));
    exports.Set("onCloseRequested", Napi::Function::New(env, SetOnCloseCallback));
    exports.Set("showDarlingWindow", Napi::Function::New(env, ShowWindowWrapped));
    exports.Set("pollEvents", Napi::Function::New(env, PollEvents));
    exports.Set("getHWND", Napi::Function::New(env, GetHWND));
    exports.Set("setParent", Napi::Function::New(env, SetParentWrapped));
    exports.Set("setWindowStyles", Napi::Function::New(env, SetWindowStylesWrapped));
    exports.Set("setWindowPos", Napi::Function::New(env, SetWindowPosWrapped));
    exports.Set("showWindow", Napi::Function::New(env, ShowWindowWrappedHWND));
    return exports;
}

NODE_API_MODULE(darling, Init)
