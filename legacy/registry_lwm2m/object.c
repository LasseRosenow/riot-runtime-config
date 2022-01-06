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
    reg_data_operation_type_t operation_type;
    registry_type_t data_type;
    int *path;
    int path_len;
} reg_data_res_t;

typedef struct {
    registry_schema_t *hndlr;
    reg_data_res_t *res_list;
    int res_list_size;
} reg_data_t;

/* static void _get_reg_data_from_registry_schemas(uint16_t* reg_data) {
    clist_node_t *node = registry_schemas.next;
    int size = 0;

    do  {
        node = node->next;
        registry_schema_t *hndlr = container_of(node, registry_schema_t, node);
        size++;

        reg_data = realloc(reg_data, size * sizeof(uint16_t));
    } while (node != registry_schemas.next);
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
            if (userData->res_list[i].operation_type == REG_DATA_OPERATION_TYPE_READ_WRITE) {
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
            registry_get_value(userData->res_list[index].path, userData->res_list[index].path_len,
                               buf, REGISTRY_MAX_VAL_LEN);
            switch (userData->res_list[index].data_type) {
            case REGISTRY_TYPE_NONE:
                return COAP_400_BAD_REQUEST;
                break;

            case REGISTRY_TYPE_INT8: {
                int8_t value;
                registry_value_from_str(buf, REGISTRY_TYPE_INT8, &value, 0);
                lwm2m_data_encode_int(value, *data_arrayP + i);
                break;
            }

            case REGISTRY_TYPE_INT16: {
                int value;
                registry_value_from_str(buf, REGISTRY_TYPE_INT16, &value, 0);
                lwm2m_data_encode_int(value, *data_arrayP + i);
                break;
            }

            case REGISTRY_TYPE_INT32: {
                int value;
                registry_value_from_str(buf, REGISTRY_TYPE_INT32, &value, 0);
                lwm2m_data_encode_int(value, *data_arrayP + i);
                break;
            }

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
            case REGISTRY_TYPE_INT64: {
                int value;
                registry_value_from_str(buf, REGISTRY_TYPE_INT64, &value, 0);
                lwm2m_data_encode_int(value, *data_arrayP + i);
                break;
            }
#endif /* CONFIG_REGISTRY_USE_INT64 */

            case REGISTRY_TYPE_STRING:
                lwm2m_data_encode_string(buf, *data_arrayP + i);
                break;

            case REGISTRY_TYPE_BOOL: {
                bool value;
                registry_value_from_str(buf, REGISTRY_TYPE_BOOL, &value, sizeof(bool));
                lwm2m_data_encode_bool(value, *data_arrayP + i);
                break;
            }

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
            case REGISTRY_TYPE_FLOAT32: {
                float value;
                registry_value_from_str(buf, REGISTRY_TYPE_FLOAT32, &value, 0);
                lwm2m_data_encode_float(value, *data_arrayP + i);
                break;
            }
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
            case REGISTRY_TYPE_FLOAT64: {
                float value;
                registry_value_from_str(buf, REGISTRY_TYPE_FLOAT64, &value, 0);
                lwm2m_data_encode_float(value, *data_arrayP + i);
                break;
            }
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

            default:
                return COAP_400_BAD_REQUEST;
                break;
            }
            result = COAP_205_CONTENT;
        }
        else {
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

            switch (data_array[i].type) {
            case LWM2M_TYPE_OPAQUE: {
                int index = data_array[i].id;
                char buf[REGISTRY_MAX_VAL_LEN] = { 0 };

                // TODO what if index would be something like 35508 even though res_list would only contain 6 items? ... something like a map or a helper function?
                switch (userData->res_list[index].data_type) {
                case REGISTRY_TYPE_NONE:
                    return COAP_400_BAD_REQUEST;
                    break;

                case REGISTRY_TYPE_INT8:
                case REGISTRY_TYPE_INT16:
                case REGISTRY_TYPE_INT32:
#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
                case REGISTRY_TYPE_INT64:
#endif /* CONFIG_REGISTRY_USE_INT64 */
                {
                    int64_t value;
                    lwm2m_data_decode_int(&data_array[i], &value);
                    snprintf(buf, REGISTRY_MAX_VAL_LEN, "%" PRId64, value);
                    break;
                }

                case REGISTRY_TYPE_STRING:
                    strncpy(buf, (char *)data_array[i].value.asBuffer.buffer,
                            data_array[i].value.asBuffer.length);
                    break;

                case REGISTRY_TYPE_BOOL: {
                    bool value;
                    lwm2m_data_decode_bool(&data_array[i], &value);
                    snprintf(buf, REGISTRY_MAX_VAL_LEN, "%d", value);
                    break;
                }

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
                case REGISTRY_TYPE_FLOAT32: {
                    double value;
                    lwm2m_data_decode_float(&data_array[i], &value);
                    snprintf(buf, REGISTRY_MAX_VAL_LEN, "%lf", value);
                    break;
                }
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
                case REGISTRY_TYPE_FLOAT64: {
                    double value;
                    lwm2m_data_decode_float(&data_array[i], &value);
                    snprintf(buf, REGISTRY_MAX_VAL_LEN, "%lf", value);
                    break;
                }
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

                default:
                    return COAP_400_BAD_REQUEST;
                    break;
                }
                registry_set_value(userData->res_list[index].path,
                                   userData->res_list[index].path_len, buf);
                result = COAP_204_CHANGED;
                break;
            }

            case LWM2M_TYPE_STRING: {
                result = COAP_204_CHANGED;
                break;
            }

            case LWM2M_TYPE_BOOLEAN: {
                result = COAP_204_CHANGED;
                break;
            }

            default:
                result = COAP_400_BAD_REQUEST;
                break;
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

    if (userData->res_list[resource_id].operation_type == REG_DATA_OPERATION_TYPE_EXEC) {
        // TODO
        //userData->hndlr->commit_cb(userData->hndlr->context);
        result = COAP_204_CHANGED;
    }
    else {
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

lwm2m_object_t *lwm2m_get_object_registry(registry_schema_t *hndlr, int obj_id)
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
    reg_data_t *userData = (reg_data_t *)(obj->userData);

    userData->hndlr = hndlr;
    userData->res_list_size = 0;
    userData->res_list = malloc(0); // Initial malloc for realloc to work properly

    /* Init the commit executable item inside res_list */
    userData->res_list =
        realloc(userData->res_list, (userData->res_list_size + 1) * sizeof(reg_data_res_t));
    userData->res_list[userData->res_list_size].operation_type = REG_DATA_OPERATION_TYPE_EXEC;
    userData->res_list_size += 1;

    /* Init the res_list */
    for (int i = 0; i < userData->hndlr->items_len; i++) {
        registry_schema_item_t schema = hndlr->items[i];
        registry_parameter_t parameter = hndlr->items[i].value.parameter;

        // TODO this only works for 1 level paths. No nesting etc.
        int path_len = 2;
        int *path = malloc(2 * sizeof(int));
        path[0] = hndlr->id;
        path[1] = schema.id;

        /* Increase the size of the list of property names to fit in the new exported property name */
        userData->res_list =
            realloc(userData->res_list, (userData->res_list_size + 1) * sizeof(reg_data_res_t));
        userData->res_list[userData->res_list_size].operation_type =
            REG_DATA_OPERATION_TYPE_READ_WRITE;
        userData->res_list[userData->res_list_size].data_type = parameter.type;
        userData->res_list[userData->res_list_size].path = path;
        userData->res_list[userData->res_list_size].path_len = path_len;

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
