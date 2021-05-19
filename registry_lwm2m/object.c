#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kernel_defines.h"
#include "liblwm2m.h"
#include "object.h"
#include "lwm2m_client_config.h"

/* Set to true if reboot requested. */
static bool reboot;

/*Descriptor of a LwM2M registry object instance */
typedef struct {
    char* example;      /**< example */
} reg_data_t;

char* sdf;

static uint8_t prv_registry_discover(uint16_t instance_id, int *num_dataP,
                                   lwm2m_data_t **data_arrayP,
                                   lwm2m_object_t *objectP)
{
    uint8_t result;
    int i;

    (void)objectP;

    if (instance_id != 0) {
        return COAP_404_NOT_FOUND;
    }

    result = COAP_205_CONTENT;

    if (*num_dataP == 0) {
        /* This list must contain all available resources */
        uint16_t res[] = {
            LWM2M_RES_EXAMPLE,
        };
        int len = ARRAY_SIZE(res);

        *data_arrayP = lwm2m_data_new(len);
        if (*data_arrayP == NULL) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
        *num_dataP = len;
        for (i = 0; i < len; i++) {
            (*data_arrayP)[i].id = res[i];
        }
    }
    else {
        /* Check if each given resource is present */
        for (i = 0; i < *num_dataP && result == COAP_205_CONTENT; i++) {
            switch ((*data_arrayP)[i].id) {
                case LWM2M_RES_EXAMPLE:
                    break;
                default:
                    result = COAP_404_NOT_FOUND;
            }
        }
    }

    return result;
}

static uint8_t prv_registry_read(uint16_t instance_id, int *num_dataP,
                               lwm2m_data_t **data_arrayP,
                               lwm2m_object_t *objectP)
{
    int i;
    uint8_t result = COAP_404_NOT_FOUND;
    reg_data_t *data = (reg_data_t *)objectP->userData;
    reg_data_t *targetP = (reg_data_t *)lwm2m_list_find(objectP->instanceList, instance_id);

    (void)data;

    /* Single instance object */
    if (instance_id != 0) {
        goto out;
    }

    /* Full object requested */
    if (*num_dataP == 0) {
        /* This list must contain all readable resources */
        uint16_t resList[] = {
            LWM2M_RES_EXAMPLE,
        };
        int cnt = ARRAY_SIZE(resList);
        *data_arrayP = lwm2m_data_new(cnt);
        if (*data_arrayP == NULL) {
            result = COAP_500_INTERNAL_SERVER_ERROR;
            goto out;
        }
        *num_dataP = cnt;
        for (i = 0; i < cnt; i++) {
            (*data_arrayP)[i].id = resList[i];
        }
    }

    for (i = 0; i < *num_dataP; i++) {
        switch ((*data_arrayP)[i].id) {
            /* Exec resources */
            case LWM2M_RES_EXAMPLE:
                lwm2m_data_encode_string(targetP->example, *data_arrayP + i);
                result = COAP_205_CONTENT;
                break;
            default:
                result = COAP_404_NOT_FOUND;
                goto out;
        }
    }

out:
    return result;
}

static uint8_t prv_registry_write(uint16_t instance_id, int num_data,
                                lwm2m_data_t *data_array,
                                lwm2m_object_t *objectP)
{
    reg_data_t *data = (reg_data_t *)objectP->userData;
    reg_data_t *targetP = (reg_data_t *)lwm2m_list_find(objectP->instanceList, instance_id);

    (void)data;
    (void)instance_id;
    (void)num_data;
    (void)data_array;

    uint8_t result = COAP_404_NOT_FOUND;

    if (data_array[0].id < LWM2M_REGISTRY_RESOURCES) {
        for (int i = 0; i < num_data; i++) {
            /* No multiple instance resources */
            if (data_array[i].type == LWM2M_TYPE_MULTIPLE_RESOURCE)
            {
                result = COAP_404_NOT_FOUND;
                continue;
            }

            switch (data_array[i].id) {
                /* Exec resources */
                case LWM2M_RES_EXAMPLE:
                    if (data_array[i].type == LWM2M_TYPE_STRING || data_array[i].type == LWM2M_TYPE_OPAQUE)
                    {
                        free(targetP->example);
                        targetP->example = (char *)malloc( data_array[i].value.asBuffer.length * sizeof(char) );
                        strncpy(targetP->example, (char*)data_array[i].value.asBuffer.buffer, data_array[i].value.asBuffer.length);
                        result = COAP_204_CHANGED;
                    }
                    else
                    {
                        result = COAP_400_BAD_REQUEST;
                    }
                    break;
                default:
                    result = COAP_404_NOT_FOUND;
            }
        }
    }
    else {
        result = COAP_404_NOT_FOUND;
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
        case LWM2M_RES_EXAMPLE:
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

lwm2m_object_t *lwm2m_get_object_registry(void)
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
    /* obj->userData = lwm2m_malloc(sizeof(reg_data_t)); */
    /* if (obj->userData == NULL) { */
    /*    goto free_ilist; */
    /* } */
    /*  */
    /* memset(obj->userData, 0, sizeof(reg_data_t)); */
    /* INT USER DATA HERE */

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