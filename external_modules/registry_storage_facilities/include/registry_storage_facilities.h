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
#if IS_ACTIVE(CONFIG_REGISTRY_ENABLE_STORAGE_FACILITY_HEAP_DUMMY) || IS_ACTIVE(DOXYGEN)
extern registry_storage_facility_t registry_storage_facility_heap_dummy;
#endif

/* vfs */
#if IS_ACTIVE(CONFIG_REGISTRY_ENABLE_STORAGE_FACILITY_VFS) || IS_ACTIVE(DOXYGEN)
extern registry_storage_facility_t registry_storage_facility_vfs;
#endif

/** @} */
#endif /* REGISTRY_STORAGE_FACILITIES_H */
