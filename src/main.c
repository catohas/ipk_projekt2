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
int tcp_socket = -1;
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

pthread_t listener_thread;
bool listener_thread_running = false;

// ---------------------------------------------

static void graceful_quit(void)
{
    printf_debug_simple(COLOR_INFO, "starting graceful shutdown...");
    
    if (use_tcp_protocol) {
        if (tcp_socket != -1) {
            send_tcp_bye_msg(display_name);
        }
    }
    else {
        struct Bye_MSG bye_msg;
        create_bye_msg(&bye_msg, confirmed_msg_ids_index, display_name);
        
        size_t out_size;
        uint8_t *out_buffer = serialize_bye_msg(&bye_msg, &out_size);
        
        printf_debug_simple(COLOR_SUCCESS, "Sending BYE message to server");

        send_network_msg_udp(out_buffer, out_size);
    }
    
    exit(EXIT_SUCCESS);
}

// ctrl+c wrapper, ctrl+d calls graceful_quit() directly
static void handle_sigint(int sig)
{
    (void)sig;    
    graceful_quit();
}

static void cleanup(void)
{
    printf_debug_simple(COLOR_SUCCESS, "cleaning up...");

    if (use_tcp_protocol) {
        cleanup_tcp();
    }
    else {
        if (servinfo != NULL) {
            freeaddrinfo(servinfo);
        }

        if (udp_socket != -1) {
            close(udp_socket);
        }

        if (listener_thread_running) {
            pthread_cancel(listener_thread);
            pthread_join(listener_thread, NULL);
            listener_thread_running = false;
        }

        free(confirmed_msg_ids);
        free(seen_ids);
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

    if (use_tcp_protocol) { // tcp
        if (establish_tcp_connection() < 0) {
            fprintf(stderr, "Failed to establish TCP connection\n");
            exit(EXIT_FAILURE);
        }

        if (start_tcp_listener() < 0) {
            fprintf(stderr, "Failed to start TCP listener thread\n");
            exit(EXIT_FAILURE);
        }
    }
    else { // udp

        // initialize array that holds confirmed msg ids
        size_t con_msg_id_arr_size = sizeof(uint16_t)*confirmed_msg_array_size;
        confirmed_msg_ids = malloc(con_msg_id_arr_size);
        if (confirmed_msg_ids == NULL) {
            fprintf(stderr, "failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i < DEFAULT_MSG_CONFIRM_ARR_SIZE; i++) {
            confirmed_msg_ids[i] = -1; // default initialization, not ideal, underflows
        }

        // initialize array that holds already received msg ids, used for deduplication
        size_t seen_ids_size = sizeof(uint16_t)*seen_ids_array_size;
        seen_ids = malloc(seen_ids_size);
        if (seen_ids == NULL) {
            fprintf(stderr, "failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }

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

        // ctrl + d exit handling
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

        // find if user input matches command
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
            default:
                fprintf(stderr, "invalid app state\n");
                exit(EXIT_FAILURE);
        }

    }

}
