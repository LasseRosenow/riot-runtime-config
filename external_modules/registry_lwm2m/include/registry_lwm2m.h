#ifndef REGISTRY_LWM2M_H
#define REGISTRY_LWM2M_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liblwm2m.h"

extern void registry_lwm2m_cli_init(void);
extern int registry_lwm2m_cli_cmd(int argc, char **argv);

typedef struct {
    int id;
    lwm2m_data_type_t type;
    int (*get_value)(int instance, void *buf, int buf_len);
} registry_lwm2m_object_property;

typedef struct {
    int id;
    registry_lwm2m_object_property *properties;
    int properties_len;
} registry_lwm2m_object;


#ifdef __cplusplus
}
#endif

#endif /* REGISTRY_LWM2M_H */
