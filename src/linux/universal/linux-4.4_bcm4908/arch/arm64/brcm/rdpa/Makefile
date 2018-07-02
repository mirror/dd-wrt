#***********************************************************************
#
#  Copyright (c) 2015  Broadcom Corporation
#  All Rights Reserved
#
#
# Makefile for Runner Data Path API GPL (RDPA GPL) kernel module 
#
#***********************************************************************
EXTRA_CFLAGS	+= -DBDMF_SYSTEM_LINUX -DBDMF_SHELL -DBDMF_HISTORY -DWL4908 -DCONFIG_BCM4908 
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx  -I$(srctree)/arch/arm64/brcm/include/shared
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/rdpa/include
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/rdpa/include/autogen

obj-y = rdpa.o

rdpa-objs += rdpa_gpl_cpu.o           rdpa_gpl_egress_tm.o  rdpa_gpl_handwritten.o    rdpa_gpl_ipsec.o     rdpa_gpl_llid.o         rdpa_gpl_pbit_to_queue.o  rdpa_gpl_spdsvc.o       rdpa_gpl_tcont.o  rdpa_gpl_vlan_action.o
rdpa-objs += rdpa_gpl_dhd_helper.o    rdpa_gpl_filter.o     rdpa_gpl_ingress_class.o  rdpa_gpl_iptv.o      rdpa_gpl_mcast.o        rdpa_gpl_policer.o        rdpa_gpl_system.o       rdpa_gpl_ucast.o  rdpa_gpl_wlan_mcast.o
rdpa-objs += rdpa_gpl_bridge.o  rdpa_gpl_dscp_to_pbit.o  rdpa_gpl_gem.o        rdpa_gpl_ip_class.o       rdpa_gpl_l2_ucast.o  rdpa_gpl_pbit_to_gem.o  rdpa_gpl_port.o             rdpa_gpl_vlan.o   rdpa_gpl_xtm.o
#rdpa_gpl_tc_to_queue.o
ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
EXTRA_CFLAGS += -DCONFIG_DHD_RUNNER
endif
EXTRA_DEFINES += -DPHYS_ADDR_64BIT

EXTRA_CFLAGS += -include rdpa_api.h
