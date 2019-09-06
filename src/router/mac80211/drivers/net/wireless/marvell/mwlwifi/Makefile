obj-$(CPTCFG_MWLWIFI)	+= mwlwifi.o

mwlwifi-objs			+= core.o
mwlwifi-objs			+= mac80211.o
mwlwifi-objs			+= mu_mimo.o
mwlwifi-objs			+= vendor_cmd.o
mwlwifi-objs			+= utils.o
mwlwifi-objs			+= thermal.o
mwlwifi-$(CONFIG_DEBUG_FS)	+= debugfs.o
mwlwifi-objs			+= hif/fwcmd.o
mwlwifi-objs			+= hif/pcie/pcie.o
mwlwifi-objs			+= hif/pcie/fwdl.o
mwlwifi-objs			+= hif/pcie/tx.o
mwlwifi-objs			+= hif/pcie/rx.o
mwlwifi-objs			+= hif/pcie/tx_ndp.o
mwlwifi-objs			+= hif/pcie/rx_ndp.o

ccflags-y += -D__CHECK_ENDIAN__ -O2 -funroll-loops -D__CHECK_ENDIAN__ -I${src}
