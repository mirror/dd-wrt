BRCM_CHIP=4908

EXTRA_CFLAGS	+= -DWL4908 -DCONFIG_BCM4908 
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx  -I$(srctree)/arch/arm64/brcm/include/shared 
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/include/rdp
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/drv 
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/include/rdpa  -I$(srctree)/arch/arm64/brcm/rdd
EXTRA_CFLAGS	+= -DBDMF_SYSTEM_LINUX -DBDMF_SHELL -DBDMF_HISTORY -DWL4908 -DCONFIG_BCM4908 
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx  -I$(srctree)/arch/arm64/brcm/include/shared -I$(srctree)/arch/arm64/brcm/bdmf -I$(srctree)/arch/arm64/brcm/bdmf/system/linux/ce
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/bdmf/system/linux
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/bdmf/system
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/bdmf/shell


obj-y +=  rdp.o

rdp-objs += data_path_init.o rdp_cpu_ring.o    rdp_drv_bbh.o  rdp_drv_ih.o    rdp_drv_unimac.o
rdp-objs += rdp_bbh_arrays.o       rdp_dma_arrays.o  rdp_drv_bpm.o  rdp_drv_sbpm.o  rdp_runner_arrays.o
rdp-objs += runner_fw_a.o
rdp-objs += runner_fw_b.o
rdp-objs += runner_fw_c.o
rdp-objs += runner_fw_d.o

rdp-objs += predict_runner_fw_a.o
rdp-objs += predict_runner_fw_b.o
rdp-objs += predict_runner_fw_c.o
rdp-objs += predict_runner_fw_d.o


