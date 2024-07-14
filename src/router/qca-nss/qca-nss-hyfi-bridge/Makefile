################################################################################
#
# Copyright (c) 2012, 2014, 2016 The Linux Foundation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as published by 
# the Free Software Foundation.
#
################################################################################

# Target module name
TARGET := hyfi-bridging

# Kernel directory - This works only for the host.
# The target kernel is different, and it can't be detected.
ifndef KERNELPATH
KERNELPATH := $(LINUXDIR)
endif

HYFI_NF := ./hyfi-netfilter
HYFI_MC := ./hyfi-multicast

# Include directories
INCLUDES := -I$(CURDIR) \
    -I$(KERNELPATH)/include/linux \
    -I$(KERNELPATH)/include/asm \
    -I$(KERNELPATH)/net/bridge \
    -I$(KBUILDPATH)/include/linux \
    -I$(KBUILDPATH)/include/asm \
    -I$(KBUILDPATH)/net/bridge \
    -I$(MDIR)/$(HYFI_NF) \
    -I$(MDIR)/$(HYFI_MC) \
    -I$(STAGING_DIR)/usr/include/qca-ssdk

ifeq ($(strip ${HYFI_BRIDGE_EMESH_ENABLE}),1)
ccflags-y+=-DHYFI_BRIDGE_EMESH_ENABLE
ifneq ($(CONFIG_BUILD_YOCTO),y)
INCLUDES+= -I$(STAGING_DIR)/usr/include/emesh-sp
endif
endif

export EXTRA_CFLAGS += $(INCLUDES)

ifeq ($(CONFIG_BUILD_YOCTO),y)
EXTRA_CFLAGS += $(HYFI_CFLAGS)
endif

# Module extra compilation flags
EXTRA_CFLAGS += -Werror -Wall -Wno-error=implicit-fallthrough -g $(QCACFLAGS)

ifeq ($(strip ${HYBRID_MC_MLD}),1)
ccflags-y+=-DHYBRID_MC_MLD
endif

ifeq ($(strip ${PLC_SUPPORT_NETFILTER}),1)
ccflags-y+=-DPLC_NF_ENABLE
endif

ifeq ($(strip ${DISABLE_SSDK_SUPPORT}),1)
ccflags-y+=-DHYFI_DISABLE_SSDK_SUPPORT
endif

ifeq ($(strip ${HYFI_MULTICAST_SUPPORT}),1)
ccflags-y+=-DHYFI_MULTICAST_SUPPORT
else
ccflags-y+=-DHYFI_MC_STANDALONE_NF
endif

ifeq ($(CONFIG_DISABLE_APS_HOOKS),y)
ccflags-y+=-DDISABLE_APS_HOOKS
endif

# #############################################################################
# Debug flags, set these to = 0 if you want to disable all debugging.
# By turning off debugs you gain maximum performance.
# #############################################################################
ccflags-y += -DHYFI_NF_DEBUG_LEVEL=1
ccflags-y += -DHYFI_MC_DEBUG_LEVEL=1

# Kernel module compilation - part 2
ifneq ($(KERNELRELEASE),)
obj-m += $(TARGET).o

# Target objects - add as many as required
$(TARGET)-objs := \
	$(HYFI_NF)/hyfi_main.o \
	$(HYFI_NF)/hyfi_bridge.o \
	$(HYFI_NF)/hyfi_netfilter.o \
	$(HYFI_NF)/hyfi_netlink.o \
	$(HYFI_NF)/hyfi_fdb.o \
	$(HYFI_NF)/hyfi_notify.o \
	$(HYFI_NF)/hyfi_seamless.o \
	$(HYFI_MC)/mc_main.o \
	$(HYFI_MC)/mc_netlink.o \
	$(HYFI_MC)/mc_netfilter.o \
	$(HYFI_MC)/mc_snooping.o \
	$(HYFI_MC)/mc_forward.o

ifneq ($(CONFIG_DISABLE_APS_HOOKS),y)
$(TARGET)-objs += \
	$(HYFI_NF)/hyfi_hatbl.o \
	$(HYFI_NF)/hyfi_hdtbl.o \
	$(HYFI_NF)/hyfi_aggr.o \
	$(HYFI_NF)/hyfi_ecm.o \
	$(HYFI_MC)/mc_ecm.o


endif

build:
	@$(MAKE) -C $(KERNEL_SRC) M=$(M) V=1 modules

endif
