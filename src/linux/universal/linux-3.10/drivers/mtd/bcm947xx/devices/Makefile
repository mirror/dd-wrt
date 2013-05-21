ifeq ($(CONFIG_BCM47XX),y)
EXTRA_CFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
endif
#
# linux/drivers/mtd/bcm947xx/devices/Makefile
#

obj-$(CONFIG_MTD_BCMSFLASH)	+= bcmsflash.o
