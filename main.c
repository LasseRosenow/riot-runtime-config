#include <string.h>
#include <shell.h>
#include <msg.h>

#include "registry_coap.h"
#include "registry_lwm2m.h"

#define SHELL_QUEUE_SIZE (8)
static msg_t _shell_queue[SHELL_QUEUE_SIZE];

static const shell_command_t shell_commands[] = {
        {"coap", "Registry CoAP cli", registry_coap_cli_cmd},
        { "lwm2m", "Start registry LwM2M client", registry_lwm2m_cli_cmd },
        {NULL, NULL, NULL}
};

int main(void) {
//    /* For the thread running the shell */
    registry_coap_cli_init();
    registry_lwm2m_cli_init();

    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}

