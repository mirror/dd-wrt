ifeq ($(CONFIG_BCM47XX),y)
EXTRA_CFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
endif
bcm47xxnflash-y				+= main.o
bcm47xxnflash-y				+= ops_bcm4706.o

obj-$(CONFIG_MTD_NAND_BCM47XXNFLASH)	+= bcm47xxnflash.o
