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
#include "fmt.h"
#include "assert.h"
#include "ps.h"
#include "thread.h"
#include "registry.h"
#include "registry_schemas.h"

#include "registry_tests.h"

static int test_instance_0_commit_cb(const registry_path_t path, const void *context)
{
    (void)context;
    (void)path;
    return 0;
}

static registry_schema_types_test_t test_instance_1_data = {
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

static registry_value_t test_value;

char test_thread_stack[4000] = { 0 };

static void setup(void)
{
    /* init registry */
    registry_init();
    registry_schemas_init();

    /* add schema instances */
    registry_register_schema_instance(REGISTRY_ROOT_GROUP_SYS, REGISTRY_SCHEMA_TYPES_TEST,
                                      &test_instance_1);
}

static void test_get(registry_path_t path)
{
    printf("\nregistry_get:\n");

    printf("\nBefore:\n");
    ps();

    printf("\nAfter:\n");
    registry_get_value(path, &test_value);
}

int registry_stack_tests_run(void)
{
    printf("\nRegistry: Test: Stack consumtions: START\n");

    setup();

    registry_path_t path = REGISTRY_PATH_SYS(REGISTRY_SCHEMA_TYPES_TEST, 0,
                                             REGISTRY_SCHEMA_TYPES_TEST_BOOL);

    test_set(path);
    // test_commit();
    // test_export();
    // test_save();
    // test_load();

    printf("\nRegistry: Test: Stack consumtions: END\n");

    return 0;
}

/** @} */
