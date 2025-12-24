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

DARLING_API DarlingWindow* darling_create_window(
    uint32_t width,
    uint32_t height
);

DARLING_API void darling_show_window(DarlingWindow* win);
DARLING_API void darling_poll_events(void);
DARLING_API void darling_destroy_window(DarlingWindow* win);
// Return the HWND of the main Darling window (0 if none).
DARLING_API uintptr_t darling_get_main_hwnd(void);

// Sets a callback to be invoked when the main window receives WM_CLOSE.
DARLING_API void darling_set_close_callback(void (*callback)());

#ifdef __cplusplus
}
#endif
