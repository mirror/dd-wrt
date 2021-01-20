# SPDX-License-Identifier: GPL-2.0
# core
ssb-y					+= main.o scan.o
ssb-$(CPTCFG_SSB_EMBEDDED)		+= embedded.o
ssb-$(CPTCFG_SSB_SPROM)			+= sprom.o

# host support
ssb-$(CPTCFG_SSB_PCIHOST)		+= pci.o pcihost_wrapper.o
ssb-$(CPTCFG_SSB_PCMCIAHOST)		+= pcmcia.o bridge_pcmcia_80211.o
ssb-$(CPTCFG_SSB_SDIOHOST)		+= sdio.o
ssb-$(CPTCFG_SSB_HOST_SOC)		+= host_soc.o

# built-in drivers
ssb-y					+= driver_chipcommon.o
ssb-y					+= driver_chipcommon_pmu.o
ssb-$(CPTCFG_SSB_SFLASH)		+= driver_chipcommon_sflash.o
ssb-$(CPTCFG_SSB_DRIVER_MIPS)		+= driver_mipscore.o
ssb-$(CPTCFG_SSB_DRIVER_EXTIF)		+= driver_extif.o
ssb-$(CPTCFG_SSB_DRIVER_PCICORE)	+= driver_pcicore.o
ssb-$(CPTCFG_SSB_DRIVER_GIGE)		+= driver_gige.o
ssb-$(CPTCFG_SSB_DRIVER_GPIO)		+= driver_gpio.o

# b43 pci-ssb-bridge driver
# Not strictly a part of SSB, but kept here for convenience
ssb-$(CPTCFG_SSB_B43_PCI_BRIDGE)	+= b43_pci_bridge.o

obj-$(CPTCFG_SSB)			+= ssb.o
