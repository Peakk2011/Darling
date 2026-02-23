#include "../../internal.h"
#include <stdlib.h>

DarlingWindow* darling_create_window(uint32_t w, uint32_t h, uintptr_t parent_hwnd) {
    darling_ensure_lock();

    if (!darling_register_class()) {
        return NULL;
    }

    DarlingWindow* win = (DarlingWindow*)calloc(1, sizeof(DarlingWindow));

    if (!win) {
        darling_output_debug(L"[Darling][win32] calloc failed in darling_create_window.\n");
        return NULL;
    }

    DWORD createStyles;
    HWND parent = NULL;
    DWORD exStyle = 0;

    int winWidth = (int)w;
    int winHeight = (int)h;

    if (parent_hwnd != (uintptr_t)0) {
        createStyles = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        parent = (HWND)(uintptr_t)parent_hwnd;

        win->isChild = TRUE;
    } else {
        createStyles =
            WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME |
            WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        win->isChild = FALSE;
        exStyle = WS_EX_DLGMODALFRAME;

        RECT rect = {0, 0, (LONG)w, (LONG)h};

        if (AdjustWindowRectEx(&rect, createStyles, FALSE, exStyle)) {
            winWidth = (int)(rect.right - rect.left);
            winHeight = (int)(rect.bottom - rect.top);
        } else {
            darling_log_last_error(L"AdjustWindowRectEx");
        }
    }

    HWND hwnd = CreateWindowExW(
        exStyle,
        DARLING_WINDOW_CLASS,
        L"Darling",
        createStyles,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        winWidth,
        winHeight,
        parent,
        NULL,
        GetModuleHandleW(NULL),
        win
    );

    if (!hwnd) {
        darling_log_create_params(w, h, parent_hwnd, createStyles);
        darling_log_last_error(L"CreateWindowExW");
        free(win);
        return NULL;
    }

    win->hwnd = hwnd;
    darling_list_add(win);

    darling_lock();
    if (!g_main_window || (g_main_window->isChild && !win->isChild)) {
        g_main_window = win;
    }
    darling_unlock();

    if (!win->isChild) {
        BOOL isDark = darling_is_system_dark_mode();
        darling_apply_dark_mode_internal(win, isDark);
    }

    return win;
}