/*
 * Copyright (C) 2023 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry_cli RIOT Registry Tests
 * @ingroup     sys
 * @brief       RIOT Registry Tests module providing unit tests for the RIOT Registry sys module
 * @{
 *
 * @file
 *
 * @author      Lasse Rosenow <lasse.rosenow@haw-hamburg.de>
 */

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
#include "registry_storage_facilities.h"
#include "vfs.h"
#include "board.h"
#include "mtd.h"

#include "registry_tests.h"

#define FLOAT_MAX_CHAR_COUNT ((FLT_MAX_10_EXP + 1) + 1 + 1 + 6)     // (FLT_MAX_10_EXP + 1) + sign + dot + 6 decimal places
#define DOUBLE_MAX_CHAR_COUNT ((DBL_MAX_10_EXP + 1) + 1 + 1 + 6)    // (DBL_MAX_10_EXP + 1) + sign + dot + 6 decimal places

// Littlefs2
#if IS_USED(MODULE_LITTLEFS2)
#include "fs/littlefs2_fs.h"
#define FS_DRIVER littlefs2_file_system
static littlefs2_desc_t fs_desc = {
    .lock = MUTEX_INIT,
};
#elif IS_USED(MODULE_FATFS_VFS)
#include "fs/fatfs.h"
#define FS_DRIVER fatfs_file_system
static fatfs_desc_t fs_desc;
#endif



static vfs_mount_t _vfs_mount = {
    .fs = &FS_DRIVER,
    .mount_point = "/sda",
    .private_data = &fs_desc,
};

static registry_storage_facility_instance_t vfs_instance_1 = {
    .itf = &registry_storage_facility_vfs,
    .data = &_vfs_mount,
};

static registry_storage_facility_instance_t vfs_instance_2 = {
    .itf = &registry_storage_facility_vfs,
    .data = &_vfs_mount,
};

static bool commit_success = false;

static int test_instance_0_commit_cb(const registry_path_t path, const void *context)
{
    (void)path;
    (void)context;

    if (*path.namespace_id == REGISTRY_ROOT_GROUP_SYS &&
        *path.schema_id == REGISTRY_SCHEMA_FULL_EXAMPLE &&
        *path.instance_id == 0) {
        commit_success = true;
    }

    return 0;
}

static registry_schema_full_example_t test_instance_1_data = {
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

static registry_instance_t test_instance_1 = {
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
    registry_register_schema_instance(REGISTRY_ROOT_GROUP_SYS, REGISTRY_SCHEMA_FULL_EXAMPLE,
                                      &test_instance_1);

    /* init storage_facilities */
    if (IS_USED(MODULE_LITTLEFS2)) {
        fs_desc.dev = MTD_0;
    }
    registry_register_storage_facility_src(&vfs_instance_1);
    registry_register_storage_facility_dst(&vfs_instance_2);
}

static void test_registry_teardown(void)
{}

static void tests_registry_register_schema(void)
{
    /* test if schema_full_example got registered */
    clist_node_t *test_node = registry_schema_full_example.instances.next->next;
    registry_schema_full_example_t *test_instance = container_of(test_node,
                                                                 registry_schema_full_example_t,
                                                                 node);

    TEST_ASSERT_EQUAL_INT((int)&test_instance_1, (int)test_instance);
}

static void tests_registry_all_min_values(void)
{
    registry_path_t path;


    /* string */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_STRING);

    registry_set_string(path, "");

    const char *ouput_string;
    size_t output_string_len;

    registry_get_string(path, &ouput_string, &output_string_len);

    TEST_ASSERT_EQUAL_STRING("", ouput_string);


    /* bool */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_BOOL);

    registry_set_bool(path, false);

    const bool *bool_out;

    registry_get_bool(path, &bool_out);

    TEST_ASSERT_EQUAL_INT(false, *bool_out);


    /* u8 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_U8);

    registry_set_uint8(path, 0);

    const uint8_t *output_u8;

    registry_get_uint8(path, &output_u8);

    TEST_ASSERT_EQUAL_INT(0, *output_u8);


    /* u16 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_U16);

    registry_set_uint16(path, 0);

    const uint16_t *output_u16;

    registry_get_uint16(path, &output_u16);

    TEST_ASSERT_EQUAL_INT(0, *output_u16);


    /* u32 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_U32);

    registry_set_uint32(path, 0);

    const uint32_t *output_u32;

    registry_get_uint32(path, &output_u32);

    TEST_ASSERT_EQUAL_INT(0, *output_u32);


    /* u64 */
#if defined(CONFIG_REGISTRY_USE_UINT64)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_U64);
    registry_set_uint64(path, 0);
    const uint64_t *output_u64;
    registry_get_uint64(path, &output_u64);
    TEST_ASSERT_EQUAL_INT(0, *output_u64);
#endif /* CONFIG_REGISTRY_USE_UINT64 */


    /* i8 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_I8);
    registry_set_int8(path, INT8_MIN);
    const int8_t *output_i8;
    registry_get_int8(path, &output_i8);
    TEST_ASSERT_EQUAL_INT(INT8_MIN, *output_i8);


    /* i16 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_I16);
    registry_set_int16(path, INT16_MIN);
    const int16_t *output_i16;
    registry_get_int16(path, &output_i16);
    TEST_ASSERT_EQUAL_INT(INT16_MIN, *output_i16);


    /* i32 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_I32);
    registry_set_int32(path, INT32_MIN);
    const int32_t *output_i32;
    registry_get_int32(path, &output_i32);
    TEST_ASSERT_EQUAL_INT(INT32_MIN, *output_i32);


    /* i64 */
#if defined(CONFIG_REGISTRY_USE_INT64)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_I64);
    registry_set_int64(path, INT64_MIN);
    const int64_t *output_i64;
    registry_get_int64(path, &output_i64);
    TEST_ASSERT_EQUAL_INT(INT64_MIN, *output_i64);
#endif /* CONFIG_REGISTRY_USE_INT64 */


    /* f32 */
#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_F32);
    registry_set_float32(path, -FLT_MAX);
    const float *output_f32;
    registry_get_float32(path, &output_f32);
    char input_f32_string[FLOAT_MAX_CHAR_COUNT + 1] = { 0 };
    char output_f32_string[FLOAT_MAX_CHAR_COUNT + 1] = { 0 };
    sprintf(input_f32_string, "%f", -FLT_MAX);
    sprintf(output_f32_string, "%f", *output_f32);
    TEST_ASSERT_EQUAL_STRING(input_f32_string, output_f32_string);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */


    /* f64 */
#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_F64);
    registry_set_float64(path, -DBL_MAX);
    const double *output_f64;
    registry_get_float64(path, &output_f64);
    char input_f64_string[DOUBLE_MAX_CHAR_COUNT + 1] = { 0 };
    char output_f64_string[DOUBLE_MAX_CHAR_COUNT + 1] = { 0 };
    sprintf(input_f64_string, "%f", -DBL_MAX);
    sprintf(output_f64_string, "%f", *output_f64);
    TEST_ASSERT_EQUAL_STRING(input_f64_string, output_f64_string);
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
}

static void tests_registry_all_max_values(void)
{
    registry_path_t path;

    /* string */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_STRING);
    char input_string[50] = { 0 };

    for (size_t i = 0; i < 50 - 1; i++) {
        input_string[i] = 'a';
    }

    registry_set_string(path, input_string);

    const char *ouput_string;

    registry_get_string(path, &ouput_string, NULL);

    TEST_ASSERT_EQUAL_STRING(input_string, ouput_string);


    /* bool */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_BOOL);

    registry_set_bool(path, true);

    const bool *output_bool;

    registry_get_bool(path, &output_bool);

    TEST_ASSERT_EQUAL_INT(true, *output_bool);


    /* u8 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_U8);

    registry_set_uint8(path, UINT8_MAX);

    const uint8_t *output_u8;

    registry_get_uint8(path, &output_u8);

    TEST_ASSERT_EQUAL_INT(UINT8_MAX, *output_u8);


    /* u16 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_U16);

    registry_set_uint16(path, UINT16_MAX);

    const uint16_t *output_u16;

    registry_get_uint16(path, &output_u16);

    TEST_ASSERT_EQUAL_INT(UINT16_MAX, *output_u16);


    /* u32 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_U32);

    registry_set_uint32(path, UINT32_MAX);

    const uint32_t *output_u32;

    registry_get_uint32(path, &output_u32);

    TEST_ASSERT_EQUAL_INT(UINT32_MAX, *output_u32);


    /* u64 */
#if defined(CONFIG_REGISTRY_USE_UINT64)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_U64);
    registry_set_uint64(path, UINT64_MAX);
    const uint64_t *output_u64;
    registry_get_uint64(path, &output_u64);
    TEST_ASSERT_EQUAL_INT(UINT64_MAX, *output_u64);
#endif /* CONFIG_REGISTRY_USE_UINT64 */


    /* i8 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_I8);
    registry_set_int8(path, INT8_MAX);
    const int8_t *output_i8;
    registry_get_int8(path, &output_i8);
    TEST_ASSERT_EQUAL_INT(INT8_MAX, *output_i8);


    /* i16 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_I16);
    registry_set_int16(path, INT16_MAX);
    const int16_t *output_i16;
    registry_get_int16(path, &output_i16);
    TEST_ASSERT_EQUAL_INT(INT16_MAX, *output_i16);


    /* i32 */
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_I32);
    registry_set_int32(path, INT32_MAX);
    const int32_t *output_i32;
    registry_get_int32(path, &output_i32);
    TEST_ASSERT_EQUAL_INT(INT32_MAX, *output_i32);


    /* i64 */
#if defined(CONFIG_REGISTRY_USE_INT64)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_I64);
    registry_set_int64(path, INT64_MAX);
    const int64_t *output_i64;
    registry_get_int64(path, &output_i64);
    TEST_ASSERT_EQUAL_INT(INT64_MAX, *output_i64);
#endif /* CONFIG_REGISTRY_USE_INT64 */


    /* f32 */
#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_F32);
    registry_set_float32(path, FLT_MAX);
    const float *output_f32;
    registry_get_float32(path, &output_f32);
    char input_f32_string[FLOAT_MAX_CHAR_COUNT + 1] = { 0 };
    char output_f32_string[FLOAT_MAX_CHAR_COUNT + 1] = { 0 };
    sprintf(input_f32_string, "%f", FLT_MAX);
    sprintf(output_f32_string, "%f", *output_f32);
    TEST_ASSERT_EQUAL_STRING(input_f32_string, output_f32_string);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */


    /* f64 */
#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0, REGISTRY_SCHEMA_FULL_EXAMPLE_F64);
    registry_set_float64(path, DBL_MAX);
    const double *output_f64;
    registry_get_float64(path, &output_f64);
    char input_f64_string[DOUBLE_MAX_CHAR_COUNT + 1] = { 0 };
    char output_f64_string[DOUBLE_MAX_CHAR_COUNT + 1] = { 0 };
    sprintf(input_f64_string, "%f", DBL_MAX);
    sprintf(output_f64_string, "%f", *output_f64);
    TEST_ASSERT_EQUAL_STRING(input_f64_string, output_f64_string);
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
}


static void tests_registry_commit(void)
{
    registry_path_t path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0);

    registry_commit(path);

    TEST_ASSERT_EQUAL_INT(true, commit_success);
}

bool export_success = false;

static int _export_func(const registry_path_t path, const registry_schema_t *schema,
                        const registry_instance_t *instance, const registry_schema_item_t *meta,
                        const registry_value_t *value, const void *context)
{
    (void)path;
    (void)schema;
    (void)instance;
    (void)meta;
    (void)value;
    (void)context;

    if (*path.namespace_id == REGISTRY_ROOT_GROUP_SYS &&
        *path.schema_id == REGISTRY_SCHEMA_FULL_EXAMPLE &&
        *path.instance_id == 0 &&
        path.path[0] == REGISTRY_SCHEMA_FULL_EXAMPLE_STRING) {
        export_success = true;
    }

    return 0;
}

static void tests_registry_export(void)
{
    registry_path_t path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0,
                                             REGISTRY_SCHEMA_FULL_EXAMPLE_STRING);

    registry_export(_export_func, path, 0, NULL);

    TEST_ASSERT_EQUAL_INT(true, export_success);
}

static void tests_registry_save_load(void)
{
    registry_path_t path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_FULL_EXAMPLE, 0,
                                             REGISTRY_SCHEMA_FULL_EXAMPLE_U8);
    uint8_t old_value = 5;

    registry_set_uint8(path, old_value);

    registry_save(_REGISTRY_PATH_0());

    registry_set_uint8(path, 10);

    registry_load(_REGISTRY_PATH_0());

    const uint8_t *new_value;

    registry_get_uint8(path, &new_value);

    TEST_ASSERT_EQUAL_INT(old_value, *new_value);
}

static Test *tests_registry(void)
{
    (void)tests_registry_register_schema;
    (void)tests_registry_all_min_values;
    (void)tests_registry_all_max_values;

    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(tests_registry_register_schema),
        new_TestFixture(tests_registry_all_min_values),
        new_TestFixture(tests_registry_all_max_values),
        new_TestFixture(tests_registry_commit),
        new_TestFixture(tests_registry_export),
        new_TestFixture(tests_registry_save_load),
    };

    EMB_UNIT_TESTCALLER(registry_tests, test_registry_setup, test_registry_teardown, fixtures);

    return (Test *)&registry_tests;
}

int registry_tests_api_run(void)
{
    TESTS_START();
    TESTS_RUN(tests_registry());
    TESTS_END();
    return 0;
}

/** @} */
