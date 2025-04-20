// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: cmd_auth.c
// April 2025

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "./commands.h"
#include "./debug.h"
#include "./deserialize.h"
#include "./maximums.h"
#include "./messages.h"
#include "./network.h"
#include "./global.h"
#include "./serialize.h"
#include "./validation.h"

void cmd_auth(void)
{
    printf_debug_simple(COLOR_INFO, "executing cmd_auth");

    char username[MAX_USERNAME_LEN+1] = {0};
    char secret[MAX_SECRET_LEN+1] = {0};
    char temp_display_name[MAX_DISPLAY_NAME_LEN+1] = {0};

    line = strtok(NULL, " "); // get rid of /auth
    if (line == NULL) {
        fprintf(stderr, "Please provide 'username' 'secret' 'display_name'\n");
        return;
    }
    strncpy(username, line, MAX_USERNAME_LEN);

    line = strtok(NULL, " ");
    if (line == NULL) {
        fprintf(stderr, "Please provide 'username' 'secret' 'display_name'\n");
        return;
    }
    strncpy(secret, line, MAX_SECRET_LEN);

    line = strtok(NULL, " ");
    if (line == NULL) {
        fprintf(stderr, "Please provide 'username' 'secret' 'display_name'\n");
        return;
    }
    strncpy(temp_display_name, line, MAX_DISPLAY_NAME_LEN);

    if (!validate_id(username)) {
        printf("ERROR: Invalid username. Must be 1-20 characters from [a-zA-Z0-9_-]\n");
        return;
    }

    if (!validate_secret(secret)) {
        printf("ERROR: Invalid secret. Must be 1-128 characters from [a-zA-Z0-9_-]\n");
        return;
    }

    if (!validate_display_name(temp_display_name)) {
        printf("ERROR: Invalid display name. Must be 1-20 printable characters\n");
        return;
    }

    memset(display_name, 0, MAX_DISPLAY_NAME_LEN);
    strcpy(display_name, temp_display_name);

    if (use_tcp_protocol) {
        state = STATE_AUTH; 
        printf_debug_simple(COLOR_SUCCESS, "transitioned to state auth");
        send_tcp_auth_msg(username, display_name, secret);
    }
    else {
        struct Auth_MSG auth_msg;
        create_auth_msg(&auth_msg, confirmed_msg_ids_index, username, display_name, secret);
        size_t buffer_size;
        uint8_t *in_buffer = serialize_auth_msg(&auth_msg, &buffer_size);
        send_network_msg_udp(in_buffer, buffer_size);
    }

}
