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

static int _parse_string_path(char *path, int *buf, int *buf_len)
{
    int buf_index = 0;
    char curr_path_segment[REGISTRY_MAX_DIR_NAME_LEN] = { 0 };
    int curr_path_segment_index = 0;

    int path_len = strlen(path);

    for (int i = 0; i <= path_len; i++) {
        if (path[i] == REGISTRY_NAME_SEPARATOR || i == path_len) {
            buf[buf_index++] = atoi(curr_path_segment);
            curr_path_segment_index = 0;
        }
        else {
            if (!isdigit(path[i])) {
                return -EINVAL;
            }
            curr_path_segment[curr_path_segment_index++] = path[i];
            curr_path_segment[curr_path_segment_index] = '\0';
        }
    }

    *buf_len = buf_index;
    return 0;
}

static int _export_func(const int *path, int path_len, const registry_schema_t *schema,
                        const registry_instance_t *instance, const registry_schema_item_t *meta,
                        const registry_value_t *value, void *context)
{
    (void)path;
    (void)path_len;
    (void)schema;
    (void)instance;
    (void)meta;
    (void)value;
    (void)context;

    if (meta == NULL) {
        if (instance == NULL) {
            /* Schema */
            printf("%d %s\n", schema->id, schema->name);
        }
        else {
            /* Instance */
            printf("   %d %s\n", 0, instance->name);
        }
    }
    else {
        /* Param or Group */
        printf("      %d %s\n", meta->id, meta->name);
    }

    return 0;
}

int registry_cli_cmd(int argc, char **argv)
{
    if (argc == 1) {
        /* show help for main commands */
        goto help_error;
    }

    bool error = false;

    int path[REGISTRY_MAX_DIR_DEPTH];
    int path_len = 0;

    if (argc > 2) {
        if (_parse_string_path(argv[2], path, &path_len) < 0) {
            error = true;
        }
    }

    if (strcmp(argv[1], "get") == 0) {
        if (error) {
            printf("usage: %s %s <path>\n", argv[0], argv[1]);
            return 1;
        }

        // char buf[REGISTRY_MAX_VAL_LEN];
        // registry_get_string(path, path_len, buf, ARRAY_SIZE(buf)); DOES NOT WORK YET... :( String needs to convert stuff
        // printf("%s\n", buf);
        return 0;
    }
    else if (strcmp(argv[1], "set") == 0) {
        if (error) {
            printf("usage: %s %s <path> <value>\n", argv[0], argv[1]);
            return 1;
        }

        // registry_set_string(path, path_len, argv[3]); DOES NOT WORK YET... :( String needs to convert stuff
        return 0;
    }
    else if (strcmp(argv[1], "commit") == 0) {
        if (error) {
            printf("usage: %s %s <path>\n", argv[0], argv[1]);
            return 1;
        }

        registry_commit(path, path_len);
        return 0;
    }
    else if (strcmp(argv[1], "export") == 0) {
        if (error && strcmp(argv[2], "-r") != 0) {
            printf("usage: %s %s <path> [-r <recursion depth>]\n", argv[0], argv[1]);
            return 1;
        }

        int recursion_level = 0;
        if (error && argc > 3 && strcmp(argv[2], "-r") == 0) {
            recursion_level = atoi(argv[3]);
        }
        else if (argc > 4 && strcmp(argv[3], "-r") == 0) {
            recursion_level = atoi(argv[4]);
        }

        registry_export(_export_func, path, path_len, recursion_level, NULL);
        return 0;
    }

help_error:
    printf("usage: %s {get|set|commit|export}\n", argv[0]);
    return 1;
}
