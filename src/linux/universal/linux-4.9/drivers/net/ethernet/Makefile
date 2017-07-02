#
# Makefile for the Linux network Ethernet device drivers.
#
IX_XSCALE_SW := drivers/ixp400/ixp400_xscale_sw
OSAL_DIR := $(IX_XSCALE_SW)/../ixp_osal

OSAL_INCLUDES += \
  -I$(OSAL_DIR)/ \
  -I$(OSAL_DIR)/common/include/ \
  -I$(OSAL_DIR)/common/include/modules/ioMem \
  -I$(OSAL_DIR)/common/include/modules/ddk \
  -I$(OSAL_DIR)/common/include/modules/bufferMgt \
  -I$(OSAL_DIR)/common/os/linux/include/modules/ioMem \
  -I$(OSAL_DIR)/common/os/linux/include/modules/ddk \
  -I$(OSAL_DIR)/common/os/linux/include/modules/bufferMgt \
  -I$(OSAL_DIR)/common/os/linux/include/core

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


CFLAGS_ixp400_eth.o	= -DWall \
			  -I$(IX_XSCALE_SW)/src/include \
			  ${OSAL_INCLUDES} \
			  ${OSAL_CFLAGS}

obj-$(CONFIG_NET_VENDOR_3COM) += 3com/
obj-$(CONFIG_NET_VENDOR_8390) += 8390/
obj-$(CONFIG_NET_VENDOR_ADAPTEC) += adaptec/
obj-$(CONFIG_GRETH) += aeroflex/
obj-$(CONFIG_NET_VENDOR_AGERE) += agere/
obj-$(CONFIG_NET_VENDOR_ALLWINNER) += allwinner/
obj-$(CONFIG_NET_VENDOR_ALTEON) += alteon/
obj-$(CONFIG_ALTERA_TSE) += altera/
obj-$(CONFIG_NET_VENDOR_AMAZON) += amazon/
obj-$(CONFIG_NET_VENDOR_AMD) += amd/
obj-$(CONFIG_NET_XGENE) += apm/
obj-$(CONFIG_NET_VENDOR_APPLE) += apple/
obj-$(CONFIG_NET_VENDOR_ARC) += arc/
obj-$(CONFIG_NET_VENDOR_ATHEROS) += atheros/
obj-$(CONFIG_NET_VENDOR_AURORA) += aurora/
obj-$(CONFIG_NET_CADENCE) += cadence/
obj-$(CONFIG_NET_BFIN) += adi/
obj-$(CONFIG_NET_VENDOR_BROADCOM) += broadcom/
obj-$(CONFIG_NET_VENDOR_BROCADE) += brocade/
obj-$(CONFIG_NET_CALXEDA_XGMAC) += calxeda/
obj-$(CONFIG_NET_VENDOR_CAVIUM) += cavium/
obj-$(CONFIG_NET_VENDOR_CHELSIO) += chelsio/
obj-$(CONFIG_NET_VENDOR_CIRRUS) += cirrus/
obj-$(CONFIG_NET_VENDOR_CISCO) += cisco/
obj-$(CONFIG_CX_ECAT) += ec_bhf.o
obj-$(CONFIG_DM9000) += davicom/
obj-$(CONFIG_DNET) += dnet.o
obj-$(CONFIG_NET_VENDOR_DEC) += dec/
obj-$(CONFIG_NET_VENDOR_DLINK) += dlink/
obj-$(CONFIG_NET_VENDOR_EMULEX) += emulex/
obj-$(CONFIG_NET_VENDOR_EZCHIP) += ezchip/
obj-$(CONFIG_NET_VENDOR_EXAR) += neterion/
obj-$(CONFIG_NET_VENDOR_FARADAY) += faraday/
obj-$(CONFIG_NET_VENDOR_FREESCALE) += freescale/
obj-$(CONFIG_NET_VENDOR_FUJITSU) += fujitsu/
obj-$(CONFIG_NET_VENDOR_HISILICON) += hisilicon/
obj-$(CONFIG_NET_VENDOR_HP) += hp/
obj-$(CONFIG_NET_VENDOR_IBM) += ibm/
obj-$(CONFIG_NET_VENDOR_INTEL) += intel/
obj-$(CONFIG_NET_VENDOR_I825XX) += i825xx/
obj-$(CONFIG_NET_VENDOR_XSCALE) += xscale/
obj-$(CONFIG_JME) += jme.o
obj-$(CONFIG_KORINA) += korina.o
obj-$(CONFIG_LANTIQ_ETOP) += lantiq_etop.o
obj-$(CONFIG_NET_VENDOR_MARVELL) += marvell/
obj-$(CONFIG_NET_VENDOR_MEDIATEK) += mtk/
obj-$(CONFIG_NET_VENDOR_MELLANOX) += mellanox/
obj-$(CONFIG_NET_VENDOR_MICREL) += micrel/
obj-$(CONFIG_NET_VENDOR_MICROCHIP) += microchip/
obj-$(CONFIG_NET_VENDOR_MOXART) += moxa/
obj-$(CONFIG_NET_VENDOR_MYRI) += myricom/
obj-$(CONFIG_FEALNX) += fealnx.o
obj-$(CONFIG_NET_VENDOR_NATSEMI) += natsemi/
obj-$(CONFIG_NET_VENDOR_NETRONOME) += netronome/
obj-$(CONFIG_NET_NETX) += netx-eth.o
obj-$(CONFIG_NET_VENDOR_NUVOTON) += nuvoton/
obj-$(CONFIG_NET_VENDOR_NVIDIA) += nvidia/
obj-$(CONFIG_LPC_ENET) += nxp/
obj-$(CONFIG_NET_VENDOR_OKI) += oki-semi/
obj-$(CONFIG_ETHOC) += ethoc.o
obj-$(CONFIG_NET_PACKET_ENGINE) += packetengines/
obj-$(CONFIG_NET_VENDOR_PASEMI) += pasemi/
obj-$(CONFIG_NET_VENDOR_QLOGIC) += qlogic/
obj-$(CONFIG_NET_VENDOR_QUALCOMM) += qualcomm/
obj-$(CONFIG_NET_VENDOR_REALTEK) += realtek/
obj-$(CONFIG_NET_VENDOR_RENESAS) += renesas/
obj-$(CONFIG_NET_VENDOR_RDC) += rdc/
obj-$(CONFIG_NET_VENDOR_ROCKER) += rocker/
obj-$(CONFIG_NET_VENDOR_SAMSUNG) += samsung/
obj-$(CONFIG_NET_VENDOR_SEEQ) += seeq/
obj-$(CONFIG_NET_VENDOR_SILAN) += silan/
obj-$(CONFIG_NET_VENDOR_SIS) += sis/
obj-$(CONFIG_SFC) += sfc/
obj-$(CONFIG_NET_VENDOR_SGI) += sgi/
obj-$(CONFIG_NET_VENDOR_SMSC) += smsc/
obj-$(CONFIG_NET_VENDOR_STMICRO) += stmicro/
obj-$(CONFIG_NET_VENDOR_SUN) += sun/
obj-$(CONFIG_NET_VENDOR_SYNOPSYS) += synopsys/
obj-$(CONFIG_NET_VENDOR_TEHUTI) += tehuti/
obj-$(CONFIG_NET_VENDOR_TI) += ti/
obj-$(CONFIG_TILE_NET) += tile/
obj-$(CONFIG_NET_VENDOR_TOSHIBA) += toshiba/
obj-$(CONFIG_NET_VENDOR_TUNDRA) += tundra/
obj-$(CONFIG_NET_VENDOR_VIA) += via/
obj-$(CONFIG_NET_VENDOR_WIZNET) += wiznet/
obj-$(CONFIG_NET_VENDOR_XILINX) += xilinx/
obj-$(CONFIG_NET_VENDOR_XIRCOM) += xircom/
obj-$(CONFIG_CNS3XXX_ETH) += cns3xxx_eth.o
obj-$(CONFIG_AG7100) += ag7100/
obj-$(CONFIG_AG7240) += ag7240/
obj-y += ag71xx/
obj-$(CONFIG_IXP400_ETH) += ixp400_eth.o
obj-$(CONFIG_LANTIQ) += ifxatm/
obj-$(CONFIG_AG934X) += ag934x/
obj-$(CONFIG_RAETH) += raeth/
obj-$(CONFIG_AR2313) += ar2313/
obj-$(CONFIG_ET) += et/
obj-$(CONFIG_BCM57XX) += bcm57xx/
obj-$(CONFIG_NET_AL_ETH) += al/
