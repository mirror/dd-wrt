smiapp-objs			+= smiapp-core.o smiapp-regs.o \
				   smiapp-quirk.o smiapp-limits.o
obj-$(CPTCFG_VIDEO_SMIAPP)	+= smiapp.o

ccflags-y += -I$(backport_srctree)/drivers/media/i2c
