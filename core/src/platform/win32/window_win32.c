#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "darling.h"

struct DarlingWindow {
    HWND hwnd;
};

// --- Globals ---
static DarlingWindow* g_main_window = NULL;
static void (*g_close_callback)(void) = NULL;
static BOOL g_class_registered = FALSE;

// GDI objects for offscreen rendering
static HDC g_hdcMem = NULL;
static HBITMAP g_hBitmap = NULL;
static uint32_t g_bitmapWidth = 0;
static uint32_t g_bitmapHeight = 0;
// --- End Globals ---

static LRESULT CALLBACK darling_wnd_proc(
    HWND hwnd,
    UINT msg,
    WPARAM wp,
    LPARAM lp
) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (g_hdcMem) {
                BitBlt(
                    hdc,
                    ps.rcPaint.left,
                    ps.rcPaint.top,
                    ps.rcPaint.right - ps.rcPaint.left,
                    ps.rcPaint.bottom - ps.rcPaint.top,
                    g_hdcMem,
                    ps.rcPaint.left,
                    ps.rcPaint.top,
                    SRCCOPY
                );
            }
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CLOSE:
            if (g_close_callback) {
                g_close_callback();
            } else {
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// New signature: accept optional parent HWND (as uintptr_t). If parent_hwnd
// is non-zero, create the window as WS_CHILD attached to that parent.
DarlingWindow* darling_create_window(uint32_t w, uint32_t h, uintptr_t parent_hwnd) {
    // Register window class only once
    if (!g_class_registered) {
        WNDCLASSW wc = {0};
        wc.lpfnWndProc = darling_wnd_proc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = L"DarlingWindowClass";
        // Allows child windows to receive paint messages.
        wc.style = CS_HREDRAW | CS_VREDRAW;

        if (!RegisterClassW(&wc)) {
            return NULL;
        }
        g_class_registered = TRUE;
    }

    DWORD createStyles;
    HWND parent = NULL;
    if (parent_hwnd != (uintptr_t)0) {
        // Create as child of provided parent HWND.
        createStyles = WS_CHILD | WS_VISIBLE;
        parent = (HWND)(uintptr_t)parent_hwnd;
    } else {
        // Top-level frameless window.
        createStyles = WS_POPUP | WS_VISIBLE;
    }

    HWND hwnd = CreateWindowExW(
        0,
        L"DarlingWindowClass",
        L"Darling",
        createStyles,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        w,
        h,
        parent,
        NULL,
        GetModuleHandleW(NULL),
        NULL
    );

    if (!hwnd) {
        return NULL;
    }

    DarlingWindow* win = (DarlingWindow*)malloc(sizeof(DarlingWindow));
    
    if (!win) {
        DestroyWindow(hwnd);
        return NULL;
    }

    win->hwnd = hwnd;
    g_main_window = win;
    return win;
}

void darling_paint_frame(const unsigned char* bgra_data, uint32_t w, uint32_t h) {
    if (!g_main_window || !g_main_window->hwnd) return;

    HWND hwnd = g_main_window->hwnd;
    HDC hdc = GetDC(hwnd);

    // If our memory DC is not set up, or the size changed, recreate it.
    if (!g_hdcMem || g_bitmapWidth != w || g_bitmapHeight != h) {
        if (g_hdcMem) DeleteDC(g_hdcMem);
        if (g_hBitmap) DeleteObject(g_hBitmap);

        g_hdcMem = CreateCompatibleDC(hdc);
        g_bitmapWidth = w;
        g_bitmapHeight = h;

        BITMAPINFO bmi = {0};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = -((int32_t)h); // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        
        void* pBits; // We don't need to use this pointer directly.
        g_hBitmap = CreateDIBSection(g_hdcMem, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
        SelectObject(g_hdcMem, g_hBitmap);
    }
    
    // Update the bitmap with the new frame data.
    if (g_hBitmap) {
        SetBitmapBits(g_hBitmap, w * h * 4, bgra_data);
    }
    
    ReleaseDC(hwnd, hdc);

    // Trigger a repaint.
    InvalidateRect(hwnd, NULL, FALSE);
}


void darling_show_window(DarlingWindow* win) {
    if (win && win->hwnd) {
        ShowWindow(win->hwnd, SW_SHOW);
    }
}

void darling_poll_events(void) {
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void darling_destroy_window(DarlingWindow* win) {
    if (!win) return;
    
    if (win == g_main_window) {
        g_main_window = NULL;
    }
    
    if (win->hwnd) {
        DestroyWindow(win->hwnd);
    }
    
    // Clean up GDI objects
    if (g_hdcMem) {
        DeleteDC(g_hdcMem);
        g_hdcMem = NULL;
    }
    if (g_hBitmap) {
        DeleteObject(g_hBitmap);
        g_hBitmap = NULL;
    }
    
    free(win);
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