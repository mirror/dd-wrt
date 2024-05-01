ccflags-y += -I$(obj)/../exports -I$(obj)/.. -I$(obj)/nss_hal/include
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DNSS_WIFI_MESHMGR_DEBUG_LEVEL=4
ccflags-y += -Wall -Werror

obj-m += qca-nss-wifi-meshmgr.o
qca-nss-wifi-meshmgr-objs := nss_wifi_meshmgr.o
