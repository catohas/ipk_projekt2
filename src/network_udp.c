#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "./debug.h"
#include "./global.h"
#include "./messages.h"
#include "./network.h"
#include "./serialize.h"

void send_network_msg_udp(uint8_t *buffer, size_t buffer_size)
{
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
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        return;
    }

    printf_debug(COLOR_INFO, "size of struct before sending: %zu", buffer_size);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = udp_timeout*1000; // msec to usec
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int retries = 0;
    while (retries < udp_retransmissions) {
        if ((numbytes = sendto(sockfd, buffer, buffer_size, 0, p->ai_addr, p->ai_addrlen)) == -1) {
            perror("client: sendto");
            exit(EXIT_FAILURE);
        }
        else {
            printf_debug(COLOR_INFO, "sent %d bytes to %s", numbytes, hostname);

            socklen_t addr_len = sizeof(hostname);
            char test_buffer[32] = {0};
            int n = recvfrom(sockfd, test_buffer, sizeof(test_buffer), 0, p->ai_addr, &(p->ai_addrlen));
            if (n >= 0) {
                // test_buffer[n] = '\0';
                fprintf(stderr, "Received response: %s\n", test_buffer);
                fprintf(stderr, "'");
                for (int i = 0; i < 30; i++) {
                    fprintf(stderr, "%c", test_buffer[i]);
                }
                fprintf(stderr, "'\n");
                for (int i = 0; i < 30; i++) {
                    fprintf(stderr, "%02x ", (unsigned char)test_buffer[i]);
                } 
                fprintf(stderr, "\n");
                return;
            }
            else {
                perror("recvfrom");
            }

        }
        retries++;
        printf_debug(COLOR_INFO, "retrying... (%d/%d)", retries, udp_retransmissions);
    }

    free(buffer);
}
