# Makefile for IGS (Ingress Shaping)

ccflags-y += $(NSS_CCFLAGS) -I$(obj)/../../exports
ccflags-y += -DNSS_IGS_DEBUG_LEVEL=2
ccflags-y += -Wall -Werror

obj-m += act_nssmirred.o
act_nssmirred-objs := \
		      nss_mirred.o \
		      nss_ifb.o
