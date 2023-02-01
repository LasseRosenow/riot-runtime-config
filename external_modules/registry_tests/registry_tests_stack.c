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
#include "registry_storage_facilities.h"
#include "mtd.h"
#include "board.h"
#include "fs/littlefs2_fs.h"
#include "registry_schemas.h"


#include "vfs.h"

#include "registry_tests.h"

/* Stack test registry schema */
#define REGISTRY_APP_SCHEMA_STACK_TEST 15

registry_schema_t registry_app_schema_stack_test;

typedef struct {
    clist_node_t node;
    uint8_t level_1;
    uint8_t level_2;
    uint8_t level_3;
    uint8_t level_4;
    uint8_t level_5;
    uint8_t level_6;
} registry_app_schema_stack_test_t;

typedef enum {
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_1,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_2,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_3,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_4,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_5,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_6,

    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_1,
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_2,
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_3,
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_4,
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_5,
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_6,
} registry_app_schema_stack_test_indices_t;

static void mapping(const registry_id_t param_id, const registry_instance_t *instance,
                    void **val,
                    size_t *val_len)
{
    registry_app_schema_stack_test_t *_instance =
        (registry_app_schema_stack_test_t *)instance->data;

    switch (param_id) {
    case REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_1:
        *val = &_instance->level_1;
        *val_len = sizeof(_instance->level_1);
        break;

    case REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_2:
        *val = &_instance->level_2;
        *val_len = sizeof(_instance->level_2);
        break;

    case REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_3:
        *val = &_instance->level_3;
        *val_len = sizeof(_instance->level_3);
        break;

    case REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_4:
        *val = &_instance->level_4;
        *val_len = sizeof(_instance->level_4);
        break;

    case REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_5:
        *val = &_instance->level_5;
        *val_len = sizeof(_instance->level_5);
        break;

    case REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_6:
        *val = &_instance->level_6;
        *val_len = sizeof(_instance->level_6);
        break;
    }
}

REGISTRY_SCHEMA(
    registry_app_schema_stack_test,
    REGISTRY_APP_SCHEMA_STACK_TEST,
    "rgb", "Representation of an rgb color.",
    mapping,

    /* Level 1 nesting */
    REGISTRY_PARAMETER_UINT8(
        REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_1,
        "parameter_level_1", "A parameter at level 1 nesting.")

    REGISTRY_GROUP(
        REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_1,
        "group_level_1", "A group at level 1 nesting.",

        /* Level 2 nesting */
        REGISTRY_PARAMETER_UINT8(
            REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_2,
            "parameter_level_2", "A parameter at level 2 nesting.")

        REGISTRY_GROUP(
            REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_2,
            "group_level_2", "A group at level 2 nesting.",

            /* Level 3 nesting */
            REGISTRY_PARAMETER_UINT8(
                REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_3,
                "parameter_level_3", "A parameter at level 3 nesting.")

            REGISTRY_GROUP(
                REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_3,
                "group_level_3", "A group at level 3 nesting.",

                /* Level 4 nesting */
                REGISTRY_PARAMETER_UINT8(
                    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_4,
                    "parameter_level_4", "A parameter at level 4 nesting.")

                REGISTRY_GROUP(
                    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_4,
                    "group_level_4", "A group at level 4 nesting.",

                    /* Level 5 nesting */
                    REGISTRY_PARAMETER_UINT8(
                        REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_5,
                        "parameter_level_5", "A parameter at level 5 nesting.")

                    )
                )
            )
        )
    );
//

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
static int stack_test_instance_commit_cb(const registry_path_t path, const void *context)
{
    (void)context;
    (void)path;
    return 0;
}

registry_app_schema_stack_test_t stack_test_instance_data = {
    .level_1 = 10,
    .level_2 = 20,
    .level_3 = 30,
    .level_4 = 40,
    .level_5 = 50,
    .level_6 = 60,
};

static registry_instance_t stack_test_instance = {
    .name = "stack-test",
    .data = &stack_test_instance_data,
    .commit_cb = &stack_test_instance_commit_cb,
};

static uint8_t uint8_value = 7;
static registry_value_t test_value = {
    .type = REGISTRY_TYPE_UINT8,
    .buf = &uint8_value,
    .buf_len = sizeof(uint8_value),
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

static registry_storage_facility_instance_t vfs_instance_1 = {
    .itf = &registry_storage_facility_vfs,
    .data = &_vfs_mount,
};

static registry_storage_facility_instance_t vfs_instance_2 = {
    .itf = &registry_storage_facility_vfs,
    .data = &_vfs_mount,
};

char test_thread_stack[THREAD_STACKSIZE_MAIN] = { 0 };

static void setup(void)
{
    /* init registry */
    registry_init();
    registry_schemas_init();

    /* Add stack test app schema */
    registry_register_schema(REGISTRY_ROOT_GROUP_APP, &registry_app_schema_stack_test);

    /* add schema instances */
    registry_register_schema_instance(REGISTRY_ROOT_GROUP_APP, REGISTRY_APP_SCHEMA_STACK_TEST,
                                      &stack_test_instance);

    /* init storage_facilities */
    if (IS_USED(MODULE_LITTLEFS2)) {
        fs_desc.dev = MTD_0;
    }
    registry_register_storage_facility_src(&vfs_instance_1);
    registry_register_storage_facility_dst(&vfs_instance_2);
}

static registry_id_t parameter_path_level_1[] = {
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_1,
};

static registry_id_t parameter_path_level_2[] = {
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_1,
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_2,
};

static registry_id_t parameter_path_level_3[] = {
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_1,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_2,
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_3,
};

static registry_id_t parameter_path_level_4[] = {
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_1,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_2,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_3,
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_4,
};

static registry_id_t parameter_path_level_5[] = {
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_1,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_2,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_3,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_4,
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_5,
};

static registry_id_t parameter_path_level_6[] = {
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_1,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_2,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_3,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_4,
    REGISTRY_APP_SCHEMA_STACK_TEST_GROUP_LEVEL_5,
    REGISTRY_APP_SCHEMA_STACK_TEST_PARAMETER_LEVEL_6,
};

static registry_path_t path = {
    .namespace_id = (registry_namespace_id_t[]) { REGISTRY_ROOT_GROUP_APP },
    .schema_id = (registry_id_t[]) { REGISTRY_APP_SCHEMA_STACK_TEST },
    .instance_id = (registry_id_t[]) { 0 },
    .path = parameter_path_level_1,
    .path_len = ARRAY_SIZE(parameter_path_level_1),
};

typedef enum {
    GET,
    SET,
    COMMIT,
    EXPORT,
    SAVE,
    LOAD,
} test_case_t;

static void print_test_case_name(test_case_t test_case)
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

static void *thread_test(void *arg)
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
        registry_export(test_export_func, path, 1, NULL);
    } break;

    case SAVE: {
        registry_save(path);
    } break;

    case LOAD: {
        registry_load(path);
    } break;
    }


    int stack_free_after = thread_measure_stack_free(test_thread_stack);

    print_test_case_name(test_case);
    printf(" %d bytes\n", stack_free_before - stack_free_after);

    return NULL;
}

static void create_test_thread(test_case_t test_case)
{
    thread_create(test_thread_stack, sizeof(test_thread_stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST,
                  thread_test, &test_case, "test");
}

static void run_all_tests(void)
{
    create_test_thread(GET);
    create_test_thread(SET);
    create_test_thread(COMMIT);
    create_test_thread(EXPORT);
    create_test_thread(SAVE);
    create_test_thread(LOAD);
}

int registry_tests_stack_run(void)
{
    printf("\nRegistry: Test: Stack consumtions: START\n");

    setup();

    printf("\nLevel 1:\n");
    path.path = parameter_path_level_1;
    path.path_len = ARRAY_SIZE(parameter_path_level_1);
    run_all_tests();

    printf("\nLevel 2:\n");
    path.path = parameter_path_level_2;
    path.path_len = ARRAY_SIZE(parameter_path_level_2);
    run_all_tests();

    printf("\nLevel 3:\n");
    path.path = parameter_path_level_3;
    path.path_len = ARRAY_SIZE(parameter_path_level_3);
    run_all_tests();

    printf("\nLevel 4:\n");
    path.path = parameter_path_level_4;
    path.path_len = ARRAY_SIZE(parameter_path_level_4);
    run_all_tests();

    printf("\nLevel 5:\n");
    path.path = parameter_path_level_5;
    path.path_len = ARRAY_SIZE(parameter_path_level_5);
    run_all_tests();

    (void)parameter_path_level_6;

    printf("\nRegistry: Test: Stack consumtions: END\n");

    return 0;
}

/** @} */
