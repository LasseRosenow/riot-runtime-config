# RIOT Runtime Config / RIOT Registry

## Abstract

This memo describes the proposed high-level architecture and
mechanisms to implement a runtime configuration system on a RIOT node.

A runtime configuration system is in charge of providing a mechanism to set and
get the values of configuration parameters that are used during the execution
of the firmware, as well as a way to persist these values. Runtime
configurations are deployment-specific and can be changed on a per node basis.
Appropriate management tools could also enable the configuration of node
groups.

Examples of runtime configurations are:

- Transmission duty cycles
- Sensor thresholds
- Security credentials
- System state variables

These parameters might have constraints, like a specific order to be applied
(due to interdependencies) or value boundaries.

The main advantages of having such a system are:

- Easy to apply per-node configuration during deployment
- No need to implement a special mechanism for per-node configurations during
  firmware updates (only in the case of migration), as the parameters persist.
- Common interface for modules to expose their runtime configuration parameters
  and handle them
- Common interface for storing configuration parameters in non-volatile
  storage devices

## Status

This document is currently under open discussion. This document is a product of the
[Configuration Task
Force](https://github.com/RIOT-OS/RIOT/wiki/Configuration-Task-Force-(CTF)), and
aims to describe the architecture of a runtime configuration system. The content
of this document is licensed with a Creative Commons CC-BY-SA license.

## Terminology

This memo uses the [RFC2119](https://www.ietf.org/rfc/rfc2119.txt) terminology
and the following acronyms and definitions:

### Acronyms

- RDM: RIOT Developer Memo
- CTF: RIOT Configuration Task Force
- RCS: Runtime Configuration System
- CP: Configuration Path
- CS: Configuration Schema
- SI: Schema Instance
- SF: Storage Facility

### Definitions

- __Configuration Path__: A complete configuration path (CP) is a unique identifier of a configuration parameter. It is used to tell the registry, which configuration namespace, configuration schema, schema instance etc. are currently addressed. The registry needs this information, so that it knows where to look for the requested values etc. Below is a regex example showing how the configuration path is structured. All path elements have to be integers:
 `namespace_id/schema_id/instance_id/(group_id/)*parameter_name`\
In reality the amount of "group_ids" is limited to 8 and can be changed with a `define`, so the regex is a bit simplified.

- __Root Configuration Group__: A namespace, that splits configuration schemas in 2 categories: `SYS=0` and `APP=1`. Configuration schemas that are part of `SYS` are RIOT internal configuration schemas that are used to abstract common configuration structures within RIOT like `IEEE802154` etc.
The `APP` configuration namespace must not be used by RIOT itself, but only by the application. This is to prevent application specific schemas from clashing with RIOT internal schemas, if RIOT gets updated.

- __Configuration Schema__: A descriptor that acts as an interface between the RIOT Registry and a module that exposes configurations. It provides a common interface to `get` and `set` configurations of a given instance and provides Meta-Data for each configuration parameter `(type, name, description, ...)` as a tree structure. A CS can have multiple Schema Instances (SI).

- __Schema Instance__: An instance of a CS (Configuration Schema), containing the configuration parameter values. Different `modules/drivers` can use their own SI of the same CS.

- __Configuration Group__: A set of key-value configurations under the same scope. E.g `LIGHT_SENSOR_THRESHOLD` and
`TRANSMISSION_PERIOD` configuration parameters can be contained in an
*Application* configuration group, as well as `IEEE802154_CHANNEL` and
`IEEE802154_TX_POWER` in an *IEEE802.15.4 Radio* configuration group.
Within RIOT, each configuration schema contains one configuration group. And each configuration group can contain multiple sub configuration groups.

- __Storage Facility__: A descriptor that acts as an interface between the RIOT Registry and a non-volatile storage device. It provides a common interface to `load` and `store` key-value data from storage devices that might have different data representations.

# 1. Introduction

This document specifies the proposed architecture by the
Configuration Task Force (CFT) to implement a secure and reliable Runtime
Configuration System (RCS) focusing on modularity, reuse of existing
technologies in RIOT (network stack, storage interface) and standards
compliance.

# 2. Architecture

The proposed RCS architecture, as shown in Figure 01, is formed by one or more
[Configuration Managers](#4-configuration-managers) and the
[RIOT Registry](#3-the-riot-registry).

The RIOT Registry acts as a common interface to access Runtime Configurations and
store them in non-volatile devices.

All runtime configuration can be accessed either from the RIOT Application or
the interfaces exposed by the Configuration Managers, via the RIOT Registry.

A RIOT Application may interact with a Configuration Manager in order to
modify access control rules or enable different exposed interfaces.

![Figure 01](./assets/architecture.svg "Runtime Configuration Architecture")
<p align="center">
Figure 01 - Runtime Configuration System architecture
</p>

The diagram differentiates between 2 different kinds of Configuration Managers:

- `Basic Configuration Managers`: These Configuration Managers are a simple representation of the default configuration structure of the RIOT registry. They only expose the parameters paths as is and do not map to any special structure.
- `Advanced Configuration Managers`: These Configuration Managers have their own configuration structure (custom predefined object models etc.) and can not automatically be mapped to from the RIOT Registry itself. To make them work, a custom mapping module needs to be implemented, which maps each configuration parameter from the registry to the correct format of the configuration manager.

# 3. The RIOT Registry

The RIOT Registry is a module for interacting with __persistent key-value configurations__. It's heavily inspired by the
[Mynewt Config subsystem](https://mynewt.apache.org/latest/os/modules/config/config.html) and [LwM2M Object Models](https://technical.openmobilealliance.org/OMNA/LwM2M/LwM2MRegistry.html)

The RIOT Registry interacts with RIOT modules via
[Configuration Schemas](#31-Registry-schemas), and with non-volatile storage
devices via [Storage Facilities](#32-Storage-facilities). This way the
functionality of the RIOT Registry is independent of the functionality of the module or storage device.

Figure 02 shows an example of two configuration schemas `(My app, LED Strip)` The application `My app` uses the custom `My app` CS to expose custom configuration parameters to the RIOT Registry and the drivers `WS2812, SK6812 and UCS1903` contain instances of the `LED Strip` CS to expose common LED Strip configuration parameters. Also, there are two Storage Facilities available: EEPROM and FAT.

![Figure 02](./assets/components.svg "The RIOT Registry components")
<p align="center">
Figure 02 - The RIOT Registry components
</p>

The API of the RIOT Registry allows to:

- Register a CS to expose a configuration group in the RIOT Registry.
- Add instances to a CS, to expose real values to the RIOT Registry.
- Register source and destination SF.
- Get or set configuration parameters for a given configuration schema instance.
- Commit changes (transactionally apply configurations).
- Export configuration parameters (e.g copy to a buffer, print, etc).
- Load and store configuration parameters from and to a persistent storage device.

Any mechanism of security (access control, encryption of configurations) is NOT
directly in the scope of the Registry but in the Configuration Managers and the
specific implementation of the CS and SF.

See [3.3 RIOT Registry Usage Flow](#33-RIOT-Registry-Usage-Flow) for more
information.

## 3.1. Configuration Schemas

A CS represents a configuration group in the RIOT Registry. A RIOT
module is required to add an instance to a given Configuration Schema in order to expose its configurations to the Registry API. Or needs to implement its own custom CS.

A CS is defined by an id, some Meta-Data (name, description) and a get and set handler for interacting with the configuration parameters of the configuration group.

- `set`: Sets a value to a configuration parameter.
- `get`: Gets the current value of a configuration parameter.

The CS also contains the struct that specifies how each instance (SI) stores the actual data.

A conceptual example of a CS implementation can be found in the
[Appendix](#Appendix).

## 3.1.1. Schema Instances

An instance of a CS, which contains the actual data values. It can be added to a CS and contains a `commit_cb` handler, to notify the module containing the instance about configuration changes that need to be applied.

- `commit_cb`: To be called once configuration parameters have been `set`, in order to apply any further logic required to make them effective (e.g. handling dependencies).

## 3.2. Storage Facilities

Storage facilities MUST implement the __storage interface__ to allow the RIOT Registry to load, search and store configuration parameters. From the point of view of the RIOT Registry all parameters are key/value pairs with certain types, it is the responsibility of the SF to transform those into a proper format to store them. (E.g. lines separated by `\n` character in a file or encoded in cbor etc.).

The interface of a SF is defined with a descriptor that has the following attributes:

- `load`: Executes a callback function for every configuration parameter stored in the storage.
- `store`: Stores one configuration parameter in the storage.

Any kind of storage encryption mechanism is not in the scope of this document, and up to the implementation of `load` and `store` or intrinsic encryption functionalities in the storage.

A minimal RIOT Registry setup requires at least one source SF from which configurations are loaded and exactly one SF destination to which configurations are stored. Having multiple SF sources can be useful when it's required to migrate the data between storage facilities (e.g to migrate all configurations from SF A to B, register B as source and destination and add A as a source).

A conceptual example of a SF can be found in the [Appendix](#Appendix).

## 3.3. RIOT Registry Usage Flow

### 3.3.1. Registry Initialization

As described in the flow in Figure 03, modules add their __Schema Instances (SI)__ to predefined __Configuration Schemas (CS)__ or declare and register their own __CS__ for configuration groups in the RIOT Registry. __Storage facilities (SF)__ are registered as sources and/or destinations of configurations in the RIOT Registry.

<center>

![Figure 03](./assets/behavioral_flow_boot.svg "Usage flow of the RIOT Registry")

</center>
<p align="center">
Figure 03 - Usage flow of the RIOT Registry
</p>

### 3.3.2. Get, set, commit and export configurations

At any time, the application or a configuration manager can *retrieve* a
configuration value (`registry_get_value`), *set* a configuration value
(`registry_set_value`), *commit* configuration changes (`registry_commit`) or
*export* configurations using a user-defined callback function
(`registry_export`).

Note these functions don't interact with the SF, so configuration changes are not reflected in the non-volatile storage devices unless `registry_save` is called (see [Load and save configurations](#load-and-save-configurations-fromto-storage))

The following diagrams show the process of each function. It's assumed there are 2 CS registered in the RIOT Registry: a *cord* configuration schema with a `rd_ip_addr` configuration parameter that is inside the *sys* namespace and a *config* configuration schema that is inside the *app* namespace and has a `foo` configuration parameter.
A registry path usually consists of integers, but the diagrams below uses strings instead just for demonstration purposes only.

#### Behavioral flow of the `get` function

![Figure 04](./assets/behavioral_flow_get.svg "Behavioral flow of the get API")
<p align="center">
Figure 04 - Behavioral flow of the "get" API
</p>

#### Behavioral flow of the `set` function

![Figure 05](./assets/behavioral_flow_set.svg "Behavioral flow of the set API")
<p align="center">
Figure 05 - Behavioral flow of the "set" API
</p>

#### Behavioral flow of the `commit` function

![Figure 06](./assets/behavioral_flow_commit.svg "Behavioral flow of the commit API")
<p align="center">
Figure 06 - Behavioral flow of the "commit" API
</p>

#### Behavioral flow of the `export` function

![Figure 07](./assets/behavioral_flow_export.svg "Behavioral flow of the export API")
<p align="center">
Figure 07 - Behavioral flow of the "export" API
</p>

### Load and save configurations from/to storage

At any time, the application or a configuration manager can *load* all configurations from all SF sources (`registry_load` function) or *store* them in the SF destination (`registry_save` function).

As one could expect, `registry_load` will call the SF `load` handler with `registry_set_value` as callback. In the a similar way, `registry_save` will call `registry_export` on all CS with the SF *store* handler as callback.

Figure 08 shows the above described processes.

![Figure 08](./assets/behavioral_flow_store.svg "Behavioral flow of the store_load and store_save calls")
<p align="center">
Figure 08 - Behavioral flow of the store_load and store_save calls
</p>

### Add custom schema to the registry

The registry itself already comes with many configuration schemas that live within the `sys` configuration namespace. But sometimes an application needs some custom runtime configurations that are too specific for the registry to abstract, so it is possible to register a custom CS within the `app` configuration namespace.
One MUST NOT register a custom schema within the `sys` configuration namespace, as this is a reserved space and using it would almost certainly result in conflicts whenever RIOT gets updated.

Below is a diagram that shows the behavioral flow of adding a custom schema:

![Figure 09](./assets/behavioral_flow_register_custom_schema.svg "Behavioral flow of the registration of custom registry schemas")
<p align="center">
Figure 09 - Behavioral flow of the registration of custom registry schemas
</p>

## 3.4. RIOT Registry API

![Figure 10](./assets/api_structure.svg "RIOT Registry API")

```c++
/* Base */
void registry_init(void);
int registry_set_value(const registry_path_t path, const registry_value_t val);
int registry_get_value(const registry_path_t path, registry_value_t *value);
int registry_commit(const registry_path_t path);
int registry_export(int (*export_func)(
    const registry_path_t path,
    const registry_schema_t *schema,
    const registry_instance_t *instance,
    const registry_schema_item_t *meta,
    const registry_value_t *value,
    const void *context
  ),
  const registry_path_t path, const int recursion_depth, const void *context
);
int registry_load(const registry_path_t path);
int registry_save(const registry_path_t path);


/* Store */
void registry_register_store_src(const registry_store_instance_t *src);
void registry_register_store_dst(const registry_store_instance_t *dst);


/* Schemas */
void registry_schemas_init(void);
int registry_register_schema(const registry_namespace_id_t namespace_id, const registry_schema_t *schema);
int registry_register_schema_instance(const registry_namespace_id_t namespace_id, const int schema_id, const registry_instance_t *instance);


/* Set convenience functions */
int registry_set_opaque(const registry_path_t path, const void *val, const size_t val_len);
int registry_set_string(const registry_path_t path, const char *val);
int registry_set_bool(const registry_path_t path, const bool val);
int registry_set_uint8(const registry_path_t path, const uint8_t val);
int registry_set_uint16(const registry_path_t path, const uint16_t val);
int registry_set_uint32(const registry_path_t path, const uint32_t val);
int registry_set_uint64(const registry_path_t path, const uint64_t val);
int registry_set_int8(const registry_path_t path, const int8_t val);
int registry_set_int16(const registry_path_t path, const int16_t val);
int registry_set_int32(const registry_path_t path, const int32_t val);
int registry_set_int64(const registry_path_t path, const int64_t val);
int registry_set_float32(const registry_path_t path, const float val);
int registry_set_float64(const registry_path_t path, const double val);


/* Get convenience functions */
int registry_get_opaque(const registry_path_t path, const void **buf, size_t *buf_len);
int registry_get_string(const registry_path_t path, const char **buf, size_t *buf_len);
int registry_get_bool(const registry_path_t path, const bool **buf);
int registry_get_uint8(const registry_path_t path, const uint8_t **buf);
int registry_get_uint16(const registry_path_t path, const uint16_t **buf);
int registry_get_uint32(const registry_path_t path, const uint32_t **buf);
int registry_get_uint64(const registry_path_t path, const uint64_t **buf);
int registry_get_int8(const registry_path_t path, const int8_t **buf);
int registry_get_int16(const registry_path_t path, const int16_t **buf);
int registry_get_int32(const registry_path_t path, const int32_t **buf);
int registry_get_int64(const registry_path_t path, const int64_t **buf);
int registry_get_float32(const registry_path_t path, const float **buf);
int registry_get_float64(const registry_path_t path, const double **buf);
```

# 4. Integrating external Configuration Managers

## 4.1. Simple Configuration Managers

Simple Configuration Managers are ways to use the RIOT Registry without the need to maintain adapters. Those managers would only be implemented once and mirror the internal structure of the RIOT Registry.
This can be quite powerful within RIOT-only environments, but is not as powerful in terms of its "plug and play"ness

### 4.1.1. CLI

The RIOT CLI can be extended with a `registry` command, which is followed by a sub-command `set | get | commit | export`.\
Each sub-command has a specific CLI interface:

- get: `<path>`
- set: `<path> <value>`
- commit: `<path>`
- export: `<path> [-r <recursion depth>]`
- load: `[path]`
- save: `[path]`

The `<path>` argument is a string of integers separated by `/`. It maps directly to the RIOT registry internal path structure.\
The `<value>` argument is just the value as a string.\
The export command also has the additional `-r <recursion depth>` flag. It defaults to 0, which means that everything will be exported recursively. A value of 1 means, that only the parameter that exactly matches the specified path will be exported. A value of 2 means the same as a value of 1 but also all of its children will be exported etc...

### 4.1.1. CoAP

The CoAP integration uses the RIOT internal registry structure and does not come with its own schema structure. But CoAP only has `get` and `set`, but no `export` or `commit`. So the get and set command of the RIOT registry will just be mapped to the get and set of CoAP. For example: `GET /namespace_id/schema_id/...` or `SET /namespace_id/schema_id/... -> new_value`
The `export` command can be realized through the `GET /.well-known/core` endpoint.
The `commit` command is less trivial as there is no equivalent construct within CoAP itself. But here are some ideas:

- Make a get request which path has a `commit` prefix like: `GET /commit/namespace_id/schema_id/...`
- Have a dedicated `commit` endpoint, which can be set to a specific path, which then will be committed. For example: `SET /commit -> /namespace_id/schema_id/...`
- Don't implement the `commit` concept at all, but rather commit every `set` message and allow to send values to whole groups / schemas with the complete group / schema or parts of it. For example in the CBOR or JSON format. This way it still is possible to change multiple values at once.

### 4.1.2. MQTT

The MQTT integration uses the RIOT internal registry structure and does not come with its own schema structure. But is limited to only having events with or without data.\
So there are no `commands` like `set`, `get`, `commit` or `export`. Values will be set, by sending a `publish` event containing the new value and subscribing to the same event will notify the subscriber whenever a new value is available. This way `set` and `get` can be realized.\
The `export` command is not necessary because the MQTT broker gets an initial publish for each parameter when the device boots. So it knows about all existing topics and can expose them.\
Because one MQTT broker can have multiple RIOT nodes, it is necessary to prefix the topic of each message with a device_id. For example: `device_id/namespace_id/schema_id/...`\
Less trivial is how `commit` can be exposed to MQTT. But here are some ideas:

- Extend the topic of the path that needs to be committed with a `commit` prefix.
For example: `commit/device_id/namespace_id/schema_id/...`
- Have a dedicated `commit` topic, which can be set to a specific path, which then will be committed. For example: `Publish commit -> /device_id/namespace_id/schema_id/...`
- Don't implement the `commit` concept at all, but rather commit every `set` message and allow to send values to whole groups / schemas with the complete group / schema or parts of it. For example in the CBOR or JSON format. This way it still is possible to change multiple values at once.

<center>

![Figure 11](./assets/behavioral_flow_mqtt_integration.svg "MQTT integration")

</center>

### 4.1.3. MCUMgr (mgmt)

TODO

## 4.2. Advanced Configuration Managers

While having the ability to use the Registry inside RIOT and using a (UART) CLI, the registry itself is designed so that it can easily integrate with common external configuration managers. This makes it possible to modify parameters for example via the ethernet, LoRa, bluetooth, 802.15.4 etc.
The basic idea is that the RIOT Registry with its common `schemas` defines a RIOT internal `Single Source of Truth`, as to which kind of data is to find where.
Then each external configuration manager has to implement its own `adapter` module, which maps/converts their own structures to the RIOT Registry.

### 4.2.1. LwM2M

LwM2M is a relatively new protocol that is similar to the RIOT registry in that it specifies official (and unofficial) `object models` that define which information can be found where. It internally uses CoAP and has a concept of `instances` as well. A typical LwM2M `path` looks like this:\
`object_id/instance_id/parameter_id`\
The `object_id` is similar to RIOTs `schema_id`, the `instance_id` is the same as in RIOT and the `parameter_id` is also the same as in RIOT except LwM2M does not know anything about nesting, so there are no paths longer than `3`.\
To integrate LwM2M to the RIOT Registry it is necessary to write an adapter that maps the `LwM2M Object Models` to the `RIOT Registry Schemas`.\
An example of how this adapter would handle a `set` call can be seen below:

![Figure 12](./assets/behavioral_flow_lwm2m_integration.svg "LwM2M integration")

# 5. Appendix

## 5.1. Example Registry Schema

### 5.1.1. rgb_led.h

```c++
#include "registry.h"

#define REGISTRY_SCHEMA_RGB_LED 1

extern registry_schema_t registry_schema_rgb_led;

typedef struct {
    clist_node_t node;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} registry_schema_rgb_led_t;

typedef enum {
    REGISTRY_SCHEMA_RGB_LED_RED,
    REGISTRY_SCHEMA_RGB_LED_GREEN,
    REGISTRY_SCHEMA_RGB_LED_BLUE,
} registry_schema_rgb_led_indices_t;
```

### 5.1.2 rgb_led.c

```c++
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "kernel_defines.h"
#include "registry.h"
#include "rgb_led.h"

static void mapping(const int param_id, const registry_instance_t *instance, void **val, size_t *val_len);

REGISTRY_SCHEMA(
    registry_schema_rgb_led,
    REGISTRY_SCHEMA_RGB_LED,
    "rgb", "Representation of an rgb color.",
    mapping,

    REGISTRY_PARAMETER_UINT8(
        REGISTRY_SCHEMA_RGB_LED_RED,
        "red", "Intensity of the red color of the rgb lamp.")

    REGISTRY_PARAMETER_UINT8(
        REGISTRY_SCHEMA_RGB_LED_GREEN,
        "green", "Intensity of the green color of the rgb lamp.")

    REGISTRY_PARAMETER_UINT8(
        REGISTRY_SCHEMA_RGB_LED_BLUE,
        "blue", "Intensity of the blue color of the rgb lamp.")

    );

static void mapping(const int param_id, const registry_instance_t *instance, void **val, size_t *val_len)
{
    registry_schema_rgb_led_t *_instance = (registry_schema_rgb_led_t *)instance->data;

    switch (param_id) {
    case REGISTRY_SCHEMA_RGB_LED_RED:
        *val = &_instance->red;
        *val_len = sizeof(_instance->red);
        break;

    case REGISTRY_SCHEMA_RGB_LED_GREEN:
        *val = &_instance->green;
        *val_len = sizeof(_instance->green);
        break;

    case REGISTRY_SCHEMA_RGB_LED_BLUE:
        *val = &_instance->blue;
        *val_len = sizeof(_instance->blue);
        break;
    }
}
```

### 5.1.3. main.c

```c++
#include <string.h>
#include <msg.h>
#include <stdio.h>

#include "registry.h"
#include "assert.h"

#include "rgb_led.h"

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

int main(void)
{
    /* init registry */
    registry_init();

    /* add schema instances */
    registry_register_schema_instance(REGISTRY_ROOT_GROUP_APP, registry_schema_rgb_led.id, &rgb_led_instance_0);

    registry_register_schema_instance(REGISTRY_ROOT_GROUP_APP, registry_schema_rgb_led.id, &rgb_led_instance_1);

    /* set value */
    registry_set_uint8(REGISTRY_PATH_APP(REGISTRY_SCHEMA_RGB_LED, 0, REGISTRY_SCHEMA_RGB_LED_BLUE), 19);

    /* get value */
    const uint8_t *value;
    registry_get_uint8(REGISTRY_PATH_APP(REGISTRY_SCHEMA_RGB_LED, 0, REGISTRY_SCHEMA_RGB_LED_BLUE), &value);

    return 0;
}
```
