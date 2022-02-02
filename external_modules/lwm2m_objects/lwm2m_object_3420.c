#include <string.h>
#include <stdio.h>

#include "kernel_defines.h"
#include "registry_schemas.h"

#include "lwm2m_objects.h"

#define OBJECT_ID 3420
enum object_properties {
    OBJECT_PROPERTIES_RGB_VALUE = 0, /**< Text string according to the RBG hexadecimal format with # (e.g. #FF0000 for 100% red). */
};

static int _get_rgb_value(int instance, void *buf, int buf_len)
{
    (void)buf_len;

    int path_red[] = { REGISTRY_SCHEMA_RGB_LED, instance, REGISTRY_SCHEMA_RGB_LED_RED };
    uint8_t red = registry_get_uint8(path_red, ARRAY_SIZE(path_red));

    int path_green[] = { REGISTRY_SCHEMA_RGB_LED, instance, REGISTRY_SCHEMA_RGB_LED_GREEN };
    uint8_t green = registry_get_uint8(path_green, ARRAY_SIZE(path_green));

    int path_blue[] = { REGISTRY_SCHEMA_RGB_LED, instance, REGISTRY_SCHEMA_RGB_LED_BLUE };
    uint8_t blue = registry_get_uint8(path_blue, ARRAY_SIZE(path_blue));

    sprintf(buf, "%02X%02X%02X", red, green, blue);

    return 0;
}

static int _set_rgb_value(int instance, void *val, int val_len)
{
    (void)val_len;

    /* Convert Hex-String to rgb integers */
    uint8_t r, g, b;

    sscanf(val, "%02hhX%02hhX%02hhX", &r, &g, &b);


    /* Set rgb values in registry */
    int path_red[] = { REGISTRY_SCHEMA_RGB_LED, instance, REGISTRY_SCHEMA_RGB_LED_RED };

    registry_set_uint8(path_red, ARRAY_SIZE(path_red), r);


    int path_green[] = { REGISTRY_SCHEMA_RGB_LED, instance, REGISTRY_SCHEMA_RGB_LED_GREEN };

    registry_set_uint8(path_green, ARRAY_SIZE(path_green), g);


    int path_blue[] = { REGISTRY_SCHEMA_RGB_LED, instance, REGISTRY_SCHEMA_RGB_LED_BLUE };

    registry_set_uint8(path_blue, ARRAY_SIZE(path_blue), b);


    /* Apply changes */
    int path_commit[] = { REGISTRY_SCHEMA_RGB_LED, instance };

    registry_commit(path_commit, ARRAY_SIZE(path_commit));


    return 0;
}

static lwm2m_objects_property_t properties[] = {
    {
        .id = OBJECT_PROPERTIES_RGB_VALUE,
        .type = LWM2M_TYPE_STRING,
        .get = _get_rgb_value,
        .set = _set_rgb_value,
    }
};

lwm2m_objects_object_t registry_lwm2m_object_3420 = {
    .id = OBJECT_ID,
    .properties = properties,
    .properties_len = ARRAY_SIZE(properties),
};
