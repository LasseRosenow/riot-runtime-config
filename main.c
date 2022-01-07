#include <string.h>
#include <shell.h>
#include <msg.h>
#include <stdio.h>

#include "registry_coap.h"
#include "registry.h"
#include "registry_schema_rgb.h"
#include "registry_schema_test.h"
#include "tests.h"
#include "storage_facility_dummy.h"
#include "cbor_example.h"

#include "assert.h"

#define SHELL_QUEUE_SIZE (8)
static msg_t _shell_queue[SHELL_QUEUE_SIZE];

static const shell_command_t shell_commands[] = {
    { "registry_coap", "Registry CoAP cli", registry_coap_cli_cmd },
    // { "registry_lwm2m", "Registry LwM2M cli", registry_lwm2m_cli_cmd },
    { NULL, NULL, NULL }
};

registry_schema_rgb_t rgb_instance_1_data = {
    .r = 0,
    .g = 255,
    .b = 70,
};
registry_instance_t rgb_instance_1 = {
    .name = "rgb-1",
    .data = &rgb_instance_1_data,
};

registry_schema_rgb_t rgb_instance_2_data = {
    .r = 90,
    .g = 4,
    .b = 0,
};
registry_instance_t rgb_instance_2 = {
    .name = "rgb-2",
    .data = &rgb_instance_2_data,
};

registry_schema_rgb_t rgb_instance_3_data = {
    .r = 7,
    .g = 8,
    .b = 9,
};
registry_instance_t rgb_instance_3 = {
    .name = "rgb-3",
    .data = &rgb_instance_3_data,
};


registry_store_t dummy_store = {
    .itf = &dummy_store_itf,
};

int _export_func(const int *path, int path_len, registry_schema_item_t *meta, char *val,
                 void *context)
{
    (void)meta;
    (void)context;

    printf("Exporting: ");

    for (int i = 0; i < path_len; i++) {
        printf("/%d", path[i]);
    }

    printf(" - %s\n", val);

    return 0;
}

int main(void)
{
    /* init registry */
    registry_init();

    /* add application registry schema */
    registry_register_schema(&registry_schema_rgb);

    /* register storage destination */
    registry_register_storage_src(&dummy_store);
    registry_register_storage_dst(&dummy_store);

    /* add schema instances */
    registry_add_instance(registry_schema_rgb.id, &rgb_instance_1);
    registry_add_instance(registry_schema_rgb.id, &rgb_instance_2);
    registry_add_instance(registry_schema_rgb.id, &rgb_instance_3);

    /* for the thread running the shell */
    //registry_coap_cli_init();
    //registry_lwm2m_cli_init();

    /* Test some exports */
    int path[] = { registry_schema_rgb.id, 2, 1 };

    registry_export(_export_func, path, ARRAY_SIZE(path));

    registry_save();
    registry_load();

    /* test registry */
    tests_run();

    cbor_example_run();

    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
