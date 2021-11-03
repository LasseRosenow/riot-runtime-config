#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "kernel_defines.h"

#include "registry.h"

#include "application_registry_schema.h"

static void get(int param_id, void *instance, void *buf, int buf_len, void *context);
static void set(int param_id, void *instance, void *val, int val_len, void *context);
static int commit_cb(void *context);

static registry_schema_item_t schemas[] = {
    {
        .id = IS_ENABLED,
        .name = "is_enabled",
        .description = "Example is_enabled description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_BOOL,
        },
    },
    {
        .id = THRESHOLD,
        .name = "threshold",
        .description = "Example threshold description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT32,
        },
    },
    {
        .id = NAME,
        .name = "name",
        .description = "Example name description.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_STRING,
        },
    },
};

registry_schema_t my_schema = {
    .id = 0,
    .name = "my_schema",
    .description = "Example my_schema description.",
    .schemas = schemas,
    .schemas_len = ARRAY_SIZE(schemas),
    .hndlr_get = get,
    .hndlr_set = set,
    .hndlr_commit_cb = commit_cb,
};

static void get(int param_id, void *instance, void *buf, int buf_len, void *context) {
    (void) param_id;
    (void) instance;
    (void) buf;
    (void) buf_len;
    (void) context;
}

static void set(int param_id, void *instance, void *val, int val_len, void *context) {
    (void) param_id;
    (void) instance;
    (void) val;
    (void) val_len;
    (void) context;
}

static int commit_cb(void *context) {
    (void) context;

    return 0;
}
