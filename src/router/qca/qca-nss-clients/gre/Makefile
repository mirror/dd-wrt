# Makefile for gre client
ccflags-y += -I$(obj)/../../exports -I$(obj)/../..
ccflags-y += -DNSS_GRE_DEBUG_LEVEL=0
ccflags-y += -Werror
obj-m += qca-nss-gre.o
qca-nss-gre-objs := nss_connmgr_gre.o nss_connmgr_gre_v4.o nss_connmgr_gre_v6.o

obj-y+= test/
