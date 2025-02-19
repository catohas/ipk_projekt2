#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <string.h>
#include <stdlib.h>

#include "./commands.h"
#include "./debug.h"
#include "./maximums.h"
#include "./messages.h"
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

    struct Auth_MSG auth_msg;
    create_auth_msg(&auth_msg, username, display_name, secret);
    size_t buffer_size;
    uint8_t *buffer = serialize_auth_msg(&auth_msg, &buffer_size);

    printf_debug("size of struct: %zu", buffer_size);

    if ((numbytes = sendto(sockfd, buffer, buffer_size, 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("client: sendto");
        exit(EXIT_FAILURE);
    }

    free(buffer);

    printf_debug("sent %d bytes to %s", numbytes, hostname);

}
