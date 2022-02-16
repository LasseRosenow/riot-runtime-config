#include <string.h>
#include <shell.h>
#include <msg.h>
#include <stdio.h>

#include "registry_coap.h"
#include "registry.h"
#include "registry_schemas.h"
#include "registry_cli.h"
#include "tests.h"
#include "storage_facility_dummy.h"
#include "cbor_example.h"
#include "assert.h"

#define SHELL_QUEUE_SIZE (8)
static msg_t _shell_queue[SHELL_QUEUE_SIZE];

static const shell_command_t shell_commands[] = {
    { "registry_coap", "Registry CoAP cli", registry_coap_cli_cmd },
    { "registry", "Registry cli", registry_cli_cmd },
    // { "registry_lwm2m", "Registry LwM2M cli", registry_lwm2m_cli_cmd },
    { NULL, NULL, NULL }
};

registry_schema_rgb_led_t rgb_led_instance_1_data = {
    .red = 0,
    .green = 255,
    .blue = 70,
};
registry_instance_t rgb_led_instance_1 = {
    .name = "rgb-1",
    .data = &rgb_led_instance_1_data,
};

registry_schema_rgb_led_t rgb_led_instance_2_data = {
    .red = 90,
    .green = 4,
    .blue = 0,
};
registry_instance_t rgb_led_instance_2 = {
    .name = "rgb-2",
    .data = &rgb_led_instance_2_data,
};

registry_schema_rgb_led_t rgb_led_instance_3_data = {
    .red = 7,
    .green = 8,
    .blue = 9,
};
registry_instance_t rgb_led_instance_3 = {
    .name = "rgb-3",
    .data = &rgb_led_instance_3_data,
};


registry_store_t dummy_store = {
    .itf = &dummy_store_itf,
};

int main(void)
{
    /* init registry */
    registry_init();
    registry_schemas_init();

    /* register store source and destination */
    registry_store_register_src(&dummy_store);
    registry_store_register_dst(&dummy_store);

    /* add schema instances */
    registry_add_instance(registry_schema_rgb_led.id, &rgb_led_instance_1);
    registry_add_instance(registry_schema_rgb_led.id, &rgb_led_instance_2);
    registry_add_instance(registry_schema_rgb_led.id, &rgb_led_instance_3);

    /* for the thread running the shell */
    //registry_coap_cli_init();
    //registry_lwm2m_cli_init();

    //registry_store_save();
    //registry_store_load();

    /* test registry */
    tests_run();

    cbor_example_run();

    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
