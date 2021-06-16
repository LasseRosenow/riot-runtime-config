#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kernel_defines.h"
#include "liblwm2m.h"
#include "object.h"
#include "lwm2m_client_config.h"
#include "registry.h"

/* Set to true if reboot requested. */
static bool reboot;

/* Descriptor of a LwM2M registry object instance */
typedef struct {
    registry_handler_t *hndlr;
    char **res_list;
    int res_list_size;
} reg_data_t;

/* static void _get_reg_data_from_registry_handlers(uint16_t* reg_data) {
    clist_node_t *node = registry_handlers.next;
    int size = 0;

    do  {
        node = node->next;
        registry_handler_t *hndlr = container_of(node, registry_handler_t, node);
        size++;

        reg_data = realloc(reg_data, size * sizeof(uint16_t));
    } while (node != registry_handlers.next);
} */

static int _registry_export(const char *name, char *val, void *context) {
    (void)val;
    reg_data_t *userData = (reg_data_t *)context;

    /* Free space for the string name of the exported property */
    char* new_name = malloc(strlen(name)+1);
    strcpy(new_name, name);

    /* Increase the size of the list of property names to fit in the new exported property name */
    userData->res_list = realloc(userData->res_list, (userData->res_list_size + 1) * sizeof(char*));
    userData->res_list[userData->res_list_size] = new_name;

    /* Increase the size counter of the list */
    userData->res_list_size += 1;

    return 0;
}

static uint8_t prv_registry_discover(uint16_t instance_id, int *num_dataP,
                                   lwm2m_data_t **data_arrayP,
                                   lwm2m_object_t *objectP)
{
    uint8_t result;
    reg_data_t *userData = (reg_data_t *)objectP->userData;


    if (instance_id != 0) {
        return COAP_404_NOT_FOUND;
    }

    result = COAP_205_CONTENT;

    if (*num_dataP == 0) {
        /* This list must contain all available resources */
        int len = userData->res_list_size;

        *data_arrayP = lwm2m_data_new(len);
        if (*data_arrayP == NULL) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }

        *num_dataP = len;
        for (int i = 0; i < len; i++) {
            (*data_arrayP)[i].id = i;
        }
    }
    else {
        /* Check if each given resource is present */
        if (userData->res_list_size > *num_dataP) {
            result = COAP_404_NOT_FOUND;
        }
    }

    return result;
}

static uint8_t prv_registry_read(uint16_t instance_id, int *num_dataP,
                               lwm2m_data_t **data_arrayP,
                               lwm2m_object_t *objectP)
{
    uint8_t result = COAP_404_NOT_FOUND;
    reg_data_t *userData = (reg_data_t *)objectP->userData;

    /* Single instance object */
    if (instance_id != 0) {
        return COAP_404_NOT_FOUND;
    }

    /* Full object requested */
    if (*num_dataP == 0) {
        /* This list must contain all available resources */
        int len = userData->res_list_size;

        *data_arrayP = lwm2m_data_new(len);
        if (*data_arrayP == NULL) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }

        *num_dataP = userData->res_list_size;
        for (int i = 0; i < len; i++) {
            (*data_arrayP)[i].id = i;
        }
    }

    /* Get values and send them to lwm2m */
    for (int i = 0; i < *num_dataP; i++) {
        int index = (*data_arrayP)[i].id;
        if (index < userData->res_list_size) {
            char buf[REGISTRY_MAX_VAL_LEN];
            char* value = registry_get_value(userData->res_list[index], buf, REGISTRY_MAX_VAL_LEN);
            lwm2m_data_encode_string(value, *data_arrayP + i);
            result = COAP_205_CONTENT;
        } else {
            result = COAP_404_NOT_FOUND;
        }
    }

    return result;
}

static uint8_t prv_registry_write(uint16_t instance_id, int num_data,
                                lwm2m_data_t *data_array,
                                lwm2m_object_t *objectP)
{
    uint8_t result = COAP_404_NOT_FOUND;
    reg_data_t *userData = (reg_data_t *)objectP->userData;

    (void)instance_id;
    (void)data_array;

    if (data_array[0].id < userData->res_list_size) {
        for (int i = 0; i < num_data; i++) {
            /* No multiple instance resources */
            if (data_array[i].type == LWM2M_TYPE_MULTIPLE_RESOURCE) {
                result = COAP_404_NOT_FOUND;
                continue;
            }

            /* The registry only supports strings */
            if (data_array[i].type == LWM2M_TYPE_STRING || data_array[i].type == LWM2M_TYPE_OPAQUE) {
                int index = data_array[i].id;
                char buf[REGISTRY_MAX_VAL_LEN] = {0};
                strncpy(buf, (char*)data_array[i].value.asBuffer.buffer, data_array[i].value.asBuffer.length);
                registry_set_value(userData->res_list[index], buf);
                result = COAP_204_CHANGED;
            } else {
                result = COAP_400_BAD_REQUEST;
            }
        }
    }

    return result;
}

static uint8_t prv_registry_execute(uint16_t instance_id, uint16_t resource_id,
                                  uint8_t *buffer, int length,
                                  lwm2m_object_t *objectP)
{
    uint8_t result;
    reg_data_t *data = (reg_data_t *)objectP->userData;

    (void)data;

    (void)buffer;
    (void)length;
    (void)objectP;

    /* single instance object */
    if (instance_id != 0) {
        result = COAP_404_NOT_FOUND;
        goto err_out;
    }

    if (length != 0) {
        result = COAP_400_BAD_REQUEST;
        goto err_out;
    }

    switch (resource_id) {
        //case LWM2M_RES_EXAMPLE:
        default:
            result = COAP_405_METHOD_NOT_ALLOWED;
    }

err_out:
    return result;
}

/*
 * Call this from the main loop to check whether a reboot was requested.
 */
bool lwm2m_registry_reboot_requested(void)
{
    return reboot;
}

lwm2m_object_t *lwm2m_get_object_registry(registry_handler_t *hndlr)
{
    lwm2m_object_t *obj;

    obj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (obj == NULL) {
        goto err_out;
    }

    memset(obj, 0, sizeof(lwm2m_object_t));
    obj->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));

    if (obj->instanceList == NULL) {
        goto free_obj;
    }

    memset(obj->instanceList, 0, sizeof(lwm2m_list_t));

    obj->objID = LWM2M_REGISTRY_OBJECT_ID;

    obj->readFunc = prv_registry_read;
    obj->writeFunc = prv_registry_write;
    obj->executeFunc = prv_registry_execute;
    obj->discoverFunc = prv_registry_discover;

    /* Don't allocate memory for stuff that isn't used at the moment */
    /* if (obj->userData == NULL) { */
    /*    goto free_ilist; */
    /* } */
    /*  */
    /* memset(obj->userData, 0, sizeof(reg_data_t)); */

    /* init userData for riot registry integration */
    obj->userData = lwm2m_malloc(sizeof(reg_data_t));
    ((reg_data_t*)(obj->userData))->hndlr = hndlr;
    ((reg_data_t*)(obj->userData))->res_list_size = 0;
    ((reg_data_t*)(obj->userData))->res_list = malloc(0); // Initial malloc for realloc to work properly
    /* call hndlr_export to init the res_list */
    char* buf = "";
    hndlr->hndlr_export(_registry_export, 0, &buf, obj->userData);

    return obj;

/* free_ilist: */
/*    lwm2m_free(obj->instanceList); */

free_obj:
    lwm2m_free(obj);

err_out:
    return NULL;
}

void lwm2m_free_object_registry(lwm2m_object_t *obj)
{
    if (obj == NULL) {
        return;
    }
    if (obj->userData) {
        lwm2m_free(obj->userData);
    }
    if (obj->instanceList) {
        lwm2m_free(obj->instanceList);
    }
    lwm2m_free(obj);
}