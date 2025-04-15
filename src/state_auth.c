#include <stdio.h>

#include "./commands.h"
#include "./debug.h"
#include "./state.h"

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

void handle_confirm_msg_state_auth(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_reply_msg_state_auth(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_auth_msg_state_auth(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_join_msg_state_auth(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_msg_state_auth(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_ping_msg_state_auth(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_err_msg_state_auth(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}

void handle_bye_msg_state_auth(unsigned char *buffer, int length)
{
    (void)buffer;
    (void)length;
}
