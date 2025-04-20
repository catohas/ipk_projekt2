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
#include "./id.h"

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

// ----------------GLOBALS-------------------

enum APP_STATE state = STATE_START;

char *line = NULL;
char unprocessed_line[MAX_MESSAGE_CONTENT_LEN];

int use_tcp_protocol = -1;
int udp_socket = -1;
struct addrinfo hints, *servinfo, *p;

char *hostname = NULL;
char *port = DEFAULT_PORT;
char display_name[MAX_DISPLAY_NAME_LEN] = DEFAULT_DISPLAY_NAME;

uint16_t udp_timeout = DEFAULT_UDP_TIMEOUT;
uint8_t udp_retransmissions = DEFAULT_UDP_RETRANSMISSIONS;

uint16_t *confirmed_msg_ids;
size_t confirmed_msg_ids_index = 0;
size_t confirmed_msg_array_size = DEFAULT_MSG_CONFIRM_ARR_SIZE;

uint16_t *seen_ids;
size_t seen_count = 0;
size_t seen_ids_array_size = DEFAULT_SEEN_MSG_ARR_SIZE;

// ---------------------------------------------

pthread_t listener_thread;
bool listener_thread_running = false;

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
    
    // int rv = getaddrinfo(hostname, port, &hints, &servinfo);
    // if (rv != 0) {
    //     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    //     exit(EXIT_FAILURE);
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
    //     exit(EXIT_FAILURE);
    // }

    // freeaddrinfo(servinfo);

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

        // only process packets from the server
        char addr[INET_ADDRSTRLEN];

        // inet_ntop(AF_INET, &((struct sockaddr_in *)&their_addr)->sin_addr, addr, sizeof(addr));
        
        // extract IP address and port from the sender
        if (their_addr.ss_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)&their_addr;
            inet_ntop(AF_INET, &ipv4->sin_addr, addr, sizeof(addr));
            sender_port = ntohs(ipv4->sin_port);
        }

        printf_debug(COLOR_INFO, "received %d bytes from %s:%d", numbytes, addr, sender_port);

        // change over to dynamic port
        if (recv_buffer[0] == 0x01 && !has_changed_to_dyn_port) {
            if (sender_port != 0 && sender_port != atoi(port)) {
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
        }

        recv_buffer[numbytes] = '\0';

        process_received_udp_message(recv_buffer, numbytes);

        
        // check if the packet is from our server
        // if (strcmp(addr, hostname) == 0) {
            
        //     // change over to dynamic port
        //     if (recv_buffer[0] == 0x01 && !has_changed_to_dyn_port) {
        //         if (sender_port != 0 && sender_port != atoi(port)) {
        //             server_dynamic_port = sender_port;

        //             printf_debug(COLOR_INFO, "updating to server's dynamic port: %s", port);

        //             port = malloc(6); // "65535" + null byte
        //             if (port == NULL) {
        //                 fprintf(stderr, "failed to allocate memory\n");
        //                 exit(EXIT_FAILURE);
        //             }
        //             snprintf(port, 6, "%d", server_dynamic_port);
        //             has_changed_to_dyn_port = true;
        //         }
        //     }
            
        //     #ifdef DEBUG_PRINT
        //         for (int i = 0; i < numbytes; i++) {
        //             fprintf(stderr, "%02x", recv_buffer[i]);
        //         }
        //         fprintf(stderr, "\n");
        //         for (int i = 0; i < numbytes; i++) {
        //             fprintf(stderr, "%c", recv_buffer[i]);
        //         }
        //         fprintf(stderr, "\n");
        //     #endif

        //     recv_buffer[numbytes] = '\0';

        //     process_received_udp_message(recv_buffer, numbytes);

        // }
        // else {
        //     printf_debug(COLOR_INFO, "ignoring packet from unknown source: %s", addr);
        // }

    }

    return NULL;
}

static void graceful_quit(void)
{
    printf_debug_simple(COLOR_INFO, "starting graceful shutdown...");
    
    if (use_tcp_protocol == 0) {
        
        struct Bye_MSG bye_msg;
        create_bye_msg(&bye_msg, confirmed_msg_ids_index, display_name);
        
        size_t out_size;
        uint8_t *out_buffer = serialize_bye_msg(&bye_msg, &out_size);
        
        printf_debug_simple(COLOR_SUCCESS, "Sending BYE message to server");

        send_network_msg_udp(out_buffer, out_size);
    }
    
    exit(EXIT_SUCCESS);
}

static void handle_sigint(int sig)
{
    (void)sig;    
    graceful_quit();
}

static void cleanup(void)
{
    printf_debug_simple(COLOR_SUCCESS, "cleaning up...");

    if (servinfo != NULL) {
        freeaddrinfo(servinfo);
    }

    // if (listener_thread_running) {
    //     pthread_cancel(listener_thread);
    //     pthread_join(listener_thread, NULL);
    //     listener_thread_running = false;
    // }

    free(line);
    free(confirmed_msg_ids);
    free(seen_ids);

    if (udp_socket != -1) {
        close(udp_socket);
    }

}

int main(int argc, char **argv)
{
    // run exit function on ctrl + c
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa, NULL);

    int reg_status = atexit(cleanup);
    if (reg_status != 0) {
        fprintf(stderr, "failed to register cleanup function.\n");
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

    size_t con_msg_id_arr_size = sizeof(uint16_t)*confirmed_msg_array_size;
    confirmed_msg_ids = malloc(con_msg_id_arr_size);
    if (confirmed_msg_ids == NULL) {
        fprintf(stderr, "failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < DEFAULT_MSG_CONFIRM_ARR_SIZE; i++) {
        confirmed_msg_ids[i] = -1; // default initialization, not ideal, underflows
    }

    size_t seen_ids_size = sizeof(uint16_t)*seen_ids_array_size;
    seen_ids = malloc(seen_ids_size);
    if (seen_ids == NULL) {
        fprintf(stderr, "failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    pthread_t listener_thread;
    if (use_tcp_protocol == 0) { // UDP protocol
        
        if (pthread_create(&listener_thread, NULL, udp_listener, NULL) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
        listener_thread_running = true;
        
        pthread_detach(listener_thread);
    }

    while (true) {
        size_t len;
        ssize_t read = getline(&line, &len, stdin);

        // ctrl + d handling
        if (read == -1) {
            if (feof(stdin)) {
                printf_debug_simple(COLOR_INFO, "EOF detected (ctrl+d)");
                graceful_quit();
            }
            else {
                perror("getline");
                exit(EXIT_FAILURE);
            }
        }

        line[strlen(line)-1] = '\0'; // overwrite newline with null char

        if (line[0] == '\0') continue; // when the user presses only enter

        memset(unprocessed_line, '\0', MAX_MESSAGE_CONTENT_LEN);
        strncpy(unprocessed_line, line, MAX_MESSAGE_CONTENT_LEN-1);
        
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
                printf_debug_simple(COLOR_INFO, "executing start state routine");
                state_start_logic(cmd_ptr);
                break;
            case STATE_AUTH:
                printf_debug_simple(COLOR_INFO, "executing auth state routine");
                state_auth_logic(cmd_ptr);
                break;
            case STATE_OPEN:
                printf_debug_simple(COLOR_INFO, "executing open state routine");
                state_open_logic(cmd_ptr);
                break;
            case STATE_JOIN:
                printf_debug_simple(COLOR_INFO, "executing join state routine");
                state_join_logic(cmd_ptr);
                break;
            case STATE_END:
                printf_debug_simple(COLOR_INFO, "executing end state routine");
                // state_end_logic(cmd_ptr);
                break;
            default:
                fprintf(stderr, "invalid app state\n");
                exit(EXIT_FAILURE);
        }

    }

}
