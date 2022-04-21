#ifndef REGISTRY_STORAGE_FACILITIES_H
#define REGISTRY_STORAGE_FACILITIES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "registry.h"

/* heap dummy */
extern registry_store_t registry_store_heap_dummy;

/* vfs */
extern registry_store_t registry_store_vfs;

#endif /* REGISTRY_STORAGE_FACILITIES_H */
