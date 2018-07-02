ifeq ($(CONFIG_PLAT_BCM5301X),y)
EXTRA_CFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
endif

#
# linux/drivers/mtd/devices/Makefile
#

obj-$(CONFIG_MTD_DOCG3)		+= docg3.o
obj-$(CONFIG_MTD_SLRAM)		+= slram.o
obj-$(CONFIG_MTD_PHRAM)		+= phram.o
obj-$(CONFIG_MTD_PMC551)	+= pmc551.o
obj-$(CONFIG_MTD_MS02NV)	+= ms02-nv.o
obj-$(CONFIG_MTD_MTDRAM)	+= mtdram.o
obj-$(CONFIG_MTD_LART)		+= lart.o
obj-$(CONFIG_MTD_BLOCK2MTD)	+= block2mtd.o
obj-$(CONFIG_MTD_DATAFLASH)	+= mtd_dataflash.o
obj-$(CONFIG_MTD_M25P80)	+= m25p80.o
obj-$(CONFIG_MTD_SPEAR_SMI)	+= spear_smi.o
obj-$(CONFIG_MTD_SST25L)	+= sst25l.o
obj-$(CONFIG_MTD_BCM47XXSFLASH)	+= bcm47xxsflash.o
obj-$(CONFIG_MTD_ST_SPI_FSM)    += st_spi_fsm.o
obj-$(CONFIG_MTD_POWERNV_FLASH)	+= powernv_flash.o
obj-$(CONFIG_MTD_AR7100_SPI_FLASH)	+= ar7100_flash.o
obj-$(CONFIG_MTD_AR9100_PARALLEL_FLASH)	+= ar9100_pflash.o
obj-$(CONFIG_MTD_AR7240_SPI_FLASH)	+= ar7240_flash.o qca-sf.o
obj-$(CONFIG_WZRAG300NH)	+= wzrag300nh_flash.o
obj-$(CONFIG_WZRG450NH)		+= wzrg450nh_flash.o
obj-$(CONFIG_MTD_SPIFLASH)	+= spiflash.o
obj-$(CONFIG_MTD_SPIFLASH_SELFMAP)	+= spiflash_selfmap.o


CFLAGS_docg3.o			+= -I$(src)
