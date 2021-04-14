#include <string.h>
#include <shell.h>

#include "cbor_util.h"

extern int coap_cli_cmd(int argc, char **argv);

extern void coap_cli_init(void);

static const shell_command_t shell_commands[] = {
        {"coap", "CoAP example", coap_cli_cmd},
        {NULL,   NULL,           NULL}
};

int main(void) {
    /* For the thread running the shell */
    coap_cli_init();

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}

