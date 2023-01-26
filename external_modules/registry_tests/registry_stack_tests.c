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
#include "registry_storage_facilities.h"
#include "mtd.h"
#include "board.h"
#include "fs/littlefs2_fs.h"

#include "vfs.h"

#include "registry_tests.h"

/* Export */
static int test_export_func(const registry_path_t path,
                            const registry_schema_t *schema,
                            const registry_instance_t *instance,
                            const registry_schema_item_t *meta,
                            const registry_value_t *value,
                            const void *context)
{
    (void)path;
    (void)schema;
    (void)instance;
    (void)meta;
    (void)value;
    (void)context;
    return 0;
}

/* Instace */
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

static bool boolean_value = 7;
static registry_value_t test_value = {
    .type = REGISTRY_TYPE_BOOL,
    .buf = &boolean_value,
    .buf_len = sizeof(boolean_value),
};

/* Store */
static littlefs2_desc_t fs_desc = {
    .lock = MUTEX_INIT,
};

static vfs_mount_t _vfs_mount = {
    .fs = &littlefs2_file_system,
    .mount_point = "/sda",
    .private_data = &fs_desc,
};

static registry_store_instance_t vfs_instance_1 = {
    .itf = &registry_store_vfs,
    .data = &_vfs_mount,
};

static registry_store_instance_t vfs_instance_2 = {
    .itf = &registry_store_vfs,
    .data = &_vfs_mount,
};

char test_thread_stack[THREAD_STACKSIZE_MAIN] = { 0 };

static void setup(void)
{
    /* init registry */
    registry_init();
    registry_schemas_init();

    /* add schema instances */
    registry_register_schema_instance(REGISTRY_ROOT_GROUP_SYS, REGISTRY_SCHEMA_TYPES_TEST,
                                      &test_instance_1);

    /* init stores */
    if (IS_USED(MODULE_LITTLEFS2)) {
        fs_desc.dev = MTD_0;
    }
    registry_register_store_src(&vfs_instance_1);
    registry_register_store_dst(&vfs_instance_2);
}

static registry_path_t path = {
    .namespace_id = (registry_namespace_id_t[]) { REGISTRY_ROOT_GROUP_SYS },
    .schema_id = (registry_path_item_t[]) { REGISTRY_SCHEMA_TYPES_TEST },
    .instance_id = (registry_path_item_t[]) { 0 },
    .path = (registry_path_item_t[]) { REGISTRY_SCHEMA_TYPES_TEST_BOOL },
    .path_len = 1,
};

typedef enum {
    GET,
    SET,
    COMMIT,
    EXPORT,
    SAVE,
    LOAD,
} test_case_t;

void print_test_case_name(test_case_t test_case)
{
    switch (test_case) {
    case GET: {
        printf("registry_get_value:");
    } break;

    case SET: {
        printf("registry_set_value:");
    } break;

    case COMMIT: {
        printf("registry_commit:   ");
    } break;

    case EXPORT: {
        printf("registry_export:   ");
    } break;

    case SAVE: {
        printf("registry_save:     ");
    } break;

    case LOAD: {
        printf("registry_load:     ");
    } break;
    }
}

void *thread_test(void *arg)
{
    test_case_t test_case = *(test_case_t *)arg;

    int stack_free_before = thread_measure_stack_free(test_thread_stack);

    switch (test_case) {
    case GET: {
        registry_get_value(path, &test_value);
    } break;

    case SET: {
        registry_set_value(path, test_value);
    } break;

    case COMMIT: {
        registry_commit(path);
    } break;

    case EXPORT: {
        registry_export(test_export_func, path, 0, NULL);
    } break;

    case SAVE: {
        registry_save(_REGISTRY_PATH_0());
    } break;

    case LOAD: {
        registry_load(_REGISTRY_PATH_0());
    } break;
    }


    int stack_free_after = thread_measure_stack_free(test_thread_stack);

    print_test_case_name(test_case);
    printf(" %d bytes\n", stack_free_before - stack_free_after);

    return NULL;
}

void create_test_thread(test_case_t test_case)
{
    thread_create(test_thread_stack, sizeof(test_thread_stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST,
                  thread_test, &test_case, "test");
}

int registry_stack_tests_run(void)
{
    printf("\nRegistry: Test: Stack consumtions: START\n");

    setup();

    create_test_thread(GET);
    create_test_thread(SET);
    create_test_thread(COMMIT);
    create_test_thread(EXPORT);
    create_test_thread(SAVE);
    create_test_thread(LOAD);

    printf("\nRegistry: Test: Stack consumtions: END\n");

    return 0;
}

/** @} */
