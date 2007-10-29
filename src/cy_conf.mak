KROMOGUI=y
#OMNI_WIFI=1

#MULTICAST_SUPPORT=1
#CONFIG_MULTICAST=y

WEB_PAGE=dd-wrt

DNSMASQ_SUPPORT=1
CONFIG_DNSMASQ=y
UDHCPD_SUPPORT=1
CONFIG_UDHCPD=y
UDHCPC_SUPPORT=1
CONFIG_UDHCPC=y

DDNS_SUPPORT=1
CONFIG_DDNS=y
CRON_SUPPORT=1
CONFIG_CRON=y
HTTPD_SUPPORT=1
CONFIG_HTTPD=y
GET_POST_SUPPORT=1
CONFIG_GET_POST=y

ifeq (1,1)
HTTPS_SUPPORT=1
CONFIG_HTTPS=y
WRITE_MAC_SUPPORT=1
CONFIG_WRITE_MAC=y
DIAG_SUPPORT=1
CONFIG_DIAG=y

endif

SPEED_BOOSTER_SUPPORT=1
CONFIG_SPEED_BOOSTER=y

ifeq (0,1)
endif


ifeq (0,1)
endif


#CY_DEPS := $(SRCBASE)/include/code_pattern.h $(SRCBASE)/include/cyutils.h $(SRCBASE)/include/cymac.h $(SRCBASE)/cy_conf.h $(SRCBASE)/cy_conf.mak Makefile $(shell find -iname "*.h")

#CY_DEPS_ := $(SRCBASE_)/include/code_pattern.h $(SRCBASE_)/include/cyutils.h $(SRCBASE_)/include/cymac.h $(SRCBASE_)/cy_conf.h $(SRCBASE_)/cy_conf.mak Makefile $(shell find -iname "*.h")
