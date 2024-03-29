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

#ifndef REGISTRY_H
#define REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "kernel_defines.h"
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
#define REGISTRY_MAX_DIR_NAME_LEN  10 /* a path is an uint32_t and uint32_t MAX has 10 digits. */

/**
 * @brief Maximum length of a configuration name.
 * @{
 */
#define REGISTRY_MAX_DIR_LEN      ((REGISTRY_MAX_DIR_NAME_LEN * \
                                    REGISTRY_MAX_DIR_DEPTH) + \
                                   (REGISTRY_MAX_DIR_DEPTH - 1))
/** @} */

/**
 * @brief Calculates the size of an @ref registry_schema_item_t array.
 *
 */
#define _REGISTRY_SCHEMA_ITEM_NUMARGS(...)  (sizeof((registry_schema_item_t[]){ __VA_ARGS__ }) / \
                                             sizeof(registry_schema_item_t))

/**
 * @brief Creates and initializes a @ref registry_schema_t struct.
 *
 */
#define REGISTRY_SCHEMA(_field_name, _id, _name, _description, _mapping, ...) \
    registry_schema_t _field_name = { \
        .id = _id, \
        .name = _name, \
        .description = _description, \
        .mapping = _mapping, \
        .items = (registry_schema_item_t[]) { __VA_ARGS__ }, \
        .items_len = _REGISTRY_SCHEMA_ITEM_NUMARGS(__VA_ARGS__), \
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
            .items_len = _REGISTRY_SCHEMA_ITEM_NUMARGS(__VA_ARGS__), \
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

#if IS_ACTIVE(CONFIG_REGISTRY_DISABLE_SCHEMA_NAME_FIELD) && \
    IS_ACTIVE(CONFIG_REGISTRY_DISABLE_SCHEMA_DESCRIPTION_FIELD)
/* no name and no description */
# define _REGISTRY_PARAMETER(_id, _name, _description, _type) \
    REGISTRY_PARAMETER(_id, "", "", _type)
#elif IS_ACTIVE(CONFIG_REGISTRY_DISABLE_SCHEMA_NAME_FIELD)
/* no name */
# define _REGISTRY_PARAMETER(_id, _name, _description, _type) \
    REGISTRY_PARAMETER(_id, "", _description, _type)
#elif IS_ACTIVE(CONFIG_REGISTRY_DISABLE_SCHEMA_DESCRIPTION_FIELD)
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

#if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64) || IS_ACTIVE(DOXYGEN)
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

#if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64) || IS_ACTIVE(DOXYGEN)
# define REGISTRY_PARAMETER_INT64(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_INT64)
#else
# define REGISTRY_PARAMETER_INT64(_id, _name, _description)
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32) || IS_ACTIVE(DOXYGEN)
# define REGISTRY_PARAMETER_FLOAT32(_id, _name, _description) \
    _REGISTRY_PARAMETER(_id, _name, _description, REGISTRY_TYPE_FLOAT32)
#else
# define REGISTRY_PARAMETER_FLOAT32(_id, _name, _description)
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64) || IS_ACTIVE(DOXYGEN)
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
    REGISTRY_TYPE_OPAQUE,       /**< OPAQUE */
    REGISTRY_TYPE_STRING,       /**< String */
    REGISTRY_TYPE_BOOL,         /**< Boolean */

    REGISTRY_TYPE_UINT8,        /**< 8-bits unsigned integer */
    REGISTRY_TYPE_UINT16,       /**< 16-bits unsigned integer */
    REGISTRY_TYPE_UINT32,       /**< 32-bits unsigned integer */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64) || IS_ACTIVE(DOXYGEN)
    REGISTRY_TYPE_UINT64,     /**< 64-bits unsigned integer */
#endif /* CONFIG_REGISTRY_USE_UINT64 */

    REGISTRY_TYPE_INT8,         /**< 8-bits signed integer */
    REGISTRY_TYPE_INT16,        /**< 16-bits signed integer */
    REGISTRY_TYPE_INT32,        /**< 32-bits signed integer */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64) || IS_ACTIVE(DOXYGEN)
    REGISTRY_TYPE_INT64,     /**< 64-bits signed integer */
#endif /* CONFIG_REGISTRY_USE_INT64 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32) || IS_ACTIVE(DOXYGEN)
    REGISTRY_TYPE_FLOAT32,     /**< 32-bits float */
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */

#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64) || IS_ACTIVE(DOXYGEN)
    REGISTRY_TYPE_FLOAT64,     /**< 64-bits float */
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */
} registry_type_t;





typedef enum {
    REGISTRY_ROOT_GROUP_SYS,
    REGISTRY_ROOT_GROUP_APP,
} registry_namespace_id_t;

typedef struct {
    registry_namespace_id_t id;     /**< Integer representing the configuration namespace */
    char *name;                     /**< String describing the configuration namespace */
    char *description;              /**< String describing the configuration namespace with more details */
    clist_node_t schemas;           /**< Linked list of schemas @ref registry_schema_t */
} registry_namespace_t;

extern registry_namespace_t registry_namespace_sys;
extern registry_namespace_t registry_namespace_app;

typedef uint32_t registry_id_t;

typedef struct {
    registry_namespace_id_t *namespace_id;
    registry_id_t *schema_id;
    registry_id_t *instance_id;
    registry_id_t *path;
    size_t path_len;
} registry_path_t;

#define _REGISTRY_PATH_NUMARGS(...)  (sizeof((registry_id_t[]){ __VA_ARGS__ }) / \
                                      sizeof(registry_id_t))

#define _REGISTRY_PATH_0() \
    (registry_path_t) { \
        .namespace_id = NULL, \
        .schema_id = NULL, \
        .instance_id = NULL, \
        .path = NULL, \
        .path_len = 0, \
    }

#define _REGISTRY_PATH_1(_namespace_id) \
    (registry_path_t) { \
        .namespace_id = (registry_namespace_id_t[]) { _namespace_id }, \
        .schema_id = NULL, \
        .instance_id = NULL, \
        .path = NULL, \
        .path_len = 0, \
    }

#define _REGISTRY_PATH_2(_namespace_id, _schema_id) \
    (registry_path_t) { \
        .namespace_id = (registry_namespace_id_t[]) { _namespace_id }, \
        .schema_id = (registry_id_t[]) { _schema_id }, \
        .instance_id = NULL, \
        .path = NULL, \
        .path_len = 0, \
    }

#define _REGISTRY_PATH_3(_namespace_id, _schema_id, _instance_id) \
    (registry_path_t) { \
        .namespace_id = (registry_namespace_id_t[]) { _namespace_id }, \
        .schema_id = (registry_id_t[]) { _schema_id }, \
        .instance_id = (registry_id_t[]) { _instance_id }, \
        .path = NULL, \
        .path_len = 0, \
    }

#define _REGISTRY_PATH_4_AND_MORE(_namespace_id, _schema_id, _instance_id, ...) \
    (registry_path_t) { \
        .namespace_id = (registry_namespace_id_t[]) { _namespace_id }, \
        .schema_id = (registry_id_t[]) { _schema_id }, \
        .instance_id = (registry_id_t[]) { _instance_id }, \
        .path = (registry_id_t[]) { __VA_ARGS__ }, \
        .path_len = _REGISTRY_PATH_NUMARGS(__VA_ARGS__), \
    }

#define GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME
#define REGISTRY_PATH(...) \
    GET_MACRO(_0, ## __VA_ARGS__, \
              _REGISTRY_PATH_4_AND_MORE, \
              _REGISTRY_PATH_4_AND_MORE, \
              _REGISTRY_PATH_4_AND_MORE, \
              _REGISTRY_PATH_4_AND_MORE, \
              _REGISTRY_PATH_4_AND_MORE, \
              _REGISTRY_PATH_4_AND_MORE, \
              _REGISTRY_PATH_4_AND_MORE, \
              _REGISTRY_PATH_3, \
              _REGISTRY_PATH_2, \
              _REGISTRY_PATH_1, \
              _REGISTRY_PATH_0) \
    (__VA_ARGS__)

#define REGISTRY_PATH_SYS(...) \
    REGISTRY_PATH(REGISTRY_ROOT_GROUP_SYS, ## __VA_ARGS__)

#define REGISTRY_PATH_APP(...) \
    REGISTRY_PATH(REGISTRY_ROOT_GROUP_APP, ## __VA_ARGS__)





/**
 * @brief Basic representation of a registry parameter, containing information about its type and its value.
 */
typedef struct {
    registry_type_t type;   /**< The type of the parameter */
    const void *buf;        /**< Pointer to the buffer containing the value of the parameter */
    size_t buf_len;         /**< Length of the buffer */
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
    size_t items_len;
} registry_schema_group_t;

typedef enum {
    REGISTRY_SCHEMA_TYPE_GROUP,
    REGISTRY_SCHEMA_TYPE_PARAMETER,
} registry_schema_type_t;

struct _registry_schema_item_t {
    registry_id_t id;                           /**< Integer representing the path id of the schema item */
    char *name;                                 /**< String describing the schema item */
    char *description;                          /**< String describing the schema item with more details */
    registry_schema_type_t type;                /**< Type of the schema item (group or parameter) */
    union {
        registry_schema_group_t group;          /**< Value of the schema item if it is a group. Contains an array of schema item children */
        registry_schema_parameter_t parameter;  /**< Value of the schema item if it is a parameter. Contains its type */
    } value;                                    /**< Union containing either group or parameter data */
};

/**
 * @brief Prototype of a callback function for the load action of a storage facility
 * interface
 */
typedef void (*load_cb_t)(const registry_path_t path, const registry_value_t val,
                          const void *cb_arg);

/**
 * @brief Descriptor used to check duplications in storage facilities
 */
typedef struct {
    const registry_path_t path; /**< path of the parameter being checked */
    const registry_value_t val; /**< value of the parameter being checked */
    bool is_dup;                /**< flag indicating if the parameter is duplicated or not */
} registry_dup_check_arg_t;

typedef struct _registry_storage_facility_t registry_storage_facility_t;

/**
 * @brief Store facility descriptor
 */
typedef struct {
    clist_node_t node;                  /**< linked list node */
    registry_storage_facility_t *itf;   /**< interface for the facility */
    void *data;                         /**< Struct containing all config data for the storage facility */
} registry_storage_facility_instance_t;

/**
 * @brief Storage facility interface.
 * All storage facilities should, at least, implement the load and save actions.
 */
struct _registry_storage_facility_t {
    /**
     * @brief Loads all saved parameters and calls the @p cb callback function.
     *
     * @param[in] instance Storage facility descriptor
     * @param[in] path Path of the parameter
     * @param[in] cb Callback function to call for every saved parameter
     * @param[in] cb_arg Argument passed to @p cb function
     * @return 0 on success, non-zero on failure
     */
    int (*load)(const registry_storage_facility_instance_t *instance, const registry_path_t path,
                const load_cb_t cb, const void *cb_arg);

    /**
     * @brief If implemented, it is used for any preparation the storage may
     * need before starting a saving process.
     *
     * @param[in] instance Storage facility descriptor
     * @return 0 on success, non-zero on failure
     */
    int (*save_start)(const registry_storage_facility_instance_t *instance);

    /**
     * @brief Saves a parameter into storage.
     *
     * @param[in] instance Storage facility descriptor
     * @param[in] path Path of the parameter
     * @param[in] value Struct representing the value of the parameter
     * @return 0 on success, non-zero on failure
     */
    int (*save)(const registry_storage_facility_instance_t *instance, const registry_path_t path,
                const registry_value_t value);

    /**
     * @brief If implemented, it is used for any tear-down the storage may need
     * after a saving process.
     *
     * @param[in] instance Storage facility descriptor
     * @return 0 on success, non-zero on failure
     */
    int (*save_end)(const registry_storage_facility_instance_t *instance);
};

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
     * @param[in] context Context of the instance
     * @return 0 on success, non-zero on failure
     */
    int (*commit_cb)(const registry_path_t path, const void *context);

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
    registry_id_t id;               /**< Integer representing the configuration group */
    char *name;                     /**< String describing the configuration group */
    char *description;              /**< String describing the configuration group with more details */
    registry_schema_item_t *items;  /**< Array representing all the configuration parameters that belong to this group */
    size_t items_len;               /**< Size of items array */
    clist_node_t instances;         /**< Linked list of schema instances @ref registry_instance_t */

    /**
     * @brief Mapping to connect configuration parameter IDs with the address in the storage.
     *
     * @param[in] param_id ID of the parameter that contains the value
     * @param[in] instance Pointer to the instance of the schema, that contains the parameter
     * @param[in] val Pointer to buffer containing the new value
     * @param[in] val_len Pointer to length of the buffer to store the current value
     */
    void (*mapping)(const registry_id_t param_id, const registry_instance_t *instance, void **val,
                    size_t *val_len);
} registry_schema_t;

/**
 * @brief Initializes the RIOT Registry.
 */
void registry_init(void);

/**
 * @brief Registers a new sys schema for a configuration group.
 *
 * @param[in] namespace_id ID of the namespace.
 * @param[in] schema Pointer to the schema structure.
 */
int registry_register_schema(const registry_namespace_id_t namespace_id,
                             const registry_schema_t *schema);

/**
 * @brief Registers a new storage as a source of configurations. Multiple
 *        storages can be configured as sources at the same time. Configurations
 *        will be loaded from all of them. This is commonly called by the
 *        storage facilities who implement their own registry_<storage-name>_src
 *        function.
 *
 * @param[in] src Pointer to the storage to register as source.
 */
void registry_register_storage_facility_src(const registry_storage_facility_instance_t *src);

/**
 * @brief Registers a new storage as a destination for saving configurations.
 *        Only one storage can be registered as destination at a time. If a
 *        previous storage had been registered before it will be replaced by the
 *        new one. This is commonly called by the storage facilities who
 *        implement their own registry_<storage-name>_dst function.
 *
 * @param[in] dst Pointer to the storage to register
 */
void registry_register_storage_facility_dst(const registry_storage_facility_instance_t *dst);

/**
 * @brief Adds a new instance of a schema.
 *
 * @param[in] namespace_id ID of the namespace.
 * @param[in] schema_id ID of the schema.
 * @param[in] instance Pointer to instance structure.
 */
int registry_register_schema_instance(const registry_namespace_id_t namespace_id,
                                      const registry_id_t schema_id,
                                      const registry_instance_t *instance);

/**
 * @brief Sets the value of a parameter that belongs to a configuration group.
 *
 * @param[in] path Path of the parameter to be set
 * @param[in] val New value for the parameter
 * @return -EINVAL if schema could not be found, otherwise returns the
 *             value of the set schema function.
 */
int registry_set_value(const registry_path_t path, const registry_value_t val);

int registry_set_opaque(const registry_path_t path, const void *val, const size_t val_len);
int registry_set_string(const registry_path_t path, const char *val);
int registry_set_bool(const registry_path_t path, const bool val);
int registry_set_uint8(const registry_path_t path, const uint8_t val);
int registry_set_uint16(const registry_path_t path, const uint16_t val);
int registry_set_uint32(const registry_path_t path, const uint32_t val);
#if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64) || IS_ACTIVE(DOXYGEN)
int registry_set_uint64(const registry_path_t path, const uint64_t val);
#endif /* CONFIG_REGISTRY_USE_UINT64 */
int registry_set_int8(const registry_path_t path, const int8_t val);
int registry_set_int16(const registry_path_t path, const int16_t val);
int registry_set_int32(const registry_path_t path, const int32_t val);
#if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64) || IS_ACTIVE(DOXYGEN)
int registry_set_int64(const registry_path_t path, const int64_t val);
#endif /* CONFIG_REGISTRY_USE_INT64 */
#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32) || IS_ACTIVE(DOXYGEN)
int registry_set_float32(const registry_path_t path, const float val);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */
#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64) || IS_ACTIVE(DOXYGEN)
int registry_set_float64(const registry_path_t path, const double val);
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

/**
 * @brief Gets the current value of a parameter that belongs to a configuration
 *        group, identified by @p path.
 * @param[in] path Path of the parameter to get the value of
 * @param[out] value Pointer to a uninitialized @ref registry_value_t struct
 * @return 0 on success, non-zero on failure
 */
int registry_get_value(const registry_path_t path, registry_value_t *value);

int registry_get_opaque(const registry_path_t path, const void **buf, size_t *buf_len);
int registry_get_string(const registry_path_t path, const char **buf, size_t *buf_len);
int registry_get_bool(const registry_path_t path, const bool **buf);
int registry_get_uint8(const registry_path_t path, const uint8_t **buf);
int registry_get_uint16(const registry_path_t path, const uint16_t **buf);
int registry_get_uint32(const registry_path_t path, const uint32_t **buf);
#if IS_ACTIVE(CONFIG_REGISTRY_USE_UINT64) || IS_ACTIVE(DOXYGEN)
int registry_get_uint64(const registry_path_t path, const uint64_t **buf);
#endif /* CONFIG_REGISTRY_USE_UINT64 */
int registry_get_int8(const registry_path_t path, const int8_t **buf);
int registry_get_int16(const registry_path_t path, const int16_t **buf);
int registry_get_int32(const registry_path_t path, const int32_t **buf);
#if IS_ACTIVE(CONFIG_REGISTRY_USE_INT64) || IS_ACTIVE(DOXYGEN)
int registry_get_int64(const registry_path_t path, const int64_t **buf);
#endif /* CONFIG_REGISTRY_USE_INT64 */
#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT32) || IS_ACTIVE(DOXYGEN)
int registry_get_float32(const registry_path_t path, const float **buf);
#endif /* CONFIG_REGISTRY_USE_FLOAT32 */
#if IS_ACTIVE(CONFIG_REGISTRY_USE_FLOAT64) || IS_ACTIVE(DOXYGEN)
int registry_get_float64(const registry_path_t path, const double **buf);
#endif /* CONFIG_REGISTRY_USE_FLOAT64 */

/**
 * @brief If a @p path is passed it calls the commit schema for that
 *        configuration group. If no @p path is passed the commit schema is
 *        called for every registered configuration group.
 *
 * @param[in] path Path of the configuration group to commit the changes (can
 * be NULL).
 * @return 0 on success, -EINVAL if the group has not implemented the commit
 * function.
 */
int registry_commit(const registry_path_t path);

/**
 * @brief Load all configuration parameters that are included in the path from the registered storage
 * facility.
 *
 * @param[in] path Path of the configuration parameters
 * @return 0 on success, non-zero on failure
 */
int registry_load(const registry_path_t path);

/**
 * @brief Save all configuration parameters of every configuration group to the
 * registered storage facility.
 *
 * @param[in] path Path of the configuration parameters
 * @return 0 on success, non-zero on failure
 */
int registry_save(const registry_path_t path);

/**
 * @brief Export an specific or all configuration parameters using the
 * @p export_func function. If @p path is NULL then @p export_func is called for
 * every configuration parameter on each configuration group.
 *
 * @param[in] export_func Exporting function call with the @p path and current
 * value of a specific or all configuration parameters
 * @param[in] path Path representing the configuration parameter. Can be NULL.
 * @param[in] recursion_depth Defines how deeply nested child groups / parameters will be shown. (0 to show all children, 1 to only show the exact match, 2 - n to show the exact match plus its children ... plus n levels of children )
 * @param[in] context Context that will be passed to @p export_func
 * @return 0 on success, non-zero on failure
 */
int registry_export(int (*export_func)(const registry_path_t path,
                                       const registry_schema_t *schema,
                                       const registry_instance_t *instance,
                                       const registry_schema_item_t *meta,
                                       const registry_value_t *value,
                                       const void *context),
                    const registry_path_t path, const int recursion_depth, const void *context);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* REGISTRY_H */
