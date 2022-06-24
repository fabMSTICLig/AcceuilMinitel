# name of your application
APPLICATION = minitel

# Use the ST B-L072Z-LRWAN1 board by default:
BOARD ?= idosens_remote

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)../../RIOT

# Pass these enviroment variables to Lora
DEVEUI ?= 361eab7573a48cc2
APPEUI ?= 0000000000000000
APPKEY ?= 7454f5caa2752d7f894eec0426350e33
DOCKER_ENV_VARS += DEVEUI
DOCKER_ENV_VARS += APPEUI
DOCKER_ENV_VARS += APPKEY


# Default radio driver is Semtech SX1276 (used by the B-L072Z-LRWAN1 board)
DRIVER ?= sx1276

# Default region is Europe and default band is 868MHz
LORA_REGION ?= EU868

# Include the Semtech-loramac package
USEPKG += semtech-loramac

USEMODULE += $(DRIVER)
USEMODULE += fmt
USEMODULE += ztimer_sec
USEMODULE += ztimer_msec
USEMODULE += ztimer_no_periph_rtt

FEATURES_REQUIRED +=  periph_uart periph_uart_modecfg
FEATURES_OPTIONAL += periph_rtc


# INCLUDES
INCLUDES += -I$(CURDIR)/include

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include


ifndef CONFIG_KCONFIG_USEMODULE_LORAWAN
  # OTAA compile time configuration keys
  CFLAGS += -DCONFIG_LORAMAC_APP_KEY_DEFAULT=\"$(APPKEY)\"
  CFLAGS += -DCONFIG_LORAMAC_APP_EUI_DEFAULT=\"$(APPEUI)\"
  CFLAGS += -DCONFIG_LORAMAC_DEV_EUI_DEFAULT=\"$(DEVEUI)\"
endif

#Baudrate par défault du minitel
#CFLAGS+=-DSTDIO_UART_BAUDRATE=1200