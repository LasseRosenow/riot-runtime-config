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
#ifdef CONFIG_REGISTRY_ENABLE_SCHEMA_FULL_EXAMPLE
    registry_register_schema(REGISTRY_ROOT_GROUP_SYS, &registry_schema_full_example);
#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_FULL_EXAMPLE */

#ifdef CONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED
    registry_register_schema(REGISTRY_ROOT_GROUP_SYS, &registry_schema_rgb_led);
#endif /* CONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED */
}

/** @} */
