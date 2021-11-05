#include <string.h>
#include <shell.h>
#include <msg.h>

#include "registry_coap.h"
#include "registry_lwm2m.h"
#include "registry.h"
#include "registry_schema_rgb.h"
#include "registry_schema_test.h"

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

registry_schema_test_t test_instance_1 = {
    .i8 = 8,
    .i16 = 16,
    .i32 = 32,
    .i64 = 64,
    .f32 = 3.2,
    .boolean = true,
    .string = "hallo",
};

int main(void) {
    /* init registry */
    registry_init();

    /* add application registry schema */
    registry_register(&registry_schema_rgb);
    registry_register(&registry_schema_test);

    /* add schema instances */
    registry_add_instance(registry_schema_rgb.id, &rgb_instance_1.node);
    registry_add_instance(registry_schema_rgb.id, &rgb_instance_2.node);
    registry_add_instance(registry_schema_rgb.id, &rgb_instance_3.node);
    registry_add_instance(registry_schema_test.id, &test_instance_1.node);

    // test rgb_schema
    clist_node_t *rgb_node = registry_schema_rgb.instances.next->next;
    registry_schema_rgb_t* rgb_instance = container_of(rgb_node, registry_schema_rgb_t, node);
    printf("Test: registry_schema_rgb: Get \"b\" of first: %d\n", rgb_instance->b);

    // test test_schema
    clist_node_t *test_node = registry_schema_test.instances.next->next;
    registry_schema_test_t* test_instance = container_of(test_node, registry_schema_test_t, node);
    printf("Test: registry_schema_test: Get \"string\" of first: %s\n", test_instance->string);

    // test get
    char buf[REGISTRY_MAX_VAL_LEN];
    int path[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_FLOAT};
    registry_get_value(path, ARRAY_SIZE(path), buf, REGISTRY_MAX_VAL_LEN);
    printf("RESULT: %s\n", buf);

    // test set
    registry_set_value(path, ARRAY_SIZE(path), "7.9");

    registry_get_value(path, ARRAY_SIZE(path), buf, REGISTRY_MAX_VAL_LEN);
    printf("RESULT: %s\n", buf);

    /* for the thread running the shell */
    //registry_coap_cli_init();
    //registry_lwm2m_cli_init();

    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
