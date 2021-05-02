ifeq ($(CONFIG_MIPS),y)
ifeq ($(CONFIG_CPU_MIPS32_R1),y)
EXTRA_CFLAGS += -I$(srctree)/brcm/mipsel_r1/include
EXTRA_CFLAGS += -I$(srctree)/brcm/mipsel_r1/common/include -I$(srctree)/net/8021q
else
EXTRA_CFLAGS += -I$(srctree)/brcm/mipsel/include
EXTRA_CFLAGS += -I$(srctree)/brcm/mipsel/common/include -I$(srctree)/net/8021q
endif
else
EXTRA_CFLAGS += -I$(srctree)/brcm/arm/include
EXTRA_CFLAGS += -I$(srctree)/brcm/arm/common/include -I$(srctree)/net/8021q
endif
#
# Makefile for the Linux PPP network device drivers.
#

obj-$(CONFIG_PPP) += ppp_generic.o
obj-$(CONFIG_PPP_ASYNC) += ppp_async.o
obj-$(CONFIG_PPP_BSDCOMP) += bsd_comp.o
obj-$(CONFIG_PPP_DEFLATE) += ppp_deflate.o
obj-$(CONFIG_PPP_MPPE) += ppp_mppe.o
obj-$(CONFIG_PPP_SYNC_TTY) += ppp_synctty.o
obj-$(CONFIG_PPPOE) += pppox.o pppoe.o
obj-$(CONFIG_PPPOL2TP) += pppox.o
obj-$(CONFIG_PPTP) += pppox.o pptp.o
