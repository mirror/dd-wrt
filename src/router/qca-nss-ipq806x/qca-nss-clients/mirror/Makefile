# Makefile for NSS MIRROR

ccflags-y += $(NSS_CCFLAGS) -I$(obj)/../exports
ccflags-y += -DNSS_MIRROR_DEBUG_LEVEL=2
ccflags-y += -Wall -Werror

obj-m += qca-nss-mirror.o
qca-nss-mirror-objs := \
		  nss_mirror.o \
		  nss_mirror_ctl.o \
		  nss_mirror_init.o
