MAKEFILE_DIR:=$(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))

KBUILD_CPPFLAGS += -Iarch/arm/mach-alpine/include/al_hal/
KBUILD_CPPFLAGS += -Iarch/arm/mach-alpine/include/mach/
KBUILD_CPPFLAGS += -I$(MAKEFILE_DIR)/al_hal_raid/

obj-$(CONFIG_AL_DMA) += al_dma.o

al_dma-objs := al_dma_main.o al_dma_core.o al_dma_cleanup.o al_dma_sysfs.o \
		al_dma_prep_interrupt.o al_dma_prep_memcpy.o al_dma_prep_sg.o \
		al_dma_prep_memset.o al_dma_prep_xor.o al_dma_prep_xor_val.o \
		al_dma_prep_pq.o al_dma_prep_pq_val.o al_hal_ssm_raid.o \
		al_dma_module_params.o