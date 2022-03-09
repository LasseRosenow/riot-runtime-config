/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_registry RIOT Registry
 * @ingroup     sys
 * @brief       RIOT Registry module for handling runtime configurations
 *
 * ## About
 *
 * The RIOT Registry is a module for interacting with persistent key-value
 * configurations. It forms part of the Runtime Configuration System for RIOT
 * nodes.
 *
 * ## Architecture
 *
 * The Registry interacts with other RIOT modules via
 * @ref registry_schema_t "Registry Schemas", and with non-volatile devices
 * via @ref sys_registry_store "Storage Facilities". This way the functionality
 * of the RIOT Registry is independent of the functionality of the module and
 * storage devices.
 *
 * ![RIOT Registry architecture](riot-registry-architecture.svg)
 *
 * ### Registry Schemas
 *
 * @ref registry_schema_t "Registry Schemas" (RS) represent Configuration
 * Groups in the RIOT Registry. A RIOT module is required to implement and
 * register a RS in order to expose its configurations in the Registry.
 *
 * A RS is defined by a @ref registry_schema_t::name "name" and a series of
 * schemas for interacting with the configuration parametes of the
 * configuration group. The schemas are:
 *
 * - @ref registry_schema_t::get "get"
 * - @ref registry_schema_t::set "set"
 * - @ref registry_schema_t::commit "commit"
 *
 * It is responsibility of the module to implement these schemas and perform
 * all necessary checks before applying values to the configuration parameters.
 *
 * ### Storage Facilities
 *
 * @ref sys_registry_store "Storage Facilities" (SF) implement the
 * @ref registry_store_itf_t "storage interface" to allow the RIOT Registry to
 * load, search and store configuration parameters.
 *
 * ## RIOT Registry usage flow
 * - 1 The RIOT registry is initialized with @ref registry_init().
 * - 2 Modules declare and register RHs for configuration groups by calling
 *     @ref registry_register_schema().
 * - 3 SFs are registered as sources and/or destinations of configurations by
 *     calling registry_<storage-name>_src() and registry_<storage-name>_dst().
 *
 * @{
 * @file
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 *
 * @}
 */

#ifndef REGISTRY_REGISTRY_H
#define REGISTRY_REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "clist.h"

/**
 * @brief Separator character to define hierarchy in configurations names.
 */
#define REGISTRY_NAME_SEPARATOR    '/'

/**
 * @brief Maximum amount of levels of hierarchy in configurations names.
 */
#define REGISTRY_MAX_DIR_DEPTH     8

/**
 * @brief Maximum amount of characters per level in configurations names.
 */
#define REGISTRY_MAX_DIR_NAME_LEN  64

/**
 * @brief Maximum length of a value when converted to string
 */
#define REGISTRY_MAX_VAL_LEN       64

/**
 * @brief Maximum length of a configuration name.
 * @{
 */
#define REGISTRY_MAX_NAME_LEN      ((REGISTRY_MAX_DIR_NAME_LEN * \
                                     REGISTRY_MAX_DIR_DEPTH) + \
                                    (REGISTRY_MAX_DIR_DEPTH - 1))
/** @} */

/**
 * @brief Calculates the size of an @ref registry_schema_item_t array.
 *
 */
#define REGISTRY_NUMARGS(...)  (sizeof((registry_schema_item_t[]){ __VA_ARGS__ }) / \
                                sizeof(registry_schema_item_t))

/**
 * @brief Creates and initializes a @ref registry_schema_t struct.
 *
 */
#define REGISTRY_SCHEMA(_field_name, _id, _name, _description, _get, _set, ...) \
    registry_schema_t _field_name = { \
        .id = _id, \
        .name = _name, \
        .description = _description, \
        .get = _get, \
        .set = _set, \
        .items = (registry_schema_item_t[]) { __VA_ARGS__ }, \
        .items_len = REGISTRY_NUMARGS(__VA_ARGS__), \
    }

/**
 * @brief Creates and initializes a @ref registry_schema_item_t struct and defaults its type to @ref REGISTRY_SCHEMA_TYPE_GROUP.
 *
 */
#define REGISTRY_GROUP(_id, _name, _description, ...) \
    { \
        .id = _id, \
        .name = _name, \
        .description = _description, \
        .type = REGISTRY_SCHEMA_TYPE_GROUP, \
        .value.group = { \
            .items = (registry_schema_item_t[]) { __VA_ARGS__ }, \
            .items_len = REGISTRY_NUMARGS(__VA_ARGS__), \
        }, \
    },

/**
 * @brief Creates and initializes a @ref registry_schema_item_t struct and defaults its type to @ref REGISTRY_SCHEMA_TYPE_PARAMETER.
 *
 */
#define REGISTRY_PARAMETER(_id, _name, _description, _type) \
    { \
        .id = _id, \
        .name = _name, \
        .description = _description, \
        .type = REGISTRY_SCHEMA_TYPE_PARAMETER, \
        .value.parameter = { \
            .type = _type, \
        }, \
    },

#if defined(CONFIG_REGISTRY_DISABLE_SCHEMA_NAME_FIELD) && \
    defined(CONFIG_REGISTRY_DISABLE_SCHEMA_DESCRIPTION_FIELD)
/* no name and no description */
# define _REGISTRY_PARAMETER(_id, _name, _description, _type) \
    REGISTRY_PARAMETER(_id, "", "", _type)
#elif defined(CONFIG_REGISTRY_DISABLE_SCHEMA_NAME_FIELD)
/* no name */
# define _REGISTRY_PARAMETER(_id, _name, _description, _type) \
    REGISTRY_PARAMETER(_id, "", _description, _type)
#elif defined(CONFIG_REGISTRY_DISABLE_SCHEMA_DESCRIPTION_FIELD)
/* no description */
# define _REGISTRY_PARAMETER(_id, _name, _description, _type) \
    REGISTRY_PARAMETER(_id, _name, "", _type)
#else
/* keep name and description */
# define _REGISTRY_PARAMETER(_id, _name, _description, _type) \
    REGISTRY_PARAMETER(_id, _name, _description, _type)
#endif

#define REGISTRY_PARAMETER_STRING(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_STRING)
#define REGISTRY_PARAMETER_BOOL(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_BOOL)
#define REGISTRY_PARAMETER_UINT8(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_UINT8)
#define REGISTRY_PARAMETER_UINT16(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_UINT16)
#define REGISTRY_PARAMETER_UINT32(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_UINT32)

#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
# define REGISTRY_PARAMETER_UINT64(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_UINT64)
#else
# define REGISTRY_PARAMETER_UINT64(_id, _name, _description)
#endif /* CONFIG_REGISTRY_USE_UINT64 */

#define REGISTRY_PARAMETER_INT8(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_INT8)
#define REGISTRY_PARAMETER_INT16(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_INT16)
#define REGISTRY_PARAMETER_INT32(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_INT32)

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
# define REGISTRY_PARAMETER_INT64(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_INT64)
#else
# define REGISTRY_PARAMETER_INT64(_id, _name, _description)
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
# define REGISTRY_PARAMETER_FLOAT32(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_FLOAT32)
#else
# define REGISTRY_PARAMETER_FLOAT32(_id, _name, _description)
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
# define REGISTRY_PARAMETER_FLOAT64(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_FLOAT64)
#else
# define REGISTRY_PARAMETER_FLOAT64(_id, _name, _description)
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

/**
 * @brief Data types of the registry
 *
 * @note Float and int64 types must be enabled by defining
 * `CONFIG_REGISTRY_USE_FLOAT` and `CONFIG_REGISTRY_USE_INT64`. Use with caution
 * as they bloat the code size.
 */
typedef enum {
    REGISTRY_TYPE_NONE = 0,     /**< No type specified */
    REGISTRY_TYPE_STRING,       /**< String */
    REGISTRY_TYPE_BOOL,         /**< Boolean */

    REGISTRY_TYPE_UINT8,        /**< 8-bits unsigned integer */
    REGISTRY_TYPE_UINT16,       /**< 16-bits unsigned integer */
    REGISTRY_TYPE_UINT32,       /**< 32-bits unsigned integer */

#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
    REGISTRY_TYPE_UINT64,     /**< 64-bits unsigned integer */
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    REGISTRY_TYPE_INT8,         /**< 8-bits signed integer */
    REGISTRY_TYPE_INT16,        /**< 16-bits signed integer */
    REGISTRY_TYPE_INT32,        /**< 32-bits signed integer */

#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
    REGISTRY_TYPE_INT64,     /**< 64-bits signed integer */
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
    REGISTRY_TYPE_FLOAT32,     /**< 32-bits float */
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
    REGISTRY_TYPE_FLOAT64,     /**< 64-bits float */
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
} registry_type_t;

/**
 * @brief Basic representation of a registry parameter, containing information about its type and its value.
 */
typedef struct {
    registry_type_t type;   /**< The type of the parameter */
    void *buf;              /**< Pointer to the buffer containing the value of the parameter */
    int buf_len;            /**< Length of the buffer */
} registry_value_t;

/**
 * @brief Parameter of a configuration group.
 */
typedef struct {
    registry_type_t type; /**< Enum representing the type of the configuration parameter */
} registry_schema_parameter_t;

typedef struct _registry_schema_item_t registry_schema_item_t;

/**
 * @brief Configuration group.
 */
typedef struct {
    registry_schema_item_t *items;
    int items_len;
} registry_schema_group_t;

typedef enum {
    REGISTRY_SCHEMA_TYPE_GROUP,
    REGISTRY_SCHEMA_TYPE_PARAMETER,
} registry_schema_type_t;

struct _registry_schema_item_t {
    int id;                                     /**< Integer representing the path id of the schema item */
    char *name;                                 /**< String describing the schema item */
    char *description;                          /**< String describing the schema item with more details */
    registry_schema_type_t type;                /**< Type of the schema item (group or parameter) */
    union {
        registry_schema_group_t group;          /**< Value of the schema item if it is a group. Contains an array of schema item children */
        registry_schema_parameter_t parameter;  /**< Value of the schema item if it is a parameter. Contains its type */
    } value;                                    /**< Union containing either group or parameter data */
};

/**
 * @brief Prototype of a callback function for the load action of a store
 * interface
 */
typedef void (*load_cb_t)(const int *path, int path_len, void *val, int val_len, void *cb_arg);

/**
 * @brief Descriptor used to check duplications in store facilities
 */
typedef struct {
    const int *path;        /**< path of the parameter being checked */
    registry_value_t val;   /**< value of the parameter being checked */
    bool is_dup;            /**< flag indicating if the parameter is duplicated or not */
} registry_dup_check_arg_t;

/**
 * @brief Store facility descriptor
 */
typedef struct {
    clist_node_t node;                      /**< linked list node */
    const struct registry_store_itf *itf;   /**< interface for the facility */
} registry_store_t;

/**
 * @brief Storage facility interface.
 * All store facilities should, at least, implement the load and save actions.
 */
typedef struct registry_store_itf {
    /**
     * @brief Loads all stored parameters and calls the @p cb callback function.
     *
     * @param[in] store Storage facility descriptor
     * @param[in] cb Callback function to call for every stored parameter
     * @param[in] cb_arg Argument passed to @p cb function
     * @return 0 on success, non-zero on failure
     */
    int (*load)(registry_store_t *store, load_cb_t cb, void *cb_arg);

    /**
     * @brief If implemented, it is used for any preparation the storage may
     * need before starting a saving process.
     *
     * @param[in] store Storage facility descriptor
     * @return 0 on success, non-zero on failure
     */
    int (*save_start)(registry_store_t *store);

    /**
     * @brief Saves a parameter into storage.
     *
     * @param[in] store Storage facility descriptor
     * @param[in] path Path of the parameter
     * @param[in] path_len Length of @p path
     * @param[in] value Struct representing the value of the parameter
     * @return 0 on success, non-zero on failure
     */
    int (*save)(registry_store_t *store, const int *path, int path_len,
                const registry_value_t value);

    /**
     * @brief If implemented, it is used for any tear-down the storage may need
     * after a saving process.
     *
     * @param[in] store Storage facility descriptor
     * @return 0 on success, non-zero on failure
     */
    int (*save_end)(registry_store_t *store);
} registry_store_itf_t;

/**
 * @brief Instance of a schema containing its data.
 */
typedef struct {
    clist_node_t node;  /**< Linked list node */
    char *name;         /**< String describing the instance */
    void *data;         /**< Struct containing all configuration parameters of the schema */

    /**
     * @brief Will be called after @ref registry_commit() was called on this instance.
     *
     * @param[in] path Path of the parameter to commit changes to
     * @param[in] path_len Length of @p path
     * @param[in] context Context of the instance
     * @return 0 on success, non-zero on failure
     */
    int (*commit_cb)(const int *path, int path_len, void *context);

    void *context; /**< Optional context used by the instance */
} registry_instance_t;

/**
 * @brief Schema for configuration groups. Each configuration group should
 * register a schema using the @ref registry_register_schema() function.
 * A schema provides the pointer to get, set and commit configuration
 * parameters.
 */
typedef struct {
    clist_node_t node;              /**< Linked list node */
    int id;                         /**< Integer representing the configuration group */
    char *name;                     /**< String describing the configuration group */
    char *description;              /**< String describing the configuration group with more details */
    registry_schema_item_t *items;  /**< Array representing all the configuration parameters that belong to this group */
    int items_len;                  /**< Size of items array */
    clist_node_t instances;         /**< Linked list of schema instances @ref registry_instance_t */

    /**
     * @brief Handler to get the current value of a configuration parameter.
     *
     * @param[in] param_id ID of the parameter that contains the value
     * @param[in] instance Pointer to the instance of the schema, that contains the parameter
     * @param[out] buf Pointer to a buffer to store the current value
     * @param[in] buf_len Length of the buffer to store the current value
     * @param[in] context Context of the schema instance
     */
    void (*get)(int param_id, registry_instance_t *instance, void *buf,
                int buf_len, void *context);

    /**
     * @brief Handler to set a the value of a configuration parameter.
     *
     * @param[in] param_id ID of the parameter that contains the value
     * @param[in] instance Pointer to the instance of the schema, that contains the parameter
     * @param[in] val Buffer containing the new value
     * @param[in] val_len Length of the buffer to store the current value
     * @param[in] context Context of the schema instance
     */
    void (*set)(int param_id, registry_instance_t *instance, const void *val,
                int val_len, void *context);
} registry_schema_t;

/**
 * @brief List of registered schemas
 */
extern clist_node_t registry_schemas;

/**
 * @brief Initializes the RIOT Registry and the store modules.
 */
void registry_init(void);

/**
 * @brief Initializes the store module.
 */
void registry_store_init(void);

/**
 * @brief Registers a new schema for a configuration group.
 *
 * @param[in] schema Pointer to the schema structure.
 */
void registry_register_schema(registry_schema_t *schema);

/**
 * @brief Registers a new storage as a source of configurations. Multiple
 *        storages can be configured as sources at the same time. Configurations
 *        will be loaded from all of them. This is commonly called by the
 *        storage facilities who implement their own registry_<storage-name>_src
 *        function.
 *
 * @param[in] src Pointer to the storage to register as source.
 */
void registry_store_register_src(registry_store_t *src);

/**
 * @brief Registers a new storage as a destination for saving configurations.
 *        Only one storage can be registered as destination at a time. If a
 *        previous storage had been registered before it will be replaced by the
 *        new one. This is commonly called by the storage facilities who
 *        implement their own registry_<storage-name>_dst function.
 *
 * @param[in] dst Pointer to the storage to register
 */
void registry_store_register_dst(registry_store_t *dst);

/**
 * @brief Adds a new instance of a schema.
 *
 * @param[in] schema_id ID of the schema.
 * @param[in] instance Pointer to instance structure.
 */
int registry_add_instance(int schema_id, registry_instance_t *instance);

/**
 * @brief Sets the value of a parameter that belongs to a configuration group.
 *
 * @param[in] path Path of the parameter to be set
 * @param[in] path_len Length of @p path
 * @param[in] val New value for the parameter (must have the correct type => use registry_set_<type>() instead!)
 * @param[in] val Length of @p val
 * @return -EINVAL if schema could not be found, otherwise returns the
 *             value of the set schema function.
 */
int registry_set_value(const int *path, int path_len, const void *val, int val_len);

int registry_set_string(const int *path, int path_len, const char *val);
int registry_set_bool(const int *path, int path_len, bool val);
int registry_set_uint8(const int *path, int path_len, uint8_t val);
int registry_set_uint16(const int *path, int path_len, uint16_t val);
int registry_set_uint32(const int *path, int path_len, uint32_t val);
#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
int registry_set_uint64(const int *path, int path_len, uint64_t val);
#endif /* CONFIG_REGISTRY_USE_UINT64 */
int registry_set_int8(const int *path, int path_len, int8_t val);
int registry_set_int16(const int *path, int path_len, int16_t val);
int registry_set_int32(const int *path, int path_len, int32_t val);
#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
int registry_set_int64(const int *path, int path_len, int64_t val);
#endif /* CONFIG_REGISTRY_USE_INT64 */
#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
int registry_set_float32(const int *path, int path_len, float val);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */
#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
int registry_set_float64(const int *path, int path_len, double val);
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

/**
 * @brief Gets the current value of a parameter that belongs to a configuration
 *        group, identified by @p path.
 * @param[in] path Path of the parameter to get the value of
 * @param[in] path_len Length of @p path
 * @param[out] value Pointer to a uninitialized @ref registry_value_t struct
 * @return @p value. Use registry_get_<type>() to get values as a concrete type
 */
registry_value_t *registry_get_value(const int *path, int path_len, registry_value_t *value);

char *registry_get_string(const int *path, int path_len, char *buf, int buf_len);
bool registry_get_bool(const int *path, int path_len);
uint8_t registry_get_uint8(const int *path, int path_len);
uint16_t registry_get_uint16(const int *path, int path_len);
uint32_t registry_get_uint32(const int *path, int path_len);
#if defined(CONFIG_REGISTRY_USE_UINT64) || defined(DOXYGEN)
uint64_t registry_get_uint64(const int *path, int path_len);
#endif /* CONFIG_REGISTRY_USE_UINT64 */
int8_t registry_get_int8(const int *path, int path_len);
int16_t registry_get_int16(const int *path, int path_len);
int32_t registry_get_int32(const int *path, int path_len);
#if defined(CONFIG_REGISTRY_USE_INT64) || defined(DOXYGEN)
int64_t registry_get_int64(const int *path, int path_len);
#endif /* CONFIG_REGISTRY_USE_INT64 */
#if defined(CONFIG_REGISTRY_USE_FLOAT32) || defined(DOXYGEN)
float registry_get_float32(const int *path, int path_len);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */
#if defined(CONFIG_REGISTRY_USE_FLOAT64) || defined(DOXYGEN)
double registry_get_float64(const int *path, int path_len);
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

/**
 * @brief If a @p path is passed it calls the commit schema for that
 *        configuration group. If no @p path is passed the commit schema is
 *        called for every registered configuration group.
 *
 * @param[in] path Path of the configuration group to commit the changes (can
 * be NULL).
 * @param[in] path_len Length of @p path
 * @return 0 on success, -EINVAL if the group has not implemented the commit
 * function.
 */
int registry_commit(const int *path, int path_len);

/**
 * @brief Convenience function to parse a configuration parameter value from
 * a string. The type of the parameter must be known and must not be `bytes`.
 * To parse the string to bytes @ref registry_bytes_from_str() function must be
 * used.
 *
 * @param[in] val_str Pointer of the string containing the value
 * @param[in] type Type of the parameter to be parsed
 * @param[out] vp Pointer to store the parsed value
 * @param[in] maxlen Maximum length of the output buffer when the type of the
 * parameter is string.
 * @return 0 on success, non-zero on failure
 */
int registry_convert_value_from_str(char *val_str, registry_type_t type, void *vp,
                                    int maxlen);

/**
 * @brief Convenience function to parse a configuration parameter value from
 * another value. The type of the parameter must be known.
 *
 * @param[in] val_in Pointer of the input value
 * @param[in] val_in_type Type of the input value
 * @param[in] val_out Pointer to the output buffer
 * @param[in] val_out_len Length of @p val_out
 * @param[in] val_out_type Type of the output value
 * @return 0 on success, non-zero on failure
 */
int registry_convert_value_from_value(const void *val_in, registry_type_t val_in_type,
                                      void *val_out, int val_out_len, registry_type_t val_out_type);

/**
 * @brief Convenience function to parse a configuration parameter value of
 * `bytes` type from a string.
 *
 * @param[in] val_str Pointer of the string containing the value
 * @param[out] vp Pointer to store the parsed value
 * @param len Length of the output buffer
 * @return 0 on success, non-zero on failure
 */
int registry_convert_bytes_from_str(char *val_str, void *vp, int *len);

/**
 * @brief Convenience function to transform a configuration parameter value into
 * a string, when the parameter is not of `bytes` type, in this case
 * @ref registry_str_from_bytes() should be used. This is used for example to
 * implement the `get` or `export` functions.
 *
 * @param[in] type Type of the parameter to be converted
 * @param[in] vp Pointer to the value to be converted
 * @param[out] buf Buffer to store the output string
 * @param[in] buf_len Length of @p buf
 * @return Pointer to the output string
 */
char *registry_convert_str_from_value(registry_type_t type, const void *vp, char *buf,
                                      int buf_len);

/**
 * @brief Convenience function to transform a configuration parameter value of
 * `bytes` type into a string. This is used for example to implement the `get`
 * or `export` schemas.
 *
 * @param[in] vp Pointer to the value to be converted
 * @param[in] vp_len Length of @p vp
 * @param[out] buf Buffer to store the output string
 * @param[in] buf_len Length of @p buf
 * @return Pointer to the output string
 */
char *registry_convert_str_from_bytes(void *vp, int vp_len, char *buf, int buf_len);

/**
 * @brief Load all configuration parameters from the registered storage
 * facilities.
 *
 * @note This should be called after the storage facilities were registered.
 *
 * @return 0 on success, non-zero on failure
 */
int registry_store_load(void);

/**
 * @brief Save all configuration parameters of every configuration group to the
 * registered storage facility.
 *
 * @return 0 on success, non-zero on failure
 */
int registry_store_save(void);

/**
 * @brief Save a specific configuration paramter to the registered storage
 * facility, with the provided value (@p val).
 *
 * @param[in] path Path of the configuration parameter
 * @param[in] path_len Length of @p path
 * @param[in] context Context of the schema instance
 * @return 0 on success, non-zero on failure
 */
int registry_store_save_one(const int *path, int path_len, void *context);

/**
 * @brief Export an specific or all configuration parameters using the
 * @p export_func function. If @p path is NULL then @p export_func is called for
 * every configuration parameter on each configuration group.
 *
 * @param[in] export_func Exporting function call with the @p path and current
 * value of a specific or all configuration parameters
 * @param[in] path Path representing the configuration parameter. Can be NULL.
 * @param[in] path_len Length of @p path
 * @param[in] recursion_depth Defines how deeply nested child groups / parameters will be shown. (0 to show all children, 1 to only show the exact match, 2 - n to show the exact match plus its children ... plus n levels of children )
 * @param[in] context Context that will be passed to @p export_func
 * @return 0 on success, non-zero on failure
 */
int registry_export(int (*export_func)(const int *path, int path_len,
                                       const registry_schema_t *schema,
                                       const registry_instance_t *instance,
                                       const registry_schema_item_t *meta,
                                       const registry_value_t *value,
                                       void *context),
                    const int *path, int path_len, int recursion_depth, void *context);

#ifdef __cplusplus
}
#endif

#endif /* REGISTRY_REGISTRY_H */
