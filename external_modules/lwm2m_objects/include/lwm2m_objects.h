#ifndef LWM2M_OBJECTS_H
#define LWM2M_OBJECTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liblwm2m.h"

void lwm2m_objects_init(void);


/* LwM2M Objects */
#ifdef CONFIG_LWM2M_OBJECTS_ENABLE_OBJECT_3420
extern lwm2m_object_t lwm2m_object_3420;
#endif /* CONFIG_LWM2M_OBJECTS_ENABLE_OBJECT_3420 */

#ifdef __cplusplus
}
#endif

#endif /* LWM2M_OBJECTS_H */
