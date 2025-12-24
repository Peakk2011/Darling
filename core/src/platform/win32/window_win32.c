#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "darling.h"

struct DarlingWindow {
    HWND hwnd;
};

static DarlingWindow* g_main_window = NULL;
static void (*g_close_callback)(void) = NULL;

static LRESULT CALLBACK darling_wnd_proc(
    HWND hwnd,
    UINT msg,
    WPARAM wp,
    LPARAM lp
) {
    switch (msg) {
        case WM_CLOSE:
            if (g_close_callback) {
                // Instead of destroying the window, notify the owner process
                // so it can perform a graceful shutdown (e.g. detach child
                // windows before destroying them).
                g_close_callback();
            } else {
                // Fallback for when no callback is set.
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

DarlingWindow* darling_create_window(uint32_t w, uint32_t h) {
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = darling_wnd_proc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"DarlingWindowClass";

    RegisterClassW(&wc);

    // Create a frameless window so that reparenting it (or making other
    // windows children of it) won't produce nested title-bars/frames.
    // Use WS_POPUP|WS_VISIBLE for a top-level frameless window. If you
    // prefer creating the window directly as a child (parent provided at
    // creation time) you can use WS_CHILD|WS_VISIBLE instead.
    DWORD createStyles = WS_POPUP | WS_VISIBLE;

    HWND hwnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        L"Darling",
        createStyles,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        w,
        h,
        NULL,
        NULL,
        wc.hInstance,
        NULL
    );

    DarlingWindow* win = (DarlingWindow*)malloc(sizeof(DarlingWindow));
    win->hwnd = hwnd;
    g_main_window = win;
    return win;
}

void darling_show_window(DarlingWindow* win) {
    ShowWindow(win->hwnd, SW_SHOW);
}

void darling_poll_events(void) {
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void darling_destroy_window(DarlingWindow* win) {
    DestroyWindow(win->hwnd);
    free(win);
}

uintptr_t darling_get_main_hwnd(void) {
    if (!g_main_window || !g_main_window->hwnd) return (uintptr_t)0;
    return (uintptr_t)g_main_window->hwnd;
}

void darling_set_close_callback(void (*callback)()) {
    g_close_callback = callback;
}
