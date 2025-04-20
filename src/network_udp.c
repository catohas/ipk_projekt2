// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: network_udp.c
// April 2025

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
#include "./deserialize.h"
#include "./state.h"
#include "./id.h"

void send_network_msg_udp(uint8_t *in_buffer, const size_t in_buffer_size)
{
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

    if (in_buffer[0] == 0x00) { // do not resend confirm messages

        printf_debug_simple(COLOR_INFO, "sending confirm message, skipping confirm mechanism");
        if (sendto(udp_socket, in_buffer, in_buffer_size, 0, 
                  servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
            perror("sendto");
            return;
        }
        printf_debug(COLOR_INFO, "sent %d bytes to server", (int)in_buffer_size);
        return;
    }

    // extract message ID from the buffer
    uint16_t msg_id = (in_buffer[1] << 8) | in_buffer[2];

    // move to next message, current message already has previous id
    confirmed_msg_ids_index++;
    
    int total_tries = 0;
    while (total_tries <= udp_retransmissions) {
        total_tries++;
        
        if (sendto(udp_socket, in_buffer, in_buffer_size, 0, 
                  servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
            perror("sendto");
            continue;
        }
        
        printf_debug(COLOR_INFO, "sent %d bytes to server (try %d/%d)", (int)in_buffer_size, total_tries-1, udp_retransmissions);

        #ifdef DEBUG_PRINT
            if (total_tries == 1) {
                for (size_t i = 0; i < in_buffer_size; i++) {
                    fprintf(stderr, "%02x", in_buffer[i]);
                }
                fprintf(stderr, "\n");
                for (size_t i = 0; i < in_buffer_size; i++) {
                    fprintf(stderr, "%c", in_buffer[i]);
                }
                fprintf(stderr, "\n");
            }
        #endif

        struct timespec ts;
        ts.tv_sec = floor(udp_timeout/1000);
        ts.tv_nsec = (udp_timeout % 1000)*1000*1000;
        nanosleep(&ts, NULL);
        
        // print msg ids in confirm array
        #ifdef DEBUG_PRINT
            if (total_tries == 1) {
                for (size_t i = 0; i < confirmed_msg_ids_index+3; i++) {
                    printf_debug(COLOR_INFO, "msg id in arr: %d", confirmed_msg_ids[i]);
                }
            }
        #endif
        
        bool confirmed = false;
        for (size_t i = 0; i < confirmed_msg_ids_index; i++) {
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
    for (size_t i = 0; i < confirmed_msg_ids_index; i++) {
        if (confirmed_msg_ids[i] == msg_id) {
            confirmed = true;
            break;
        }
    }
    
    if (!confirmed) {
        printf_debug_simple(COLOR_ERR, "message was not confirmed shutting down...");
        printf("ERROR: message with id '%d' was not confirmed\n", msg_id);
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(servinfo);
    free(in_buffer);
}

void process_received_udp_message(unsigned char *buffer, int length)
{
    // check msg type
    switch (buffer[0]) {
        case 0x00: { // confirm
            printf_debug_simple(COLOR_SUCCESS, "handling confirm message");
            struct Confirm_MSG *con_msg = deserialize_confirm_msg(buffer, length);
            uint16_t id = con_msg->ref_message_id;
            add_confirmed_msg_id(id);
            free_confirm_msg(con_msg);
            break;
        }
        case 0x01: { // reply
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
            }
            break;
        }
        case 0x02: { // auth
            printf("ERROR: Received auth message from server, that should not happen\n");
            exit(EXIT_FAILURE);
        }
        case 0x03: { // join
            printf("ERROR: Received join message from server, that should not happen\n");
            exit(EXIT_FAILURE);
            break;
        }
        case 0x04: { // msg
            switch (state) {
                case STATE_AUTH:
                case STATE_OPEN:
                case STATE_JOIN:
                    printf_debug_simple(COLOR_SUCCESS, "handling normal message");

                    struct MSG *msg = deserialize_msg(buffer, length);

                    if (!is_message_duplicate(msg->message_id)) {
                        printf("%s: %s\n", msg->display_name, msg->message_contents);
                    }
                    
                    struct Confirm_MSG con_msg;
                    create_confirm_msg(&con_msg, msg->message_id);
                    size_t out_size;
                    uint8_t *out_buffer = serialize_confirm_msg(&con_msg, &out_size);
                    send_network_msg_udp(out_buffer, out_size);

                    free_msg(msg);
                    free(out_buffer);

                    break;
                case STATE_START:
                    printf("ERROR: received msg when should not have\n");
                    break;
            }
            break;
        }
        case 0xFD: { // ping
            printf_debug_simple(COLOR_INFO, "got ping message, processing...");

            struct Ping_MSG *ping_msg = deserialize_ping_msg(buffer, length);

            struct Confirm_MSG con_msg;
            create_confirm_msg(&con_msg, ping_msg->message_id);

            size_t out_size;
            uint8_t *out_buffer = serialize_confirm_msg(&con_msg, &out_size);
            send_network_msg_udp(out_buffer, out_size);
            
            free_ping_msg(ping_msg);
            free(out_buffer);

            break;
        }
        case 0xFE: { // err
            printf_debug_simple(COLOR_INFO, "got err message, processing...");

            struct Err_MSG *err_msg = deserialize_err_msg(buffer, length);

            printf("ERROR FROM %s: %s\n", err_msg->display_name, err_msg->message_contents);

            struct Confirm_MSG con_msg;
            create_confirm_msg(&con_msg, err_msg->message_id);

            size_t out_size;
            uint8_t *out_buffer = serialize_confirm_msg(&con_msg, &out_size);
            send_network_msg_udp(out_buffer, out_size);

            free_err_msg(err_msg);
            free(out_buffer);

            exit(EXIT_SUCCESS);

            break;
        }
        case 0xFF: { // bye
            printf_debug_simple(COLOR_INFO, "got bye message, processing...");

            struct Bye_MSG *bye_msg = deserialize_bye_msg(buffer, length);

            struct Confirm_MSG con_msg;
            create_confirm_msg(&con_msg, bye_msg->message_id);

            size_t out_size;
            uint8_t *out_buffer = serialize_confirm_msg(&con_msg, &out_size);
            send_network_msg_udp(out_buffer, out_size);

            free_bye_msg(bye_msg);
            free(out_buffer);

            exit(EXIT_SUCCESS);

            break;
        }
        default: {
            printf_debug_simple(COLOR_ERR, "received malformed message, aborting...");
            printf("ERROR: received malformed message from server\n");

            // try to send confirm message even though we received an unknown message
            uint16_t msg_id = (buffer[1] << 8) | buffer[2];
            struct Confirm_MSG con_msg;
            create_confirm_msg(&con_msg, msg_id);
            size_t buffer_size;
            uint8_t *in_buffer = serialize_confirm_msg(&con_msg, &buffer_size);

            send_network_msg_udp(in_buffer, buffer_size);

            free(in_buffer);

            struct Err_MSG err_msg;
            char err_string[] = "received malformed message from server";
            create_err_msg(&err_msg, confirmed_msg_ids_index, display_name, err_string);
            in_buffer = serialize_err_msg(&err_msg, &buffer_size);

            send_network_msg_udp(in_buffer, buffer_size);
            
            free(in_buffer);
            
            exit(EXIT_FAILURE);

            break;
        }

    }
}
