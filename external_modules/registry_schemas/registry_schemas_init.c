/*
 * Copyright (C) 2023 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry_cli RIOT Registry Schemas
 * @ingroup     sys
 * @brief       RIOT Registry Schemas module providing common sys schemas for the RIOT Registry sys module
 * @{
 *
 * @file
 *
 * @author      Lasse Rosenow <lasse.rosenow@haw-hamburg.de>
 */

#include "registry.h"
#include "registry_schemas.h"

void registry_schemas_init(void)
{
#if IS_ACTIVE(CONFIG_REGISTRY_ENABLE_SCHEMA_FULL_EXAMPLE) || IS_ACTIVE(DOXYGEN)
    registry_register_schema(REGISTRY_ROOT_GROUP_SYS, &registry_schema_full_example);
#endif

#if IS_ACTIVE(CONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED) || IS_ACTIVE(DOXYGEN)
    registry_register_schema(REGISTRY_ROOT_GROUP_SYS, &registry_schema_rgb_led);
#endif
}

/** @} */
