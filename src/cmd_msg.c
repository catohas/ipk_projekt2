#include <stdio.h>

#include "./commands.h"
#include "./debug.h"

void cmd_msg(void)
{
    printf_debug_simple(COLOR_INFO, "in cmd_msg function");
}
