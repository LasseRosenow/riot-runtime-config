#ifndef RUNTIME_CONFIG_REGISTRY_SCHEMA_2_H
#define RUNTIME_CONFIG_REGISTRY_SCHEMA_2_H

extern registry_schema_t my_schema_2;

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
} my_schema_2_t;

typedef enum {
    I_8,
    I_16,
    I_32,
    
#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    I_64,
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
    FLOAT,
#endif /* CONFIG_REGISTRY_USE_FLOAT */

    BOOL,
    STRING,
} schema_indices_t;

#endif //RUNTIME_CONFIG_REGISTRY_SCHEMA_2_H
