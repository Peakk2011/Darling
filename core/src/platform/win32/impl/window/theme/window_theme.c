#include "../../internal.h"

void darling_apply_dark_mode_internal(DarlingWindow* win, BOOL enable) {
    if (!win || !win->hwnd) {
        return;
    }

    BOOL value = enable;
    DwmSetWindowAttribute(
        win->hwnd,
        DWMWA_USE_IMMERSIVE_DARK_MODE,
        &value,
        sizeof(value)
    );

    win->darkMode = enable;

    SetWindowPos(
        win->hwnd,
        NULL,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
    );

    RedrawWindow(
        win->hwnd,
        NULL,
        NULL,
        RDW_FRAME |
        RDW_INVALIDATE |
        RDW_UPDATENOW
    );
}

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

    COLORREF bgColorRef = RGB(
        (bg_color >> 0) & 0xFF,
        (bg_color >> 8) & 0xFF,
        (bg_color >> 16) & 0xFF
    );

    COLORREF textColorRef = RGB(
        (text_color >> 0) & 0xFF,
        (text_color >> 8) & 0xFF,
        (text_color >> 16) & 0xFF
    );

    DwmSetWindowAttribute(
        win->hwnd,
        DWMWA_CAPTION_COLOR,
        &bgColorRef,
        sizeof(bgColorRef)
    );

    DwmSetWindowAttribute(
        win->hwnd,
        DWMWA_TEXT_COLOR,
        &textColorRef,
        sizeof(textColorRef)
    );

    SetWindowPos(
        win->hwnd,
        NULL,
        0,
        0,
        0,
        0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
    );

    RedrawWindow(
        win->hwnd,
        NULL,
        NULL,
        RDW_FRAME |
        RDW_INVALIDATE |
        RDW_UPDATENOW
    );
}

void darling_set_titlebar_color(DarlingWindow* win, uint32_t color) {
    if (!win || !win->hwnd) {
        return;
    }

    COLORREF titlebarColor = RGB(
        (color >> 16) & 0xFF,
        (color >> 8) & 0xFF,
        color & 0xFF
    );

    DwmSetWindowAttribute(
        win->hwnd,
        DWMWA_CAPTION_COLOR,
        &titlebarColor,
        sizeof(titlebarColor)
    );

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

void darling_set_corner_preference(DarlingWindow* win, DarlingCornerPreference pref) {
    if (!win || !win->hwnd) {
        return;
    }

    int cornerPref = (int)pref;
    
    DwmSetWindowAttribute(
        win->hwnd,
        DWMWA_WINDOW_CORNER_PREFERENCE,
        &cornerPref,
        sizeof(cornerPref)
    );
}

void darling_flash_window(DarlingWindow* win, int continuous) {
    if (!win || !win->hwnd) {
        return;
    }

    FLASHWINFO fwi = {0};

    fwi.cbSize = sizeof(FLASHWINFO);
    fwi.hwnd = win->hwnd;
    fwi.dwFlags = FLASHW_ALL | (continuous ? FLASHW_TIMER : 0);
    fwi.uCount = continuous ? 0 : 3;
    fwi.dwTimeout = 0;

    FlashWindowEx(&fwi);
}

uint32_t darling_get_dpi(DarlingWindow* win) {
    if (!win || !win->hwnd) {
        return 96;
    }

    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (!user32) {
        return 96;
    }

    typedef UINT (WINAPI *DarlingGetDpiForWindowFn)(HWND);

    DarlingGetDpiForWindowFn getDpiForWindow =
        (DarlingGetDpiForWindowFn)GetProcAddress(user32, "GetDpiForWindow");

    if (!getDpiForWindow) {
        return 96;
    }

    return (uint32_t)getDpiForWindow(win->hwnd);
}

float darling_get_scale_factor(DarlingWindow* win) {
    uint32_t dpi = darling_get_dpi(win);
    return (float)dpi / 96.0f;
}