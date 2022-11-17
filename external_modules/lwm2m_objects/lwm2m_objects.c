#include "kernel_defines.h"
#include "lwm2m_client.h"
#include "lwm2m_client_objects.h"
#include "lwm2m_platform.h"
#include "float.h"
#include "assert.h"

#include "lwm2m_objects.h"
#include "registry.h"

/* counts how many objects are stored in obj_list_counter, which is necessary no know its size for reallocation */
int obj_list_counter = 3;

uint8_t connected = 0;
lwm2m_object_t **obj_list;
lwm2m_client_data_t client_data;

void _register_lwm2m_object(lwm2m_object_t *object)
{
    obj_list_counter++;
    obj_list = realloc(obj_list, obj_list_counter * sizeof(lwm2m_object_t *));
    obj_list[obj_list_counter - 1] = object;
}

void lwm2m_objects_init(void)
{
    /* this call is needed before creating any objects */
    lwm2m_client_init(&client_data);

    /* remove in the future? */
    /* add objects that will be registered */
    obj_list = malloc(obj_list_counter * sizeof(lwm2m_object_t *));
    obj_list[0] = lwm2m_client_get_security_object(&client_data);
    obj_list[1] = lwm2m_client_get_server_object(&client_data);
    obj_list[2] = lwm2m_client_get_device_object(&client_data);

    if (!obj_list[0] || !obj_list[1] || !obj_list[2]) {
        puts("Could not create mandatory objects");
    }

    /* LwM2M objects */
    if (IS_ACTIVE(CONFIG_LWM2M_OBJECTS_ENABLE_OBJECT_3420)) {
        _register_lwm2m_object(&lwm2m_object_3420);
    }

    /* start the lwm2m client */
    lwm2m_client_run(&client_data, obj_list, obj_list_counter);
}
