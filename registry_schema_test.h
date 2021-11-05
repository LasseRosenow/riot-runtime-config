#ifndef RUNTIME_CONFIG_REGISTRY_SCHEMA_TEST_H
#define RUNTIME_CONFIG_REGISTRY_SCHEMA_TEST_H

extern registry_schema_t registry_schema_test;

typedef struct {
    clist_node_t node;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    
#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    int64_t i64;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
    float f32;
#endif /* CONFIG_REGISTRY_USE_FLOAT */

    bool boolean;
    char* string;
} registry_schema_test_t;

typedef enum {
    REGISTRY_SCHEMA_TEST_I_8,
    REGISTRY_SCHEMA_TEST_I_16,
    REGISTRY_SCHEMA_TEST_I_32,
    
#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    REGISTRY_SCHEMA_TEST_I_64,
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
    REGISTRY_SCHEMA_TEST_FLOAT,
#endif /* CONFIG_REGISTRY_USE_FLOAT */

    REGISTRY_SCHEMA_TEST_BOOL,
    REGISTRY_SCHEMA_TEST_STRING,
} registry_schema_test_indices_t;

#endif //RUNTIME_CONFIG_REGISTRY_SCHEMA_TEST_H
