# Because of periph_i2c and periph_rtt
CONTINUE_ON_EXPECTED_ERRORS=1

# name of your application
APPLICATION = runtime_config

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/RIOT

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules
USEMODULE += gnrc_ipv6_default
USEMODULE += gcoap
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo

# Required by gcoap example
USEMODULE += od
USEMODULE += fmt
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Unit tests
USEMODULE += embunit

# Cbor
USEPKG += tinycbor
USEMODULE += tinycbor_float








# Riot registry config
# Enable int64 and float support
CFLAGS += -DCONFIG_REGISTRY_USE_UINT64=1
CFLAGS += -DCONFIG_REGISTRY_USE_INT64=1
CFLAGS += -DCONFIG_REGISTRY_USE_FLOAT32=1
CFLAGS += -DCONFIG_REGISTRY_USE_FLOAT64=1

# Enable registry schemas
CFLAGS += -DCONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED=1
CFLAGS += -DCONFIG_REGISTRY_ENABLE_SCHEMA_TYPES_TEST=1

# Disable name or description fields in schemas
#CFLAGS += -DCONFIG_REGISTRY_DISABLE_SCHEMA_NAME_FIELD=1
#CFLAGS += -DCONFIG_REGISTRY_DISABLE_SCHEMA_DESCRIPTION_FIELD=1

# External modules
USEMODULE += registry
USEMODULE += registry_schemas
USEMODULE += registry_cli
USEMODULE += registry_coap
#USEMODULE += lwm2m_objects
USEMODULE += lwm2m_objects_abstract
EXTERNAL_MODULE_DIRS += external_modules

# Enable lwM2M objects
CFLAGS += -DCONFIG_LWM2M_OBJECTS_ENABLE_OBJECT_3420=1









# LwM2M config
CFLAGS += -DCONFIG_LWM2M_WITH_LOGS=1
# Specific the server URI  address (NOTE: Domain names not supported yet)
ifndef CONFIG_LWM2M_SERVER_URI
  CFLAGS += -DCONFIG_LWM2M_SERVER_URI='"coap://[fe80::845c:97ff:feec:c1c6]:5683"'
endif
ifndef CONFIG_LWM2M_DEVICE_NAME
  CFLAGS += -DCONFIG_LWM2M_DEVICE_NAME='"riot-device"'
endif






# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
