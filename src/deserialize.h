// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: deserialize.h
// February 2025

#ifndef DESERIALIZE_H__
#define DESERIALIZE_H__

#include <stdio.h>

#include "./messages.h"

// For testing using googletest
#ifdef __cplusplus
extern "C" {
#endif

struct Confirm_MSG *deserialize_confirm_msg(uint8_t *buffer, const size_t buffer_size);
struct Reply_MSG *deserialize_reply_msg(uint8_t *buffer, const size_t buffer_size);
struct Auth_MSG *deserialize_auth_msg(uint8_t *buffer, const size_t buffer_size);
struct Join_MSG *deserialize_join_msg(uint8_t *buffer, const size_t buffer_size);
struct MSG *deserialize_msg(uint8_t *buffer, const size_t buffer_size);
struct Err_MSG *deserialize_err_msg(uint8_t *buffer, const size_t buffer_size);
struct Bye_MSG *deserialize_bye_msg(uint8_t *buffer, const size_t buffer_size);
struct Ping_MSG *deserialize_ping_msg(uint8_t *buffer, const size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
