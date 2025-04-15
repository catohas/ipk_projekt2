#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "./debug.h"
#include "./global.h"
#include "./maximums.h"
#include "./messages.h"
#include "./network.h"
#include "./serialize.h"
#include "./state.h"

void send_network_msg_udp(uint8_t *in_buffer, const size_t in_buffer_size)
{
    // extract message ID from the buffer
    uint16_t msg_id = (in_buffer[1] << 8) | in_buffer[2];
    
    // create a separate socket for sending messages
    if (udp_send_socket == -1) {
        udp_send_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_send_socket == -1) {
            perror("send socket creation failed");
            free(in_buffer);
            exit(EXIT_FAILURE);
        }
    }
    
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    int rv = getaddrinfo(hostname, port, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        free(in_buffer);
        exit(EXIT_FAILURE);
    }
    
    int total_tries = 0;
    while (total_tries <= udp_retransmissions) {
        total_tries++;
        
        if (sendto(udp_send_socket, in_buffer, in_buffer_size, 0, 
                  servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
            perror("sendto");
            continue;
        }
        
        printf_debug(COLOR_INFO, "sent %d bytes to server (try %d/%d)", (int)in_buffer_size, total_tries-1, udp_retransmissions);

        if (total_tries == 1) {
            for (size_t i = 0; i < in_buffer_size; i++) {
                printf("%02x", in_buffer[i]);
            }
            printf("\n");
            for (size_t i = 0; i < in_buffer_size; i++) {
                printf("%c", in_buffer[i]);
            }
            printf("\n");
        }

        struct timespec ts;
        ts.tv_sec = floor(udp_timeout/1000);
        ts.tv_nsec = (udp_timeout % 1000)*1000*1000;
        nanosleep(&ts, NULL);
        
        bool confirmed = false;
        for (size_t i = 0; i < confirmed_msg_ids_amount; i++) {
            if (confirmed_msg_ids[i] == msg_id) {
                confirmed = true;
                break;
            }
        }
        
        if (confirmed) {
            printf_debug(COLOR_INFO, "message ID %d confirmed", msg_id);
            break;
        }
    }

    bool confirmed = false;
    for (size_t i = 0; i < confirmed_msg_ids_amount; i++) {
        if (confirmed_msg_ids[i] == msg_id) {
            confirmed = true;
            break;
        }
    }
    
    if (!confirmed) {
        printf_debug_simple(COLOR_ERR, "message was not confirmed, would shut down...");
    }
    
    freeaddrinfo(servinfo);
    free(in_buffer);
}

void process_received_udp_message(unsigned char *buffer, int length)
{
    // check msg type
    switch (buffer[0]) {
        // confirm
        case 0x00:
            switch (state) {
                case STATE_START:
                    handle_confirm_msg_state_start(buffer, length);
                    break;
                case STATE_AUTH:
                    handle_confirm_msg_state_auth(buffer, length);
                    break;
                case STATE_OPEN:
                    handle_confirm_msg_state_open(buffer, length);
                    break;
                case STATE_JOIN:
                    handle_confirm_msg_state_join(buffer, length);
                    break;
                case STATE_END:
                    handle_confirm_msg_state_end(buffer, length);
                    break;
            }
            break;
        // reply
        case 0x01:
            switch (state) {
                case STATE_START:
                    handle_reply_msg_state_start(buffer, length);
                    break;
                case STATE_AUTH:
                    handle_reply_msg_state_auth(buffer, length);
                    break;
                case STATE_OPEN:
                    handle_reply_msg_state_open(buffer, length);
                    break;
                case STATE_JOIN:
                    handle_reply_msg_state_join(buffer, length);
                    break;
                case STATE_END:
                    handle_reply_msg_state_end(buffer, length);
                    break;
            }
            break;
        // auth
        case 0x02:
            switch (state) {
                case STATE_START:
                    handle_auth_msg_state_start(buffer, length);
                    break;
                case STATE_AUTH:
                    handle_auth_msg_state_auth(buffer, length);
                    break;
                case STATE_OPEN:
                    handle_auth_msg_state_open(buffer, length);
                    break;
                case STATE_JOIN:
                    handle_auth_msg_state_join(buffer, length);
                    break;
                case STATE_END:
                    handle_auth_msg_state_end(buffer, length);
                    break;
            }
            break;
        // join
        case 0x03:
            switch (state) {
                case STATE_START:
                    handle_join_msg_state_start(buffer, length);
                    break;
                case STATE_AUTH:
                    handle_join_msg_state_auth(buffer, length);
                    break;
                case STATE_OPEN:
                    handle_join_msg_state_open(buffer, length);
                    break;
                case STATE_JOIN:
                    handle_join_msg_state_join(buffer, length);
                    break;
                case STATE_END:
                    handle_join_msg_state_end(buffer, length);
                    break;
            }
            break;
        // msg
        case 0x04:
            switch (state) {
                case STATE_START:
                    handle_msg_state_start(buffer, length);
                    break;
                case STATE_AUTH:
                    handle_msg_state_auth(buffer, length);
                    break;
                case STATE_OPEN:
                    handle_msg_state_open(buffer, length);
                    break;
                case STATE_JOIN:
                    handle_msg_state_join(buffer, length);
                    break;
                case STATE_END:
                    handle_msg_state_end(buffer, length);
                    break;
            }
            break;
        // ping
        case 0xFD:
            switch (state) {
                case STATE_START:
                    handle_ping_msg_state_start(buffer, length);
                    break;
                case STATE_AUTH:
                    handle_ping_msg_state_auth(buffer, length);
                    break;
                case STATE_OPEN:
                    handle_ping_msg_state_open(buffer, length);
                    break;
                case STATE_JOIN:
                    handle_ping_msg_state_join(buffer, length);
                    break;
                case STATE_END:
                    handle_ping_msg_state_end(buffer, length);
                    break;
            }
            break;
        // err
        case 0xFE:
            switch (state) {
                case STATE_START:
                    handle_err_msg_state_start(buffer, length);
                    break;
                case STATE_AUTH:
                    handle_err_msg_state_auth(buffer, length);
                    break;
                case STATE_OPEN:
                    handle_err_msg_state_open(buffer, length);
                    break;
                case STATE_JOIN:
                    handle_err_msg_state_join(buffer, length);
                    break;
                case STATE_END:
                    handle_err_msg_state_end(buffer, length);
                    break;
            }
            break;
        // bye
        case 0xFF:
            switch (state) {
                case STATE_START:
                    handle_bye_msg_state_start(buffer, length);
                    break;
                case STATE_AUTH:
                    handle_bye_msg_state_auth(buffer, length);
                    break;
                case STATE_OPEN:
                    handle_bye_msg_state_open(buffer, length);
                    break;
                case STATE_JOIN:
                    handle_bye_msg_state_join(buffer, length);
                    break;
                case STATE_END:
                    handle_bye_msg_state_end(buffer, length);
                    break;
            }
            break;
        
        default:
            // unknown message, refer to client exception handling section
            break;
    }
}
