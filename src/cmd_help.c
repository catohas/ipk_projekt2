#include <stdio.h>

#include "./commands.h"

void cmd_help(void)
{
    fprintf(stderr, "flags: -t -s -p -d -r -h\n");
    fprintf(stderr, "available commands: /auth /join /rename /help\n");
}
