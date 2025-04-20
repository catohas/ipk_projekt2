// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: serialize.h
// February 2025

#ifndef SERIALIZE_H__
#define SERIALIZE_H__

#include <stdio.h>
#include <stdint.h>

#include "./messages.h"

// For testing using googletest
#ifdef __cplusplus
extern "C" {
#endif

uint8_t *serialize_confirm_msg(struct Confirm_MSG *confirm_msg, size_t *out_size);
uint8_t *serialize_auth_msg(struct Auth_MSG *auth_msg, size_t *out_size);
uint8_t *serialize_join_msg(struct Join_MSG *join_msg, size_t *out_size);
uint8_t *serialize_msg(struct MSG *msg, size_t *out_size);
uint8_t *serialize_err_msg(struct Err_MSG *err_msg, size_t *out_size);
uint8_t *serialize_bye_msg(struct Bye_MSG *bye_msg, size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif
