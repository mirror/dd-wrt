#
# Makefile for Shortcut FE.
#

#obj-m += shortcut-fe.o

ifeq ($(CONFIG_IPV6),y)
SFE_SUPPORT_IPV6=y
endif
ifeq ($(CONFIG_IPV6),m)
SFE_SUPPORT_IPV6=y
endif

#ifdef SFE_SUPPORT_IPV6
#obj-m += shortcut-fe-ipv6.o
#endif

#obj-m += shortcut-fe-cm.o
obj-m += fast-classifier.o

#fast_classifier_test-y += fast-classifier.o

#shortcut-fe-objs := \
#	sfe_ipv4.o

#ifdef SFE_SUPPORT_IPV6
#shortcut-fe-ipv6-objs := \
#	sfe_ipv6.o
#endif
#
#shortcut-fe-cm-objs := \
#	sfe_cm.o


ccflags-$(SFE_SUPPORT_IPV6) += -DSFE_SUPPORT_IPV6
#-DSFE_TOS
ccflags-y += -DCONFIG_NF_CONNTRACK_CHAIN_EVENTS -g