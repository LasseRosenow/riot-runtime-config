#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "kernel_defines.h"

#include "registry.h"

#include "application_registry_handler_2.h"

#define MAX_THRESHOLD (500)

static void my_get_cb(int argc, char **argv, const char *val, int val_len_max, void *context);
static void my_set_cb(int argc, char **argv, const char *val, void *context);
static int my_commit_handler(void *context);

/*
This is a conceptual example of a "my_handler" Registry Handler that exposes `threshold` and `is_enabled` parameters.
For education purposes, it will be assumed the application crashes if a `set_threshold` function is called
when `is_enabled==false`.

Note the Registry Handler is not aware of any storage mechanism.
*/

static registry_parameter_t parameters[] = {
    {
        .id = 0,
        .name = "i8",
        .description = "Example i8 description.",
        .data = {
            .value.i8 = 2,
            .type = REGISTRY_TYPE_INT8,
        },
    },
    {
        .id = 1,
        .name = "i16",
        .description = "Example i16 description.",
        .data = {
            .value.i16 = 9,
            .type = REGISTRY_TYPE_INT16,
        },
    },
    {
        .id = 2,
        .name = "i32",
        .description = "Example i32 description.",
        .data = {
            .value.i32 = 42,
            .type = REGISTRY_TYPE_INT32,
        },
    },
    {
        .id = 3,
        .name = "i62",
        .description = "Example i64 description.",
        .data = {
            .value.i64 = 407,
            .type = REGISTRY_TYPE_INT64,
        },
    },
    {
        .id = 4,
        .name = "float",
        .description = "Example string description.",
        .data = {
            .value.f32 = 7.4,
            .type = REGISTRY_TYPE_FLOAT,
        },
    },
    {
        .id = 5,
        .name = "bool",
        .description = "Example bool description.",
        .data = {
            .value.boolean = true,
            .type = REGISTRY_TYPE_BOOL,
        },
    },
    {
        .id = 6,
        .name = "string",
        .description = "Example string description.",
        .data = {
            .value.string = "This is a string",
            .type = REGISTRY_TYPE_STRING,
        },
    },
};

/* Define a registry handler for the current RIOT module.
   To be registered in the RIOT Registry */
registry_handler_t my_handler_2 = {
    .id = 1,
    .name = "my_handler_2",
    .description = "Example my_handler_2 description.",
    .parameters = parameters,
    .parameters_len = ARRAY_SIZE(parameters),
    .hndlr_get_cb = my_get_cb,
    .hndlr_set_cb = my_set_cb,
    .hndlr_commit = my_commit_handler,
};

/* Dummy implementation of `get` handler.
   For both configuration parameters, it copies the value to a `val` variable.
*/
static void my_get_cb(int argc, char **argv, const char *val, int val_len_max, void *context) {
    (void) argc;
    (void) argv;
    (void) val;
    (void) val_len_max;
    (void) context;
}

/* Dummy implementation of `set` handler.
   For both configuration parameters, it sets the value from `val`.
*/
static void my_set_cb(int argc, char **argv, const char *val, void *context) {
    (void) argc;
    (void) argv;
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
