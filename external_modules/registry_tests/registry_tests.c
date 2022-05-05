#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>
#include "embUnit.h"
#include "fmt.h"
#include "assert.h"
#include "registry.h"
#include "registry_schemas.h"

#include "registry_tests.h"

int test_instance_0_commit_cb(const registry_path_t path, void *context)
{
    (void)context;
    printf("Test instance commit_cb was executed: %d", *path.root_group_id);
    if (path.schema_id) {
        printf("/%d", *path.schema_id);
    }
    if (path.instance_id) {
        printf("/%d", *path.instance_id);
    }
    printf("\n");
    return 0;
}

registry_schema_types_test_t test_instance_1_data = {
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

registry_instance_t test_instance_1 = {
    .name = "test-1",
    .data = &test_instance_1_data,
    .commit_cb = &test_instance_0_commit_cb,
};

static void test_registry_setup(void)
{
    /* init registry */
    registry_init();
    registry_schemas_init();

    /* add schema instances */
    registry_add_instance(REGISTRY_ROOT_GROUP_SYS, REGISTRY_SCHEMA_TYPES_TEST, &test_instance_1);
}

static void test_registry_teardown(void)
{}

static void tests_registry_register_schema(void)
{
    // test if schema_types_test got registered
    clist_node_t *test_node = registry_schema_types_test.instances.next->next;
    registry_schema_types_test_t *test_instance = container_of(test_node,
                                                               registry_schema_types_test_t, node);

    TEST_ASSERT_EQUAL_INT((int)&test_instance_1, (int)test_instance);
}

static void tests_registry_all_min_values(void)
{
    registry_path_t path;

    // string
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_STRING);
    char ouput_string[REGISTRY_MAX_VAL_LEN] = { 0 };

    registry_set_string(path, "");
    registry_get_string(path, ouput_string, ARRAY_SIZE(ouput_string));

    TEST_ASSERT_EQUAL_STRING("", ouput_string);

    // bool
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_BOOL);

    registry_set_bool(path, false);
    bool output_bool = registry_get_bool(path);

    TEST_ASSERT_EQUAL_INT(false, output_bool);

    // u8
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_U8);

    registry_set_uint8(path, 0);
    uint8_t output_u8 = registry_get_uint8(path);

    TEST_ASSERT_EQUAL_INT(0, output_u8);

    // u16
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_U16);

    registry_set_uint16(path, 0);
    uint16_t output_u16 = registry_get_uint16(path);

    TEST_ASSERT_EQUAL_INT(0, output_u16);

    // u32
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_U32);

    registry_set_uint32(path, 0);
    uint32_t output_u32 = registry_get_uint32(path);

    TEST_ASSERT_EQUAL_INT(0, output_u32);

    // u64
#if defined(CONFIG_REGISTRY_USE_UINT64)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_U64);
    registry_set_uint64(path, 0);
    uint64_t output_u64 = registry_get_uint64(path);
    TEST_ASSERT_EQUAL_INT(0, output_u64);
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    // i8
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_I8);
    registry_set_int8(path, INT8_MIN);
    int8_t output_i8 = registry_get_int8(path);
    TEST_ASSERT_EQUAL_INT(INT8_MIN, output_i8);

    // i16
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_I16);
    registry_set_int16(path, INT16_MIN);
    int16_t output_i16 = registry_get_int16(path);
    TEST_ASSERT_EQUAL_INT(INT16_MIN, output_i16);

    // i32
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_I32);
    registry_set_int32(path, INT32_MIN);
    int32_t output_i32 = registry_get_int32(path);
    TEST_ASSERT_EQUAL_INT(INT32_MIN, output_i32);

    // i64
#if defined(CONFIG_REGISTRY_USE_INT64)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_I64);
    registry_set_int64(path, INT64_MIN);
    int64_t output_i64 = registry_get_int64(path);
    TEST_ASSERT_EQUAL_INT(INT64_MIN, output_i64);
#endif /* CONFIG_REGISTRY_USE_INT64 */

    // f32
#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_F32);
    registry_set_float32(path, -FLT_MAX);
    float output_f32 = registry_get_float32(path);
    char input_f32_string[REGISTRY_MAX_VAL_LEN] = { 0 };
    char output_f32_string[REGISTRY_MAX_VAL_LEN] = { 0 };
    sprintf(input_f32_string, "%f", -FLT_MAX);
    sprintf(output_f32_string, "%f", output_f32);
    TEST_ASSERT_EQUAL_STRING(input_f32_string, output_f32_string);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

    // f64
#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    if (REGISTRY_MAX_VAL_LEN >= (DBL_MAX_10_EXP + 1) + 1 + 1 + 6) { // (DBL_MAX_10_EXP + 1) + sign + dot + 6 decimal places
        path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_F64);
        registry_set_float64(path, -DBL_MAX);
        float output_f64 = registry_get_float64(path);
        char input_f64_string[REGISTRY_MAX_VAL_LEN] = { 0 };
        char output_f64_string[REGISTRY_MAX_VAL_LEN] = { 0 };
        sprintf(input_f64_string, "%f", -DBL_MAX);
        sprintf(output_f64_string, "%f", output_f64);
        TEST_ASSERT_EQUAL_STRING(input_f64_string, output_f64_string);
    }
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
}

static void tests_registry_all_max_values(void)
{
    registry_path_t path;

    // string
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_STRING);
    char input_string[REGISTRY_MAX_VAL_LEN] = { 0 };
    char ouput_string[REGISTRY_MAX_VAL_LEN] = { 0 };

    for (int i = 0; i < REGISTRY_MAX_VAL_LEN - 1; i++) {
        input_string[i] = 'a';
    }

    registry_set_string(path, input_string);
    registry_get_string(path, ouput_string, ARRAY_SIZE(ouput_string));
    TEST_ASSERT_EQUAL_STRING(input_string, ouput_string);

    // bool
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_BOOL);

    registry_set_bool(path, true);
    bool output_bool = registry_get_bool(path);

    TEST_ASSERT_EQUAL_INT(true, output_bool);

    // u8
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_U8);

    registry_set_uint8(path, UINT8_MAX);
    uint8_t output_u8 = registry_get_uint8(path);

    TEST_ASSERT_EQUAL_INT(UINT8_MAX, output_u8);

    // u16
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_U16);

    registry_set_uint16(path, UINT16_MAX);
    uint16_t output_u16 = registry_get_uint16(path);

    TEST_ASSERT_EQUAL_INT(UINT16_MAX, output_u16);

    // u32
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_U32);

    registry_set_uint32(path, UINT32_MAX);
    uint32_t output_u32 = registry_get_uint32(path);

    TEST_ASSERT_EQUAL_INT(UINT32_MAX, output_u32);

    // u64
#if defined(CONFIG_REGISTRY_USE_UINT64)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_U64);
    registry_set_uint64(path, UINT64_MAX);
    uint64_t output_u64 = registry_get_uint64(path);
    TEST_ASSERT_EQUAL_INT(UINT64_MAX, output_u64);
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    // i8
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_I8);
    registry_set_int8(path, INT8_MAX);
    int8_t output_i8 = registry_get_int8(path);
    TEST_ASSERT_EQUAL_INT(INT8_MAX, output_i8);

    // i16
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_I16);
    registry_set_int16(path, INT16_MAX);
    int16_t output_i16 = registry_get_int16(path);
    TEST_ASSERT_EQUAL_INT(INT16_MAX, output_i16);

    // i32
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_I32);
    registry_set_int32(path, INT32_MAX);
    int32_t output_i32 = registry_get_int32(path);
    TEST_ASSERT_EQUAL_INT(INT32_MAX, output_i32);

    // i64
#if defined(CONFIG_REGISTRY_USE_INT64)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_I64);
    registry_set_int64(path, INT64_MAX);
    int64_t output_i64 = registry_get_int64(path);
    TEST_ASSERT_EQUAL_INT(INT64_MAX, output_i64);
#endif /* CONFIG_REGISTRY_USE_INT64 */

    // f32
#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_F32);
    registry_set_float32(path, FLT_MAX);
    float output_f32 = registry_get_float32(path);
    char input_f32_string[REGISTRY_MAX_VAL_LEN] = { 0 };
    char output_f32_string[REGISTRY_MAX_VAL_LEN] = { 0 };
    sprintf(input_f32_string, "%f", FLT_MAX);
    sprintf(output_f32_string, "%f", output_f32);
    TEST_ASSERT_EQUAL_STRING(input_f32_string, output_f32_string);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

    // f64
#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    if (REGISTRY_MAX_VAL_LEN >= (DBL_MAX_10_EXP + 1) + 1 + 1 + 6) { // (DBL_MAX_10_EXP + 1) + sign + dot + 6 decimal places
        path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0, REGISTRY_SCHEMA_TYPES_TEST_F64);
        registry_set_float64(path, DBL_MAX);
        float output_f64 = registry_get_float64(path);
        char input_f64_string[REGISTRY_MAX_VAL_LEN] = { 0 };
        char output_f64_string[REGISTRY_MAX_VAL_LEN] = { 0 };
        sprintf(input_f64_string, "%f", DBL_MAX);
        sprintf(output_f64_string, "%f", output_f64);
        TEST_ASSERT_EQUAL_STRING(input_f64_string, output_f64_string);
    }
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
}

Test *tests_registry(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(tests_registry_register_schema),
        new_TestFixture(tests_registry_all_min_values),
        new_TestFixture(tests_registry_all_max_values),
    };

    EMB_UNIT_TESTCALLER(registry_tests, test_registry_setup, test_registry_teardown, fixtures);

    return (Test *)&registry_tests;
}

int registry_tests_run(void)
{
    TESTS_START();
    TESTS_RUN(tests_registry());
    TESTS_END();
    return 0;
}