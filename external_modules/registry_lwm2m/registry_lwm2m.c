/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Wakaama LwM2M Client CLI support
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */

#include "kernel_defines.h"
#include "lwm2m_client.h"
#include "lwm2m_client_objects.h"
#include "lwm2m_platform.h"
#include "object.h"

//#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
#include "float.h"
//#endif /* CONFIG_REGISTRY_USE_INT64 */

#include "registry_lwm2m.h"
#include "registry.h"

#define LWM2M_OBJECT_ID_PRIVATE_RANGE_START 32769
#define LWM2M_OBJECT_ID_PRIVATE_RANGE_END 42768

/* counts how many objects are stored in obj_list_counter, which is necessary no know its size for reallocation */
int obj_list_counter = 3;

uint8_t connected = 0;
lwm2m_object_t **obj_list;
lwm2m_client_data_t client_data;

void registry_lwm2m_cli_init(void)
{
    /* this call is needed before creating any objects */
    lwm2m_client_init(&client_data);

    /* add objects that will be registered */
    obj_list = malloc(obj_list_counter * sizeof(lwm2m_object_t*));
    obj_list[0] = lwm2m_client_get_security_object(&client_data);
    obj_list[1] = lwm2m_client_get_server_object(&client_data);
    obj_list[2] = lwm2m_client_get_device_object(&client_data);

    if (!obj_list[0] || !obj_list[1] || !obj_list[2]) {
        puts("Could not create mandatory objects");
    }

    /* create the Riot Registry objects */

    clist_node_t *node = registry_schemas.next;

    if (!node) {
        puts("No registry schema instances registered.");
    }

    int obj_id = LWM2M_OBJECT_ID_PRIVATE_RANGE_START;
    do {
        node = node->next;
        registry_schema_t *hndlr = container_of(node, registry_schema_t, node);

        obj_list_counter++;
        obj_list = realloc(obj_list, obj_list_counter * sizeof(*obj_list));
        obj_list[obj_list_counter - 1] = lwm2m_get_object_registry(hndlr, obj_id);

        obj_id++;
    } while (node != registry_schemas.next);

    /* start the lwm2m client */
    lwm2m_client_run(&client_data, obj_list, obj_list_counter);
}

int registry_lwm2m_cli_cmd(int argc, char **argv)
{
    if (argc == 1) {
        goto help_error;
    }

    if (!strcmp(argv[1], "generate")) {
        /* TODO generate lwm2m server models */
        printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
        printf("<LWM2M xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://openmobilealliance.org/tech/profiles/LWM2M.xsd\">\n");

        clist_node_t *node = registry_schemas.next;

        do  {
            node = node->next;
            registry_schema_t *hndlr = container_of(node, registry_schema_t, node);
            int obj_id = LWM2M_OBJECT_ID_PRIVATE_RANGE_START + hndlr->id;
            
            printf("    <Object ObjectType=\"MODefinition\">\n");
            printf("        <Name>%s</Name>\n", hndlr->name);
	        printf("        <Description1>%s</Description1>\n", hndlr->description);
            printf("        <ObjectID>%d</ObjectID>\n", obj_id);
            printf("        <ObjectURN>urn:oma:lwm2m:x:%d</ObjectURN>\n", obj_id);
            printf("        <LWM2MVersion>1.0</LWM2MVersion>\n");
            printf("        <ObjectVersion>1.0</ObjectVersion>\n");
            printf("        <MultipleInstances>Single</MultipleInstances>\n");
            printf("        <Mandatory>Optional</Mandatory>\n");
            printf("        <Resources>\n");

            /* Print the commit executable item */
            printf("            <Item ID=\"0\">\n");
            printf("                <Name>commit</Name>\n");
            printf("                <Operations>E</Operations>\n");
            printf("                <MultipleInstances>Single</MultipleInstances>\n");
            printf("                <Mandatory>Optional</Mandatory>\n");
            printf("                <Type></Type>\n");
            printf("                <RangeEnumeration></RangeEnumeration>\n");
            printf("                <Units></Units>\n");
            printf("                <Description>Commit changes</Description>\n");
            printf("            </Item>\n");

            for (int i = 0; i < hndlr->schemas_len; i++) {
                registry_schema_item_t schema = hndlr->schemas[i];
                registry_parameter_t parameter = schema.value.parameter;

                printf("            <Item ID=\"%d\">\n", schema.id + 1); // Increase by 1 because the first item is the commit executable
                printf("                <Name>%s</Name>\n", schema.name);
                printf("                <Operations>RW</Operations>\n");
                printf("                <MultipleInstances>Single</MultipleInstances>\n");
                printf("                <Mandatory>Optional</Mandatory>\n");

                printf("                <Type>");
                switch (parameter.type) {
                    case REGISTRY_TYPE_NONE:
                        printf("String");
                        break;

                    case REGISTRY_TYPE_INT8:
                        printf("Integer");
                        break;

                    case REGISTRY_TYPE_INT16:
                        printf("Integer");
                        break;

                    case REGISTRY_TYPE_INT32:
                        printf("Integer");
                        break;

                    case REGISTRY_TYPE_STRING:
                        printf("String");
                        break;

                    case REGISTRY_TYPE_BOOL:
                        printf("Boolean");
                        break;

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
                    case REGISTRY_TYPE_INT64:
                        printf("Integer");
                        break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
                    case REGISTRY_TYPE_FLOAT:
                        printf("Float");
                        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT */
                    
                    default:
                        printf("String");
                        break;
                }
                printf("</Type>\n");

                printf("                <RangeEnumeration>");
                switch (parameter.type) {
                    case REGISTRY_TYPE_NONE: break;

                    case REGISTRY_TYPE_INT8:
                        printf("%d-%d", INT8_MIN, INT8_MAX);
                        break;

                    case REGISTRY_TYPE_INT16:
                        printf("%d-%d", INT16_MIN, INT16_MAX);
                        break;

                    case REGISTRY_TYPE_INT32:
                        printf("%d-%d", INT32_MIN, INT32_MAX);
                        break;

                    case REGISTRY_TYPE_STRING:
                        printf("0-%d", REGISTRY_MAX_VAL_LEN);
                        break;

                    case REGISTRY_TYPE_BOOL: break;

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
                    case REGISTRY_TYPE_INT64:
                        printf("%lld-%lld", INT64_MIN, INT64_MAX);
                        break;
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT) || defined(DOXYGEN)
                    case REGISTRY_TYPE_FLOAT:
                        printf("-%f-%f", FLT_MAX, FLT_MAX);
                        break;
#endif /* CONFIG_REGISTRY_USE_FLOAT */
                    
                    default: break;
                }
                printf("</RangeEnumeration>\n");
                
                printf("                <Units></Units>\n");
                printf("                <Description>%s</Description>\n", schema.description);
                printf("            </Item>\n");
            }
            
            printf("        </Resources>\n");
            printf("        <Description2></Description2>\n");
            printf("    </Object>\n");

            obj_id++;
        } while (node != registry_schemas.next);

        printf("</LWM2M>\n");

        return 0;
    }

    help_error:
    printf("usage: %s <generate>\n", argv[0]);

    return 1;
}
