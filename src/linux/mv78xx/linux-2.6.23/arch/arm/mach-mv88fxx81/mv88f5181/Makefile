#*******************************************************************************
# Copyright (C) Marvell International Ltd. and its affiliates
#
# This software file (the "File") is owned and distributed by Marvell 
# International Ltd. and/or its affiliates ("Marvell") under the following
# alternative licensing terms.  Once you have made an election to distribute the
# File under one of the following license alternatives, please (i) delete this
# introductory statement regarding license alternatives, (ii) delete the two
# license alternatives that you have not elected to use and (iii) preserve the
# Marvell copyright notice above.
#
#
#*******************************************************************************
# Marvell GPL License Option
#
# If you received this File from Marvell, you may opt to use, redistribute and/or 
# modify this File in accordance with the terms and conditions of the General 
# Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
# available along with the File in the license.txt file or by writing to the Free 
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
# on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 
#
# THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
# DISCLAIMED.  The GPL License provides additional details about this warranty 
# disclaimer.
#*******************************************************************************/
include $(TOPDIR)/arch/arm/mach-mv88fxx81/mv88f5181/mvRules.mk

# Objects list

COMMON_OBJS        = $(COMMON_DIR)/mvDebug.o $(COMMON_DIR)/mvCommon.o

OSSERVICES_OBJS    = $(OSSERV_LINUX)/mvOsLinux.o $(OSSERV_ARCH_DIR)/mvLinuxArm.o

BOARD_OBJS      = $(BOARD_FLASH_DIR)/mvFlash.o $(BOARD_RTC_DIR)/mvDS1339.o              \
                  $(BOARD_FLASH_DIR)/mvAmdFlash.o $(BOARD_FLASH_DIR)/mvIntelFlash.o     \
                  $(BOARD_FLASH_DIR)/mvFlashCom.o $(BOARD_ENV_DIR)/mvBoardEnvLib.o      \
                  $(BOARD_ENV_PLAT_DIR)/mvBoardEnvSpec.o $(BOARD_DRAM_DIR)/mvDram.o     \
                  $(BOARD_ETHPHY_DIR)/mvEthPhy.o

CONTROLLER_OBJS = $(SOC_CPUIF_DIR)/mvCpuIf.o $(SOC_ENV_DIR)/mvCtrlEnvLib.o              	\
                  $(SOC_ENV_DIR)/mvCtrlEnvAddrDec.o $(SOC_CNTMR_DIR)/mvCntmr.o        		\
                  $(SOC_DEVICE_DIR)/mvDevice.o $(SOC_TWSI_DIR)/mvTwsi.o               		\
                  $(SOC_UART_DIR)/mvUart.o $(SOC_GPP_DIR)/mvGpp.o               		\
                  $(SOC_AHB_TO_MBUS_DIR)/mvAhbToMbus.o $(SOC_DRAM_DIR)/mvDramIf.o     		\
                  $(SOC_PCI_DIR)/mvPci.o $(SOC_PEX_DIR)/mvPex.o $(SOC_PCIIF_DIR)/mvPciIf.o 	\
                  $(SOC_IDMA_DIR)/mvIdma.o $(SOC_ETH_DIR)/mvEth.o $(SOC_ETH_DIR)/mvEthAddrDec.o	\
                  $(SOC_CPU_PLAT_DIR)/mvCpuArm.o $(SOC_USB_DIR)/mvUsb.o $(SOC_XOR_DIR)/mvXor.o

CESA_OBJS       = $(SOC_CESA_DIR)/mvCesa.o $(SOC_CESA_DIR)/mvCesaDebug.o                        \
              $(SOC_CESA_DIR)/mvMD5.o $(SOC_CESA_DIR)/mvSHA1.o $(SOC_CESA_DIR)/mvLru.o      \
              $(SOC_CESA_AES_DIR)/mvAesAlg.o   $(SOC_CESA_AES_DIR)/mvAesApi.o

CESA_EXTRA_OBJS = $(LSP_CESA_DIR)/cesa_dev.o

ifeq ($(CONFIG_MV_CESA_TEST),y)
CESA_EXTRA_OBJS += $(SOC_CESA_DIR)/mvCesaTest.o
EXTRA_CFLAGS += -DCONFIG_MV_CESA_TEST
endif

ifeq ($(CONFIG_MV_CESA_OCF),y)
CESA_OBJS       += $(LSP_CESA_DIR)/cesa_ocf_drv.o
EXTRA_CFLAGS    += -I$(TOPDIR)/crypto/ocf
endif

LSP_OBJS        = $(LSP_DIR)/core.o $(LSP_DIR)/irq.o $(LSP_DIR)/mm.o $(LSP_DIR)/time.o  \
                  $(LSP_DIR)/leds.o $(LSP_DIR)/usb.o 

QD_OBJS         = $(QD_DIR)/src/driver/gtDrvConfig.o $(QD_DIR)/src/driver/gtDrvEvents.o \
                    $(QD_DIR)/src/driver/gtHwCntl.o $(QD_DIR)/src/platform/gtMiiSmiIf.o\
                    $(QD_DIR)/src/platform/platformDeps.o $(QD_DIR)/src/platform/gtSem.o\
                    $(QD_DIR)/src/platform/gtDebug.o $(QD_DIR)/src/msapi/gtBrgFdb.o \
                    $(QD_DIR)/src/msapi/gtBrgStp.o $(QD_DIR)/src/msapi/gtBrgVlan.o \
                    $(QD_DIR)/src/msapi/gtEvents.o $(QD_DIR)/src/msapi/gtPortCtrl.o \
                    $(QD_DIR)/src/msapi/gtPortStat.o $(QD_DIR)/src/msapi/gtPortStatus.o \
                    $(QD_DIR)/src/msapi/gtQosMap.o $(QD_DIR)/src/msapi/gtPIRL.o  \
                    $(QD_DIR)/src/msapi/gtPhyCtrl.o $(QD_DIR)/src/msapi/gtPhyInt.o \
                    $(QD_DIR)/src/msapi/gtSysConfig.o $(QD_DIR)/src/msapi/gtSysCtrl.o\
                    $(QD_DIR)/src/msapi/gtVersion.o $(QD_DIR)/src/msapi/gtUtils.o \
                    $(QD_DIR)/src/msapi/gtBrgVtu.o $(QD_DIR)/src/msapi/gtPortRmon.o \
                    $(QD_DIR)/src/msapi/gtSysStatus.o $(QD_DIR)/src/msapi/gtPortRateCtrl.o \
                    $(QD_DIR)/src/msapi/gtPortPav.o $(QD_DIR)/src/msapi/gtVct.o 

ifeq ($(CONFIG_MV_DMA_COPYUSER),y)
LSP_OBJS +=  $(LSP_DIR)/dma.o
endif

mv88f5181-objs  := $(LSP_OBJS) $(COMMON_OBJS) $(OSSERVICES_OBJS) $(BOARD_OBJS) $(CONTROLLER_OBJS)

cesa_extra-objs := $(CESA_EXTRA_OBJS)
obj-$(CONFIG_MV_CESA_TOOL)  += cesa_extra.o

ifeq ($(CONFIG_MV_CESA),y)
mv88f5181-objs  += $(CESA_OBJS)
endif

ifeq ($(CONFIG_PCI),y)
mv88f5181-objs  += $(LSP_DIR)/pci.o
endif

ifeq ($(CONFIG_MV88fxx81_PROC),y)
mv88f5181-objs  += $(LSP_DIR)/proc.o
endif

ifeq ($(CONFIG_EGIGA_MULTI_Q),y)
mv88f5181-objs  += $(SOC_ETH_DIR)/mvEthPolicy.o
endif

ifeq ($(CONFIG_EGIGA_PROC),y)
mv88f5181-objs  += $(SOC_ETH_DIR)/mvEthDebug.o
endif

ifeq ($(CONFIG_MV_GATEWAY),y)
mv88f5181-objs  += $(QD_OBJS)
endif

ifeq ($(CONFIG_MV_GTW_PROC),y)
mv88f5181-objs  += $(SOC_ETH_DIR)/mvEthDebug.o
endif

ifeq ($(CONFIG_MV_DBG_TRACE),y)
mv88f5181-objs  += $(LSP_DIR)/dbg-trace.o
endif

#mv88f5181-objs :=

obj-y           := mv88f5181.o
