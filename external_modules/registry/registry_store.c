#include <string.h>
#include <errno.h>
#include "kernel_defines.h"
#include "clist.h"
#include "registry.h"
#include "assert.h"
#define ENABLE_DEBUG (0)
#include "debug.h"

static registry_store_t *save_dst;
static clist_node_t load_srcs;

static void _debug_print_path(const registry_path_t path)
{
    DEBUG("%d", path.root_group);

    if (path.schema_id != NULL) {
        DEBUG("/%d", path.schema_id);

        if (path.instance_id != NULL) {
            DEBUG("/%d", path.instance_id);

            if (path.path_len > 0) {
                DEBUG("/");

                for (int i = 0; i < path.path_len; i++) {
                    DEBUG("%d", path.path[i]);

                    if (i < path.path_len - 1) {
                        DEBUG("/");
                    }
                }
            }
        }
    }
}

static void _registry_store_load_cb(const registry_path_t path, void *val, int val_len,
                                    void *cb_arg)
{
    (void)cb_arg;
    DEBUG("[registry_store] Setting ");
    _debug_print_path(path);
    // TODO DEBUG(" to %s\n", val);

    registry_set_value(path, val, val_len);
}

void registry_store_init(void)
{
    load_srcs.next = NULL;
}

void registry_store_register_src(registry_store_t *src)
{
    assert(src != NULL);
    clist_rpush(&load_srcs, &(src->node));
}

void registry_store_register_dst(registry_store_t *dst)
{
    assert(dst != NULL);
    save_dst = dst;
}

int registry_store_load(void)
{
    clist_node_t *node = load_srcs.next;

    if (!node) {
        return -ENOENT;
    }

    do {
        registry_store_t *src;
        src = container_of(node, registry_store_t, node);
        src->itf->load(src, _registry_store_load_cb, NULL);
    } while (node != load_srcs.next);
    return 0;
}

static void _registry_store_dup_check_cb(const registry_path_t path, void *val, int val_len,
                                         void *cb_arg)
{
    assert(cb_arg != NULL);
    registry_dup_check_arg_t *dup_arg = (registry_dup_check_arg_t *)cb_arg;

    if (path.root_group != dup_arg->path.root_group || path.schema_id != dup_arg->path.schema_id ||
        path.instance_id != dup_arg->path.instance_id) {
        return;
    }

    for (int i = 0; i < path.path_len; i++) {
        if (path.path[i] != dup_arg->path.path[i]) {
            return;
        }
    }

    if (memcmp(val, dup_arg->val.buf, val_len) == 0) {
        dup_arg->is_dup = true;
    }
}

static int _registry_store_save_one_export_func(const registry_path_t path,
                                                const registry_schema_t *schema,
                                                const registry_instance_t *instance,
                                                const registry_schema_item_t *meta,
                                                const registry_value_t *value, void *context)
{
    (void)context;
    (void)schema;
    (void)meta;
    (void)instance;
    registry_store_t *dst = save_dst;

    DEBUG("[registry_store] Saving: ");
    _debug_print_path(path);
    // TODO DEBUG(" = %s\n", value);

    if (!dst) {
        return -ENOENT;
    }

    registry_dup_check_arg_t dup = {
        .path = path,
        .val = *value,
        .is_dup = false,
    };

    save_dst->itf->load(save_dst, _registry_store_dup_check_cb, &dup);

    if (dup.is_dup) {
        return -EEXIST;
    }

    return dst->itf->save(dst, path, *value);
}

int registry_store_save_one(const registry_path_t path, void *context)
{
    (void)context;

    char buf[REGISTRY_MAX_VAL_LEN];
    registry_value_t value = {
        .buf = buf,
        .buf_len = ARRAY_SIZE(buf),
    };

    registry_get_value(path, &value);

    return _registry_store_save_one_export_func(path, NULL, NULL, NULL, &value, context);
}

static int _registry_store_save_internal(clist_node_t schemas, registry_root_group_t root_group)
{
    registry_schema_t *schema;
    clist_node_t *node = schemas.next;
    int res = 0;
    int res2;

    if (!node) {
        return -1;
    }

    if (!save_dst) {
        return -ENOENT;
    }

    if (save_dst->itf->save_start) {
        save_dst->itf->save_start(save_dst);
    }

    do {
        schema = container_of(node, registry_schema_t, node);

        registry_path_t path = {
            .root_group = root_group,
            .schema_id = schema->id,
            .instance_id = NULL,
            .path = NULL,
            .path_len = 0,
        };

        res2 = registry_export(_registry_store_save_one_export_func, path, 0, NULL);
        if (res == 0) {
            res = res2;
        }
    } while (node != registry_schemas_sys.next);

    if (save_dst->itf->save_end) {
        save_dst->itf->save_end(save_dst);
    }

    return res;
}

int registry_store_save(void)
{
    registry_schema_t *schema;
    clist_node_t *node = registry_schemas_sys.next;
    int res = 0;
    int res2;

    res = _registry_store_save_internal(registry_schemas_sys, REGISTRY_ROOT_GROUP_SYS);
    res2 = _registry_store_save_internal(registry_schemas_app, REGISTRY_ROOT_GROUP_APP);

    if (res == 0) {
        res = res2;
    }

    return res;
}
