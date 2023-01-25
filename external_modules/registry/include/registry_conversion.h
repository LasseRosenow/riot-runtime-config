/*
 * Copyright (C) 2023 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry RIOT Registry
 * @ingroup     sys
 * @brief       RIOT Registry module for handling runtime configurations
 * @{
 *
 * @file
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @author      Lasse Rosenow <lasse.rosenow@haw-hamburg.de>
 */

#ifndef REGISTRY_REGISTRY_CONVERSION_H
#define REGISTRY_REGISTRY_CONVERSION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convenience function to parse a configuration parameter value from
 * a string. The type of the parameter must be known and must not be `bytes`.
 * To parse the string to bytes @ref registry_bytes_from_str() function must be
 * used.
 *
 * @param[in] src Pointer of the input value
 * @param[out] dest Pointer to the output buffer
 * @param[in] dest_len Length of @p dest
 * @param[in] dest_type Type of the output value
 * @return 0 on success, non-zero on failure
 */
int registry_convert_str_to_value(const char *src, void *dest, const size_t dest_len,
                                  const registry_type_t dest_type);

/**
 * @brief Convenience function to parse a configuration parameter value from
 * another value. The type of the parameter must be known.
 *
 * @param[in] src Pointer of the input value
 * @param[out] dest Pointer to the output buffer
 * @param[in] dest_len Length of @p dest
 * @param[in] dest_type Type of the output value
 * @return 0 on success, non-zero on failure
 */
int registry_convert_value_to_value(const registry_value_t *src, void *dest,
                                    const size_t dest_len, const registry_type_t dest_type);

/**
 * @brief Convenience function to parse a configuration parameter value of
 * `bytes` type from a string.
 *
 * @param[in] src Pointer of the string containing the value
 * @param[out] dest Pointer to store the parsed value
 * @param[in out] len Length of the output buffer
 * @return 0 on success, non-zero on failure
 */
int registry_convert_str_to_bytes(const char *src, void *dest, size_t *len);

/**
 * @brief Convenience function to transform a configuration parameter value into
 * a string, when the parameter is not of `bytes` type, in this case
 * @ref registry_str_from_bytes() should be used. This is used for example to
 * implement the `get` or `export` functions.
 *
 * @param[in] src Pointer to the value to be converted
 * @param[out] dest Buffer to store the output string
 * @param[in] dest_len Length of @p buf
 * @return Pointer to the output string
 */
char *registry_convert_value_to_str(const registry_value_t *src, char *dest,
                                    const size_t dest_len);

/**
 * @brief Convenience function to transform a configuration parameter value of
 * `bytes` type into a string. This is used for example to implement the `get`
 * or `export` schemas.
 *
 * @param[in] src Pointer to the value to be converted
 * @param[in] src_len Length of @p vp
 * @param[out] dest Buffer to store the output string
 * @param[in out] dest_len Length of @p buf
 * @return Pointer to the output string
 */
char *registry_convert_bytes_to_str(const void *src, const size_t src_len, char *dest,
                                    size_t *dest_len);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* REGISTRY_REGISTRY_CONVERSION_H */
