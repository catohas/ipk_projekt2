// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: debug.h
// February 2025 

#ifndef DEBUG_H__
#define DEBUG_H__

#define COLOR_INFO  "\x1b[33m"
#define COLOR_ERR   "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

#ifdef DEBUG_PRINT
    #define printf_debug_simple(level, format) \
        fprintf(stderr, level "%s:%-4d | %15s | " format COLOR_RESET "\n", __FILE__, __LINE__, __func__);

    #define printf_debug(level, format, ...) \
        fprintf(stderr, level "%s:%-4d | %15s | " format COLOR_RESET "\n", __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
    #define printf_debug_simple(level, format) do {} while(0)
    #define printf_debug(level, format, ...) do {} while(0)
#endif

#endif
