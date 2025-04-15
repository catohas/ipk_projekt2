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
    create_auth_msg(&auth_msg, confirmed_msg_ids_amount, username, display_name, secret);
    size_t buffer_size;
    uint8_t *in_buffer = serialize_auth_msg(&auth_msg, &buffer_size);

    send_network_msg_udp(in_buffer, buffer_size);
}
