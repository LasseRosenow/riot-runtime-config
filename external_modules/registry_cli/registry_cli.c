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

static registry_root_group_t *_root_group_lookup(registry_root_group_id_t root_group_id)
{
    switch (root_group_id) {
    case REGISTRY_ROOT_GROUP_SYS:
        return &registry_root_group_sys;
    case REGISTRY_ROOT_GROUP_APP:
        return &registry_root_group_app;
    }

    return NULL;
}


static int _parse_string_path(char *string_path, int *buf, int *buf_len)
{
    int buf_index = 0;
    char curr_path_segment[REGISTRY_MAX_DIR_NAME_LEN] = { 0 };
    int curr_path_segment_index = 0;

    int path_len = strlen(string_path);

    for (int i = 0; i <= path_len; i++) {
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

static int _registry_path_from_string_path(char *string_path, int *int_path_buf,
                                           int *int_path_buf_len, registry_path_t *registry_path)
{
    int res = _parse_string_path(string_path, int_path_buf, int_path_buf_len);

    if (res < 0) {
        return res;
    }
    else {
        for (int i = 0; i < *int_path_buf_len; i++) {
            switch (i) {
            case 0: registry_path->root_group_id = (registry_root_group_id_t *)&int_path_buf[i];
                break;
            case 1: registry_path->schema_id = &int_path_buf[i]; break;
            case 2: registry_path->instance_id = &int_path_buf[i]; break;
            case 3: registry_path->path = &int_path_buf[i]; registry_path->path_len++; break;     // Add path.path to correct position in int_path array
            default: registry_path->path_len++; break;
            }
        }
    }

    return 0;
}

static int _export_func(const registry_path_t path, const registry_schema_t *schema,
                        const registry_instance_t *instance, const registry_schema_item_t *meta,
                        const registry_value_t *value, void *context)
{
    (void)value;
    (void)context;

    registry_root_group_t *root_group = _root_group_lookup(*path.root_group_id);

    int path_len = path.path_len;

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
    int int_path_len = REGISTRY_MAX_DIR_DEPTH + 3;
    int int_path[int_path_len];
    registry_path_t path = REGISTRY_PATH();

    if (argc == 1) {
        /* show help for main commands */
        goto help_error;
    }

    if (strcmp(argv[1], "get") == 0) {
        if (_registry_path_from_string_path(argv[2], int_path, &int_path_len, &path) < 0) {
            printf("usage: %s %s <path>\n", argv[0], argv[1]);
            return 1;
        }

        char buf[REGISTRY_MAX_VAL_LEN];
        registry_get_string(path, buf, ARRAY_SIZE(buf));
        printf("%s\n", buf);
        return 0;
    }
    else if (strcmp(argv[1], "set") == 0) {
        if (_registry_path_from_string_path(argv[2], int_path, &int_path_len, &path) < 0) {
            printf("usage: %s %s <path> <value>\n", argv[0], argv[1]);
            return 1;
        }

        registry_set_string(path, argv[3]);
        return 0;
    }
    else if (strcmp(argv[1], "commit") == 0) {
        if (_registry_path_from_string_path(argv[2], int_path, &int_path_len, &path) < 0) {
            printf("usage: %s %s <path>\n", argv[0], argv[1]);
            return 1;
        }

        registry_commit(path);
        return 0;
    }
    else if (strcmp(argv[1], "export") == 0) {
        /* If the path is invalid, it can also just be non existend, so other arguments like -r need to be checked */
        bool invalid_path = false;
        if (_registry_path_from_string_path(argv[2], int_path, &int_path_len, &path) < 0) {
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
    else if (strcmp(argv[1], "store") == 0) {
        /* If the path is invalid, it can also just be non existend, so other arguments like -r need to be checked */
        if (strcmp(argv[2], "load") == 0) {
            registry_store_load();
        }
        else if (strcmp(argv[2], "save") == 0) {
            if (argc > 3) {
                if (_registry_path_from_string_path(argv[3], int_path, &int_path_len, &path) < 0) {
                    printf("usage: %s %s %s [<path>]\n", argv[0], argv[1], argv[2]);
                    return 1;
                }
                else {
                    registry_store_save_one(path, NULL);
                }
            }
            else {
                registry_store_save();
            }
        }
        else {
            printf("usage: %s %s {load|save}\n", argv[0], argv[1]);
            return 1;
        }

        return 0;
    }

help_error:
    printf("usage: %s {get|set|commit|export|store}\n", argv[0]);

    return 1;
}
