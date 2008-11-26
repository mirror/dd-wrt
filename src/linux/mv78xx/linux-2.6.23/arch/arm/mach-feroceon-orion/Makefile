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
include 	  $(TOPDIR)/arch/arm/mach-feroceon-orion/config/mvRules.mk

# Objects list
COMMON_OBJS	= $(COMMON_DIR)/mvDebug.o $(COMMON_DIR)/mvCommon.o $(COMMON_DIR)/mvStack.o

OSSERVICES_OBJS	= $(OSSERV_DIR)/mvOs.o

HAL_OBJS	= $(HAL_RTC_DIR)/mvDS1339.o $(HAL_DRAM_DIR)/mvDram.o 		\
                  $(HAL_CNTMR_DIR)/mvCntmr.o 					\
                  $(HAL_TWSI_DIR)/mvTwsi.o  $(HAL_DRAM_DIR)/mvDramIf.o       	\
                  $(HAL_UART_DIR)/mvUart.o $(HAL_GPP_DIR)/mvGpp.o               \
		  $(HAL_ETHPHY_DIR)/mvEthPhy.o $(HAL_ETH_DIR)/mvEth.o 		\
		  $(HAL_ETH_DIR)/mvEthDebug.o $(HAL_ETH_DIR)/../mvEthPolicy.o

ORION_FAM_OBJS	= $(BOARD_ENV_DIR)/mvBoardEnvSpec.o $(SOC_ENV_DIR)/mvCtrlEnvLib.o	\
		  $(BOARD_ENV_DIR)/mvBoardEnvLib.o $(SOC_ENV_DIR)/mvCtrlEnvAddrDec.o 	\
		  $(SOC_SYS_DIR)/mvAhbToMbus.o $(SOC_SYS_DIR)/mvCpuIf.o 		\
		  $(SOC_ENV_DIR)/mvCtrlEnvPadCalibration.o $(SOC_SYS_DIR)/mvSysPci.o	\
		  $(SOC_SYS_DIR)/mvSysPex.o $(SOC_DEVICE_DIR)/mvDevice.o 		\
		  $(SOC_CPU_DIR)/mvCpuArm.o
                  
QD_OBJS		= $(HAL_QD_DIR)/src/driver/gtDrvConfig.o $(HAL_QD_DIR)/src/driver/gtDrvEvents.o \
                  $(HAL_QD_DIR)/src/driver/gtHwCntl.o $(HAL_QD_DIR)/src/platform/gtMiiSmiIf.o	\
                  $(HAL_QD_DIR)/src/platform/platformDeps.o $(HAL_QD_DIR)/src/platform/gtSem.o	\
                  $(HAL_QD_DIR)/src/platform/gtDebug.o $(HAL_QD_DIR)/src/msapi/gtBrgFdb.o 	\
                  $(HAL_QD_DIR)/src/msapi/gtBrgStp.o $(HAL_QD_DIR)/src/msapi/gtBrgVlan.o 	\
                  $(HAL_QD_DIR)/src/msapi/gtEvents.o $(HAL_QD_DIR)/src/msapi/gtPortCtrl.o 	\
                  $(HAL_QD_DIR)/src/msapi/gtPortStat.o $(HAL_QD_DIR)/src/msapi/gtPortStatus.o 	\
                  $(HAL_QD_DIR)/src/msapi/gtQosMap.o $(HAL_QD_DIR)/src/msapi/gtPIRL.o  		\
                  $(HAL_QD_DIR)/src/msapi/gtPhyCtrl.o $(HAL_QD_DIR)/src/msapi/gtPhyInt.o 	\
                  $(HAL_QD_DIR)/src/msapi/gtSysConfig.o $(HAL_QD_DIR)/src/msapi/gtSysCtrl.o	\
                  $(HAL_QD_DIR)/src/msapi/gtVersion.o $(HAL_QD_DIR)/src/msapi/gtUtils.o 	\
                  $(HAL_QD_DIR)/src/msapi/gtBrgVtu.o $(HAL_QD_DIR)/src/msapi/gtPortRmon.o 	\
                  $(HAL_QD_DIR)/src/msapi/gtSysStatus.o $(HAL_QD_DIR)/src/msapi/gtPortRateCtrl.o\
                  $(HAL_QD_DIR)/src/msapi/gtPortPav.o $(HAL_QD_DIR)/src/msapi/gtVct.o		\
		  $(HAL_QD_DIR)/src/msapi/gtPIRL2.o

LSP_OBJS        = $(LSP_DIR)/core.o $(LSP_DIR)/irq.o $(LSP_DIR)/time.o  	\
                  $(LSP_DIR)/leds.o $(LSP_DIR)/sysmap.o  $(LSP_DIR)/rtc.o 	\
		  $(LSP_DIR)/export.o

obj-y   				:=  feroceon.o
feroceon-objs  				:=$(LSP_OBJS) $(COMMON_OBJS) $(OSSERVICES_OBJS) $(HAL_OBJS) 	\
					  $(ORION_FAM_OBJS)

feroceon-$(CONFIG_MV_INCLUDE_IDMA) 	+= $(HAL_IDMA_DIR)/mvIdma.o $(SOC_SYS_DIR)/mvSysIdma.o
feroceon-$(CONFIG_MV_INCLUDE_XOR) 	+= $(HAL_XOR_DIR)/mvXor.o $(SOC_SYS_DIR)/mvSysXor.o
feroceon-$(CONFIG_MV_INCLUDE_PCI) 	+= $(HAL_PCI_DIR)/mvPci.o $(HAL_PCIIF_DIR)/mvPciIf.o
feroceon-$(CONFIG_MV_INCLUDE_PEX) 	+= $(HAL_PEX_DIR)/mvPex.o
feroceon-$(CONFIG_MV_INCLUDE_USB) 	+= $(HAL_USB_DIR)/mvUsb.o $(SOC_SYS_DIR)/mvSysUsb.o
feroceon-$(CONFIG_MV_INCLUDE_GIG_ETH)   += $(SOC_SYS_DIR)/mvSysGbe.o
feroceon-$(CONFIG_MV_FAST_PATH)         += $(HAL_ETH_FP_DIR)/mvFastPath.o  $(HAL_ETH_FP_DIR)/mvFpNat.o
feroceon-$(CONFIG_MV_INCLUDE_CESA) 	+= $(HAL_CESA_DIR)/mvCesa.o $(HAL_CESA_DIR)/mvCesaDebug.o 	\
                        		   $(HAL_CESA_DIR)/mvMD5.o $(HAL_CESA_DIR)/mvSHA1.o 		\
					   $(HAL_CESA_AES_DIR)/mvAesAlg.o $(HAL_CESA_AES_DIR)/mvAesApi.o\
					   $(SOC_SYS_DIR)/mvSysCesa.o
feroceon-$(CONFIG_MV_CESA_TEST) 	+= $(HAL_CESA_DIR)/mvCesaTest.o
feroceon-$(CONFIG_MV_INCLUDE_INTEG_SATA)+= $(SOC_SYS_DIR)/mvSysSata.o $(HAL_SATA_DIR)/mvSataSoc.o
feroceon-$(CONFIG_MV_INCLUDE_INTEG_MFLASH)+= $(HAL_MFLASH_DIR)/mvMFlash.o $(HAL_MFLASH_DIR)/mvPMFlash.o \
                         		   $(HAL_MFLASH_DIR)/mvSMFlash.o
feroceon-$(CONFIG_MV_INCLUDE_SPI) 	+= $(HAL_SPI_DIR)/mvSpi.o $(HAL_SPI_DIR)/mvSpiCmnd.o 		\
                         		   $(HAL_SFLASH_DIR)/mvSFlash.o
feroceon-$(CONFIG_MV_INCLUDE_AUDIO) 	+= $(HAL_AUDIO_DIR)/mvAudio.o $(SOC_SYS_DIR)/mvSysAudio.o
feroceon-$(CONFIG_MV_INCLUDE_TDM) 	+= $(SOC_SYS_DIR)/mvSysTdm.o

# drivers part
obj-$(CONFIG_MV_USE_IDMA_ENGINE) 	+= ../plat-feroceon/mv_drivers_lsp/mv_dma/
obj-$(CONFIG_MV_USE_XOR_ENGINE) 	+= ../plat-feroceon/mv_drivers_lsp/mv_xor/
obj-$(CONFIG_MV_CESA) 			+= ../plat-feroceon/mv_drivers_lsp/mv_cesa/
feroceon-$(CONFIG_MV_FAST_PATH) 	+= ../plat-feroceon/mv_drivers_lsp/mv_network/fast_path_mgr/mv_fast_path_manager.o

# The rest of the drivers are compiled through the driver dir directly.


# LSP part
feroceon-$(CONFIG_MV_INCLUDE_USB)    	+= $(LSP_DIR)/usb.o
feroceon-$(CONFIG_PCI)  		+= $(LSP_DIR)/pci.o
feroceon-$(CONFIG_FEROCEON_PROC) 	+= $(LSP_DIR)/proc.o
feroceon-$(CONFIG_MV_GATEWAY) 		+= $(QD_OBJS)
feroceon-$(CONFIG_MV_DBG_TRACE) 	+= $(LSP_DIR)/dbg-trace.o

