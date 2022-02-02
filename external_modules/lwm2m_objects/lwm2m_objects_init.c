#include "lwm2m_objects.h"

void lwm2m_objects_init(void)
{
#ifdef CONFIG_LWM2M_OBJECTS_ENABLE_OBJECT_3420
    lwm2m_objects_register_object(&lwm2m_object_3420);
#endif /* CONFIG_LWM2M_OBJECTS_ENABLE_OBJECT_3420 */
}
