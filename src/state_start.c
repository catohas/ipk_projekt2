#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "./commands.h"
#include "./debug.h"
#include "./global.h"
#include "./state.h"
#include "./maximums.h"

// void state_start_logic(cmd_ptr cmd) {
//     return;
// }

void state_start_logic(cmd_ptr cmd)
{
    // int rv;
    // // int numbytes;

    // memset(&hints, 0, sizeof hints);
    // hints.ai_family = AF_INET;
    // hints.ai_socktype = SOCK_DGRAM;
    
    // if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
    //     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    //     exit(EXIT_FAILURE);
    // }

    // for (p = servinfo; p != NULL; p = p->ai_next) {

    //     sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    //     fcntl(sockfd, F_SETFD, O_NONBLOCK);

    //     if (sockfd == -1) {
    //         perror("client: socket");
    //         exit(EXIT_FAILURE);
    //     }

    //     break;
    // }

    // if (p == NULL) {
    //     fprintf(stderr, "client: failed to create socket\n");
    //     exit(EXIT_FAILURE);
    // }

    // struct timeval timeout;
    // timeout.tv_sec = 1;
    // timeout.tv_usec = 0;
    // if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    //     perror("setsockopt failed");
    //     exit(EXIT_FAILURE);
    // }

    // while (true) {

        // uint8_t recv_buffer[MAX_MSG_SIZE] = {0};
        // if (recvfrom(sockfd, recv_buffer, MAX_MSG_SIZE, 0, p->ai_addr, &(p->ai_addrlen)) == -1) {
        //     // perror("recvfrom");
        //     continue;
        // }

        if (cmd == cmd_auth){
            cmd();
            return;
        }
        else if (cmd == cmd_join) {
            printf("ERROR: trying to join channel while not authenticated\n");
        }
        else if (cmd == cmd_rename) {
            printf("ERROR: trying to rename while not authenticated\n");
        }
        else if (cmd == cmd_help) {
            cmd();
        }
        else if (cmd == cmd_msg) {
            printf("ERROR: trying to send message while not authenticated\n");
        }
        else {
            printf("ERROR: unknown input\n");
        }

    // }

}

void handle_confirm_msg_state_start(unsigned char *buffer, int length)
{
    
}

void handle_reply_msg_state_start(unsigned char *buffer, int length)
{

}

void handle_auth_msg_state_start(unsigned char *buffer, int length)
{

}

void handle_join_msg_state_start(unsigned char *buffer, int length)
{

}

void handle_msg_state_start(unsigned char *buffer, int length)
{

}

void handle_ping_msg_state_start(unsigned char *buffer, int length)
{

}

void handle_err_msg_state_start(unsigned char *buffer, int length)
{

}

void handle_bye_msg_state_start(unsigned char *buffer, int length)
{

}
