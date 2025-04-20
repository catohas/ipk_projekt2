// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: state.h
// April 2025

#ifndef STATE_H__
#define STATE_H__

enum APP_STATE
{
    STATE_START,
    STATE_AUTH,
    STATE_OPEN,
    STATE_JOIN,
};

typedef void (*cmd_ptr)(void);

void state_start_logic(cmd_ptr cmd);
void state_auth_logic(cmd_ptr cmd);
void state_open_logic(cmd_ptr cmd);
void state_join_logic(cmd_ptr cmd);

void handle_reply_msg_state_auth(unsigned char *buffer, int length);
void handle_reply_msg_state_open(unsigned char *buffer, int length);
void handle_reply_msg_state_join(unsigned char *buffer, int length);

#endif
