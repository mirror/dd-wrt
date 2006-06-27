USE_STANDARD_AS_RULE := true

L_TARGET = sbswarm.a

obj-y			+= setup.o cmdline.o rtc_xicor1241.o rtc_m41t81.o

obj-$(CONFIG_KGDB)	+= dbg_io.o

include $(TOPDIR)/Rules.make
