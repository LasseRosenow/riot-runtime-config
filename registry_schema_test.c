#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "kernel_defines.h"

#include "registry.h"

#include "registry_schema_test.h"

static void get(int param_id, void *instance, void *buf, int buf_len, void *context);
static void set(int param_id, void *instance, void *val, int val_len, void *context);
static int commit_cb(void *context);

static registry_schema_item_t schemas[] = {
    {
        .id = REGISTRY_SCHEMA_TEST_I_8,
        .name = "i8",
        .description = "Example i8 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT8,
        },
    },
    {
        .id = REGISTRY_SCHEMA_TEST_I_16,
        .name = "i16",
        .description = "Example i16 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT16,
        },
    },
    {
        .id = REGISTRY_SCHEMA_TEST_I_32,
        .name = "i32",
        .description = "Example i32 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT32,
        },
    },
    
#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    {
        .id = REGISTRY_SCHEMA_TEST_I_64,
        .name = "i64",
        .description = "Example i64 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT64,
        },
    },
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
    {
        .id = REGISTRY_SCHEMA_TEST_FLOAT,
        .name = "float",
        .description = "Example string description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_FLOAT,
        },
    },
#endif /* CONFIG_REGISTRY_USE_FLOAT */

    {
        .id = REGISTRY_SCHEMA_TEST_BOOL,
        .name = "bool",
        .description = "Example bool description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_BOOL,
        },
    },
    {
        .id = REGISTRY_SCHEMA_TEST_STRING,
        .name = "string",
        .description = "Example string description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_STRING,
        },
    },
};

registry_schema_t registry_schema_test = {
    .id = 1,
    .name = "test",
    .description = "Test schema containing all possible types for testing purposes.",
    .schemas = schemas,
    .schemas_len = ARRAY_SIZE(schemas),
    .get = get,
    .set = set,
    .commit_cb = commit_cb,
};

static void get(int param_id, void *instance, void *buf, int buf_len, void *context) {
    (void) buf_len;
    (void) context;

    registry_schema_test_t* _instance = (registry_schema_test_t*) instance;

    switch (param_id) {
        case REGISTRY_SCHEMA_TEST_I_8:
            memcpy(buf, &_instance->i8, sizeof(_instance->i8));
            break;
        
        case REGISTRY_SCHEMA_TEST_I_16:
            memcpy(buf, &_instance->i16, sizeof(_instance->i16));
            break;

        case REGISTRY_SCHEMA_TEST_I_32:
            memcpy(buf, &_instance->i32, sizeof(_instance->i32));
            break;

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
        case REGISTRY_SCHEMA_TEST_I_64:
            memcpy(buf, &_instance->i64, sizeof(_instance->i64));
            break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
        case REGISTRY_SCHEMA_TEST_FLOAT:
            memcpy(buf, &_instance->f32, sizeof(_instance->f32));
            break;
#endif /* CONFIG_REGISTRY_USE_FLOAT */        

        case REGISTRY_SCHEMA_TEST_BOOL:
            memcpy(buf, &_instance->boolean, sizeof(_instance->boolean));
            break;

        case REGISTRY_SCHEMA_TEST_STRING:
            strcpy(buf, _instance->string);
            break;
    }
}

static void set(int param_id, void *instance, void *val, int val_len, void *context) {
    (void) val_len;
    (void) context;

    registry_schema_test_t* _instance = (registry_schema_test_t*) instance;

    switch (param_id) {
        case REGISTRY_SCHEMA_TEST_I_8:
            memcpy(&_instance->i8, val, sizeof(_instance->i8));
            break;
        
        case REGISTRY_SCHEMA_TEST_I_16:
            memcpy(&_instance->i16, val, sizeof(_instance->i16));
            break;

        case REGISTRY_SCHEMA_TEST_I_32:
            memcpy(&_instance->i32, val, sizeof(_instance->i32));
            break;

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
        case REGISTRY_SCHEMA_TEST_I_64:
            memcpy(&_instance->i64, val, sizeof(_instance->i64));
            break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
        case REGISTRY_SCHEMA_TEST_FLOAT:
            memcpy(&_instance->f32, val, sizeof(_instance->f32));
            break;
#endif /* CONFIG_REGISTRY_USE_FLOAT */        

        case REGISTRY_SCHEMA_TEST_BOOL:
            memcpy(&_instance->boolean, val, sizeof(_instance->boolean));
            break;

        case REGISTRY_SCHEMA_TEST_STRING:
            strcpy(_instance->string, val);
            break;
    }
}

static int commit_cb(void *context) {
    (void) context;

    return 0;
}
