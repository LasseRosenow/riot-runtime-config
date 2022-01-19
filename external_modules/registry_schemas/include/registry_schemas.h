#ifndef REGISTRY_REGISTRY_SCHEMAS_H
#define REGISTRY_REGISTRY_SCHEMAS_H

#ifdef __cplusplus
extern "C" {
#endif

void registry_schema_init(void);

/* Schema IDs */
typedef enum {
    REGISTRY_SCHEMA_TYPES_TEST  = 0,
    REGISTRY_SCHEMA_RGB         = 1,
} registry_schema_id_t;

/* Types-Test */
#if defined(CONFIG_REGISTRY_ENABLE_SCHEMA_TYPES_TEST) || defined(DOXYGEN)
extern registry_schema_t registry_schema_types_test;

typedef struct {
    clist_node_t node;

    bool boolean;
    char string[REGISTRY_MAX_VAL_LEN];

    uint8_t u8;
    uint16_t u16;
    uint32_t u32;

#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
    uint64_t u64;
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    int8_t i8;
    int16_t i16;
    int32_t i32;

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    int64_t i64;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
    float f32;
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
    double f64;
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

} registry_schema_types_test_t;

typedef enum {
    REGISTRY_SCHEMA_TYPES_TEST_BOOL,
    REGISTRY_SCHEMA_TYPES_TEST_STRING,

    REGISTRY_SCHEMA_TYPES_TEST_U8,
    REGISTRY_SCHEMA_TYPES_TEST_U16,
    REGISTRY_SCHEMA_TYPES_TEST_U32,

#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
    REGISTRY_SCHEMA_TYPES_TEST_U64,
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    REGISTRY_SCHEMA_TYPES_TEST_I8,
    REGISTRY_SCHEMA_TYPES_TEST_I16,
    REGISTRY_SCHEMA_TYPES_TEST_I32,

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    REGISTRY_SCHEMA_TYPES_TEST_I64,
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
    REGISTRY_SCHEMA_TYPES_TEST_F32,
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
    REGISTRY_SCHEMA_TYPES_TEST_F64,
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

} registry_schema_types_test_indices_t;
#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_TYPES_TEST */

/* RGB-LED */
#if defined(CONFIG_REGISTRY_ENABLE_SCHEMA_RGB) || defined(DOXYGEN)
extern registry_schema_t registry_schema_rgb;

typedef struct {
    clist_node_t node;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} registry_schema_rgb_t;

typedef enum {
    REGISTRY_SCHEMA_RGB_RED,
    REGISTRY_SCHEMA_RGB_GREEN,
    REGISTRY_SCHEMA_RGB_BLUE,
} registry_schema_rgb_indices_t;
#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_RGB */

#ifdef __cplusplus
}
#endif

#endif /* REGISTRY_REGISTRY_SCHEMAS_H */
