#ifndef RUNTIME_CONFIG_REGISTRY_HANDLER_H
#define RUNTIME_CONFIG_REGISTRY_HANDLER_H

int my_get_handler(int argc, char **argv, char *val, int val_len_max);
int my_set_handler(int argc, char **argv, char *val);
int my_commit_handler(int argc, char **argv, char *val);
void my_export_handler(int (*export_func)(const char *name, char *val), int argc, char **argv);

#endif //RUNTIME_CONFIG_REGISTRY_HANDLER_H
