# Makefile for pptp client
ccflags-y += -I$(obj)/../exports -I$(obj)/..  -I$(obj)/nss_hal/include
ccflags-y += -DNSS_PPTP_DEBUG_LEVEL=0
ccflags-y += -Wall -Werror
obj-m += qca-nss-pptp-$(SoC).o
qca-nss-pptp-$(SoC)-objs := nss_connmgr_pptp.o
