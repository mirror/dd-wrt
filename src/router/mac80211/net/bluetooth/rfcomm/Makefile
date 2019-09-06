#
# Makefile for the Linux Bluetooth RFCOMM layer.
#

obj-$(CPTCFG_BT_RFCOMM) += rfcomm.o

rfcomm-y			:= core.o sock.o
rfcomm-$(CPTCFG_BT_RFCOMM_TTY)	+= tty.o
