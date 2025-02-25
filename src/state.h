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

#endif
