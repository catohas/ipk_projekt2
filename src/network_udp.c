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
#include <arpa/inet.h>

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
                    printf("ERROR: received reply message when should not have\n");
                    exit(EXIT_FAILURE);
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

            if (!is_message_duplicate(err_msg->message_id)) {
                printf("ERROR FROM %s: %s\n", err_msg->display_name, err_msg->message_contents);
            }

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

            // try to send confirm message
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

void *udp_listener(void *arg)
{
    (void)arg;

    printf_debug_simple(COLOR_SUCCESS, "starting udp listener...");

    struct addrinfo hints;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
        perror("listener socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udp_socket, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {
        perror("bind");
        close(udp_socket);
        return NULL;
    }

    unsigned char recv_buffer[MAX_MSG_SIZE];
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    struct sockaddr_storage their_addr;

    int server_dynamic_port = -1;
    bool has_changed_to_dyn_port = false;
    int sender_port = 0;
    
    while (true) {

        printf_debug_simple(COLOR_SUCCESS, "in listening loop...");

        int numbytes = recvfrom(udp_socket, recv_buffer, MAX_MSG_SIZE - 1, 0, (struct sockaddr *)&their_addr, &addr_len);
        if (numbytes == -1) {
            perror("recvfrom");
            continue;
        }

        char addr[INET_ADDRSTRLEN];
        // extract IP address and port from the sender
        if (their_addr.ss_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)&their_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, addr, sizeof(addr));
            sender_port = ntohs(ipv4->sin_port);
        }

        printf_debug(COLOR_INFO, "received %d bytes from %s:%d", numbytes, addr, sender_port);

        // change over to dynamic port on first reply msg
        if (recv_buffer[0] == 0x01 && !has_changed_to_dyn_port) {

            server_dynamic_port = sender_port;
            printf_debug(COLOR_INFO, "updating to server's dynamic port: %s", port);

            port = malloc(6); // "65535" + null byte
            if (port == NULL) {
                fprintf(stderr, "failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }
            snprintf(port, 6, "%d", server_dynamic_port);
            has_changed_to_dyn_port = true;
        }
        
        #ifdef DEBUG_PRINT
            for (int i = 0; i < numbytes; i++) {
                fprintf(stderr, "%02x", recv_buffer[i]);
            }
            fprintf(stderr, "\n");
            for (int i = 0; i < numbytes; i++) {
                fprintf(stderr, "%c", recv_buffer[i]);
            }
            fprintf(stderr, "\n");
        #endif

        recv_buffer[numbytes] = '\0';

        process_received_udp_message(recv_buffer, numbytes);


    }

    return NULL;
}
