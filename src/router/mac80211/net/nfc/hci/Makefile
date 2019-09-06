#
# Makefile for the Linux NFC HCI layer.
#

obj-$(CPTCFG_NFC_HCI) += hci.o

hci-y			:= core.o hcp.o command.o llc.o llc_nop.o
hci-$(CPTCFG_NFC_SHDLC) += llc_shdlc.o
