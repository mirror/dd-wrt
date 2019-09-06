
smipcie-objs	:= smipcie-main.o smipcie-ir.o

obj-$(CPTCFG_DVB_SMIPCIE) += smipcie.o

ccflags-y += -I$(backport_srctree)/drivers/media/tuners
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-frontends

