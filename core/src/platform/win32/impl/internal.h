#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#include <stdint.h>

#pragma comment(lib, "dwmapi.lib")

// Constants
#define DARLING_LOG_BUFFER_SIZE 1024
#define DARLING_LOG_PARAMS_SIZE 512
#define DARLING_WINDOW_CLASS L"DarlingWindowClass"

// DWM Attributes (for older Windows SDKs)
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36
#endif

// Types

typedef struct DarlingWindow {
    HWND hwnd;
    HDC hdcMem;
    HBITMAP hBitmap;
    uint32_t bitmapWidth;
    uint32_t bitmapHeight;
    void* dibBits;
    
    BOOL isChild;
    BOOL inList;
    BOOL darkMode;
    HWND childHwnd;
    
    struct DarlingWindow* prev;
    struct DarlingWindow* next;
} DarlingWindow;

// Global State (defined in window.c)

extern DarlingWindow* g_main_window;
extern DarlingWindow* g_window_head;
extern void (*g_close_callback)(void);
extern BOOL g_class_registered;
extern CRITICAL_SECTION g_lock;
extern BOOL g_lock_initialized;

// Internal Function Declarations

// Thread Safety (utils.c)
void darling_ensure_lock(void);
void darling_lock(void);
void darling_unlock(void);

// Logging (utils.c)
void darling_output_debug(const wchar_t* text);
void darling_log_last_error(const wchar_t* context);
void darling_log_create_params(uint32_t w, uint32_t h, uintptr_t parent_hwnd, DWORD styles);

// Theme Detection (utils.c)
BOOL darling_is_system_dark_mode(void);

// GDI Resource Management (paint.c)
void darling_free_gdi(DarlingWindow* win);

// Window List Management (list.c)
void darling_list_add(DarlingWindow* win);
void darling_list_remove(DarlingWindow* win);
DarlingWindow* darling_select_new_main_window(void);
void darling_update_main_on_remove(DarlingWindow* removed);

// Window Procedure (window.c)
LRESULT CALLBACK darling_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
BOOL darling_register_class(void);

// Theme Application (window.c)
void darling_apply_dark_mode_internal(DarlingWindow* win, BOOL enable);