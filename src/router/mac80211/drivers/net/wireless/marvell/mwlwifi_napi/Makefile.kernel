obj-$(CONFIG_MWLWIFI)	+= mwlwifi.o

mwlwifi-objs			+= core.o
mwlwifi-objs			+= mac80211.o
mwlwifi-objs			+= utils.o
mwlwifi-$(CONFIG_THERMAL)	+= thermal.o
mwlwifi-$(CONFIG_DEBUG_FS)	+= debugfs.o
mwlwifi-objs			+= hif/fwcmd.o
mwlwifi-objs			+= hif/pcie/pcie.o
mwlwifi-objs			+= hif/pcie/fwdl.o
mwlwifi-objs			+= hif/pcie/tx.o
mwlwifi-objs			+= hif/pcie/rx.o
mwlwifi-objs			+= hif/pcie/tx_ndp.o
mwlwifi-objs			+= hif/pcie/rx_ndp.o

ccflags-y += -I$(src)
ccflags-y += -D__CHECK_ENDIAN__
