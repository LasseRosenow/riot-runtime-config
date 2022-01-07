#include "storage_facility_dummy.h"

#include <string.h>
#include <stdio.h>

#define DUMMY_STORE_CAPACITY 100

/* The store argument is the descriptor of the storage facility */
static int registry_dummy_load(registry_store_t *store, load_cb_t cb,
                               void *cb_arg);
static int registry_dummy_save(registry_store_t *store, const int *path, int path_len,
                               const char *value);

/*
   This conceptual example shows the implementation of a dummy Storage Facility.
   The storage device is a dummy_storage array (represented by the
   `dummy_store_storage_t` struct).
 */
typedef struct {
    int path[REGISTRY_MAX_DIR_DEPTH];
    int path_len;
    char val[REGISTRY_MAX_VAL_LEN];
} dummy_store_storage_t;

/* This will be our "storage device" */
static dummy_store_storage_t dummy_store[DUMMY_STORE_CAPACITY];

/* Storage Facility interface descriptor to be registered in the RIOT
   Registry */
registry_store_itf_t dummy_store_itf = {
    .load = registry_dummy_load,
    .save = registry_dummy_save,
};

/* Implementation of `load`. Execute a `cb` callback for each configuration
   found in the dummy storage array */
static int registry_dummy_load(registry_store_t *store, load_cb_t cb,
                               void *cb_arg)
{
    (void)store;
    int path[REGISTRY_MAX_DIR_DEPTH];
    char val[REGISTRY_MAX_NAME_LEN];

    for (int i = 0; i < DUMMY_STORE_CAPACITY; i++) {
        if (dummy_store[i].path_len > 0) {
            memcpy(path, dummy_store[i].path, dummy_store[i].path_len * sizeof(int));
            strcpy(val, dummy_store[i].val);
            cb(path, dummy_store[i].path_len, val, cb_arg);
        }
    }
    return 0;
}

/* Implementation of `store`. Save parameter with given name and value in
   the dummy storage array */
static int registry_dummy_save(registry_store_t *store, const int *path, int path_len,
                               const char *value)
{
    int free_slot = -1;

    (void)store;

    for (int i = 0; i < DUMMY_STORE_CAPACITY; i++) {
        if (dummy_store[i].path_len > 0) {
            if (dummy_store[i].path_len == path_len) {
                for (int j = 0; j < dummy_store[i].path_len; j++) {
                    if (dummy_store[i].path[j] != path[j]) {
                        break;
                    }
                    else if (j == dummy_store[i].path_len - 1) {
                        // All ids within the path matched and this was the last one.
                        strcpy(dummy_store[i].val, value);
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

    memcpy(dummy_store[free_slot].path, path, path_len * sizeof(int));
    strcpy(dummy_store[free_slot].val, value);
    dummy_store[free_slot].path_len = path_len;
    return 0;
}
