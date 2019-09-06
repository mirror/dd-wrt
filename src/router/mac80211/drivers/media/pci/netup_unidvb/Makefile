netup-unidvb-objs += netup_unidvb_core.o
netup-unidvb-objs += netup_unidvb_i2c.o
netup-unidvb-objs += netup_unidvb_ci.o
netup-unidvb-objs += netup_unidvb_spi.o

obj-$(CPTCFG_DVB_NETUP_UNIDVB) += netup-unidvb.o

ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-frontends
