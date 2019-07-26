#
# Makefile for Shortcut FE.
#

obj-m += shortcut-fe.o


ifeq ($(CONFIG_IPV6),y)
SFE_SUPPORT_IPV6=y
endif
ifeq ($(CONFIG_IPV6),m)
SFE_SUPPORT_IPV6=y
endif

ccflags-$(SFE_SUPPORT_IPV6) += -DSFE_SUPPORT_IPV6 
#-DSFE_TOS
ccflags-y += -DCONFIG_NF_CONNTRACK_CHAIN_EVENTS

shortcut-fe-objs := sfe_cm.o

