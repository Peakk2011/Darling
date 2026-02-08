#pragma once
#include <stdint.h>

#ifdef _WIN32
    #ifdef DARLING_BUILD
        #define DARLING_API __declspec(dllexport)
    #else
        #define DARLING_API __declspec(dllimport)
    #endif
#else
    #define DARLING_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DarlingWindow DarlingWindow;

typedef enum DarlingCornerPreference {
    DARLING_CORNER_DEFAULT = 0,
    DARLING_CORNER_NONE = 1,
    DARLING_CORNER_SMALL = 2,
    DARLING_CORNER_LARGE = 3
} DarlingCornerPreference;

// Window Management

// Create a Darling native window. If `parent_hwnd` is non-zero on Windows,
// the window will be created with `WS_CHILD | WS_VISIBLE` and attached to the
// specified parent. Pass 0 to create a top-level window (with title bar by default).
DARLING_API DarlingWindow* darling_create_window(
    uint32_t width,
    uint32_t height,
    uintptr_t parent_hwnd
);

// Show a window
DARLING_API void darling_show_window(DarlingWindow* win);

// Hide a window
DARLING_API void darling_hide_window(DarlingWindow* win);

// Destroy a window and free its resources
DARLING_API void darling_destroy_window(DarlingWindow* win);

// Return the HWND of the main Darling window (0 if none)
DARLING_API uintptr_t darling_get_main_hwnd(void);

// Window Properties

// Associate a child HWND with a Darling window (used for resize parenting)
DARLING_API void darling_set_child_hwnd(DarlingWindow* win, uintptr_t child_hwnd);

// Set the title text for a Darling window
DARLING_API void darling_set_window_title(DarlingWindow* win, const wchar_t* title);

// Show or hide the title bar icon (uses default app icon when shown)
DARLING_API void darling_set_window_icon_visible(DarlingWindow* win, int visible);

// Focus a window (bring to foreground)
DARLING_API void darling_focus_window(DarlingWindow* win);

// Check visibility (1 = visible, 0 = hidden)
DARLING_API int darling_is_visible(DarlingWindow* win);

// Check focus state (1 = focused, 0 = not focused)
DARLING_API int darling_is_focused(DarlingWindow* win);

// Set window opacity (0-255)
DARLING_API void darling_set_window_opacity(DarlingWindow* win, uint8_t opacity);

// Enable or disable always-on-top
DARLING_API void darling_set_always_on_top(DarlingWindow* win, int enable);

// Theme Management

// Check if system is in dark mode (1 = dark, 0 = light)
DARLING_API int darling_is_dark_mode(void);

// Manually set dark mode for a window (1 = dark, 0 = light)
DARLING_API void darling_set_dark_mode(DarlingWindow* win, int enable);

// Auto-detect and apply system theme to a window
DARLING_API void darling_set_auto_dark_mode(DarlingWindow* win);

// Set custom titlebar colors (Windows 11+)
// Colors are in RGBA format: 0xRRGGBBAA (alpha ignored)
// Example: 0x202020FF = dark gray, 0xFFFFFFFF = white
DARLING_API void darling_set_titlebar_colors(
    DarlingWindow* win,
    uint32_t bg_color,
    uint32_t text_color
);

// Set titlebar color only (Windows 11+)
// Color is RGB format: 0xRRGGBB
DARLING_API void darling_set_titlebar_color(DarlingWindow* win, uint32_t color);

// Set rounded corner preference (Windows 11+)
DARLING_API void darling_set_corner_preference(
    DarlingWindow* win,
    DarlingCornerPreference pref
);

// Flash the window (continuous = 1 for continuous flashing)
DARLING_API void darling_flash_window(DarlingWindow* win, int continuous);

// DPI helpers
DARLING_API uint32_t darling_get_dpi(DarlingWindow* win);
DARLING_API float darling_get_scale_factor(DarlingWindow* win);

// Rendering

// Paint a BGRA bitmap onto the main window
DARLING_API void darling_paint_frame(
    const unsigned char* bgra_data,
    uint32_t width,
    uint32_t height
);

// Paint a BGRA bitmap onto a specific window
DARLING_API void darling_paint_frame_window(
    DarlingWindow* win,
    const unsigned char* bgra_data,
    uint32_t width,
    uint32_t height
);

// Event Loop

// Process all pending window messages
DARLING_API void darling_poll_events(void);

// Set a callback to be invoked when the main window receives WM_CLOSE
DARLING_API void darling_set_close_callback(void (*callback)());

// Initialization

// Initialize global state (thread-safety, etc)
DARLING_API void darling_init(void);

// Clean up global state and unregister the window class
DARLING_API void darling_cleanup(void);

#ifdef __cplusplus
}
#endif