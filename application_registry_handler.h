#ifndef RUNTIME_CONFIG_REGISTRY_HANDLER_H
#define RUNTIME_CONFIG_REGISTRY_HANDLER_H

extern registry_handler_t my_handler;

char *my_get_handler(int argc, char **argv, char *val, int val_len_max, void *context);
int my_set_handler(int argc, char **argv, char *val, void *context);
int my_commit_handler(void *context);
int my_export_handler(int (*export_func)(const char *name, char *val, void *context), int argc, char **argv, void *context);

#endif //RUNTIME_CONFIG_REGISTRY_HANDLER_H
