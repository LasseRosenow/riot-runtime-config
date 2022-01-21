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

#include "tests.h"

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
};

static void test_registry_setup(void)
{
    /* init registry */
    registry_init();
    registry_schemas_init();

    /* add schema instances */
    registry_add_instance(registry_schema_types_test.id, &test_instance_1);
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
    // string
    int path_string[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_STRING };
    char ouput_string[REGISTRY_MAX_VAL_LEN] = { 0 };

    registry_set_string(path_string, ARRAY_SIZE(path_string), "");
    registry_get_string(path_string, ARRAY_SIZE(path_string), ouput_string,
                        ARRAY_SIZE(ouput_string));
    TEST_ASSERT_EQUAL_STRING("", ouput_string);

    // bool
    int path_bool[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_BOOL };

    registry_set_bool(path_bool, ARRAY_SIZE(path_bool), false);
    bool output_bool = registry_get_bool(path_bool, ARRAY_SIZE(path_bool));

    TEST_ASSERT_EQUAL_INT(false, output_bool);

    // u8
    int path_u8[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_U8 };

    registry_set_uint8(path_u8, ARRAY_SIZE(path_u8), 0);
    uint8_t output_u8 = registry_get_uint8(path_u8, ARRAY_SIZE(path_u8));

    TEST_ASSERT_EQUAL_INT(0, output_u8);

    // u16
    int path_u16[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_U16 };

    registry_set_uint16(path_u16, ARRAY_SIZE(path_u16), 0);
    uint16_t output_u16 = registry_get_uint16(path_u16, ARRAY_SIZE(path_u16));

    TEST_ASSERT_EQUAL_INT(0, output_u16);

    // u32
    int path_u32[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_U32 };

    registry_set_uint32(path_u32, ARRAY_SIZE(path_u32), 0);
    uint32_t output_u32 = registry_get_uint32(path_u32, ARRAY_SIZE(path_u32));

    TEST_ASSERT_EQUAL_INT(0, output_u32);

    // u64
#if defined(CONFIG_REGISTRY_USE_UINT64)
    int path_u64[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_U64 };
    registry_set_uint64(path_u64, ARRAY_SIZE(path_u64), 0);
    uint64_t output_u64 = registry_get_uint64(path_u64, ARRAY_SIZE(path_u64));
    TEST_ASSERT_EQUAL_INT(0, output_u64);
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    // i8
    int path_i8[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_I8 };
    registry_set_int8(path_i8, ARRAY_SIZE(path_i8), INT8_MIN);
    int8_t output_i8 = registry_get_int8(path_i8, ARRAY_SIZE(path_i8));
    TEST_ASSERT_EQUAL_INT(INT8_MIN, output_i8);

    // i16
    int path_i16[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_I16 };
    registry_set_int16(path_i16, ARRAY_SIZE(path_i16), INT16_MIN);
    int16_t output_i16 = registry_get_int16(path_i16, ARRAY_SIZE(path_i16));
    TEST_ASSERT_EQUAL_INT(INT16_MIN, output_i16);

    // i32
    int path_i32[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_I32 };
    registry_set_int32(path_i32, ARRAY_SIZE(path_i32), INT32_MIN);
    int32_t output_i32 = registry_get_int32(path_i32, ARRAY_SIZE(path_i32));
    TEST_ASSERT_EQUAL_INT(INT32_MIN, output_i32);

    // i64
#if defined(CONFIG_REGISTRY_USE_INT64)
    int path_i64[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_I64 };
    registry_set_int64(path_i64, ARRAY_SIZE(path_i64), INT64_MIN);
    int64_t output_i64 = registry_get_int64(path_i64, ARRAY_SIZE(path_i64));
    TEST_ASSERT_EQUAL_INT(INT64_MIN, output_i64);
#endif /* CONFIG_REGISTRY_USE_INT64 */

    // f32
#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    int path_f32[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_F32 };
    registry_set_float32(path_f32, ARRAY_SIZE(path_f32), -FLT_MAX);
    float output_f32 = registry_get_float32(path_f32, ARRAY_SIZE(path_f32));
    char input_f32_string[REGISTRY_MAX_VAL_LEN] = { 0 };
    char output_f32_string[REGISTRY_MAX_VAL_LEN] = { 0 };
    sprintf(input_f32_string, "%f", -FLT_MAX);
    sprintf(output_f32_string, "%f", output_f32);
    TEST_ASSERT_EQUAL_STRING(input_f32_string, output_f32_string);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

    // f64
#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    if (REGISTRY_MAX_VAL_LEN >= (DBL_MAX_10_EXP + 1) + 1 + 1 + 6) { // (DBL_MAX_10_EXP + 1) + sign + dot + 6 decimal places
        int path_f64[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_F64 };
        registry_set_float64(path_f64, ARRAY_SIZE(path_f64), -DBL_MAX);
        float output_f64 = registry_get_float64(path_f64, ARRAY_SIZE(path_f64));
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
    // string
    int path_string[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_STRING };
    char input_string[REGISTRY_MAX_VAL_LEN] = { 0 };
    char ouput_string[REGISTRY_MAX_VAL_LEN] = { 0 };

    for (int i = 0; i < REGISTRY_MAX_VAL_LEN - 1; i++) {
        input_string[i] = 'a';
    }

    registry_set_string(path_string, ARRAY_SIZE(path_string), input_string);
    registry_get_string(path_string, ARRAY_SIZE(path_string), ouput_string,
                        ARRAY_SIZE(ouput_string));
    TEST_ASSERT_EQUAL_STRING(input_string, ouput_string);

    // bool
    int path_bool[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_BOOL };

    registry_set_bool(path_bool, ARRAY_SIZE(path_bool), true);
    bool output_bool = registry_get_bool(path_bool, ARRAY_SIZE(path_bool));

    TEST_ASSERT_EQUAL_INT(true, output_bool);

    // u8
    int path_u8[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_U8 };

    registry_set_uint8(path_u8, ARRAY_SIZE(path_u8), UINT8_MAX);
    uint8_t output_u8 = registry_get_uint8(path_u8, ARRAY_SIZE(path_u8));

    TEST_ASSERT_EQUAL_INT(UINT8_MAX, output_u8);

    // u16
    int path_u16[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_U16 };

    registry_set_uint16(path_u16, ARRAY_SIZE(path_u16), UINT16_MAX);
    uint16_t output_u16 = registry_get_uint16(path_u16, ARRAY_SIZE(path_u16));

    TEST_ASSERT_EQUAL_INT(UINT16_MAX, output_u16);

    // u32
    int path_u32[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_U32 };

    registry_set_uint32(path_u32, ARRAY_SIZE(path_u32), UINT32_MAX);
    uint32_t output_u32 = registry_get_uint32(path_u32, ARRAY_SIZE(path_u32));

    TEST_ASSERT_EQUAL_INT(UINT32_MAX, output_u32);

    // u64
#if defined(CONFIG_REGISTRY_USE_UINT64)
    int path_u64[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_U64 };
    registry_set_uint64(path_u64, ARRAY_SIZE(path_u64), UINT64_MAX);
    uint64_t output_u64 = registry_get_uint64(path_u64, ARRAY_SIZE(path_u64));
    TEST_ASSERT_EQUAL_INT(UINT64_MAX, output_u64);
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    // i8
    int path_i8[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_I8 };
    registry_set_int8(path_i8, ARRAY_SIZE(path_i8), INT8_MAX);
    int8_t output_i8 = registry_get_int8(path_i8, ARRAY_SIZE(path_i8));
    TEST_ASSERT_EQUAL_INT(INT8_MAX, output_i8);

    // i16
    int path_i16[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_I16 };
    registry_set_int16(path_i16, ARRAY_SIZE(path_i16), INT16_MAX);
    int16_t output_i16 = registry_get_int16(path_i16, ARRAY_SIZE(path_i16));
    TEST_ASSERT_EQUAL_INT(INT16_MAX, output_i16);

    // i32
    int path_i32[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_I32 };
    registry_set_int32(path_i32, ARRAY_SIZE(path_i32), INT32_MAX);
    int32_t output_i32 = registry_get_int32(path_i32, ARRAY_SIZE(path_i32));
    TEST_ASSERT_EQUAL_INT(INT32_MAX, output_i32);

    // i64
#if defined(CONFIG_REGISTRY_USE_INT64)
    int path_i64[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_I64 };
    registry_set_int64(path_i64, ARRAY_SIZE(path_i64), INT64_MAX);
    int64_t output_i64 = registry_get_int64(path_i64, ARRAY_SIZE(path_i64));
    TEST_ASSERT_EQUAL_INT(INT64_MAX, output_i64);
#endif /* CONFIG_REGISTRY_USE_INT64 */

    // f32
#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    int path_f32[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_F32 };
    registry_set_float32(path_f32, ARRAY_SIZE(path_f32), FLT_MAX);
    float output_f32 = registry_get_float32(path_f32, ARRAY_SIZE(path_f32));
    char input_f32_string[REGISTRY_MAX_VAL_LEN] = { 0 };
    char output_f32_string[REGISTRY_MAX_VAL_LEN] = { 0 };
    sprintf(input_f32_string, "%f", FLT_MAX);
    sprintf(output_f32_string, "%f", output_f32);
    TEST_ASSERT_EQUAL_STRING(input_f32_string, output_f32_string);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

    // f64
#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    if (REGISTRY_MAX_VAL_LEN >= (DBL_MAX_10_EXP + 1) + 1 + 1 + 6) { // (DBL_MAX_10_EXP + 1) + sign + dot + 6 decimal places
        int path_f64[] = { registry_schema_types_test.id, 0, REGISTRY_SCHEMA_TYPES_TEST_F64 };
        registry_set_float64(path_f64, ARRAY_SIZE(path_f64), DBL_MAX);
        float output_f64 = registry_get_float64(path_f64, ARRAY_SIZE(path_f64));
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

int tests_run(void)
{
    TESTS_START();
    TESTS_RUN(tests_registry());
    TESTS_END();
    return 0;
}
