# Makefile for custom gre test module
ccflags-y += -I$(obj)/../../../exports -I$(obj)/../../.. -I$(obj)/../
ccflags-y += -DNSS_GRE_DEBUG_LEVEL=0
ccflags-y += -Wall -Werror
obj-m +=  qca-nss-gre-test.o
qca-nss-gre-test-objs := nss_connmgr_gre_test.o
