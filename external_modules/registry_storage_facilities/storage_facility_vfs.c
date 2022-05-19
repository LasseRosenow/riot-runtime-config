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

static int load(registry_store_instance_t *store, const registry_path_t path, load_cb_t cb,
                void *cb_arg);
static int save(registry_store_instance_t *store, const registry_path_t path,
                const registry_value_t value);

registry_store_t registry_store_vfs = {
    .load = load,
    .save = save,
};

static int _parse_string_path(char *path, int *buf, int *buf_len)
{
    int buf_index = 0;
    char curr_path_segment[REGISTRY_MAX_DIR_NAME_LEN] = { 0 };
    int curr_path_segment_index = 0;

    int path_len = strlen(path);

    int i = 0;

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
    DEBUG("formating %s....\t", mount->mount_point);
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
    //printf("PS - %s: %d\n", __FILE__, __LINE__); ps();
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
    //printf("PS - %s: %d\n", __FILE__, __LINE__); ps();

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

static int _load_recursive(vfs_DIR *dirp, vfs_dirent_t *dir_entry, struct stat *_stat,
                           const char *mount_point, uint8_t *value_buf,
                           char *string_path, load_cb_t cb, void *cb_arg)
{
    int i = 0;
    int last_dir_entry_positions[REGISTRY_MAX_DIR_DEPTH] = { -1 };
    int last_dir_string_path_lens[REGISTRY_MAX_DIR_DEPTH] = { 0 };
    int res = 0;

    while (true) {
        int dir_entry_position = -1;
        do {
            res = vfs_readdir(dirp, dir_entry);
            dir_entry_position++;

            if (dir_entry_position > last_dir_entry_positions[i]) {
                last_dir_entry_positions[i] = dir_entry_position;
                for (int j = i + 1; j < REGISTRY_MAX_DIR_DEPTH; j++) {
                    last_dir_entry_positions[j] = -1;
                }

                if (res == 1) {
                    if (strcmp(dir_entry->d_name,
                               ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
                        /* save string_path length to restore it later */
                        last_dir_string_path_lens[i] = strlen(string_path);

                        /* add new directory to string_path */
                        sprintf(string_path, "%s/%s", string_path, dir_entry->d_name);

                        vfs_stat(string_path, _stat);

                        if (S_ISDIR(_stat->st_mode)) {
                            /* close old directory */
                            if (vfs_closedir(dirp) != 0) {
                                DEBUG("[registry storage_facility_vfs] load: Can not close dir\n");
                            }

                            dir_entry_position = -1;

                            /* open new directory */
                            if (vfs_opendir(dirp, string_path) != 0) {
                                DEBUG("[registry storage_facility_vfs] load: Can not open dir\n");
                            }

                            /* move on to next sub path */
                            i++;
                        }
                        else {
                            /* open file */
                            int fd = vfs_open(string_path, O_RDONLY, 0);

                            if (fd <= 0) {
                                DEBUG(
                                    "[registry storage_facility_vfs] load: Can not open file: %d\n",
                                    fd);
                            }

                            /* read value from file */
                            if (vfs_read(fd, value_buf, sizeof(uint8_t)) < 0) {
                                DEBUG(
                                    "[registry storage_facility_vfs] load: Can not read from file\n");
                            }
                            else {
                                int int_path_len = REGISTRY_MAX_DIR_DEPTH + 3;
                                int int_path[int_path_len];

                                /* try to convert string path to registry int path */
                                if (_parse_string_path(string_path + strlen(mount_point), int_path,
                                                       &int_path_len) < 0) {
                                    DEBUG(
                                        "[registry storage_facility_vfs] load: Invalid registry path\n");
                                }
                                else {
                                    /* convert int path to registry_path_t */
                                    // TODO: Why is REGISTRY_PATH() Not working? (It should resolve to _REGISTRY_PATH_0()
                                    // but somehow its not initializing root group with NULL?? (makes no sense:( ... )))
                                    registry_path_t path = _REGISTRY_PATH_0();
                                    for (int i = 0; i < int_path_len; i++) {
                                        switch (i) {
                                        case 0: path.root_group_id =
                                            (registry_root_group_id_t *)&int_path[i];
                                            break;
                                        case 1: path.schema_id = &int_path[i]; break;
                                        case 2: path.instance_id = &int_path[i]; break;
                                        case 3: path.path = &int_path[i]; path.path_len++; break; // Add path.path to correct position in int_path array
                                        default: path.path_len++; break;
                                        }
                                    }

                                    /* get registry meta data of configuration parameter */
                                    static char buf[REGISTRY_MAX_VAL_LEN];
                                    registry_value_t value = {
                                        .buf = buf,
                                        .buf_len = ARRAY_SIZE(buf),
                                    };
                                    registry_get_value(path, &value);

                                    /* add read value to value */
                                    value.buf = value_buf;

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
                        return 0;
                    }

                    /* move up one path back to the parent */
                    i--;

                    /* restore old string_path */
                    string_path[last_dir_string_path_lens[i]] = '\0';

                    /* close old directory */
                    if (vfs_closedir(dirp) != 0) {
                        DEBUG("[registry storage_facility_vfs] load: Can not close dir\n");
                    }

                    /* open new directory */
                    if (vfs_opendir(dirp, string_path) != 0) {
                        DEBUG("[registry storage_facility_vfs] load: Can not open dir\n");
                    }
                }
            }
        } while (res == 1);
    }



    return 0;
}

static int load(registry_store_instance_t *store, const registry_path_t path, load_cb_t cb,
                void *cb_arg)
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

    if (path.root_group_id != NULL) {
        sprintf(string_path, "%s/%d", string_path, *path.root_group_id);
    }

    if (path.schema_id != NULL) {
        sprintf(string_path, "%s/%d", string_path, *path.schema_id);
    }

    if (path.instance_id != NULL) {
        sprintf(string_path, "%s/%d", string_path, *path.instance_id);
    }

    /* read dirs */
    vfs_DIR dirp;

    if (vfs_opendir(&dirp, string_path) != 0) {
        DEBUG("[registry storage_facility_vfs] load: Can not open dir\n");
    }

    struct stat _stat;
    vfs_dirent_t dir_entry;
    uint8_t value_buf[REGISTRY_MAX_VAL_LEN] = { 0 };

    _load_recursive(&dirp, &dir_entry, &_stat, mount->mount_point, value_buf, string_path, cb,
                    cb_arg);

    if (vfs_closedir(&dirp) != 0) {
        DEBUG("[registry storage_facility_vfs] load: Can not close dir\n");
    }

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

    /* create dir path */
    char string_path[REGISTRY_MAX_DIR_LEN];

    sprintf(string_path, "%s/%d", mount->mount_point, *path.root_group_id);
    vfs_mkdir(string_path, 0);

    sprintf(string_path, "%s/%d", string_path, *path.schema_id);
    vfs_mkdir(string_path, 0);

    sprintf(string_path, "%s/%d", string_path, *path.instance_id);
    vfs_mkdir(string_path, 0);

    /* exclude the last element, as it will be the file name and not a folder */
    for (int i = 0; i < path.path_len - 1; i++) {
        sprintf(string_path, "%s/%d", string_path, path.path[i]);
        int res = vfs_mkdir(string_path, 0);
        if (res != 0 && res != -EEXIST) {
            DEBUG("[registry storage_facility_vfs] save: Can not create dir: %d\n", res);
        }
    }

    /* open file */
    sprintf(string_path, "%s/%d", string_path, path.path[path.path_len - 1]);

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
