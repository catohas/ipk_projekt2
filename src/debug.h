// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: debug.h
// February 2025 

#ifndef DEBUG_H__
#define DEBUG_H__

#ifdef DEBUG_PRINT
    #define printf_debug_simple(format) fprintf(stderr, "%s:%-4d | %15s | " format "\n", __FILE__, __LINE__, __func__)
    #define printf_debug(format, ...) fprintf(stderr, "%s:%-4d | %15s | " format "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
    #define printf_debug_simple(format) do {} while(0)
    #define printf_debug(format, ...) do {} while(0)
#endif

#endif
