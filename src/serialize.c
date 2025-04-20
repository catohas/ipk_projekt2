#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "./messages.h"
#include "./serialize.h" 

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
