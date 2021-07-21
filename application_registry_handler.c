#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "kernel_defines.h"

#include "registry.h"

#include "application_registry_handler.h"

#define MAX_THRESHOLD (500)

static void my_get_cb(int* path, int path_len, const char *val, int val_len_max, void *context);
static void my_set_cb(int* path, int path_len, const char *val, void *context);
static int my_commit_handler(void *context);

/*
This is a conceptual example of a "my_handler" Registry Handler that exposes `threshold` and `is_enabled` parameters.
For education purposes, it will be assumed the application crashes if a `set_threshold` function is called
when `is_enabled==false`.

Note the Registry Handler is not aware of any storage mechanism.
*/

static registry_schema_t schemas[] = {
    {
        .id = 0,
        .name = "is_enabled",
        .description = "Example is_enabled description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_BOOL,
            .value.boolean = false,
        },
    },
    {
        .id = 1,
        .name = "threshold",
        .description = "Example threshold description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT32,
            .value.i32 = 7,
        },
    },
    {
        .id = 2,
        .name = "name",
        .description = "Example name description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_STRING,
            .value.string = "Lasse",
        },
    },
};

/* Define a registry handler for the current RIOT module.
   To be registered in the RIOT Registry */
registry_handler_t my_handler = {
    .id = 0,
    .name = "my_handler",
    .description = "Example my_handler description.",
    .schemas = schemas,
    .schemas_len = ARRAY_SIZE(schemas),
    .hndlr_get_cb = my_get_cb,
    .hndlr_set_cb = my_set_cb,
    .hndlr_commit = my_commit_handler,
};

/* Dummy implementation of `get` handler.
   For both configuration parameters, it copies the value to a `val` variable.
*/
static void my_get_cb(int* path, int path_len, const char *val, int val_len_max, void *context) {
    (void) path;
    (void) path_len;
    (void) val;
    (void) val_len_max;
    (void) context;
}

/* Dummy implementation of `set` handler.
   For both configuration parameters, it sets the value from `val`.
*/
static void my_set_cb(int* path, int path_len, const char *val, void *context) {
    (void) path;
    (void) path_len;
    (void) val;
    (void) context;
}

/* Dummy implementation of `commit` handler.
   This is intended to be called by the Registry when all configurations have
   to be applied. Because of this, it's possible to implement transactions or
   protect against faulty combinations of configs, race conditions, etc.
*/
static int my_commit_handler(void *context) {
    (void) context;

    return 0;
}
