# SPDX-License-Identifier: GPL-2.0
#
# linux/drivers/maps/Makefile
#

EXTRA_CFLAGS := -g

ifeq ($(CONFIG_MTD_COMPLEX_MAPPINGS),y)
obj-$(CONFIG_MTD)		+= map_funcs.o
endif

# Chip mappings
obj-$(CONFIG_MTD_CFI_FLAGADM)	+= cfi_flagadm.o
obj-$(CONFIG_MTD_DC21285)	+= dc21285.o
obj-$(CONFIG_MTD_L440GX)	+= l440gx.o
obj-$(CONFIG_MTD_AMD76XROM)	+= amd76xrom.o
obj-$(CONFIG_MTD_ESB2ROM)	+= esb2rom.o
obj-$(CONFIG_MTD_ICHXROM)	+= ichxrom.o
obj-$(CONFIG_MTD_CK804XROM)	+= ck804xrom.o
obj-$(CONFIG_MTD_TSUNAMI)	+= tsunami_flash.o
obj-$(CONFIG_MTD_PXA2XX)	+= pxa2xx-flash.o
physmap-objs-y			+= physmap-core.o
physmap-objs-$(CONFIG_MTD_PHYSMAP_BT1_ROM) += physmap-bt1-rom.o
physmap-objs-$(CONFIG_MTD_PHYSMAP_VERSATILE) += physmap-versatile.o
physmap-objs-$(CONFIG_MTD_PHYSMAP_GEMINI) += physmap-gemini.o
physmap-objs-$(CONFIG_MTD_PHYSMAP_IXP4XX) += physmap-ixp4xx.o
physmap-objs			:= $(physmap-objs-y)
obj-$(CONFIG_MTD_PHYSMAP)	+= physmap.o
obj-$(CONFIG_MTD_PISMO)		+= pismo.o
obj-$(CONFIG_MTD_PCMCIA)	+= pcmciamtd.o
obj-$(CONFIG_MTD_SA1100)	+= sa1100-flash.o
obj-$(CONFIG_MTD_SBC_GXX)	+= sbc_gxx.o
obj-$(CONFIG_MTD_SC520CDP)	+= sc520cdp.o
obj-$(CONFIG_MTD_NETSC520)	+= netsc520.o
obj-$(CONFIG_MTD_TS5500)	+= ts5500_flash.o
obj-$(CONFIG_MTD_SUN_UFLASH)	+= sun_uflash.o
obj-$(CONFIG_MTD_SCx200_DOCFLASH)+= scx200_docflash.o
obj-$(CONFIG_MTD_SOLUTIONENGINE)+= solutionengine.o
obj-$(CONFIG_MTD_PCI)		+= pci.o
obj-$(CONFIG_MTD_IMPA7)		+= impa7.o
obj-$(CONFIG_MTD_UCLINUX)	+= uclinux.o
obj-$(CONFIG_MTD_NETtel)	+= nettel.o
obj-$(CONFIG_MTD_SCB2_FLASH)	+= scb2_flash.o
obj-$(CONFIG_MTD_PLATRAM)	+= plat-ram.o
obj-$(CONFIG_MTD_INTEL_VR_NOR)	+= intel_vr_nor.o
obj-$(CONFIG_MTD_VMU)		+= vmu-flash.o
obj-$(CONFIG_MTD_LANTIQ)	+= lantiq-flash.o
obj-$(CONFIG_MTD_AR531X)	+= ar531x_flash.o
obj-$(CONFIG_MTD_EOC5610)	+= eoc5610_flash.o
obj-$(CONFIG_MTD_BCM947XX)	+= bcm947xx-flash.o
ifneq ($(CONFIG_BCM47XX),y)
obj-y				+= nvram.o 
obj-$(CONFIG_MACH_AR7100)	+= nvram_kernel.o
ifeq ($(CONFIG_RB2011),y)
obj-$(CONFIG_MACH_AR7240)	+= nvram_kernel_x86.o
else
obj-$(CONFIG_MACH_AR7240)	+= nvram_kernel.o
endif
obj-$(CONFIG_SOC_MT7620_OPENWRT)	+= nvram_kernel.o
obj-$(CONFIG_MACH_HORNET)	+= nvram_kernel.o
obj-$(CONFIG_MACH_GW2388)	+= nvram_kernel.o
obj-$(CONFIG_ARCH_MVEBU)	+= nvram_kernel.o
obj-$(CONFIG_ARCH_QCOM)		+= nvram_kernel.o
obj-$(CONFIG_LANTIQ)		+= nvram_kernel.o
obj-$(CONFIG_ARCH_IXP4XX)	+= nvram_kernel.o
obj-$(CONFIG_X86)		+= nvram_kernel_x86.o
obj-$(CONFIG_ARCH_THUNDER)		+= nvram_kernel_x86.o
obj-$(CONFIG_MACH_KS8695_VSOPENRISC) += nvram_kernel_x86.o
obj-$(CONFIG_SOC_IMX6Q) 	+= nvram_kernel.o
ifeq ($(CONFIG_TL_WDR4900_V1),y)
obj-$(CONFIG_PPC)		+= nvram_kernel.o
else
ifeq ($(CONFIG_PPC_83xx),y)
obj-$(CONFIG_PPC)		+= nvram_kernel.o
else
obj-$(CONFIG_PPC)		+= nvram_kernel_x86.o
endif
endif
obj-$(CONFIG_RALINK)	+= nvram_kernel.o
obj-$(CONFIG_ADM5120)	+= nvram_kernel.o
obj-$(CONFIG_ATHEROS)		+= nvram_kernel.o
obj-$(CONFIG_ARCH_ALPINE)		+= nvram_kernel.o
obj-$(CONFIG_MTD_RALINK)        += ralink-flash.o
obj-$(CONFIG_CAVIUM_OCTEON_SOC) += nvram_kernel.o

endif

