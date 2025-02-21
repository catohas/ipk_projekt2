// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: deserialize.h
// February 2025

#ifndef DESERIALIZE_H__
#define DESERIALIZE_H__

#include <stdio.h>

#include "./messages.h"

struct Confirm_MSG *deserialize_confirm_msg(const uint8_t *buffer, size_t buffer_size);
struct Reply_MSG *deserialize_reply_msg(const uint8_t *buffer, size_t buffer_size);
struct Auth_MSG *deserialize_auth_msg(const uint8_t *buffer, size_t buffer_size);

void free_confirm_msg(struct Confirm_MSG *msg);
void free_reply_msg(struct Reply_MSG *msg);

#endif
