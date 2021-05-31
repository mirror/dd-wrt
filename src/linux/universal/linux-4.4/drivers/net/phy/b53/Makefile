ifeq ($(CONFIG_MIPS),y)
ifeq ($(CONFIG_CPU_MIPS32_R1),y)
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel_r1/include -I$(srctree)/brcm/mipsel_r1/common/include -I$(srctree)/brcm/mipsel_r1/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel_r1/include -I$(srctree)/brcm/mipsel_r1/common/include -I$(srctree)/brcm/mipsel_r1/shared/bcmwifi/include -DBCMDRIVER -Dlinux
else
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel/include -I$(srctree)/brcm/mipsel/common/include -I$(srctree)/brcm/mipsel/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel/include -I$(srctree)/brcm/mipsel/common/include -I$(srctree)/brcm/mipsel/shared/bcmwifi/include -DBCMDRIVER -Dlinux
endif
else
EXTRA_CFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/common/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/common/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
endif



#obj-$(CONFIG_B53)		+= b53_common.o
obj-$(CONFIG_B53)		+= b5301x_common.o

obj-$(CONFIG_B53_PHY_FIXUP)	+= b53_phy_fixup.o

obj-$(CONFIG_B53_MMAP_DRIVER)	+= b53_mmap.o
#obj-$(CONFIG_B53_SRAB_DRIVER)	+= b53_srab.o
obj-$(CONFIG_B53_SRAB_DRIVER)	+= b5301x_srab.o
obj-$(CONFIG_B53_PHY_DRIVER)	+= b53_mdio.o
obj-$(CONFIG_B53_SPI_DRIVER)	+= b53_spi.o

ccflags-y			+= -Werror
