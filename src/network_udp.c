#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <string.h>
#include <stdlib.h>
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
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        free(in_buffer);
        exit(EXIT_FAILURE);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            free(in_buffer);
            exit(EXIT_FAILURE);
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        free(in_buffer);
        exit(EXIT_FAILURE);
    }

    printf_debug(COLOR_INFO, "size of struct before sending: %zu bytes", in_buffer_size);

    // struct timeval timeout;
    // timeout.tv_sec = floor(udp_timeout/1000);
    // timeout.tv_usec = (udp_timeout % 1000)*1000; // msec to usec
    // if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    //     perror("setsockopt failed");
    //     free(in_buffer);
    //     exit(EXIT_FAILURE);
    // }

    int total_tries = 0;
    while (total_tries <= udp_retransmissions) {
        printf_debug(COLOR_INFO, "try number: (%d/%d)", total_tries, udp_retransmissions);
        total_tries++;
        if ((numbytes = sendto(sockfd, in_buffer, in_buffer_size, 0, p->ai_addr, p->ai_addrlen)) == -1) {
            perror("client: sendto");
            free(in_buffer);
            exit(EXIT_FAILURE);
        }
        else {
            printf_debug(COLOR_INFO, "sent %d bytes to %s", numbytes, hostname);

            struct timespec ts;
            ts.tv_sec = floor(udp_timeout/1000);
            ts.tv_nsec = (udp_timeout % 1000)*1000*1000; // msec to nsec
            nanosleep(&ts, NULL);


            // for (size_t i = 0; i < confirmed_msg_ids_amount; i++) {

            // }

            // check confirmed msg ids array
            // if id is not present wait timeout and repeat

            // uint8_t recv_buffer[MAX_MSG_SIZE] = {0};
            // if (recvfrom(sockfd, recv_buffer, MAX_MSG_SIZE, 0, p->ai_addr, &(p->ai_addrlen)) == -1) {
            //     perror("recvfrom");
            //     continue;
            // }

            // uint8_t *return_recv_buffer = malloc(sizeof(uint8_t)*MAX_MSG_SIZE);
            // if (return_recv_buffer == NULL) {
            //     perror("failed to allocate return buffer");
            //     free(in_buffer);
            //     exit(EXIT_FAILURE);
            // }
            // memcpy(return_recv_buffer, recv_buffer, MAX_MSG_SIZE);

            // close(sockfd);
            // free(in_buffer);

            // return;
        }
    }

    close(sockfd);
    free(in_buffer);
    // return NULL;
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
