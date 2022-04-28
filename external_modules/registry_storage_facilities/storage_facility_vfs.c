#include "registry_storage_facilities.h"

#include <string.h>
#include <stdio.h>
#include <kernel_defines.h>
#include "errno.h"
#include "vfs.h"
#include <fcntl.h>
#define ENABLE_DEBUG (0)
#include "debug.h"

static int load(registry_store_instance_t *store, load_cb_t cb,
                void *cb_arg);
static int save(registry_store_instance_t *store, const registry_path_t path,
                const registry_value_t value);

registry_store_t registry_store_vfs = {
    .load = load,
    .save = save,
};

static int _format(vfs_mount_t *mount)
{
    printf("formating %s....\t", mount->mount_point);
    if (vfs_format(mount) < 0) {
        puts("[Failed]");
        return 1;
    }
    else {
        puts("[OK]");
    }

    return 0;
}

static int _mount(vfs_mount_t *mount)
{
    int res = vfs_mount(mount);

    if (res < 0) {
        printf("Error while mounting %s ... (%d) ... try format\n", mount->mount_point, res);

        /* format to fix mount */
        _format(mount);

        /* try to mount again */
        res = vfs_mount(mount);
        if (res != 0) {
            return -1;
        }
    }

    printf("%s successfully mounted\n", mount->mount_point);
    return 0;
}

static int _umount(vfs_mount_t *mount)
{
    int res = vfs_umount(mount);

    if (res < 0) {
        printf("Error while unmounting %s...\n", mount->mount_point);
        return 1;
    }

    printf("%s successfully unmounted\n", mount->mount_point);
    return 0;
}

static int load(registry_store_instance_t *store, load_cb_t cb,
                void *cb_arg)
{
    (void)cb;
    (void)cb_arg;

    vfs_mount_t *mount = store->data;

    /* mount */
    _mount(mount);

    /* save data */

    vfs_DIR dirp;

    if (vfs_opendir(&dirp, "/sda/dgdg/jsfklsdjfk/lksjdfkl/lsjdkfl/") != 0) {
        printf("CAN NOT OPEN DIR\n");
    }

    vfs_dirent_t entry;
    int res = 0;

    do {
        res = vfs_readdir(&dirp, &entry);
        printf("PATH: %s\n", entry.d_name);
    } while (res == 1);

    if (vfs_closedir(&dirp) != 0) {
        printf("CAN NOT CLOSE DIR\n");
    }

    // int fd = vfs_open(string_path, O_RDONLY, 0);

    // if (fd <= 0) {
    //     printf("[registry storace_facility_vfs] save: Can not open file\n");
    // }

    // uint8_t read_val_buf[REGISTRY_MAX_VAL_LEN] = { 0 };

    // if (vfs_read(fd, read_val_buf, value.buf_len) < 0) {
    //     printf("[registry storace_facility_vfs] save: Can not read from file\n");
    // }
    // else {
    //     printf("LOOOOOOOOOL: %d\n", *read_val_buf);
    // }

    // if (vfs_close(fd) != 0) {
    //     printf("[registry storace_facility_vfs] save: Can not close file\n");
    // }

    /* umount */
    _umount(mount);

    printf("\n\n");

    return 0;
}

static int save(registry_store_instance_t *store, const registry_path_t path,
                const registry_value_t value)
{
    (void)path;
    (void)value;

    vfs_mount_t *mount = store->data;

    /* mount */
    _mount(mount);

    vfs_mkdir("/0", O_CREAT);
    vfs_mkdir("/asdf", O_CREAT);

    /* save data */

    char string_path[REGISTRY_MAX_DIR_NAME_LEN];

    sprintf(string_path, "%s/%d/%d/%d", mount->mount_point, *path.root_group_id, *path.schema_id,
            *path.instance_id);

    for (int i = 0; i < path.path_len; i++) {
        sprintf(string_path, "%s/%d", string_path, path.path[i]);
    }

    int fd = vfs_open(string_path, O_CREAT | O_RDWR, 0);

    if (fd <= 0) {
        DEBUG("[registry storace_facility_vfs] save: Can not open file\n");
    }

    if (vfs_write(fd, value.buf, value.buf_len) < 0) {
        DEBUG("[registry storace_facility_vfs] save: Can not write to file\n");
    }

    if (vfs_close(fd) != 0) {
        DEBUG("[registry storace_facility_vfs] save: Can not close file\n");
    }

    /* umount */
    _umount(mount);

    return 0;
}
