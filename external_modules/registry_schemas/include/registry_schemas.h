/*
 * Copyright (C) 2023 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry_cli RIOT Registry Schemas
 * @ingroup     sys
 * @brief       RIOT Registry Schemas module providing common sys schemas for the RIOT Registry sys module
 * @{
 *
 * @file
 *
 * @author      Lasse Rosenow <lasse.rosenow@haw-hamburg.de>
 */

#ifndef REGISTRY_SCHEMAS_H
#define REGISTRY_SCHEMAS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "registry.h"

void registry_schemas_init(void);

/* Schema IDs */
typedef enum {
    REGISTRY_SCHEMA_FULL_EXAMPLE    = 0,
    REGISTRY_SCHEMA_RGB_LED         = 1,
} registry_schema_id_t;

/* Types-Test */
#if IS_ACTIVE(CONFIG_REGISTRY_ENABLE_SCHEMA_FULL_EXAMPLE) || IS_ACTIVE(DOXYGEN)
extern registry_schema_t registry_schema_full_example;

typedef struct {
    clist_node_t node;

    char string[50];
    bool boolean;

    uint8_t u8;
    uint16_t u16;
    uint32_t u32;

#if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64) || IS_ACTIVE(DOXYGEN)
    uint64_t u64;
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    int8_t i8;
    int16_t i16;
    int32_t i32;

#if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64) || IS_ACTIVE(DOXYGEN)
    int64_t i64;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32) || IS_ACTIVE(DOXYGEN)
    float f32;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64) || IS_ACTIVE(DOXYGEN)
    double f64;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

} registry_schema_full_example_t;

typedef enum {
    REGISTRY_SCHEMA_FULL_EXAMPLE_STRING,
    REGISTRY_SCHEMA_FULL_EXAMPLE_BOOL,

    REGISTRY_SCHEMA_FULL_EXAMPLE_U8,
    REGISTRY_SCHEMA_FULL_EXAMPLE_U16,
    REGISTRY_SCHEMA_FULL_EXAMPLE_U32,

#if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64) || IS_ACTIVE(DOXYGEN)
    REGISTRY_SCHEMA_FULL_EXAMPLE_U64,
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    REGISTRY_SCHEMA_FULL_EXAMPLE_I8,
    REGISTRY_SCHEMA_FULL_EXAMPLE_I16,
    REGISTRY_SCHEMA_FULL_EXAMPLE_I32,

#if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64) || IS_ACTIVE(DOXYGEN)
    REGISTRY_SCHEMA_FULL_EXAMPLE_I64,
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32) || IS_ACTIVE(DOXYGEN)
    REGISTRY_SCHEMA_FULL_EXAMPLE_F32,
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64) || IS_ACTIVE(DOXYGEN)
    REGISTRY_SCHEMA_FULL_EXAMPLE_F64,
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

} registry_schema_full_example_indices_t;
#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_FULL_EXAMPLE */

/* RGB-LED */
#if IS_ACTIVE(CONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED) || IS_ACTIVE(DOXYGEN)
extern registry_schema_t registry_schema_rgb_led;

typedef struct {
    clist_node_t node;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} registry_schema_rgb_led_t;

typedef enum {
    REGISTRY_SCHEMA_RGB_LED_RED,
    REGISTRY_SCHEMA_RGB_LED_GREEN,
    REGISTRY_SCHEMA_RGB_LED_BLUE,
} registry_schema_rgb_led_indices_t;
#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED */

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* REGISTRY_SCHEMAS_H */
