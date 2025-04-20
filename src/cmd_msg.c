// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: cmd_msg.c
// April 2025

#include <stdio.h>
#include <string.h>

#include "./commands.h"
#include "./debug.h"
#include "./global.h"
#include "./network.h"
#include "./maximums.h"
#include "./messages.h"
#include "./serialize.h"

void cmd_msg(void)
{
    printf_debug_simple(COLOR_INFO, "executing cmd_msg");

    char message[MAX_MESSAGE_CONTENT_LEN] = {0};
    memcpy(message, unprocessed_line, MAX_MESSAGE_CONTENT_LEN);

    if (use_tcp_protocol) {
        send_tcp_msg_msg(display_name, message);
    }
    else {
        struct MSG msg;
        create_msg(&msg, confirmed_msg_ids_index, display_name, message);
        size_t buffer_size;
        uint8_t *in_buffer = serialize_msg(&msg, &buffer_size);

        send_network_msg_udp(in_buffer, buffer_size);
    }

}
