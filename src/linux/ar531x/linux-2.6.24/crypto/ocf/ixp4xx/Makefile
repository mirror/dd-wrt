# for SGlinux builds
-include $(ROOTDIR)/modules/.config

#
# You will need to point this at your Intel ixp425 includes,  this portion
# of the Makefile only really works under SGLinux with the appropriate libs
# installed.  They can be downloaded from http://www.snapgear.org/
#

IX_XSCALE_SW := drivers/ixp400/ixp400_xscale_sw
OSAL_DIR := $(IX_XSCALE_SW)/../ixp_osal

OSAL_INCLUDES += \
  -I$(IX_XSCALE_SW)/src/include \
  -I$(OSAL_DIR)/ \
  -I$(OSAL_DIR)/common/include/ \
  -I$(OSAL_DIR)/common/include/modules/ioMem \
  -I$(OSAL_DIR)/common/include/modules/ddk \
  -I$(OSAL_DIR)/common/include/modules/bufferMgt \
  -I$(OSAL_DIR)/common/os/linux/include/modules/ioMem \
  -I$(OSAL_DIR)/common/os/linux/include/modules/ddk \
  -I$(OSAL_DIR)/common/os/linux/include/modules/bufferMgt \
  -I$(OSAL_DIR)/common/os/linux/include/core \
  -I$(IX_XSCALE_SW)/src/linux \
  -DUSE_IXP4XX_CRYPTO

OSAL_CFLAGS += \
  -D__linux \
  -DIX_TARGET=linuxbe \
  -DIX_UTOPIAMODE=0 \
  -DIX_MPHYSINGLEPORT=0 \
  -D__LINUX_ARM_ARCH__=5 \
  -DCPU=33 \
  -DXSCALE=33 \
	-DWall \
  -DENABLE_IOMEM \
  -DENABLE_BUFFERMGT

ifeq ($(CONFIG_CPU_IXP46X),y)
OSAL_CFLAGS += -D__ixp46X
OSAL_INCLUDES += -I$(OSAL_DIR)/platforms/ixp46X/include
OSAL_INCLUDES += -I$(OSAL_DIR)/platforms/ixp46X/os/linux/include
else
ifeq ($(CONFIG_CPU_IXP43X),y)
OSAL_CFLAGS += -D__ixp43X
OSAL_INCLUDES += -I$(OSAL_DIR)/platforms/ixp43X/include
OSAL_INCLUDES += -I$(OSAL_DIR)/platforms/ixp43X/os/linux/include
else
OSAL_CFLAGS += -D__ixp42X
OSAL_INCLUDES += -I$(OSAL_DIR)/platforms/ixp42X/include
OSAL_INCLUDES += -I$(OSAL_DIR)/platforms/ixp42X/os/linux/include
endif
endif

obj-$(CONFIG_OCF_IXP4XX) += ixp4xx.o

obj ?= .
EXTRA_CFLAGS += -I$(obj)/.. -I$(obj)/. \
			  -I$(IX_XSCALE_SW)/src/include \
			  ${OSAL_INCLUDES} \
			  ${OSAL_CFLAGS}

ifdef TOPDIR
-include $(TOPDIR)/Rules.make
endif

