# Helper makefile for building Broadcom dongle host driver (DHD) for
# router platforms
# This file maps dhd feature flags DHDFLAGS(import) and DHDFILES_SRC(export).
#
# Copyright (C) 2016, Broadcom. All Rights Reserved.
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


#ifdef BCMDBG_MEM
ifeq ($(BCMDBG_MEM),1)
    DHDFLAGS += -DBCMDBG_MEM
endif
#endif

#ifdef BCMDBG_ASSERT
ifeq ($(BCMDBG_ASSERT),1)
    DHDFLAGS += -DBCMDBG_ASSERT
endif
#endif

ifeq ($(BCMQT),1)
    DHDFLAGS += -DBCMSLTGT -DBCMQT
endif

# Common Flags
DHDFLAGS        += -Werror
DHDFLAGS        += -DLINUX
DHDFLAGS        += -DBCMDRIVER
DHDFLAGS        += -DBCMDONGLEHOST

ifneq ($(strip $(BCA_HNDROUTER)),)
DHDFLAGS        += -DDHDAP
DHDFLAGS        += -DPKTC
DHDFLAGS        += -DBCM47XX
# NBUFF (fkb/skb)
ifneq ($(strip $(CONFIG_BCM_KF_NBUFF)),)
DHDFLAGS	+= -DBCM_NBUFF
endif
# BLOG/FCACHE
ifneq ($(strip $(CONFIG_BLOG)),)
DHDFLAGS	+= -DBCM_BLOG
endif
DHDIFLAGS	+= -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
DHDIFLAGS	+= -I$(BRCMDRIVERS_DIR)/broadcom/char/wlcsm_ext/impl1/include
DHDIFLAGS	+= -I$(BRCMDRIVERS_DIR)/opensource/include/bcm963xx
# WFD (WiFi Forwarding Driver)
ifneq ($(strip $(CONFIG_BCM_WIFI_FORWARDING_DRV)),)
DHDFLAGS += -DBCM_WFD
DHDFLAGS += -DBCM_GMAC3
DHDFLAGS += -DBCM_NO_WOFA
DHDIFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
endif
#
# DHD Runner Offload
#   Internally Enables DHD_D2H_SOFT_DOORBELL_SUPPORT
#   Internally Disables CONFIG_BCM_WFD_CHAIN_SUPPORT, DHD_RX_CHAINING
#
ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
DHDFLAGS += -DBCM_DHD_RUNNER

# Runner interface include paths and flags
DHDIFLAGS += -I$(INC_RDPA_MW_PATH) $(INC_RDP_FLAGS)
endif

endif

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

ifneq ($(CONFIG_LBR_AGGR),)
DHDFLAGS        += -DDHD_LBR_AGGR_BCM_ROUTER
endif

#M2M host memory allocation
DHDFLAGS		+= -DBCM_HOST_MEM_SCB -DDMA_HOST_BUFFER_LEN=0x80000

ifeq ($(BCM_BUZZZ),1)
DHDFLAGS        += -DBCM_BUZZZ
endif

# Dongle DMAs indices to from TCM.
# If BCM_INDX_DMA is defined, then dongle MUST support DMAing of indices.
# When not defined, DHD learns dongle's DMAing capability and adopts the
# advertized RD/WR index size.
# DHDFLAGS        += -DBCM_INDX_DMA

# WMF Specific Flags
DHDFLAGS        += -DDHD_WMF
DHDFLAGS        += -DDHD_IGMP_UCQUERY
DHDFLAGS        += -DDHD_UCAST_UPNP

# DHD Include Paths
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/../components/shared
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/common/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/shared
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/shared/bcmwifi/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/dhd/sys
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/dongle/include
DHDIFLAGS       += -I$(SRCBASE)/../../$(SRCBASE_DHD)/wl/sys

# DHD Source files - For pciefd-msgbuf target
DHDFILES_SRC    := src/shared/aiutils.c
DHDFILES_SRC    += src/shared/bcmevent.c
DHDFILES_SRC    += src/shared/bcm_l2_filter.c
DHDFILES_SRC    += src/shared/bcmotp.c
DHDFILES_SRC    += src/shared/bcmsrom.c
DHDFILES_SRC    += src/shared/bcmutils.c
DHDFILES_SRC    += src/shared/bcmxtlv.c
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
ifneq ($(CONFIG_LBR_AGGR),)
DHDFILES_SRC    += src/dhd/sys/dhd_aggr.c
DHDFILES_SRC    += src/dhd/sys/dhd_lbr_aggr_linux.c
endif
DHDFILES_SRC    += src/dhd/sys/dhd_msgbuf.c
DHDFILES_SRC    += src/dhd/sys/dhd_flowring.c
DHDFILES_SRC    += src/dhd/sys/dhd_pcie.c
DHDFILES_SRC    += src/dhd/sys/dhd_pcie_linux.c
DHDFILES_SRC    += src/dhd/sys/dhd_wmf_linux.c
DHDFILES_SRC    += src/dhd/sys/dhd_l2_filter.c
DHDFILES_SRC    += src/dhd/sys/dhd_psta.c
DHDFILES_SRC    += src/dhd/sys/dhd_wet.c

ifeq ($(strip $(USE_WLAN_SHARED)), 1)
DHDIFLAGS     += -I$(BRCMDRIVERS_DIR)/broadcom/net/wl/shared/impl$(WLAN_SHARED_IMPL)
WLAN_DHD_PATH := ../../../shared/impl$(WLAN_SHARED_IMPL)
else
WLAN_DHD_PATH := src/dhd/sys
endif

ifneq ($(strip $(BCA_HNDROUTER)),)
ifneq ($(strip $(CONFIG_BCM_KF_NBUFF)),)
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_nbuff.c
endif
ifneq ($(strip $(CONFIG_BLOG)),)
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_blog.c
endif
ifneq ($(strip $(CONFIG_BCM_WIFI_FORWARDING_DRV)),)
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_wfd.c
endif
ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
DHDFILES_SRC    += $(WLAN_DHD_PATH)/dhd_runner.c
endif
DHDFILES_SRC    += src/shared/linux_osl.c
DHDFILES_SRC    += src/shared/nvram_ext.c
endif

DHD_OBJS := $(sort $(patsubst %.c,%.o,$(addprefix $(SRCBASE_OFFSET)/,$(patsubst src/%,%,$(DHDFILES_SRC)))))

EXTRA_CFLAGS += $(DHDFLAGS) $(DHDIFLAGS)
