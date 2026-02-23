#include "../../internal.h"

// Global State

DarlingWindow* g_main_window = NULL;
DarlingWindow* g_window_head = NULL;
void (*g_close_callback)(void) = NULL;

BOOL g_class_registered = FALSE;
CRITICAL_SECTION g_lock;
BOOL g_lock_initialized = FALSE;

static void darling_handle_size(DarlingWindow* win, HWND hwnd) {
    if (win && win->childHwnd) {
        RECT rc;

        if (GetClientRect(hwnd, &rc)) {
            int cw = (int)(rc.right - rc.left);
            int ch = (int)(rc.bottom - rc.top);

            SetWindowPos(
                win->childHwnd,
                NULL,
                0,
                0,
                cw,
                ch,
                SWP_NOZORDER | SWP_NOACTIVATE
            );
        }
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

            if (!isChild) {
                PostQuitMessage(0);
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