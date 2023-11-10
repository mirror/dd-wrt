# SPDX-License-Identifier: ISC
EXTRA_CFLAGS += -DCONFIG_MT76_LEDS

obj-$(CPTCFG_MT7996E) += mt7996e.o

mt7996e-y := pci.o init.o dma.o eeprom.o main.o mcu.o mac.o \
	     debugfs.o mmio.o
