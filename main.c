#include <string.h>
#include <shell.h>
#include <msg.h>

#include "registry_coap.h"
#include "registry_lwm2m.h"
#include "registry.h"
#include "registry_schema_rgb.h"
#include "registry_schema_test.h"
#include "tests.h"

#include "assert.h"

#define SHELL_QUEUE_SIZE (8)
static msg_t _shell_queue[SHELL_QUEUE_SIZE];

static const shell_command_t shell_commands[] = {
    { "registry_coap", "Registry CoAP cli", registry_coap_cli_cmd },
    { "registry_lwm2m", "Registry LwM2M cli", registry_lwm2m_cli_cmd },
    { NULL, NULL, NULL }
};

registry_schema_rgb_t rgb_instance_1 = {
    .r = 0,
    .g = 255,
    .b = 70,
};

registry_schema_rgb_t rgb_instance_2 = {
    .r = 90,
    .g = 4,
    .b = 0,
};

registry_schema_rgb_t rgb_instance_3 = {
    .r = 7,
    .g = 8,
    .b = 9,
};

int _export_func(const int *path, int path_len, registry_schema_item_t *meta, char* val, void *context) {
    (void) meta;
    (void) context;

    printf("Exporting: ");

    for (int i = 0; i < path_len; i++) {
        printf("/%d", path[i]);
    }

    printf(" - %s\n", val);

    return 0;
}

int main(void) {
    /* init registry */
    registry_init();

    /* add application registry schema */
    registry_register_schema(&registry_schema_rgb);

    /* add schema instances */
    registry_add_instance(registry_schema_rgb.id, &rgb_instance_1.node);
    registry_add_instance(registry_schema_rgb.id, &rgb_instance_2.node);
    registry_add_instance(registry_schema_rgb.id, &rgb_instance_3.node);

    /* for the thread running the shell */
    //registry_coap_cli_init();
    //registry_lwm2m_cli_init();

    /* Test some exports */
    int path[] = {registry_schema_rgb.id, 2, 1};
    registry_export(_export_func, path, ARRAY_SIZE(path));

    /* test registry */
    tests_run();

    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
