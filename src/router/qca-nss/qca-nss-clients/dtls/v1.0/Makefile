# Makefile for DTLS manager

ccflags-y += $(NSS_CCFLAGS) -I$(obj)/../../exports
ccflags-y += -DNSS_DTLSMGR_DEBUG_LEVEL=0

obj-m += qca-nss-dtlsmgr.o
qca-nss-dtlsmgr-objs := nss_connmgr_dtls.o nss_connmgr_dtls_netdev.o
