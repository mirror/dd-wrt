ifeq ($(CONFIG_PLAT_BCM5301X),y)
EXTRA_CFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
endif
#
# Makefile for the Broadcom NAND MTD
#

#obj-$(CONFIG_MTD_BRCMNAND) += brcmnand_47xx.o brcmnand_bbt.o
#obj-$(CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING) += brcmnand_cet.o
obj-$(CONFIG_MTD_BRCMNAND) += brcmnand.o
obj-$(CONFIG_MTD_NFLASH) += bcm_nflash.o
