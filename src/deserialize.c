#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "./deserialize.h"
#include "./debug.h"
#include "./maximums.h"

struct Confirm_MSG *deserialize_confirm_msg(uint8_t *buffer, const size_t buffer_size)
{
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t);
    
    if (buffer_size < min_size) {
        printf_debug_simple(COLOR_ERR, "confirm msg buffer is smaller than needed");
        return NULL;
    }

    struct Confirm_MSG *confirm_msg = malloc(sizeof(struct Confirm_MSG));
    if (confirm_msg == NULL) {
        free(buffer);
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

struct Reply_MSG *deserialize_reply_msg(uint8_t *buffer, const size_t buffer_size)
{
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t) + 1*sizeof(uint8_t) + 1; // 1 one byte item + 1 null byte

    if (buffer_size < min_size) {
        printf_debug_simple(COLOR_ERR, "reply msg buffer is smaller than needed");
        return NULL;
    }

    struct Reply_MSG *reply_msg = malloc(sizeof(struct Reply_MSG));
    if (reply_msg == NULL) {
        free(buffer);
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

    // Deserialize message contents
    size_t message_content_len = strlen((char*)&buffer[offset])+1; // null character
    reply_msg->message_contents = malloc(message_content_len * sizeof(char));
    if (reply_msg->message_contents == NULL) {
        perror("Memory allocation failed");
        free(buffer);
        free(reply_msg);
        exit(EXIT_FAILURE);
    }
    strcpy(reply_msg->message_contents, (char*)&buffer[offset]);

    return reply_msg;
}

struct MSG *deserialize_msg(uint8_t *buffer, const size_t buffer_size)
{
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t) + 2*sizeof(uint8_t) + 2;

    if (buffer_size < min_size) {
        printf_debug_simple(COLOR_ERR, "text msg buffer is smaller than needed");
        return NULL;
    }

    struct MSG *msg = malloc(sizeof(struct MSG));
    if (msg == NULL) {
        free(buffer);
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

    size_t display_name_len = strlen((char*)&buffer[offset])+1;
    msg->display_name = malloc(display_name_len * sizeof(char));
    if (msg->display_name == NULL) {
        perror("Memory allocation failed");
        free(buffer);
        free(msg);
        exit(EXIT_FAILURE);
    }
    strcpy(msg->display_name, (char*)&buffer[offset]);

    offset += display_name_len;

    size_t message_contents_len = strlen((char*)&buffer[offset])+1;
    msg->message_contents = malloc(message_contents_len * sizeof(char));
    if (msg->message_contents == NULL) {
        perror("Memory allocation failed");
        free(buffer);
        free(msg);
        exit(EXIT_FAILURE);
    }
    strcpy(msg->message_contents, (char*)&buffer[offset]);

    return msg;
}

struct Err_MSG *deserialize_err_msg(uint8_t *buffer, const size_t buffer_size)
{
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t) + 2*sizeof(uint8_t) + 2;

    if (buffer_size < min_size) {
        printf_debug_simple(COLOR_ERR, "err msg buffer is smaller than needed");
        return NULL;
    }

    struct Err_MSG *err_msg = malloc(sizeof(struct MSG));
    if (err_msg == NULL) {
        free(buffer);
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

    size_t display_name_len = strlen((char*)&buffer[offset])+1;
    err_msg->display_name = malloc(display_name_len * sizeof(char));
    if (err_msg->display_name == NULL) {
        perror("Memory allocation failed");
        free(buffer);
        free(err_msg);
        exit(EXIT_FAILURE);
    }
    strcpy(err_msg->display_name, (char*)&buffer[offset]);

    offset += display_name_len;

    size_t message_contents_len = strlen((char*)&buffer[offset])+1;
    err_msg->message_contents = malloc(message_contents_len * sizeof(char));
    if (err_msg->message_contents == NULL) {
        perror("Memory allocation failed");
        free(buffer);
        free(err_msg);
        exit(EXIT_FAILURE);
    }
    strcpy(err_msg->message_contents, (char*)&buffer[offset]);

    return err_msg;
}

struct Bye_MSG *deserialize_bye_msg(uint8_t *buffer, const size_t buffer_size)
{
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t) + 1*sizeof(uint8_t) + 1;

    if (buffer_size < min_size) {
        printf_debug_simple(COLOR_ERR, "bye msg buffer is smaller than needed");
        return NULL;
    }

    struct Bye_MSG *bye_msg = malloc(sizeof(struct Bye_MSG));
    if (bye_msg == NULL) {
        free(buffer);
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

    size_t display_name_len = strlen((char*)&buffer[offset])+1; 
    bye_msg->display_name = malloc(display_name_len * sizeof(char));
    if (bye_msg->display_name == NULL) {
        perror("Memory allocation failed");
        free(buffer);
        free(bye_msg);
        exit(EXIT_FAILURE);
    }
    strcpy(bye_msg->display_name, (char*)&buffer[offset]);

    return bye_msg;
}

struct Ping_MSG *deserialize_ping_msg(uint8_t *buffer, const size_t buffer_size)
{
    const size_t min_size = sizeof(uint8_t) + sizeof(uint16_t);

    if (buffer_size < min_size) {
        printf_debug_simple(COLOR_ERR, "ping msg buffer is smaller than needed");
        return NULL;
    }

    struct Ping_MSG *ping_msg = malloc(sizeof(struct Ping_MSG));
    if (ping_msg == NULL) {
        free(buffer);
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
