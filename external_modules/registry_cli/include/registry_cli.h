/*
 * Copyright (C) 2023 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry_cli RIOT Registry CLI
 * @ingroup     sys
 * @brief       RIOT Registry module providing a CLI for the RIOT Registry sys module
 * @{
 *
 * @file
 *
 * @author      Lasse Rosenow <lasse.rosenow@haw-hamburg.de>
 */

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
