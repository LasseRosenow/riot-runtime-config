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

#ifndef OBJECTS_REGISTRY_H
#define OBJECTS_REGISTRY_H

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

/**
 * @brief Error codes for the
 * @ref lwm2m_registry_resources::LWM2M_RES_ERROR_CODE "Error" resource in the
 * registry object of LwM2M
 */
enum lwm2m_registry_error_codes {
    LWM2M_REGISTRY_ERR_NO_ERR     = 0, /**< No error */
    LWM2M_REGISTRY_ERR_LOW_BATT   = 1, /**< Low battery power */
    LWM2M_REGISTRY_ERR_EXT_OFF    = 2, /**< External power supply off */
    LWM2M_REGISTRY_ERR_GPS_ERR    = 3, /**< GPS module failure */
    LWM2M_REGISTRY_ERR_LOW_SIGNAL = 4, /**< Low received signal strength */
    LWM2M_REGISTRY_ERR_NO_MEM     = 5, /**< Out of memory */
    LWM2M_REGISTRY_ERR_SMS_ERR    = 6, /**< SMS failure */
    LWM2M_REGISTRY_ERR_IP_ERR     = 7, /**< IP connectivity failure */
    LWM2M_REGISTRY_ERR_PERIPH_ERR = 8  /**< Peripheral malfunction */
};

lwm2m_object_t *lwm2m_get_object_registry(registry_handler_t *hndlr, int obj_id);

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

#endif /* OBJECTS_REGISTRY_H */
/** @} */
