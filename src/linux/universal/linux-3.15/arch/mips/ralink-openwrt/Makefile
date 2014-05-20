# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.#
# Makefile for the Ralink common stuff
#
# Copyright (C) 2009-2011 Gabor Juhos <juhosg@openwrt.org>
# Copyright (C) 2013 John Crispin <blogic@openwrt.org>

obj-y := prom.o of.o reset.o clk.o irq.o timer.o

obj-$(CONFIG_CLKEVT_RT3352) += cevt-rt3352.o

obj-$(CONFIG_RALINK_ILL_ACC) += ill_acc.o

obj-$(CONFIG_SOC_RT288X_OPENWRT) += rt288x.o
obj-$(CONFIG_SOC_RT305X_OPENWRT) += rt305x.o
obj-$(CONFIG_SOC_RT3883_OPENWRT) += rt3883.o
obj-$(CONFIG_SOC_MT7620_OPENWRT) += mt7620.o

obj-$(CONFIG_EARLY_PRINTK) += early_printk.o

obj-$(CONFIG_DEBUG_FS) += bootrom.o

#obj-y += dts/
