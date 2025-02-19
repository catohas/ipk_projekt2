// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: global.h
// February 2025

#ifndef GLOBAL_H__
#define GLOBAL_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "./state.h"

extern enum APP_STATE state;

extern char *line;
extern int sockfd;
extern struct addrinfo hints, *servinfo, *p;

extern int use_tcp_protocol;
extern char *hostname;
extern char *port;
extern uint16_t udp_timeout;
extern uint8_t udp_retransmissions;

#endif
