// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: messages.h
// February 2025

#ifndef MESSAGES_H__
#define MESSAGES_H__

#include <stdint.h>

// For testing using googletest
#ifdef __cplusplus
extern "C" {
#endif

struct Auth_MSG;
struct Confirm_MSG;
struct Reply_MSG;
struct Join_MSG;
struct MSG;
struct Err_MSG;
struct Bye_MSG;
struct Ping_MSG;

void create_confirm_msg(struct Confirm_MSG *confirm_msg, uint16_t ref_message_id);
void create_reply_msg(struct Reply_MSG *reply_msg, uint16_t message_id, uint8_t result, uint16_t ref_message_id, char *message_contents);
void create_auth_msg(struct Auth_MSG *auth_msg, char *username, char *display_name, char *secret);
void create_join_msg(struct Join_MSG *join_msg, uint16_t message_id, char *channel_id, char *display_name);
void create_msg(struct MSG *msg, uint16_t message_id, char *display_name, char *message_contents);
void create_err_msg(struct Err_MSG *err_msg, uint16_t message_id, char *display_name, char *message_contents);
void create_bye_msg(struct Bye_MSG *bye_msg, uint16_t message_id, char *display_name);
void create_ping_msg(struct Ping_MSG *ping_msg, uint16_t message_id);

void free_confirm_msg(struct Confirm_MSG *msg);
void free_reply_msg(struct Reply_MSG *msg);
void free_auth_msg(struct Auth_MSG *msg);
void free_join_msg(struct Join_MSG *msg);
void free_msg(struct MSG *msg);
void free_err_msg(struct Err_MSG *msg);
void free_bye_msg(struct Bye_MSG *msg);
void free_ping_msg(struct Ping_MSG *msg);

enum MSG_TYPE
{
    CONFIRM,
    REPLY,
    AUTH,
    JOIN,
    MSG,
    PING,
    ERR,
    BYE,
};

static const uint8_t MSG_TYPE_VAL[] = {
    0x00,
    0x01,
    0x02,
    0x03,
    0x04,
    0xFD,
    0xFE,
    0xFF,
};

#pragma pack(1)

struct Confirm_MSG
{
    uint8_t type;
    uint16_t ref_message_id;
};

struct Reply_MSG
{
    uint8_t type;
    uint16_t message_id;
    uint8_t result;
    uint16_t ref_message_id;
    char *message_contents;
};

struct Auth_MSG
{
    uint8_t type;
    uint16_t message_id;
    char *username;
    char *display_name;
    char *secret;
};

struct Join_MSG
{
    uint8_t type;
    uint16_t message_id;
    char *channel_id;
    char *display_name;
};

struct MSG
{
    uint8_t type;
    uint16_t message_id;
    char *display_name;
    char *message_contents;
};

struct Ping_MSG
{
    uint8_t type;
    uint16_t message_id;
};

struct Err_MSG
{
    uint8_t type;
    uint16_t message_id;
    char *display_name;
    char *message_contents;
};

struct Bye_MSG
{
    uint8_t type;
    uint16_t message_id;
    char *display_name;
};

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif
