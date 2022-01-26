#include <string.h>
#include <stdio.h>

#include "kernel_defines.h"
#include "registry_schemas.h"

#include "registry_lwm2m.h"

static int _get_1(int instance, void *buf, int buf_len)
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

registry_lwm2m_object_property properties[] = {
    {
        .id = 1,
        .type = LWM2M_TYPE_STRING,
        .get_value = _get_1,
    }
};

registry_lwm2m_object registry_lwm2m_objects[] = {
    {
        .id = 3420,
        .properties = properties,
        .properties_len = ARRAY_SIZE(properties),
    }
};
