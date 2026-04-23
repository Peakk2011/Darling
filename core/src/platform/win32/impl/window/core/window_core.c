#include "../../internal.h"
#include <stdio.h>

// Global State

DarlingWindow* g_main_window = NULL;
DarlingWindow* g_window_head = NULL;
void (*g_close_callback)(void) = NULL;
DarlingCloseCallbackHWND g_close_callback_hwnd = NULL;

BOOL g_class_registered = FALSE;
CRITICAL_SECTION g_lock;
BOOL g_lock_initialized = FALSE;

static int g_toplevel_count = 0;

static void darling_log(const char* fmt, ...) {
    FILE* f = fopen("C:\\Users\\Public\\darling_debug.log", "a");
    if (!f) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fclose(f);
}

static void darling_handle_size(DarlingWindow* win, HWND hwnd) {
    RECT rc;
    if (!GetClientRect(hwnd, &rc)) {
        darling_log("[WM_SIZE] GetClientRect failed hwnd=%p\n", hwnd);
        return;
    }

    int cw = (int)(rc.right - rc.left);
    int ch = (int)(rc.bottom - rc.top);

    darling_log("[WM_SIZE] hwnd=%p childHwnd=%p cw=%d ch=%d\n",
        hwnd, win ? win->childHwnd : NULL, cw, ch);

    if (win && win->childHwnd && cw > 0 && ch > 0) {
        BOOL ok = SetWindowPos(
            win->childHwnd,
            NULL,
            0, 0, cw, ch,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
        darling_log("[WM_SIZE] SetWindowPos result=%d\n", ok);
        InvalidateRect(win->childHwnd, NULL, FALSE);
    } else {
        darling_log("[WM_SIZE] skipped — win=%p childHwnd=%p cw=%d ch=%d\n",
            win, win ? win->childHwnd : NULL, cw, ch);
    }
}

LRESULT CALLBACK darling_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    DarlingWindow* win = (DarlingWindow*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (msg) {
        case WM_NCCREATE: {
            CREATESTRUCTW* cs = (CREATESTRUCTW*)lp;
            DarlingWindow* init = (DarlingWindow*)cs->lpCreateParams;

            if (init) {
                init->hwnd = hwnd;
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)init);
            }

            darling_log("[WM_NCCREATE] hwnd=%p\n", hwnd);
            return TRUE;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_SETFOCUS:
            if (win && win->childHwnd) {
                SetFocus(win->childHwnd);
            }
            return 0;

        case WM_LBUTTONDOWN:
            if (win && win->childHwnd) {
                SetFocus(win->childHwnd);
            }
            return 0;

        case WM_PAINT:
            darling_handle_paint(win, hwnd);
            return 0;

        case WM_CLOSE:
            darling_log("[WM_CLOSE] hwnd=%p\n", hwnd);
            if (g_close_callback_hwnd) {
                g_close_callback_hwnd((uintptr_t)hwnd);
            }
            if (g_close_callback) {
                g_close_callback();
            }
            DestroyWindow(hwnd);
            return 0;

        case WM_SIZE:
            darling_handle_size(win, hwnd);
            return 0;

        case WM_SETTINGCHANGE: {
            if (lp && lstrcmpW((LPCWSTR)lp, L"ImmersiveColorSet") == 0) {
                if (win && !win->isChild) {
                    BOOL isDark = darling_is_system_dark_mode();
                    darling_apply_dark_mode_internal(win, isDark);
                }
            }
            return 0;
        }

        case WM_DESTROY: {
            BOOL isChild = FALSE;

            if (win) {
                isChild = win->isChild;
            } else {
                LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
                isChild = (style & WS_CHILD) != 0;
            }

            darling_log("[WM_DESTROY] hwnd=%p isChild=%d toplevel_count=%d\n",
                hwnd, isChild, g_toplevel_count);

            if (!isChild) {
                g_toplevel_count--;
                if (g_toplevel_count <= 0) {
                    g_toplevel_count = 0;
                    darling_log("[WM_DESTROY] PostQuitMessage\n");
                    PostQuitMessage(0);
                }
            }

            return 0;
        }

        case WM_NCDESTROY:
            if (win) {
                win->hwnd = NULL;
                win->childHwnd = NULL;

                if (win->inList) {
                    darling_list_remove(win);
                    darling_update_main_on_remove(win);
                }
            }

            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wp, lp);
}

BOOL darling_register_class(void) {
    if (g_class_registered) {
        return TRUE;
    }

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = darling_wnd_proc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = DARLING_WINDOW_CLASS;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(IDC_ARROW));
    wc.hIcon = NULL;
    wc.hIconSm = NULL;
    wc.hbrBackground = NULL;

    if (!RegisterClassExW(&wc)) {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            darling_log_last_error(L"RegisterClassExW");
            return FALSE;
        }
    }

    g_class_registered = TRUE;
    return TRUE;
}