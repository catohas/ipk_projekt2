// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: state_join.c
// April 2025

#include <stdint.h>
#include <stdlib.h>

#include "./commands.h"
#include "./debug.h"
#include "./serialize.h"
#include "./deserialize.h"
#include "./network.h"
#include "./messages.h"
#include "./state.h"
#include "./global.h"

void state_join_logic(cmd_ptr cmd)
{
    if (cmd == cmd_auth){
        printf_debug_simple(COLOR_ERR, "trying to auth while in state JOIN");
        printf("ERROR: already authenticated\n");
    }
    else if (cmd == cmd_join) {
        printf_debug_simple(COLOR_ERR, "trying to join while in state JOIN");
        printf("ERROR: already joining\n");
    }
    else if (cmd == cmd_rename) {
        printf_debug_simple(COLOR_INFO, "renaming...");
        cmd();
    }
    else if (cmd == cmd_help) {
        cmd();
    }
    else if (cmd == cmd_msg) {
        printf("ERROR: trying to send message while joining...\n");
    }
    else {
        printf("ERROR: unknown input\n");
    }
}

void handle_reply_msg_state_join(unsigned char *buffer, int length)
{
    printf_debug_simple(COLOR_SUCCESS, "handling reply message, state: JOIN");

    struct Reply_MSG *reply_msg = deserialize_reply_msg(buffer, length);

    if (reply_msg->result == 1) {
        printf("Action Success: %s\n", reply_msg->message_contents);
        printf_debug_simple(COLOR_SUCCESS, "transitioning to state OPEN");
        state = STATE_OPEN;
    }
    else if (reply_msg->result == 0) {
        printf("Action Failure: %s\n", reply_msg->message_contents);
        printf_debug_simple(COLOR_SUCCESS, "transitioning to state OPEN");
        state = STATE_OPEN;
    }
    else {
        printf("ERROR: received malformed message\n");
        free_reply_msg(reply_msg);
        exit(EXIT_FAILURE);
    }

    struct Confirm_MSG con_msg;
    create_confirm_msg(&con_msg, reply_msg->message_id);

    size_t out_size;
    uint8_t *out_buffer = serialize_confirm_msg(&con_msg, &out_size);
    send_network_msg_udp(out_buffer, out_size);

    free_reply_msg(reply_msg);
    free(out_buffer);
}
