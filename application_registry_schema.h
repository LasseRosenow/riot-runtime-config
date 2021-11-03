#ifndef RUNTIME_CONFIG_REGISTRY_SCHEMA_H
#define RUNTIME_CONFIG_REGISTRY_SCHEMA_H

extern registry_schema_t my_schema;

typedef struct {
    clist_node_t node;
    bool is_enabled;
    int threshold;
    char* name;
} my_schema_t;

typedef enum {
    IS_ENABLED,
    THRESHOLD,
    NAME,
} my_schema_indices_t;

#endif //RUNTIME_CONFIG_REGISTRY_SCHEMA_H
