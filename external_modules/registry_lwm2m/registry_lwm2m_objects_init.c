#include "registry_lwm2m.h"
#include "registry_lwm2m_objects.h"

void registry_lwm2m_objects_init(void)
{
#ifdef CONFIG_REGISTRY_LWM2M_ENABLE_OBJECT_3420
    registry_lwm2m_register_object(&registry_lwm2m_object_3420);
#endif /* CONFIG_REGISTRY_LWM2M_ENABLE_OBJECT_3420 */
}
