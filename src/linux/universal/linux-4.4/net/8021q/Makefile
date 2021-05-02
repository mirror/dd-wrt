ifeq ($(CONFIG_MIPS),y)
ifeq ($(CONFIG_CPU_MIPS32_R1),y)
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel_r1/include -I$(srctree)/brcm/mipsel_r1/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel_r1/include -I$(srctree)/brcm/mipsel_r1/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel_r1/common/include
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel_r1/common/include
else
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel/include -I$(srctree)/brcm/mipsel/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel/include -I$(srctree)/brcm/mipsel/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_CFLAGS+=-I$(srctree)/brcm/mipsel/common/include
EXTRA_AFLAGS+=-I$(srctree)/brcm/mipsel/common/include
endif
else
EXTRA_CFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_AFLAGS+=-I$(srctree)/brcm/arm/include -I$(srctree)/brcm/arm/shared/bcmwifi/include -DBCMDRIVER -Dlinux
EXTRA_CFLAGS+=-I$(srctree)/brcm/arm/common/include
EXTRA_AFLAGS+=-I$(srctree)/brcm/arm/common/include
endif
#
# Makefile for the Linux VLAN layer.
#
obj-$(subst m,y,$(CONFIG_VLAN_8021Q))	+= vlan_core.o
obj-$(CONFIG_VLAN_8021Q)		+= 8021q.o

8021q-y					:= vlan.o vlan_dev.o vlan_netlink.o
8021q-$(CONFIG_VLAN_8021Q_GVRP)		+= vlan_gvrp.o
8021q-$(CONFIG_VLAN_8021Q_MVRP)		+= vlan_mvrp.o
8021q-$(CONFIG_PROC_FS)			+= vlanproc.o

