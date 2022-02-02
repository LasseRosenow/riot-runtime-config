#ifndef LWM2M_OBJECTS_H
#define LWM2M_OBJECTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liblwm2m.h"
#include "clist.h"

// void lwm2m_objects_cli_init(void);
// int lwm2m_objects_cli_cmd(int argc, char **argv);

void lwm2m_objects_init(void);

typedef struct {
    int id;
    lwm2m_data_type_t type;
    int (*get)(int instance, void *buf, int buf_len);
    int (*set)(int instance, void *val, int val_len);
} lwm2m_objects_property_t;

typedef struct {
    clist_node_t node;              /**< Linked list node */
    int id;
    lwm2m_objects_property_t *properties;
    int properties_len;
} lwm2m_objects_object_t;

void lwm2m_objects_register_object(lwm2m_objects_object_t *object);

/**
 * @brief List of registered objects
 */
extern clist_node_t lwm2m_objects;


/* LwM2M Objects */
#ifdef CONFIG_LWM2M_OBJECTS_ENABLE_OBJECT_3420
extern lwm2m_objects_object_t lwm2m_object_3420;
#endif /* CONFIG_LWM2M_OBJECTS_ENABLE_OBJECT_3420 */

#ifdef __cplusplus
}
#endif

#endif /* LWM2M_OBJECTS_H */
