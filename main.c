// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// February 2025

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

/*
struct Message
{
    uint16_t MessageID;
    uint8_t Username[20];
    uint8_t ChannelID[20];
    uint8_t Secret[128];
    uint8_t DisplayName[20];
    uint8_t MessageContent[60000];
};
*/

#define MAX_USERNAME_LEN 20
#define MAX_DISPLAY_NAME_LEN 20
#define MAX_SECRET_LEN 128

enum APP_STATE
{
    STATE_AUTH,
    STATE_JOIN,
    STATE_ERR,
    STATE_BYE,
    STATE_MSG,
    STATE_REPLY,
    STATE_NEG_REPLY,
};

enum MSG_TYPE
{
    CONFIRM,
    REPLY,
    AUTH,
    JOIN,
    MSG,
    PING,
    ERR,
    BYE,
};

static const uint8_t MSG_TYPE_VAL[] = {
    0x00,
    0x01,
    0x02,
    0x03,
    0x04,
    0xFD,
    0xFE,
    0xFF,
};

#pragma pack(1)
struct Auth_MSG
{
    uint8_t type;
    uint16_t message_id;
    char *username;
    char null1;
    char *display_name;
    char null2;
    char *secret;
    char null3;
};
#pragma pack()

uint8_t *serialize_auth_msg(struct Auth_MSG *auth_msg, size_t *out_size)
{
    *out_size = sizeof(auth_msg->type) + sizeof(auth_msg->message_id) +
        strlen(auth_msg->username) + strlen(auth_msg->display_name) + strlen(auth_msg->secret) + 3; // 3 null bytes as per spec

    uint8_t *buffer = malloc(*out_size);

    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    memcpy(buffer + offset, &auth_msg->type, sizeof(auth_msg->type));
    offset += sizeof(auth_msg->type);

    uint16_t net_msg_id = htons(auth_msg->message_id);
    memcpy(buffer + offset, &net_msg_id, sizeof(net_msg_id));
    offset += sizeof(net_msg_id);

    memcpy(buffer + offset, auth_msg->username, strlen(auth_msg->username));
    offset += strlen(auth_msg->username);

    buffer[offset] = '\0';
    offset++;

    memcpy(buffer + offset, auth_msg->display_name, strlen(auth_msg->display_name));
    offset += strlen(auth_msg->display_name);

    buffer[offset] = '\0';
    offset++;

    memcpy(buffer + offset, auth_msg->secret, strlen(auth_msg->secret));
    offset += strlen(auth_msg->secret);

    buffer[offset] = '\0';

    return buffer;
}

void create_auth_msg(struct Auth_MSG *auth_msg, char *username, char *display_name, char *secret)
{
    auth_msg->type = MSG_TYPE_VAL[AUTH];
    auth_msg->message_id = 0x0000;
    auth_msg->username = username; 
    auth_msg->null1 = '\0';
    auth_msg->display_name = display_name;
    auth_msg->null2 = '\0';
    auth_msg->secret = secret;
    auth_msg->null3 = '\0';
}

/*
static void tcp(void)
{
    printf("tcp\n");
}

static void udp(void)
{
    printf("udp\n");
}
*/

static void help(void)
{
    printf("useful help string\n");
}

static void parse_args(int argc, char **argv, char **port, char **hostname)
{
    for (int i = 0; i < argc; i++) {
        
        /*
        if (strcmp(argv[i], "-t") == 0) {
            i++;
            if (i >= argc) {
                printf("-t flag expects argument 'tcp' or 'udp'\n");
                exit(1);
            }
            else if (strcmp(argv[i], "tcp") == 0) {
                tcp();
            }
            else if (strcmp(argv[i], "udp") == 0) {
                udp();
            }
            else {
                printf("invalid -t flag argument, expected 'tcp' or 'udp'\n");
                exit(1);
            }

        }
        */

        if (strcmp(argv[i], "-s") == 0) {
            i++;
            if (i >= argc) {
                printf("-s flag expects argument of ip or hostname\n");
                exit(EXIT_FAILURE);
            }
            *hostname = argv[i];
        }

        if (strcmp(argv[i], "-p") == 0) {
            i++;
            if (i >= argc) {
                printf("-p flag expects argument of port number\n");
                exit(EXIT_FAILURE);
            }
            *port = argv[i];
            
            /*
            char *endptr = NULL;
            long converted_port = strtol(argv[i], &endptr, 10);

            if (strcmp(endptr, "\0") != 0) {
                printf("invalid port\n");
                exit(1);
            }

            *port = converted_port;

            */
        }

        /*
        if (strcmp(argv[i], "-d") == 0) {
            printf("too lazy rn\n");
            exit(1);
        }

        if (strcmp(argv[i], "-r") == 0) {
            printf("too lazy rn\n");
            exit(1);
        }
        */

        if (strcmp(argv[i], "-h") == 0) {
            help();
        }
    }
}

/* 
static void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    } else { 
        fprintf(stderr, "sa_family was not AF_INET for some reason\n");
        exit(1);
    }
}
*/

int main(int argc, char **argv)
{
    char *hostname = NULL;
    char *port = "4567";

    if (argc == 1) help();

    parse_args(argc, argv, &port, &hostname);

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    // char s[INET_ADDRSTRLEN];
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return EXIT_FAILURE;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        /*
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        */

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        return EXIT_FAILURE;
    }

    // struct Auth_MSG *packet = malloc(sizeof(struct Auth_MSG));
    struct Auth_MSG auth_msg;
    create_auth_msg(&auth_msg, "user1", "UDP_Man-1", "secret1");
    size_t buffer_size;
    uint8_t *buffer = serialize_auth_msg(&auth_msg, &buffer_size);

    printf("size of struct: %zu\n", buffer_size);

    // if ((numbytes = sendto(sockfd, "foobar", strlen("foobar"), 0,
    if ((numbytes = sendto(sockfd, buffer, buffer_size, 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("client: sendto");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo);

    printf("sent %d bytes to %s\n", numbytes, hostname);
    close(sockfd);

    return EXIT_SUCCESS;
}
