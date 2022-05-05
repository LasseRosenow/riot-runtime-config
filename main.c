#include <string.h>
#include <shell.h>
#include <msg.h>
#include <stdio.h>

#include "registry.h"
#include "registry_schemas.h"
#include "registry_cli.h"
#include "registry_tests.h"
#include "registry_storage_facilities.h"
#include "assert.h"
#include "fs/littlefs2_fs.h"
#include "vfs.h"
#include "board.h"
#include "mtd.h"

#define SHELL_QUEUE_SIZE (8)
static msg_t _shell_queue[SHELL_QUEUE_SIZE];

static const shell_command_t shell_commands[] = {
    //{ "registry_coap", "Registry CoAP cli", registry_coap_cli_cmd },
    { "registry", "Registry cli", registry_cli_cmd },
    // { "registry_lwm2m", "Registry LwM2M cli", registry_lwm2m_cli_cmd },
    { NULL, NULL, NULL }
};

int rgb_led_instance_0_commit_cb(const registry_path_t path, void *context)
{
    (void)context;
    printf("RGB instance commit_cb was executed: %d", *path.root_group_id);
    if (path.schema_id) {
        printf("/%d", *path.schema_id);
    }
    if (path.instance_id) {
        printf("/%d", *path.instance_id);
    }
    printf("\n");
    return 0;
}

registry_schema_rgb_led_t rgb_led_instance_0_data = {
    .red = 0,
    .green = 255,
    .blue = 70,
};
registry_instance_t rgb_led_instance_0 = {
    .name = "rgb-0",
    .data = &rgb_led_instance_0_data,
    .commit_cb = &rgb_led_instance_0_commit_cb,
};

registry_schema_rgb_led_t rgb_led_instance_1_data = {
    .red = 90,
    .green = 4,
    .blue = 0,
};
registry_instance_t rgb_led_instance_1 = {
    .name = "rgb-1",
    .data = &rgb_led_instance_1_data,
    .commit_cb = &rgb_led_instance_0_commit_cb,
};

registry_schema_rgb_led_t rgb_led_instance_2_data = {
    .red = 7,
    .green = 8,
    .blue = 9,
};
registry_instance_t rgb_led_instance_2 = {
    .name = "rgb-2",
    .data = &rgb_led_instance_2_data,
    .commit_cb = &rgb_led_instance_0_commit_cb,
};


// static struct spiffs_desc spiffs_desc = {
//     .lock = MUTEX_INIT,
// };

// static vfs_mount_t _vfs_mount = {
//     .fs = &spiffs_file_system,
//     .mount_point = "/sda",
//     .private_data = &spiffs_desc,
// };

static littlefs2_desc_t littlefs2_desc = {
    .lock = MUTEX_INIT,
};

static vfs_mount_t _vfs_mount = {
    .fs = &littlefs2_file_system,
    .mount_point = "/sda",
    .private_data = &littlefs2_desc,
};

registry_store_instance_t vfs_instance_1 = {
    .itf = &registry_store_vfs,
    .data = &_vfs_mount,
};

registry_store_instance_t vfs_instance_2 = {
    .itf = &registry_store_vfs,
    .data = &_vfs_mount,
};

int main(void)
{
// #if defined(MODULE_SPIFFS)
//     spiffs_desc.dev = MTD_0;
// #endif
// #if defined(MODULE_LITTLEFS2)
    littlefs2_desc.dev = MTD_0;
// #endif

    /* init registry */
    registry_init();
    registry_schemas_init();

    /* register store source and destination */
    registry_store_register_src(&vfs_instance_1);
    registry_store_register_dst(&vfs_instance_2);

    /* add schema instances */
    registry_add_instance(REGISTRY_ROOT_GROUP_SYS, registry_schema_rgb_led.id, &rgb_led_instance_0);
    registry_add_instance(REGISTRY_ROOT_GROUP_SYS, registry_schema_rgb_led.id, &rgb_led_instance_1);
    registry_add_instance(REGISTRY_ROOT_GROUP_SYS, registry_schema_rgb_led.id, &rgb_led_instance_2);

    /* for the thread running the shell */
    // registry_coap_cli_init();
    // registry_lwm2m_cli_init();

    // registry_get_bool(REGISTRY_PATH_SYS(REGISTRY_SCHEMA_RGB_LED, 0, REGISTRY_SCHEMA_RGB_LED_BLUE));

    // registry_store_save();
    registry_store_load();

    /* test registry */
    // registry_tests_run();

    msg_init_queue(_shell_queue, SHELL_QUEUE_SIZE);
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
