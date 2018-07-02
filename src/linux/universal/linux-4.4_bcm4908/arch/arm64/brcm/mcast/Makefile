#
# Makefile for the BCM Multicast module
#
BRCM_CHIP=4908
# File: bcmdrivers/opensource/char/plat-bcm
#
# Makefile for the BCM63xx/68xx  platform drivers
EXTRA_CFLAGS	+= -DWL4908 -DCONFIG_BCM4908 
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx  -I$(srctree)/arch/arm64/brcm/include/shared -I$(srctree)/arch/arm64/brcm/include/pmc

bcmmcast-objs  = bcm_mcast.o bcm_mcast_netlink.o bcm_mcast_if.o 
ifneq ($(strip $(CONFIG_BR_IGMP_SNOOP)),)
bcmmcast-objs += bcm_mcast_igmp.o
endif
ifneq ($(strip $(CONFIG_BR_MLD_SNOOP)),)
bcmmcast-objs += bcm_mcast_mld.o
endif

ifneq ($(and $(strip $(CONFIG_BR_IGMP_SNOOP))$(strip $(CONFIG_BR_MLD_SNOOP)), $(strip $(CONFIG_BLOG))),)
bcmmcast-objs += bcm_mcast_blog.o
endif

obj-$(CONFIG_BCM_MCAST) += bcmmcast.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -Werror -Wfatal-errors

clean:
	rm -f core *.o *.a .*.cmd *.ko

