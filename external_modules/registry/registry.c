#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <clist.h>
#include <kernel_defines.h>
#include <assert.h>
#define ENABLE_DEBUG (0)
#include <debug.h>

#include "registry.h"

static int _registry_call_commit(clist_node_t *current, void *name);

clist_node_t registry_schemas;

static int _registry_cmp_schema_id(clist_node_t *current, void *id)
{
    assert(current != NULL);
    registry_schema_t *schema = container_of(current, registry_schema_t, node);
    return !(schema->id - *(int*)id);
}

static registry_schema_t *_schema_lookup(int id)
{
    clist_node_t *node;
    registry_schema_t *schema = NULL;
    node = clist_foreach(&registry_schemas, _registry_cmp_schema_id, &id);

    if (node != NULL) {
        schema = container_of(node, registry_schema_t, node);
    }

    return schema;
}

static void *_instance_lookup(registry_schema_t *schema, int instance_id) {
    assert(schema != NULL);

    /* find instance with correct instance_id */
    clist_node_t *node = schema->instances.next;
    int index = 0;
    do {
        node = node->next;
        // TODO registry_schema_t is WRONG (registry_schema_t should be registry_instance_t but this type does not exist)
        void *instance = container_of(node, registry_schema_t, node);
        
        /* check if index equals instance_id */
        if (index == instance_id) {
            return instance;
        }

        index++;
    } while (node != schema->instances.next);

    return NULL;
}

void registry_init(void)
{
    registry_schemas.next = NULL;
    registry_store_init();
}

void registry_register(registry_schema_t *schema)
{
    assert(schema != NULL);
    clist_rpush(&registry_schemas, &(schema->node));
}

static size_t _get_registry_parameter_data_len(registry_type_t type)
{
    switch (type) {
        case REGISTRY_TYPE_STRING: return REGISTRY_MAX_VAL_LEN;
        case REGISTRY_TYPE_BOOL: return sizeof(bool);

        case REGISTRY_TYPE_UINT8: return sizeof(uint8_t);
        case REGISTRY_TYPE_UINT16: return sizeof(uint16_t);
        case REGISTRY_TYPE_UINT32: return sizeof(uint32_t);
#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
        case REGISTRY_TYPE_UINT64: return sizeof(uint64_t);
#endif // CONFIG_REGISTRY_USE_UINT64

        case REGISTRY_TYPE_INT8: return sizeof(int8_t);
        case REGISTRY_TYPE_INT16: return sizeof(int16_t);
        case REGISTRY_TYPE_INT32: return sizeof(int32_t);

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
        case REGISTRY_TYPE_INT64: return sizeof(int64_t);
#endif // CONFIG_REGISTRY_USE_INT64

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
        case REGISTRY_TYPE_FLOAT32: return sizeof(float);
#endif // CONFIG_REGISTRY_USE_FLOAT32

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
        case REGISTRY_TYPE_FLOAT64: return sizeof(double);
#endif // CONFIG_REGISTRY_USE_FLOAT32
        
        default: return 0;
    }
}

static registry_schema_item_t *_parameter_meta_lookup(const int *path, int path_len, registry_schema_t *schema) {
    registry_schema_item_t *schema_item;
    registry_schema_item_t *schema_items = schema->items;
    int schema_items_len = schema->items_len;

    for (int path_index = 0; path_index < path_len; path_index++) {
        for (int i = 0; i < schema_items_len; i++) {
            schema_item = &schema->items[i];

            if (schema_item->id == path[path_index]) {
                if (schema_item->type == REGISTRY_SCHEMA_TYPE_PARAMETER && path_index == path_len -1) {
                    // If this is the last path segment and it is a parameter => return the parameter
                    return schema_item;
                } else if (schema_item->type == REGISTRY_SCHEMA_TYPE_GROUP) {
                    // If this is not the last path segment and its a group => update schemas and schemas_len values
                    schema_items = schema_item->value.group.items;
                    schema_items_len = schema_items->value.group.items_len;
                }
            }
        }
    }

    return NULL;
}

int registry_add_instance(int schema_id, clist_node_t* instance)
{
    assert(instance != NULL);

    /* find schema with correct schema_id */
    clist_node_t *node = registry_schemas.next;
    do {
        node = node->next;
        registry_schema_t *schema = container_of(node, registry_schema_t, node);
        
        /* check if schema has correct schema_id */
        if (schema->id == schema_id) {
            /* add instance to schema */
            clist_rpush(&(schema->instances), instance);

            /* count instance index */
            return clist_count(schema->instances.next) - 1;
        }
    } while (node != registry_schemas.next);

    return -EINVAL;
}

int registry_set_value(int *path, int path_len, char *val_str)
{
    /* lookup schema */
    registry_schema_t *schema = _schema_lookup(path[0]);
    if (!schema) {
        return -EINVAL;
    }

    /* lookup instance */
    void *instance = _instance_lookup(schema, path[1]);
    if (!instance) {
        return -EINVAL;
    }

    /* lookup parameter meta data */
    registry_schema_item_t *param_meta = _parameter_meta_lookup(path, path_len, schema);
    if (!param_meta) {
        return -EINVAL;
    }

    /* convert string value to native value */
    int buf_len = REGISTRY_MAX_VAL_LEN;
    uint8_t buf[buf_len]; /* max_val_len is the largest allowed size as a string => largest size in general */
    registry_value_from_str(val_str, param_meta->value.parameter.type, &buf, _get_registry_parameter_data_len(param_meta->value.parameter.type));

    /* call handler to apply the new value to the correct parameter in the instance of the schema */
    schema->set(param_meta->id, instance, &buf, buf_len, schema->context);

    return 0;
}

char *registry_get_value(const int *path, int path_len, char *buf, int buf_len)
{
    /* lookup schema */
    registry_schema_t *schema = _schema_lookup(path[0]);
    if (!schema) {
        return NULL;
    }

    /* lookup instance */
    void *instance = _instance_lookup(schema, path[1]);
    if (!instance) {
        return NULL;
    }

    /* lookup parameter meta data */
    registry_schema_item_t *param_meta = _parameter_meta_lookup(path, path_len, schema);
    if (!param_meta) {
        return NULL;
    }

    /* call handler to get the parameter value from the instance of the schema */
    int native_buf_len = REGISTRY_MAX_VAL_LEN;
    uint8_t native_buf[native_buf_len]; /* max_val_len is the largest allowed size as a string => largest size in general */
    schema->get(param_meta->id, instance, native_buf, native_buf_len, schema->context);

    /* convert native value to string value */
    registry_str_from_value(param_meta->value.parameter.type, &native_buf, buf, buf_len);

    return buf;
}

static int _registry_call_commit(clist_node_t *current, void *res)
{
    assert(current != NULL);
    int _res = *(int *)res;
    registry_schema_t *schema = container_of(current, registry_schema_t, node);
    if (schema->commit_cb) {
        _res = schema->commit_cb(schema->context);
        if (!*(int *)res) {
            *(int *)res = _res;
        }
    }
    return 0;
}

int registry_commit(int *path, int path_len)
{
    int rc = 0;

    if (path_len > 0) {
        registry_schema_t *schema = _schema_lookup(path[0]);
        if (!schema) {
            return -EINVAL;
        }

        if (schema->commit_cb) {
            return schema->commit_cb(schema->context);
        } else {
            return 0;
        }
    }
    else {
        clist_foreach(&registry_schemas, _registry_call_commit,
                      (void *)(&rc));
        return rc;
    }
}

static void _registry_export_recursive(int (*export_func)(const int *path, int path_len, registry_schema_item_t *meta, char* val, void *context), int *current_path, int current_path_len, registry_schema_item_t *schema_items, int schema_items_len, void *context)
{
    for (int i = 0; i < schema_items_len; i++) {
        registry_schema_item_t schema_item = schema_items[i];

        int new_path_len = current_path_len + 1;
        int new_path[new_path_len];
        for (int j = 0; j < current_path_len; j++) {
            new_path[j] = current_path[j];
        }
        new_path[new_path_len - 1] = schema_item.id;

        if (schema_item.type == REGISTRY_SCHEMA_TYPE_PARAMETER) {
            // Parameter found => Export
            char val_buf[REGISTRY_MAX_VAL_LEN] = {0};
            registry_get_value(new_path, new_path_len, val_buf, ARRAY_SIZE(val_buf));
            export_func(new_path, new_path_len, &schema_item, val_buf, context);
        } else if (schema_item.type == REGISTRY_SCHEMA_TYPE_GROUP) {
            // Group => search for parameters
            registry_group_t group = schema_item.value.group;
            for (int i = 0; i < group.items_len; i++) {
                new_path[new_path_len - 1] = schema_item.id;
                _registry_export_recursive(export_func, new_path, current_path_len + 1, group.items, group.items_len, context);
            }
        }
    }
}

int registry_export(int (*export_func)(const int *path, int path_len, registry_schema_item_t *meta, char* val, void *context), int *path, int path_len)
{    
    assert(export_func != NULL);
    registry_schema_t *schema;

    DEBUG("[registry export] exporting all in ");
    for (int i = 0; i < path_len; i++) {
        DEBUG("/%d", path[i]);
    }
    DEBUG("\n");

    // Get schema, if in path
    if (path_len >= 1) {
        schema = _schema_lookup(path[0]);
        if (!schema) {
            return -EINVAL;
        }

        // Schema/Instance/Item => Export concrete schema item with data of the given instance
        if (path_len >= 3) {
            registry_schema_item_t *schema_item = _parameter_meta_lookup(path, path_len, schema);

            // Create a new path which does not include the last value, because _registry_export_recursive will add it inside
            int new_path_len = path_len - 1;
            int new_path[new_path_len];
            for (int j = 0; j < path_len; j++) {
                new_path[j] = path[j];
            }
            
            _registry_export_recursive(export_func, new_path, new_path_len, schema_item, 1, schema->context);
        }
        // Schema/Instance => Export all schema items with data of the given instance
        else if (path_len == 2) {
            _registry_export_recursive(export_func, path, path_len, schema->items, schema->items_len, schema->context);
        }
        // Schema => Export all schema items with data of all instances
        else if (path_len == 1) {
            for (size_t i = 0; i < clist_count(&schema->instances); i++) {
                int new_path[] = {path[0], i};
                _registry_export_recursive(export_func, new_path, ARRAY_SIZE(new_path), schema->items, schema->items_len, schema->context);
            }
        }
    }
    // Empty path => Export all schema items of all schemas with data of all instances
    else if (path_len == 0) {
        clist_node_t *node = registry_schemas.next;

        if (!node) {
            return -1;
        }

        do {
            node = node->next;
            schema = container_of(node, registry_schema_t, node);

            for (size_t i = 0; i < clist_count(&schema->instances); i++) {
                int new_path[] = {schema->id, i};
                _registry_export_recursive(export_func, new_path, ARRAY_SIZE(new_path), schema->items, schema->items_len, schema->context);
            }
        } while (node != registry_schemas.next);
    }

    return 0;
}
