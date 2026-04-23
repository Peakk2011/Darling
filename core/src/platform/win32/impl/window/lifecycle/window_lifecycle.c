#include "../../internal.h"
#include <stdlib.h>

void darling_show_window(DarlingWindow* win) {
    if (win && win->hwnd) {
        ShowWindow(win->hwnd, SW_SHOW);
    }
}

void darling_hide_window(DarlingWindow* win) {
    if (win && win->hwnd) {
        ShowWindow(win->hwnd, SW_HIDE);
    }
}

void darling_focus_window(DarlingWindow* win) {
    if (!win || !win->hwnd) {
        return;
    }

    SetForegroundWindow(win->hwnd);
    SetFocus(win->hwnd);
}

int darling_is_visible(DarlingWindow* win) {
    if (!win || !win->hwnd) {
        return 0;
    }

    return IsWindowVisible(win->hwnd) ? 1 : 0;
}

int darling_is_focused(DarlingWindow* win) {
    if (!win || !win->hwnd) {
        return 0;
    }

    return GetForegroundWindow() == win->hwnd ? 1 : 0;
}

void darling_destroy_window(DarlingWindow* win) {
    if (!win) {
        return;
    }

    BOOL wasMain = FALSE;
    darling_lock();
    
    wasMain = (win == g_main_window);
    darling_unlock();

    HWND hwnd = win->hwnd;
    if (hwnd) {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    }

    if (win->inList) {
        darling_list_remove(win);
    }

    if (wasMain) {
        darling_update_main_on_remove(win);
    }

    darling_cleanup_window_icon(win);
    darling_free_gdi(win);
    free(win);

    if (hwnd) {
        DestroyWindow(hwnd);
    }
}

void darling_set_child_hwnd(DarlingWindow* win, uintptr_t child_hwnd) {
    if (!win) {
        return;
    }

    darling_lock();
    win->childHwnd = (HWND)(uintptr_t)child_hwnd;
    darling_unlock();
}

uintptr_t darling_get_main_hwnd(void) {
    if (!g_main_window || !g_main_window->hwnd) {
        return (uintptr_t)0;
    }

    return (uintptr_t)g_main_window->hwnd;
}

uintptr_t darling_get_window_hwnd(DarlingWindow* win) {
    if (!win || !win->hwnd) {
        return (uintptr_t)0;
    }

    return (uintptr_t)win->hwnd;
}

void darling_set_close_callback(void (*callback)(void)) {
    g_close_callback = callback;
}

void darling_set_close_callback_hwnd(DarlingCloseCallbackHWND callback) {
    g_close_callback_hwnd = callback;
}

void darling_poll_events(void) {
    MSG msg;

    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            PostQuitMessage((int)msg.wParam);
            break;
        }

        // dispatch HWND in Darling window list
        BOOL isDarlingMsg = FALSE;
        darling_lock();
        DarlingWindow* cur = g_window_head;
        while (cur) {
            if (cur->hwnd == msg.hwnd || cur->childHwnd == msg.hwnd) {
                isDarlingMsg = TRUE;
                break;
            }
            cur = cur->next;
        }
        darling_unlock();

        if (isDarlingMsg || msg.hwnd == NULL) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
}

void darling_init(void) {
    darling_ensure_lock();
}

void darling_cleanup(void) {
    if (g_class_registered) {
        UnregisterClassW(DARLING_WINDOW_CLASS, GetModuleHandleW(NULL));
        g_class_registered = FALSE;
    }

    if (g_lock_initialized) {
        DeleteCriticalSection(&g_lock);
        g_lock_initialized = FALSE;
    }
}