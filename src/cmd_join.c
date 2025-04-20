#include <stdio.h>
#include <string.h>

#include "./commands.h"
#include "./debug.h"
#include "./global.h"
#include "./network.h"
#include "./messages.h"
#include "./serialize.h"

void cmd_join(void)
{
    printf_debug_simple(COLOR_INFO, "executing cmd_join");

    char channel_id[MAX_CHANNEL_ID_LEN] = {0};

    line = strtok(NULL, " "); // get rid of /join
    if (line == NULL) {
        fprintf(stderr, "Please provide 'channel_id'\n");
        return;
    }
    memcpy(channel_id, line, MAX_CHANNEL_ID_LEN);

    state = STATE_JOIN;
    printf_debug_simple(COLOR_SUCCESS, "transitioned to state join");

    struct Join_MSG join_msg;
    create_join_msg(&join_msg, confirmed_msg_ids_index, channel_id, display_name);
    size_t buffer_size;
    uint8_t *in_buffer = serialize_join_msg(&join_msg, &buffer_size);

    send_network_msg_udp(in_buffer, buffer_size);
}
