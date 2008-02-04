#
# Makefile for the Marvell egiga ethernet driver
#

include $(TOPDIR)/arch/arm/mach-mv88fxx81/mv88f5181/mvRules.mk

IAL_OBJS        := mvLinuxIalLib.o mvLinuxIalHt.o mvLinuxIalOs.o mvIALCommon.o mvIALCommonUtils.o mvLinuxIalSmart.o
SAL_OBJS        := mvScsiAtaLayer.o

obj-$(CONFIG_SCSI_MVSATA) := mvsata.o
mvsata-y        := $(IAL_OBJS) $(SAL_OBJS) ../$(SATA_CORE_DIR)/mvSata.o ../$(SATA_CORE_DIR)/mvStorageDev.o             \
                  ../$(SATA_CORE_DIR)/mvLog.o


INCLUDE_DIRS    := -Idrivers/scsi
EXTRA_CFLAGS += -DLINUX -D__LINUX__ $(INCLUDE_DIRS)
