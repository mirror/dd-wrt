BRCM_CHIP=4908
EXTRA_CFLAGS += -DWL4908
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx -I$(srctree)/arch/arm64/brcm/include/pmc  -I$(srctree)/arch/arm64/brcm/include/rdpa -I$(srctree)/arch/arm64/brcm/include/shared

# Linux
obj-y		+= shared_utils.o


EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
