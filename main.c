// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// 02-2025

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Default server port: 4567
 * Network protocols: IPv4
 * Transport protocols: TCP, UDP
 * Supported charset: us-ascii
 */

#ifdef DEBUG_PRINT
#define printf_debug(format, ...) fprintf(stderr, "%s:%-4d | %15s | " format "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define printf_debug(format, ...) (0)
#endif

struct Message {
    uint16_t MessageID;
    char Username[20];
    char ChannelID[20];
    char Secret[128];
    char DisplayName[20];
    char MessageContent[60000];
};

enum MSG_TYPE {
    AUTH,
    BYE,
    CONFIRM,
    ERR,
    JOIN,
    MSG,
    PING,
    REPLY
};

void tcp(void) {
    printf("tcp\n");
}

void udp(void) {
    printf("udp\n");
}

void ip(char *ip) {
    printf("ip: %s\n", ip);
}

void help(void) {
    printf("useful help string\n");
}

void port(long port) {
    printf("%ld\n", port);
}

int main(int argc, char **argv) {
    if (argc == 1) {
        help();
    }

    for (int i = 0; i < argc; i++) {

        if (strcmp(argv[i], "-t") == 0) {
            i++;
            if (i >= argc) {
                printf("-t flag expects argument 'tcp' or 'udp'\n");
                return 1;
            }
            else if (strcmp(argv[i], "tcp") == 0) {
                tcp();
            }
            else if (strcmp(argv[i], "udp") == 0) {
                udp();
            }
            else {
                printf("invalid -t flag argument, expected 'tcp' or 'udp'\n");
                return 1;
            }

        }

        if (strcmp(argv[i], "-s") == 0) {
            i++;
            if (i >= argc) {
                printf("-s flag expects argument of ip or hostname\n");
                return 1;
            }
            ip(argv[i]);
        }

        if (strcmp(argv[i], "-p") == 0) {
            i++;
            if (i >= argc) {
                printf("-p flag expects argument of port number\n");
                return 1;
            }
            
            char *endptr = NULL;
            long converted_port = strtol(argv[i], &endptr, 10);

            if (strcmp(endptr, "\0") != 0) {
                printf("invalid port\n");
                return 1;
            }

            port(converted_port);
        }

        if (strcmp(argv[i], "-d") == 0) {
            printf("too lazy rn\n");
            return 1;
        }

        if (strcmp(argv[i], "-r") == 0) {
            printf("too lazy rn\n");
            return 1;
        }

        if (strcmp(argv[i], "-h") == 0) {
            help();
        }
    }

    return 0;
}

