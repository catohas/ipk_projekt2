// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: messages.c
// April 2025

#include <stdlib.h>

#include "./messages.h"

void create_confirm_msg(struct Confirm_MSG *confirm_msg, uint16_t ref_message_id)
{
    confirm_msg->type = MSG_TYPE_VAL[CONFIRM];
    confirm_msg->ref_message_id = ref_message_id;
}

void create_reply_msg(struct Reply_MSG *reply_msg, uint16_t message_id, uint8_t result, uint16_t ref_message_id, char *message_contents)
{
    reply_msg->type = MSG_TYPE_VAL[REPLY];
    reply_msg->message_id = message_id;
    reply_msg->result = result;
    reply_msg->ref_message_id = ref_message_id;
    reply_msg->message_contents = message_contents;
}

void create_auth_msg(struct Auth_MSG *auth_msg, uint16_t message_id, char *username, char *display_name, char *secret)
{
    auth_msg->type = MSG_TYPE_VAL[AUTH];
    auth_msg->message_id = message_id;
    auth_msg->username = username; 
    auth_msg->display_name = display_name;
    auth_msg->secret = secret;
}

void create_join_msg(struct Join_MSG *join_msg, uint16_t message_id, char *channel_id, char *display_name)
{
    join_msg->type = MSG_TYPE_VAL[JOIN];
    join_msg->message_id = message_id;
    join_msg->channel_id = channel_id;
    join_msg->display_name = display_name;
}

void create_msg(struct MSG *msg, uint16_t message_id, char *display_name, char *message_contents)
{
    msg->type = MSG_TYPE_VAL[MSG];
    msg->message_id = message_id;
    msg->display_name = display_name;
    msg->message_contents = message_contents;
}

void create_err_msg(struct Err_MSG *err_msg, uint16_t message_id, char *display_name, char *message_contents)
{
    err_msg->type = MSG_TYPE_VAL[ERR];
    err_msg->message_id = message_id;
    err_msg->display_name = display_name;
    err_msg->message_contents = message_contents;
}

void create_bye_msg(struct Bye_MSG *bye_msg, uint16_t message_id, char *display_name)
{
    bye_msg->type = MSG_TYPE_VAL[BYE];
    bye_msg->message_id = message_id;
    bye_msg->display_name = display_name;
}

void create_ping_msg(struct Ping_MSG *ping_msg, uint16_t message_id)
{
    ping_msg->type = MSG_TYPE_VAL[PING];
    ping_msg->message_id = message_id;
}

void free_confirm_msg(struct Confirm_MSG *msg) {
    free(msg);
}

void free_reply_msg(struct Reply_MSG *msg) {
    if (msg) {
        if (msg->message_contents) free(msg->message_contents);
        free(msg);
    }
}

void free_auth_msg(struct Auth_MSG *msg) {
    if (msg) {
        if (msg->username) free(msg->username);
        if (msg->display_name) free(msg->display_name);
        if (msg->secret) free(msg->secret);
        free(msg);
    }
}

void free_join_msg(struct Join_MSG *msg) {
    if (msg) {
        if (msg->channel_id) free(msg->channel_id);
        if (msg->display_name) free(msg->display_name);
        free(msg);
    }
}

void free_msg(struct MSG *msg) {
    if (msg) {
        if (msg->display_name) free(msg->display_name);
        if (msg->message_contents) free(msg->message_contents);
        free(msg);
    }
}

void free_err_msg(struct Err_MSG *msg) {
    if (msg) {
        if (msg->display_name) free(msg->display_name);
        if (msg->message_contents) free(msg->message_contents);
        free(msg);
    }
}

void free_bye_msg(struct Bye_MSG *msg) {
    if (msg) {
        if (msg->display_name) free(msg->display_name);
        free(msg);
    }
}

void free_ping_msg(struct Ping_MSG *msg) {
    free(msg);
}
