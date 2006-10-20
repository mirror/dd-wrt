#
# Makefile for the Intel IXP400 Ethernet Device
#

obj-$(CONFIG_IXP400_ETH) += ixp400_eth.o

IX_XSCALE_SW := drivers/ixp400/ixp400_xscale_sw
OSAL_DIR := $(IX_XSCALE_SW)/../ixp_osal

ifeq ($(CPU_IXP46X),y)
EXTRA_CFLAGS += -D__ixp46X
EXTRA_CFLAGS += -DIX_DEVICE=ixp46X
EXTRA_CFLAGS += -DIX_PLATFORM=ixdp46x
else
EXTRA_CFLAGS += -D__ixp42X
EXTRA_CFLAGS += -DIX_DEVICE=ixp42X
EXTRA_CFLAGS += -DIX_PLATFORM=ixdp42x
endif

EXTRA_CFLAGS += \
	-D__linux \
	-DIX_TARGET=linuxbe \
	-DIX_UTOPIAMODE=0 \
	-DIX_MPHYSINGLEPORT=0 \
	-D__LINUX_ARM_ARCH__=5 \
	-DCPU=33 \
	-DXSCALE=33 \
	-I$(IX_XSCALE_SW)/src/include \
	-I$(OSAL_DIR)/ \
	-I$(OSAL_DIR)/os/linux/include/ \
	-I$(OSAL_DIR)/os/linux/include/core/  \
	-I$(OSAL_DIR)/os/linux/include/modules/ \
	-I$(OSAL_DIR)/os/linux/include/modules/ioMem/ \
	-I$(OSAL_DIR)/os/linux/include/modules/core/ \
	-I$(OSAL_DIR)/os/linux/include/modules/bufferMgt/ \
	-I$(OSAL_DIR)/os/linux/include/platforms/ \
	-I$(OSAL_DIR)/os/linux/include/platforms/ixp400/
ifeq ($(CPU_IXP46X),y)
EXTRA_CFLAGS += \
	-I$(OSAL_DIR)/os/linux/include/platforms/ixp400/ixp465/
else
EXTRA_CFLAGS += \
	-I$(OSAL_DIR)/os/linux/include/platforms/ixp400/ixp425/
endif
EXTRA_CFLAGS += \
	-I$(OSAL_DIR)/include/ \
	-I$(OSAL_DIR)/include/modules/ \
	-I$(OSAL_DIR)/include/modules/bufferMgt/ \
	-I$(OSAL_DIR)/include/modules/ioMem/ \
	-I$(OSAL_DIR)/include/modules/core/ \
	-I$(OSAL_DIR)/include/platforms/ \
	-I$(OSAL_DIR)/include/platforms/ixp400/
ifeq ($(CPU_IXP46X),y)
EXTRA_CFLAGS += \
	-I$(OSAL_DIR)/include/platforms/ixp400/ixp465/
else
EXTRA_CFLAGS += \
	-I$(OSAL_DIR)/include/platforms/ixp400/ixp425/
endif
