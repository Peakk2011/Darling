#include "internal.h"
#include <stdio.h>

// Thread Safety

void darling_ensure_lock(void) {
    if (!g_lock_initialized) {
        InitializeCriticalSection(&g_lock);
        g_lock_initialized = TRUE;
    }
}

void darling_lock(void) {
    if (!g_lock_initialized) {
        darling_ensure_lock();
    }
    EnterCriticalSection(&g_lock);
}

void darling_unlock(void) {
    if (g_lock_initialized) {
        LeaveCriticalSection(&g_lock);
    }
}

// Logging and Debugging

void darling_output_debug(const wchar_t* text) {
    if (text) {
        OutputDebugStringW(text);
    }
}

void darling_log_last_error(const wchar_t* context) {
    DWORD err = GetLastError();
    wchar_t* sys = NULL;
    
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM | 
                  FORMAT_MESSAGE_IGNORE_INSERTS;
    
    DWORD len = FormatMessageW(
        flags,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&sys,
        0,
        NULL
    );

    wchar_t buffer[DARLING_LOG_BUFFER_SIZE];
    
    if (len && sys) {
        swprintf_s(
            buffer,
            ARRAYSIZE(buffer),
            L"[Darling][win32] %ls failed with error %lu: %ls\n",
            context,
            (unsigned long)err,
            sys
        );
    } else {
        swprintf_s(
            buffer,
            ARRAYSIZE(buffer),
            L"[Darling][win32] %ls failed with error %lu.\n",
            context,
            (unsigned long)err
        );
    }

    darling_output_debug(buffer);

    if (sys) {
        LocalFree(sys);
    }
}

void darling_log_create_params(uint32_t w, uint32_t h, uintptr_t parent_hwnd, DWORD styles) {
    wchar_t buffer[DARLING_LOG_PARAMS_SIZE];
    
    swprintf_s(
        buffer,
        ARRAYSIZE(buffer),
        L"[Darling][win32] CreateWindowExW params: w=%u h=%u parent=0x%p styles=0x%08lX\n",
        w,
        h,
        (void*)(uintptr_t)parent_hwnd,
        (unsigned long)styles
    );
    
    darling_output_debug(buffer);
}

// Theme Detection

BOOL darling_is_system_dark_mode(void) {
    DWORD value = 0;
    DWORD size = sizeof(value);
    HKEY key = NULL;
    BOOL isDark = FALSE;
    
    // Open registry key
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0,
        KEY_READ,
        &key
    );
    
    if (result == ERROR_SUCCESS) {
        // Query the value
        result = RegQueryValueExW(
            key,
            L"AppsUseLightTheme",
            NULL,
            NULL,
            (LPBYTE)&value,
            &size
        );
        
        if (result == ERROR_SUCCESS) {
            // 0 = Dark mode, 1 = Light mode
            isDark = (value == 0);
        }
        
        RegCloseKey(key);
    }
    
    return isDark;
}