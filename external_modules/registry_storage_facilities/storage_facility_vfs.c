#include "registry_storage_facilities.h"

#include <string.h>
#include <stdio.h>
#include <kernel_defines.h>
#include "errno.h"
#include "vfs.h"

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

    //vfs_mount_t *vfs_mount = (vfs_mount_t *)store->data;

    /* mount */
    _mount(mount);

    /* load data */
    //int fd = vfs_open("/sda/test.txt", O_RDONLY, 0);

    /* umount */
    _umount(mount);

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

    /* save data */
    // vfs_DIR root_group_dir;

    // char root_group_string[REGISTRY_MAX_DIR_NAME_LEN];

    // sprintf(root_group_string, "%d", *path.root_group_id);

    // if (vfs_opendir(&root_group_dir, root_group_string) != 0) {
    //     printf("CAN NOT OPEN\n\n");
    //     /* vfs_mkdir(root_group_string)
    //        int fd = vfs_open("/sda/test.txt", O_CREAT | O_RDWR, 0); */
    // }


    // vfs_write(fd, buf, sizeof(buf));

    // vfs_close(fd);

    /* umount */
    _umount(mount);

    return 0;
}
