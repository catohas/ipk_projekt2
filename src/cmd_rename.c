// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: cmd_rename.c
// April 2025

#include <stdio.h>
#include <string.h>

#include "./commands.h"
#include "./debug.h"
#include "./maximums.h"
#include "./global.h"

void cmd_rename(void)
{
    printf_debug_simple(COLOR_INFO, "executing cmd_rename");

    line = strtok(NULL, " "); // get rid of /rename
    if (line == NULL) {
        fprintf(stderr, "Please provide 'display_name'\n");
        return;
    }
    memcpy(display_name, line, MAX_DISPLAY_NAME_LEN);

    printf_debug(COLOR_SUCCESS, "changed display name to %s", display_name);
}
