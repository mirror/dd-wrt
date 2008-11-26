
ifeq ($(CONFIG_ARCH_FEROCEON_ORION),y)
	include $(TOPDIR)/arch/arm/mach-feroceon-orion/config/mvRules.mk
endif
 
ifeq ($(CONFIG_ARCH_FEROCEON_KW),y)
	include $(TOPDIR)/arch/arm/mach-feroceon-kw/config/mvRules.mk
endif

ifeq ($(CONFIG_ARCH_FEROCEON_MV78XX0),y)
	include $(TOPDIR)/arch/arm/mach-feroceon-mv78xx0/config/mvRules.mk
endif


obj-$(CONFIG_MV_INCLUDE_SFLASH_MTD) += sflash.o
obj-$(CONFIG_MV_INCLUDE_MFLASH_MTD) += mflash.o

