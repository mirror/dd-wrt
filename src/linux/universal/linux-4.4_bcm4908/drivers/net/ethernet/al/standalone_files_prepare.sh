#! /bin/bash

# this script links all files needed for standalone driver
# parameters: path to the alpine linux kernel source tree

KERNEL_PATH=$1
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/Makefile.standalone Makefile

ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_eth.c .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_eth.h .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_eth_sysfs.c .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_eth_sysfs.h .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_hal_eth_kr.c .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_hal_eth_kr.h .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_init_eth_lm.h .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_init_eth_lm.c .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_init_eth_kr.c .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_common.h .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_hal_eth_ec_regs.h .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_hal_eth.h .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_hal_eth_mac_regs.h .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_hal_eth_macsec.h .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_hal_eth_main.c .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/al_hal/al_hal_iofic.c .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_iofic.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_iofic_regs.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_plat_services.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_plat_types.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_reg_utils.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/al_hal/al_hal_serdes.c .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_serdes.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_serdes_internal_regs.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_serdes_regs.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_types.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/al_hal/al_hal_udma_config.c .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_udma_config.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/al_hal/al_hal_udma_debug.c .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_udma_debug.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_udma.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/al_hal/al_hal_udma_iofic.c .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_udma_iofic.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_udma_iofic_regs.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/al_hal/al_hal_udma_main.c .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_udma_regs_gen.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_udma_regs.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_udma_regs_m2s.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_udma_regs_s2m.h .
ln -s ${KERNEL_PATH}/arch/arm/mach-alpine/include/al_hal/al_hal_unit_adapter_regs.h .
ln -s ${KERNEL_PATH}/drivers/net/ethernet/al/al_init_eth_kr.h .
