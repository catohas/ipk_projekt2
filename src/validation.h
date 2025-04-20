// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: validation.h
// April 2025

#ifndef VALIDATION_H__
#define VALIDATION_H__

#include <stdbool.h>

bool validate_id(const char *id);
bool validate_secret(const char *secret);
bool validate_display_name(const char *display_name);
bool validate_message_content(const char *content);
char *truncate_message_content(const char *content);

#endif
