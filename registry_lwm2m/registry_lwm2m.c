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

/* static int export_func(const char *name, char *val) {
    (void)val;

    printf("Export: %s\n", name);

    return 0;
}

static void _get_reg_data_from_registry_handlers(uint16_t* reg_data) {
    clist_node_t *node = registry_handlers.next;
    uint16_t size = 0;

    do  {
        node = node->next;
        registry_handler_t *hndlr = container_of(node, registry_handler_t, node);
        (void)hndlr;
        size++;

        char* buf = "";
        hndlr->hndlr_export(export_func, 0, &buf, "");

        printf("test\n");

        reg_data = realloc(reg_data, size * sizeof(uint16_t));
        reg_data[size - 1] = 5;
    } while (node != registry_handlers.next);
} */

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

int registry_lwm2m_cli_cmd(int argc, char **argv)
{
    if (argc == 1) {
        goto help_error;
    }

    if (!strcmp(argv[1], "generate")) {
        /* TODO generate lwm2m server models */
        return 0;
    }

    help_error:
    printf("usage: %s <generate>\n", argv[0]);

    return 1;
}
