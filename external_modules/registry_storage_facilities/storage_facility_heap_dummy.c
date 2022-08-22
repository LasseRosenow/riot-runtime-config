#include "registry_storage_facilities.h"

#include <string.h>
#include <stdio.h>
#include <kernel_defines.h>
#include "errno.h"

#define DUMMY_STORE_CAPACITY 100

/* The store argument is the descriptor of the storage facility */
static int load(registry_store_instance_t *store, const registry_path_t path, load_cb_t cb,
                void *cb_arg);
static int save(registry_store_instance_t *store, const registry_path_t path,
                const registry_value_t value);

/*
   This conceptual example shows the implementation of a dummy Storage Facility.
   The storage device is a dummy_storage array (represented by the
   `dummy_store_storage_t` struct).
 */
typedef struct {
    registry_root_group_id_t root_group_id;
    int schema_id;
    int instance_id;
    int path[REGISTRY_MAX_DIR_DEPTH];
    size_t path_len;
    uint8_t val[REGISTRY_MAX_VAL_LEN];
} dummy_store_storage_t;

/* This will be our "storage device" */
static dummy_store_storage_t dummy_store[DUMMY_STORE_CAPACITY];

/* Storage Facility interface descriptor to be registered in the RIOT
   Registry */
registry_store_t registry_store_heap_dummy = {
    .load = load,
    .save = save,
};

/* Implementation of `load`. Execute a `cb` callback for each configuration
   found in the dummy storage array */
static int load(registry_store_instance_t *store, const registry_path_t path, load_cb_t cb,
                void *cb_arg)
{
    // TODO implement "path" parameter!!
    (void)store;
    (void)path;
    registry_path_t new_path;
    uint8_t val[REGISTRY_MAX_VAL_LEN];

    for (size_t i = 0; i < DUMMY_STORE_CAPACITY; i++) {
        if (dummy_store[i].path_len > 0) {
            new_path = (registry_path_t) {
                .root_group_id = &dummy_store[i].root_group_id,
                .schema_id = &dummy_store[i].schema_id,
                .instance_id = &dummy_store[i].instance_id,
                .path = dummy_store[i].path,
                .path_len = dummy_store[i].path_len,
            };

            registry_value_t value = {
                .type = REGISTRY_TYPE_NONE,
                .buf = dummy_store[i].val,
                .buf_len = ARRAY_SIZE(val),
            };

            cb(new_path, value, cb_arg);
        }
    }
    return 0;
}

/* Implementation of `store`. Save parameter with given name and value in
   the dummy storage array */
static int save(registry_store_instance_t *store, const registry_path_t path,
                const registry_value_t value)
{
    int free_slot = -1;

    (void)store;

    if (path.schema_id == NULL) {
        return -EINVAL;
    }

    if (path.instance_id == NULL) {
        return -EINVAL;
    }

    for (size_t i = 0; i < DUMMY_STORE_CAPACITY; i++) {
        if (dummy_store[i].path_len > 0) {
            if (dummy_store[i].path_len == path.path_len) {
                for (size_t j = 0; j < dummy_store[i].path_len; j++) {
                    if (dummy_store[i].path[j] != path.path[j]) {
                        break;
                    }
                    else if (j == dummy_store[i].path_len - 1) {
                        // All ids within the path matched and this was the last one.
                        memcpy(dummy_store[i].val, value.buf, value.buf_len);
                        return 0;
                    }
                }
            }
        }
        else {
            if (free_slot == -1) {
                free_slot = i;
            }
        }
    }

    if (free_slot == -1) {
        return -1;
    }

    dummy_store[free_slot].root_group_id = *path.root_group_id;
    dummy_store[free_slot].schema_id = *path.schema_id;
    dummy_store[free_slot].instance_id = *path.instance_id;
    memcpy(dummy_store[free_slot].path, path.path, path.path_len * sizeof(int));
    memcpy(dummy_store[free_slot].val, value.buf, value.buf_len);
    dummy_store[free_slot].path_len = path.path_len;
    return 0;
}
