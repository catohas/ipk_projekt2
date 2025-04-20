// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: cmd_help.c
// April 2025

#include <stdio.h>

#include "./debug.h"
#include "./commands.h"

void cmd_help(void)
{
    printf_debug_simple(COLOR_INFO, "executing cmd_help");

    printf("IPK25-CHAT Client\n");
    printf("=================\n\n");
    
    printf("USAGE: ./ipk25chat-client [OPTIONS]\n\n");
    
    printf("Command line options:\n");
    printf("  -t <protocol>    Transport protocol (tcp|udp) [REQUIRED]\n");
    printf("  -s <host>        Server hostname or IP address [REQUIRED]\n");
    printf("  -p <port>        Server port (default: 4567)\n");
    printf("  -d <timeout>     UDP confirmation timeout in milliseconds (default: 250)\n");
    printf("  -r <retries>     Maximum UDP retransmissions (default: 3)\n");
    printf("  -h               Display this help message\n\n");
    
    printf("Available commands:\n");
    printf("  /auth <username> <secret> <display_name>    Authenticate with the server\n");
    printf("  /join <channel_id>                          Join a specific channel\n");
    printf("  /rename <display_name>                      Change your display name\n");
    printf("  /help                                       Display this help message\n\n");
    
    printf("Notes:\n");
    printf("- You must authenticate with /auth before sending messages or joining channels\n");
    printf("- Any input not starting with '/' is sent as a message to the current channel\n");
    printf("- Press Ctrl+C or Ctrl+D to exit the program\n");
}
