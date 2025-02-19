// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: main.c
// February 2025

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./args.h"
#include "./commands.h"
#include "./debug.h"
#include "./global.h"
#include "./maximums.h"
#include "./messages.h"
#include "./state.h"
#include "./serialize.h"
#include "./deserialize.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

enum APP_STATE state = STATE_AUTH;

char *line = NULL;
int sockfd = -1;
struct addrinfo hints, *servinfo, *p;

int use_tcp_protocol = -1;
char *hostname = NULL;
char *port = "4567";
uint16_t udp_timeout = 250;
uint8_t udp_retransmissions = 3;

static void cleanup()
{
    printf_debug_simple(COLOR_INFO, "cleaning up...");
    // free(line);
    freeaddrinfo(servinfo);
    if (sockfd != -1) {
        close(sockfd);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, exit);

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

    while (1) {
        size_t len;
        getline(&line, &len, stdin);

        line[strlen(line)-1] = '\0'; // overwrite newline with null char

        if (line[0] == '\0') continue; // when the user presses only enter

        char *token = strtok(line, " ");

        for (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
            if (strcmp(token, commands[i]) == 0) {
                (*command_functions[i])();
            }
        }
    }

}
