#ifdef CONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "kernel_defines.h"
#include "registry.h"
#include "registry_schemas.h"

static void get(int param_id, registry_instance_t *instance, void *buf, int buf_len, void *context);
static void set(int param_id, registry_instance_t *instance, const void *val, int val_len,
                void *context);

static registry_schema_item_t schema_items[] = {
    {
        .id = REGISTRY_SCHEMA_RGB_LED_RED,
        .name = "red",
        .description = "Intensity of the red color of the rgb lamp.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_UINT8,
        },
    },
    {
        .id = REGISTRY_SCHEMA_RGB_LED_GREEN,
        .name = "green",
        .description = "Intensity of the green color of the rgb lamp.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_UINT8,
        },
    },
    {
        .id = REGISTRY_SCHEMA_RGB_LED_BLUE,
        .name = "blue",
        .description = "Intensity of the blue color of the rgb lamp.",
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER,
        .value.parameter = {
            .type = REGISTRY_TYPE_UINT8,
        },
    },
};

registry_schema_t registry_schema_rgb_led = {
    .id = REGISTRY_SCHEMA_RGB_LED,
    .name = "rgb",
    .description = "Representation of a rgb color.",
    .items = schema_items,
    .items_len = ARRAY_SIZE(schema_items),
    .get = get,
    .set = set,
};

static void get(int param_id, registry_instance_t *instance, void *buf, int buf_len, void *context)
{
    (void)buf_len;
    (void)context;

    registry_schema_rgb_led_t *_instance = (registry_schema_rgb_led_t *)instance->data;

    switch (param_id) {
    case REGISTRY_SCHEMA_RGB_LED_RED:
        memcpy(buf, &_instance->red, sizeof(_instance->red));
        break;

    case REGISTRY_SCHEMA_RGB_LED_GREEN:
        memcpy(buf, &_instance->green, sizeof(_instance->green));
        break;

    case REGISTRY_SCHEMA_RGB_LED_BLUE:
        memcpy(buf, &_instance->blue, sizeof(_instance->blue));
        break;
    }
}

static void set(int param_id, registry_instance_t *instance, const void *val, int val_len,
                void *context)
{
    (void)val_len;
    (void)context;

    registry_schema_rgb_led_t *_instance = (registry_schema_rgb_led_t *)instance->data;

    switch (param_id) {
    case REGISTRY_SCHEMA_RGB_LED_RED:
        memcpy(&_instance->red, val, sizeof(_instance->red));
        break;

    case REGISTRY_SCHEMA_RGB_LED_GREEN:
        memcpy(&_instance->green, val, sizeof(_instance->green));
        break;

    case REGISTRY_SCHEMA_RGB_LED_BLUE:
        memcpy(&_instance->blue, val, sizeof(_instance->blue));
        break;
    }
}

#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED */
