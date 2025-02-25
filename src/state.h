#ifndef STATE_H__
#define STATE_H__

enum APP_STATE
{
    STATE_START,
    STATE_AUTH,
    STATE_JOIN,
    STATE_ERR,
    STATE_BYE,
    STATE_MSG,
    STATE_REPLY,
    STATE_NEG_REPLY,
};

typedef void (*cmd_ptr)(void);

void state_start_logic(cmd_ptr cmd);
void state_auth_logic(cmd_ptr cmd);
void state_join_logic(cmd_ptr cmd);
void state_err_logic(cmd_ptr cmd);
void state_bye_logic(cmd_ptr cmd);
void state_msg_logic(cmd_ptr cmd);
void state_reply_logic(cmd_ptr cmd);
void state_neg_reply_logic(cmd_ptr cmd);

#endif
