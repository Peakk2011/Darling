#include "internal.h"
#include <string.h>

// GDI Resource Management

void darling_free_gdi(DarlingWindow* win) {
    if (!win) {
        return;
    }

    if (win->hdcMem) {
        DeleteDC(win->hdcMem);
        win->hdcMem = NULL;
    }
    
    if (win->hBitmap) {
        DeleteObject(win->hBitmap);
        win->hBitmap = NULL;
    }
    
    win->dibBits = NULL;
    win->bitmapWidth = 0;
    win->bitmapHeight = 0;
}

// Window Painting

void darling_handle_paint(DarlingWindow* win, HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    
    if (win && win->hdcMem) {
        int srcLeft = ps.rcPaint.left;
        int srcTop = ps.rcPaint.top;
        int srcRight = ps.rcPaint.right;
        int srcBottom = ps.rcPaint.bottom;

        int copyWidth = srcRight - srcLeft;
        int copyHeight = srcBottom - srcTop;

        // Validate copy region
        if (copyWidth > 0 && copyHeight > 0) {
            if (srcLeft < 0 || srcTop < 0 ||
                (uint32_t)srcLeft >= win->bitmapWidth || 
                (uint32_t)srcTop >= win->bitmapHeight) {
                copyWidth = 0;
                copyHeight = 0;
            } else {
                uint32_t maxWidth = win->bitmapWidth - (uint32_t)srcLeft;
                uint32_t maxHeight = win->bitmapHeight - (uint32_t)srcTop;
                
                if ((uint32_t)copyWidth > maxWidth) {
                    copyWidth = (int)maxWidth;
                }
                if ((uint32_t)copyHeight > maxHeight) {
                    copyHeight = (int)maxHeight;
                }
            }
        }

        // Perform blit
        if (copyWidth > 0 && copyHeight > 0) {
            BitBlt(
                hdc,
                srcLeft,
                srcTop,
                copyWidth,
                copyHeight,
                win->hdcMem,
                srcLeft,
                srcTop,
                SRCCOPY
            );
        }
    }
    
    EndPaint(hwnd, &ps);
}

// Public API - Window Painting

void darling_paint_frame_window(DarlingWindow* win, const unsigned char* bgra_data, uint32_t w, uint32_t h) {
    if (!win || !win->hwnd || !bgra_data || w == 0 || h == 0) {
        return;
    }

    HWND hwnd = win->hwnd;
    HDC hdc = GetDC(hwnd);
    if (!hdc) {
        return;
    }

    // Recreate bitmap if size changed
    if (!win->hdcMem || win->bitmapWidth != w || win->bitmapHeight != h) {
        darling_free_gdi(win);

        win->hdcMem = CreateCompatibleDC(hdc);
        if (!win->hdcMem) {
            ReleaseDC(hwnd, hdc);
            return;
        }

        win->bitmapWidth = w;
        win->bitmapHeight = h;

        BITMAPINFO bmi = {0};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = (LONG)w;
        bmi.bmiHeader.biHeight = -((LONG)h);  // Top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits = NULL;
        win->hBitmap = CreateDIBSection(win->hdcMem, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
        
        if (!win->hBitmap || !pBits) {
            darling_free_gdi(win);
            ReleaseDC(hwnd, hdc);
            return;
        }

        win->dibBits = pBits;
        SelectObject(win->hdcMem, win->hBitmap);
    }

    // Update bitmap data
    if (win->dibBits) {
        // Check for overflow
        if (w > SIZE_MAX / h / 4u) {
            ReleaseDC(hwnd, hdc);
            return;
        }
        
        size_t size = (size_t)w * (size_t)h * 4u;
        memcpy(win->dibBits, bgra_data, size);
    }

    ReleaseDC(hwnd, hdc);

    // Trigger repaint
    InvalidateRect(hwnd, NULL, FALSE);
}

void darling_paint_frame(const unsigned char* bgra_data, uint32_t w, uint32_t h) {
    darling_paint_frame_window(g_main_window, bgra_data, w, h);
}