// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: network_tcp.c
// April 2025

#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "./network.h"
#include "./debug.h"
#include "./messages.h"
#include "./global.h"

int establish_tcp_connection(void)
{
    struct addrinfo hints, *servinfo, *p;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int rv = getaddrinfo(hostname, port, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }
    
    // find the first available connection
    for (p = servinfo; p != NULL; p = p->ai_next) {
        tcp_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (tcp_socket == -1) {
            perror("socket");
            continue;
        }
        
        if (connect(tcp_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(tcp_socket);
            perror("connect");
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "failed to connect\n");
        return -1;
    }
    
    freeaddrinfo(servinfo);

    printf_debug_simple(COLOR_SUCCESS, "TCP connection established");

    return tcp_socket;
}

void close_tcp_connection(void)
{
    if (tcp_socket != -1) {
        close(tcp_socket);
        tcp_socket = -1;
        printf_debug_simple(COLOR_INFO, "TCP connection closed");
    }
}

void send_network_msg_tcp(const char *message)
{
    if (tcp_socket == -1) {
        printf_debug_simple(COLOR_ERR, "Attempted to send message with no TCP connection");
        return;
    }

    size_t len = strlen(message);
    ssize_t bytes_sent = send(tcp_socket, message, len, 0);
    
    if (bytes_sent == -1) {
        perror("send");
        return;
    }
    
    printf_debug(COLOR_INFO, "Sent %zd bytes '%s' using TCP", bytes_sent, message);
}

void send_tcp_auth_msg(const char *username, const char *display_name, const char *secret)
{
    char buffer[MAX_MSG_SIZE];
    snprintf(buffer, MAX_MSG_SIZE, "AUTH %s AS %s USING %s\r\n", username, display_name, secret);
    send_network_msg_tcp(buffer);
}

void send_tcp_join_msg(const char *channel_id, const char *display_name)
{
    char buffer[MAX_MSG_SIZE];
    snprintf(buffer, MAX_MSG_SIZE, "JOIN %s AS %s\r\n", channel_id, display_name);
    send_network_msg_tcp(buffer);
}

void send_tcp_msg_msg(const char *display_name, const char *message_content)
{
    char buffer[MAX_MSG_SIZE];
    snprintf(buffer, MAX_MSG_SIZE, "MSG FROM %s IS %s\r\n", display_name, message_content);
    send_network_msg_tcp(buffer);
}

void send_tcp_bye_msg(const char *display_name)
{
    char buffer[MAX_MSG_SIZE];
    snprintf(buffer, MAX_MSG_SIZE, "BYE FROM %s\r\n", display_name);
    send_network_msg_tcp(buffer);
}

void send_tcp_err_msg(const char *display_name, const char *message_content) {
    char buffer[MAX_MSG_SIZE];
    snprintf(buffer, MAX_MSG_SIZE, "ERR FROM %s IS %s\r\n", display_name, message_content);
    send_network_msg_tcp(buffer);
}

void process_tcp_reply_message(const char *message)
{
    char result[4]; // "OK" or "NOK"
    char content[MAX_MESSAGE_CONTENT_LEN + 1] = {0};
    
    // parse the message
    if (sscanf(message, "REPLY %3s IS %[^\r\n]", result, content) < 2) {
        printf("ERROR: Malformed REPLY message\n");
        return;
    }
    
    // handle based on current state
    if (strcmp(result, "OK") == 0) {
        printf("Action Success: %s\n", content);
        
        switch (state) { // state transitions
            case STATE_AUTH:
                state = STATE_OPEN;
                printf_debug_simple(COLOR_SUCCESS, "Transitioned to OPEN state");
                break;
            case STATE_JOIN:
                state = STATE_OPEN;
                printf_debug_simple(COLOR_SUCCESS, "Returned to OPEN state");
                break;
            default:
                break;
        }
    }
    else if (strcmp(result, "NOK") == 0) {
        printf("Action Failure: %s\n", content);
        
        // return to OPEN state if we were in JOIN and it failed
        if (state == STATE_JOIN) {
            state = STATE_OPEN;
            printf_debug_simple(COLOR_INFO, "Returned to OPEN state after failed JOIN");
        }
    }
    else {
        printf("ERROR: Invalid REPLY type\n");
    }
}

void process_tcp_msg_message(const char *message)
{
    char display_name[MAX_DISPLAY_NAME_LEN + 1] = {0};
    char content[MAX_MESSAGE_CONTENT_LEN + 1] = {0};
    
    // extract the display name (which can contain spaces)
    const char *from_pos = strstr(message, "FROM ");
    if (!from_pos) {
        printf("ERROR: Malformed MSG message (missing FROM)\n");
        return;
    }
    
    const char *is_pos = strstr(from_pos + 5, " IS ");
    if (!is_pos) {
        printf("ERROR: Malformed MSG message (missing IS)\n");
        return;
    }
    
    // extract display name
    size_t display_name_len = is_pos - (from_pos + 5);
    if (display_name_len > MAX_DISPLAY_NAME_LEN) {
        display_name_len = MAX_DISPLAY_NAME_LEN;
    }
    strncpy(display_name, from_pos + 5, display_name_len);
    display_name[display_name_len] = '\0';
    
    // extract message content
    strcpy(content, is_pos + 4); // 4 is length of " IS "
    
    // display the message
    printf("%s: %s\n", display_name, content);
}

void process_tcp_err_message(const char *message)
{
    char display_name[MAX_DISPLAY_NAME_LEN + 1] = {0};
    char content[MAX_MESSAGE_CONTENT_LEN + 1] = {0};
    
    const char *from_pos = strstr(message, "FROM ");
    if (!from_pos) {
        printf("ERROR: Malformed ERR message (missing FROM)\n");
        return;
    }
    
    const char *is_pos = strstr(from_pos + 5, " IS ");
    if (!is_pos) {
        printf("ERROR: Malformed ERR message (missing IS)\n");
        return;
    }
    
    // extract display name
    size_t display_name_len = is_pos - (from_pos + 5);
    if (display_name_len > MAX_DISPLAY_NAME_LEN) {
        display_name_len = MAX_DISPLAY_NAME_LEN;
    }
    strncpy(display_name, from_pos + 5, display_name_len);
    display_name[display_name_len] = '\0';
    
    // extract message content
    strcpy(content, is_pos + 4);
    
    printf("ERROR FROM %s: %s\n", display_name, content);
    
    printf_debug_simple(COLOR_ERR, "Server sent ERR message, terminating connection");

    exit(EXIT_SUCCESS);
}

void process_tcp_bye_message(const char *message)
{
    char display_name[MAX_DISPLAY_NAME_LEN + 1] = {0};
    
    const char *from_pos = strstr(message, "FROM ");
    if (!from_pos) {
        printf("ERROR: Malformed BYE message\n");
        return;
    }
    
    // extract display name
    strcpy(display_name, from_pos + 5);
    
    printf_debug(COLOR_INFO, "BYE message received from %s", display_name);

    exit(EXIT_SUCCESS);
}

void process_tcp_message(const char *message)
{
    printf_debug(COLOR_INFO, "Processing TCP message: %s", message);
    
    if (strncmp(message, "REPLY ", 6) == 0) {
        process_tcp_reply_message(message);
    } 
    else if (strncmp(message, "MSG FROM ", 9) == 0) {
        process_tcp_msg_message(message);
    }
    else if (strncmp(message, "ERR FROM ", 9) == 0) {
        process_tcp_err_message(message);
    }
    else if (strncmp(message, "BYE FROM ", 9) == 0) {
        process_tcp_bye_message(message);
    }
    else {
        printf("ERROR: Unknown message type received: '%s'\n", message);

        char error_message[MAX_MESSAGE_CONTENT_LEN];
        snprintf(error_message, MAX_MESSAGE_CONTENT_LEN, "Received unknown message type: %.20s", message);
        
        printf_debug_simple(COLOR_ERR, "Sending ERR message and terminating connection");

        send_tcp_err_msg(display_name, error_message);
        
        exit(EXIT_SUCCESS);
    }
}

void *tcp_listener(void *arg)
{
    (void)arg;
    
    char recv_buffer[MAX_MSG_SIZE];
    char *line_buffer = NULL;
    size_t line_buffer_size = 0;
    
    listener_thread_running = true;
    
    while (listener_thread_running && tcp_socket != -1) {
        ssize_t bytes_read = recv(tcp_socket, recv_buffer, sizeof(recv_buffer) - 1, 0);
        
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                printf_debug_simple(COLOR_ERR, "Server closed connection");
            }
            else {
                perror("recv");
            }
            break;
        }
        
        recv_buffer[bytes_read] = '\0';
        printf_debug(COLOR_INFO, "Received %zd bytes: %s", bytes_read, recv_buffer);
        
        // process all complete lines in the buffer
        char *start = recv_buffer;
        char *end;
        
        while ((end = strstr(start, "\r\n")) != NULL) {
            *end = '\0'; // temporarily terminate the line
            
            // allocate or reallocate line buffer if needed
            size_t line_len = strlen(start);
            if (line_buffer_size == 0) {
                line_buffer = malloc(line_len + 1);
                if (!line_buffer) {
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                line_buffer_size = line_len + 1;
                line_buffer[0] = '\0';
            } else if (line_buffer_size < strlen(line_buffer) + line_len + 1) {
                line_buffer_size = strlen(line_buffer) + line_len + 1;
                line_buffer = realloc(line_buffer, line_buffer_size);
                if (!line_buffer) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
            }
            
            // add this segment to existing line buffer
            strcat(line_buffer, start);
            
            // process the complete line
            process_tcp_message(line_buffer);
            
            // reset line buffer for next message
            line_buffer[0] = '\0';
            
            // move past this line and the \r\n
            start = end + 2;
        }
        
        // if there's an incomplete line at the end, save it
        if (*start != '\0') {
            size_t remaining = strlen(start);
            if (line_buffer_size == 0) {
                line_buffer = malloc(remaining + 1);
                if (!line_buffer) {
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                line_buffer_size = remaining + 1;
                strcpy(line_buffer, start);
            }
            else {
                if (line_buffer_size < strlen(line_buffer) + remaining + 1) {
                    line_buffer_size = strlen(line_buffer) + remaining + 1;
                    line_buffer = realloc(line_buffer, line_buffer_size);
                    if (!line_buffer) {
                        perror("realloc");
                        exit(EXIT_FAILURE);
                    }
                }
                strcat(line_buffer, start);
            }
        }
    }
    
    free(line_buffer);

    listener_thread_running = false;
    
    // if socket is still open but thread is ending, close it
    if (tcp_socket != -1) {
        close(tcp_socket);
        tcp_socket = -1;
    }
    
    return NULL;
}

int start_tcp_listener(void)
{
    if (listener_thread_running) {
        printf_debug_simple(COLOR_INFO, "TCP listener already running");
        return 0;
    }
    
    if (pthread_create(&listener_thread, NULL, tcp_listener, NULL) != 0) {
        perror("pthread_create");
        return -1;
    }
    
    pthread_detach(listener_thread);

    return 0;
}

void stop_tcp_listener(void)
{
    if (listener_thread_running) {
        listener_thread_running = false;
        if (tcp_socket != -1) {
            shutdown(tcp_socket, SHUT_RDWR);
        }
    }
}

void send_network_msg_tcp_wrapper(const char* type, void* data)
{
    if (strcmp(type, "AUTH") == 0) {
        struct Auth_Data* auth_data = (struct Auth_Data*)data;
        send_tcp_auth_msg(auth_data->username, auth_data->display_name, auth_data->secret);
    }
    else if (strcmp(type, "JOIN") == 0) {
        struct Join_Data* join_data = (struct Join_Data*)data;
        send_tcp_join_msg(join_data->channel_id, join_data->display_name);
    }
    else if (strcmp(type, "MSG") == 0) {
        struct Msg_Data* msg_data = (struct Msg_Data*)data;
        send_tcp_msg_msg(msg_data->display_name, msg_data->message_content);
    }
    else if (strcmp(type, "BYE") == 0) {
        struct Bye_Data* bye_data = (struct Bye_Data*)data;
        send_tcp_bye_msg(bye_data->display_name);
    }
    else if (strcmp(type, "ERR") == 0) {
        struct Err_Data* err_data = (struct Err_Data*)data;
        send_tcp_err_msg(err_data->display_name, err_data->message_content);
    }
}

void cleanup_tcp(void)
{
    stop_tcp_listener();
    close_tcp_connection();
}
