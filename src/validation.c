// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: validation.c
// April 2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "./validation.h"
#include "./maximums.h"

bool validate_id(const char *id)
{
    if (!id || strlen(id) == 0 || strlen(id) > MAX_USERNAME_LEN) {
        return false;
    }
    
    for (size_t i = 0; i < strlen(id); i++) {
        char c = id[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.')) {
            return false;
        }
    }
    
    return true;
}

bool validate_secret(const char *secret)
{
    if (!secret || strlen(secret) == 0 || strlen(secret) > MAX_SECRET_LEN) {
        return false;
    }
    
    for (size_t i = 0; i < strlen(secret); i++) {
        char c = secret[i];
        if (!(isalnum(c) || c == '_' || c == '-')) {
            return false;
        }
    }
    
    return true;
}

bool validate_display_name(const char *display_name)
{
    if (!display_name || strlen(display_name) == 0 || strlen(display_name) > MAX_DISPLAY_NAME_LEN) {
        return false;
    }
    
    for (size_t i = 0; i < strlen(display_name); i++) {
        char c = display_name[i];
        if (c < 0x21 || c > 0x7E) { // not a printable character
            return false;
        }
    }
    
    return true;
}

bool validate_message_content(const char *content)
{
    if (!content || strlen(content) == 0) {
        return false;
    }
    
    for (size_t i = 0; i < strlen(content); i++) {
        char c = content[i];
        if (!(c == 0x0A || c == 0x20 || (c >= 0x21 && c <= 0x7E))) {
            return false;
        }
    }
    
    return true;
}

char *truncate_message_content(const char *content)
{
    if (!content) {
        return NULL;
    }
    
    size_t content_len = strlen(content);
    size_t truncated_len = (content_len > MAX_MESSAGE_CONTENT_LEN) ? MAX_MESSAGE_CONTENT_LEN : content_len;
    
    char *truncated = malloc(truncated_len + 1);
    if (!truncated) {
        return NULL;
    }
    
    strncpy(truncated, content, truncated_len);
    truncated[truncated_len] = '\0';
    
    return truncated;
}
