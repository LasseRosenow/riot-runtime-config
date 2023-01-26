/*
 * Copyright (C) 2023 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry RIOT Registry
 * @ingroup     sys
 * @brief       RIOT Registry module for handling runtime configurations
 * @{
 *
 * @file
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @author      Lasse Rosenow <lasse.rosenow@haw-hamburg.de>
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <clist.h>
#include <kernel_defines.h>
#include <assert.h>
#define ENABLE_DEBUG (0)
#include <debug.h>

#include "registry.h"
#include "registry_conversion.h"

registry_namespace_t registry_namespace_sys = {
    .id = REGISTRY_ROOT_GROUP_SYS,
    .name = "sys",
    .description = "List of RIOT sys schemas.",
    .schemas = { .next = NULL },
};

registry_namespace_t registry_namespace_app = {
    .id = REGISTRY_ROOT_GROUP_APP,
    .name = "app",
    .description = "List of custom app schemas.",
    .schemas = { .next = NULL },
};

static const registry_store_instance_t *store_dst;
static clist_node_t store_srcs;

static void _debug_print_path(const registry_path_t path)
{
    if (ENABLE_DEBUG) {
        DEBUG("%d", *path.namespace_id);

        if (path.schema_id != NULL) {
            DEBUG("/%d", *path.schema_id);

            if (path.instance_id != NULL) {
                DEBUG("/%d", *path.instance_id);

                if (path.path_len > 0) {
                    DEBUG("/");

                    for (size_t i = 0; i < path.path_len; i++) {
                        DEBUG("%d", path.path[i]);

                        if (i < path.path_len - 1) {
                            DEBUG("/");
                        }
                    }
                }
            }
        }
    }
}

static void _debug_print_value(const registry_value_t *value)
{
    if (ENABLE_DEBUG) {
        switch (value->type) {
        case REGISTRY_TYPE_NONE: break;
        case REGISTRY_TYPE_OPAQUE: {
            DEBUG("opaque (hex): ");
            for (size_t i = 0; i < value->buf_len; i++) {
                DEBUG("%02x", ((uint8_t *)value->buf)[i]);
            }
            break;
        }
        case REGISTRY_TYPE_STRING: DEBUG("string: %s", (char *)value->buf); break;
        case REGISTRY_TYPE_BOOL: DEBUG("bool: %d", *(bool *)value->buf); break;

        case REGISTRY_TYPE_UINT8: DEBUG("uint8: %d", *(uint8_t *)value->buf); break;
        case REGISTRY_TYPE_UINT16: DEBUG("uint16: %d", *(uint16_t *)value->buf); break;
        case REGISTRY_TYPE_UINT32: DEBUG("uint32: %d", *(uint32_t *)value->buf); break;
    #if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64)
        case REGISTRY_TYPE_UINT64: DEBUG("uint64: %lld", *(uint64_t *)value->buf); break;
    #endif /* CONFIG_REGISTRY_USE_UINT64 */

        case REGISTRY_TYPE_INT8: DEBUG("int8: %d", *(int8_t *)value->buf); break;
        case REGISTRY_TYPE_INT16: DEBUG("int16: %d", *(int16_t *)value->buf); break;
        case REGISTRY_TYPE_INT32: DEBUG("int32: %d", *(int32_t *)value->buf); break;

    #if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64)
        case REGISTRY_TYPE_INT64: DEBUG("int64: %lld", *(int64_t *)value->buf); break;
    #endif /* CONFIG_REGISTRY_USE_INT64 */

    #if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32)
        case REGISTRY_TYPE_FLOAT32: DEBUG("f32: %f", *(float *)value->buf); break;
    #endif /* CONFIG_REGISTRY_USE_FLOAT32 */

    #if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64)
        case REGISTRY_TYPE_FLOAT64: DEBUG("f64: %f", *(double *)value->buf); break;
    #endif /* CONFIG_REGISTRY_USE_FLOAT32 */
        }
    }
}

static int _registry_cmp_schema_id(clist_node_t *current, void *id)
{
    assert(current != NULL);
    registry_schema_t *schema = container_of(current, registry_schema_t, node);

    return !(schema->id - *(int *)id);
}

static registry_namespace_t *_namespace_lookup(const registry_namespace_id_t namespace_id)
{
    switch (namespace_id) {
    case REGISTRY_ROOT_GROUP_SYS:
        return &registry_namespace_sys;
    case REGISTRY_ROOT_GROUP_APP:
        return &registry_namespace_app;
    }

    return NULL;
}

static registry_schema_t *_schema_lookup(const registry_namespace_t *namespace,
                                         const int schema_id)
{
    clist_node_t *node;
    registry_schema_t *schema = NULL;

    node =
        clist_foreach((clist_node_t *)&namespace->schemas, _registry_cmp_schema_id,
                      (clist_node_t *)&schema_id);

    if (node != NULL) {
        schema = container_of(node, registry_schema_t, node);
    }

    return schema;
}

static registry_instance_t *_instance_lookup(const registry_schema_t *schema, const int instance_id)
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
    store_srcs.next = NULL;
}

int registry_register_schema(const registry_namespace_id_t namespace_id,
                             const registry_schema_t *schema)
{
    assert(schema != NULL);

    /* find namespace with correct namespace id */
    registry_namespace_t *namespace = _namespace_lookup(namespace_id);

    if (!namespace) {
        return -EINVAL;
    }

    clist_rpush((clist_node_t *)&namespace->schemas, (clist_node_t *)&(schema->node));

    return 0;
}

static registry_schema_item_t *_parameter_meta_lookup(const registry_path_t path,
                                                      const registry_schema_t *schema)
{
    registry_schema_item_t *schema_item;
    registry_schema_item_t *schema_items = schema->items;
    size_t schema_items_len = schema->items_len;

    for (size_t path_index = 0; path_index < path.path_len; path_index++) {
        for (size_t i = 0; i < schema_items_len; i++) {
            schema_item = &schema->items[i];

            if (schema_item->id == path.path[path_index]) {
                if (schema_item->type == REGISTRY_SCHEMA_TYPE_PARAMETER &&
                    path_index == path.path_len - 1) {
                    /* if this is the last path segment and it is a parameter => return the parameter */
                    return schema_item;
                }
                else if (schema_item->type == REGISTRY_SCHEMA_TYPE_GROUP) {
                    /* if this is not the last path segment and its a group => update schemas and schemas_len values */
                    schema_items = schema_item->value.group.items;
                    schema_items_len = schema_items->value.group.items_len;
                }
            }
        }
    }

    return NULL;
}

int registry_register_schema_instance(const registry_namespace_id_t namespace_id,
                                      const registry_path_item_t schema_id,
                                      const registry_instance_t *instance)
{
    assert(instance != NULL);

    /* find namespace with correct namespace id */
    registry_namespace_t *namespace = _namespace_lookup(namespace_id);

    if (!namespace) {
        return -EINVAL;
    }

    /* find schema with correct schema_id */
    clist_node_t *node = namespace->schemas.next;

    do {
        node = node->next;
        registry_schema_t *schema = container_of(node, registry_schema_t, node);

        /* check if schema has correct schema_id */
        if (schema->id == schema_id) {
            /* add instance to schema */
            clist_rpush((clist_node_t *)&(schema->instances), (clist_node_t *)&instance->node);

            /* count instance index */
            return clist_count(&schema->instances) - 1;
        }
    } while (node != namespace->schemas.next);

    return -EINVAL;
}

static int _registry_set(const registry_path_t path, const void *val, const int val_len,
                         const registry_type_t val_type)
{
    /* lookup namespace */
    registry_namespace_t *namespace = _namespace_lookup(*path.namespace_id);

    if (!namespace) {
        return -EINVAL;
    }

    /* lookup schema */
    registry_schema_t *schema = _schema_lookup(namespace, *path.schema_id);

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

    /* get pointer to registry internal value buffer and length */
    size_t intern_val_len;
    void *intern_val = NULL;

    schema->mapping(param_meta->id, instance, &intern_val, &intern_val_len);

    /* check if val_type is compatible with param_meta->value.parameter.type */
    if (val_type != param_meta->value.parameter.type) {
        uint8_t new_val[intern_val_len];
        registry_value_t old_val = {
            .type = val_type,
            .buf = val,
            .buf_len = val_len,
        };
        int conversion_error_code = registry_convert_value_to_value(&old_val, new_val,
                                                                    intern_val_len,
                                                                    param_meta->value.parameter.type);
        if (conversion_error_code == 0) {
            /* call handler to apply the new value to the correct parameter in the instance of the schema */
            memcpy(intern_val, new_val, intern_val_len);
        }
        else {
            return conversion_error_code;
        }
    }
    else {
        /* call handler to apply the new value to the correct parameter in the instance of the schema */
        memcpy(intern_val, val, intern_val_len);
    }

    return 0;
}

static int _registry_get(const registry_path_t path, const registry_type_t requested_val_type,
                         registry_value_t *val_buf)
{
    /* lookup namespace */
    registry_namespace_t *namespace = _namespace_lookup(*path.namespace_id);

    if (!namespace) {
        return -EINVAL;
    }

    /* lookup schema */
    registry_schema_t *schema = _schema_lookup(namespace, *path.schema_id);

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

    /* if no specific type was requested, set the registry_value_t type to the type of the schema param */
    if (requested_val_type == REGISTRY_TYPE_NONE) {
        val_buf->type = param_meta->value.parameter.type;
    }
    /* check if the requested val_type is compatible with the actual type of the parameter */
    else if (requested_val_type != param_meta->value.parameter.type) {
        return -EINVAL;
    }

    /* call handler to get the parameter value from the instance of the schema */
    void *buf = NULL;
    size_t buf_len;

    schema->mapping(param_meta->id, instance, &buf, &buf_len);

    /* update buf pointer in registry_value_t to point to the value inside the registry and set buf_len */
    val_buf->buf = buf;
    val_buf->buf_len = buf_len;

    return 0;
}

static int _registry_commit_schema(const registry_path_t path)
{
    int rc = 0;

    /* lookup namespace */
    registry_namespace_t *namespace = _namespace_lookup(*path.namespace_id);

    if (!namespace) {
        return -EINVAL;
    }

    /* lookup schema */
    registry_schema_t *schema = _schema_lookup(namespace, *path.schema_id);

    if (!schema) {
        return -EINVAL;
    }

    /* schema/instance */
    if (path.instance_id != NULL) {
        /* lookup instance */
        registry_instance_t *instance = _instance_lookup(schema, *path.instance_id);
        if (!instance) {
            return -EINVAL;
        }
        if (instance->commit_cb) {
            registry_path_t new_path = REGISTRY_PATH(*path.namespace_id, *path.schema_id,
                                                     *path.instance_id);
            int _rc = instance->commit_cb(new_path, instance->context);
            if (!_rc) {
                rc = _rc;
            }
        }
        else {
            rc = -EINVAL;
        }
    }
    /* only schema */
    else {
        for (size_t i = 0; i < clist_count(&schema->instances); i++) {
            registry_instance_t *instance = _instance_lookup(schema, i);
            if (instance->commit_cb) {
                registry_path_t new_path = REGISTRY_PATH(*path.namespace_id, *path.schema_id, i);
                int _rc = instance->commit_cb(new_path, instance->context);
                if (!_rc) {
                    rc = _rc;
                }
            }
            else {
                rc = -EINVAL;
            }
        }
    }

    return rc;
}

static int _registry_commit_namespace(const registry_path_t path)
{
    int rc = 0;

    /* lookup namespace */
    registry_namespace_t *namespace = _namespace_lookup(*path.namespace_id);

    if (!namespace) {
        return -EINVAL;
    }

    /* schema/? */
    if (path.schema_id != NULL) {
        int _rc = _registry_commit_schema(path);
        if (!_rc) {
            rc = _rc;
        }
    }
    /* no schema => call all */
    else {
        clist_node_t *node = namespace->schemas.next;

        if (!node) {
            return -EINVAL;
        }

        do {
            node = node->next;
            registry_schema_t *schema = container_of(node, registry_schema_t, node);

            int _rc = _registry_commit_schema(REGISTRY_PATH(*path.namespace_id, schema->id));
            if (!_rc) {
                rc = _rc;
            }
        } while (node != namespace->schemas.next);
    }

    return rc;
}

int registry_commit(const registry_path_t path)
{
    int rc = 0;

    if (path.namespace_id != NULL) {
        int _rc = _registry_commit_namespace(path);
        if (!_rc) {
            rc = _rc;
        }
    }
    else {
        /* commit sys namespace */
        int _rc = _registry_commit_namespace(REGISTRY_PATH_SYS());
        if (!_rc) {
            rc = _rc;
        }

        /* commit app namespace */
        _rc = _registry_commit_namespace(REGISTRY_PATH_APP());
        if (!_rc) {
            rc = _rc;
        }
    }

    return rc;
}

static void _registry_export_params(int (*export_func)(const registry_path_t path,
                                                       const registry_schema_t *schema,
                                                       const registry_instance_t *instance,
                                                       const registry_schema_item_t *meta,
                                                       const registry_value_t *value,
                                                       const void *context),
                                    const registry_path_t current_path, const registry_schema_t *schema,
                                    const registry_instance_t *instance, const registry_schema_item_t *schema_items,
                                    const size_t schema_items_len, const int recursion_depth, const void *context)
{
    for (size_t i = 0; i < schema_items_len; i++) {
        registry_schema_item_t schema_item = schema_items[i];

        /* create new path including the current schema_item */
        registry_path_item_t _new_path_path[current_path.path_len + 1];
        for (size_t j = 0; j < current_path.path_len; j++) {
            _new_path_path[j] = current_path.path[j];
        }
        _new_path_path[ARRAY_SIZE(_new_path_path) - 1] = schema_item.id;
        registry_path_t new_path = {
            .namespace_id = current_path.namespace_id,
            .schema_id = current_path.schema_id,
            .instance_id = current_path.instance_id,
            .path = _new_path_path,
            .path_len = ARRAY_SIZE(_new_path_path),
        };

        /* check if the current schema_item is a group or a parameter */
        if (schema_item.type == REGISTRY_SCHEMA_TYPE_PARAMETER) {
            /* parameter found => export */
            registry_value_t val;
            registry_get_value(new_path, &val);
            export_func(new_path, schema, instance, &schema_item, &val, context);
        }
        else if (schema_item.type == REGISTRY_SCHEMA_TYPE_GROUP) {
            /* group => search for parameters */
            registry_schema_group_t group = schema_item.value.group;

            export_func(new_path, schema, instance, &schema_item, NULL, context);

            /* if recursion_depth is not 1 then not only the group itself will be exported, but also its children depending on recursion_depth */
            if (recursion_depth != 1) {
                int new_recursion_depth = 0;     // create a new variable, because recursion_depth would otherwise be decreased in each cycle of the for loop
                if (recursion_depth != 0) {
                    new_recursion_depth = recursion_depth - 1;
                }

                _registry_export_params(export_func, new_path, NULL, NULL, group.items,
                                        group.items_len, new_recursion_depth, context);
            }
        }
    }
}

static int _registry_export_instance(int (*export_func)(const registry_path_t path,
                                                        const registry_schema_t *schema,
                                                        const registry_instance_t *instance,
                                                        const registry_schema_item_t *meta,
                                                        const registry_value_t *value,
                                                        const void *context),
                                     const registry_path_t path, const registry_schema_t *schema,
                                     const int recursion_depth, const void *context)
{
    registry_instance_t *instance = _instance_lookup(schema, *path.instance_id);

    if (!instance) {
        return -EINVAL;
    }

    /* export instance */
    export_func(path, schema, instance, NULL, NULL, context);

    /* schema/instance/item => export concrete schema item with data of the given instance */
    if (path.path_len > 0) {
        registry_schema_item_t *schema_item = _parameter_meta_lookup(path, schema);

        /* create a new path which does not include the last value, because _registry_export_params will add it inside */
        registry_path_item_t _new_path_path[path.path_len - 1];
        for (size_t j = 0; j < path.path_len; j++) {
            _new_path_path[j] = path.path[j];
        }
        registry_path_t new_path = {
            .namespace_id = path.namespace_id,
            .schema_id = path.schema_id,
            .instance_id = path.instance_id,
            .path = _new_path_path,
            .path_len = ARRAY_SIZE(_new_path_path),
        };

        _registry_export_params(export_func, new_path, schema, instance,
                                schema_item, 1, recursion_depth, context);
    }
    /* schema/instance => export the schema instance meta data (name) and its parameters recursively depending on recursion_depth */
    else if (path.path_len == 0) {
        /* export instance parameters (recursion_depth == 1 means only the exact path, which would only be a specific instance in this case) */
        if (recursion_depth != 1) {
            int new_recursion_depth = 0;
            if (recursion_depth != 0) {
                new_recursion_depth = recursion_depth - 1;
            }

            _registry_export_params(export_func, path, schema, instance,
                                    schema->items,
                                    schema->items_len, new_recursion_depth, context);
        }
    }

    return 0;
}

static int _registry_export_schema(int (*export_func)(const registry_path_t path,
                                                      const registry_schema_t *schema,
                                                      const registry_instance_t *instance,
                                                      const registry_schema_item_t *meta,
                                                      const registry_value_t *value,
                                                      const void *context),
                                   const registry_path_t path, const int recursion_depth, const void *context)
{
    int rc = 0;

    /* lookup namespace */
    registry_namespace_t *namespace = _namespace_lookup(*path.namespace_id);

    if (!namespace) {
        return -EINVAL;
    }

    /* lookup schema */
    registry_schema_t *schema = _schema_lookup(namespace, *path.schema_id);

    if (!schema) {
        return -EINVAL;
    }

    /* export schema */
    export_func(path, schema, NULL, NULL, NULL, context);

    /* get instance, if in path */
    if (path.instance_id != NULL) {
        int _rc =
            _registry_export_instance(export_func, path, schema, recursion_depth, context);
        if (!_rc) {
            rc = _rc;
        }
    }
    /* schema => Export schema meta data (name, description etc.) and its items depending on recursion_depth */
    else {
        /* export instances (recursion_depth == 1 means only the exact path, which would only be a specific schema in this case) */
        if (recursion_depth != 1) {
            int new_recursion_depth = 0;
            if (recursion_depth != 0) {
                new_recursion_depth = recursion_depth - 1;
            }

            clist_node_t *instance_node = schema->instances.next;

            if (!instance_node) {
                return -EINVAL;
            }

            registry_path_item_t instance_id = 0;

            do {
                instance_node = instance_node->next;
                registry_instance_t *instance = container_of(instance_node, registry_instance_t,
                                                             node);

                if (!instance) {
                    return -EINVAL;
                }

                /* create new path that includes the new instance_id */
                registry_path_t new_path = {
                    .namespace_id = path.namespace_id,
                    .schema_id = path.schema_id,
                    .instance_id = &instance_id,
                    .path = NULL,
                    .path_len = 0,
                };

                int _rc = _registry_export_instance(export_func, new_path, schema,
                                                    new_recursion_depth,
                                                    context);
                if (!_rc) {
                    rc = _rc;
                }

                instance_id++;
            } while (instance_node != schema->instances.next);
        }
    }

    return rc;
}

static int _registry_export_namespace(int (*export_func)(const registry_path_t path,
                                                         const registry_schema_t *schema,
                                                         const registry_instance_t *instance,
                                                         const registry_schema_item_t *meta,
                                                         const registry_value_t *value,
                                                         const void *context),
                                      const registry_path_t path, const int recursion_depth, const void *context)
{
    int rc = 0;

    /* lookup namespace */
    registry_namespace_t *namespace = _namespace_lookup(*path.namespace_id);

    if (!namespace) {
        return -EINVAL;
    }

    /* export namespace */
    export_func(path, NULL, NULL, NULL, NULL, context);

    /* get schema, if in path */
    if (path.schema_id != NULL) {
        int _rc = _registry_export_schema(export_func, path, recursion_depth, context);
        if (!_rc) {
            rc = _rc;
        }
    }
    /* empty path => export everything depending on recursion_depth (0 = everything, 1 = nothing, 2 = all schemas, 3 = all schemas and all their instances etc.) */
    else {
        clist_node_t *schema_node = namespace->schemas.next;

        if (!schema_node) {
            return -EINVAL;
        }

        if (recursion_depth != 1) {
            int new_recursion_depth = 0;
            if (recursion_depth != 0) {
                new_recursion_depth = recursion_depth - 1;
            }

            do {
                schema_node = schema_node->next;
                registry_schema_t *schema = container_of(schema_node, registry_schema_t, node);

                /* create new path that includes the new schema_id */
                registry_path_t new_path = {
                    .namespace_id = path.namespace_id,
                    .schema_id = &schema->id,
                    .instance_id = NULL,
                    .path = NULL,
                    .path_len = 0,
                };

                int _rc = _registry_export_schema(export_func, new_path, new_recursion_depth,
                                                  context);
                if (!_rc) {
                    rc = _rc;
                }
            } while (schema_node != namespace->schemas.next);
        }
    }

    return rc;
}

int registry_export(int (*export_func)(const registry_path_t path,
                                       const registry_schema_t *schema,
                                       const registry_instance_t *instance,
                                       const registry_schema_item_t *meta,
                                       const registry_value_t *value,
                                       const void *context),
                    const registry_path_t path, const int recursion_depth, const void *context)
{
    assert(export_func != NULL);

    int rc = 0;

    DEBUG("[registry export] exporting all in ");
    for (size_t i = 0; i < path.path_len; i++) {
        DEBUG("/%d", path.path[i]);
    }
    DEBUG("\n");

    /* get namespace if in path */
    if (path.namespace_id != NULL) {
        int _rc = _registry_export_namespace(export_func, path, recursion_depth, context);
        if (!_rc) {
            rc = _rc;
        }
    }
    else {
        if (recursion_depth != 1) {
            int new_recursion_depth = 0;
            if (recursion_depth != 0) {
                new_recursion_depth = recursion_depth - 1;
            }

            /* export sys namespace */
            int _rc = _registry_export_namespace(export_func,
                                                 REGISTRY_PATH_SYS(), new_recursion_depth,
                                                 context);
            if (!_rc) {
                rc = _rc;
            }

            /* export app namespace */
            _rc = _registry_export_namespace(export_func,
                                             REGISTRY_PATH_APP(), new_recursion_depth, context);
            if (!_rc) {
                rc = _rc;
            }
        }
    }

    return rc;
}

/* registry_set functions */
int registry_set_value(const registry_path_t path, const registry_value_t val)
{
    return _registry_set(path, val.buf, val.buf_len, val.type);
}

int registry_set_opaque(const registry_path_t path, const void *val, const size_t val_len)
{
    return _registry_set(path, val, val_len, REGISTRY_TYPE_OPAQUE);
}

int registry_set_string(const registry_path_t path, const char *val)
{
    return _registry_set(path, val, strlen(val), REGISTRY_TYPE_STRING);
}

int registry_set_bool(const registry_path_t path, const bool val)
{
    return _registry_set(path, &val, sizeof(bool), REGISTRY_TYPE_BOOL);
}

int registry_set_uint8(const registry_path_t path, const uint8_t val)
{
    return _registry_set(path, &val, sizeof(uint8_t), REGISTRY_TYPE_UINT8);
}

int registry_set_uint16(const registry_path_t path, const uint16_t val)
{
    return _registry_set(path, &val, sizeof(uint16_t), REGISTRY_TYPE_UINT16);
}

int registry_set_uint32(const registry_path_t path, const uint32_t val)
{
    return _registry_set(path, &val, sizeof(uint32_t), REGISTRY_TYPE_UINT32);
}

#if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64)
int registry_set_uint64(const registry_path_t path, const uint64_t val)
{
    return _registry_set(path, &val, sizeof(uint16_t), REGISTRY_TYPE_UINT64);
}

#endif /* CONFIG_REGISTRY_USE_UINT64 */

int registry_set_int8(const registry_path_t path, const int8_t val)
{
    return _registry_set(path, &val, sizeof(int8_t), REGISTRY_TYPE_INT8);
}

int registry_set_int16(const registry_path_t path, const int16_t val)
{
    return _registry_set(path, &val, sizeof(int16_t), REGISTRY_TYPE_INT16);
}

int registry_set_int32(const registry_path_t path, const int32_t val)
{
    return _registry_set(path, &val, sizeof(int32_t), REGISTRY_TYPE_INT32);
}

#if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64)
int registry_set_int64(const registry_path_t path, const int64_t val)
{
    return _registry_set(path, &val, sizeof(int64_t), REGISTRY_TYPE_INT64);
}
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32)
int registry_set_float32(const registry_path_t path, const float val)
{
    return _registry_set(path, &val, sizeof(float), REGISTRY_TYPE_FLOAT32);
}
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64)
int registry_set_float64(const registry_path_t path, const double val)
{
    return _registry_set(path, &val, sizeof(double), REGISTRY_TYPE_FLOAT64);
}
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

/* registry_get functions */
int registry_get_value(const registry_path_t path, registry_value_t *value)
{
    return _registry_get(path, REGISTRY_TYPE_NONE, value);
}

static int _registry_get_buf(const registry_path_t path,
                             const registry_type_t requested_val_type,
                             const void **buf,
                             size_t *buf_len)
{
    registry_value_t value;

    int res = _registry_get(path, requested_val_type, &value);

    *buf = value.buf;

    if (buf_len != NULL) {
        *buf_len = value.buf_len;
    }

    return res;
}
int registry_get_opaque(const registry_path_t path, const void **buf, size_t *buf_len)
{
    return _registry_get_buf(path, REGISTRY_TYPE_OPAQUE, buf, buf_len);
}
int registry_get_string(const registry_path_t path, const char **buf, size_t *buf_len)
{
    return _registry_get_buf(path, REGISTRY_TYPE_STRING, (const void **)buf, buf_len);
}
int registry_get_bool(const registry_path_t path, const bool **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_BOOL, (const void **)buf, NULL);
}
int registry_get_uint8(const registry_path_t path, const uint8_t **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_UINT8, (const void **)buf, NULL);
}
int registry_get_uint16(const registry_path_t path, const uint16_t **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_UINT16, (const void **)buf, NULL);
}

int registry_get_uint32(const registry_path_t path, const uint32_t **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_UINT32, (const void **)buf, NULL);
}

#if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64)
int registry_get_uint64(const registry_path_t path, const uint64_t **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_UINT64, (const void **)buf, NULL);
}
#endif /* CONFIG_REGISTRY_USE_UINT64 */

int registry_get_int8(const registry_path_t path, const int8_t **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_INT8, (const void **)buf, NULL);
}

int registry_get_int16(const registry_path_t path, const int16_t **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_INT16, (const void **)buf, NULL);
}

int registry_get_int32(const registry_path_t path, const int32_t **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_INT32, (const void **)buf, NULL);
}

#if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64)
int registry_get_int64(const registry_path_t path, const int64_t **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_INT64, (const void **)buf, NULL);
}
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32)
int registry_get_float32(const registry_path_t path, const float **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_FLOAT32, (const void **)buf, NULL);
}
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64)
int registry_get_float64(const registry_path_t path, const double **buf)
{
    return _registry_get_buf(path, REGISTRY_TYPE_FLOAT64, (const void **)buf, NULL);
}
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

static void _registry_load_cb(const registry_path_t path, const registry_value_t value,
                              const void *cb_arg)
{
    (void)cb_arg;

    if (ENABLE_DEBUG) {
        DEBUG("[registry_store] Loading: ");
        _debug_print_path(path);
        DEBUG(" = ");
        _debug_print_value(&value);
        DEBUG("\n");
    }

    registry_set_value(path, value);
}

void registry_register_store_src(const registry_store_instance_t *src)
{
    assert(src != NULL);
    clist_rpush((clist_node_t *)&store_srcs, (clist_node_t *)&(src->node));
}

void registry_register_store_dst(const registry_store_instance_t *dst)
{
    assert(dst != NULL);
    store_dst = dst;
}

int registry_load(const registry_path_t path)
{
    clist_node_t *node = store_srcs.next;

    if (!node) {
        return -ENOENT;
    }

    do {
        registry_store_instance_t *src;
        src = container_of(node, registry_store_instance_t, node);
        src->itf->load(src, path, _registry_load_cb, NULL);
    } while (node != store_srcs.next);

    return 0;
}

static void _registry_store_dup_check_cb(const registry_path_t path, const registry_value_t val,
                                         const void *cb_arg)
{
    assert(cb_arg != NULL);
    registry_dup_check_arg_t *dup_arg = (registry_dup_check_arg_t *)cb_arg;

    if (path.namespace_id != dup_arg->path.namespace_id ||
        path.schema_id != dup_arg->path.schema_id ||
        path.instance_id != dup_arg->path.instance_id) {
        return;
    }

    for (size_t i = 0; i < path.path_len; i++) {
        if (path.path[i] != dup_arg->path.path[i]) {
            return;
        }
    }

    if (memcmp(val.buf, dup_arg->val.buf, val.buf_len) == 0) {
        dup_arg->is_dup = true;
    }
}

static int _registry_save_export_func(const registry_path_t path,
                                      const registry_schema_t *schema,
                                      const registry_instance_t *instance,
                                      const registry_schema_item_t *meta,
                                      const registry_value_t *value,
                                      const void *context)
{
    (void)schema;
    (void)meta;
    (void)instance;
    (void)context;
    (void)_registry_store_dup_check_cb;

    /* The registry also exports just the namespace or just a schema, but the storage facility is only interested in paths with values */
    if (value == NULL) {
        return 0;
    }

    const registry_store_instance_t *dst = store_dst;

    if (ENABLE_DEBUG) {
        DEBUG("[registry_store] Saving: ");
        _debug_print_path(path);
        DEBUG(" = ");
        _debug_print_value(value);
        DEBUG("\n");
    }

    if (!dst) {
        return -ENOENT;
    }

    // TODO use registry_load_one() to remove overhead
    // registry_dup_check_arg_t dup = {
    //     .path = path,
    //     .val = *value,
    //     .is_dup = false,
    // };

    // store_dst->itf->load(store_dst, _registry_store_dup_check_cb, &dup);

    // if (dup.is_dup) {
    //     return -EEXIST;
    // }

    return dst->itf->save(dst, path, *value);
}

int registry_save(const registry_path_t path)
{
    int res;

    if (!store_dst) {
        return -ENOENT;
    }

    if (store_dst->itf->save_start) {
        store_dst->itf->save_start(store_dst);
    }

    res = registry_export(_registry_save_export_func, path, 0, NULL);

    if (store_dst->itf->save_end) {
        store_dst->itf->save_end(store_dst);
    }

    return res;
}
