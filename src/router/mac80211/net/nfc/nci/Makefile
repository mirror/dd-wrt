#
# Makefile for the Linux NFC NCI layer.
#

obj-$(CPTCFG_NFC_NCI) += nci.o

nci-objs := core.o data.o lib.o ntf.o rsp.o hci.o

nci_spi-y += spi.o
obj-$(CPTCFG_NFC_NCI_SPI) += nci_spi.o

nci_uart-y += uart.o
obj-$(CPTCFG_NFC_NCI_UART) += nci_uart.o
