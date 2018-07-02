EXTRA_CFLAGS	+= -DBDMF_SYSTEM_LINUX -DBDMF_SHELL -DBDMF_HISTORY -DWL4908 -DCONFIG_BCM4908 
EXTRA_CFLAGS	+= -I$(srctree)/include/asm-generic -I$(srctree)/arch/arm64/brcm/include/bcm963xx  -I$(srctree)/arch/arm64/brcm/include/shared -I$(srctree)/arch/arm64/brcm/bdmf -I$(srctree)/arch/arm64/brcm/bdmf/system/linux/ce
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/bdmf/system/linux
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/bdmf/system
EXTRA_CFLAGS	+= -I$(srctree)/arch/arm64/brcm/bdmf/shell

obj-y = bdmf.o

bdmf-objs += bdmf_attr.o bdmf_fini.o  bdmf_history.o  bdmf_init.o  bdmf_memarea.o  bdmf_mon.o  bdmf_object.o  bdmf_trace.o  bdmf_type.o  bdmf_utils.o
bdmf-objs += system/bdmf_errno.o system/bdmf_session.o
bdmf-objs += system/linux/bdmf_chrdev.o system/linux/bdmf_system_common.o
bdmf-objs += system/linux/ce/bdmf_system.o
bdmf-objs += shell/bdmf_shell.o
