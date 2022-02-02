/*
 * Copyright (C) 2019 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     lwm2m_objects
 * @defgroup    lwm2m_objects_registry Registry LwM2M object
 * @brief       Registry object implementation for LwM2M client using Wakaama
 * @{
 *
 * @file
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 */

#ifndef LWM2M_OBJECTS_ABSTRACT_OBJECT_H
#define LWM2M_OBJECTS_ABSTRACT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liblwm2m.h"
#include "lwm2m_client_config.h"
#include "lwm2m_client.h"
#include "registry.h"

lwm2m_object_t *lwm2m_get_object_registry(registry_schema_t *hndlr, int obj_id);

void lwm2m_free_object_registry(lwm2m_object_t *obj);

/**
 * @brief Frees the memory of @p obj registry object
 *
 * @param[in] obj pointer to the registry object
 */
void lwm2m_free_object_registry(lwm2m_object_t *obj);

/**
 * @brief Determines if a reboot request has been issued to the device by a
 *        server.
 *
 * @return true reboot has been requested
 * @return false reboot has not been requested
 */
bool lwm2m_registry_reboot_requested(void);


#ifdef __cplusplus
}
#endif

#endif /* LWM2M_OBJECTS_ABSTRACT_OBJECT_H */
/** @} */
