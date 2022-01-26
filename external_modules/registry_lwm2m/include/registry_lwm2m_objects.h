#ifndef REGISTRY_LWM2M_OBJECTS_H
#define REGISTRY_LWM2M_OBJECTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "registry_lwm2m.h"

#ifdef CONFIG_REGISTRY_LWM2M_ENABLE_OBJECT_3420
    extern registry_lwm2m_object_t registry_lwm2m_object_3420;
#endif /* CONFIG_REGISTRY_LWM2M_ENABLE_OBJECT_3420 */

#ifdef __cplusplus
}
#endif

#endif /* REGISTRY_LWM2M_OBJECTS_H */