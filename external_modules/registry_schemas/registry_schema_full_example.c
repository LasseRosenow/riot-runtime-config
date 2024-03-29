/*
 * Copyright (C) 2023 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry_cli RIOT Registry Schemas: Types Test
 * @ingroup     sys
 * @brief       RIOT Registry Types Test Schema, used for testing all the types of the RIOT Registry
 * @{
 *
 * @file
 *
 * @author      Lasse Rosenow <lasse.rosenow@haw-hamburg.de>
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "kernel_defines.h"
#include "registry.h"
#include "registry_schemas.h"

#if IS_ACTIVE(CONFIG_REGISTRY_ENABLE_SCHEMA_FULL_EXAMPLE) || IS_ACTIVE(DOXYGEN)

static void mapping(const registry_id_t param_id, const registry_instance_t *instance,
                    void **val,
                    size_t *val_len);

REGISTRY_SCHEMA(
    registry_schema_full_example,
    REGISTRY_SCHEMA_FULL_EXAMPLE,
    "test", "Test schema containing all possible types for testing purposes.",
    mapping,

    REGISTRY_PARAMETER_STRING(
        REGISTRY_SCHEMA_FULL_EXAMPLE_STRING,
        "string", "Example string description.")

    REGISTRY_PARAMETER_BOOL(
        REGISTRY_SCHEMA_FULL_EXAMPLE_BOOL,
        "bool", "Example bool description.")

    REGISTRY_PARAMETER_UINT8(
        REGISTRY_SCHEMA_FULL_EXAMPLE_U8,
        "u8", "Example u8 description.")

    REGISTRY_PARAMETER_UINT16(
        REGISTRY_SCHEMA_FULL_EXAMPLE_U16,
        "u16", "Example u16 description.")

    REGISTRY_PARAMETER_UINT32(
        REGISTRY_SCHEMA_FULL_EXAMPLE_U32,
        "u32", "Example u32 description.")

    REGISTRY_PARAMETER_UINT64(
        REGISTRY_SCHEMA_FULL_EXAMPLE_U64,
        "u64", "Example u64 description.")

    REGISTRY_PARAMETER_INT8(
        REGISTRY_SCHEMA_FULL_EXAMPLE_I8,
        "i8", "Example i8 description.")

    REGISTRY_PARAMETER_INT16(
        REGISTRY_SCHEMA_FULL_EXAMPLE_I16,
        "i16", "Example i16 description.")

    REGISTRY_PARAMETER_INT32(
        REGISTRY_SCHEMA_FULL_EXAMPLE_I32,
        "i32", "Example i32 description.")

    REGISTRY_PARAMETER_INT64(
        REGISTRY_SCHEMA_FULL_EXAMPLE_I64,
        "i64", "Example i64 description.")

    REGISTRY_PARAMETER_FLOAT32(
        REGISTRY_SCHEMA_FULL_EXAMPLE_F32,
        "f32", "Example f32 description.")

    REGISTRY_PARAMETER_FLOAT64(
        REGISTRY_SCHEMA_FULL_EXAMPLE_F64,
        "f64", "Example f64 description.")

    );

static void mapping(const registry_id_t param_id, const registry_instance_t *instance,
                    void **val,
                    size_t *val_len)
{
    registry_schema_full_example_t *_instance = (registry_schema_full_example_t *)instance->data;

    switch (param_id) {
    case REGISTRY_SCHEMA_FULL_EXAMPLE_STRING:
        *val = &_instance->string;
        *val_len = sizeof(_instance->string);
        break;

    case REGISTRY_SCHEMA_FULL_EXAMPLE_BOOL:
        *val = &_instance->boolean;
        *val_len = sizeof(_instance->boolean);
        break;

    case REGISTRY_SCHEMA_FULL_EXAMPLE_U8:
        *val = &_instance->u8;
        *val_len = sizeof(_instance->u8);
        break;

    case REGISTRY_SCHEMA_FULL_EXAMPLE_U16:
        *val = &_instance->u16;
        *val_len = sizeof(_instance->u16);
        break;

    case REGISTRY_SCHEMA_FULL_EXAMPLE_U32:
        *val = &_instance->u32;
        *val_len = sizeof(_instance->u32);
        break;

#if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64)
    case REGISTRY_SCHEMA_FULL_EXAMPLE_U64:
        *val = &_instance->u64;
        *val_len = sizeof(_instance->u64);
        break;
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    case REGISTRY_SCHEMA_FULL_EXAMPLE_I8:
        *val = &_instance->i8;
        *val_len = sizeof(_instance->i8);
        break;

    case REGISTRY_SCHEMA_FULL_EXAMPLE_I16:
        *val = &_instance->i16;
        *val_len = sizeof(_instance->i16);
        break;

    case REGISTRY_SCHEMA_FULL_EXAMPLE_I32:
        *val = &_instance->i32;
        *val_len = sizeof(_instance->i32);
        break;

#if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64)
    case REGISTRY_SCHEMA_FULL_EXAMPLE_I64:
        *val = &_instance->i64;
        *val_len = sizeof(_instance->i64);
        break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32)
    case REGISTRY_SCHEMA_FULL_EXAMPLE_F32:
        *val = &_instance->f32;
        *val_len = sizeof(_instance->f32);
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64)
    case REGISTRY_SCHEMA_FULL_EXAMPLE_F64:
        *val = &_instance->f64;
        *val_len = sizeof(_instance->f64);
        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
    }
}

#endif

/** @} */
