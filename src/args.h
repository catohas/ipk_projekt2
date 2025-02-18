// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: args.h
// February 2025

#ifndef ARGS_H__
#define ARGS_H__

#include <stdint.h>

void parse_args(int argc, char **argv, int *use_tcp_protocol, char **hostname, char **port, uint16_t *udp_timeout, uint8_t *udp_retransmissions);

#endif
