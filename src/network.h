#ifndef NETWORK_H__
#define NETWORK_H__

#include "./global.h"

void send_network_msg_udp(uint8_t *in_buffer, const size_t in_buffer_size);
void process_received_udp_message(unsigned char *buffer, int length);

#endif
