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

#include "registry_lwm2m.h"
#include "registry.h"

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

    clist_node_t *node = registry_handlers.next;

    if (!node) {
        puts("No registry handlers registered.");
    }

    do  {
        node = node->next;
        registry_handler_t *hndlr = container_of(node, registry_handler_t, node);

        obj_list_counter++;
        obj_list = realloc(obj_list, obj_list_counter * sizeof(*obj_list));
        obj_list[obj_list_counter - 1] = lwm2m_get_object_registry(hndlr);
    } while (node != registry_handlers.next);

    /* start the lwm2m client */
    lwm2m_client_run(&client_data, obj_list, obj_list_counter);
}

static int _registry_generate_items(const char *name, char *val, void *context) {
    (void)val;

    /* Remove "handler name" from "name" */
    while ((*name) != REGISTRY_NAME_SEPARATOR) {
        name++;
    }

    name++;

    /* Print the Item part of the lwm2m model */
    printf("            <Item ID=\"%d\">\n", *((int*)context));
	printf("                <Name>%s</Name>\n", name);
	printf("                <Operations>RW</Operations>\n");
	printf("                <MultipleInstances>Single</MultipleInstances>\n");
	printf("                <Mandatory>Optional</Mandatory>\n");
	printf("                <Type>String</Type>\n");
	printf("                <RangeEnumeration></RangeEnumeration>\n");
	printf("                <Units></Units>\n");
	printf("                <Description></Description>\n");
	printf("            </Item>\n");

    /* Increment the id, so that the next call will have a higher id */
    (*(int*)context)++;

    return 0;
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

        clist_node_t *node = registry_handlers.next;

        int obj_id = 32769;
        do  {
            node = node->next;
            registry_handler_t *hndlr = container_of(node, registry_handler_t, node);
            
            printf("    <Object ObjectType=\"MODefinition\">\n");
            printf("        <Name>%s</Name>\n", hndlr->name);
	        printf("        <Description1></Description1>\n");
            printf("        <ObjectID>%d</ObjectID>\n", obj_id);
            printf("        <ObjectURN>urn:oma:lwm2m:x:%d</ObjectURN>\n", obj_id);
            printf("        <LWM2MVersion>1.0</LWM2MVersion>\n");
            printf("        <ObjectVersion>1.0</ObjectVersion>\n");
            printf("        <MultipleInstances>Single</MultipleInstances>\n");
            printf("        <Mandatory>Optional</Mandatory>\n");
            printf("        <Resources>\n");

            int index = 0;
            char* buf = "";
            hndlr->hndlr_export(_registry_generate_items, 0, &buf, &index);
            
            printf("        </Resources>\n");
            printf("        <Description2></Description2>\n");
            printf("    </Object>\n");

            obj_id++;
        } while (node != registry_handlers.next);

        printf("</LWM2M>\n");

        return 0;
    }

    help_error:
    printf("usage: %s <generate>\n", argv[0]);

    return 1;
}
