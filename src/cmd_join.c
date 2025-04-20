// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: cmd_join.c
// April 2025

#include <stdio.h>
#include <string.h>

#include "./commands.h"
#include "./debug.h"
#include "./global.h"
#include "./network.h"
#include "./messages.h"
#include "./serialize.h"
#include "./validation.h"

void cmd_join(void)
{
    printf_debug_simple(COLOR_INFO, "executing cmd_join");

    char channel_id[MAX_CHANNEL_ID_LEN+1] = {0};

    line = strtok(NULL, " "); // get rid of /join
    if (line == NULL) {
        fprintf(stderr, "Please provide 'channel_id'\n");
        return;
    }
    strncpy(channel_id, line, MAX_CHANNEL_ID_LEN);

    if (!validate_id(channel_id)) {
        printf("ERROR: Invalid channel ID. Must be 1-20 characters from [a-zA-Z0-9_-]\n");
        return;
    }

    if (!validate_display_name(display_name)) {
        printf("ERROR: Invalid display name. Must be 1-20 printable characters\n");
        return;
    }

    state = STATE_JOIN;
    printf_debug_simple(COLOR_SUCCESS, "transitioned to state JOIN");

    if (use_tcp_protocol) {
        send_tcp_join_msg(channel_id, display_name);
    }
    else {
        struct Join_MSG join_msg;
        create_join_msg(&join_msg, confirmed_msg_ids_index, channel_id, display_name);
        size_t buffer_size;
        uint8_t *in_buffer = serialize_join_msg(&join_msg, &buffer_size);

        send_network_msg_udp(in_buffer, buffer_size);
    }
}
