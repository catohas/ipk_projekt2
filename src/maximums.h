// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: maximums.h
// February 2025

#ifndef MAXIMUMS_H__
#define MAXIMUMS_H__

#define MAX_USERNAME_LEN 20
#define MAX_CHANNEL_ID_LEN 20
#define MAX_SECRET_LEN 128
#define MAX_DISPLAY_NAME_LEN 20
#define MAX_MESSAGE_CONTENT_LEN 60000

// The sum of all Reply_MSG struct members, it is the biggest struct possible to be sent
#define MAX_MSG_SIZE (sizeof(uint8_t)*MAX_MESSAGE_CONTENT_LEN + sizeof(uint8_t)*2 + sizeof(uint16_t)*2 + 1) // 1 null byte

#endif
