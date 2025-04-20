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
#include "./validation.h"

void cmd_rename(void)
{
    printf_debug_simple(COLOR_INFO, "executing cmd_rename");

    char new_display_name[MAX_DISPLAY_NAME_LEN+1] = {0};

    line = strtok(NULL, " "); // get rid of /rename
    if (line == NULL) {
        fprintf(stderr, "Please provide 'display_name'\n");
        return;
    }
    strncpy(new_display_name, line, MAX_DISPLAY_NAME_LEN);

    if (!validate_display_name(new_display_name)) {
        printf("ERROR: Invalid display name. Must be 1-20 printable characters\n");
        return;
    }

    // update global var
    memset(display_name, 0, MAX_DISPLAY_NAME_LEN);
    strcpy(display_name, new_display_name);

    printf_debug(COLOR_SUCCESS, "changed display name to %s", display_name);
}
