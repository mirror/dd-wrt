#SPDX-License-Identifier: ISC

EXTRA_CFLAGS += -DCONFIG_MT76_LEDS
obj-$(CPTCFG_MT7615_COMMON) += mt7615-common.o
obj-$(CPTCFG_MT7615E) += mt7615e.o
obj-$(CPTCFG_MT7663_USB_SDIO_COMMON) += mt7663-usb-sdio-common.o
obj-$(CPTCFG_MT7663U) += mt7663u.o
obj-$(CPTCFG_MT7663S) += mt7663s.o

CFLAGS_trace.o := -I$(src)

mt7615-common-y := main.o init.o mcu.o eeprom.o mac.o \
		   debugfs.o trace.o
mt7615-common-$(CPTCFG_NL80211_TESTMODE) += testmode.o

mt7615e-y := pci.o pci_init.o dma.o pci_mac.o mmio.o
mt7615e-$(CPTCFG_MT7622_WMAC) += soc.o

mt7663-usb-sdio-common-y := usb_sdio.o
mt7663u-y := usb.o usb_mcu.o
mt7663s-y := sdio.o sdio_mcu.o
