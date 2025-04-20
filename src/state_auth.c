#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "./commands.h"
#include "./debug.h"
#include "./serialize.h"
#include "./deserialize.h"
#include "./state.h"
#include "./global.h"
#include "./network.h"
#include "./messages.h"

void state_auth_logic(cmd_ptr cmd)
{
    if (cmd == cmd_auth){
        cmd();
    }
    else if (cmd == cmd_join) {
        printf("ERROR: trying to join channel while not authenticated\n");
        return;
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

void handle_reply_msg_state_auth(unsigned char *buffer, int length)
{
    printf_debug_simple(COLOR_SUCCESS, "handling reply message, state: AUTH");
    struct Reply_MSG *reply_msg = deserialize_reply_msg(buffer, length);
    if (reply_msg->result == 1) {
        printf("Action Success: %s\n", reply_msg->message_contents);
        printf_debug_simple(COLOR_SUCCESS, "transitioning to state OPEN");
        state = STATE_OPEN;
    }
    else if (reply_msg->result == 0) {
        printf("Action Failure: %s\n", reply_msg->message_contents);
    }
    else {
        // malformed message i guess
    }

    struct Confirm_MSG con_msg;
    create_confirm_msg(&con_msg, reply_msg->message_id);

    size_t out_size;
    uint8_t *out_buffer = serialize_confirm_msg(&con_msg, &out_size);
    send_network_msg_udp(out_buffer, out_size);

    free_reply_msg(reply_msg);
    free(out_buffer);
}
