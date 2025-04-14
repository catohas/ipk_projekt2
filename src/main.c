// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: main.c
// February 2025

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./args.h"
#include "./commands.h"
#include "./debug.h"
#include "./defaults.h"
#include "./global.h"
#include "./network.h"
#include "./maximums.h"
#include "./messages.h"
#include "./state.h"
#include "./serialize.h"
#include "./deserialize.h"

const char *commands[] = {
    "/auth",
    "/join",
    "/rename",
    "/help"
};

void (*command_functions[])(void) = {
    cmd_auth,
    cmd_join,
    cmd_rename,
    cmd_help
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

enum APP_STATE state = STATE_START;

char *line = NULL;
int sockfd = -1;
struct addrinfo hints, *servinfo, *p;

int use_tcp_protocol = -1;
char *hostname = NULL;
char *port = DEFAULT_PORT;
uint16_t udp_timeout = DEFAULT_UDP_TIMEOUT;
uint8_t udp_retransmissions = DEFAULT_UDP_RETRANSMISSIONS;

uint16_t *confirmed_msg_ids;
size_t confirmed_msg_ids_amount = 0;

pthread_t listener_thread;
bool listener_thread_running = false;

// add a message ID to the confirmed messages array
void add_confirmed_msg_id(uint16_t msg_id)
{
    if (confirmed_msg_ids_amount >= DEFAULT_MSG_CONFIRM_ARR_SIZE) {
        // resize array or handle overflow
        uint16_t *new_array = realloc(confirmed_msg_ids, sizeof(uint16_t) * (DEFAULT_MSG_CONFIRM_ARR_SIZE * 2));
        if (new_array == NULL) {
            fprintf(stderr, "Failed to resize confirmed message IDs array\n");
            exit(EXIT_FAILURE);
        }
        confirmed_msg_ids = new_array;
    }
    
    confirmed_msg_ids[confirmed_msg_ids_amount++] = msg_id;
}

void *udp_listener(void *arg)
{
    (void)arg;

    int sockfd = -1;
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    // hints.ai_flags = AI_PASSIVE;
    
    int rv = getaddrinfo(hostname, port, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo);

    unsigned char recv_buffer[MAX_MSG_SIZE];
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    struct sockaddr_storage their_addr;
    
    while (true) {
        printf_debug_simple(COLOR_INFO, "starting udp listener...");
        int numbytes = recvfrom(sockfd, recv_buffer, MAX_MSG_SIZE - 1, 0, (struct sockaddr *)&their_addr, &addr_len);
        if (numbytes == -1) {
            perror("recvfrom");
            return NULL;
        }

        printf_debug(COLOR_INFO, "received %d bytes\n", numbytes);
        
        recv_buffer[numbytes] = '\0';
        
        // deserialization here
        printf("Received: %s\n", recv_buffer);
        
        // handle message confirmation here
        process_received_udp_message(recv_buffer, numbytes);
    }

    return NULL;
}

static void cleanup()
{
    printf_debug_simple(COLOR_INFO, "cleaning up...");

    // free(line);

    // if (listener_thread_running) {
    //     pthread_cancel(listener_thread);
    //     pthread_join(listener_thread, NULL);
    //     listener_thread_running = false;
    // }

    free(confirmed_msg_ids);
    freeaddrinfo(servinfo);
    if (sockfd != -1) {
        close(sockfd);
    }
    // exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    // Run exit function on ctrl + c
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = exit;
    sigaction(SIGINT, &sa, NULL);

    int reg_status = atexit(cleanup);
    if (reg_status != 0) {
        fprintf(stderr, "Failed to register cleanup function.\n");
        exit(EXIT_FAILURE);
    }

    if (argc == 1) {
        fprintf(stderr, "please provide a -t flag value, 'tcp' or 'udp'\n");
        fprintf(stderr, "please provide a -s flag value, hostname\n\n");
        cmd_help();
        exit(EXIT_SUCCESS);
    }

    parse_args(argc, argv, &use_tcp_protocol, &hostname, &port, &udp_timeout, &udp_retransmissions);

    if (use_tcp_protocol == -1) {
        fprintf(stderr, "please provide a -t flag value, 'tcp' or 'udp'\n");
        return EXIT_SUCCESS;
    }

    if (hostname == NULL) {
        fprintf(stderr, "please provide a -s flag value, hostname\n");
        return EXIT_SUCCESS;
    }

    confirmed_msg_ids = malloc(sizeof(uint16_t)*DEFAULT_MSG_CONFIRM_ARR_SIZE);
    if (confirmed_msg_ids == NULL) {
        fprintf(stderr, "failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    pthread_t listener_thread;
    if (use_tcp_protocol == 0) {  // if using UDP
        
        // memset(&hints, 0, sizeof hints);
        // hints.ai_family = AF_INET;
        // hints.ai_socktype = SOCK_DGRAM;
        
        // int rv = getaddrinfo(hostname, port, &hints, &servinfo);
        // if (rv != 0) {
        //     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        //     return EXIT_FAILURE;
        // }
        
        // for (p = servinfo; p != NULL; p = p->ai_next) {
        //     sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        //     if (sockfd == -1) {
        //         perror("socket");
        //         continue;
        //     }

        //     if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        //         close(sockfd);
        //         perror("bind");
        //         continue;
        //     }
        //     break;
        // }
        
        // if (p == NULL) {
        //     fprintf(stderr, "Failed to create socket\n");
        //     return EXIT_FAILURE;
        // }

        // freeaddrinfo();
        
        if (pthread_create(&listener_thread, NULL, udp_listener, NULL) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
        listener_thread_running = true;
        
        // detach thread so it cleans up itself when done
        pthread_detach(listener_thread);
    }

    // uint8_t recv_buffer[] = {0};
    // if (recvfrom(sockfd, recv_buffer, MAX_MSG_SIZE, 0, p->ai_addr, &(p->ai_addrlen)) == -1) {
    //     perror("recvfrom");
    //     continue;
    // }

    while (true) {
        size_t len;
        getline(&line, &len, stdin);

        line[strlen(line)-1] = '\0'; // overwrite newline with null char

        if (line[0] == '\0') continue; // when the user presses only enter
        
        bool command_selected = false;
        char *token = strtok(line, " ");
        cmd_ptr cmd_ptr;

        for (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
            if (strcmp(token, commands[i]) == 0) {
                cmd_ptr = *command_functions[i];
                command_selected = true;
                break;
            }
        }

        if (!command_selected) {
            cmd_ptr = cmd_msg;
        }

        switch (state) {
            case STATE_START:
                state_start_logic(cmd_ptr);
                break;
            case STATE_AUTH:
                // state_auth_logic(cmd_ptr);
                break;
            case STATE_OPEN:
                // state_open_logic(cmd_ptr);
                break;
            case STATE_JOIN:
                // state_join_logic(cmd_ptr);
                break;
            case STATE_END:
                // state_end_logic(cmd_ptr);
                break;
            default:
                fprintf(stderr, "invalid app state\n");
                exit(EXIT_FAILURE);
        }

    }

}
