#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <clist.h>
#include <kernel_defines.h>
#include <assert.h>
#define ENABLE_DEBUG (0)
#include <debug.h>

#include "registry.h"

clist_node_t registry_schemas;

static int _registry_cmp_schema_id(clist_node_t *current, void *id)
{
    assert(current != NULL);
    registry_schema_t *schema = container_of(current, registry_schema_t, node);

    return !(schema->id - *(int *)id);
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

static registry_instance_t *_instance_lookup(registry_schema_t *schema, int instance_id)
{
    assert(schema != NULL);

    /* find instance with correct instance_id */
    clist_node_t *node = schema->instances.next;
    int index = 0;

    do {
        node = node->next;
        registry_instance_t *instance = container_of(node, registry_instance_t, node);

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

void registry_register_schema(registry_schema_t *schema)
{
    assert(schema != NULL);
    clist_rpush(&registry_schemas, &(schema->node));
}

// TODO is this still necessary?
// static size_t _get_registry_parameter_data_len(registry_type_t type)
// {
//     switch (type) {
//     case REGISTRY_TYPE_STRING: return REGISTRY_MAX_VAL_LEN;
//     case REGISTRY_TYPE_BOOL: return sizeof(bool);
//
//     case REGISTRY_TYPE_UINT8: return sizeof(uint8_t);
//     case REGISTRY_TYPE_UINT16: return sizeof(uint16_t);
//     case REGISTRY_TYPE_UINT32: return sizeof(uint32_t);
// #if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
//     case REGISTRY_TYPE_UINT64: return sizeof(uint64_t);
// #endif // CONFIG_REGISTRY_USE_UINT64
//
//     case REGISTRY_TYPE_INT8: return sizeof(int8_t);
//     case REGISTRY_TYPE_INT16: return sizeof(int16_t);
//     case REGISTRY_TYPE_INT32: return sizeof(int32_t);
//
// #if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
//     case REGISTRY_TYPE_INT64: return sizeof(int64_t);
// #endif // CONFIG_REGISTRY_USE_INT64
//
// #if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
//     case REGISTRY_TYPE_FLOAT32: return sizeof(float);
// #endif // CONFIG_REGISTRY_USE_FLOAT32
//
// #if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
//     case REGISTRY_TYPE_FLOAT64: return sizeof(double);
// #endif // CONFIG_REGISTRY_USE_FLOAT32
//
//     default: return 0;
//     }
// }

static registry_schema_item_t *_parameter_meta_lookup(const int *path, int path_len,
                                                      registry_schema_t *schema)
{
    registry_schema_item_t *schema_item;
    registry_schema_item_t *schema_items = schema->items;
    int schema_items_len = schema->items_len;

    for (int path_index = 0; path_index < path_len; path_index++) {
        for (int i = 0; i < schema_items_len; i++) {
            schema_item = &schema->items[i];

            if (schema_item->id == path[path_index]) {
                if (schema_item->type == REGISTRY_SCHEMA_TYPE_PARAMETER &&
                    path_index == path_len - 1) {
                    // If this is the last path segment and it is a parameter => return the parameter
                    return schema_item;
                }
                else if (schema_item->type == REGISTRY_SCHEMA_TYPE_GROUP) {
                    // If this is not the last path segment and its a group => update schemas and schemas_len values
                    schema_items = schema_item->value.group.items;
                    schema_items_len = schema_items->value.group.items_len;
                }
            }
        }
    }

    return NULL;
}

int registry_add_instance(int schema_id, registry_instance_t *instance)
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
            clist_rpush(&(schema->instances), &instance->node);

            /* count instance index */
            return clist_count(&schema->instances) - 1;
        }
    } while (node != registry_schemas.next);

    return -EINVAL;
}

int registry_set_value(const int *path, int path_len, const void *val, int val_len)
{
    /* lookup schema */
    registry_schema_t *schema = _schema_lookup(path[0]);

    if (!schema) {
        return -EINVAL;
    }

    /* lookup instance */
    registry_instance_t *instance = _instance_lookup(schema, path[1]);

    if (!instance) {
        return -EINVAL;
    }

    /* lookup parameter meta data */
    registry_schema_item_t *param_meta = _parameter_meta_lookup(path, path_len, schema);

    if (!param_meta) {
        return -EINVAL;
    }

    /* call handler to apply the new value to the correct parameter in the instance of the schema */
    schema->set(param_meta->id, instance, val, val_len, schema->context);

    return 0;
}

registry_value_t *registry_get_value(const int *path, int path_len, registry_value_t *value)
{
    /* lookup schema */
    registry_schema_t *schema = _schema_lookup(path[0]);

    if (!schema) {
        return NULL;
    }

    /* lookup instance */
    registry_instance_t *instance = _instance_lookup(schema, path[1]);

    if (!instance) {
        return NULL;
    }

    /* lookup parameter meta data */
    registry_schema_item_t *param_meta = _parameter_meta_lookup(path, path_len, schema);

    if (!param_meta) {
        return NULL;
    }

    /* call handler to get the parameter value from the instance of the schema */
    uint8_t buf[REGISTRY_MAX_VAL_LEN]; /* max_val_len is the largest allowed size as a string => largest size in general */

    schema->get(param_meta->id, instance, buf, ARRAY_SIZE(buf), schema->context);

    /* convert native value to string value */
    memcpy(value->buf, buf, value->buf_len);

    return value;
}

int registry_commit(const int *path, int path_len)
{
    int rc = 0;

    /* Schema/? */
    if (path_len >= 1) {
        /* lookup schema */
        registry_schema_t *schema = _schema_lookup(path[0]);
        if (!schema) {
            return -EINVAL;
        }

        /* Schema/Instance */
        if (path_len >= 2) {
            /* lookup instance */
            registry_instance_t *instance = _instance_lookup(schema, path[1]);
            if (!instance) {
                return -EINVAL;
            }
            return instance->commit_cb(path, path_len, instance->context);
        }
        /* Only Schema */
        else {
            for (size_t i = 0; i < clist_count(&schema->instances); i++) {
                registry_instance_t *instance = _instance_lookup(schema, i);
                int _rc = instance->commit_cb(path, path_len, instance->context);
                if (!_rc) {
                    rc = _rc;
                }
            }
            return rc;
        }
    }
    /* No schema => call all */
    else {
        clist_node_t *node = registry_schemas.next;

        if (!node) {
            return -EINVAL;
        }

        do {
            node = node->next;
            registry_schema_t *schema = container_of(node, registry_schema_t, node);

            for (size_t i = 0; i < clist_count(&schema->instances); i++) {
                registry_instance_t *instance = _instance_lookup(schema, i);
                int _rc = instance->commit_cb(path, path_len, instance->context);
                if (!_rc) {
                    rc = _rc;
                }
            }
        } while (node != registry_schemas.next);

        return rc;
    }
}

static void _registry_export_recursive(int (*export_func)(const int *path, int path_len,
                                                          registry_schema_item_t *meta,
                                                          const registry_value_t *value,
                                                          void *context),
                                       const int *current_path, int current_path_len, registry_schema_item_t *schema_items,
                                       int schema_items_len, int recursion_depth, void *context)
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
            /* Parameter found => Export */
            char val_buf[REGISTRY_MAX_VAL_LEN] = { 0 };
            registry_value_t val = {
                .buf = val_buf,
                .buf_len = ARRAY_SIZE(val_buf),
            };
            registry_get_value(new_path, new_path_len, &val);
            export_func(new_path, new_path_len, &schema_item, &val, context);
        }
        else if (schema_item.type == REGISTRY_SCHEMA_TYPE_GROUP) {
            /* Group => search for parameters */
            registry_schema_group_t group = schema_item.value.group;

            /* If recursion_depth is 0 then only the group itself will be exported */
            if (recursion_depth > 0) {
                for (int i = 0; i < group.items_len; i++) {
                    new_path[new_path_len - 1] = schema_item.id;
                    _registry_export_recursive(export_func, new_path, current_path_len + 1,
                                               group.items,
                                               group.items_len, recursion_depth - 1, context);
                }
            }
            else {
                export_func(new_path, new_path_len, &schema_item, NULL, context);
            }
        }
    }
}

int registry_export(int (*export_func)(const int *path, int path_len, registry_schema_item_t *meta,
                                       const registry_value_t *value,
                                       void *context), const int *path, int path_len, int recursion_depth)
{
    assert(export_func != NULL);
    registry_schema_t *schema;

    DEBUG("[registry export] exporting all in ");
    for (int i = 0; i < path_len; i++) {
        DEBUG("/%d", path[i]);
    }
    DEBUG("\n");

    /* Get schema, if in path */
    if (path_len >= 1) {
        schema = _schema_lookup(path[0]);
        if (!schema) {
            return -EINVAL;
        }

        /* Schema/Instance/Item => Export concrete schema item with data of the given instance */
        if (path_len >= 3) {
            registry_schema_item_t *schema_item = _parameter_meta_lookup(path, path_len, schema);

            /* Create a new path which does not include the last value, because _registry_export_recursive will add it inside */
            int new_path_len = path_len - 1;
            int new_path[new_path_len];
            for (int j = 0; j < path_len; j++) {
                new_path[j] = path[j];
            }

            _registry_export_recursive(export_func, new_path, new_path_len, schema_item, 1,
                                       recursion_depth, schema->context);
        }
        /* Schema/Instance => Export the schema instance meta data (name) and its parameters recursively depending on recursion_depth */
        else if (path_len == 2) {
            /* Export instance TODO export function is not capable */
            export_func(path, path_len, NULL, NULL, schema->context);

            /* Export instance parameters */
            if (recursion_depth > 0) {
                _registry_export_recursive(export_func, path, path_len, schema->items,
                                           recursion_depth, schema->items_len, schema->context);
            }
        }
        /* Schema => Export schema meta data (name, description etc.) and its items depending on recursion_depth */
        else if (path_len == 1) {
            /* Export schema TODO export function is not capable */
            export_func(path, path_len, NULL, NULL, schema->context);
            if (recursion_depth > 0) {
                for (size_t i = 0; i < clist_count(&schema->instances); i++) {
                    int new_path[] = { path[0], i };
                    /* Export instance TODO export function is not capable */
                    export_func(new_path, ARRAY_SIZE(new_path), NULL, NULL, schema->context);

                    /* Export instance parameters */
                    if (recursion_depth > 1) {
                        _registry_export_recursive(export_func, new_path, ARRAY_SIZE(
                                                       new_path), schema->items, schema->items_len,
                                                   recursion_depth, schema->context);
                    }

                }
            }
        }
    }
    /* Empty path => Export everything depending on recursion_depth (0 = nothing, 1 = all schemas, 2 = all schemas and all their instances etc.) */
    else if (path_len == 0) {
        clist_node_t *node = registry_schemas.next;

        if (!node) {
            return -1;
        }

        if (recursion_depth > 0) {
            do {
                node = node->next;
                schema = container_of(node, registry_schema_t, node);

                /* Export schema TODO export function is not capable */
                export_func(path, path_len, NULL, NULL, schema->context);

                if (recursion_depth > 1) {
                    for (size_t i = 0; i < clist_count(&schema->instances); i++) {
                        int new_path[] = { schema->id, i };
                        /* Export instance TODO export function is not capable */
                        export_func(new_path, ARRAY_SIZE(new_path), NULL, NULL, schema->context);

                        /* Export instance parameters */
                        if (recursion_depth > 2) {
                            _registry_export_recursive(export_func, new_path, ARRAY_SIZE(
                                                           new_path), schema->items, schema->items_len,
                                                       recursion_depth, schema->context);
                        }
                    }
                }
            } while (node != registry_schemas.next);
        }
    }

    return 0;
}

/* registry_set_value convenience functions */
int registry_set_string(const int *path, int path_len, const char *val)
{
    return registry_set_value(path, path_len, val, strlen(val));
}

int registry_set_bool(const int *path, int path_len, bool val)
{
    return registry_set_value(path, path_len, &val, sizeof(bool));
}

int registry_set_uint8(const int *path, int path_len, uint8_t val)
{
    return registry_set_value(path, path_len, &val, sizeof(uint8_t));
}

int registry_set_uint16(const int *path, int path_len, uint16_t val)
{
    return registry_set_value(path, path_len, &val, sizeof(uint16_t));
}

int registry_set_uint32(const int *path, int path_len, uint32_t val)
{
    return registry_set_value(path, path_len, &val, sizeof(uint32_t));
}

#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
int registry_set_uint64(const int *path, int path_len, uint64_t val)
{
    return registry_set_value(path, path_len, &val, sizeof(uint16_t));
}

#endif /* CONFIG_REGISTRY_USE_UINT64 */

int registry_set_int8(const int *path, int path_len, int8_t val)
{
    return registry_set_value(path, path_len, &val, sizeof(int8_t));
}

int registry_set_int16(const int *path, int path_len, int16_t val)
{
    return registry_set_value(path, path_len, &val, sizeof(int16_t));
}

int registry_set_int32(const int *path, int path_len, int32_t val)
{
    return registry_set_value(path, path_len, &val, sizeof(int32_t));
}

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
int registry_set_int64(const int *path, int path_len, int64_t val)
{
    return registry_set_value(path, path_len, &val, sizeof(int64_t));
}
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
int registry_set_float32(const int *path, int path_len, float val)
{
    return registry_set_value(path, path_len, &val, sizeof(float));
}
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
int registry_set_float64(const int *path, int path_len, double val)
{
    return registry_set_value(path, path_len, &val, sizeof(double));
}
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

/* registry_get_value convenience functions */
static void _registry_get_value_buf(const int *path, int path_len, void *buf, int buf_len)
{
    registry_value_t value = {
        .buf = buf,
        .buf_len = buf_len,
    };

    registry_get_value(path, path_len, &value);
}
char *registry_get_string(const int *path, int path_len, char *buf, int buf_len)
{
    _registry_get_value_buf(path, path_len, buf, buf_len);
    return buf;
}
bool registry_get_bool(const int *path, int path_len)
{
    bool buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(bool));

    return buf;
}
uint8_t registry_get_uint8(const int *path, int path_len)
{
    uint8_t buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(uint8_t));

    return buf;
}
uint16_t registry_get_uint16(const int *path, int path_len)
{
    uint16_t buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(uint16_t));

    return buf;
}

uint32_t registry_get_uint32(const int *path, int path_len)
{
    uint32_t buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(uint32_t));

    return buf;
}

#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
uint64_t registry_get_uint64(const int *path, int path_len)
{
    uint64_t buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(uint64_t));

    return buf;
}
#endif /* CONFIG_REGISTRY_USE_UINT64 */

int8_t registry_get_int8(const int *path, int path_len)
{
    int8_t buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(int8_t));

    return buf;
}

int16_t registry_get_int16(const int *path, int path_len)
{
    int16_t buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(int16_t));

    return buf;
}

int32_t registry_get_int32(const int *path, int path_len)
{
    int32_t buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(int32_t));

    return buf;
}

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
int64_t registry_get_int64(const int *path, int path_len)
{
    int64_t buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(int64_t));

    return buf;
}
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
float registry_get_float32(const int *path, int path_len)
{
    float buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(float));

    return buf;
}
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
double registry_get_float64(const int *path, int path_len)
{
    double buf;

    _registry_get_value_buf(path, path_len, &buf, sizeof(double));

    return buf;
}
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
