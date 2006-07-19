
ifeq ($(CONFIG_MIPS_BRCM),y)

# Linux
obj-y		+= boardparms.o
EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP)
-include $(TOPDIR)/Rules.make

else

# CFE
BSPOBJS += boardparms.o

endif


