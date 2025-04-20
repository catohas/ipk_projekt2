// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: network.h
// April 2025

#ifndef NETWORK_H__
#define NETWORK_H__

#include "./global.h"

// UDP functions
void send_network_msg_udp(uint8_t *in_buffer, const size_t in_buffer_size);
void process_received_udp_message(unsigned char *buffer, int length);

// TCP functions
int establish_tcp_connection(void);
void close_tcp_connection(void);
void *tcp_listener(void *arg);
int start_tcp_listener(void);
void stop_tcp_listener(void);
void send_network_msg_tcp(const char *message);
void send_network_msg_tcp_wrapper(const char* type, void* data);
void cleanup_tcp(void);

void send_tcp_auth_msg(const char *username, const char *display_name, const char *secret);
void send_tcp_join_msg(const char *channel_id, const char *display_name);
void send_tcp_msg_msg(const char *display_name, const char *message_content);
void send_tcp_bye_msg(const char *display_name);
void send_tcp_err_msg(const char *display_name, const char *message_content);

#endif
