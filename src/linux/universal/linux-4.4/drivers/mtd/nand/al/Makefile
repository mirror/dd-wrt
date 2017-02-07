KBUILD_CPPFLAGS += -Iarch/arm/mach-alpine/include/al_hal/

# TODO: temp solution to allow include raid.h - will be removed after
# implementing read/write with dma support
KBUILD_CPPFLAGS += -Idrivers/dma/al

obj-$(CONFIG_MTD_NAND_AL) += al-nand.o
al-nand-objs += al_nand.o
al-nand-objs += al_hal_nand.o
