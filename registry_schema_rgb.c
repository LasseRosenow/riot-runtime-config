#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "kernel_defines.h"

#include "registry.h"

#include "registry_schema_rgb.h"

static void get(int param_id, void *instance, void *buf, int buf_len, void *context);
static void set(int param_id, void *instance, void *val, int val_len, void *context);
static int commit_cb(void *context);

static registry_schema_item_t schema[] = {
    {
        .id = REGISTRY_SCHEMA_RGB_RED,
        .name = "red",
        .description = "Intensity of the red color of the rgb lamp.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT16,
        },
    },
    {
        .id = REGISTRY_SCHEMA_RGB_GREEN,
        .name = "green",
        .description = "Intensity of the green color of the rgb lamp.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT16,
        },
    },
    {
        .id = REGISTRY_SCHEMA_RGB_BLUE,
        .name = "blue",
        .description = "Intensity of the blue color of the rgb lamp.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_INT16,
        },
    },
};

registry_schema_t registry_schema_rgb = {
    .id = 0,
    .name = "rgb",
    .description = "Representation of a rgb color.",
    .schema = schema,
    .schema_len = ARRAY_SIZE(schema),
    .get = get,
    .set = set,
    .commit_cb = commit_cb,
};

static void get(int param_id, void *instance, void *buf, int buf_len, void *context) {
    (void) buf_len;
    (void) context;

    registry_schema_rgb_t* _instance = (registry_schema_rgb_t*) instance;

    switch (param_id) {
        case REGISTRY_SCHEMA_RGB_RED:
            memcpy(buf, &_instance->r, sizeof(_instance->r));
            break;
        
        case REGISTRY_SCHEMA_RGB_GREEN:
            memcpy(buf, &_instance->g, sizeof(_instance->g));
            break;

        case REGISTRY_SCHEMA_RGB_BLUE:
            memcpy(buf, &_instance->b, sizeof(_instance->b));
            break;
    }
}

static void set(int param_id, void *instance, void *val, int val_len, void *context) {
    (void) val_len;
    (void) context;

    registry_schema_rgb_t* _instance = (registry_schema_rgb_t*) instance;

    switch (param_id) {
        case REGISTRY_SCHEMA_RGB_RED:
            memcpy(&_instance->r, val , sizeof(_instance->r));
            break;
        
        case REGISTRY_SCHEMA_RGB_GREEN:
            memcpy(&_instance->g, val, sizeof(_instance->g));
            break;

        case REGISTRY_SCHEMA_RGB_BLUE:
            memcpy(&_instance->b, val, sizeof(_instance->b));
            break;
    }
}

static int commit_cb(void *context) {
    (void) context;

    return 0;
}
