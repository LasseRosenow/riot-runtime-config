#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "kernel_defines.h"
#include "registry.h"
#include "registry_schemas.h"

static void mapping(int param_id, registry_instance_t *instance, void **val, size_t *val_len);

REGISTRY_SCHEMA(
    registry_schema_rgb_led,
    REGISTRY_SCHEMA_RGB_LED,
    "rgb", "Representation of an rgb color.",
    mapping,

    REGISTRY_PARAMETER_UINT8(
        REGISTRY_SCHEMA_RGB_LED_RED,
        "red", "Intensity of the red color of the rgb lamp.")

    REGISTRY_PARAMETER_UINT8(
        REGISTRY_SCHEMA_RGB_LED_GREEN,
        "green", "Intensity of the green color of the rgb lamp.")

    REGISTRY_PARAMETER_UINT8(
        REGISTRY_SCHEMA_RGB_LED_BLUE,
        "blue", "Intensity of the blue color of the rgb lamp.")

    );

static void mapping(int param_id, registry_instance_t *instance, void **val, size_t *val_len)
{
    registry_schema_rgb_led_t *_instance = (registry_schema_rgb_led_t *)instance->data;

    switch (param_id) {
    case REGISTRY_SCHEMA_RGB_LED_RED:
        *val = &_instance->red;
        *val_len = sizeof(_instance->red);
        break;

    case REGISTRY_SCHEMA_RGB_LED_GREEN:
        *val = &_instance->green;
        *val_len = sizeof(_instance->green);
        break;

    case REGISTRY_SCHEMA_RGB_LED_BLUE:
        *val = &_instance->blue;
        *val_len = sizeof(_instance->blue);
        break;
    }
}
