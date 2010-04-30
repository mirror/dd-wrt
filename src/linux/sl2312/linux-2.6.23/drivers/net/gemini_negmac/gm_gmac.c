/*
 *  Ethernet device driver for Gemini SoC.
 *
 *  Copyright (C) 2006, Storlink, Corp.
 *  Copyright (C) 2008-2009, Paulius Zaleckas <paulius.zaleckas@teltonika.lt>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/compiler.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/io.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/mdio-gpio.h>

#define JUMBOFRAME 

#include "gmac.h"

#include "gm_gmac.h"



/* #define GMAX_TX_INTR_DISABLED */
/* #define DO_HW_CHKSUM */
/* #define ENABLE_TSO */
#define GMAC_USE_TXQ0
/* #define GMAC_LEN_1_2_ISSUE */

#define DEFAULT_RXQ_MAX_CNT			256


#ifdef	JUMBOFRAME
#define TCPHDRLEN(tcp_hdr2)  ((ntohs(*((__u16 *)tcp_hdr2 + 6)) >> 12) & 0x000F)
#endif

/* define chip information */
#define DRV_VERSION			"0.2"
#define SL351x_DRIVER_NAME		"Gemini Ethernet driver " DRV_VERSION

#ifdef GMAC_LEN_1_2_ISSUE
	#define _DEBUG_PREFETCH_NUM	256
static	int	_debug_prefetch_cnt;
static	char _debug_prefetch_buf[_DEBUG_PREFETCH_NUM][4] __attribute__((aligned(4)));
#endif

static inline void gmac_write_reg(unsigned int base, unsigned int offset, unsigned int data, unsigned int bit_mask)
{
	unsigned int reg_val;

	reg_val = (__raw_readl(base + offset) & (~bit_mask)) | (data & bit_mask);
	__raw_writel(reg_val, base + offset);
}

/*----------------------------------------------------------------------
*	toe_init_free_queue
*	(1) Initialize the Free Queue Descriptor Base Address & size
*		Register: TOE_GLOBAL_BASE + 0x0004
*	(2) Initialize DMA Read/Write pointer for
*		SW Free Queue and HW Free Queue
*	(3)	Initialize DMA Descriptors for
*		SW Free Queue and HW Free Queue,
*----------------------------------------------------------------------*/
static void toe_init_free_queue(struct toe_private *toe)
{
	int			i;
	DMA_RWPTR_T		rwptr_reg;
	void			*desc_buf;
	GMAC_RXDESC_T		*sw_desc_ptr;
	struct sk_buff		*skb;

	desc_buf = dma_alloc_coherent(toe->dev, TOE_SW_FREEQ_DESC_NUM * sizeof(GMAC_RXDESC_T),
				      &toe->sw_freeq_desc_base_dma, GFP_KERNEL);
	sw_desc_ptr = (GMAC_RXDESC_T *)desc_buf;
	if (!desc_buf) {
		dev_err(toe->dev, "%s::DMA ALLOC fail\n", __func__);
		return;
	}
	memset(desc_buf, 0, TOE_SW_FREEQ_DESC_NUM * sizeof(GMAC_RXDESC_T));

	/* DMA Queue Base & Size */
	__raw_writel((toe->sw_freeq_desc_base_dma & DMA_Q_BASE_MASK) | TOE_SW_FREEQ_DESC_POWER,
			toe->global_base + GLOBAL_SW_FREEQ_BASE_SIZE_REG);

	/* init descriptor base */
	toe->swfq_desc_base = (unsigned int)desc_buf;

	/* SW Free Queue Descriptors */
	for (i = 0; i < TOE_SW_FREEQ_DESC_NUM; i++) {
		sw_desc_ptr->word0.bits.buffer_size = SW_RX_BUF_SIZE;
		skb = dev_alloc_skb(SW_RX_BUF_SIZE);	/* allocate socket buffer */
		if (!skb) {
			dev_err(toe->dev, "%s::skb buffer allocation fail\n", __func__);
			return;
		}
		REG32(skb->data) = (unsigned int)skb;
		skb_reserve(skb, SKB_RESERVE_BYTES);
		sw_desc_ptr->word2.buf_adr = dma_map_single(toe->dev, skb->data,
					SW_RX_BUF_SIZE - SKB_RESERVE_BYTES,
					DMA_FROM_DEVICE);
		sw_desc_ptr++;
	}

	//dma_sync_single_for_device(toe->dev, toe->sw_freeq_desc_base_dma,
	//			TOE_SW_FREEQ_DESC_NUM * sizeof(GMAC_RXDESC_T),
	//			DMA_TO_DEVICE);

	/* SW Free Queue Read/Write Pointer */
	rwptr_reg.bits.wptr = TOE_SW_FREEQ_DESC_NUM - 1;
	rwptr_reg.bits.rptr = 0;
	__raw_writel(rwptr_reg.bits32, toe->global_base + GLOBAL_SWFQ_RWPTR_REG);

	/* DMA Queue Base & Size */
	__raw_writel(TOE_HW_FREEQ_DESC_POWER,
		toe->global_base + GLOBAL_HW_FREEQ_BASE_SIZE_REG);
	rwptr_reg.bits.wptr = TOE_HW_FREEQ_DESC_NUM - 1;
	rwptr_reg.bits.rptr = 0;
	__raw_writel(rwptr_reg.bits32, toe->global_base + GLOBAL_HWFQ_RWPTR_REG);
}

/*----------------------------------------------------------------------
*	toe_init_swtx_queue
*	(2) Initialize the GMAC 0/1 SW TXQ Queue Descriptor Base Address & sizeup
*		GMAC_SW_TX_QUEUE_BASE_REG(0x0050)
*	(2) Initialize DMA Read/Write pointer for
*		GMAC 0/1 SW TX Q0-5
*----------------------------------------------------------------------*/
static void toe_init_swtx_queue(struct net_device *dev)
{
	int			i;
	struct gmac_private	*gmac = netdev_priv(dev);
	struct toe_private	*toe = gmac->toe;
	DMA_RWPTR_T		rwptr_reg;
	unsigned int		rwptr_addr;
	void			*desc_buf;
	unsigned int		offset;

	desc_buf = dma_alloc_coherent(toe->dev, TOE_GMAC_SWTXQ_DESC_NUM * TOE_SW_TXQ_NUM * sizeof(GMAC_TXDESC_T),
				      &gmac->swtxq_desc_base_dma, GFP_KERNEL);
	gmac->swtxq_desc_base = (unsigned int)desc_buf;
	if (!desc_buf) {
		dev_err(toe->dev, "%s::DMA ALLOC fail\n", __func__);
		return;
	}
	memset(desc_buf, 0, TOE_GMAC_SWTXQ_DESC_NUM * TOE_SW_TXQ_NUM * sizeof(GMAC_TXDESC_T));
	//dma_sync_single_for_device(toe->dev, gmac->swtxq_desc_base_dma,
	//			TOE_GMAC_SWTXQ_DESC_NUM * TOE_SW_TXQ_NUM * sizeof(GMAC_TXDESC_T),
	//			DMA_TO_DEVICE);
	__raw_writel((gmac->swtxq_desc_base_dma & DMA_Q_BASE_MASK) | TOE_GMAC_SWTXQ_DESC_POWER,
			gmac->dma_base_addr + GMAC_SW_TX_QUEUE_BASE_REG);

	/* GMAC0 SW TX Q0-Q5 */
	offset = 0;
	rwptr_reg.bits.wptr = 0;
	rwptr_reg.bits.rptr = 0;
	rwptr_addr = gmac->dma_base_addr + GMAC_SW_TX_QUEUE0_PTR_REG;
	for (i = 0; i < TOE_SW_TXQ_NUM; i++) {
		gmac->swtxq[i].rwptr_reg = rwptr_addr;
		gmac->swtxq[i].desc_base_dma = (unsigned int)gmac->swtxq_desc_base_dma + offset;
		gmac->swtxq[i].desc_base = (unsigned int)desc_buf + offset;
		offset += TOE_GMAC_SWTXQ_DESC_NUM * sizeof(GMAC_TXDESC_T);
		__raw_writel(rwptr_reg.bits32, rwptr_addr);
		rwptr_addr += 4;
	}
}

/*----------------------------------------------------------------------
*	toe_init_default_queue
*	(1) Initialize the default 0/1 Queue Header
*		Register: TOE_DEFAULT_Q0_HDR_BASE (0x60002000)
*			  TOE_DEFAULT_Q1_HDR_BASE (0x60002008)
*	(2)	Initialize Descriptors of Default Queue 0/1
*----------------------------------------------------------------------*/
static void toe_init_default_queue(struct net_device *dev)
{
	struct gmac_private	*gmac = netdev_priv(dev);
	struct toe_private	*toe = gmac->toe;
	volatile NONTOE_QHDR_T	*qhdr;
	GMAC_RXDESC_T		*desc_ptr;

	desc_ptr = dma_alloc_coherent(toe->dev, TOE_DEFAULT_Q_DESC_NUM * sizeof(GMAC_RXDESC_T),
				      &gmac->default_desc_base_dma, GFP_KERNEL);
	if (!desc_ptr) {
		dev_err(toe->dev, "%s::DMA ALLOC fail\n", __func__);
		return;
	}
	memset(desc_ptr, 0, TOE_DEFAULT_Q_DESC_NUM * sizeof(GMAC_RXDESC_T));
	//dma_sync_single_for_device(toe->dev, gmac->default_desc_base_dma,
	//		TOE_DEFAULT_Q_DESC_NUM * sizeof(GMAC_RXDESC_T),
	//		DMA_TO_DEVICE);
	gmac->default_desc_base = (unsigned int)desc_ptr;
	qhdr = (volatile NONTOE_QHDR_T *)(toe->global_base + TOE_DEFAULT_Q_HDR_BASE(gmac->port_id));
	qhdr->word0.base_size = ((unsigned int)gmac->default_desc_base_dma & NONTOE_QHDR0_BASE_MASK) | TOE_DEFAULT_Q_DESC_POWER;
	qhdr->word1.bits32 = 0;
	gmac->default_qhdr = (NONTOE_QHDR_T *)qhdr;
}

/*----------------------------------------------------------------------
*	toe_init_interrupt_config
*	Interrupt Select Registers are used to map interrupt to int0 or int1
*	Int0 and int1 are wired to CPU 0/1 GMAC 0/1
*	Interrupt Device Inteface data are used to pass device info to
*		upper device driver or store status/statistics
*	ISR handler
*		(1) If status bit ON but masked, the prinf error message (bug issue)
*		(2) If select bits are for me, handle it, else skip to let
*			the other ISR handles it.
*  Notes:
*		GMACx init routine (for eCOS) or open routine (for Linux)
*		enable the interrupt bits only which are selected for it.
*
*	Default Setting:
*		GMAC0 intr bits ------>	int0 ----> eth0
*		GMAC1 intr bits ------> int1 ----> eth1
*		TOE intr -------------> int0 ----> eth0
*		Classification Intr --> int0 ----> eth0
*		Default Q0 -----------> int0 ----> eth0
*		Default Q1 -----------> int1 ----> eth1
*----------------------------------------------------------------------*/
static void toe_init_interrupt_config(struct toe_private *toe)
{
	/* clear all status bits */
	__raw_writel(0xffffffff, toe->global_base + GLOBAL_INTERRUPT_STATUS_0_REG);
	__raw_writel(0xffffffff, toe->global_base + GLOBAL_INTERRUPT_STATUS_1_REG);
	__raw_writel(0xffffffff, toe->global_base + GLOBAL_INTERRUPT_STATUS_2_REG);
	__raw_writel(0xffffffff, toe->global_base + GLOBAL_INTERRUPT_STATUS_3_REG);
	__raw_writel(0xffffffff, toe->global_base + GLOBAL_INTERRUPT_STATUS_4_REG);

	/* Init select registers */
	__raw_writel(0, toe->global_base + GLOBAL_INTERRUPT_SELECT_0_REG);
	__raw_writel(0, toe->global_base + GLOBAL_INTERRUPT_SELECT_1_REG);
	__raw_writel(0, toe->global_base + GLOBAL_INTERRUPT_SELECT_2_REG);
	__raw_writel(0, toe->global_base + GLOBAL_INTERRUPT_SELECT_3_REG);
	__raw_writel(0, toe->global_base + GLOBAL_INTERRUPT_SELECT_4_REG);

	/* disable all interrupt */
	__raw_writel(0, toe->global_base + GLOBAL_INTERRUPT_ENABLE_0_REG);
	__raw_writel(0, toe->global_base + GLOBAL_INTERRUPT_ENABLE_1_REG);
	__raw_writel(0, toe->global_base + GLOBAL_INTERRUPT_ENABLE_2_REG);
	__raw_writel(0, toe->global_base + GLOBAL_INTERRUPT_ENABLE_3_REG);
	__raw_writel(0, toe->global_base + GLOBAL_INTERRUPT_ENABLE_4_REG);
}

static void toe_gmac_hw_start(struct gmac_private *gmac)
{
	GMAC_DMA_CTRL_T	dma_ctrl;

	/* program dma control register */
	dma_ctrl.bits32 = __raw_readl(gmac->dma_base_addr + GMAC_DMA_CTRL_REG);
	dma_ctrl.bits.rd_enable = 1;
	dma_ctrl.bits.td_enable = 1;
	dma_ctrl.bits.loopback = 0;
	dma_ctrl.bits.drop_small_ack = 0;
	dma_ctrl.bits.rd_prot = 0;
	dma_ctrl.bits.rd_burst_size = 3;
	dma_ctrl.bits.rd_insert_bytes = RX_INSERT_BYTES;
	dma_ctrl.bits.rd_bus = 3;
	dma_ctrl.bits.td_prot = 0;
	dma_ctrl.bits.td_burst_size = 3;
	dma_ctrl.bits.td_bus = 3;

	__raw_writel(dma_ctrl.bits32, gmac->dma_base_addr + GMAC_DMA_CTRL_REG);
}

static void toe_gmac_hw_stop(struct gmac_private *gmac)
{
	GMAC_DMA_CTRL_T	dma_ctrl;

	/* program dma control register */
	dma_ctrl.bits32 = __raw_readl(gmac->dma_base_addr + GMAC_DMA_CTRL_REG);
	dma_ctrl.bits.rd_enable = 0;
	dma_ctrl.bits.td_enable = 0;
	__raw_writel(dma_ctrl.bits32, gmac->dma_base_addr + GMAC_DMA_CTRL_REG);
}

static void toe_gmac_init_chip(struct net_device *dev)
{
	struct gmac_private	*gmac = netdev_priv(dev);
	GMAC_CONFIG2_T	config2_val;
	GMAC_CONFIG0_T	config0;
	GMAC_CONFIG1_T	config1;
	GMAC_STATUS_T	status;
	GMAC_TX_WCR0_T	hw_weigh;
	GMAC_TX_WCR1_T	sw_weigh;
	GMAC_RX_FLTR_T	rx_filter;

	/* set RX_FLTR register to receive all multicast packet */
	rx_filter.bits32 = __raw_readl(dev->base_addr + GMAC_RX_FLTR);
	rx_filter.bits.unicast = 1;
	rx_filter.bits.multicast = 1;
	rx_filter.bits.broadcast = 1;
	__raw_writel(rx_filter.bits32, dev->base_addr + GMAC_RX_FLTR);

	/* set flow control threshold */
	config1.bits32 = 0;
	config1.bits.set_threshold = 32 / 2;
	config1.bits.rel_threshold = 32 / 4 * 3;
	__raw_writel(config1.bits32, dev->base_addr + GMAC_CONFIG1);

	/* set flow control threshold */
	config2_val.bits32 = 0;
	config2_val.bits.set_threshold = TOE_SW_FREEQ_DESC_NUM / 4;
	config2_val.bits.rel_threshold = TOE_SW_FREEQ_DESC_NUM / 2;
	__raw_writel(config2_val.bits32, dev->base_addr + GMAC_CONFIG2);

	/* disable TX/RX and disable internal loop back */
	config0.bits32 = __raw_readl(dev->base_addr + GMAC_CONFIG0);

#ifdef JUMBOFRAME
	config0.bits.max_len = 5;
#else
	config0.bits.max_len = 2;
#endif
	gmac->flow_control_enable = 0;

	config0.bits.tx_fc_en = 0;	/* disable tx flow control */
	config0.bits.rx_fc_en = 0;	/* disable rx flow control */
	config0.bits.dis_rx = 1;	/* disable rx */
	config0.bits.dis_tx = 1;	/* disable tx */
	config0.bits.loop_back = 0;	/* enable/disable GMAC loopback */
	config0.bits.rx_err_detect = 1;
	config0.bits.rgmii_en = 0;
	config0.bits.rgmm_edge = 1;
	config0.bits.rxc_inv = 0;
	config0.bits.ipv4_rx_chksum = 1;	/* enable H/W to check ip checksum */
	config0.bits.ipv6_rx_chksum = 1;	/* enable H/W to check ip checksum */
	config0.bits.port0_chk_hwq = 1;
	config0.bits.port1_chk_hwq = 1;
	config0.bits.port0_chk_toeq = 1;
	config0.bits.port1_chk_toeq = 1;
	config0.bits.port0_chk_classq = 1;
	config0.bits.port1_chk_classq = 1;

	__raw_writel(config0.bits32, dev->base_addr + GMAC_CONFIG0);

	hw_weigh.bits32 = 0;
	hw_weigh.bits.hw_tq3 = 1;
	hw_weigh.bits.hw_tq2 = 1;
	hw_weigh.bits.hw_tq1 = 1;
	hw_weigh.bits.hw_tq0 = 1;
	__raw_writel(hw_weigh.bits32, gmac->dma_base_addr + GMAC_TX_WEIGHTING_CTRL_0_REG);

	sw_weigh.bits32 = 0;
	sw_weigh.bits.sw_tq5 = 1;
	sw_weigh.bits.sw_tq4 = 1;
	sw_weigh.bits.sw_tq3 = 1;
	sw_weigh.bits.sw_tq2 = 1;
	sw_weigh.bits.sw_tq1 = 1;
	sw_weigh.bits.sw_tq0 = 1;
	__raw_writel(sw_weigh.bits32, gmac->dma_base_addr + GMAC_TX_WEIGHTING_CTRL_1_REG);

	/* set interface type */
	status.bits32 = __raw_readl(dev->base_addr + GMAC_STATUS);

	switch (gmac->phydev->interface) {
	case PHY_INTERFACE_MODE_MII:
		status.bits.mii_rmii = GMAC_PHY_MII;
		break;
	case PHY_INTERFACE_MODE_GMII:
		status.bits.mii_rmii = GMAC_PHY_GMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
		status.bits.mii_rmii = GMAC_PHY_RGMII_100_10;
		break;
	default:
		dev_err(&dev->dev, "Unsupported MII interface\n");
		return;
	}
//	status.bits.link = 1;

	__raw_writel(status.bits32, dev->base_addr + GMAC_STATUS);
}

static void toe_init_gmac(struct net_device *dev)
{
	struct gmac_private	*gmac = netdev_priv(dev);
	struct toe_private	*toe = gmac->toe;
	u32			data;

	/* GMAC initialization */
	toe_gmac_init_chip(dev);

	/* -----------------------------------------------------------
	Enable GMAC interrupt & disable loopback
	Notes:
		GMACx init routine (for eCOS) or open routine (for Linux)
		enable the interrupt bits only which are selected for him.
	--------------------------------------------------------------*/

	/* Enable Interrupt Bits */
	if (gmac->port_id == 0) {
		gmac->intr0_selected =	GMAC0_TXDERR_INT_BIT | GMAC0_TXPERR_INT_BIT	|
					GMAC0_RXDERR_INT_BIT | GMAC0_RXPERR_INT_BIT	|
					GMAC0_SWTQ05_FIN_INT_BIT | GMAC0_SWTQ05_EOF_INT_BIT |
					GMAC0_SWTQ04_FIN_INT_BIT | GMAC0_SWTQ04_EOF_INT_BIT |
					GMAC0_SWTQ03_FIN_INT_BIT | GMAC0_SWTQ03_EOF_INT_BIT |
					GMAC0_SWTQ02_FIN_INT_BIT | GMAC0_SWTQ02_EOF_INT_BIT |
					GMAC0_SWTQ01_FIN_INT_BIT | GMAC0_SWTQ01_EOF_INT_BIT |
					GMAC0_SWTQ00_FIN_INT_BIT | GMAC0_SWTQ00_EOF_INT_BIT;

#ifdef GMAX_TX_INTR_DISABLED
		gmac->intr0_enabled = 0;
#else
		gmac->intr0_enabled = GMAC0_SWTQ00_FIN_INT_BIT | GMAC0_SWTQ00_EOF_INT_BIT;
#endif

		gmac->intr1_selected = TOE_IQ_ALL_BITS | TOE_CLASS_RX_INT_BITS |
					GMAC0_HWTQ03_EOF_INT_BIT | GMAC0_HWTQ02_EOF_INT_BIT |
					GMAC0_HWTQ01_EOF_INT_BIT | GMAC0_HWTQ00_EOF_INT_BIT |
					DEFAULT_Q0_INT_BIT;
		gmac->intr1_enabled =	DEFAULT_Q0_INT_BIT | TOE_IQ_ALL_BITS;
		gmac->intr2_selected =	0xffffffff;	 /* TOE Queue 32-63 FUUL Intr */
		gmac->intr2_enabled =	0xffffffff;
		gmac->intr3_selected =	0xffffffff;	 /* TOE Queue 0-31 FUUL Intr */
		gmac->intr3_enabled =	0xffffffff;
		gmac->intr4_selected =	GMAC0_INT_BITS | CLASS_RX_FULL_INT_BITS |
							HWFQ_EMPTY_INT_BIT | SWFQ_EMPTY_INT_BIT;
		gmac->intr4_enabled =	GMAC0_INT_BITS | SWFQ_EMPTY_INT_BIT;

		data = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_SELECT_0_REG) & ~gmac->intr0_selected;
		__raw_writel(data, toe->global_base + GLOBAL_INTERRUPT_SELECT_0_REG);
		data = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_SELECT_1_REG) & ~gmac->intr1_selected;
		__raw_writel(data, toe->global_base + GLOBAL_INTERRUPT_SELECT_1_REG);
		data = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_SELECT_2_REG) & ~gmac->intr2_selected;
		__raw_writel(data, toe->global_base + GLOBAL_INTERRUPT_SELECT_2_REG);
		data = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_SELECT_3_REG) & ~gmac->intr3_selected;
		__raw_writel(data, toe->global_base + GLOBAL_INTERRUPT_SELECT_3_REG);
		data = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_SELECT_4_REG) & ~gmac->intr4_selected;
		__raw_writel(data, toe->global_base + GLOBAL_INTERRUPT_SELECT_4_REG);
	} else {
		gmac->intr0_selected =	GMAC1_TXDERR_INT_BIT | GMAC1_TXPERR_INT_BIT	|
					GMAC1_RXDERR_INT_BIT | GMAC1_RXPERR_INT_BIT	|
					GMAC1_SWTQ15_FIN_INT_BIT | GMAC1_SWTQ15_EOF_INT_BIT |
					GMAC1_SWTQ14_FIN_INT_BIT | GMAC1_SWTQ14_EOF_INT_BIT |
					GMAC1_SWTQ13_FIN_INT_BIT | GMAC1_SWTQ13_EOF_INT_BIT |
					GMAC1_SWTQ12_FIN_INT_BIT | GMAC1_SWTQ12_EOF_INT_BIT |
					GMAC1_SWTQ11_FIN_INT_BIT | GMAC1_SWTQ11_EOF_INT_BIT |
					GMAC1_SWTQ10_FIN_INT_BIT | GMAC1_SWTQ10_EOF_INT_BIT;
#ifdef GMAX_TX_INTR_DISABLED
		gmac->intr0_enabled =		0;
#else
		gmac->intr0_enabled =		GMAC1_SWTQ10_FIN_INT_BIT | GMAC1_SWTQ10_EOF_INT_BIT;
#endif

		gmac->intr1_selected =	DEFAULT_Q1_INT_BIT;
		gmac->intr1_enabled =	DEFAULT_Q1_INT_BIT | TOE_IQ_ALL_BITS;
		gmac->intr2_selected =	0;	 /* TOE Queue 32-63 FUUL Intr */
		gmac->intr2_enabled =	0;
		gmac->intr3_selected =	0;	 /* TOE Queue 0-31 FUUL Intr */
		gmac->intr3_enabled =	0;
		gmac->intr4_selected =	GMAC1_INT_BITS;
		gmac->intr4_enabled =	GMAC1_INT_BITS;

		data = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_SELECT_0_REG) | gmac->intr0_selected;
		__raw_writel(data, toe->global_base + GLOBAL_INTERRUPT_SELECT_0_REG);
		data = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_SELECT_1_REG) | gmac->intr1_selected;
		__raw_writel(data, toe->global_base + GLOBAL_INTERRUPT_SELECT_1_REG);
		data = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_SELECT_2_REG) | gmac->intr2_selected;
		__raw_writel(data, toe->global_base + GLOBAL_INTERRUPT_SELECT_2_REG);
		data = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_SELECT_3_REG) | gmac->intr3_selected;
		__raw_writel(data, toe->global_base + GLOBAL_INTERRUPT_SELECT_3_REG);
		data = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_SELECT_4_REG) | gmac->intr4_selected;
		__raw_writel(data, toe->global_base + GLOBAL_INTERRUPT_SELECT_4_REG);
	}

	/* enable only selected bits */
	gmac_write_reg(toe->global_base, GLOBAL_INTERRUPT_ENABLE_0_REG,
					gmac->intr0_enabled, gmac->intr0_selected);
	gmac_write_reg(toe->global_base, GLOBAL_INTERRUPT_ENABLE_1_REG,
					gmac->intr1_enabled, gmac->intr1_selected);
	gmac_write_reg(toe->global_base, GLOBAL_INTERRUPT_ENABLE_2_REG,
					gmac->intr2_enabled, gmac->intr2_selected);
	gmac_write_reg(toe->global_base, GLOBAL_INTERRUPT_ENABLE_3_REG,
					gmac->intr3_enabled, gmac->intr3_selected);
	gmac_write_reg(toe->global_base, GLOBAL_INTERRUPT_ENABLE_4_REG,
					gmac->intr4_enabled, gmac->intr4_selected);

	/* start DMA process */
	toe_gmac_hw_start(gmac);
}

static void toe_gmac_enable_tx_rx(struct net_device *dev)
{
	GMAC_CONFIG0_T	config0;

	/* enable TX/RX */
	config0.bits32 = __raw_readl(dev->base_addr + GMAC_CONFIG0);
	config0.bits.dis_rx = 0;	/* enable rx */
	config0.bits.dis_tx = 0;	/* enable tx */
	__raw_writel(config0.bits32, dev->base_addr + GMAC_CONFIG0);
}

static void toe_gmac_disable_tx_rx(struct net_device *dev)
{
	GMAC_CONFIG0_T	config0;

	/* enable TX/RX */
	config0.bits32 = __raw_readl(dev->base_addr + GMAC_CONFIG0);
	config0.bits.dis_rx = 1;	/* disable rx */
	config0.bits.dis_tx = 1;	/* disable tx */
	__raw_writel(config0.bits32, dev->base_addr + GMAC_CONFIG0);
}

static void toe_gmac_tx_complete(struct net_device *dev, unsigned int tx_qid)
{
	struct gmac_private		*gmac = netdev_priv(dev);
	struct toe_private		*toe = gmac->toe;
	GMAC_TXDESC_T			*curr_desc;
	GMAC_TXDESC_0_T			word0;
	GMAC_TXDESC_1_T			word1;
	unsigned int			desc_count;
	GMAC_SWTXQ_T			*swtxq;
	DMA_RWPTR_T			rwptr;

	/* get tx H/W completed descriptor virtual address */
	/* check tx status and accumulate tx statistics */
	swtxq = &gmac->swtxq[tx_qid];
	for (;;) {
		rwptr.bits32 = __raw_readl(swtxq->rwptr_reg);
		if (rwptr.bits.rptr == swtxq->finished_idx)
			break;
		curr_desc = (GMAC_TXDESC_T *)swtxq->desc_base + swtxq->finished_idx;
		//dma_sync_single_for_device(toe->dev, swtxq->desc_base_dma+(swtxq->finished_idx*sizeof(GMAC_TXDESC_T)), sizeof(GMAC_TXDESC_T),DMA_FROM_DEVICE);
		word0.bits32 = curr_desc->word0.bits32;
		word1.bits32 = curr_desc->word1.bits32;

		if (word0.bits.status_tx_ok) {
			dev->stats.tx_bytes += word1.bits.byte_count;
			desc_count = word0.bits.desc_count;
			if (desc_count == 0) {
				dev_err(&dev->dev, "%s::Desc 0x%x = 0x%x, desc_count=%d\n", __func__, (u32)curr_desc, word0.bits32, desc_count);
				BUG();
			}
			while (--desc_count) {
				word0.bits.status_tx_ok = 0;
				curr_desc->word0.bits32 = word0.bits32;
				//dma_sync_single_for_device(toe->dev, swtxq->desc_base_dma+swtxq->finished_idx * sizeof(GMAC_TXDESC_T),sizeof(GMAC_TXDESC_T),
				//				DMA_TO_DEVICE);
				swtxq->finished_idx = RWPTR_ADVANCE_ONE(swtxq->finished_idx, TOE_GMAC_SWTXQ_DESC_NUM);
				curr_desc = (GMAC_TXDESC_T *)swtxq->desc_base + swtxq->finished_idx;
				//dma_sync_single_for_device(toe->dev, swtxq->desc_base_dma+swtxq->finished_idx * sizeof(GMAC_TXDESC_T),sizeof(GMAC_TXDESC_T),DMA_FROM_DEVICE);
				word0.bits32 = curr_desc->word0.bits32;
			}

			word0.bits.status_tx_ok = 0;
			dev_kfree_skb_any(swtxq->tx_skb[swtxq->finished_idx]);
			swtxq->tx_skb[swtxq->finished_idx] = NULL;

			curr_desc->word0.bits32 = word0.bits32;
			//dma_sync_single_for_device(toe->dev, swtxq->desc_base_dma+
			//				swtxq->finished_idx * sizeof(GMAC_TXDESC_T),sizeof(GMAC_TXDESC_T),DMA_TO_DEVICE);
			dev->stats.tx_packets++;
			swtxq->finished_idx = RWPTR_ADVANCE_ONE(swtxq->finished_idx, TOE_GMAC_SWTXQ_DESC_NUM);
		} else {
			break;
		}
	}

	if (netif_queue_stopped(dev))
		netif_wake_queue(dev);
}

static int gmac_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct gmac_private	*gmac = netdev_priv(dev);
	struct toe_private	*toe = gmac->toe;
	DMA_RWPTR_T		rwptr;
	GMAC_TXDESC_T		*curr_desc;
	int			snd_pages = skb_shinfo(skb)->nr_frags + 1;	/* get number of descriptor */
	int			frag_id = 0;
	int			len, total_len = skb->len;
	struct net_device_stats	*isPtr = &dev->stats;
	unsigned int		free_desc;
	GMAC_SWTXQ_T		*swtxq;
	register unsigned long	word0, word1, word2, word3;
	unsigned short		wptr, rptr;
#ifdef	JUMBOFRAME
	int header_len = skb->len;
	struct iphdr	*ip_hdr2;
	struct tcphdr	*tcp_hdr2;
	int             tcp_hdr_len;
	unsigned char 	*ptr;
	int             data_len,a;
	unsigned int    val;
#endif

#ifdef GMAC_LEN_1_2_ISSUE
	int			total_pages;
	total_pages = snd_pages;
#endif

	if (skb->len >= 0x10000) {
		isPtr->tx_dropped++;
		dev_err(&dev->dev, "%s::skb->len %d >= 64K\n", __func__, skb->len);
		netif_stop_queue(dev);
		return 1;
	}

#ifdef GMAC_USE_TXQ0
	#define tx_qid	0
#endif

	swtxq = &gmac->swtxq[tx_qid];

	rwptr.bits32 = __raw_readl(swtxq->rwptr_reg);
	wptr = rwptr.bits.wptr;
	rptr = rwptr.bits.rptr;

	/*
	 * check finished desc or empty BD
	 * cannot check by read ptr of RW PTR register,
	 * because the HW complete to send but the SW may NOT handle it
	 */
#ifdef	GMAX_TX_INTR_DISABLED
	toe_gmac_tx_complete(dev, tx_qid);
#endif
	if (wptr >= swtxq->finished_idx)
		free_desc = TOE_GMAC_SWTXQ_DESC_NUM - wptr + swtxq->finished_idx;
	else
		free_desc = swtxq->finished_idx - wptr;

	if (free_desc < snd_pages) {
		isPtr->tx_dropped++;
		netif_stop_queue(dev);
		return 1;
	}

#ifdef	JUMBOFRAME
		if (((ip_hdr(skb)->protocol & 0x00ff)  == IPPROTO_TCP))
		{
				tcp_hdr2 = tcp_hdr(skb);
				tcp_hdr_len = TCPHDRLEN(tcp_hdr2) * 4;
				tcp_hdr_len = TCPHDRLEN(tcp_hdr2) * 4;

				if ((tcp_hdr2->syn) && (tcp_hdr_len > 20))
				{
					ptr = (unsigned char *)(tcp_hdr2+1);
					if ((ptr[0] == 0x02) && (ptr[1] == 0x04) && (ptr[2] == 0x07) && (ptr[3] == 0xba)) // 0x07 aa=2016-54=1962  ,0x07ba=2032-54=1978
					{
						ptr[2]=0x20;	//23
						ptr[3]=0x00;   	//00
						printk("-----> Change MSS to 8K \n" );
					}
				}
		}
#endif

	while (snd_pages) {
		char *pkt_datap;

		curr_desc = (GMAC_TXDESC_T *)swtxq->desc_base + wptr;
		if (frag_id == 0) {
			len = skb_headlen(skb);
			pkt_datap = dma_map_single(toe->dev, skb->data, len, DMA_TO_DEVICE);
		} else {
			skb_frag_t *frag = &skb_shinfo(skb)->frags[frag_id - 1];
			len = frag->size;
			pkt_datap = dma_map_page(toe->dev, frag->page, frag->page_offset, len, DMA_TO_DEVICE);
		}

		/* set TX descriptor */
		word0 = len;
		word3 = (dev->mtu + 14) | EOFIE_BIT;

#ifdef DO_HW_CHKSUM
#ifdef	JUMBOFRAME
		if (total_len >= (dev->mtu+14) && ip_hdr(skb) && (ip_hdr(skb)->protocol == 0x011) && (ip_hdr(skb)->frag_off & __constant_htons(0x3fff)))
#else
		if (total_len <= 1514 && ip_hdr(skb) && (ip_hdr(skb)->frag_off & __constant_htons(0x3fff)))
#endif
			word1  = total_len |
					TSS_IP_CHKSUM_BIT  |
					TSS_IPV6_ENABLE_BIT |
					TSS_MTU_ENABLE_BIT;
		else
			word1 = total_len |
					TSS_UDP_CHKSUM_BIT |
					TSS_TCP_CHKSUM_BIT |
					TSS_IP_CHKSUM_BIT  |
					TSS_IPV6_ENABLE_BIT |
					TSS_MTU_ENABLE_BIT;
#else
		word1 = total_len | TSS_MTU_ENABLE_BIT;
#endif
		word2 = pkt_datap;

		if (frag_id == 0)
			word3 |= SOF_BIT;

		if (snd_pages == 1) {
			word3 |= EOF_BIT;
			swtxq->tx_skb[wptr] = skb;
		} else
			swtxq->tx_skb[wptr] = NULL;

#ifdef GMAC_LEN_1_2_ISSUE
		if ((total_pages != snd_pages) && (len == 1 || len == 2) && ((u32)pkt_datap & 0x03)) {
			memcpy((void *)&_debug_prefetch_buf[_debug_prefetch_cnt][0], pkt_datap, len);
			pkt_datap = (char *)&_debug_prefetch_buf[_debug_prefetch_cnt][0];
			word2 = (unsigned long)__pa(pkt_datap);
			_debug_prefetch_cnt++;
			if (_debug_prefetch_cnt >= _DEBUG_PREFETCH_NUM)
				_debug_prefetch_cnt = 0;
		}
#endif
		curr_desc->word0.bits32 = word0;
		curr_desc->word1.bits32 = word1;
		curr_desc->word2.bits32 = word2;
		curr_desc->word3.bits32 = word3;
		free_desc--;

		//dma_sync_single_for_device(toe->dev, swtxq->desc_base_dma+
		//				wptr * sizeof(GMAC_TXDESC_T),
		//				sizeof(GMAC_TXDESC_T),
		//				DMA_TO_DEVICE);

		wptr = RWPTR_ADVANCE_ONE(wptr, TOE_GMAC_SWTXQ_DESC_NUM);
		frag_id++;
		snd_pages--;
	}

	SET_WPTR(swtxq->rwptr_reg, wptr);
	dev->trans_start = jiffies;

	return 0;
}

static void __gmac_set_mac_address(struct net_device *dev)
{
	unsigned int    reg_val;

	reg_val = dev->dev_addr[0] + (dev->dev_addr[1] << 8) +
		  (dev->dev_addr[2] << 16) + (dev->dev_addr[3] << 24);
	__raw_writel(reg_val, dev->base_addr + GMAC_STA_ADD0);
	reg_val = (__raw_readl(dev->base_addr + GMAC_STA_ADD1) & 0xFFFF0000) +
		  dev->dev_addr[4] + (dev->dev_addr[5] << 8);
	__raw_writel(reg_val, dev->base_addr + GMAC_STA_ADD1);
}

static int gmac_set_mac_address(struct net_device *dev, void *addr)
{
	struct sockaddr *sa = addr;

	memcpy(dev->dev_addr, sa->sa_data, dev->addr_len);

	__gmac_set_mac_address(dev);

	return 0;
}

static void gmac_get_mac_address(struct net_device *dev)
{
	unsigned int reg_val;
	reg_val = __raw_readl(dev->base_addr + GMAC_STA_ADD0);
	dev->dev_addr[0] = reg_val & 0xFF;
	dev->dev_addr[1] = (reg_val >> 8) & 0xFF;
	dev->dev_addr[2] = (reg_val >> 16) & 0xFF;
	dev->dev_addr[3] = (reg_val >> 24) & 0xFF;
	reg_val = __raw_readl(dev->base_addr + GMAC_STA_ADD1);
	dev->dev_addr[4] = reg_val & 0xFF;
	dev->dev_addr[5] = (reg_val >> 8) & 0xFF;

	if (!is_valid_ether_addr(dev->dev_addr)) {
		random_ether_addr(dev->dev_addr);
		__gmac_set_mac_address(dev);
	}
}

static struct net_device_stats *gmac_get_stats(struct net_device *dev)
{
	if (netif_running(dev)) {
		unsigned short multicast;

		multicast = __raw_readw(dev->base_addr + GMAC_IN_MCAST) +
				__raw_readw(dev->base_addr + GMAC_IN_BCAST);

		dev->stats.rx_dropped += __raw_readw(dev->base_addr + GMAC_IN_DISCARDS);
		dev->stats.rx_errors += __raw_readw(dev->base_addr + GMAC_IN_ERRORS);
		dev->stats.rx_packets += __raw_readl(dev->base_addr + GMAC_IN_MAC1) + multicast;
		dev->stats.multicast += multicast;
	}

	return &dev->stats;
}

/* TODO: If possible use crc32 from kernel lib */
static unsigned const ethernet_polynomial = 0x04c11db7U;
static unsigned int ether_crc(int length, unsigned char *data)
{
	int crc = -1;
	unsigned int i;
	unsigned int crc_val = 0;

	while (--length >= 0) {
		unsigned char current_octet = *data++;
		int bit;
		for (bit = 0; bit < 8; bit++, current_octet >>= 1)
			crc = (crc << 1) ^ ((crc < 0) ^ (current_octet & 1) ?
				ethernet_polynomial : 0);
	}
	crc = ~crc;
	for (i = 0; i < 32; i++)
		crc_val = crc_val + (((crc << i) & 0x80000000) >> (31 - i));

	return crc_val;
}

/*----------------------------------------------------------------------
* toe_gmac_fill_free_q
* allocate buffers for free queue.
*----------------------------------------------------------------------*/
static void toe_gmac_fill_free_q(struct toe_private *toe)
{
	struct sk_buff	*skb;
	DMA_RWPTR_T	fq_rwptr;
	GMAC_RXDESC_T	*fq_desc;
	unsigned long	flags;

	spin_lock_irqsave(&toe->freeq_lock, flags);
	fq_rwptr.bits32 = __raw_readl(toe->global_base + GLOBAL_SWFQ_RWPTR_REG);
	while ((unsigned short)RWPTR_ADVANCE_ONE(fq_rwptr.bits.wptr,
		TOE_SW_FREEQ_DESC_NUM) != fq_rwptr.bits.rptr) {
		skb = dev_alloc_skb(SW_RX_BUF_SIZE);
		if (skb == NULL) {
			dev_err(toe->dev, "%s::skb allocation fail\n", __func__);
			break;
		}
		REG32(skb->data) = (unsigned int)skb;
		skb_reserve(skb, SKB_RESERVE_BYTES);
		fq_rwptr.bits.wptr = RWPTR_ADVANCE_ONE(fq_rwptr.bits.wptr,
			TOE_SW_FREEQ_DESC_NUM);
		fq_desc = (GMAC_RXDESC_T *)toe->swfq_desc_base + fq_rwptr.bits.wptr;
		fq_desc->word2.buf_adr = dma_map_single(toe->dev, skb->data,
					SW_RX_BUF_SIZE - SKB_RESERVE_BYTES,
					DMA_FROM_DEVICE);
		dma_sync_single_for_device(toe->dev,
				toe->sw_freeq_desc_base_dma+
				fq_rwptr.bits.wptr * sizeof(GMAC_RXDESC_T),
				sizeof(GMAC_RXDESC_T),
				DMA_TO_DEVICE);
		SET_WPTR(toe->global_base + GLOBAL_SWFQ_RWPTR_REG, fq_rwptr.bits.wptr);
	}
	spin_unlock_irqrestore(&toe->freeq_lock, flags);
}

static void fill_free_q_worker(struct work_struct *work)
{
	struct toe_private *toe = container_of(work, struct toe_private, freq_work);

	toe_gmac_fill_free_q(toe);
}

/*----------------------------------------------------------------------
*	toe_gmac_handle_default_rxq
*	(1) Get rx Buffer for default Rx queue
*	(2) notify or call upper-routine to handle it
*	(3) get a new buffer and insert it into SW free queue
*	(4) Note: The SW free queue Read-Write Pointer should be locked when accessing
*----------------------------------------------------------------------*/
static void toe_gmac_handle_default_rxq(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	GMAC_RXDESC_T	*curr_desc;
	struct sk_buff	*skb;
	DMA_RWPTR_T	rwptr;
	unsigned int	pkt_size;
	int		max_cnt;
	unsigned int	desc_count;
	unsigned int	chksum_status, rx_status;
	struct net_device_stats *isPtr = &dev->stats;

	rwptr.bits32 = __raw_readl(&gmac->default_qhdr->word1);
	max_cnt = DEFAULT_RXQ_MAX_CNT;
	while ((--max_cnt) && rwptr.bits.rptr != rwptr.bits.wptr) {
		curr_desc = (GMAC_RXDESC_T *)gmac->default_desc_base + rwptr.bits.rptr;
		dma_sync_single_for_device(toe->dev,
				gmac->default_desc_base_dma+
				rwptr.bits.rptr * sizeof(GMAC_RXDESC_T),
				sizeof(GMAC_RXDESC_T),
				DMA_FROM_DEVICE);
		rx_status = curr_desc->word0.bits.status;
		chksum_status = curr_desc->word0.bits.chksum_status;
		pkt_size = curr_desc->word1.bits.byte_count;	/* total byte count in a frame */
		desc_count = curr_desc->word0.bits.desc_count;	/* get descriptor count per frame */
		skb = (struct sk_buff *)(REG32(__va(curr_desc->word2.buf_adr) - SKB_RESERVE_BYTES));

		if ((curr_desc->word0.bits32 & (GMAC_RXDESC_0_T_derr | GMAC_RXDESC_0_T_perr))
			|| (pkt_size < 60) || (chksum_status & 0x4) || rx_status) {
			if (curr_desc->word0.bits32 & GMAC_RXDESC_0_T_derr)
				dev_err(&dev->dev, "%s::derr\n", __func__);
			if (curr_desc->word0.bits32 & GMAC_RXDESC_0_T_perr)
				dev_err(&dev->dev, "%s::perr\n", __func__);
			if (rx_status && (rx_status == 4 || rx_status == 7))
				isPtr->rx_crc_errors++;

			dev_kfree_skb_irq(skb);
			goto bad_frame;
		}

		if (curr_desc->word0.bits.drop)
			dev_warn(&dev->dev, "%s::Drop\n", __func__);

		/* get frame information from the first descriptor of the frame */
		skb_reserve(skb, RX_INSERT_BYTES);	/* 16 byte align the IP fields. */
		skb_put(skb, pkt_size);
		skb->dev = dev;
		skb->protocol = eth_type_trans(skb, dev);
		if (chksum_status == RX_CHKSUM_IP_UDP_TCP_OK || chksum_status == RX_CHKSUM_IP_OK_ONLY)
			skb->ip_summed = CHECKSUM_UNNECESSARY;

		netif_rx(skb);	/* socket rx */
		dev->last_rx = jiffies;

		isPtr->rx_bytes += pkt_size;

bad_frame:
		/* advance one for Rx default Q 0/1 */
		rwptr.bits.rptr = RWPTR_ADVANCE_ONE(rwptr.bits.rptr, TOE_DEFAULT_Q_DESC_NUM);
		SET_RPTR(&gmac->default_qhdr->word1, rwptr.bits.rptr);
	}

	schedule_work(&toe->freq_work);
}

static irqreturn_t toe_gmac_interrupt(int irq, void *dev_instance)
{
	struct net_device	*dev = dev_instance;
	struct gmac_private	*gmac = netdev_priv(dev);
	struct toe_private	*toe = gmac->toe;
	unsigned int		status0;
	unsigned int		status1;
	unsigned int		status2;
	unsigned int		status3;
	unsigned int		status4;
	int			handled = 0;

	/* read Interrupt status */
	status0 = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_STATUS_0_REG);
	status1 = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_STATUS_1_REG);
	status2 = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_STATUS_2_REG);
	status3 = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_STATUS_3_REG);
	status4 = __raw_readl(toe->global_base + GLOBAL_INTERRUPT_STATUS_4_REG);

	/* clear interrupts */
	if (status0)
		__raw_writel(status0, toe->global_base + GLOBAL_INTERRUPT_STATUS_0_REG);
	if (status1)
		__raw_writel(status1, toe->global_base + GLOBAL_INTERRUPT_STATUS_1_REG);
	if (status2)
		__raw_writel(status2, toe->global_base + GLOBAL_INTERRUPT_STATUS_2_REG);
	if (status3)
		__raw_writel(status3, toe->global_base + GLOBAL_INTERRUPT_STATUS_3_REG);
	if (status4)
		__raw_writel(status4, toe->global_base + GLOBAL_INTERRUPT_STATUS_4_REG);

	/* handle freeq interrupt first */
	if (status4 & gmac->intr4_enabled) {
		if ((status4 & SWFQ_EMPTY_INT_BIT) && (gmac->intr4_enabled & SWFQ_EMPTY_INT_BIT)) {
			toe_gmac_fill_free_q(toe);
			handled = 1;
		}
	}

	/* Interrupt Status 1 */
	if (status1 & gmac->intr1_enabled) {
		/*
		 * Handle GMAC 0/1 HW Tx queue 0-3 EOF events
		 * Only count
		 * TOE, Classification, and default queues interrupts are handled by ISR
		 * because they should pass packets to upper layer
		 */
		if (gmac->port_id == 0) {
			if (netif_running(dev) && (status1 & DEFAULT_Q0_INT_BIT) && (gmac->intr1_enabled & DEFAULT_Q0_INT_BIT)) {
				toe_gmac_handle_default_rxq(dev);
				handled = 1;
			}
		} else if (gmac->port_id == 1) {
			if (netif_running(dev) && (status1 & DEFAULT_Q1_INT_BIT) && (gmac->intr1_enabled & DEFAULT_Q1_INT_BIT)) {
				toe_gmac_handle_default_rxq(dev);
				handled = 1;
			}
		}
	}

	/* Interrupt Status 0 */
	if (status0 & gmac->intr0_enabled) {
#ifndef	GMAX_TX_INTR_DISABLED
		if (gmac->port_id == 1 && netif_running(dev) &&
			(((status0 & GMAC1_SWTQ10_FIN_INT_BIT) && (gmac->intr0_enabled & GMAC1_SWTQ10_FIN_INT_BIT))
			||
			((status0 & GMAC1_SWTQ10_EOF_INT_BIT) && (gmac->intr0_enabled & GMAC1_SWTQ10_EOF_INT_BIT)))) {
			toe_gmac_tx_complete(dev, 0);
			handled = 1;
		}

		if (gmac->port_id == 0 && netif_running(dev) &&
			(((status0 & GMAC0_SWTQ00_FIN_INT_BIT) && (gmac->intr0_enabled & GMAC0_SWTQ00_FIN_INT_BIT))
			||
			((status0 & GMAC0_SWTQ00_EOF_INT_BIT) && (gmac->intr0_enabled & GMAC0_SWTQ00_EOF_INT_BIT)))) {
			toe_gmac_tx_complete(dev, 0);
			handled = 1;
		}
#endif
	}

	return IRQ_RETVAL(handled);
}

static int gmac_open(struct net_device *dev)
{
	struct gmac_private	*gmac = netdev_priv(dev);
	int			retval;

	/* hook ISR */
	retval = request_irq(dev->irq, toe_gmac_interrupt, 0, dev->name, dev);
	if (retval)
		return retval;

	toe_init_gmac(dev);

//	netif_carrier_on(dev);
	if (gmac->phydev)
	    phy_start(gmac->phydev);

	netif_start_queue(dev);
//	enable_irq(dev->irq);
	toe_gmac_enable_tx_rx(dev);

	return 0;
}

static int gmac_close(struct net_device *dev)
{
	struct gmac_private	*gmac = netdev_priv(dev);

	netif_stop_queue(dev);
	mdelay(20);

	if (gmac->phydev)
		phy_stop(gmac->phydev);

	/* stop tx/rx packet */
	toe_gmac_disable_tx_rx(dev);
	mdelay(20);

	/* stop the chip's Tx and Rx DMA processes */
	toe_gmac_hw_stop(gmac);

	disable_irq(dev->irq);
	free_irq(dev->irq, dev);

	return 0;
}

static void gmac_get_phy_status(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	GMAC_CONFIG0_T	config0;
	GMAC_STATUS_T	status, old_status;
	struct phy_device *phydev = gmac->phydev;

	old_status.bits32 = status.bits32 = __raw_readl(dev->base_addr + GMAC_STATUS);

	status.bits.link = phydev->link;
	status.bits.duplex = phydev->duplex;

	switch (phydev->speed) {
	case 1000:
		status.bits.speed = GMAC_SPEED_1000;
		if (phydev->interface == PHY_INTERFACE_MODE_RGMII)
			status.bits.mii_rmii = GMAC_PHY_RGMII_1000;
		break;
	case 100:
		status.bits.speed = GMAC_SPEED_100;
		if (phydev->interface == PHY_INTERFACE_MODE_RGMII)
			status.bits.mii_rmii = GMAC_PHY_RGMII_100_10;
		break;
	case 10:
		status.bits.speed = GMAC_SPEED_10;
		if (phydev->interface == PHY_INTERFACE_MODE_RGMII)
			status.bits.mii_rmii = GMAC_PHY_RGMII_100_10;
		break;
	default:
		dev_warn(&dev->dev, "Not supported PHY speed (%d)\n", phydev->speed);
	}

	if (phydev->pause) {
		if (gmac->flow_control_enable == 0) {
			config0.bits32 = __raw_readl(dev->base_addr + GMAC_CONFIG0);
			config0.bits.tx_fc_en = 1;	/* enable tx flow control */
			config0.bits.rx_fc_en = 1;	/* enable rx flow control */
			__raw_writel(config0.bits32, dev->base_addr + GMAC_CONFIG0);
			dev_info(&dev->dev, "MII flow control enabled\n");
		}
		gmac->flow_control_enable = 1;
	} else {
		if (gmac->flow_control_enable == 1) {
			config0.bits32 = __raw_readl(dev->base_addr + GMAC_CONFIG0);
			config0.bits.tx_fc_en = 0;	/* disable tx flow control */
			config0.bits.rx_fc_en = 0;	/* disable rx flow control */
			__raw_writel(config0.bits32, dev->base_addr + GMAC_CONFIG0);
			dev_info(&dev->dev, "MII flow control disabled\n");
		}
		gmac->flow_control_enable = 0;
	}

	if (old_status.bits32 != status.bits32) {
		toe_gmac_disable_tx_rx(dev);
		phy_print_status(phydev);
		mdelay(10);	/* let GMAC consume packet */
		__raw_writel(status.bits32, dev->base_addr + GMAC_STATUS);
		if (status.bits.link)
			{
			printk(KERN_INFO "enable link\n");
			toe_gmac_enable_tx_rx(dev);
			}
	}
}

static void gmac_set_rx_mode(struct net_device *dev)
{
	GMAC_RX_FLTR_T	filter;
	unsigned int	mc_filter[2];	/* Multicast hash filter */
	int		bit_nr;
	unsigned int	i;

	filter.bits32 = 0;
	filter.bits.error = 0;
	if (dev->flags & IFF_PROMISC) {
		filter.bits.error = 1;
		filter.bits.promiscuous = 1;
		filter.bits.broadcast = 1;
		filter.bits.multicast = 1;
		filter.bits.unicast = 1;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else if (dev->flags & IFF_ALLMULTI) {
		filter.bits.broadcast = 1;
		filter.bits.multicast = 1;
		filter.bits.unicast = 1;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	} else {
		struct dev_mc_list *mclist;

		filter.bits.broadcast = 1;
		filter.bits.multicast = 1;
		filter.bits.unicast = 1;
		mc_filter[1] = mc_filter[0] = 0;
		for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count; i++, mclist = mclist->next) {
			bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) & 0x3f;
			if (bit_nr <= 32)
				mc_filter[0] = mc_filter[0] | (1 << bit_nr);
			else
				mc_filter[1] = mc_filter[1] | (1 << (bit_nr - 32));
		}
	}
	__raw_writel(filter.bits32, dev->base_addr + GMAC_RX_FLTR);
	__raw_writel(mc_filter[0], dev->base_addr + GMAC_MCAST_FIL0);
	__raw_writel(mc_filter[1], dev->base_addr + GMAC_MCAST_FIL1);
}

static void gmac_tx_timeout(struct net_device *dev)
{
	if (!netif_queue_stopped(dev))
		netif_wake_queue(dev);

	dev_warn(&dev->dev, "TX timeout\n");
}

static int gmac_change_mtu(struct net_device *dev, int new_mtu)
{
	struct gmac_private *gmac = netdev_priv(dev);
	int max_frame = new_mtu + 14 + 4;
	
	
	if((max_frame < 64) ||	(max_frame > 9100)) 
	{
		printk("Invalid MTU setting\n");
		return -EINVAL;
	}

	dev->mtu = new_mtu;
	if (new_mtu > 9100)
	{
		printk("MTU must <= %d \n",9100);
		return -EINVAL;
	}
	else 
	{
		printk("Change MTU = %d\n",new_mtu);
	}
			
	return 0;


}


static void __init mac_init_drv(struct toe_private *toe)
{
	QUEUE_THRESHOLD_T	threshold;
	DMA_SKB_SIZE_T		skb_size;

	/* clear non TOE Queue Header Area */
	memset(toe->global_base + TOE_NONTOE_QUE_HDR_BASE, 0,
		NONTOE_Q_HDR_AREA_END - TOE_NONTOE_QUE_HDR_BASE);

	/* clear TOE Queue Header Area */
	memset(toe->global_base + TOE_TOE_QUE_HDR_BASE, 0,
		TOE_Q_HDR_AREA_END - TOE_TOE_QUE_HDR_BASE);

	/* Write GLOBAL_QUEUE_THRESHOLD_REG */
	threshold.bits32 = 0;
	threshold.bits.swfq_empty = (TOE_SW_FREEQ_DESC_NUM > 256) ? 255 :
					TOE_SW_FREEQ_DESC_NUM / 2;
	threshold.bits.hwfq_empty = (TOE_HW_FREEQ_DESC_NUM > 256) ? 256 / 4 :
					TOE_HW_FREEQ_DESC_NUM / 4;
	threshold.bits.toe_class = (TOE_TOE_DESC_NUM > 256) ? 256 / 4 :
					TOE_TOE_DESC_NUM / 4;
	threshold.bits.intrq = (TOE_INTR_DESC_NUM > 256) ? 256 / 4 :
					TOE_INTR_DESC_NUM / 4;
	__raw_writel(threshold.bits32, toe->global_base + GLOBAL_QUEUE_THRESHOLD_REG);

	/* Init skb size */
	skb_size.bits.hw_skb_size = HW_RX_BUF_SIZE;
	skb_size.bits.sw_skb_size = SW_RX_BUF_SIZE;
	__raw_writel(skb_size.bits32, toe->global_base + GLOBAL_DMA_SKB_SIZE_REG);

	toe_init_free_queue(toe);
	toe_init_interrupt_config(toe);
}

/*const static struct net_device_ops gemini_gmac_ops = {
	.ndo_open		= gmac_open,
	.ndo_stop		= gmac_close,
	.ndo_start_xmit		= gmac_start_xmit,
	.ndo_get_stats		= gmac_get_stats,
	.ndo_set_multicast_list	= gmac_set_rx_mode,
	.ndo_set_mac_address	= gmac_set_mac_address,
	.ndo_tx_timeout		= gmac_tx_timeout,
};
*/

typedef struct
{
  unsigned char mac[6];
  unsigned char vlanid;
  unsigned char vlanmap;
} vlaninfo;

static int __init gmac_init_eth(struct platform_device *pdev, unsigned int num)
{
	struct gmac_private	*gmac;
	struct net_device	*dev;
	struct toe_private	*toe = platform_get_drvdata(pdev);
	struct gemini_gmac_platform_data *pdata = pdev->dev.platform_data;
	extern int get_vlaninfo(vlaninfo* vlan);
	static vlaninfo    vlan[2];

	if (!pdata->bus_id[num])
		return 0;

	dev = alloc_etherdev(sizeof(*gmac));
	if (dev == NULL) {
		dev_err(&pdev->dev, "Can't allocate ethernet device #%d\n", num);
		return -ENOMEM;
	}

	gmac = netdev_priv(dev);
	gmac->toe=toe;
	toe->net_dev[num] = dev;

	gmac->dma_base_addr = toe->global_base + TOE_GMAC_DMA_BASE(num);
	gmac->port_id = num;

	dev->base_addr = toe->global_base + TOE_GMAC_BASE(num);
	dev->irq = platform_get_irq(pdev, num);
	dev->open = gmac_open;
	dev->stop = gmac_close;
	dev->hard_start_xmit = gmac_start_xmit;
	dev->get_stats = gmac_get_stats;
	dev->set_multicast_list = gmac_set_rx_mode;
	dev->set_mac_address = gmac_set_mac_address;
	dev->tx_timeout = gmac_tx_timeout;
	dev->change_mtu = gmac_change_mtu;
	//dev->netdev_ops = &gemini_gmac_ops;
	dev->watchdog_timeo = GMAC_DEV_TX_TIMEOUT;
	dev->tx_queue_len = TOE_GMAC_SWTXQ_DESC_NUM;

#ifdef DO_HW_CHKSUM
	dev->features = NETIF_F_SG | NETIF_F_HW_CSUM;
#ifdef ENABLE_TSO
	dev->features |= NETIF_F_TSO;
#endif
#endif

	toe_init_swtx_queue(dev);
	toe_init_default_queue(dev);

	if (get_vlaninfo(&vlan[0]))
	{
    	    memcpy((void *)&dev->dev_addr[0],vlan[num].mac,6);
	}
	__gmac_set_mac_address(dev);

	/* TODO: Do we need this? */
	__raw_writel(0x55aa55aa, dev->base_addr + GMAC_STA_ADD2);

	if (register_netdev(dev))
		return -1;

	gmac->phydev = phy_connect(dev, pdata->bus_id[num], &gmac_get_phy_status, 0,
				 pdata->interface[num]);
	if (IS_ERR(gmac->phydev))
		return PTR_ERR(gmac->phydev);

	gmac->phydev->supported &= PHY_GBIT_FEATURES | SUPPORTED_Pause;
	gmac->phydev->advertising = gmac->phydev->supported;


	return 0;
}



static int __init gmac_probe(struct platform_device *pdev)
{
	struct resource				*res;
	struct toe_private			*toe;
	int					retval;

	if (!pdev->dev.platform_data)
		return -EINVAL;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "can't get device resources\n");
		return -ENODEV;
	}

	toe = kzalloc(sizeof(struct toe_private), GFP_KERNEL);
	if (!toe)
		return -ENOMEM;

	toe->dev = &pdev->dev;

	toe->global_base = ioremap(res->start, (res->end-res->start)+1);
	if (!toe->global_base) {
		dev_err(toe->dev, "ioremap failed\n");
		retval = -EIO;
		goto err_data;
	}

	platform_set_drvdata(pdev, toe);

	mac_init_drv(toe);

	INIT_WORK(&toe->freq_work, fill_free_q_worker);
	spin_lock_init(&toe->freeq_lock);

	retval = gmac_init_eth(pdev, GMAC_PORT0);
	if (retval)
		goto err_unmap;
	retval = gmac_init_eth(pdev, GMAC_PORT1);
	if (retval)
		goto err_unmap;

	dev_info(&pdev->dev, SL351x_DRIVER_NAME "\n");

	return 0;

err_unmap:
	iounmap(toe->global_base);
err_data:
	kfree(toe);
	return retval;
}

static int __exit gmac_remove(struct platform_device *pdev)
{
	struct toe_private *toe = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < 2; i++)
		if (toe->net_dev[i]) {
			unregister_netdev(toe->net_dev[i]);
			kfree(toe->net_dev[i]);
		}

	iounmap(toe->global_base);

	kfree(toe);

	return 0;
}

static struct platform_driver gemini_gmac_driver = {
	.probe		= gmac_probe,
	.remove		= __exit_p(gmac_remove),

	.driver		= {
		.name	= "gemini-gmac",
		.owner	= THIS_MODULE,
	},
};
static struct resource gmac_resources[] = {
	{
		.start	= 0x60000000,
		.end	= 0x6000ffff,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= IRQ_GMAC0,
		.end	= IRQ_GMAC0,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= IRQ_GMAC1,
		.end	= IRQ_GMAC1,
		.flags	= IORESOURCE_IRQ,
	},
};

static u64 gmac_dmamask = 0xffffffffUL;

static struct platform_device ethernet_device = {
	.name	= "gemini-gmac",
	.id	= 0,
	.dev	= {
		.dma_mask = &gmac_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(gmac_resources),
	.resource	= gmac_resources,
};

#define GEMINI_GLOBAL_BASE	0x40000000
#define GLOBAL_MISC_CTRL		0x30
#define GMAC_GMII			(1 << 28)
#define GMAC_1_ENABLE			(1 << 27)

static int platform_register_ethernet(struct gemini_gmac_platform_data *pdata)
{
	unsigned int reg = __raw_readl(IO_ADDRESS(GEMINI_GLOBAL_BASE) + GLOBAL_MISC_CTRL);

	reg &= ~(GMAC_GMII | GMAC_1_ENABLE);

	if (pdata->bus_id[1])
		reg |= GMAC_1_ENABLE;
	else if (pdata->interface[0] == PHY_INTERFACE_MODE_GMII)
		reg |= GMAC_GMII;		

	__raw_writel(reg, IO_ADDRESS(GEMINI_GLOBAL_BASE) + GLOBAL_MISC_CTRL);

	ethernet_device.dev.platform_data = pdata;

	return platform_device_register(&ethernet_device);
}
#ifdef CONFIG_MACH_WBD222
static struct gemini_gmac_platform_data gmac_data = {
	.bus_id[0] = "0:01",
	.interface[0] = PHY_INTERFACE_MODE_MII,
	.bus_id[1] = "0:03",
        .interface[1] = PHY_INTERFACE_MODE_MII,
};
static struct mdio_gpio_platform_data wbd_mdio = {
	.mdc		= 22,
	.mdio		= 21,
	.phy_mask	= ~((1 << 1) | (1 << 3)),
};

static struct platform_device wbd_phy_device = {
	.name	= "mdio-gpio",
	.id	= 0,
	.dev	= {
		.platform_data = &wbd_mdio,
	},
};
#else
static struct mdio_gpio_platform_data wbd_mdio = {
	.mdc		= 22,
	.mdio		= 21,
	.phy_mask	= ~(1 << 1),
};

static struct platform_device wbd_phy_device = {
	.name	= "mdio-gpio",
	.id	= 0,
	.dev	= {
		.platform_data = &wbd_mdio,
	},
};
static struct gemini_gmac_platform_data gmac_data = {
	.bus_id[0] = "0:01",
	.interface[0] = PHY_INTERFACE_MODE_MII,
};
#endif



static int __init gemini_gmac_init(void)
{
	platform_device_register(&wbd_phy_device);
	platform_register_ethernet(&gmac_data);
	return platform_driver_register(&gemini_gmac_driver);
}

static void __exit gemini_gmac_exit(void)
{
	platform_driver_unregister(&gemini_gmac_driver);
}

module_init(gemini_gmac_init);
module_exit(gemini_gmac_exit);

MODULE_AUTHOR("Paulius Zaleckas");
MODULE_DESCRIPTION("Ethernet device driver for Gemini SoC");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:gemini-gmac");


