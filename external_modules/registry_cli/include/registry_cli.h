#ifndef REGISTRY_CLI_H
#define REGISTRY_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

extern void registry_cli_init(void);
extern int registry_cli_cmd(int argc, char **argv);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* REGISTRY_CLI_H */
