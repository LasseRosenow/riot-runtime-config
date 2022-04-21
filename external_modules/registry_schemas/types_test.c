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

REGISTRY_SCHEMA(
    registry_schema_types_test,
    REGISTRY_SCHEMA_TYPES_TEST,
    "test", "Test schema containing all possible types for testing purposes.",
    get, set,

    REGISTRY_PARAMETER_STRING(
        REGISTRY_SCHEMA_TYPES_TEST_STRING,
        "string", "Example string description.")

    REGISTRY_PARAMETER_BOOL(
        REGISTRY_SCHEMA_TYPES_TEST_BOOL,
        "bool", "Example bool description.")

    REGISTRY_PARAMETER_UINT8(
        REGISTRY_SCHEMA_TYPES_TEST_U8,
        "u8", "Example u8 description.")

    REGISTRY_PARAMETER_UINT16(
        REGISTRY_SCHEMA_TYPES_TEST_U16,
        "u16", "Example u16 description.")

    REGISTRY_PARAMETER_UINT32(
        REGISTRY_SCHEMA_TYPES_TEST_U32,
        "u32", "Example u32 description.")

    REGISTRY_PARAMETER_UINT64(
        REGISTRY_SCHEMA_TYPES_TEST_U64,
        "u64", "Example u64 description.")

    REGISTRY_PARAMETER_INT8(
        REGISTRY_SCHEMA_TYPES_TEST_I8,
        "i8", "Example i8 description.")

    REGISTRY_PARAMETER_INT16(
        REGISTRY_SCHEMA_TYPES_TEST_I16,
        "i16", "Example i16 description.")

    REGISTRY_PARAMETER_INT32(
        REGISTRY_SCHEMA_TYPES_TEST_I32,
        "i32", "Example i32 description.")

    REGISTRY_PARAMETER_INT64(
        REGISTRY_SCHEMA_TYPES_TEST_I64,
        "i64", "Example i64 description.")

    REGISTRY_PARAMETER_FLOAT32(
        REGISTRY_SCHEMA_TYPES_TEST_F32,
        "f32", "Example f32 description.")

    REGISTRY_PARAMETER_FLOAT64(
        REGISTRY_SCHEMA_TYPES_TEST_F64,
        "f64", "Example f64 description.")

    );

static void get(int param_id, registry_instance_t *instance, void *buf, int buf_len, void *context)
{
    (void)buf_len;
    (void)context;

    registry_schema_types_test_t *_instance = (registry_schema_types_test_t *)instance->data;

    switch (param_id) {
    case REGISTRY_SCHEMA_TYPES_TEST_STRING:
        strcpy(buf, _instance->string);
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_BOOL:
        memcpy(buf, &_instance->boolean, sizeof(_instance->boolean));
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

#if defined(CONFIG_REGISTRY_USE_UINT64)
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

#if defined(CONFIG_REGISTRY_USE_INT64)
    case REGISTRY_SCHEMA_TYPES_TEST_I64:
        memcpy(buf, &_instance->i64, sizeof(_instance->i64));
        break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    case REGISTRY_SCHEMA_TYPES_TEST_F32:
        memcpy(buf, &_instance->f32, sizeof(_instance->f32));
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64)
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
    case REGISTRY_SCHEMA_TYPES_TEST_STRING:
        strcpy(_instance->string, val);
        break;

    case REGISTRY_SCHEMA_TYPES_TEST_BOOL:
        memcpy(&_instance->boolean, val, sizeof(_instance->boolean));
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

#if defined(CONFIG_REGISTRY_USE_UINT64)
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

#if defined(CONFIG_REGISTRY_USE_INT64)
    case REGISTRY_SCHEMA_TYPES_TEST_I64:
        memcpy(&_instance->i64, val, sizeof(_instance->i64));
        break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    case REGISTRY_SCHEMA_TYPES_TEST_F32:
        memcpy(&_instance->f32, val, sizeof(_instance->f32));
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    case REGISTRY_SCHEMA_TYPES_TEST_F64:
        memcpy(&_instance->f64, val, sizeof(_instance->f64));
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

    }
}
