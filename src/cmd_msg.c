// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: cmd_msg.c
// April 2025

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./commands.h"
#include "./debug.h"
#include "./global.h"
#include "./network.h"
#include "./maximums.h"
#include "./messages.h"
#include "./serialize.h"
#include "./validation.h"

void cmd_msg(void)
{
    printf_debug_simple(COLOR_INFO, "executing cmd_msg");

    if (!validate_display_name(display_name)) {
        printf("ERROR: Invalid display name. Must be 1-20 printable characters\n");
        return;
    }

    if (!validate_message_content(unprocessed_line)) {
        printf("ERROR: Invalid message content. Must contain only printable characters, spaces, or line feeds\n");
        return;
    }

    char *message;
    if (strlen(unprocessed_line) > MAX_MESSAGE_CONTENT_LEN) {
        message = truncate_message_content(unprocessed_line);
        if (!message) {
            printf("ERROR: Failed to allocate memory for message truncation\n");
            return;
        }
        printf("ERROR: Message too long, truncated to %d characters\n", MAX_MESSAGE_CONTENT_LEN);
    } else {
        message = strdup(unprocessed_line);
        if (!message) {
            printf("ERROR: Failed to allocate memory for message\n");
            return;
        }
    }

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

    free(message);
}
