/*
 * Copyright (C) 2023 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry_cli RIOT Registry Storage Facilities
 * @ingroup     sys
 * @brief       RIOT Registry Storage Facilities module providing officially supported storage facilities for the RIOT Registry sys module
 * @{
 *
 * @file
 *
 * @author      Lasse Rosenow <lasse.rosenow@haw-hamburg.de>
 */

#ifndef REGISTRY_STORAGE_FACILITIES_H
#define REGISTRY_STORAGE_FACILITIES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "registry.h"

/* heap dummy */
extern registry_storage_facility_t registry_storage_facility_heap_dummy;

/* vfs */
extern registry_storage_facility_t registry_storage_facility_vfs;

/** @} */
#endif /* REGISTRY_STORAGE_FACILITIES_H */
