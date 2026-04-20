# Makefile for mscs client
ccflags-y += -I$(obj)/../exports -I$(obj)/..
ccflags-y += -DNSS_CLIENT_BUILD_ID="$(BUILD_ID)"
ccflags-y += -DNSS_MSCS_DEBUG_LEVEL=2
ccflags-y += -Wall -Werror
obj-m += qca-nss-mscs.o
qca-nss-mscs-objs := nss_mscs.o
