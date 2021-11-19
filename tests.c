#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include "embUnit.h"
#include <inttypes.h>

#include "registry.h"
#include "registry_schema_test.h"

#include "assert.h"

#include "tests.h"
#include <fmt.h>
registry_schema_test_t test_instance_1 = {
    .string = "hallo",
    .boolean = true,
    .u8 = 9,
    .u16 = 17,
    .u32 = 33,
#if defined(CONFIG_REGISTRY_USE_UINT64)
    .u64 = 65,
#endif /* CONFIG_REGISTRY_USE_UINT64 */
    .i8 = 8,
    .i16 = 16,
    .i32 = 32,
#if defined(CONFIG_REGISTRY_USE_INT64)
    .i64 = 64,
#endif /* CONFIG_REGISTRY_USE_INT64 */
#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    .f32 = 3.2,
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */
#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    .f64 = 6.4,
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
};

static void _registry_set_and_get(int* path, int path_len, char* input, char* output_buf, int output_buf_len) {
    registry_set_value(path, path_len, input);
    registry_get_value(path, path_len, output_buf, output_buf_len);
}

static void test_registry_setup(void) {
    /* init registry */
    registry_init();

    /* add application registry schema */
    registry_register(&registry_schema_test);

    /* add schema instances */
    registry_add_instance(registry_schema_test.id, &test_instance_1.node);
}

static void test_registry_teardown(void) {
    
}

static void tests_registry_register(void) {
    // test if schema_test got registered
    clist_node_t *test_node = registry_schema_test.instances.next->next;
    registry_schema_test_t* test_instance = container_of(test_node, registry_schema_test_t, node);
    TEST_ASSERT_EQUAL_INT((int)&test_instance_1, (int)test_instance);
}

static void tests_registry_all_min_values(void) {
    char input_buf[REGISTRY_MAX_VAL_LEN] = {0};
    char output_buf[REGISTRY_MAX_VAL_LEN] = {0};

    // string
    int path_string[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_STRING};
    _registry_set_and_get(path_string, ARRAY_SIZE(path_string), "", output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING("", output_buf);

    // bool
    int path_bool[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_BOOL};
    _registry_set_and_get(path_bool, ARRAY_SIZE(path_bool), "0", output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING("0", output_buf);

    // u8
    int path_u8[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_U8};
    _registry_set_and_get(path_u8, ARRAY_SIZE(path_u8), "0", output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING("0", output_buf);

    // u16
    int path_u16[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_U16};
    _registry_set_and_get(path_u16, ARRAY_SIZE(path_u16), "0", output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING("0", output_buf);

    // u32
    int path_u32[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_U32};
    _registry_set_and_get(path_u32, ARRAY_SIZE(path_u32), "0", output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING("0", output_buf);

    // u64
#if defined(CONFIG_REGISTRY_USE_UINT64)
    int path_u64[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_U64};
    _registry_set_and_get(path_u64, ARRAY_SIZE(path_u64), "0", output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING("0", output_buf);
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    // i8
    int path_i8[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_I8};
    sprintf(input_buf, "%" PRIi8, INT8_MIN);
    _registry_set_and_get(path_i8, ARRAY_SIZE(path_i8), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);

    // i16
    int path_i16[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_I16};
    sprintf(input_buf, "%" PRIi16, INT16_MIN);
    _registry_set_and_get(path_i16, ARRAY_SIZE(path_i16), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);

    // i32
    int path_i32[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_I32};
    sprintf(input_buf, "%" PRIi32, INT32_MIN);
    _registry_set_and_get(path_i32, ARRAY_SIZE(path_i32), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);

    // i64
#if defined(CONFIG_REGISTRY_USE_INT64)
    int path_i64[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_I64};
    sprintf(input_buf, "%" PRIi64, INT64_MIN);
    _registry_set_and_get(path_i64, ARRAY_SIZE(path_i64), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);
#endif /* CONFIG_REGISTRY_USE_INT64 */

    // f32
#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    int path_f32[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_F32};
    sprintf(input_buf, "%f", -FLT_MAX);
    _registry_set_and_get(path_f32, ARRAY_SIZE(path_f32), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

    // f64
#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    if (REGISTRY_MAX_VAL_LEN >= (DBL_MAX_10_EXP + 1) + 1 + 1 + 6) { // (DBL_MAX_10_EXP + 1) + sign + dot + 6 decimal places
        int path_f64[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_F64};
        sprintf(input_buf, "%lf", -DBL_MAX);
        _registry_set_and_get(path_f64, ARRAY_SIZE(path_f64), input_buf, output_buf, ARRAY_SIZE(output_buf));
        TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);
    }
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
}

static void tests_registry_all_max_values(void) {
    char input_buf[REGISTRY_MAX_VAL_LEN] = {0};
    char output_buf[REGISTRY_MAX_VAL_LEN] = {0};

    // string
    int path_string[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_STRING};
    for (int i = 0; i < REGISTRY_MAX_VAL_LEN - 1; i++) {
        input_buf[i] = 'a';
    }
    _registry_set_and_get(path_string, ARRAY_SIZE(path_string), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);

    // bool
    int path_bool[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_BOOL};
    _registry_set_and_get(path_bool, ARRAY_SIZE(path_bool), "1", output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING("1", output_buf);

    // u8
    int path_u8[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_U8};
    sprintf(input_buf, "%" PRIu8, UINT8_MAX);
    _registry_set_and_get(path_u8, ARRAY_SIZE(path_u8), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);

    // u16
    int path_u16[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_U16};
    sprintf(input_buf, "%" PRIu16, UINT16_MAX);
    _registry_set_and_get(path_u16, ARRAY_SIZE(path_u16), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);

    // u32
    int path_u32[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_U32};
    sprintf(input_buf, "%" PRIu32, UINT32_MAX);
    _registry_set_and_get(path_u32, ARRAY_SIZE(path_u32), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);

    // u64
#if defined(CONFIG_REGISTRY_USE_UINT64)
    int path_u64[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_U64};
    sprintf(input_buf, "%" PRIu64, UINT64_MAX);
    _registry_set_and_get(path_u64, ARRAY_SIZE(path_u64), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    // i8
    int path_i8[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_I8};
    sprintf(input_buf, "%" PRIi8, INT8_MAX);
    _registry_set_and_get(path_i8, ARRAY_SIZE(path_i8), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);

    // i16
    int path_i16[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_I16};
    sprintf(input_buf, "%" PRIi16, INT16_MAX);
    _registry_set_and_get(path_i16, ARRAY_SIZE(path_i16), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);

    // i32
    int path_i32[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_I32};
    sprintf(input_buf, "%" PRIi32, INT32_MAX);
    _registry_set_and_get(path_i32, ARRAY_SIZE(path_i32), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);

    // i64
#if defined(CONFIG_REGISTRY_USE_INT64)
    int path_i64[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_I64};
    sprintf(input_buf, "%" PRIi64, INT64_MAX);
    _registry_set_and_get(path_i64, ARRAY_SIZE(path_i64), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);
#endif /* CONFIG_REGISTRY_USE_INT64 */

    // f32
#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    int path_f32[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_F32};
    sprintf(input_buf, "%f", FLT_MAX);
    _registry_set_and_get(path_f32, ARRAY_SIZE(path_f32), input_buf, output_buf, ARRAY_SIZE(output_buf));
    TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

    // f64
#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    if (REGISTRY_MAX_VAL_LEN >= (DBL_MAX_10_EXP + 1) + 1 + 6) { // (DBL_MAX_10_EXP + 1) + dot + 6 decimal places
        int path_f64[] = {registry_schema_test.id, 0, REGISTRY_SCHEMA_TEST_F64};
        sprintf(input_buf, "%lf", DBL_MAX);
        _registry_set_and_get(path_f64, ARRAY_SIZE(path_f64), input_buf, output_buf, ARRAY_SIZE(output_buf));
        TEST_ASSERT_EQUAL_STRING(input_buf, output_buf);
    }
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
}

Test *tests_registry(void) {
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(tests_registry_register),
        new_TestFixture(tests_registry_all_min_values),
        new_TestFixture(tests_registry_all_max_values),
    };

    EMB_UNIT_TESTCALLER(registry_tests, test_registry_setup, test_registry_teardown, fixtures);

    return (Test *)&registry_tests;
}

int tests_run(void) {
    TESTS_START();
    TESTS_RUN(tests_registry());
    TESTS_END();
    return 0;
}
