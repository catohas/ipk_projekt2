#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "./deserialize.h"
#include "./debug.h"

struct Confirm_MSG *deserialize_confirm_msg(const uint8_t *buffer, size_t buffer_size) {
    // Validate buffer size
    if (buffer_size < (sizeof(uint8_t) + sizeof(uint16_t))) {
        printf_debug_simple(COLOR_ERR, "buffer is smaller than needed");
        return NULL;
    }

    struct Confirm_MSG *confirm_msg = malloc(sizeof(struct Confirm_MSG));
    if (confirm_msg == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;
    
    memcpy(&confirm_msg->type, buffer + offset, sizeof(confirm_msg->type));
    offset += sizeof(confirm_msg->type);

    uint16_t net_ref_message_id;
    memcpy(&net_ref_message_id, buffer + offset, sizeof(net_ref_message_id));
    confirm_msg->ref_message_id = ntohs(net_ref_message_id);

    return confirm_msg;
}

struct Reply_MSG *deserialize_reply_msg(const uint8_t *buffer, size_t buffer_size) {
    // Minimum size check (excluding variable-length message_contents)
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t) + 
                           sizeof(uint8_t) + sizeof(uint16_t) + 1; // +1 for null terminator
    if (buffer_size < min_size) {
        return NULL;
    }

    struct Reply_MSG *reply_msg = malloc(sizeof(struct Reply_MSG));
    if (reply_msg == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    // Deserialize type
    memcpy(&reply_msg->type, buffer + offset, sizeof(reply_msg->type));
    offset += sizeof(reply_msg->type);

    // Deserialize message_id and convert from network byte order
    uint16_t net_message_id;
    memcpy(&net_message_id, buffer + offset, sizeof(net_message_id));
    reply_msg->message_id = ntohs(net_message_id);
    offset += sizeof(net_message_id);

    // Deserialize result
    memcpy(&reply_msg->result, buffer + offset, sizeof(reply_msg->result));
    offset += sizeof(reply_msg->result);

    // Deserialize ref_message_id and convert from network byte order
    uint16_t net_ref_message_id;
    memcpy(&net_ref_message_id, buffer + offset, sizeof(net_ref_message_id));
    reply_msg->ref_message_id = ntohs(net_ref_message_id);
    offset += sizeof(net_ref_message_id);

    // Calculate remaining buffer size for message_contents
    size_t remaining_size = buffer_size - offset;
    
    // Allocate and copy message_contents
    char *message_contents = malloc(remaining_size);
    if (message_contents == NULL) {
        free(reply_msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    
    memcpy(message_contents, buffer + offset, remaining_size);
    reply_msg->message_contents = message_contents;

    // Verify null termination
    if (message_contents[remaining_size - 1] != '\0') {
        printf_debug_simple(COLOR_ERR, "deserialized reply msg did not have null terminator at the end");
        free(message_contents);
        free(reply_msg);
        return NULL;
    }

    return reply_msg;
}

struct Auth_MSG *deserialize_auth_msg(const uint8_t *buffer, size_t buffer_size) {
    // Minimum size check (type + message_id + at least 3 null terminators)
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t) + 3;
    if (buffer_size < min_size) {
        return NULL;
    }

    struct Auth_MSG *auth_msg = malloc(sizeof(struct Auth_MSG));
    if (auth_msg == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    // Deserialize type
    memcpy(&auth_msg->type, buffer + offset, sizeof(auth_msg->type));
    offset += sizeof(auth_msg->type);

    // Deserialize message_id and convert from network byte order
    uint16_t net_message_id;
    memcpy(&net_message_id, buffer + offset, sizeof(net_message_id));
    auth_msg->message_id = ntohs(net_message_id);
    offset += sizeof(net_message_id);

    // Find username (null-terminated string)
    const char *username_start = (const char *)(buffer + offset);
    size_t username_len = strlen(username_start);
    
    // Validate that the username fits within the buffer
    if (offset + username_len + 1 > buffer_size) {
        free(auth_msg);
        return NULL;
    }
    
    // Allocate and copy username
    auth_msg->username = malloc(username_len + 1);
    if (auth_msg->username == NULL) {
        free(auth_msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(auth_msg->username, username_start);
    offset += username_len + 1; // Skip past null terminator
    
    // Find display_name (null-terminated string)
    const char *display_name_start = (const char *)(buffer + offset);
    size_t display_name_len = strlen(display_name_start);
    
    // Validate that the display_name fits within the buffer
    if (offset + display_name_len + 1 > buffer_size) {
        free(auth_msg->username);
        free(auth_msg);
        return NULL;
    }
    
    // Allocate and copy display_name
    auth_msg->display_name = malloc(display_name_len + 1);
    if (auth_msg->display_name == NULL) {
        free(auth_msg->username);
        free(auth_msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(auth_msg->display_name, display_name_start);
    offset += display_name_len + 1; // Skip past null terminator
    
    // Find secret (null-terminated string)
    const char *secret_start = (const char *)(buffer + offset);
    size_t secret_len = strlen(secret_start);
    
    // Validate that the secret fits within the buffer
    if (offset + secret_len + 1 > buffer_size) {
        free(auth_msg->display_name);
        free(auth_msg->username);
        free(auth_msg);
        return NULL;
    }
    
    // Allocate and copy secret
    auth_msg->secret = malloc(secret_len + 1);
    if (auth_msg->secret == NULL) {
        free(auth_msg->display_name);
        free(auth_msg->username);
        free(auth_msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(auth_msg->secret, secret_start);
    
    return auth_msg;
}

struct Join_MSG *deserialize_join_msg(const uint8_t *buffer, size_t buffer_size) {
    // Minimum size check (type + message_id + at least 2 null terminators)
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t) + 2;
    if (buffer_size < min_size) {
        return NULL;
    }

    struct Join_MSG *join_msg = malloc(sizeof(struct Join_MSG));
    if (join_msg == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    // Deserialize type
    memcpy(&join_msg->type, buffer + offset, sizeof(join_msg->type));
    offset += sizeof(join_msg->type);

    // Deserialize message_id and convert from network byte order
    uint16_t net_message_id;
    memcpy(&net_message_id, buffer + offset, sizeof(net_message_id));
    join_msg->message_id = ntohs(net_message_id);
    offset += sizeof(net_message_id);

    // Find channel_id (null-terminated string)
    const char *channel_id_start = (const char *)(buffer + offset);
    size_t channel_id_len = strlen(channel_id_start);
    
    // Validate that the channel_id fits within the buffer
    if (offset + channel_id_len + 1 > buffer_size) {
        free(join_msg);
        return NULL;
    }
    
    // Allocate and copy channel_id
    join_msg->channel_id = malloc(channel_id_len + 1);
    if (join_msg->channel_id == NULL) {
        free(join_msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(join_msg->channel_id, channel_id_start);
    offset += channel_id_len + 1; // Skip past null terminator
    
    // Find display_name (null-terminated string)
    const char *display_name_start = (const char *)(buffer + offset);
    size_t display_name_len = strlen(display_name_start);
    
    // Validate that the display_name fits within the buffer
    if (offset + display_name_len + 1 > buffer_size) {
        free(join_msg->channel_id);
        free(join_msg);
        return NULL;
    }
    
    // Allocate and copy display_name
    join_msg->display_name = malloc(display_name_len + 1);
    if (join_msg->display_name == NULL) {
        free(join_msg->channel_id);
        free(join_msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(join_msg->display_name, display_name_start);
    
    return join_msg;
}

struct MSG *deserialize_msg(const uint8_t *buffer, size_t buffer_size) {
    // Minimum size check (type + message_id + at least 2 null terminators)
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t) + 2;
    if (buffer_size < min_size) {
        return NULL;
    }

    struct MSG *msg = malloc(sizeof(struct MSG));
    if (msg == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    // Deserialize type
    memcpy(&msg->type, buffer + offset, sizeof(msg->type));
    offset += sizeof(msg->type);

    // Deserialize message_id and convert from network byte order
    uint16_t net_message_id;
    memcpy(&net_message_id, buffer + offset, sizeof(net_message_id));
    msg->message_id = ntohs(net_message_id);
    offset += sizeof(net_message_id);

    // Find display_name (null-terminated string)
    const char *display_name_start = (const char *)(buffer + offset);
    size_t display_name_len = strlen(display_name_start);
    
    // Validate that the display_name fits within the buffer
    if (offset + display_name_len + 1 > buffer_size) {
        free(msg);
        return NULL;
    }
    
    // Allocate and copy display_name
    msg->display_name = malloc(display_name_len + 1);
    if (msg->display_name == NULL) {
        free(msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(msg->display_name, display_name_start);
    offset += display_name_len + 1; // Skip past null terminator
    
    // Find message_contents (null-terminated string)
    const char *message_contents_start = (const char *)(buffer + offset);
    size_t message_contents_len = strlen(message_contents_start);
    
    // Validate that the message_contents fits within the buffer
    if (offset + message_contents_len + 1 > buffer_size) {
        free(msg->display_name);
        free(msg);
        return NULL;
    }
    
    // Allocate and copy message_contents
    msg->message_contents = malloc(message_contents_len + 1);
    if (msg->message_contents == NULL) {
        free(msg->display_name);
        free(msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(msg->message_contents, message_contents_start);
    
    return msg;
}

struct Ping_MSG *deserialize_ping_msg(const uint8_t *buffer, size_t buffer_size) {
    // Validate buffer size
    if (buffer_size < (sizeof(uint8_t) + sizeof(uint16_t))) {
        return NULL;
    }

    struct Ping_MSG *ping_msg = malloc(sizeof(struct Ping_MSG));
    if (ping_msg == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;
    
    // Deserialize type
    memcpy(&ping_msg->type, buffer + offset, sizeof(ping_msg->type));
    offset += sizeof(ping_msg->type);

    // Deserialize message_id and convert from network byte order
    uint16_t net_message_id;
    memcpy(&net_message_id, buffer + offset, sizeof(net_message_id));
    ping_msg->message_id = ntohs(net_message_id);

    return ping_msg;
}

struct Err_MSG *deserialize_err_msg(const uint8_t *buffer, size_t buffer_size) {
    // Minimum size check (type + message_id + at least 2 null terminators)
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t) + 2;
    if (buffer_size < min_size) {
        return NULL;
    }

    struct Err_MSG *err_msg = malloc(sizeof(struct Err_MSG));
    if (err_msg == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    // Deserialize type
    memcpy(&err_msg->type, buffer + offset, sizeof(err_msg->type));
    offset += sizeof(err_msg->type);

    // Deserialize message_id and convert from network byte order
    uint16_t net_message_id;
    memcpy(&net_message_id, buffer + offset, sizeof(net_message_id));
    err_msg->message_id = ntohs(net_message_id);
    offset += sizeof(net_message_id);

    // Find display_name (null-terminated string)
    const char *display_name_start = (const char *)(buffer + offset);
    size_t display_name_len = strlen(display_name_start);
    
    // Validate that the display_name fits within the buffer
    if (offset + display_name_len + 1 > buffer_size) {
        free(err_msg);
        return NULL;
    }
    
    // Allocate and copy display_name
    err_msg->display_name = malloc(display_name_len + 1);
    if (err_msg->display_name == NULL) {
        free(err_msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(err_msg->display_name, display_name_start);
    offset += display_name_len + 1; // Skip past null terminator
    
    // Find message_contents (null-terminated string)
    const char *message_contents_start = (const char *)(buffer + offset);
    size_t message_contents_len = strlen(message_contents_start);
    
    // Validate that the message_contents fits within the buffer
    if (offset + message_contents_len + 1 > buffer_size) {
        free(err_msg->display_name);
        free(err_msg);
        return NULL;
    }
    
    // Allocate and copy message_contents
    err_msg->message_contents = malloc(message_contents_len + 1);
    if (err_msg->message_contents == NULL) {
        free(err_msg->display_name);
        free(err_msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(err_msg->message_contents, message_contents_start);
    
    return err_msg;
}

struct Bye_MSG *deserialize_bye_msg(const uint8_t *buffer, size_t buffer_size) {
    // Minimum size check (type + message_id + at least 1 null terminator)
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t) + 1;
    if (buffer_size < min_size) {
        return NULL;
    }

    struct Bye_MSG *bye_msg = malloc(sizeof(struct Bye_MSG));
    if (bye_msg == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t offset = 0;

    // Deserialize type
    memcpy(&bye_msg->type, buffer + offset, sizeof(bye_msg->type));
    offset += sizeof(bye_msg->type);

    // Deserialize message_id and convert from network byte order
    uint16_t net_message_id;
    memcpy(&net_message_id, buffer + offset, sizeof(net_message_id));
    bye_msg->message_id = ntohs(net_message_id);
    offset += sizeof(net_message_id);

    // Find display_name (null-terminated string)
    const char *display_name_start = (const char *)(buffer + offset);
    size_t display_name_len = strlen(display_name_start);
    
    // Validate that the display_name fits within the buffer
    if (offset + display_name_len + 1 > buffer_size) {
        free(bye_msg);
        return NULL;
    }
    
    // Allocate and copy display_name
    bye_msg->display_name = malloc(display_name_len + 1);
    if (bye_msg->display_name == NULL) {
        free(bye_msg);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(bye_msg->display_name, display_name_start);
    
    return bye_msg;
}

void free_confirm_msg(struct Confirm_MSG *msg) {
    free(msg);
}

void free_reply_msg(struct Reply_MSG *msg) {
    if (msg) {
        free(msg->message_contents);
        free(msg);
    }
}

void free_auth_msg(struct Auth_MSG *msg) {
    if (msg) {
        free(msg->username);
        free(msg->display_name);
        free(msg->secret);
        free(msg);
    }
}

void free_join_msg(struct Join_MSG *msg) {
    if (msg) {
        free(msg->channel_id);
        free(msg->display_name);
        free(msg);
    }
}

void free_msg(struct MSG *msg) {
    if (msg) {
        free(msg->display_name);
        free(msg->message_contents);
        free(msg);
    }
}

void free_ping_msg(struct Ping_MSG *msg) {
    free(msg);
}

void free_err_msg(struct Err_MSG *msg) {
    if (msg) {
        free(msg->display_name);
        free(msg->message_contents);
        free(msg);
    }
}

void free_bye_msg(struct Bye_MSG *msg) {
    if (msg) {
        free(msg->display_name);
        free(msg);
    }
}
