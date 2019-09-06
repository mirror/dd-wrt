libertas-y += cfg.o
libertas-y += cmd.o
libertas-y += cmdresp.o
libertas-y += debugfs.o
libertas-y += ethtool.o
libertas-y += main.o
libertas-y += rx.o
libertas-y += tx.o
libertas-y += firmware.o
libertas-$(CPTCFG_LIBERTAS_MESH) += mesh.o

usb8xxx-objs += if_usb.o
libertas_cs-objs += if_cs.o
libertas_sdio-objs += if_sdio.o
libertas_spi-objs += if_spi.o

obj-$(CPTCFG_LIBERTAS)     += libertas.o
obj-$(CPTCFG_LIBERTAS_USB) += usb8xxx.o
obj-$(CPTCFG_LIBERTAS_CS)  += libertas_cs.o
obj-$(CPTCFG_LIBERTAS_SDIO) += libertas_sdio.o
obj-$(CPTCFG_LIBERTAS_SPI) += libertas_spi.o
