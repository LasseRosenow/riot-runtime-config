#include <string.h>
#include <shell.h>
#include <msg.h>
#include "embUnit.h"

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
    .boolean = true,
    .string = "hallo",
    .u8 = 9,
    .u16 = 17,
    .u32 = 33,
    .u64 = 65,
    .i8 = 8,
    .i16 = 16,
    .i32 = 32,
    .i64 = 64,
    .f32 = 3.2,
    .f64 = 6.4,
};

static void test_registry_setup(void) {
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
}

static void test_registry_teardown(void) {
    
}

static void tests_registry_register(void) {
    // test rgb_schema
    clist_node_t *rgb_node = registry_schema_rgb.instances.next->next;
    registry_schema_rgb_t* rgb_instance = container_of(rgb_node, registry_schema_rgb_t, node);
    TEST_ASSERT_EQUAL_INT(0, rgb_instance->r);
    TEST_ASSERT_EQUAL_INT(255, rgb_instance->g);
    TEST_ASSERT_EQUAL_INT(70, rgb_instance->b);

    // test test_schema
    clist_node_t *test_node = registry_schema_test.instances.next->next;
    registry_schema_test_t* test_instance = container_of(test_node, registry_schema_test_t, node);
    TEST_ASSERT_EQUAL_STRING("hallo", test_instance->string);

    // test get
    char buf[REGISTRY_MAX_VAL_LEN];
    int path[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_I32};
    registry_get_value(path, ARRAY_SIZE(path), buf, REGISTRY_MAX_VAL_LEN);
    TEST_ASSERT_EQUAL_STRING("32", buf);

    int path_f32[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_F32};
    registry_get_value(path_f32, ARRAY_SIZE(path_f32), buf, REGISTRY_MAX_VAL_LEN);
    TEST_ASSERT_EQUAL_STRING("3.2", buf);

    // test set
    registry_set_value(path_f32, ARRAY_SIZE(path_f32), "7.9");

    registry_get_value(path_f32, ARRAY_SIZE(path_f32), buf, REGISTRY_MAX_VAL_LEN);
    TEST_ASSERT_EQUAL_STRING("7.9", buf);
}

Test *tests_registry(void) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(tests_registry_register),
    };

    EMB_UNIT_TESTCALLER(registry_tests, test_registry_setup, test_registry_teardown, fixtures);

    return (Test *)&registry_tests;
}

int main(void) {
    TESTS_START();
    TESTS_RUN(tests_registry());
    TESTS_END();

    /* for the thread running the shell */
    //registry_coap_cli_init();
    //registry_lwm2m_cli_init();

    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
