#ifndef RUNTIME_CONFIG_REGISTRY_SCHEMA_RGB_H
#define RUNTIME_CONFIG_REGISTRY_SCHEMA_RGB_H

extern registry_schema_t registry_schema_rgb;

typedef struct {
    clist_node_t node;
    int r;
    int g;
    int b;
} registry_schema_rgb_t;

typedef enum {
    REGISTRY_SCHEMA_RGB_RED,
    REGISTRY_SCHEMA_RGB_GREEN,
    REGISTRY_SCHEMA_RGB_BLUE,
} registry_schema_rgb_indices_t;

#endif //RUNTIME_CONFIG_REGISTRY_SCHEMA_RGB_H
