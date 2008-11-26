#
#
# Note: This Makefile is for 2.6 kernel, non-raid only, at present.
#
# V0.0.0.1 Ver.Make for kbuild
# A.C. <ake at marvell dot com>

CORE_OBJS       := core/core_exp.o core/core_xor.o\
		   core/core_api.o core/core_init.o core/scsi2sata.o

COMM_OBJS       := common/com_util.o \
		   common/com_u64.o common/com_scsi.o common/com_tag.o

# OS-Dependent Objs
OSDEP_OBJS      := linux/linux_sense.o  linux/linux_helper.o   \
		   linux/hba_mod.o      linux/hba_timer.o \
		   linux/hba_exp.o      linux/linux_iface.o \
		   linux/linux_main.o   

LIBMV_OBJS	:= $(CORE_OBJS) $(COMM_OBJS) $(raid_objs_y)

OSD_OBJS        := $(OSDEP_OBJS) $(LIBMV_OBJS)

KERNEL_SRC    :=   $(srctree)/$(src)

obj-$(CONFIG_SCSI_MV_THOR) :=   mv61xx.o

mv61xx-objs   :=   $(OSD_OBJS) 

EXTRA_CFLAGS  :=   -I$(KERNEL_SRC)/common -I$(KERNEL_SRC)/core \
		   -I$(KERNEL_SRC)/linux  -I$(KERNEL_SRC)/. 

EXTRA_CFLAGS  +=   -D__MV_LINUX__ $(INCLUDE_DIR)

ifneq ($(CONFIG_64BIT), y)
EXTRA_CFLAGS  +=   -D_32_LEGACY_
else
EXTRA_CFLAGS  +=   -D_64_SYS_
endif

EXTRA_CFLAGS  +=   -include $(KERNEL_SRC)/mv_config.h

EXTRA_CFLAGS  +=   -D__MV_DEBUG__

EXTRA_CFLAGS  += -D__AC_DBG__

