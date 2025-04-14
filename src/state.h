#ifndef STATE_H__
#define STATE_H__

enum APP_STATE
{
    STATE_START,
    STATE_AUTH,
    STATE_OPEN,
    STATE_JOIN,
    STATE_END,
};

typedef void (*cmd_ptr)(void);

void state_start_logic(cmd_ptr cmd);
void state_auth_logic(cmd_ptr cmd);
void state_open_logic(cmd_ptr cmd);
void state_join_logic(cmd_ptr cmd);
void state_end_logic(cmd_ptr cmd);

void handle_confirm_msg_state_start(unsigned char *buffer, int length);
void handle_reply_msg_state_start(unsigned char *buffer, int length);
void handle_auth_msg_state_start(unsigned char *buffer, int length);
void handle_join_msg_state_start(unsigned char *buffer, int length);
void handle_msg_state_start(unsigned char *buffer, int length);
void handle_ping_msg_state_start(unsigned char *buffer, int length);
void handle_err_msg_state_start(unsigned char *buffer, int length);
void handle_bye_msg_state_start(unsigned char *buffer, int length);

void handle_confirm_msg_state_auth(unsigned char *buffer, int length);
void handle_reply_msg_state_auth(unsigned char *buffer, int length);
void handle_auth_msg_state_auth(unsigned char *buffer, int length);
void handle_join_msg_state_auth(unsigned char *buffer, int length);
void handle_msg_state_auth(unsigned char *buffer, int length);
void handle_ping_msg_state_auth(unsigned char *buffer, int length);
void handle_err_msg_state_auth(unsigned char *buffer, int length);
void handle_bye_msg_state_auth(unsigned char *buffer, int length);

void handle_confirm_msg_state_open(unsigned char *buffer, int length);
void handle_reply_msg_state_open(unsigned char *buffer, int length);
void handle_auth_msg_state_open(unsigned char *buffer, int length);
void handle_join_msg_state_open(unsigned char *buffer, int length);
void handle_msg_state_open(unsigned char *buffer, int length);
void handle_ping_msg_state_open(unsigned char *buffer, int length);
void handle_err_msg_state_open(unsigned char *buffer, int length);
void handle_bye_msg_state_open(unsigned char *buffer, int length);

void handle_confirm_msg_state_join(unsigned char *buffer, int length);
void handle_reply_msg_state_join(unsigned char *buffer, int length);
void handle_auth_msg_state_join(unsigned char *buffer, int length);
void handle_join_msg_state_join(unsigned char *buffer, int length);
void handle_msg_state_join(unsigned char *buffer, int length);
void handle_ping_msg_state_join(unsigned char *buffer, int length);
void handle_err_msg_state_join(unsigned char *buffer, int length);
void handle_bye_msg_state_join(unsigned char *buffer, int length);

void handle_confirm_msg_state_end(unsigned char *buffer, int length);
void handle_reply_msg_state_end(unsigned char *buffer, int length);
void handle_auth_msg_state_end(unsigned char *buffer, int length);
void handle_join_msg_state_end(unsigned char *buffer, int length);
void handle_msg_state_end(unsigned char *buffer, int length);
void handle_ping_msg_state_end(unsigned char *buffer, int length);
void handle_err_msg_state_end(unsigned char *buffer, int length);
void handle_bye_msg_state_end(unsigned char *buffer, int length);

#endif
