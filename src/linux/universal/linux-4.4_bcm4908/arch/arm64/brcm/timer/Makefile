# File: bcmdrivers/opensource/char/timer
#
# Makefile for the BCM963xx external timer driver
BRCM_CHIP=4908
EXTRA_CFLAGS += -DWL4908
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx -I$(srctree)/arch/arm64/brcm/include/pmc  -I$(srctree)/arch/arm64/brcm/include/rdpa -I$(srctree)/arch/arm64/brcm/include/shared


obj-$(CONFIG_BCM_EXT_TIMER) += bcm963xx_timer.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -Werror -Wfatal-errors

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

