# Because of periph_i2c and periph_rtt
CONTINUE_ON_EXPECTED_ERRORS=1

# name of your application
APPLICATION = runtime_config

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
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
USEMODULE += saul_default

USEMODULE += tinycbor_float
USEMODULE += xtimer

USEMODULE += periph_uart







# Riot registry enable int64 and float support
CFLAGS += -DCONFIG_REGISTRY_USE_INT64=1
CFLAGS += -DCONFIG_REGISTRY_USE_FLOAT=1

# External modules
USEMODULE += registry
USEMODULE += registry_coap
USEMODULE += registry_lwm2m
EXTERNAL_MODULE_DIRS += external_modules
# LwM2M logging
CFLAGS += -DCONFIG_LWM2M_WITH_LOGS=1
# Specific the server URI  address (NOTE: Domain names not supported yet)
ifndef CONFIG_LWM2M_SERVER_URI
  CFLAGS += -DCONFIG_LWM2M_SERVER_URI='"coap://[fe80::845c:97ff:feec:c1c6]:5683"'
endif
ifndef CONFIG_LWM2M_DEVICE_NAME
  CFLAGS += -DCONFIG_LWM2M_DEVICE_NAME='"riot-device"'
endif













USEPKG += tinycbor
USEPKG += minmea

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include

# For now this goes after the inclusion of Makefile.include so Kconfig symbols
# are available. Only set configuration via CFLAGS if Kconfig is not being used
# for this module.
ifndef CONFIG_KCONFIG_MODULE_GCOAP
## Uncomment to redefine port, for example use 61616 for RFC 6282 UDP compression.
#GCOAP_PORT = 5683
#CFLAGS += -DCONFIG_GCOAP_PORT=$(GCOAP_PORT)

## Uncomment to redefine request token length, max 8.
#GCOAP_TOKENLEN = 2
#CFLAGS += -DCONFIG_GCOAP_TOKENLEN=$(GCOAP_TOKENLEN)

# Increase from default for confirmable block2 follow-on requests
GCOAP_RESEND_BUFS_MAX ?= 2
CFLAGS += -DCONFIG_GCOAP_RESEND_BUFS_MAX=$(GCOAP_RESEND_BUFS_MAX)
endif
