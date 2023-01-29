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
#USEMODULE += netdev_default
#USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules
#USEMODULE += gnrc_ipv6_default
#USEMODULE += gcoap
# Additional networking modules that can be dropped if not needed
#USEMODULE += gnrc_icmpv6_echo

# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

# Unit tests
USEMODULE += embunit

# File systems
USEMODULE += littlefs2
USEMODULE += mtd
USEMODULE += vfs

# Riot registry config
# Enable int64 and float support
CFLAGS += -DCONFIG_REGISTRY_USE_UINT64=1
CFLAGS += -DCONFIG_REGISTRY_USE_INT64=1
CFLAGS += -DCONFIG_REGISTRY_USE_FLOAT32=1
CFLAGS += -DCONFIG_REGISTRY_USE_FLOAT64=1

# Enable registry schemas
CFLAGS += -DCONFIG_REGISTRY_ENABLE_SCHEMA_RGB_LED=1
CFLAGS += -DCONFIG_REGISTRY_ENABLE_SCHEMA_FULL_EXAMPLE=1

# Disable name or description fields in schemas
#CFLAGS += -DCONFIG_REGISTRY_DISABLE_SCHEMA_NAME_FIELD=1
#CFLAGS += -DCONFIG_REGISTRY_DISABLE_SCHEMA_DESCRIPTION_FIELD=1

# External modules
USEMODULE += registry
USEMODULE += registry_schemas
USEMODULE += registry_storage_facilities
USEMODULE += registry_cli
USEMODULE += registry_tests
EXTERNAL_MODULE_DIRS += external_modules





# DEMO
# USEMODULE += ws281x
# USEMODULE += ws281x_esp32
# CFLAGS += '-DWS281X_PARAM_PIN=GPIO_PIN(0, 5)'
# CFLAGS += '-DWS281X_PARAM_NUMOF=1'




# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
