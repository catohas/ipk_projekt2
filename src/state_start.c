// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: state_start.c
// April 2025

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "./commands.h"
#include "./debug.h"
#include "./deserialize.h"
#include "./serialize.h"
#include "./global.h"
#include "./state.h"
#include "./network.h"
#include "./maximums.h"

void state_start_logic(cmd_ptr cmd)
{
    if (cmd == cmd_auth){
        printf_debug_simple(COLOR_INFO, "changing state to auth");
        state = STATE_AUTH;
        cmd();
    }
    else if (cmd == cmd_join) {
        printf("ERROR: trying to join channel while not authenticated\n");
    }
    else if (cmd == cmd_rename) {
        printf_debug_simple(COLOR_INFO, "renaming...");
        cmd();
    }
    else if (cmd == cmd_help) {
        cmd();
    }
    else if (cmd == cmd_msg) {
        printf("ERROR: trying to send message while not authenticated\n");
    }
    else {
        printf("ERROR: unknown input\n");
    }

}
