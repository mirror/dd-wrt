# Makefile for eogre client
ccflags-y := -I$(obj)/../exports -I$(obj)/.. \
	-DNSS_CLIENT_BUILD_ID="$(BUILD_ID)" -DNSS_EOGREMGR_DEBUG_LEVEL=2 -Wall -Werror

obj-m += qca-nss-eogremgr.o
qca-nss-eogremgr-objs := nss_eogremgr.o
