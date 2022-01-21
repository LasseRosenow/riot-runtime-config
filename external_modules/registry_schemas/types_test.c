#ifdef CONFIG_REGISTRY_ENABLE_SCHEMA_TYPES_TEST

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "kernel_defines.h"
#include "registry.h"
#include "registry_schemas.h"

static void get(int param_id, registry_instance_t *instance, void *buf, int buf_len, void *context);
static void set(int param_id, registry_instance_t *instance, const void *val, int val_len,
                void *context);

static registry_schema_item_t schema_items[] = {
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_BOOL,
        .name = "bool",
        .description = "Example bool description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_BOOL,
        },
    },
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_STRING,
        .name = "string",
        .description = "Example string description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_STRING,
        },
    },
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_U8,
        .name = "u8",
        .description = "Example u8 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_UINT8,
        },
    },
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_U16,
        .name = "u16",
        .description = "Example u16 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_UINT16,
        },
    },
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_U32,
        .name = "u32",
        .description = "Example u32 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_UINT32,
        },
    },

#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_U64,
        .name = "u64",
        .description = "Example u64 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_UINT64,
        },
    },
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_I8,
        .name = "i8",
        .description = "Example i8 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT8,
        },
    },
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_I16,
        .name = "i16",
        .description = "Example i16 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT16,
        },
    },
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_I32,
        .name = "i32",
        .description = "Example i32 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT32,
        },
    },

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_I64,
        .name = "i64",
        .description = "Example i64 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT64,
        },
    },
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_F32,
        .name = "f32",
        .description = "Example f32 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_FLOAT32,
        },
    },
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
    {
        .id = REGISTRY_SCHEMA_TYPES_TEST_F64,
        .name = "f64",
        .description = "Example f64 description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_FLOAT64,
        },
    },
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

};

registry_schema_t registry_schema_types_test = {
    .id = REGISTRY_SCHEMA_TYPES_TEST,
    .name = "test",
    .description = "Test schema containing all possible types for testing purposes.",
    .items = schema_items,
    .items_len = ARRAY_SIZE(schema_items),
    .get = get,
    .set = set,
};

static void get(int param_id, registry_instance_t *instance, void *buf, int buf_len, void *context)
{
    (void)buf_len;
    (void)context;

    registry_schema_types_test_t *_instance = (registry_schema_types_test_t *)instance->data;

    switch (param_id) {
    case REGISTRY_SCHEMA_TYPES_TEST_BOOL:
        memcpy(buf, &_instance->boolean, sizeof(_instance->boolean));
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_STRING:
        strcpy(buf, _instance->string);
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_U8:
        memcpy(buf, &_instance->u8, sizeof(_instance->u8));
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_U16:
        memcpy(buf, &_instance->u16, sizeof(_instance->u16));
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_U32:
        memcpy(buf, &_instance->u32, sizeof(_instance->u32));
        break;

#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
    case REGISTRY_SCHEMA_TYPES_TEST_U64:
        memcpy(buf, &_instance->u64, sizeof(_instance->u64));
        break;
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    case REGISTRY_SCHEMA_TYPES_TEST_I8:
        memcpy(buf, &_instance->i8, sizeof(_instance->i8));
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_I16:
        memcpy(buf, &_instance->i16, sizeof(_instance->i16));
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_I32:
        memcpy(buf, &_instance->i32, sizeof(_instance->i32));
        break;

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    case REGISTRY_SCHEMA_TYPES_TEST_I64:
        memcpy(buf, &_instance->i64, sizeof(_instance->i64));
        break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
    case REGISTRY_SCHEMA_TYPES_TEST_F32:
        memcpy(buf, &_instance->f32, sizeof(_instance->f32));
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
    case REGISTRY_SCHEMA_TYPES_TEST_F64:
        memcpy(buf, &_instance->f64, sizeof(_instance->f64));
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

    }
}

static void set(int param_id, registry_instance_t *instance, const void *val, int val_len,
                void *context)
{
    (void)val_len;
    (void)context;

    registry_schema_types_test_t *_instance = (registry_schema_types_test_t *)instance->data;

    switch (param_id) {
    case REGISTRY_SCHEMA_TYPES_TEST_BOOL:
        memcpy(&_instance->boolean, val, sizeof(_instance->boolean));
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_STRING:
        strcpy(_instance->string, val);
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_U8:
        memcpy(&_instance->u8, val, sizeof(_instance->u8));
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_U16:
        memcpy(&_instance->u16, val, sizeof(_instance->u16));
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_U32:
        memcpy(&_instance->u32, val, sizeof(_instance->u32));
        break;

#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
    case REGISTRY_SCHEMA_TYPES_TEST_U64:
        memcpy(&_instance->u64, val, sizeof(_instance->u64));
        break;
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    case REGISTRY_SCHEMA_TYPES_TEST_I8:
        memcpy(&_instance->i8, val, sizeof(_instance->i8));
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_I16:
        memcpy(&_instance->i16, val, sizeof(_instance->i16));
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_I32:
        memcpy(&_instance->i32, val, sizeof(_instance->i32));
        break;

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    case REGISTRY_SCHEMA_TYPES_TEST_I64:
        memcpy(&_instance->i64, val, sizeof(_instance->i64));
        break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
    case REGISTRY_SCHEMA_TYPES_TEST_F32:
        memcpy(&_instance->f32, val, sizeof(_instance->f32));
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
    case REGISTRY_SCHEMA_TYPES_TEST_F64:
        memcpy(&_instance->f64, val, sizeof(_instance->f64));
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

    }
}

#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_TYPES_TEST */
