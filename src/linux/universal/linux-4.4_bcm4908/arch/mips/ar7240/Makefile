#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
#

# Makefile for Atheros ar531x boards
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

#.S.s:
#	$(CPP) $(CFLAGS) $< -o $*.s
#.S.o:
#	$(CC) $(CFLAGS) -D__ASSEMBLY__ -c $< -o $*.o

obj-y	+=	prom.o	\
			proc_gpio.o \
			proc_wlgpio.o \
			irq.o	\
			setup.o	\
			gpio_driver.o	\
			dev-leds-gpio.o \
			dev-ar9xxx-wmac.o \
			ar7240.o \
			ar71xx.o \
			pci.o \
			dev-ap91-pci.o \
			pci-ath9k-fixup.o \
			nvram.o \
			platform.o \
			devices.o 

obj-$(CONFIG_EARLY_PRINTK)		+= early_printk.o
#obj-$(CONFIG_AR9100)	+= wdt.o


#EXTRA_CFLAGS				+=	-fno-schedule-insns -fno-schedule-insns2
ifdef CONFIG_AP_USB_LED_GPIO
CFLAGS_proc_gpio.o += -DAP_USB_LED_GPIO=$(CONFIG_AP_USB_LED_GPIO)
endif
