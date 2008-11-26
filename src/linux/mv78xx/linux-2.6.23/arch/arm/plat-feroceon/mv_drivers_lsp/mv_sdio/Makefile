#
# Makefile for the Marvell Audio ALSA Device Driver
#
ifeq ($(CONFIG_ARCH_FEROCEON_ORION),y)
	include $(TOPDIR)/arch/arm/mach-feroceon-orion/config/mvRules.mk
endif
 
ifeq ($(CONFIG_ARCH_FEROCEON_KW),y)
	include $(TOPDIR)/arch/arm/mach-feroceon-kw/config/mvRules.mk
endif

EXTRA_CFLAGS    += -DMVSDMMC_DEBUG
obj-$(CONFIG_MMC_MVSDMMC)	+= mvsdmmc.o


