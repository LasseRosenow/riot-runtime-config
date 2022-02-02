#include "kernel_defines.h"
#include "lwm2m_client.h"
#include "lwm2m_client_objects.h"
#include "lwm2m_platform.h"
#include "object.h"
#include "float.h"
#include "assert.h"

#include "lwm2m_objects.h"
#include "registry.h"

clist_node_t lwm2m_objects;

void lwm2m_objects_register_object(lwm2m_objects_object_t *object)
{
    assert(object != NULL);
    clist_rpush(&lwm2m_objects, &(object->node));
}

/* counts how many objects are stored in obj_list_counter, which is necessary no know its size for reallocation */
//int obj_list_counter = 3;
//
//uint8_t connected = 0;
//lwm2m_object_t **obj_list;
//lwm2m_client_data_t client_data;
//
//void lwm2m_objects_cli_init(void)
//{
//    /* this call is needed before creating any objects */
//    lwm2m_client_init(&client_data);
//
//    /* remove in the future? */
//    /* add objects that will be registered */
//    obj_list = malloc(obj_list_counter * sizeof(lwm2m_object_t *));
//    obj_list[0] = lwm2m_client_get_security_object(&client_data);
//    obj_list[1] = lwm2m_client_get_server_object(&client_data);
//    obj_list[2] = lwm2m_client_get_device_object(&client_data);
//
//    if (!obj_list[0] || !obj_list[1] || !obj_list[2]) {
//        puts("Could not create mandatory objects");
//    }
//
//    /* create the Riot Registry objects */
//
//    clist_node_t *node = registry_schemas.next;
//
//    if (!node) {
//        puts("No registry schema instances registered.");
//    }
//
//    int obj_id = LWM2M_OBJECT_ID_PRIVATE_RANGE_START;
//
//    do {
//        node = node->next;
//        registry_schema_t *hndlr = container_of(node, registry_schema_t, node);
//
//        obj_list_counter++;
//        obj_list = realloc(obj_list, obj_list_counter * sizeof(*obj_list));
//        obj_list[obj_list_counter - 1] = lwm2m_get_object_registry(hndlr, obj_id);
//
//        obj_id++;
//    } while (node != registry_schemas.next);
//
//    /* start the lwm2m client */
//    lwm2m_client_run(&client_data, obj_list, obj_list_counter);
//}
//
//int lwm2m_objects_cli_cmd(int argc, char **argv)
//{
//    if (argc == 1) {
//        goto help_error;
//    }
//
//    if (!strcmp(argv[1], "generate")) {
//        /* TODO generate lwm2m server models */
//        printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
//        printf(
//            "<LWM2M xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://openmobilealliance.org/tech/profiles/LWM2M.xsd\">\n");
//
//        clist_node_t *node = registry_schemas.next;
//
//        do {
//            node = node->next;
//            registry_schema_t *hndlr = container_of(node, registry_schema_t, node);
//            int obj_id = LWM2M_OBJECT_ID_PRIVATE_RANGE_START + hndlr->id;
//
//            printf("    <Object ObjectType=\"MODefinition\">\n");
//            printf("        <Name>%s</Name>\n", hndlr->name);
//            printf("        <Description1>%s</Description1>\n", hndlr->description);
//            printf("        <ObjectID>%d</ObjectID>\n", obj_id);
//            printf("        <ObjectURN>urn:oma:lwm2m:x:%d</ObjectURN>\n", obj_id);
//            printf("        <LWM2MVersion>1.0</LWM2MVersion>\n");
//            printf("        <ObjectVersion>1.0</ObjectVersion>\n");
//            printf("        <MultipleInstances>Single</MultipleInstances>\n");
//            printf("        <Mandatory>Optional</Mandatory>\n");
//            printf("        <Resources>\n");
//
//            /* Print the commit executable item */
//            printf("            <Item ID=\"0\">\n");
//            printf("                <Name>commit</Name>\n");
//            printf("                <Operations>E</Operations>\n");
//            printf("                <MultipleInstances>Single</MultipleInstances>\n");
//            printf("                <Mandatory>Optional</Mandatory>\n");
//            printf("                <Type></Type>\n");
//            printf("                <RangeEnumeration></RangeEnumeration>\n");
//            printf("                <Units></Units>\n");
//            printf("                <Description>Commit changes</Description>\n");
//            printf("            </Item>\n");
//
//            for (int i = 0; i < hndlr->items_len; i++) {
//                registry_schema_item_t schema = hndlr->items[i];
//                registry_parameter_t parameter = schema.value.parameter;
//
//                printf("            <Item ID=\"%d\">\n", schema.id + 1);  // Increase by 1 because the first item is the commit executable
//                printf("                <Name>%s</Name>\n", schema.name);
//                printf("                <Operations>RW</Operations>\n");
//                printf("                <MultipleInstances>Single</MultipleInstances>\n");
//                printf("                <Mandatory>Optional</Mandatory>\n");
//
//                printf("                <Type>");
//                switch (parameter.type) {
//                case REGISTRY_TYPE_NONE:
//                    printf("String");
//                    break;
//
//                case REGISTRY_TYPE_STRING:
//                    printf("String");
//                    break;
//
//                case REGISTRY_TYPE_BOOL:
//                    printf("Boolean");
//                    break;
//
//                case REGISTRY_TYPE_UINT8:
//                    printf("Integer");
//                    break;
//
//                case REGISTRY_TYPE_UINT16:
//                    printf("Integer");
//                    break;
//
//                case REGISTRY_TYPE_UINT32:
//                    printf("Integer");
//                    break;
//
//#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
//                case REGISTRY_TYPE_UINT64:
//                    printf("Integer");
//                    break;
//#endif /* CONFIG_REGISTRY_USE_UINT64 */
//
//                case REGISTRY_TYPE_INT8:
//                    printf("Integer");
//                    break;
//
//                case REGISTRY_TYPE_INT16:
//                    printf("Integer");
//                    break;
//
//                case REGISTRY_TYPE_INT32:
//                    printf("Integer");
//                    break;
//
//#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
//                case REGISTRY_TYPE_INT64:
//                    printf("Integer");
//                    break;
//#endif /* CONFIG_REGISTRY_USE_INT64 */
//
//#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
//                case REGISTRY_TYPE_FLOAT32:
//                    printf("Float");
//                    break;
//#endif /* CONFIG_REGISTRY_USE_FLOAT32 */
//
//#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
//                case REGISTRY_TYPE_FLOAT64:
//                    printf("Float");
//                    break;
//#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
//
//                default:
//                    printf("String");
//                    break;
//                }
//                printf("</Type>\n");
//
//                printf("                <RangeEnumeration>");
//                switch (parameter.type) {
//                case REGISTRY_TYPE_NONE: break;
//
//                case REGISTRY_TYPE_STRING:
//                    printf("0-%d", REGISTRY_MAX_VAL_LEN);
//                    break;
//
//                case REGISTRY_TYPE_BOOL: break;
//
//                case REGISTRY_TYPE_UINT8:
//                    printf("0-%" PRIu8, UINT8_MAX);
//                    break;
//
//                case REGISTRY_TYPE_UINT16:
//                    printf("0-%" PRIu16, UINT16_MAX);
//                    break;
//
//                case REGISTRY_TYPE_UINT32:
//                    printf("0-%" PRIu32, (uint32_t)UINT32_MAX);
//                    break;
//
//#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
//                case REGISTRY_TYPE_UINT64:
//                    printf("0-%" PRIu64, UINT64_MAX);
//                    break;
//#endif /* CONFIG_REGISTRY_USE_UINT64 */
//
//                case REGISTRY_TYPE_INT8:
//                    printf("%" PRIi8 "-%" PRIi8, INT8_MIN, INT8_MAX);
//                    break;
//
//                case REGISTRY_TYPE_INT16:
//                    printf("%" PRIi16 "-%" PRIi16, INT16_MIN, INT16_MAX);
//                    break;
//
//                case REGISTRY_TYPE_INT32:
//                    printf("%" PRIi32 "-%" PRIi32, (int32_t)INT32_MIN, (int32_t)INT32_MAX);
//                    break;
//
//#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
//                case REGISTRY_TYPE_INT64:
//                    printf("%" PRIi64 "-%" PRIi64, INT64_MIN, INT64_MAX);
//                    break;
//#endif /* CONFIG_REGISTRY_USE_INT64 */
//
//#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
//                case REGISTRY_TYPE_FLOAT32:
//                    printf("-%f-%f", FLT_MAX, FLT_MAX);
//                    break;
//#endif /* CONFIG_REGISTRY_USE_FLOAT32 */
//
//#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
//                case REGISTRY_TYPE_FLOAT64:
//                    printf("-%lf-%lf", DBL_MAX, DBL_MAX);
//                    break;
//#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
//
//                default: break;
//                }
//                printf("</RangeEnumeration>\n");
//
//                printf("                <Units></Units>\n");
//                printf("                <Description>%s</Description>\n", schema.description);
//                printf("            </Item>\n");
//            }
//
//            printf("        </Resources>\n");
//            printf("        <Description2></Description2>\n");
//            printf("    </Object>\n");
//
//            obj_id++;
//        } while (node != registry_schemas.next);
//
//        printf("</LWM2M>\n");
//
//        return 0;
//    }
//
//help_error:
//    printf("usage: %s <generate>\n", argv[0]);
//
//    return 1;
//}
