#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <clist.h>
#include <kernel_defines.h>
#include <assert.h>
#define ENABLE_DEBUG (0)
#include <debug.h>

#include "registry.h"

clist_node_t registry_schemas_sys;
clist_node_t registry_schemas_app;

static int _registry_cmp_schema_id(clist_node_t *current, void *id)
{
    assert(current != NULL);
    registry_schema_t *schema = container_of(current, registry_schema_t, node);

    return !(schema->id - *(int *)id);
}

static size_t _get_registry_parameter_data_len(registry_type_t type)
{
    switch (type) {
    case REGISTRY_TYPE_STRING: return REGISTRY_MAX_VAL_LEN;
    case REGISTRY_TYPE_BOOL: return sizeof(bool);

    case REGISTRY_TYPE_UINT8: return sizeof(uint8_t);
    case REGISTRY_TYPE_UINT16: return sizeof(uint16_t);
    case REGISTRY_TYPE_UINT32: return sizeof(uint32_t);
#if defined(CONFIG_REGISTRY_USE_UINT64)
    case REGISTRY_TYPE_UINT64: return sizeof(uint64_t);
#endif // CONFIG_REGISTRY_USE_UINT64

    case REGISTRY_TYPE_INT8: return sizeof(int8_t);
    case REGISTRY_TYPE_INT16: return sizeof(int16_t);
    case REGISTRY_TYPE_INT32: return sizeof(int32_t);

#if defined(CONFIG_REGISTRY_USE_INT64)
    case REGISTRY_TYPE_INT64: return sizeof(int64_t);
#endif // CONFIG_REGISTRY_USE_INT64

#if defined(CONFIG_REGISTRY_USE_FLOAT32)
    case REGISTRY_TYPE_FLOAT32: return sizeof(float);
#endif // CONFIG_REGISTRY_USE_FLOAT32

#if defined(CONFIG_REGISTRY_USE_FLOAT64)
    case REGISTRY_TYPE_FLOAT64: return sizeof(double);
#endif // CONFIG_REGISTRY_USE_FLOAT32

    default: return 0;
    }
}

static registry_schema_t *_schema_lookup(int id)
{
    clist_node_t *node;
    registry_schema_t *schema = NULL;

    node = clist_foreach(&registry_schemas_sys, _registry_cmp_schema_id, &id);

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
    registry_schemas_sys.next = NULL;
    registry_store_init();
}

void registry_register_schema(registry_schema_t *schema)
{
    assert(schema != NULL);
    clist_rpush(&registry_schemas_sys, &(schema->node));
}

static registry_schema_item_t *_parameter_meta_lookup(const registry_path_t path,
                                                      registry_schema_t *schema)
{
    registry_schema_item_t *schema_item;
    registry_schema_item_t *schema_items = schema->items;
    int schema_items_len = schema->items_len;

    for (int path_index = 0; path_index < path.path_len; path_index++) {
        for (int i = 0; i < schema_items_len; i++) {
            schema_item = &schema->items[i];

            if (schema_item->id == path.path[path_index]) {
                if (schema_item->type == REGISTRY_SCHEMA_TYPE_PARAMETER &&
                    path_index == path.path_len - 1) {
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
    clist_node_t *node = registry_schemas_sys.next;

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
    } while (node != registry_schemas_sys.next);

    return -EINVAL;
}

static int _registry_set(const registry_path_t path, const void *val, int val_len,
                         registry_type_t val_type)
{
    /* lookup schema */
    registry_schema_t *schema = _schema_lookup(*path.schema_id);

    if (!schema) {
        return -EINVAL;
    }

    /* lookup instance */
    registry_instance_t *instance = _instance_lookup(schema, *path.instance_id);

    if (!instance) {
        return -EINVAL;
    }

    /* lookup parameter meta data */
    registry_schema_item_t *param_meta = _parameter_meta_lookup(path, schema);

    if (!param_meta) {
        return -EINVAL;
    }

    /* check if val_type is compatible with param_meta->value.parameter.type */
    if (val_type != param_meta->value.parameter.type) {
        int new_val_len = _get_registry_parameter_data_len(val_type);
        uint8_t new_val[new_val_len];
        int conversion_error_code = registry_convert_value_from_value(val, val_type,
                                                                      new_val, new_val_len,
                                                                      param_meta->value.parameter.type);
        if (conversion_error_code == 0) {
            /* call handler to apply the new value to the correct parameter in the instance of the schema */
            schema->set(param_meta->id, instance, new_val, new_val_len, instance->context);
        }
        else {
            return conversion_error_code;
        }
    }
    else {
        /* call handler to apply the new value to the correct parameter in the instance of the schema */
        schema->set(param_meta->id, instance, val, val_len, instance->context);
    }

    return 0;
}

static int _registry_get(const registry_path_t path, registry_value_t *val,
                         registry_type_t val_type)
{
    /* lookup schema */
    registry_schema_t *schema = _schema_lookup(*path.schema_id);

    if (!schema) {
        return -EINVAL;
    }

    /* lookup instance */
    registry_instance_t *instance = _instance_lookup(schema, *path.instance_id);

    if (!instance) {
        return -EINVAL;
    }

    /* lookup parameter meta data */
    registry_schema_item_t *param_meta = _parameter_meta_lookup(path, schema);

    if (!param_meta) {
        return -EINVAL;
    }

    /* call handler to get the parameter value from the instance of the schema */
    uint8_t buf[REGISTRY_MAX_VAL_LEN]; /* max_val_len is the largest allowed size as a string => largest size in general */

    schema->get(param_meta->id, instance, buf, ARRAY_SIZE(buf), instance->context);

    /* check if val_type is requested and compatible with param_meta->value.parameter.type */
    if (val_type != REGISTRY_TYPE_NONE && val_type != param_meta->value.parameter.type) {
        int new_buf_len = _get_registry_parameter_data_len(val_type);
        uint8_t new_buf[new_buf_len];
        int conversion_error_code = registry_convert_value_from_value(buf,
                                                                      param_meta->value.parameter.type,
                                                                      new_buf, new_buf_len,
                                                                      val_type);

        if (conversion_error_code == 0) {
            /* call handler to apply the new value to the correct parameter in the instance of the schema */
            memcpy(val->buf, new_buf, new_buf_len);
        }
        else {
            return conversion_error_code;
        }
    }
    else {
        /* convert native value to string value */
        memcpy(val->buf, buf, val->buf_len);
    }

    return 0;
}

int registry_commit(const registry_path_t path)
{
    int rc = 0;

    /* schema/? */
    if (path.schema_id != NULL) {
        /* lookup schema */
        registry_schema_t *schema = _schema_lookup(*path.schema_id);
        if (!schema) {
            return -EINVAL;
        }

        /* schema/Instance */
        if (path.instance_id != NULL) {
            /* lookup instance */
            registry_instance_t *instance = _instance_lookup(schema, *path.instance_id);
            if (!instance) {
                return -EINVAL;
            }
            return instance->commit_cb(path, instance->context);
        }
        /* only Schema */
        else {
            for (size_t i = 0; i < clist_count(&schema->instances); i++) {
                registry_instance_t *instance = _instance_lookup(schema, i);
                int _rc = instance->commit_cb(path, instance->context);
                if (!_rc) {
                    rc = _rc;
                }
            }
            return rc;
        }
    }
    /* no schema => call all */
    else {
        clist_node_t *node = registry_schemas_sys.next;

        if (!node) {
            return -EINVAL;
        }

        do {
            node = node->next;
            registry_schema_t *schema = container_of(node, registry_schema_t, node);

            for (size_t i = 0; i < clist_count(&schema->instances); i++) {
                registry_instance_t *instance = _instance_lookup(schema, i);
                int _rc = instance->commit_cb(path, instance->context);
                if (!_rc) {
                    rc = _rc;
                }
            }
        } while (node != registry_schemas_sys.next);

        return rc;
    }
}

static void _registry_export_recursive(int (*export_func)(const registry_path_t path,
                                                          const registry_schema_t *schema,
                                                          const registry_instance_t *instance,
                                                          const registry_schema_item_t *meta,
                                                          const registry_value_t *value,
                                                          void *context),
                                       const registry_path_t current_path, registry_schema_t *schema,
                                       registry_instance_t *instance, registry_schema_item_t *schema_items,
                                       int schema_items_len, int recursion_depth, void *context)
{
    for (int i = 0; i < schema_items_len; i++) {
        registry_schema_item_t schema_item = schema_items[i];

        /* Create new path including the current schema_item */
        int _new_path_path[current_path.path_len + 1];
        for (int j = 0; j < current_path.path_len; j++) {
            _new_path_path[j] = current_path.path[j];
        }
        _new_path_path[ARRAY_SIZE(_new_path_path) - 1] = schema_item.id;
        registry_path_t new_path = {
            .root_group = current_path.root_group,
            .schema_id = current_path.schema_id,
            .instance_id = current_path.instance_id,
            .path = _new_path_path,
            .path_len = ARRAY_SIZE(_new_path_path),
        };

        /* Check if the current schema_item is a group or a parameter */
        if (schema_item.type == REGISTRY_SCHEMA_TYPE_PARAMETER) {
            /* Parameter found => Export */
            char val_buf[REGISTRY_MAX_VAL_LEN] = { 0 };
            registry_value_t val = {
                .buf = val_buf,
                .buf_len = ARRAY_SIZE(val_buf),
            };
            registry_get_value(new_path, &val);
            export_func(new_path, schema, instance, &schema_item, &val, context);
        }
        else if (schema_item.type == REGISTRY_SCHEMA_TYPE_GROUP) {
            /* Group => search for parameters */
            registry_schema_group_t group = schema_item.value.group;

            export_func(new_path, schema, instance, &schema_item, NULL, context);

            /* If recursion_depth is not 1 then not only the group itself will be exported, but also its children depending on recursion_depth */
            if (recursion_depth != 1) {
                int new_recursion_depth = 0; // Create a new variable, because recursion_depth would otherwise be decreased in each cycle of the for loop
                if (recursion_depth != 0) {
                    new_recursion_depth = recursion_depth - 1;
                }

                _registry_export_recursive(export_func, new_path, NULL, NULL, group.items,
                                           group.items_len, new_recursion_depth, context);
            }
        }
    }
}

int registry_export(int (*export_func)(const registry_path_t path,
                                       const registry_schema_t *schema,
                                       const registry_instance_t *instance,
                                       const registry_schema_item_t *meta,
                                       const registry_value_t *value,
                                       void *context),
                    const registry_path_t path, int recursion_depth, void *context)
{
    assert(export_func != NULL);
    registry_schema_t *schema;
    registry_instance_t *instance;

    DEBUG("[registry export] exporting all in ");
    for (int i = 0; i < path.path_len; i++) {
        DEBUG("/%d", path.path[i]);
    }
    DEBUG("\n");

    /* Get schema, if in path */
    if (path.schema_id != NULL) {
        schema = _schema_lookup(*path.schema_id);
        if (!schema) {
            return -EINVAL;
        }

        /* Export schema */
        export_func(path, schema, NULL, NULL, NULL, context);

        /* Get instance, if in path */
        if (path.instance_id != NULL) {
            instance = _instance_lookup(schema, *path.instance_id);
            if (!instance) {
                return -EINVAL;
            }

            /* Export instance */
            export_func(path, schema, instance, NULL, NULL, context);

            /* Schema/Instance/Item => Export concrete schema item with data of the given instance */
            if (path.path_len > 0) {
                registry_schema_item_t *schema_item = _parameter_meta_lookup(path, schema);

                /* Create a new path which does not include the last value, because _registry_export_recursive will add it inside */
                int _new_path_path[path.path_len - 1];
                for (int j = 0; j < path.path_len; j++) {
                    _new_path_path[j] = path.path[j];
                }
                registry_path_t new_path = {
                    .root_group = path.root_group,
                    .schema_id = path.schema_id,
                    .instance_id = path.instance_id,
                    .path = _new_path_path,
                    .path_len = ARRAY_SIZE(_new_path_path),
                };

                _registry_export_recursive(export_func, new_path, schema, instance,
                                           schema_item, 1, recursion_depth, context);
            }
            /* Schema/Instance => Export the schema instance meta data (name) and its parameters recursively depending on recursion_depth */
            else if (path.path_len == 0) {
                /* Export instance parameters (recursion_depth == 1 means only the exact path, which would only be a specific instance in this case) */
                if (recursion_depth != 1) {
                    if (recursion_depth != 0) {
                        recursion_depth--;
                    }

                    _registry_export_recursive(export_func, path, schema, instance, schema->items,
                                               schema->items_len, recursion_depth, context);
                }
            }
        }
        /* Schema => Export schema meta data (name, description etc.) and its items depending on recursion_depth */
        else {
            /* Export instances (recursion_depth == 1 means only the exact path, which would only be a specific schema in this case) */
            if (recursion_depth != 1) {
                if (recursion_depth != 0) {
                    recursion_depth--;
                }

                clist_node_t *instance_node = schema->instances.next;

                if (!instance_node) {
                    return -EINVAL;
                }

                int instance_id = 0;

                do {
                    instance_node = instance_node->next;
                    instance = container_of(instance_node, registry_instance_t, node);

                    if (!instance) {
                        return -EINVAL;
                    }

                    /* Create new path that includes the new instance_id */
                    registry_path_t new_path = {
                        .root_group = path.root_group,
                        .schema_id = path.schema_id,
                        .instance_id = &instance_id,
                        .path = NULL,
                        .path_len = 0,
                    };

                    /* Export instance */
                    export_func(new_path, schema, instance, NULL, NULL, context);

                    /* Export instance parameters (recursion_depth == 1 at this point means only the exact path + 1 */
                    if (recursion_depth != 1) {
                        int new_recursion_depth = 0; // Create a new variable, because recursion_depth would otherwise be decreased in each cycle of the for loop
                        if (recursion_depth != 0) {
                            new_recursion_depth = recursion_depth - 1;
                        }

                        _registry_export_recursive(export_func, new_path, schema, instance,
                                                   schema->items, schema->items_len,
                                                   new_recursion_depth, context);
                    }

                    instance_id++;
                } while (instance_node != schema->instances.next);
            }
        }
    }
    /* Empty path => Export everything depending on recursion_depth (0 = everything, 1 = nothing, 2 = all schemas, 3 = all schemas and all their instances etc.) */
    else {
        clist_node_t *schema_node = registry_schemas_sys.next;

        if (!schema_node) {
            return -EINVAL;
        }

        if (recursion_depth != 1) {
            if (recursion_depth != 0) {
                recursion_depth--;
            }

            do {
                schema_node = schema_node->next;
                schema = container_of(schema_node, registry_schema_t, node);

                /* Create new path that includes the new schema_id */
                registry_path_t new_path = {
                    .root_group = path.root_group,
                    .schema_id = &schema->id,
                    .instance_id = NULL,
                    .path = NULL,
                    .path_len = 0,
                };

                /* Export schema */
                export_func(new_path, schema, NULL, NULL, NULL, context);

                if (recursion_depth != 1) {
                    int new_recursion_depth = 0; // Create a new variable, because recursion_depth would otherwise be decreased in each cycle of the for loop
                    if (recursion_depth != 0) {
                        new_recursion_depth = recursion_depth - 1;
                    }

                    clist_node_t *instance_node = schema->instances.next;

                    if (!instance_node) {
                        return -EINVAL;
                    }

                    int instance_id = 0;

                    do {
                        instance_node = instance_node->next;
                        instance = container_of(instance_node, registry_instance_t, node);

                        /* Create new path that includes the new schema_id and instance_id */
                        registry_path_t new_path = {
                            .root_group = path.root_group,
                            .schema_id = &instance_id,
                            .instance_id = NULL,
                            .path = NULL,
                            .path_len = 0,
                        };

                        /* Export instance */
                        export_func(new_path, schema, instance, NULL, NULL, context);

                        /* Export instance parameters */
                        if (new_recursion_depth != 1) {
                            int new_new_recursion_depth = 0; // Create a new variable, because new_recursion_depth would otherwise be decreased in each cycle of the for loop
                            if (new_recursion_depth != 0) {
                                new_new_recursion_depth = new_recursion_depth - 1;
                            }

                            _registry_export_recursive(export_func, new_path, schema, instance,
                                                       schema->items, schema->items_len,
                                                       new_new_recursion_depth, context);
                        }

                        instance_id++;
                    } while (instance_node != schema->instances.next);
                }
            } while (schema_node != registry_schemas_sys.next);
        }
    }

    return 0;
}

/* registry_set functions */
int registry_set_value(const registry_path_t path, const void *val, int val_len)
{
    return _registry_set(path, val, val_len, REGISTRY_TYPE_NONE);
}

int registry_set_string(const registry_path_t path, const char *val)
{
    return _registry_set(path, val, strlen(val), REGISTRY_TYPE_STRING);
}

int registry_set_bool(const registry_path_t path, bool val)
{
    return _registry_set(path, &val, sizeof(bool), REGISTRY_TYPE_BOOL);
}

int registry_set_uint8(const registry_path_t path, uint8_t val)
{
    return _registry_set(path, &val, sizeof(uint8_t), REGISTRY_TYPE_UINT8);
}

int registry_set_uint16(const registry_path_t path, uint16_t val)
{
    return _registry_set(path, &val, sizeof(uint16_t), REGISTRY_TYPE_UINT16);
}

int registry_set_uint32(const registry_path_t path, uint32_t val)
{
    return _registry_set(path, &val, sizeof(uint32_t), REGISTRY_TYPE_UINT32);
}

#if defined(CONFIG_REGISTRY_USE_UINT64)
int registry_set_uint64(const registry_path_t path, uint64_t val)
{
    return _registry_set(path, &val, sizeof(uint16_t), REGISTRY_TYPE_UINT64);
}

#endif /* CONFIG_REGISTRY_USE_UINT64 */

int registry_set_int8(const registry_path_t path, int8_t val)
{
    return _registry_set(path, &val, sizeof(int8_t), REGISTRY_TYPE_INT8);
}

int registry_set_int16(const registry_path_t path, int16_t val)
{
    return _registry_set(path, &val, sizeof(int16_t), REGISTRY_TYPE_INT16);
}

int registry_set_int32(const registry_path_t path, int32_t val)
{
    return _registry_set(path, &val, sizeof(int32_t), REGISTRY_TYPE_INT32);
}

#if defined(CONFIG_REGISTRY_USE_INT64)
int registry_set_int64(const registry_path_t path, int64_t val)
{
    return _registry_set(path, &val, sizeof(int64_t), REGISTRY_TYPE_INT64);
}
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32)
int registry_set_float32(const registry_path_t path, float val)
{
    return _registry_set(path, &val, sizeof(float), REGISTRY_TYPE_FLOAT32);
}
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64)
int registry_set_float64(const registry_path_t path, double val)
{
    return _registry_set(path, &val, sizeof(double), REGISTRY_TYPE_FLOAT64);
}
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

/* registry_get functions */
registry_value_t *registry_get_value(const registry_path_t path, registry_value_t *value)
{
    _registry_get(path, value, REGISTRY_TYPE_NONE);

    return value;
}

static void _registry_get_buf(const registry_path_t path, void *buf, int buf_len,
                              registry_type_t type)
{
    registry_value_t value = {
        .buf = buf,
        .buf_len = buf_len,
    };

    _registry_get(path, &value, type);
}
char *registry_get_string(const registry_path_t path, char *buf, int buf_len)
{
    _registry_get_buf(path, buf, buf_len, REGISTRY_TYPE_STRING);
    return buf;
}
bool registry_get_bool(const registry_path_t path)
{
    bool buf;

    _registry_get_buf(path, &buf, sizeof(bool), REGISTRY_TYPE_BOOL);

    return buf;
}
uint8_t registry_get_uint8(const registry_path_t path)
{
    uint8_t buf;

    _registry_get_buf(path, &buf, sizeof(uint8_t), REGISTRY_TYPE_UINT8);

    return buf;
}
uint16_t registry_get_uint16(const registry_path_t path)
{
    uint16_t buf;

    _registry_get_buf(path, &buf, sizeof(uint16_t), REGISTRY_TYPE_UINT16);

    return buf;
}

uint32_t registry_get_uint32(const registry_path_t path)
{
    uint32_t buf;

    _registry_get_buf(path, &buf, sizeof(uint32_t), REGISTRY_TYPE_UINT32);

    return buf;
}

#if defined(CONFIG_REGISTRY_USE_UINT64)
uint64_t registry_get_uint64(const registry_path_t path)
{
    uint64_t buf;

    _registry_get_buf(path, &buf, sizeof(uint64_t), REGISTRY_TYPE_UINT64);

    return buf;
}
#endif /* CONFIG_REGISTRY_USE_UINT64 */

int8_t registry_get_int8(const registry_path_t path)
{
    int8_t buf;

    _registry_get_buf(path, &buf, sizeof(int8_t), REGISTRY_TYPE_INT8);

    return buf;
}

int16_t registry_get_int16(const registry_path_t path)
{
    int16_t buf;

    _registry_get_buf(path, &buf, sizeof(int16_t), REGISTRY_TYPE_INT16);

    return buf;
}

int32_t registry_get_int32(const registry_path_t path)
{
    int32_t buf;

    _registry_get_buf(path, &buf, sizeof(int32_t), REGISTRY_TYPE_INT32);

    return buf;
}

#if defined(CONFIG_REGISTRY_USE_INT64)
int64_t registry_get_int64(const registry_path_t path)
{
    int64_t buf;

    _registry_get_buf(path, &buf, sizeof(int64_t), REGISTRY_TYPE_INT64);

    return buf;
}
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32)
float registry_get_float32(const registry_path_t path)
{
    float buf;

    _registry_get_buf(path, &buf, sizeof(float), REGISTRY_TYPE_FLOAT32);

    return buf;
}
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64)
double registry_get_float64(const registry_path_t path)
{
    double buf;

    _registry_get_buf(path, &buf, sizeof(double), REGISTRY_TYPE_FLOAT64);

    return buf;
}
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
