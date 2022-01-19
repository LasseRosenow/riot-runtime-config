#include "registry.h"
#include "registry_schemas.h"

void registry_schema_init(void)
{
#ifdef CONFIG_REGISTRY_ENABLE_SCHEMA_TYPES_TEST
    registry_register_schema(&registry_schema_types_test);
#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_TYPES_TEST */

#ifdef CONFIG_REGISTRY_ENABLE_SCHEMA_RGB
    registry_register_schema(&registry_schema_rgb);
#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_RGB */
}
