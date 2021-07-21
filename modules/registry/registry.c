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
static registry_parameter_t *_get_registry_parameter(int *path, int path_len, registry_handler_t *hndlr);

clist_node_t registry_handlers;

static int _registry_cmp_id(clist_node_t *current, void *id)
{
    assert(current != NULL);
    registry_handler_t *hndlr = container_of(current, registry_handler_t, node);
    return !(hndlr->id - *(int*)id);
}

static registry_handler_t *_handler_lookup(int id)
{
    clist_node_t *node;
    registry_handler_t *hndlr = NULL;
    node = clist_foreach(&registry_handlers, _registry_cmp_id, &id);

    if (node != NULL) {
        hndlr = container_of(node, registry_handler_t, node);
    }

    return hndlr;
}

void registry_init(void)
{
    registry_handlers.next = NULL;
    registry_store_init();
}

void registry_register(registry_handler_t *handler)
{
    assert(handler != NULL);
    clist_rpush(&registry_handlers, &(handler->node));
}

static size_t _get_registry_parameter_data_len(registry_type_t type)
{
    switch (type) {
        case REGISTRY_TYPE_INT8: return sizeof(int8_t);
        case REGISTRY_TYPE_INT16: return sizeof(int16_t);
        case REGISTRY_TYPE_INT32: return sizeof(int32_t);
        case REGISTRY_TYPE_STRING: return REGISTRY_MAX_VAL_LEN;
        case REGISTRY_TYPE_BOOL: return sizeof(bool);
#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
        case REGISTRY_TYPE_INT64: return sizeof(int64_t);
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
        case REGISTRY_TYPE_FLOAT: return sizeof(float);
#endif /* CONFIG_REGISTRY_USE_FLOAT */
        
        default: return 0;
    }
}

static registry_parameter_t *_get_registry_parameter(int *path, int path_len, registry_handler_t *hndlr) {
    registry_schema_t *schema;
    registry_schema_t *schemas = hndlr->schemas;
    int schemas_len = hndlr->schemas_len;

    for (int path_index = 0; path_index < path_len; path_index++) {
        for (int i = 0; i < schemas_len; i++) {
            schema = &hndlr->schemas[i];

            if (schema->id == path[path_index]) {
                if (schema->type == REGISTRY_SCHEMA_TYPE_PARAMETER && path_index == path_len -1) {
                    // If this is the last path segment and it is a parameter => return the parameter
                    return &schema->value.parameter;
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

int registry_set_value(int *path, int path_len, char *val_str)
{
    registry_handler_t *hndlr = _handler_lookup(path[0]);

    if (!hndlr) {
        return -EINVAL;
    }

    registry_parameter_t *param = _get_registry_parameter(path, path_len, hndlr);
    
    if (!param) {
        return -EINVAL;
    }

    registry_value_from_str(val_str, param->type, &param->value, _get_registry_parameter_data_len(param->type));

    if (!hndlr->hndlr_set_cb) {
        hndlr->hndlr_set_cb(path, path_len, val_str, hndlr->context);
    }

    return 0;
}

char *registry_get_value(int *path, int path_len, char *buf, int buf_len)
{
    registry_handler_t *hndlr = _handler_lookup(path[0]);

    if (!hndlr) {
        return NULL;
    }

    registry_parameter_t *param = _get_registry_parameter(path, path_len, hndlr);
    
    if (!param) {
        return NULL;
    }
    
    registry_str_from_value(param->type, &param->value, buf, buf_len);

    if (!hndlr->hndlr_get_cb) {
        hndlr->hndlr_get_cb(path, path_len, buf, buf_len, hndlr->context);
    }

    return buf;
}

static int _registry_call_commit(clist_node_t *current, void *res)
{
    assert(current != NULL);
    int _res = *(int *)res;
    registry_handler_t *hndlr = container_of(current, registry_handler_t, node);
    if (hndlr->hndlr_commit) {
        _res = hndlr->hndlr_commit(hndlr->context);
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
        registry_handler_t *hndlr = _handler_lookup(path[0]);

        if (!hndlr) {
            return -EINVAL;
        }

        if (hndlr->hndlr_commit) {
            return hndlr->hndlr_commit(hndlr->context);
        }
        else {
            return 0;
        }
    }
    else {
        clist_foreach(&registry_handlers, _registry_call_commit,
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
    registry_handler_t *hndlr;

    if (path_len > 0) {
        DEBUG("[registry export] exporting %s\n", name);

        hndlr = _handler_lookup(path[0]);
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
        clist_node_t *node = registry_handlers.next;

        if (!node) {
            return -1;
        }

        do  {
            node = node->next;
            hndlr = container_of(node, registry_handler_t, node);
            
            for (int i = 0; i < hndlr->schemas_len; i++) {
                registry_parameter_t param = hndlr->schemas[i];
                
                // Generate whole registry parameter path from group and parameter name
                char path[strlen(hndlr->name) + strlen(param.name) + 1];
                strcpy(path, hndlr->name);
                strcpy(path + strlen(hndlr->name), "/");
                strcpy(path + strlen(hndlr->name) + 1, param.name);
                export_func(path, path_len, param.data, hndlr->context);
            }
        } while (node != registry_handlers.next);
        return 0;
    } */
}
