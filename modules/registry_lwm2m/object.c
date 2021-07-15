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
typedef enum {
    REG_DATA_OPERATION_TYPE_READ_WRITE,
    REG_DATA_OPERATION_TYPE_EXEC,
} reg_data_operation_type_t;

typedef struct {
    reg_data_operation_type_t type;
    char *value;
} reg_data_res_t;

typedef struct {
    registry_handler_t *hndlr;
    reg_data_res_t *res_list;
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
        int res_len = userData->res_list_size;

        /* Create data_arrayP */
        *data_arrayP = lwm2m_data_new(res_len);
        if (*data_arrayP == NULL) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }

        /* Calculate readable res count and init data_arrayP */
        int data_arrayP_index = 0;
        for (int i = 0; i < res_len; i++) {
            if (userData->res_list[i].type == REG_DATA_OPERATION_TYPE_READ_WRITE) {
                (*data_arrayP)[data_arrayP_index].id = i;
                data_arrayP_index++;
                (*num_dataP) += 1;
            }
        }
    }

    /* Get values and send them to lwm2m */
    for (int i = 0; i < *num_dataP; i++) {
        int index = (*data_arrayP)[i].id;
        if (index < userData->res_list_size) {
            char buf[REGISTRY_MAX_VAL_LEN];
            registry_get_value(userData->res_list[index].value, buf, REGISTRY_MAX_VAL_LEN);
            lwm2m_data_encode_string(buf, *data_arrayP + i); // TODO Add types!
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
                registry_set_value(userData->res_list[index].value, buf);
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
    reg_data_t *userData = (reg_data_t *)objectP->userData;

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

    if (userData->res_list[resource_id].type == REG_DATA_OPERATION_TYPE_EXEC) {
        userData->hndlr->hndlr_commit(userData->hndlr->context);
        result = COAP_204_CHANGED;
    } else {
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

lwm2m_object_t *lwm2m_get_object_registry(registry_handler_t *hndlr, int obj_id)
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

    obj->objID = obj_id;

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

    /* Init userData for riot registry integration */
    obj->userData = lwm2m_malloc(sizeof(reg_data_t));
    reg_data_t* userData = (reg_data_t*)(obj->userData);
    userData->hndlr = hndlr;
    userData->res_list_size = 0;
    userData->res_list = malloc(0); // Initial malloc for realloc to work properly

    /* Init the commit executable item inside res_list */
    userData->res_list = realloc(userData->res_list, (userData->res_list_size + 1) * sizeof(reg_data_res_t));
    userData->res_list[userData->res_list_size].type = REG_DATA_OPERATION_TYPE_EXEC;
    userData->res_list_size += 1;

    /* Init the res_list */
    for (int i = 0; i < userData->hndlr->parameters_len; i++) {
        registry_parameter_t parameter = hndlr->parameters[i];

        /* Generate whole registry parameter path from group and parameter name */
        char *path = malloc(strlen(hndlr->name) + strlen(parameter.name) + 1);
        strcpy(path, hndlr->name);
        strcpy(path + strlen(hndlr->name), "/");
        strcpy(path + strlen(hndlr->name) + 1, parameter.name);

        /* Increase the size of the list of property names to fit in the new exported property name */
        userData->res_list = realloc(userData->res_list, (userData->res_list_size + 1) * sizeof(reg_data_res_t));
        userData->res_list[userData->res_list_size].type = REG_DATA_OPERATION_TYPE_READ_WRITE;
        userData->res_list[userData->res_list_size].value = path; // TODO replace with id

        /* Increase the size counter of the list */
        userData->res_list_size += 1;
    }

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