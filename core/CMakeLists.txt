cmake_minimum_required(VERSION 3.15)
project(darling C)

add_library(darling STATIC
    src/darling.c
    src/platform/win32/window_win32.c
)

target_include_directories(darling PUBLIC include)
