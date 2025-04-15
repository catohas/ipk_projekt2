#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "./commands.h"
#include "./debug.h"
#include "./deserialize.h"
#include "./global.h"
#include "./state.h"
#include "./maximums.h"
#include "./id.h"

void state_start_logic(cmd_ptr cmd)
{
    if (cmd == cmd_auth){
        printf_debug_simple(COLOR_INFO, "changing state to auth");
        state = STATE_AUTH;
        cmd();
        return;
    }
    else if (cmd == cmd_join) {
        printf("ERROR: trying to join channel while not authenticated\n");
    }
    else if (cmd == cmd_rename) {
        printf("ERROR: trying to rename while not authenticated\n");
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

void handle_confirm_msg_state_start(unsigned char *buffer, int length)
{
    struct Confirm_MSG *con_msg = deserialize_confirm_msg(buffer, length);
    uint16_t id = con_msg->ref_message_id;
    add_confirmed_msg_id(id);
    free_confirm_msg(con_msg);
}

void handle_reply_msg_state_start(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_auth_msg_state_start(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_join_msg_state_start(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_msg_state_start(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_ping_msg_state_start(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_err_msg_state_start(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_bye_msg_state_start(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}
