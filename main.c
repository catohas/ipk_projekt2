// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// February 2025

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <regex.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static void help_cmd(void);

/*
 * Default server port: 4567
 * Network protocols: IPv4
 * Transport protocols: TCP, UDP
 * Supported charset: us-ascii
 */

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

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
#define MAX_CHANNEL_ID_LEN 20
#define MAX_SECRET_LEN 128
#define MAX_DISPLAY_NAME_LEN 20
#define MAX_MESSAGE_CONTENT_LEN 60000

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
    char *display_name;
    char *secret;
};

struct Confirm_MSG
{
    uint8_t type;
    uint16_t ref_message_id;
};

struct Reply_MSG
{
    uint8_t type;
    uint16_t message_id;
    uint8_t result;
    uint16_t ref_message_id;
    char *message_contents;
};

struct Join_MSG
{
    uint8_t type;
    uint16_t message_id;
    char *channel_id;
    char *display_name;
};

struct MSG
{
    uint8_t type;
    uint16_t message_id;
    char *display_name;
    char *message_contents;
};

struct Err_MSG
{
    uint8_t type;
    uint16_t message_id;
    char *display_name;
    char *message_contents;
};

struct Bye_MSG
{
    uint8_t type;
    uint16_t message_id;
    char *display_name;
};

struct Ping_MSG
{
    uint8_t type;
    uint16_t message_id;
};

#pragma pack()

/*
 *    1 byte       2 bytes      
 *  +--------+--------+--------+
 *  |  0x00  |  Ref_MessageID  |
 *  +--------+--------+--------+
*/
uint8_t *serialize_confirm_msg(struct Confirm_MSG *confirm_msg, size_t *out_size)
{
    *out_size = sizeof(confirm_msg->type) + sizeof(confirm_msg->ref_message_id);

    uint8_t *buffer = malloc(*out_size);

    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    memcpy(buffer + offset, &confirm_msg->type, sizeof(confirm_msg->type));
    offset += sizeof(confirm_msg->type);

    uint16_t net_ref_message_id = htons(confirm_msg->ref_message_id);
    memcpy(buffer + offset, &net_ref_message_id, sizeof(net_ref_message_id));

    return buffer;
}

/*
 *    1 byte       2 bytes       1 byte       2 bytes      
 *  +--------+--------+--------+--------+--------+--------+--------~~---------+---+
 *  |  0x01  |    MessageID    | Result |  Ref_MessageID  |  MessageContents  | 0 |
 *  +--------+--------+--------+--------+--------+--------+--------~~---------+---+
 * 
*/
uint8_t *serialize_reply_msg(struct Reply_MSG *reply_msg, size_t *out_size)
{
    *out_size = sizeof(reply_msg->type) + sizeof(reply_msg->message_id) +
        sizeof(reply_msg->result) + sizeof(reply_msg->ref_message_id) +
        strlen(reply_msg->message_contents) + 1; // 1 null byte
    
    uint8_t *buffer = malloc(*out_size);

    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    memcpy(buffer + offset, &reply_msg->type, sizeof(reply_msg->type));
    offset += sizeof(reply_msg->type);

    uint16_t net_message_id = htons(reply_msg->message_id);
    memcpy(buffer + offset, &net_message_id, sizeof(net_message_id));
    offset += sizeof(net_message_id);

    memcpy(buffer + offset, &reply_msg->result, sizeof(reply_msg->result));
    offset += sizeof(reply_msg->result);

    uint16_t net_ref_message_id = htons(reply_msg->ref_message_id);
    memcpy(buffer + offset, &net_ref_message_id, sizeof(net_ref_message_id));
    offset += sizeof(net_ref_message_id);

    memcpy(buffer + offset, reply_msg->message_contents, strlen(reply_msg->message_contents));
    offset += strlen(reply_msg->message_contents);

    buffer[offset] = '\0';

    return buffer;
}

/*
 *   1 byte       2 bytes      
 *  +--------+--------+--------+-----~~-----+---+-------~~------+---+----~~----+---+
 *  |  0x02  |    MessageID    |  Username  | 0 |  DisplayName  | 0 |  Secret  | 0 |
 *  +--------+--------+--------+-----~~-----+---+-------~~------+---+----~~----+---+
*/
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

    uint16_t net_message_id = htons(auth_msg->message_id);
    memcpy(buffer + offset, &net_message_id, sizeof(net_message_id));
    offset += sizeof(net_message_id);

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

/*
 *    1 byte       2 bytes      
 *  +--------+--------+--------+-----~~-----+---+-------~~------+---+
 *  |  0x03  |    MessageID    |  ChannelID | 0 |  DisplayName  | 0 |
 *  +--------+--------+--------+-----~~-----+---+-------~~------+---+
*/
uint8_t *serialize_join_msg(struct Join_MSG *join_msg, size_t *out_size)
{
    *out_size = sizeof(join_msg->type) + sizeof(join_msg->message_id) + 
        strlen(join_msg->channel_id) + strlen(join_msg->display_name) + 2; // 2 null bytes

    uint8_t *buffer = malloc(*out_size);

    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    memcpy(buffer + offset, &join_msg->type, sizeof(join_msg->type));
    offset += sizeof(join_msg->type);

    uint16_t net_message_id = htons(join_msg->message_id);
    memcpy(buffer + offset, &net_message_id, sizeof(net_message_id));
    offset += sizeof(net_message_id);

    memcpy(buffer + offset, join_msg->channel_id, strlen(join_msg->channel_id));
    offset += strlen(join_msg->channel_id);

    buffer[offset] = '\0';
    offset++;

    memcpy(buffer + offset, join_msg->display_name, strlen(join_msg->display_name));
    offset += strlen(join_msg->display_name);

    buffer[offset] = '\0';

    return buffer;
}

/*
 *    1 byte       2 bytes      
 *  +--------+--------+--------+-------~~------+---+--------~~---------+---+
 *  |  0x04  |    MessageID    |  DisplayName  | 0 |  MessageContents  | 0 |
 *  +--------+--------+--------+-------~~------+---+--------~~---------+---+
*/
uint8_t *serialize_msg(struct MSG *msg, size_t *out_size)
{
    *out_size = sizeof(msg->type) + sizeof(msg->message_id) +
        strlen(msg->display_name) + strlen(msg->message_contents) + 2; // 2 null bytes

    uint8_t *buffer = malloc(*out_size);

    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    memcpy(buffer + offset, &msg->type, sizeof(msg->type));
    offset += sizeof(msg->type);

    uint16_t net_message_id = htons(msg->message_id);
    memcpy(buffer + offset, &net_message_id, sizeof(net_message_id));
    offset += sizeof(net_message_id);

    memcpy(buffer + offset, msg->display_name, strlen(msg->display_name));
    offset += strlen(msg->display_name);

    buffer[offset] = '\0';
    offset++;

    memcpy(buffer + offset, msg->message_contents, strlen(msg->message_contents));
    offset += strlen(msg->message_contents);

    buffer[offset] = '\0';

    return buffer;
}

/*
 *    1 byte       2 bytes
 *  +--------+--------+--------+-------~~------+---+--------~~---------+---+
 *  |  0xFE  |    MessageID    |  DisplayName  | 0 |  MessageContents  | 0 |
 *  +--------+--------+--------+-------~~------+---+--------~~---------+---+
*/
uint8_t *serialize_err_msg(struct Err_MSG *err_msg, size_t *out_size)
{
    *out_size = sizeof(err_msg->type) + sizeof(err_msg->message_id) +
        strlen(err_msg->display_name) + strlen(err_msg->message_contents) + 2; // 2 null bytes

    uint8_t *buffer = malloc(*out_size);

    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    memcpy(buffer + offset, &err_msg->type, sizeof(err_msg->type));
    offset += sizeof(err_msg->type);

    uint16_t net_message_id = htons(err_msg->message_id);
    memcpy(buffer + offset, &net_message_id, sizeof(net_message_id));
    offset += sizeof(net_message_id);

    memcpy(buffer + offset, err_msg->display_name, strlen(err_msg->display_name));
    offset += strlen(err_msg->display_name);

    buffer[offset] = '\0';
    offset++;

    memcpy(buffer + offset, err_msg->message_contents, strlen(err_msg->message_contents));
    offset += strlen(err_msg->message_contents);

    buffer[offset] = '\0';

    return buffer;
}

/*
 *    1 byte       2 bytes
 *  +--------+--------+--------+-------~~------+---+
 *  |  0xFF  |    MessageID    |  DisplayName  | 0 |
 *  +--------+--------+--------+-------~~------+---+
*/
uint8_t *serialize_bye_msg(struct Bye_MSG *bye_msg, size_t *out_size)
{
    *out_size = sizeof(bye_msg->type) + sizeof(bye_msg->message_id) +
        strlen(bye_msg->display_name) + 1; // 1 null byte

    uint8_t *buffer = malloc(*out_size);

    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    memcpy(buffer + offset, &bye_msg->type, sizeof(bye_msg->type));
    offset += sizeof(bye_msg->type);

    uint16_t net_message_id = htons(bye_msg->message_id);
    memcpy(buffer + offset, &net_message_id, sizeof(net_message_id));
    offset += sizeof(net_message_id);

    memcpy(buffer + offset, bye_msg->display_name, strlen(bye_msg->display_name));
    offset += strlen(bye_msg->display_name);

    buffer[offset] = '\0';

    return buffer;
}

/*
 *    1 byte       2 bytes
 *  +--------+--------+--------+
 *  |  0xFD  |    MessageID    |
 *  +--------+--------+--------+
*/
uint8_t *serialize_ping_msg(struct Ping_MSG *ping_msg, size_t *out_size)
{
    *out_size = sizeof(ping_msg->type) + sizeof(ping_msg->message_id);

    uint8_t *buffer = malloc(*out_size);

    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    memcpy(buffer + offset, &ping_msg->type, sizeof(ping_msg->type));
    offset += sizeof(ping_msg->type);

    memcpy(buffer + offset, &ping_msg->message_id, sizeof(ping_msg->message_id));

    return buffer;
}

void create_confirm_msg(struct Confirm_MSG *confirm_msg, uint16_t ref_message_id)
{
    confirm_msg->type = MSG_TYPE_VAL[CONFIRM];
    confirm_msg->ref_message_id = ref_message_id;
}

void create_reply_msg(struct Reply_MSG *reply_msg, uint16_t message_id, uint8_t result, uint16_t ref_message_id, char *message_contents)
{
    reply_msg->type = MSG_TYPE_VAL[REPLY];
    reply_msg->message_id = message_id;
    reply_msg->result = result;
    reply_msg->ref_message_id = ref_message_id;
    reply_msg->message_contents = message_contents;
}

void create_auth_msg(struct Auth_MSG *auth_msg, char *username, char *display_name, char *secret)
{
    auth_msg->type = MSG_TYPE_VAL[AUTH];
    auth_msg->message_id = 0x0000;
    auth_msg->username = username; 
    auth_msg->display_name = display_name;
    auth_msg->secret = secret;
}

void create_join_msg(struct Join_MSG *join_msg, uint16_t message_id, char *channel_id, char *display_name)
{
    join_msg->type = MSG_TYPE_VAL[JOIN];
    join_msg->message_id = message_id;
    join_msg->channel_id = channel_id;
    join_msg->display_name = display_name;
}

void create_msg(struct MSG *msg, uint16_t message_id, char *display_name, char *message_contents)
{
    msg->type = MSG_TYPE_VAL[MSG];
    msg->message_id = message_id;
    msg->display_name = display_name;
    msg->message_contents = message_contents;
}

void create_err_msg(struct Err_MSG *err_msg, uint16_t message_id, char *display_name, char *message_contents)
{
    err_msg->type = MSG_TYPE_VAL[ERR];
    err_msg->message_id = message_id;
    err_msg->display_name = display_name;
    err_msg->message_contents = message_contents;
}

void create_bye_msg(struct Bye_MSG *bye_msg, uint16_t message_id, char *display_name)
{
    bye_msg->type = MSG_TYPE_VAL[BYE];
    bye_msg->message_id = message_id;
    bye_msg->display_name = display_name;
}

void create_ping_msg(struct Ping_MSG *ping_msg, uint16_t message_id)
{
    ping_msg->type = MSG_TYPE_VAL[PING];
    ping_msg->message_id = message_id;
}

// int match_regex(const char *pattern, const char *string)
// {
//     regex_t regex;
//     int ret;
//     char error_buffer[256];
    
//     char anchored_pattern[256];
//     snprintf(anchored_pattern, sizeof(anchored_pattern), "^%s$", pattern);
    
//     ret = regcomp(&regex, anchored_pattern, REG_EXTENDED);
//     if (ret) {
//         regerror(ret, &regex, error_buffer, sizeof(error_buffer));
//         printf("Regex compilation failed: %s\n", error_buffer);
//         return 1;
//     }
    
//     ret = regexec(&regex, string, 0, NULL, 0);
//     if (!ret) {
//         printf("'%s' matches pattern '%s'\n", string, pattern);
//     }
//     else if (ret == REG_NOMATCH) {
//         printf("'%s' does not match pattern '%s'\n", string, pattern);
//     }
//     else {
//         regerror(ret, &regex, error_buffer, sizeof(error_buffer));
//         printf("Regex match failed: %s\n", error_buffer);
//     }
    
//     regfree(&regex);
//     return ret;
// }

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

static void parse_args(int argc, char **argv, int *use_tcp_protocol, char **hostname, char **port, uint16_t *udp_timeout, uint8_t *udp_retransmissions)
{
    for (int i = 0; i < argc; i++) {
        
        if (strcmp(argv[i], "-t") == 0) {
            i++;
            if (i >= argc) {
                printf("-t flag expects argument 'tcp' or 'udp'\n");
                exit(EXIT_SUCCESS);
            }
            else if (strcmp(argv[i], "tcp") == 0) {
                *use_tcp_protocol = 1;
            }
            else if (strcmp(argv[i], "udp") == 0) {
                *use_tcp_protocol = 0;
            }
            else {
                printf("invalid -t flag argument, expected 'tcp' or 'udp'\n");
                exit(EXIT_SUCCESS);
            }
        }

        if (strcmp(argv[i], "-s") == 0) {
            i++;
            if (i >= argc) {
                printf("-s flag expects argument of ip or hostname\n");
                exit(EXIT_SUCCESS);
            }
            *hostname = argv[i];
        }

        if (strcmp(argv[i], "-p") == 0) {
            i++;
            if (i >= argc) {
                printf("-p flag expects argument of port number\n");
                exit(EXIT_SUCCESS);
            }
            *port = argv[i];
        }

        if (strcmp(argv[i], "-d") == 0) {
            i++;
            if (i >= argc) {
                printf("-d flag expects argument of udp confirmation timeout\n");
                exit(EXIT_SUCCESS);
            }
            
            char *endptr = NULL;
            long converted_timeout = strtol(argv[i], &endptr, 10);

            if (strcmp(endptr, "\0") != 0) {
                printf("invalid udp confirmation timeout value\n");
                exit(EXIT_SUCCESS);
            }

            *udp_timeout = (uint16_t)converted_timeout;
        }

        if (strcmp(argv[i], "-r") == 0) {
            i++;
            if (i >= argc) {
                printf("-r flag expects argument of max number of udp retransmissions\n");
                exit(EXIT_SUCCESS);
            }

            char *endptr = NULL;
            long converted_retrans = strtol(argv[i], &endptr, 10);

            if (strcmp(endptr, "\0") != 0) {
                printf("invalid udp retransmissions value\n");
                exit(EXIT_SUCCESS);
            }

            *udp_retransmissions = (uint8_t)converted_retrans;
        }

        if (strcmp(argv[i], "-h") == 0) {
            help_cmd();
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

char *line = NULL;
int sockfd = -1;
struct addrinfo hints, *servinfo, *p;

int use_tcp_protocol = -1;
char *hostname = NULL;
char *port = "4567";
uint16_t udp_timeout = 250;
uint8_t udp_retransmissions = 3;

static void auth_cmd(void)
{
    char username[MAX_USERNAME_LEN] = {0};
    char display_name[MAX_DISPLAY_NAME_LEN] = {0};
    char secret[MAX_SECRET_LEN] = {0};

    // username[MAX_USERNAME_LEN] = '\0';
    // display_name[MAX_DISPLAY_NAME_LEN] = '\0';
    // secret[MAX_SECRET_LEN] = '\0';

    line = strtok(NULL, " "); // get rid of /auth
    if (line == NULL) {
        fprintf(stderr, "Please provide 'username' 'display_name' 'secret'\n");
        return;
    }
    memcpy(username, line, strlen(line));

    line = strtok(NULL, " ");
    if (line == NULL) {
        fprintf(stderr, "Please provide 'username' 'display_name' 'secret'\n");
        return;
    }
    memcpy(display_name, line, strlen(line));

    line = strtok(NULL, " ");
    if (line == NULL) {
        fprintf(stderr, "Please provide 'username' 'display_name' 'secret'\n");
        return;
    }
    memcpy(secret, line, strlen(line));

    // line = NULL;

    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
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
        return;
    }

    // struct Auth_MSG *packet = malloc(sizeof(struct Auth_MSG));
    struct Auth_MSG auth_msg;
    create_auth_msg(&auth_msg, username, display_name, secret);
    size_t buffer_size;
    uint8_t *buffer = serialize_auth_msg(&auth_msg, &buffer_size);

    printf("size of struct: %zu\n", buffer_size);

    if ((numbytes = sendto(sockfd, buffer, buffer_size, 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("client: sendto");
        exit(EXIT_FAILURE);
    }

    free(buffer);

    printf("sent %d bytes to %s\n", numbytes, hostname);



}

static void join_cmd(void)
{

}

static void rename_cmd(void)
{

}

static void help_cmd(void)
{
    printf("flags: -t -s -p -d -r -h\n");
}

static void cleanup()
{
    printf("\ncleaning up...\n");
    // free(line);
    freeaddrinfo(servinfo);
    if (sockfd != -1) {
        close(sockfd);
    }
    exit(EXIT_SUCCESS);
}

const char *commands[] = {
    "/auth",
    "/join",
    "/rename",
    "/help"
};

void (*command_functions[4])(void) = {
    auth_cmd,
    join_cmd,
    rename_cmd,
    help_cmd
};

int main(int argc, char *argv[])
{
    signal(SIGINT, exit);

    int reg_status = atexit(cleanup);
    if (reg_status != 0) {
        fprintf(stderr, "Failed to register cleanup function.\n");
        exit(EXIT_FAILURE);
    }

    // int use_tcp_protocol = -1;
    // char *hostname = NULL;
    // char *port = "4567";
    // uint16_t udp_timeout = 250;
    // uint8_t udp_retransmissions = 3;

    if (argc == 1) {
        fprintf(stderr, "please provide a -t flag value, 'tcp' or 'udp'\n");
        fprintf(stderr, "please provide a -s flag value, hostname\n\n");
        help_cmd();
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

        // printf("\'%s\'\n", token);
    }


    // int sockfd;
    // struct addrinfo hints, *servinfo, *p;
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
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
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

    if ((numbytes = sendto(sockfd, buffer, buffer_size, 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("client: sendto");
        exit(EXIT_FAILURE);
    }

    printf("sent %d bytes to %s\n", numbytes, hostname);

    // freeaddrinfo(servinfo);


    // close(sockfd);

    // return EXIT_SUCCESS;
}
