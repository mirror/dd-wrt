#
# Makefile for the kernel SAA7146 FULL TS DVB device driver
# and the AV7110 DVB device driver
#

dvb-ttpci-objs := av7110_hw.o av7110_v4l.o av7110_av.o av7110_ca.o av7110.o av7110_ipack.o dvb_filter.o

ifdef CPTCFG_DVB_AV7110_IR
dvb-ttpci-objs += av7110_ir.o
endif

obj-$(CPTCFG_TTPCI_EEPROM) += ttpci-eeprom.o
obj-$(CPTCFG_DVB_BUDGET_CORE) += budget-core.o
obj-$(CPTCFG_DVB_BUDGET) += budget.o
obj-$(CPTCFG_DVB_BUDGET_AV) += budget-av.o
obj-$(CPTCFG_DVB_BUDGET_CI) += budget-ci.o
obj-$(CPTCFG_DVB_BUDGET_PATCH) += budget-patch.o
obj-$(CPTCFG_DVB_AV7110) += dvb-ttpci.o

ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core/ -I$(backport_srctree)/drivers/media/dvb-frontends/
ccflags-y += -I$(backport_srctree)/drivers/media/tuners
