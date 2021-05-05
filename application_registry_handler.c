#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "registry.h"

#include "application_registry_handler.h"

#define MAX_THRESHOLD (500)

/*
This is a conceptual example of a "my_handler" Registry Handler that exposes `threshold` and `is_enabled` parameters.
For education purposes, it will be assumed the application crashes if a `set_threshold` function is called
when `is_enabled==false`.

Note the Registry Handler is not aware of any storage mechanism.
*/

/* Define a registry handler for the current RIOT module.
   To be registered in the RIOT Registry */
registry_handler_t my_handler = {
        .name = "my_handler",
        .hndlr_get = my_get_handler,
        .hndlr_set = my_set_handler,
        .hndlr_commit = my_commit_handler,
        .hndlr_export = my_export_handler
};

/* These are the state variables and parameters defined for this module */
static int is_enabled = false;
static int threshold = 0;

/* ... */

/* Dummy implementation of `get` handler.
   For both configuration parameters, it copies the value to a `val` variable.
*/
char *my_get_handler(int argc, char **argv, char *val, int val_len_max, void *context) {
    (void) val_len_max;
    (void) context;

    if (argc) {
        if (!strcmp("is_enabled", argv[0])) {
            /* Copy the value of `is_enabled` to `val` so the user can read it */
            memcpy(val, &is_enabled, sizeof(is_enabled));

            return val;
        } else if (!strcmp("threshold", argv[0])) {
            /* Copy the value of `threshold` to `val` so the user can read it */
            memcpy(val, &threshold, sizeof(threshold));

            return val;
        }
    }

    return NULL;
}

/* Dummy implementation of `set` handler.
   For both configuration parameters, it sets the value from `val`.
*/
int my_set_handler(int argc, char **argv, char *val, void *context) {
    (void) context;

    if (argc) {
        if (!strcmp("is_enabled", argv[0])) {
            /* Set the value of `is_enabled` from `val` */
            memcpy(&is_enabled, val, sizeof(is_enabled));
        } else if (!strcmp("threshold", argv[0])) {
            /* Validate threshold */
            if (atoi(val) > MAX_THRESHOLD)
                return -EINVAL;

            /* Set the value of `threshold` from `val` */
            memcpy(&threshold, val, sizeof(threshold));
        }
    }

    return 0;
}

/* Dummy implementation of `commit` handler.
   This is intended to be called by the Registry when all configurations have
   to be applied. Because of this, it's possible to implement transactions or
   protect against faulty combinations of configs, race conditions, etc.
*/
int my_commit_handler(void *context) {
    (void) context;

    /* Do something if the module is enable */
    if (is_enabled) {
        // trigger_something();
    }

    /* As stated before, the application crashes if `set_threshold` is called when is_enabled is false.
    We protect it here */
    if (is_enabled) {
        /* We can safely set the threshold without crashing the app */
        // set_threshold(threshold);
    }

    return 0;
}

/* Dummy implementation of `export` handler.
   This handler has to call `export_func` for one or all parameters.
   There can be different behaviors depending on the export function (e.g printing all configs
   to STDOUT, save them in a non-volatile storage device, etc)
*/
int my_export_handler(int (*export_func)(const char *name, char *val), int argc, char **argv, void *context) {
    /* argc  and argv can be used to export only one parameter */
    (void) argv;
    (void) argc;
    (void) context;
    char buf[sizeof(is_enabled)];

    /* We export every parameter with the export function */

    /* Prepare `buf` to contain is_enabled in a string representation */
    memcpy(buf, &is_enabled, sizeof(is_enabled));

    export_func("my_handler/is_enabled", buf);

    /* Prepare `buf` to contain threshold in a string representation */
    memcpy(buf, &threshold, sizeof(threshold));

    export_func("my_handler/threshold", buf);

    return 0;
}
