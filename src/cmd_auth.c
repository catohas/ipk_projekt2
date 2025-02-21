#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

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

void cmd_auth(void)
{
    char username[MAX_USERNAME_LEN] = {0};
    char display_name[MAX_DISPLAY_NAME_LEN] = {0};
    char secret[MAX_SECRET_LEN] = {0};

    line = strtok(NULL, " "); // get rid of /auth
    if (line == NULL) {
        fprintf(stderr, "Please provide 'username' 'display_name' 'secret'\n");
        return;
    }
    memcpy(username, line, MAX_USERNAME_LEN);

    line = strtok(NULL, " ");
    if (line == NULL) {
        fprintf(stderr, "Please provide 'username' 'display_name' 'secret'\n");
        return;
    }
    memcpy(display_name, line, MAX_DISPLAY_NAME_LEN);

    line = strtok(NULL, " ");
    if (line == NULL) {
        fprintf(stderr, "Please provide 'username' 'display_name' 'secret'\n");
        return;
    }
    memcpy(secret, line, MAX_SECRET_LEN);

    struct Auth_MSG auth_msg;
    create_auth_msg(&auth_msg, username, display_name, secret);
    size_t buffer_size;
    uint8_t *in_buffer = serialize_auth_msg(&auth_msg, &buffer_size);

    // struct Confirm_MSG confirm_msg;
    // create_confirm_msg(&confirm_msg, 255);
    // size_t buffer_size;
    // uint8_t *in_buffer = serialize_confirm_msg(&confirm_msg, &buffer_size);

    size_t out_buffer_size = sizeof(uint8_t)*MAX_PACKET_SIZE;
    uint8_t *out_buffer = malloc(out_buffer_size);

    if (out_buffer == NULL) {
        perror("failed to allocate receive buffer");
        exit(EXIT_FAILURE);
    }

    uint8_t *recv_buffer = send_network_msg_udp(in_buffer, buffer_size);

    // struct Auth_MSG *a_msg = deserialize_auth_msg(recv_buffer, buffer_size);

    // printf_debug(COLOR_ERR, "%s", a_msg->username);
    // printf_debug(COLOR_ERR, "%s", a_msg->display_name);
    // printf_debug(COLOR_ERR, "%s", a_msg->secret);


    // free(in_buffer); in_buffer = NULL;
    // free(out_buffer); out_buffer = NULL;
    // free(recv_buffer); recv_buffer = NULL;
    // free_confirm_msg(con_msg);

    // fprintf(stderr, "Received response: %s'\n", recv_buffer);
    // for (int i = 0; i < 31; i++) {
    //     fprintf(stderr, "%c", recv_buffer[i]);
    // }
    // fprintf(stderr, "'\n");
    // for (int i = 0; i < 31; i++) {
    //     fprintf(stderr, "%02x ", (unsigned char)recv_buffer[i]);
    // } 
    // fprintf(stderr, "\n");
}
