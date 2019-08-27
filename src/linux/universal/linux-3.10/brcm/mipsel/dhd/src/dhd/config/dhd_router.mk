# Helper makefile for building Broadcom dongle host driver (DHD) for
# router platforms
# This file maps dhd feature flags DHDFLAGS(import) and DHDFILES_SRC(export).
#
# Copyright (C) 2015, Broadcom Corporation. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id: dhd.mk 387757 2013-02-27 01:09:03Z $ $(dir $(lastword $(MAKEFILE_LIST)))/../../makefiles/WLAN_Common.mk

# Let the mogrifier to handle it in router platform 

#ifdef WLTEST
ifeq ($(WLTEST),1)
    DHDFLAGS += -DWLTEST -DIOCTL_RESP_TIMEOUT=20000
endif
#endif

#ifdef BCMDBG
ifeq ($(BCMDBG),1)
    DHDFLAGS += -DBCMDBG
endif
#endif



ifeq ($(BCMQT),1)
    DHDFLAGS += -DBCMSLTGT -DBCMQT
endif

# Common Flags
#DHDFLAGS        += -Werror
DHDFLAGS        += -DLINUX
DHDFLAGS        += -DBCMDRIVER
DHDFLAGS        += -DBCMDONGLEHOST
DHDFLAGS	+= -DWAR4360_UCODE
# PCIe specific flags
DHDFLAGS        += -DPCIE_FULL_DONGLE
DHDFLAGS        += -DBCMPCIE
DHDFLAGS        += -DCUSTOM_DPC_PRIO_SETTING=-1
DHDFLAGS        += -DBCM_ROUTER_DHD
DHDFLAGS        += -DDHD_DEBUG
DHDFLAGS        += -DBCMEMBEDIMAGE=\"rtecdc_router.h\"
DHDFLAGS        += -DDHD_UNICAST_DHCP
DHDFLAGS        += -DDHD_L2_FILTER
DHDFLAGS        += -DQOS_MAP_SET
DHDFLAGS        += -DDHD_PSTA
DHDFLAGS        += -DDHD_WET
DHDFLAGS        += -DDHD_MCAST_REGEN

#M2M host memory allocation
DHDFLAGS		+= -DBCM_HOST_MEM_SCB -DDMA_HOST_BUFFER_LEN=0x80000

#ifneq ($(CONFIG_LBR_AGGR),)
DHDFLAGS        += -DDHD_LBR_AGGR_BCM_ROUTER
#endif
ifeq ($(BCM_BUZZZ),1)
DHDFLAGS        += -DBCM_BUZZZ
endif

# Dongle DMAs indices to from TCM.
# If BCM_INDX_DMA is defined, then dongle MUST support DMAing of indices.
# When not defined, DHD learns dongle's DMAing capability and adopts the
# advertized RD/WR index size.
# DHDFLAGS        += -DBCM_INDX_DMA

# WMF Specific Flags
#ifneq ($(CONFIG_EMF_ENABLED),)
#DHDFLAGS        += -DDHD_WMF
#DHDFLAGS        += -DDHD_IGMP_UCQUERY
#DHDFLAGS        += -DDHD_UCAST_UPNP
#endif

# DHD Include Paths
DHDIFLAGS       += -I$(SRCBASE_DHD)/include
DHDIFLAGS       += -I$(SRCBASE_DHD)/../components/shared
DHDIFLAGS       += -I$(SRCBASE_DHD)/common/include
DHDIFLAGS       += -I$(SRCBASE_DHD)/shared
DHDIFLAGS       += -I$(SRCBASE_DHD)/shared/bcmwifi/include
DHDIFLAGS       += -I$(SRCBASE_DHD)/dhd/sys
DHDIFLAGS       += -I$(SRCBASE_DHD)/dongle/include
DHDIFLAGS       += -I$(SRCBASE_DHD)/wl/sys


# DHD Source files - For pciefd-msgbuf target
DHDFILES_SRC    := src/shared/aiutils.c
DHDFILES_SRC    += src/shared/bcmevent.c
DHDFILES_SRC    += src/shared/bcm_l2_filter.c
DHDFILES_SRC    += src/shared/bcmotp.c
DHDFILES_SRC    += src/shared/bcmsrom.c
DHDFILES_SRC    += src/shared/bcmutils.c
DHDFILES_SRC    += src/shared/hnd_pktq.c
DHDFILES_SRC    += src/shared/hnd_pktpool.c
DHDFILES_SRC    += src/shared/hndpmu.c
DHDFILES_SRC    += src/shared/sbutils.c
DHDFILES_SRC    += src/shared/siutils.c
DHDFILES_SRC    += src/shared/pcie_core.c
DHDFILES_SRC    += src/shared/bcm_psta.c
DHDFILES_SRC    += src/dhd/sys/dhd_common.c
DHDFILES_SRC    += src/dhd/sys/dhd_custom_gpio.c
DHDFILES_SRC    += src/dhd/sys/dhd_ip.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux_platdev.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux_sched.c
DHDFILES_SRC    += src/dhd/sys/dhd_linux_wq.c
#ifneq ($(CONFIG_LBR_AGGR),)
DHDFILES_SRC    += src/dhd/sys/dhd_aggr.c
DHDFILES_SRC    += src/dhd/sys/dhd_lbr_aggr_linux.c
#endif
DHDFILES_SRC    += src/dhd/sys/dhd_msgbuf.c
DHDFILES_SRC    += src/dhd/sys/dhd_flowring.c
DHDFILES_SRC    += src/dhd/sys/dhd_pcie.c
DHDFILES_SRC    += src/dhd/sys/dhd_pcie_linux.c
#ifneq ($(CONFIG_EMF_ENABLED)),)
#DHDFILES_SRC    += src/dhd/sys/dhd_wmf_linux.c
#endif
DHDFILES_SRC    += src/dhd/sys/dhd_l2_filter.c
DHDFILES_SRC    += src/dhd/sys/dhd_psta.c
DHDFILES_SRC    += src/dhd/sys/dhd_wet.c

DHD_OBJS := $(sort $(patsubst %.c,%.o,$(addprefix $(SRCBASE_OFFSET)/,$(patsubst src/%,%,$(DHDFILES_SRC)))))

EXTRA_CFLAGS += $(DHDFLAGS) $(DHDIFLAGS)
