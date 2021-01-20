# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the orinoco wireless device drivers.
#
orinoco-objs := main.o fw.o hw.o mic.o scan.o wext.o hermes_dld.o hermes.o cfg.o

obj-$(CPTCFG_HERMES)		+= orinoco.o
obj-$(CPTCFG_PCMCIA_HERMES)	+= orinoco_cs.o
obj-$(CPTCFG_APPLE_AIRPORT)	+= airport.o
obj-$(CPTCFG_PLX_HERMES)	+= orinoco_plx.o
obj-$(CPTCFG_PCI_HERMES)	+= orinoco_pci.o
obj-$(CPTCFG_TMD_HERMES)	+= orinoco_tmd.o
obj-$(CPTCFG_NORTEL_HERMES)	+= orinoco_nortel.o
obj-$(CPTCFG_PCMCIA_SPECTRUM)	+= spectrum_cs.o
obj-$(CPTCFG_ORINOCO_USB)	+= orinoco_usb.o
