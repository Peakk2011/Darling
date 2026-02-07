#include "internal.h"

// Window List Management

void darling_list_add(DarlingWindow* win) {
    if (!win || win->inList) {
        return;
    }

    darling_lock();
    
    win->prev = NULL;
    win->next = g_window_head;
    
    if (g_window_head) {
        g_window_head->prev = win;
    }
    
    g_window_head = win;
    win->inList = TRUE;
    
    darling_unlock();
}

void darling_list_remove(DarlingWindow* win) {
    if (!win || !win->inList) {
        return;
    }

    darling_lock();
    
    if (win->prev) {
        win->prev->next = win->next;
    } else {
        g_window_head = win->next;
    }
    
    if (win->next) {
        win->next->prev = win->prev;
    }
    
    win->prev = NULL;
    win->next = NULL;
    win->inList = FALSE;
    
    darling_unlock();
}

DarlingWindow* darling_select_new_main_window(void) {
    DarlingWindow* cur = g_window_head;
    
    while (cur) {
        if (!cur->isChild) {
            return cur;
        }
        cur = cur->next;
    }
    
    return g_window_head;
}

void darling_update_main_on_remove(DarlingWindow* removed) {
    if (removed == g_main_window) {
        g_main_window = darling_select_new_main_window();
    }
}