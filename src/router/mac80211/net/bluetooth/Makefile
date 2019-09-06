#
# Makefile for the Linux Bluetooth subsystem.
#

obj-$(CPTCFG_BT)	+= bluetooth.o
obj-$(CPTCFG_BT_RFCOMM)	+= rfcomm/
obj-$(CPTCFG_BT_BNEP)	+= bnep/
obj-$(CPTCFG_BT_CMTP)	+= cmtp/
obj-$(CPTCFG_BT_HIDP)	+= hidp/
obj-$(CPTCFG_BT_6LOWPAN) += bluetooth_6lowpan.o

bluetooth_6lowpan-y := 6lowpan.o

bluetooth-y := af_bluetooth.o hci_core.o hci_conn.o hci_event.o mgmt.o \
	hci_sock.o hci_sysfs.o l2cap_core.o l2cap_sock.o smp.o lib.o \
	ecc.o hci_request.o mgmt_util.o

bluetooth-$(CPTCFG_BT_BREDR) += sco.o
bluetooth-$(CPTCFG_BT_HS) += a2mp.o amp.o
bluetooth-$(CPTCFG_BT_LEDS) += leds.o
bluetooth-$(CPTCFG_BT_DEBUGFS) += hci_debugfs.o
bluetooth-$(CPTCFG_BT_SELFTEST) += selftest.o
