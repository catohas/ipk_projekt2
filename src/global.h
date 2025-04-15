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
extern int udp_listen_socket;
extern int udp_send_socket;
extern struct addrinfo hints, *servinfo, *p;

extern int use_tcp_protocol;
extern char *hostname;
extern char *port;
extern uint16_t udp_timeout;
extern uint8_t udp_retransmissions;

extern uint16_t *confirmed_msg_ids;
extern size_t confirmed_msg_ids_amount;

#endif
