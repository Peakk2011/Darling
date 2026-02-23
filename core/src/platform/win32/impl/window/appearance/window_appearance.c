#include "../../internal.h"

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

    if (!win->isChild) {
        LONG exStyle = GetWindowLongW(win->hwnd, GWL_EXSTYLE);
        LONG style = GetWindowLongW(win->hwnd, GWL_STYLE);

        if (visible) {
            exStyle &= ~WS_EX_DLGMODALFRAME;
            style &= ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
            style |= WS_OVERLAPPEDWINDOW;
        } else {
            exStyle |= WS_EX_DLGMODALFRAME;
            style &= ~WS_OVERLAPPEDWINDOW;
            style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
        }

        SetWindowLongW(win->hwnd, GWL_EXSTYLE, exStyle);
        SetWindowLongW(win->hwnd, GWL_STYLE, style);

        SetWindowPos(
            win->hwnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOACTIVATE | SWP_FRAMECHANGED
        );

        RedrawWindow(win->hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
    }

    darling_unlock();
}

void darling_set_window_opacity(DarlingWindow* win, uint8_t opacity) {
    if (!win || !win->hwnd) {
        return;
    }

    LONG exStyle = GetWindowLongW(win->hwnd, GWL_EXSTYLE);

    if (opacity < 255) {
        if ((exStyle & WS_EX_LAYERED) == 0) {
            SetWindowLongW(win->hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
        }

        SetLayeredWindowAttributes(win->hwnd, 0, opacity, LWA_ALPHA);
    } else {
        if (exStyle & WS_EX_LAYERED) {
            SetWindowLongW(win->hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED);
        }
    }
}

void darling_set_always_on_top(DarlingWindow* win, int enable) {
    if (!win || !win->hwnd) {
        return;
    }

    HWND insertAfter = enable ? HWND_TOPMOST : HWND_NOTOPMOST;
    SetWindowPos(
        win->hwnd,
        insertAfter,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
    );
}

void darling_cleanup_window_icon(DarlingWindow* win) {
    if (win && win->customIcon) {
        DestroyIcon(win->customIcon);
        win->customIcon = NULL;
    }
}