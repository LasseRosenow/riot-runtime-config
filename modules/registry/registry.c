#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <clist.h>
#include <kernel_defines.h>
#include <assert.h>
#define ENABLE_DEBUG (0)
#include <debug.h>

#include "registry.h"

static int _registry_cmp_name(clist_node_t *current, void *name);
static int _registry_call_commit(clist_node_t *current, void *name);
static void _parse_name(char *name, int *name_argc, char *name_argv[]);
static registry_handler_t *_handler_parse_and_lookup(char *name, int *name_argc,
                                                     char *name_argv[]);

clist_node_t registry_handlers;

static int _registry_cmp_name(clist_node_t *current, void *name)
{
    assert(current != NULL);
    registry_handler_t *hndlr = container_of(current, registry_handler_t, node);
    return !strcmp(hndlr->name, (char *)name);
}

static registry_handler_t *_handler_lookup(char *name)
{
    clist_node_t *node;
    registry_handler_t *hndlr = NULL;
    node = clist_foreach(&registry_handlers, _registry_cmp_name, name);

    if (node != NULL) {
        hndlr = container_of(node, registry_handler_t, node);
    }

    return hndlr;
}

static void _parse_name(char *name, int *name_argc, char *name_argv[])
{
    int i = 0;
    char *_name_p = &name[0];

    while (_name_p) {
        name_argv[i++] = _name_p;
        while(1) {
            if (*_name_p == '\0') {
                _name_p = NULL;
                break;
            }

            if (*_name_p == REGISTRY_NAME_SEPARATOR) {
                *_name_p = '\0';
                _name_p++;
                break;
            }
            _name_p++;
        }
    }

    *name_argc = i;
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

static registry_handler_t *_handler_parse_and_lookup(char *name, int *name_argc,
                                                     char *name_argv[])
{
    _parse_name(name, name_argc, name_argv);
    return _handler_lookup(name_argv[0]);
}

static size_t _get_registry_parameter_data_len(registry_parameter_data_t *param)
{
    switch (param->type) {
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

int registry_set_value(char *name, char *val_str)
{
    int name_argc;
    char *name_argv[REGISTRY_MAX_DIR_DEPTH];
    registry_handler_t *hndlr;

    char name_buf[REGISTRY_MAX_VAL_LEN]; // To prevent the _parse_name function from corrupting the original name pointer
    strcpy(name_buf, name);

    hndlr = _handler_parse_and_lookup(name_buf, &name_argc, name_argv);

    if (!hndlr) {
        return -EINVAL;
    }

    for (int i = 0; i < hndlr->parameters_len; i++) {
        registry_parameter_t *param = &hndlr->parameters[i];

        /* If name contains this params name */
        int str_len = strlen(name);
        int suffix_len = strlen(param->name);
        if ((str_len >= suffix_len) && (0 == strcmp(name + (str_len-suffix_len), param->name))) {
            registry_value_from_str(val_str, param->data.type, &param->data.value, _get_registry_parameter_data_len(&param->data));
            break;
        }
    }

    if (!hndlr->hndlr_set_cb) {
        hndlr->hndlr_set_cb(name_argc - 1, &name_argv[1], val_str, hndlr->context);
    }

    return 0;
}

char *registry_get_value(const char *name, char *buf, int buf_len)
{
    int name_argc;
    char *name_argv[REGISTRY_MAX_DIR_DEPTH];
    registry_handler_t *hndlr;

    char name_buf[REGISTRY_MAX_VAL_LEN]; // To prevent the _parse_name function from corrupting the original name pointer
    strcpy(name_buf, name);

    hndlr = _handler_parse_and_lookup(name_buf, &name_argc, name_argv);

    if (!hndlr) {
        return NULL;
    }
    
    for (int i = 0; i < hndlr->parameters_len; i++) {
        registry_parameter_t *param = &hndlr->parameters[i];

        /* If name contains this params name */
        int str_len = strlen(name);
        int suffix_len = strlen(param->name);
        if ((str_len >= suffix_len) && (0 == strcmp(name + (str_len-suffix_len), param->name))) {
            registry_str_from_value(param->data.type, &param->data.value, buf, buf_len);
            break;
        }
    }

    if (!hndlr->hndlr_get_cb) {
        hndlr->hndlr_get_cb(name_argc - 1, &name_argv[1], buf, buf_len, hndlr->context);
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

int registry_commit(char *name)
{
    int name_argc;
    int rc = 0;

    if (name) {
        registry_handler_t *hndlr;
        char *name_argv[REGISTRY_MAX_DIR_DEPTH];

        char name_buf[REGISTRY_MAX_VAL_LEN]; // To prevent the _parse_name function from corrupting the original name pointer
        strcpy(name_buf, name);

        hndlr = _handler_parse_and_lookup(name_buf, &name_argc, name_argv);
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

int registry_export(int (*export_func)(const char *name, registry_parameter_data_t val, void *context), char *name)
{
    assert(export_func != NULL);
    int name_argc;
    char *name_argv[REGISTRY_MAX_DIR_DEPTH];
    registry_handler_t *hndlr;

    if (name) {
        DEBUG("[registry export] exporting %s\n", name);

        char name_buf[REGISTRY_MAX_VAL_LEN]; // To prevent the _parse_name function from corrupting the original name pointer
        strcpy(name_buf, name);

        hndlr = _handler_parse_and_lookup(name_buf, &name_argc, name_argv);
        if (!hndlr) {
            return -EINVAL;
        }

        for (int i = 0; i < hndlr->parameters_len; i++) {
            registry_parameter_t param = hndlr->parameters[i];
            
            /* Generate whole registry parameter path from group and parameter name */
            char path[strlen(hndlr->name) + strlen(param.name) + 1];
            strcpy(path, hndlr->name);
            strcpy(path + strlen(hndlr->name), "/");
            strcpy(path + strlen(hndlr->name) + 1, param.name);
            export_func(path, param.data, hndlr->context);
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
            
            for (int i = 0; i < hndlr->parameters_len; i++) {
                registry_parameter_t param = hndlr->parameters[i];
                
                /* Generate whole registry parameter path from group and parameter name */
                char path[strlen(hndlr->name) + strlen(param.name) + 1];
                strcpy(path, hndlr->name);
                strcpy(path + strlen(hndlr->name), "/");
                strcpy(path + strlen(hndlr->name) + 1, param.name);
                export_func(path, param.data, hndlr->context);
            }
        } while (node != registry_handlers.next);
        return 0;
    }
}
