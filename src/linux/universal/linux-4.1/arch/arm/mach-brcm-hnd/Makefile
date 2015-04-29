EXTRA_CFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/include -I$(srctree)/brcm/shared/bcmwifi/include -DBCMDRIVER -Dlinux

#
# Board-level BSP
# Preliminary
#

obj-$(CONFIG_MACH_BRCM_NS) += board_ns.o gpio.o
obj-$(CONFIG_MACH_BRCM_NS_QT) += board_ns.o