/*
 * Copyright (C) 2023 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry_cli RIOT Registry Storage Facilities: VFS
 * @ingroup     sys
 * @brief       RIOT Registry VFS Storage Facility allows using the RIOT VFS module as a RIOT Registry data store.
 * @{
 *
 * @file
 *
 * @author      Lasse Rosenow <lasse.rosenow@haw-hamburg.de>
 */

#include "registry_storage_facilities.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <kernel_defines.h>
#include "errno.h"
#include "vfs.h"
#include <fcntl.h>
#define ENABLE_DEBUG (0)
#include "debug.h"
#include "ps.h"

static int load(const registry_store_instance_t *store, const registry_path_t path,
                const load_cb_t cb, const void *cb_arg);
static int save(const registry_store_instance_t *store, const registry_path_t path,
                const registry_value_t value);

registry_store_t registry_store_vfs = {
    .load = load,
    .save = save,
};

static void _string_path_append_item(char *dest, registry_path_item_t number)
{
    int size = snprintf(NULL, 0, "/%d", number);

    char buf[size];

    sprintf(buf, "/%d", number);

    strcat(dest, buf);
}

static int _parse_string_path(char *path, registry_path_item_t *buf, size_t *buf_len)
{
    size_t buf_index = 0;
    char curr_path_segment[REGISTRY_MAX_DIR_NAME_LEN] = { 0 };
    size_t curr_path_segment_index = 0;

    size_t path_len = strlen(path);

    size_t i = 0;

    if (path[0] == REGISTRY_NAME_SEPARATOR) {
        i = 1;
    }

    for (; i <= path_len; i++) {
        if (path[i] == REGISTRY_NAME_SEPARATOR || i == path_len) {
            buf[buf_index++] = atoi(curr_path_segment);
            curr_path_segment_index = 0;
        }
        else {
            if (!isdigit(path[i])) {
                return -EINVAL;
            }
            curr_path_segment[curr_path_segment_index++] = path[i];
            curr_path_segment[curr_path_segment_index] = '\0';
        }
    }

    *buf_len = buf_index;
    return 0;
}

static int _format(vfs_mount_t *mount)
{
    DEBUG("formatting %s....\t", mount->mount_point);
    if (vfs_format(mount) < 0) {
        DEBUG("[Failed]\n");
        return 1;
    }
    else {
        DEBUG("[OK]\n");
    }

    return 0;
}

static int _mount(vfs_mount_t *mount)
{
    int res = vfs_mount(mount);

    if (res < 0) {
        DEBUG("Error while mounting %s ... (%d) ... try format\n", mount->mount_point, res);

        /* format to fix mount */
        _format(mount);

        /* try to mount again */
        res = vfs_mount(mount);
        if (res != 0) {
            return -1;
        }
    }

    return 0;
}

static int _umount(vfs_mount_t *mount)
{
    int res = vfs_umount(mount);

    if (res < 0) {
        DEBUG("Error while unmounting %s...\n", mount->mount_point);
        return 1;
    }

    return 0;
}

static int load(const registry_store_instance_t *store, const registry_path_t path,
                const load_cb_t cb, const void *cb_arg)
{
    (void)cb;
    (void)cb_arg;
    (void)store;

    vfs_mount_t *mount = store->data;

    /* mount */
    _mount(mount);

    /* create dir path */
    char string_path[REGISTRY_MAX_DIR_LEN];

    sprintf(string_path, "%s", mount->mount_point);

    if (path.namespace_id != NULL) {
        _string_path_append_item(string_path, *path.namespace_id);
    }

    if (path.schema_id != NULL) {
        _string_path_append_item(string_path, *path.schema_id);
    }

    if (path.instance_id != NULL) {
        _string_path_append_item(string_path, *path.instance_id);
    }

    /* read dirs */
    vfs_DIR dirp;

    if (vfs_opendir(&dirp, string_path) != 0) {
        DEBUG("[registry storage_facility_vfs] load: Can not open dir\n");
    }
    else {
        struct stat _stat;
        vfs_dirent_t dir_entry;

        size_t i = 0;
        int last_dir_entry_positions[REGISTRY_MAX_DIR_DEPTH] = { -1 };
        size_t last_dir_string_path_lens[REGISTRY_MAX_DIR_DEPTH] = { 0 };
        int res = 0;
        bool exit_folder_iteration = false;

        while (exit_folder_iteration == false) {
            int dir_entry_position = -1;
            do {
                res = vfs_readdir(&dirp, &dir_entry);
                dir_entry_position++;

                if (dir_entry_position > last_dir_entry_positions[i]) {
                    last_dir_entry_positions[i] = dir_entry_position;
                    for (size_t j = i + 1; j < REGISTRY_MAX_DIR_DEPTH; j++) {
                        last_dir_entry_positions[j] = -1;
                    }

                    if (res == 1) {
                        if (strcmp(dir_entry.d_name,
                                   ".") != 0 && strcmp(dir_entry.d_name, "..") != 0) {
                            /* save string_path length to restore it later */
                            last_dir_string_path_lens[i] = strlen(string_path);

                            /* add new directory to string_path */
                            strcat(string_path, "/");
                            strcat(string_path, dir_entry.d_name);

                            vfs_stat(string_path, &_stat);

                            if (S_ISDIR(_stat.st_mode)) {
                                /* close old directory */
                                if (vfs_closedir(&dirp) != 0) {
                                    DEBUG(
                                        "[registry storage_facility_vfs] load: Can not close dir\n");
                                }

                                /* open new directory */
                                if (vfs_opendir(&dirp, string_path) != 0) {
                                    DEBUG("[registry storage_facility_vfs] load: Can not open dir\n");
                                }

                                /* move on to next sub path */
                                i++;

                                /* reset position within current dir, because the dir changed */
                                dir_entry_position = -1;
                            }
                            else {
                                /* open file */
                                int fd = vfs_open(string_path, O_RDONLY, 0);

                                if (fd <= 0) {
                                    DEBUG(
                                        "[registry storage_facility_vfs] load: Can not open file: %d\n",
                                        fd);
                                }

                                /* get filesize */
                                vfs_fstat(fd, &_stat);

                                /* try to convert string path to registry int path */
                                size_t path_items_len = REGISTRY_MAX_DIR_DEPTH + 3;
                                registry_path_item_t path_items[path_items_len];
                                if (_parse_string_path(string_path + strlen(mount->mount_point),
                                                       path_items,
                                                       &path_items_len) < 0) {
                                    DEBUG(
                                        "[registry storage_facility_vfs] load: Invalid registry path\n");
                                }
                                else {
                                    /* convert int path to registry_path_t */
                                    // TODO: Why is REGISTRY_PATH() Not working? (It should resolve to _REGISTRY_PATH_0()
                                    // but somehow its not initializing namespace with NULL?? (makes no sense:( ... )))
                                    registry_path_t path = _REGISTRY_PATH_0();
                                    for (size_t i = 0; i < path_items_len; i++) {
                                        switch (i) {
                                        case 0: path.namespace_id =
                                            (registry_namespace_id_t *)&path_items[i];
                                            break;
                                        case 1: path.schema_id = &path_items[i]; break;
                                        case 2: path.instance_id = &path_items[i]; break;
                                        case 3: path.path = &path_items[i]; path.path_len++; break; // Add path.path to correct position in path_items array
                                        default: path.path_len++; break;
                                        }
                                    }

                                    /* get registry meta data of configuration parameter */
                                    registry_value_t value;
                                    registry_get_value(path, &value);

                                    /* read value from file */
                                    uint8_t new_value_buf[value.buf_len];
                                    if (vfs_read(fd, new_value_buf, value.buf_len) < 0) {
                                        DEBUG(
                                            "[registry storage_facility_vfs] load: Can not read from file\n");
                                    }
                                    else {
                                        /* add read value to value */
                                        value.buf = new_value_buf;

                                        /* call callback with value and path */
                                        cb(path, value, cb_arg);
                                    }
                                }

                                /* close file */
                                if (vfs_close(fd) != 0) {
                                    DEBUG(
                                        "[registry storage_facility_vfs] load: Can not close file: %d\n",
                                        fd);
                                }

                                /* restore old string_path */
                                string_path[last_dir_string_path_lens[i]] = '\0';
                            }
                        }
                    }
                    else {
                        /* if i == 0 it can't be further decreased => exit */
                        if (i == 0) {
                            exit_folder_iteration = true;
                        }
                        else {
                            /* move up one path back to the parent */
                            i--;

                            /* restore old string_path */
                            string_path[last_dir_string_path_lens[i]] = '\0';

                            /* close old directory */
                            if (vfs_closedir(&dirp) != 0) {
                                DEBUG("[registry storage_facility_vfs] load: Can not close dir\n");
                            }

                            /* open new directory */
                            if (vfs_opendir(&dirp, string_path) != 0) {
                                DEBUG("[registry storage_facility_vfs] load: Can not open dir\n");
                            }
                        }
                    }
                }
            } while (res == 1);
        }

        if (vfs_closedir(&dirp) != 0) {
            DEBUG("[registry storage_facility_vfs] load: Can not close dir\n");
        }
    }

    /* umount */
    _umount(mount);

    return 0;
}

static int save(const registry_store_instance_t *store, const registry_path_t path,
                const registry_value_t value)
{
    (void)path;
    (void)value;

    vfs_mount_t *mount = store->data;

    /* mount */
    _mount(mount);

    /* create dir path */
    char string_path[REGISTRY_MAX_DIR_LEN];

    sprintf(string_path, "%s", mount->mount_point);

    _string_path_append_item(string_path, *path.namespace_id);
    int res = vfs_mkdir(string_path, 0);

    if (res < 0 && res != -EEXIST) {
        DEBUG("[registry storage_facility_vfs] save: Can not make dir: %s\n", string_path);
    }

    _string_path_append_item(string_path, *path.schema_id);
    res = vfs_mkdir(string_path, 0);

    if (res < 0 && res != -EEXIST) {
        DEBUG("[registry storage_facility_vfs] save: Can not make dir: %s\n", string_path);
    }

    _string_path_append_item(string_path, *path.instance_id);
    res = vfs_mkdir(string_path, 0);

    if (res < 0 && res != -EEXIST) {
        DEBUG("[registry storage_facility_vfs] save: Can not make dir: %s\n", string_path);
    }

    /* exclude the last element, as it will be the file name and not a folder */
    for (size_t i = 0; i < path.path_len - 1; i++) {
        _string_path_append_item(string_path, path.path[i]);
        res = vfs_mkdir(string_path, 0);
        if (res != 0 && res != -EEXIST) {
            DEBUG("[registry storage_facility_vfs] save: Can not create dir: %d\n", res);
        }
    }

    /* open file */
    _string_path_append_item(string_path, path.path[path.path_len - 1]);

    int fd = vfs_open(string_path, O_CREAT | O_RDWR, 0);

    if (fd <= 0) {
        DEBUG("[registry storage_facility_vfs] save: Can not open file: %d\n", fd);
    }

    if (vfs_write(fd, value.buf, value.buf_len) < 0) {
        DEBUG("[registry storage_facility_vfs] save: Can not write to file: %d\n", fd);
    }

    if (vfs_close(fd) != 0) {
        DEBUG("[registry storage_facility_vfs] save: Can not close file: %d\n", fd);
    }

    /* umount */
    _umount(mount);

    return 0;
}

/** @} */
