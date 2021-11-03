#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <clist.h>
#include <kernel_defines.h>
#include <assert.h>
#define ENABLE_DEBUG (0)
#include <debug.h>

#include "registry.h"

static int _registry_cmp_id(clist_node_t *current, void *id);
static int _registry_call_commit(clist_node_t *current, void *name);

clist_node_t registry_schemas;

static int _registry_cmp_id(clist_node_t *current, void *id)
{
    assert(current != NULL);
    registry_schema_t *hndlr = container_of(current, registry_schema_t, node);
    return !(hndlr->id - *(int*)id);
}

static registry_schema_t *_schema_lookup(int id)
{
    clist_node_t *node;
    registry_schema_t *hndlr = NULL;
    node = clist_foreach(&registry_schemas, _registry_cmp_id, &id);

    if (node != NULL) {
        hndlr = container_of(node, registry_schema_t, node);
    }

    return hndlr;
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

// TODO
/* static size_t _get_registry_parameter_data_len(registry_type_t type)
{
    switch (type) {
        case REGISTRY_TYPE_INT8: return sizeof(int8_t);
        case REGISTRY_TYPE_INT16: return sizeof(int16_t);
        case REGISTRY_TYPE_INT32: return sizeof(int32_t);
        case REGISTRY_TYPE_STRING: return REGISTRY_MAX_VAL_LEN;
        case REGISTRY_TYPE_BOOL: return sizeof(bool);
#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
        case REGISTRY_TYPE_INT64: return sizeof(int64_t);
#endif // CONFIG_REGISTRY_USE_INT64

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
        case REGISTRY_TYPE_FLOAT: return sizeof(float);
#endif // CONFIG_REGISTRY_USE_FLOAT
        
        default: return 0;
    }
} */

static void *_instance_lookup(registry_schema_t *schema, int instance_id) {
    assert(schema != NULL);

    /* find instance with correct instance_id */
    clist_node_t *node = schema->instances.next;
    int index = 0;
    do {
        node = node->next;
        void *instance = container_of(node, void, node);
        
        /* check if index equals instance_id */
        if (index == instance_id) {
            return instance;
        }

        index++;
    } while (node != schema->instances.next);

    return NULL;
}

static registry_schema_item_t *_parameter_meta_lookup(int *path, int path_len, registry_schema_t *hndlr) {
    registry_schema_item_t *schema;
    registry_schema_item_t *schemas = hndlr->schemas;
    int schemas_len = hndlr->schemas_len;

    for (int path_index = 0; path_index < path_len; path_index++) {
        for (int i = 0; i < schemas_len; i++) {
            schema = &hndlr->schemas[i];

            if (schema->id == path[path_index]) {
                if (schema->type == REGISTRY_SCHEMA_TYPE_PARAMETER && path_index == path_len -1) {
                    // If this is the last path segment and it is a parameter => return the parameter
                    return schema;
                } else if (schema->type == REGISTRY_SCHEMA_TYPE_GROUP) {
                    // If this is not the last path segment and its a group => update schemas and schemas_len values
                    schemas = schema->value.group.schemas;
                    schemas_len = schemas->value.group.schemas_len;
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
    registry_schema_item_t *param_meta = _parameter_meta_lookup(path, path_len, hndlr);
    
    if (!param_meta) {
        return -EINVAL;
    }

    /* convert string value to native value */
    int buf_len = REGISTRY_MAX_VAL_LEN;
    void buf[buf_len];
    registry_value_from_str(val_str, param_meta->value.parameter.type, &buf, _get_registry_parameter_data_len(param_meta->value.parameter.type));

    /* call handler to apply the new value to the correct parameter in the instance of the schema */
    schema->hndlr_set(param_meta->id, instance, &buf, buf_len, schema->context);

    return 0;
}

char *registry_get_value(int *path, int path_len, char *buf, int buf_len)
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
    registry_schema_item_t *param_meta = _parameter_meta_lookup(path, path_len, hndlr);
    
    if (!param_meta) {
        return -EINVAL;
    }

    /* call handler to get the parameter value from the instance of the schema */
    int native_buf_len = REGISTRY_MAX_VAL_LEN;
    void native_buf[native_buf_len];
    schema->hndlr_get(param_meta->id, instance, native_buf, native_buf_len, schema->context);

    /* convert native value to string value */
    registry_str_from_value(param_meta->value.parameter.type, &native_buf, buf, buf_len);

    return buf;
}

static int _registry_call_commit(clist_node_t *current, void *res)
{
    assert(current != NULL);
    int _res = *(int *)res;
    registry_schema_t *hndlr = container_of(current, registry_schema_t, node);
    if (hndlr->hndlr_commit_cb) {
        _res = hndlr->hndlr_commit_cb(hndlr->context);
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
        registry_schema_t *hndlr = _schema_lookup(path[0]);

        if (!hndlr) {
            return -EINVAL;
        }

        if (hndlr->hndlr_commit_cb) {
            return hndlr->hndlr_commit_cb(hndlr->context);
        }
        else {
            return 0;
        }
    }
    else {
        clist_foreach(&registry_schemas, _registry_call_commit,
                      (void *)(&rc));
        return rc;
    }
}

int registry_export(int (*export_func)(const int *path, int path_len, registry_parameter_t val, void *context), int *path, int path_len)
{
    (void)export_func;
    (void)path;
    (void)path_len;
    return 0;
    /* assert(export_func != NULL);
    registry_schema_t *hndlr;

    if (path_len > 0) {
        DEBUG("[registry export] exporting %s\n", name);

        hndlr = _schema_lookup(path[0]);
        if (!hndlr) {
            return -EINVAL;
        }

        for (int i = 0; i < hndlr->schemas_len; i++) {
            registry_parameter_t param = hndlr->schemas[i];
            
            // Generate whole registry parameter path from group and parameter name
            char path[strlen(hndlr->name) + strlen(param.name) + 1];
            strcpy(path, hndlr->name);
            strcpy(path + strlen(hndlr->name), "/");
            strcpy(path + strlen(hndlr->name) + 1, param.name);
            export_func(path, path_len, param.data, hndlr->context);
        }
        return 0;
    }
    else {
        DEBUG("[registry export] exporting all\n");
        clist_node_t *node = registry_schemas.next;

        if (!node) {
            return -1;
        }

        do  {
            node = node->next;
            hndlr = container_of(node, registry_schema_t, node);
            
            for (int i = 0; i < hndlr->schemas_len; i++) {
                registry_parameter_t param = hndlr->schemas[i];
                
                // Generate whole registry parameter path from group and parameter name
                char path[strlen(hndlr->name) + strlen(param.name) + 1];
                strcpy(path, hndlr->name);
                strcpy(path + strlen(hndlr->name), "/");
                strcpy(path + strlen(hndlr->name) + 1, param.name);
                export_func(path, path_len, param.data, hndlr->context);
            }
        } while (node != registry_schemas.next);
        return 0;
    } */
}
