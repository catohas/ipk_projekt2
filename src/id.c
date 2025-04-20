#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "./global.h"
#include "./debug.h"
#include "./id.h"

// add a message ID to the confirmed messages array
void add_confirmed_msg_id(uint16_t msg_id)
{
    printf_debug(COLOR_SUCCESS, "adding msg id %d", msg_id);

    for (size_t i = 0; i < confirmed_msg_ids_index; i++) {
        if (confirmed_msg_ids[i] == msg_id) {
            printf_debug(COLOR_ERR, "id: %d was already confirmed, ignoring...", msg_id);
            return;
        }
    }

    if (confirmed_msg_ids_index >= confirmed_msg_array_size) {
        // resize array to handle overflow
        confirmed_msg_array_size = confirmed_msg_array_size*2;
        uint16_t *new_array = realloc(confirmed_msg_ids, sizeof(uint16_t)*confirmed_msg_array_size);
        if (new_array == NULL) {
            fprintf(stderr, "failed to resize confirmed message IDs array\n");
            exit(EXIT_FAILURE);
        }
        confirmed_msg_ids = new_array;

        for (size_t i = confirmed_msg_ids_index; i < confirmed_msg_array_size; i++) {
            confirmed_msg_ids[i] = -1;
        }
    }
    
    // after sending index gets incremented by one, we are confirming the previous one
    confirmed_msg_ids[confirmed_msg_ids_index-1] = msg_id;
}

bool is_message_duplicate(uint16_t msg_id)
{
    for (size_t i = 0; i < seen_count; i++) {
        if (seen_ids[i] == msg_id) {
            return true;
        }
    }
    
    // if not found, add to the list
    if (seen_count < seen_ids_array_size) {
        seen_ids[seen_count] = msg_id;
        seen_count++;
    }
    else {
        // array full, resize
        seen_ids_array_size = seen_ids_array_size*2;
        uint16_t *new_array = realloc(seen_ids, sizeof(uint16_t)*seen_ids_array_size);
        if (new_array == NULL) {
            fprintf(stderr, "failed to resize seen message IDs array\n");
            exit(EXIT_FAILURE);
        }
        seen_ids = new_array;

        seen_ids[seen_count] = msg_id;
        seen_count++;
    }
    
    return false;
}
