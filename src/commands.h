// VUT FIT - IPK Project 2
// Author: Petr Hron (xhronpe00)
// File: commands.h
// February 2025

#ifndef COMMANDS_H__
#define COMMANDS_H__

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void cmd_msg(void);
void cmd_auth(void);
void cmd_join(void);
void cmd_rename(void);
void cmd_help(void);

static const char *commands[] = {
    "/auth",
    "/join",
    "/rename",
    "/help"
};

static void (*command_functions[])(void) = {
    cmd_auth,
    cmd_join,
    cmd_rename,
    cmd_help
};

#endif
