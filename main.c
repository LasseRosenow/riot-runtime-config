#include <string.h>
#include <shell.h>
#include <msg.h>

#include "registry_coap.h"
#include "registry_lwm2m.h"
#include "registry.h"
#include "application_registry_schema.h"
#include "application_registry_schema_2.h"

#define SHELL_QUEUE_SIZE (8)
static msg_t _shell_queue[SHELL_QUEUE_SIZE];

static const shell_command_t shell_commands[] = {
    { "registry_coap", "Registry CoAP cli", registry_coap_cli_cmd },
    { "registry_lwm2m", "Registry LwM2M cli", registry_lwm2m_cli_cmd },
    { NULL, NULL, NULL }
};

my_schema_t my_schema_instance_1 = {
    .is_enabled = false,
    .threshold = 30,
    .name = "Gollum",
};

my_schema_t my_schema_instance_2 = {
    .is_enabled = true,
    .threshold = 9999,
    .name = "Smeagol",
};

my_schema_t my_schema_instance_3 = {
    .is_enabled = false,
    .threshold = 0,
    .name = "Frodo",
};

my_schema_2_t my_schema_2_instance_1 = {
    .i8 = 8,
    .i16 = 16,
    .i32 = 32,
    .boolean = true,
    .string = "hallo",
};

int main(void) {
    /* init registry */
    registry_init();

    /* add application registry schema */
    registry_register(&my_schema);
    registry_register(&my_schema_2);

    /* add schema instances */
    printf("my_schema: Add instance -> index: %d\n", registry_add_instance(my_schema.id, &my_schema_instance_1.node));
    printf("my_schema: Add instance -> index: %d\n", registry_add_instance(my_schema.id, &my_schema_instance_2.node));
    printf("my_schema: Add instance -> index: %d\n", registry_add_instance(my_schema.id, &my_schema_instance_3.node));
    printf("my_schema_2: Add instance -> index: %d\n", registry_add_instance(my_schema_2.id, &my_schema_2_instance_1.node));

    // test my_schema
    clist_node_t *my_schema_node = my_schema.instances.next->next;
    my_schema_t* my_schema_instance = container_of(my_schema_node, my_schema_t, node);
    printf("Test: my_schema: Get \"name\" of first: %s\n", my_schema_instance->name);

    // test my_schema_2
    clist_node_t *my_schema_2_node = my_schema_2.instances.next->next;
    my_schema_2_t* my_schema_2_instance = container_of(my_schema_2_node, my_schema_2_t, node);
    printf("Test: my_schema_2: Get \"string\" of first: %s\n", my_schema_2_instance->string);

    // test get
    char buf[REGISTRY_MAX_VAL_LEN];
    int path[] = {my_schema.id, 0, 2};
    registry_get_value(path, 3, buf, REGISTRY_MAX_VAL_LEN);
    printf("RESULT: %s\n", buf);

    // test set
    registry_set_value(path, 3, "Banaana");

    registry_get_value(path, 3, buf, REGISTRY_MAX_VAL_LEN);
    printf("RESULT: %s\n", buf);

    /* for the thread running the shell */
    //registry_coap_cli_init();
    //registry_lwm2m_cli_init();

    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
