BRCM_CHIP=4908

EXTRA_CFLAGS	+= -DWL4908 -DCONFIG_BCM4908 
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx  -I$(srctree)/arch/arm64/brcm/include/shared 
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/include/rdp
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/drv 
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/include/rdpa  -I$(srctree)/arch/arm64/brcm/rdd
EXTRA_CFLAGS	+= -DBDMF_SYSTEM_LINUX -DBDMF_SHELL -DBDMF_HISTORY -DWL4908 -DCONFIG_BCM4908  -DUSE_BDMF_SHELL
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx  -I$(srctree)/arch/arm64/brcm/include/shared -I$(srctree)/arch/arm64/brcm/bdmf -I$(srctree)/arch/arm64/brcm/bdmf/system/linux/ce
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/bdmf/system/linux
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/bdmf/system
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/bdmf/shell


obj-y +=  rdd.o
rdd-objs += rdd_bridge.o  rdd_cpu.o         rdd_dhd_helper_shell.o  rdd_interworking.o   rdd_router.o           rdd_runner_reg_dump_addrs.o  rdd_simulator.o  rdd_utils.o
rdd-objs += rdd_common.o  rdd_dhd_helper.o  rdd_init.o              rdd_lookup_engine.o  rdd_runner_reg_dump.o  rdd_shell.o                  rdd_tm.o         rdd_wlan_mcast.o
