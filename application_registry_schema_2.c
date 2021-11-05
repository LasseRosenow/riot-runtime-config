#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "kernel_defines.h"

#include "registry.h"

#include "application_registry_schema_2.h"

#define MAX_THRESHOLD (500)

static void get(int param_id, void *instance, void *buf, int buf_len, void *context);
static void set(int param_id, void *instance, void *val, int val_len, void *context);
static int commit_cb(void *context);

static registry_schema_item_t schemas[] = {
    {
        .id = I_8,
        .name = "i8",
        .description = "Example i8 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT8,
        },
    },
    {
        .id = I_16,
        .name = "i16",
        .description = "Example i16 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT16,
        },
    },
    {
        .id = I_32,
        .name = "i32",
        .description = "Example i32 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT32,
        },
    },
    
#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    {
        .id = I_64,
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
        .id = FLOAT,
        .name = "float",
        .description = "Example string description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_FLOAT,
        },
    },
#endif /* CONFIG_REGISTRY_USE_FLOAT */

    {
        .id = BOOL,
        .name = "bool",
        .description = "Example bool description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_BOOL,
        },
    },
    {
        .id = STRING,
        .name = "string",
        .description = "Example string description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_STRING,
        },
    },
};

registry_schema_t my_schema_2 = {
    .id = 1,
    .name = "my_schema_2",
    .description = "Example my_schema_2 description.",
    .schemas = schemas,
    .schemas_len = ARRAY_SIZE(schemas),
    .hndlr_get = get,
    .hndlr_set = set,
    .hndlr_commit_cb = commit_cb,
};

static void get(int param_id, void *instance, void *buf, int buf_len, void *context) {
    (void) buf_len;
    (void) context;

    my_schema_2_t* _instance = (my_schema_2_t*) instance;

    switch (param_id) {
        case I_8:
            memcpy(buf, &_instance->i8, sizeof(_instance->i8));
            break;
        
        case I_16:
            memcpy(buf, &_instance->i16, sizeof(_instance->i16));
            break;

        case I_32:
            memcpy(buf, &_instance->i32, sizeof(_instance->i32));
            break;

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
        case I_64:
            memcpy(buf, &_instance->i64, sizeof(_instance->i64));
            break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
        case FLOAT:
            memcpy(buf, &_instance->f32, sizeof(_instance->f32));
            break;
#endif /* CONFIG_REGISTRY_USE_FLOAT */        

        case BOOL:
            memcpy(buf, &_instance->boolean, sizeof(_instance->boolean));
            break;

        case STRING:
            strcpy(buf, _instance->string);
            break;
    }
}

static void set(int param_id, void *instance, void *val, int val_len, void *context) {
    (void) val_len;
    (void) context;

    my_schema_2_t* _instance = (my_schema_2_t*) instance;

    switch (param_id) {
        case I_8:
            memcpy(&_instance->i8, val, sizeof(_instance->i8));
            break;
        
        case I_16:
            memcpy(&_instance->i16, val, sizeof(_instance->i16));
            break;

        case I_32:
            memcpy(&_instance->i32, val, sizeof(_instance->i32));
            break;

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
        case I_64:
            memcpy(&_instance->i64, val, sizeof(_instance->i64));
            break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
        case FLOAT:
            memcpy(&_instance->f32, val, sizeof(_instance->f32));
            break;
#endif /* CONFIG_REGISTRY_USE_FLOAT */        

        case BOOL:
            memcpy(&_instance->boolean, val, sizeof(_instance->boolean));
            break;

        case STRING:
            strcpy(_instance->string, val);
            break;
    }
}

static int commit_cb(void *context) {
    (void) context;

    return 0;
}
