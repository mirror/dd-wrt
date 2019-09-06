c8sectpfe-y += c8sectpfe-core.o c8sectpfe-common.o c8sectpfe-dvb.o \
		c8sectpfe-debugfs.o

obj-$(CPTCFG_DVB_C8SECTPFE) += c8sectpfe.o

ccflags-y += -I$(backport_srctree)/drivers/media/i2c
ccflags-y += -I$(backport_srctree)/drivers/media/common
ccflags-y += -I$(backport_srctree)/drivers/media/dvb-core/ -I$(backport_srctree)/drivers/media/dvb-frontends/ \
		-I$(backport_srctree)/drivers/media/tuners/
