#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "registry.h"

#include "registry_cli.h"
#include "errno.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static void _print_registry_value(const registry_value_t *value)
{
    switch (value->type) {
    case REGISTRY_TYPE_NONE: break;
    case REGISTRY_TYPE_OPAQUE: {
        printf("opaque (hex): ");
        for (size_t i = 0; i < value->buf_len; i++) {
            printf("%02x", ((uint8_t *)value->buf)[i]);
        }
        break;
    }
    case REGISTRY_TYPE_STRING: printf("string: %s", (char *)value->buf); break;
    case REGISTRY_TYPE_BOOL: printf("bool: %d", *(bool *)value->buf); break;

    case REGISTRY_TYPE_UINT8: printf("uint8: %d", *(uint8_t *)value->buf); break;
    case REGISTRY_TYPE_UINT16: printf("uint16: %d", *(uint16_t *)value->buf); break;
    case REGISTRY_TYPE_UINT32: printf("uint32: %d", *(uint32_t *)value->buf); break;
#if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64)
    case REGISTRY_TYPE_UINT64: printf("uint64: %lld", *(uint64_t *)value->buf); break;
#endif // CONFIG_REGISTRY_USE_UINT64

    case REGISTRY_TYPE_INT8: printf("int8: %d", *(int8_t *)value->buf); break;
    case REGISTRY_TYPE_INT16: printf("int16: %d", *(int16_t *)value->buf); break;
    case REGISTRY_TYPE_INT32: printf("int32: %d", *(int32_t *)value->buf); break;

#if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64)
    case REGISTRY_TYPE_INT64: printf("int64: %lld", *(int64_t *)value->buf); break;
#endif // CONFIG_REGISTRY_USE_INT64

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32)
    case REGISTRY_TYPE_FLOAT32: printf("f32: %f", *(float *)value->buf); break;
#endif // CONFIG_REGISTRY_USE_FLOAT32

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64)
    case REGISTRY_TYPE_FLOAT64: printf("f64: %f", *(double *)value->buf); break;
#endif // CONFIG_REGISTRY_USE_FLOAT32
    }
}

static registry_root_group_t *_root_group_lookup(const registry_root_group_id_t root_group_id)
{
    switch (root_group_id) {
    case REGISTRY_ROOT_GROUP_SYS:
        return &registry_root_group_sys;
    case REGISTRY_ROOT_GROUP_APP:
        return &registry_root_group_app;
    }

    return NULL;
}


static int _parse_string_path(const char *string_path, registry_path_item_t *buf, size_t *buf_len)
{
    size_t buf_index = 0;
    char curr_path_segment[REGISTRY_MAX_DIR_NAME_LEN] = { 0 };
    size_t curr_path_segment_index = 0;

    size_t path_len = strlen(string_path);

    for (size_t i = 0; i <= path_len; i++) {
        if (string_path[i] == REGISTRY_NAME_SEPARATOR || i == path_len) {
            buf[buf_index++] = atoi(curr_path_segment);
            curr_path_segment_index = 0;
        }
        else {
            if (!isdigit(string_path[i])) {
                return -EINVAL;
            }
            curr_path_segment[curr_path_segment_index++] = string_path[i];
            curr_path_segment[curr_path_segment_index] = '\0';
        }
    }

    *buf_len = buf_index;
    return 0;
}

static int _registry_path_from_string_path(const char *string_path, registry_path_item_t *path_items_buf,
                                           size_t *path_items_buf_len, registry_path_t *registry_path)
{
    int res = _parse_string_path(string_path, path_items_buf, path_items_buf_len);

    if (res < 0) {
        return res;
    }
    else {
        for (size_t i = 0; i < *path_items_buf_len; i++) {
            switch (i) {
            case 0: registry_path->root_group_id = (registry_root_group_id_t *)&path_items_buf[i];
                break;
            case 1: registry_path->schema_id = &path_items_buf[i]; break;
            case 2: registry_path->instance_id = &path_items_buf[i]; break;
            case 3: registry_path->path = &path_items_buf[i]; registry_path->path_len++; break;     // Add path.path to correct position in int_path array
            default: registry_path->path_len++; break;
            }
        }
    }

    return 0;
}

static int _export_func(const registry_path_t path, const registry_schema_t *schema,
                        const registry_instance_t *instance, const registry_schema_item_t *meta,
                        const registry_value_t *value, const void *context)
{
    (void)value;
    (void)context;

    registry_root_group_t *root_group = _root_group_lookup(*path.root_group_id);

    size_t path_len = path.path_len;

    if (path.root_group_id != NULL) {
        path_len++;
    }

    if (schema != NULL) {
        path_len++;
    }

    if (instance != NULL) {
        path_len++;
    }

    printf("%*c\b", ((path_len - 1) * 2) + 1, ' ');

    if (meta == NULL) {
        if (instance == NULL) {
            if (schema == NULL) {
                /* Root Group */
                printf("%d %s\n", *path.root_group_id, root_group->name);
            }
            else {
                /* Schema */
                printf("%d %s\n", *path.schema_id, schema->name);
            }
        }
        else {
            /* Instance */
            printf("%d %s\n", *path.instance_id, instance->name);
        }
    }
    else {
        /* Param or Group */
        printf("%d %s\n", meta->id, meta->name);
    }

    return 0;
}

int registry_cli_cmd(int argc, char **argv)
{
    size_t path_items_buf_len = REGISTRY_MAX_DIR_DEPTH + 3;
    registry_path_item_t path_items_buf[path_items_buf_len];
    // TODO: Why is REGISTRY_PATH() Not working? (It should resolve to _REGISTRY_PATH_0()
    // but somehow its not initializing root group with NULL?? (makes no sense:( ... )))
    registry_path_t path = _REGISTRY_PATH_0();

    if (argc == 1) {
        /* show help for main commands */
        goto help_error;
    }

    if (strcmp(argv[1], "get") == 0) {
        if (_registry_path_from_string_path(argv[2], path_items_buf, &path_items_buf_len, &path) < 0) {
            printf("usage: %s %s <path>\n", argv[0], argv[1]);
            return 1;
        }

        registry_value_t value;
        int res = registry_get_value(path, &value);

        if (res != 0) {
            printf("error: %d\n", res);
            return 1;
        }

        _print_registry_value(&value);
        printf("\n");

        return 0;
    }
    else if (strcmp(argv[1], "set") == 0) {
        if (_registry_path_from_string_path(argv[2], path_items_buf, &path_items_buf_len, &path) < 0) {
            printf("usage: %s %s <path> <value>\n", argv[0], argv[1]);
            return 1;
        }

        registry_set_string(path, argv[3]);
        return 0;
    }
    else if (strcmp(argv[1], "commit") == 0) {
        if (_registry_path_from_string_path(argv[2], path_items_buf, &path_items_buf_len, &path) < 0) {
            printf("usage: %s %s <path>\n", argv[0], argv[1]);
            return 1;
        }

        registry_commit(path);
        return 0;
    }
    else if (strcmp(argv[1], "export") == 0) {
        /* If the path is invalid, it can also just be non existent, so other arguments like -r need to be checked */
        bool invalid_path = false;
        if (_registry_path_from_string_path(argv[2], path_items_buf, &path_items_buf_len, &path) < 0) {
            invalid_path = true;
        }
        if (invalid_path && strcmp(argv[2], "-r") != 0) {
            printf("usage: %s %s <path> [-r <recursion depth>]\n", argv[0], argv[1]);
            return 1;
        }

        /* The argv index of -r varies depending on if a path was specified or not */
        int recursion_level = 0;
        if (invalid_path && argc > 3 && strcmp(argv[2], "-r") == 0) {
            recursion_level = atoi(argv[3]);
        }
        else if (argc > 4 && strcmp(argv[3], "-r") == 0) {
            recursion_level = atoi(argv[4]);
        }

        registry_export(_export_func, path, recursion_level, NULL);
        return 0;
    }
    else if (strcmp(argv[1], "load") == 0) {
        if (argc > 2) {
            if (_registry_path_from_string_path(argv[2], path_items_buf, &path_items_buf_len, &path) < 0) {
                printf("usage: %s %s [path]\n", argv[0], argv[1]);
                return 1;
            }
            else {
                registry_load(path);
            }
        }
        else {
            registry_load(_REGISTRY_PATH_0());
        }

        return 0;
    }
    else if (strcmp(argv[1], "save") == 0) {
        if (argc > 2) {
            if (_registry_path_from_string_path(argv[2], path_items_buf, &path_items_buf_len, &path) < 0) {
                printf("usage: %s %s [path]\n", argv[0], argv[1]);
                return 1;
            }
            else {
                registry_save(path);
            }
        }
        else {
            registry_save(_REGISTRY_PATH_0());
        }

        return 0;
    }

help_error:
    printf("usage: %s {get|set|commit|export|load|save}\n", argv[0]);

    return 1;
}
