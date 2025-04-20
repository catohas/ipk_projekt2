// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: id.h
// April 2025

#ifndef ID_H__
#define ID_H__

#include <stdint.h>

void add_confirmed_msg_id(uint16_t msg_id);
bool is_message_duplicate(uint16_t msg_id);

#endif
