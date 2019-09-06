#
# Makefile for the Linux NFC subsystem.
#

obj-$(CPTCFG_NFC) += nfc.o
obj-$(CPTCFG_NFC_NCI) += nci/
obj-$(CPTCFG_NFC_HCI) += hci/
obj-$(CPTCFG_NFC_DIGITAL) += nfc_digital.o

nfc-objs := core.o netlink.o af_nfc.o rawsock.o llcp_core.o llcp_commands.o \
		llcp_sock.o

nfc_digital-objs := digital_core.o digital_technology.o digital_dep.o
