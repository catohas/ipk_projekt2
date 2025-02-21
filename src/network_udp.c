#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "./debug.h"
#include "./global.h"
#include "./maximums.h"
#include "./messages.h"
#include "./network.h"
#include "./serialize.h"

uint8_t *send_network_msg_udp(uint8_t *in_buffer, size_t in_buffer_size)
{
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        free(in_buffer);
        exit(EXIT_FAILURE);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            free(in_buffer);
            exit(EXIT_FAILURE);
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        free(in_buffer);
        exit(EXIT_FAILURE);
    }

    printf_debug(COLOR_INFO, "size of struct before sending: %zu bytes", in_buffer_size);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = udp_timeout*1000; // msec to usec
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        free(in_buffer);
        exit(EXIT_FAILURE);
    }

    int total_tries = 0;
    while (total_tries <= udp_retransmissions) {
        printf_debug(COLOR_INFO, "try number: (%d/%d)", total_tries, udp_retransmissions);
        total_tries++;
        if ((numbytes = sendto(sockfd, in_buffer, in_buffer_size, 0, p->ai_addr, p->ai_addrlen)) == -1) {
            perror("client: sendto");
            free(in_buffer);
            exit(EXIT_FAILURE);
        }
        else {
            printf_debug(COLOR_INFO, "sent %d bytes to %s", numbytes, hostname);

            uint8_t recv_buffer[MAX_PACKET_SIZE] = {0};
            if (recvfrom(sockfd, recv_buffer, MAX_PACKET_SIZE, 0, p->ai_addr, &(p->ai_addrlen)) == -1) {
                perror("recvfrom");
                continue;
            }

            uint8_t *return_recv_buffer = malloc(sizeof(uint8_t)*MAX_PACKET_SIZE);
            if (return_recv_buffer == NULL) {
                perror("failed to allocate return buffer");
                free(in_buffer);
                exit(EXIT_FAILURE);
            }
            memcpy(return_recv_buffer, recv_buffer, MAX_PACKET_SIZE);

            free(in_buffer);

            return return_recv_buffer;
        }
    }

    free(in_buffer);
    return NULL;
}
