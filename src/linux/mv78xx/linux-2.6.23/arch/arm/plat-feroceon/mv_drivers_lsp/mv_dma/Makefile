#
# Makefile for the Marvell DMA Driver
#

ifeq ($(CONFIG_ARCH_FEROCEON_ORION),y)
	include $(TOPDIR)/arch/arm/mach-feroceon-orion/config/mvRules.mk
endif

ifeq ($(CONFIG_ARCH_FEROCEON_MV78XX0),y)
	include $(TOPDIR)/arch/arm/mach-feroceon-mv78xx0/config/mvRules.mk
endif

obj-$(CONFIG_MV_USE_IDMA_ENGINE) += mv_dma.o 


