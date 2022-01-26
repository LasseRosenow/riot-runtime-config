#ifndef REGISTRY_LWM2M_H
#define REGISTRY_LWM2M_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liblwm2m.h"
#include "clist.h"

// void registry_lwm2m_cli_init(void);
// int registry_lwm2m_cli_cmd(int argc, char **argv);

void registry_lwm2m_objects_init(void);

typedef struct {
    int id;
    lwm2m_data_type_t type;
    int (*get_value)(int instance, void *buf, int buf_len);
} registry_lwm2m_object_property_t;

typedef struct {
    clist_node_t node;              /**< Linked list node */
    int id;
    registry_lwm2m_object_property_t *properties;
    int properties_len;
} registry_lwm2m_object_t;

void registry_lwm2m_register_object(registry_lwm2m_object_t *object);

/**
 * @brief List of registered objects
 */
extern clist_node_t registry_lwm2m_objects;

#ifdef __cplusplus
}
#endif

#endif /* REGISTRY_LWM2M_H */
