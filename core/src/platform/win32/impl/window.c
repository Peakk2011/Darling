#include "internal.h"
#include "darling.h"
#include <stdlib.h>

// Global State

DarlingWindow* g_main_window = NULL;
DarlingWindow* g_window_head = NULL;
void (*g_close_callback)(void) = NULL;

BOOL g_class_registered = FALSE;
CRITICAL_SECTION g_lock;
BOOL g_lock_initialized = FALSE;

// Forward Declarations

void darling_handle_paint(DarlingWindow* win, HWND hwnd);

// Theme Application

void darling_apply_dark_mode_internal(DarlingWindow* win, BOOL enable) {
    if (!win || !win->hwnd) {
        return;
    }
    
    // Set immersive dark mode for title bar
    BOOL value = enable;
    DwmSetWindowAttribute(
        win->hwnd,
        DWMWA_USE_IMMERSIVE_DARK_MODE,
        &value,
        sizeof(value)
    );
    
    // Store the current mode
    win->darkMode = enable;
    
    // Force redraw
    SetWindowPos(
        win->hwnd,
        NULL,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
    );
    
    RedrawWindow(win->hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
}

// Window Resizing

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

// Window Procedure

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
            // Detect system theme change
            if (lp && lstrcmpW((LPCWSTR)lp, L"ImmersiveColorSet") == 0) {
                if (win && !win->isChild) {
                    // Auto-update to match system theme
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

// Window Class Registration

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

// Public API - Window Creation

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

    // Determine window style and parent
    DWORD createStyles;
    HWND parent = NULL;
    DWORD exStyle = 0;
    
    int winWidth = (int)w;
    int winHeight = (int)h;
    
    if (parent_hwnd != (uintptr_t)0) {
        // Child window
        createStyles = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        parent = (HWND)(uintptr_t)parent_hwnd;
        
        win->isChild = TRUE;
    } else {
        // Top-level window
        createStyles = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        win->isChild = FALSE;
        
        // Adjust for window decorations
        RECT rect = {0, 0, (LONG)w, (LONG)h};

        if (AdjustWindowRectEx(&rect, createStyles, FALSE, exStyle)) {
            winWidth = (int)(rect.right - rect.left);
            winHeight = (int)(rect.bottom - rect.top);
        } else {
            darling_log_last_error(L"AdjustWindowRectEx");
        }
    }

    // Create the window
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

    // Update main window if needed
    darling_lock();
    if (!g_main_window || (g_main_window->isChild && !win->isChild)) {
        g_main_window = win;
    }
    darling_unlock();

    // Auto-apply system dark mode for top-level windows
    if (!win->isChild) {
        BOOL isDark = darling_is_system_dark_mode();
        darling_apply_dark_mode_internal(win, isDark);
    }

    return win;
}

// Public API - Window Management

void darling_show_window(DarlingWindow* win) {
    if (win && win->hwnd) {
        ShowWindow(win->hwnd, SW_SHOW);
    }
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

// Public API - Window Properties

void darling_set_window_title(DarlingWindow* win, const wchar_t* title) {
    if (!win || !win->hwnd) {
        return;
    }
    
    SetWindowTextW(win->hwnd, title ? title : L"");
}

void darling_set_window_icon_visible(DarlingWindow* win, int visible) {
    if (!win || !win->hwnd) {
        return;
    }
    
    darling_lock();
    
    // ทำลาย icon เก่า
    if (win->customIcon) {
        DestroyIcon(win->customIcon);
        win->customIcon = NULL;
    }
    
    HICON icon = NULL;
    
    if (visible) {
        // System icon
        icon = (HICON)LoadImageW(
            NULL,
            MAKEINTRESOURCEW(IDI_APPLICATION),
            IMAGE_ICON,
            0, 0,
            LR_DEFAULTSIZE | LR_SHARED
        );
    } else {
        // Transparent icon 1x1
        BYTE andMask[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        BYTE xorMask[4] = {0x00, 0x00, 0x00, 0x00};
        
        icon = CreateIcon(
            GetModuleHandleW(NULL),
            1, 1, 1, 1,
            andMask,
            xorMask
        );
        
        if (icon) {
            win->customIcon = icon;
        } else {
            darling_log_last_error(L"CreateIcon");
        }
    }
    
    if (icon) {
        SendMessageW(win->hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
        SendMessageW(win->hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
        
        SetWindowPos(
            win->hwnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | 
            SWP_NOACTIVATE | SWP_FRAMECHANGED
        );
    }
    
    darling_unlock();
}

void darling_cleanup_window_icon(DarlingWindow* win) {
    if (win && win->customIcon) {
        DestroyIcon(win->customIcon);
        win->customIcon = NULL;
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

void darling_set_close_callback(void (*callback)(void)) {
    g_close_callback = callback;
}

// Public API - Theme Management

int darling_is_dark_mode(void) {
    return darling_is_system_dark_mode() ? 1 : 0;
}

void darling_set_dark_mode(DarlingWindow* win, int enable) {
    if (!win || !win->hwnd) {
        return;
    }
    
    darling_apply_dark_mode_internal(win, enable ? TRUE : FALSE);
}

void darling_set_auto_dark_mode(DarlingWindow* win) {
    if (!win || !win->hwnd) {
        return;
    }
    
    BOOL isDark = darling_is_system_dark_mode();
    darling_apply_dark_mode_internal(win, isDark);
}

void darling_set_titlebar_colors(DarlingWindow* win, uint32_t bg_color, uint32_t text_color) {
    if (!win || !win->hwnd) {
        return;
    }
    
    // Convert RGBA to COLORREF (BGR format)
    COLORREF bgColorRef = RGB(
        (bg_color >> 0) & 0xFF,   // R
        (bg_color >> 8) & 0xFF,   // G
        (bg_color >> 16) & 0xFF   // B
    );
    
    COLORREF textColorRef = RGB(
        (text_color >> 0) & 0xFF,  // R
        (text_color >> 8) & 0xFF,  // G
        (text_color >> 16) & 0xFF  // B
    );
    
    // Set caption color (Windows 11+)
    DwmSetWindowAttribute(
        win->hwnd,
        DWMWA_CAPTION_COLOR,
        &bgColorRef,
        sizeof(bgColorRef)
    );
    
    // Set text color (Windows 11+)
    DwmSetWindowAttribute(
        win->hwnd,
        DWMWA_TEXT_COLOR,
        &textColorRef,
        sizeof(textColorRef)
    );
    
    // Force redraw
    SetWindowPos(
        win->hwnd,
        NULL,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
    );
    
    RedrawWindow(win->hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
}

// Public API - Event Loop

void darling_poll_events(void) {
    MSG msg;
    
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

// Public API - Initialization and Cleanup

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