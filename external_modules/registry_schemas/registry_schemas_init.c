#include "registry.h"
#include "registry_schemas.h"

void registry_schemas_init(void)
{
#ifdef CONFIG_REGISTRY_ENABLE_SCHEMA_TYPES_TEST
    registry_register_schema(REGISTRY_ROOT_GROUP_SYS, &registry_schema_types_test);
#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_TYPES_TEST */

#ifdef CONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED
    registry_register_schema(REGISTRY_ROOT_GROUP_SYS, &registry_schema_rgb_led);
#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED */
}
