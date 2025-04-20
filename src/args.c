// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: args.c
// February 2025

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./args.h"
#include "./commands.h"

void parse_args(int argc, char **argv, int *use_tcp_protocol, char **hostname, char **port, uint16_t *udp_timeout, uint8_t *udp_retransmissions)
{
    for (int i = 1; i < argc; i++) {
        
        if (strcmp(argv[i], "-t") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "-t flag expects argument 'tcp' or 'udp'\n");
                exit(EXIT_SUCCESS);
            }
            else if (strcmp(argv[i], "tcp") == 0) {
                *use_tcp_protocol = 1;
            }
            else if (strcmp(argv[i], "udp") == 0) {
                *use_tcp_protocol = 0;
            }
            else {
                fprintf(stderr, "invalid -t flag argument, expected 'tcp' or 'udp'\n");
                exit(EXIT_SUCCESS);
            }
        }

        if (strcmp(argv[i], "-s") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "-s flag expects argument of ip or hostname\n");
                exit(EXIT_SUCCESS);
            }
            *hostname = argv[i];
        }

        if (strcmp(argv[i], "-p") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "-p flag expects argument of port number\n");
                exit(EXIT_SUCCESS);
            }
            *port = argv[i];
        }

        if (strcmp(argv[i], "-d") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "-d flag expects argument of udp confirmation timeout\n");
                exit(EXIT_SUCCESS);
            }
            
            char *endptr = NULL;
            long converted_timeout = strtol(argv[i], &endptr, 10);

            if (strcmp(endptr, "\0") != 0) {
                fprintf(stderr, "invalid udp confirmation timeout value\n");
                exit(EXIT_SUCCESS);
            }

            *udp_timeout = (uint16_t)converted_timeout;
        }

        if (strcmp(argv[i], "-r") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "-r flag expects argument of max number of udp retransmissions\n");
                exit(EXIT_SUCCESS);
            }

            char *endptr = NULL;
            long converted_retrans = strtol(argv[i], &endptr, 10);

            if (strcmp(endptr, "\0") != 0) {
                fprintf(stderr, "invalid udp retransmissions value\n");
                exit(EXIT_SUCCESS);
            }

            *udp_retransmissions = (uint8_t)converted_retrans;
        }

        if (strcmp(argv[i], "-h") == 0) {
            cmd_help();
            exit(EXIT_SUCCESS);
        }
    }
}
