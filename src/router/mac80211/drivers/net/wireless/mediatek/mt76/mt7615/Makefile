#SPDX-License-Identifier: ISC


obj-$(CPTCFG_MT7615E) += mt7615e.o

CFLAGS_trace.o := -I$(src)

mt7615e-y := pci.o init.o dma.o eeprom.o main.o mcu.o mac.o debugfs.o trace.o mmio.o
mt7615e-$(CONFIG_MT7622_WMAC) += soc.o

