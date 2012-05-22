/*
 * ar231x.h: Linux driver for the Atheros AR231x Ethernet device.
 *
 * Copyright (C) 2004 by Sameer Dekate <sdekate@arubanetworks.com>
 * Copyright (C) 2006 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006-2009 Felix Fietkau <nbd@openwrt.org>
 *
 * Thanks to Atheros for providing hardware and documentation
 * enabling me to write this driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _AR2313_H_
#define _AR2313_H_

#include <generated/autoconf.h>
#include <linux/bitops.h>
#include <asm/bootinfo.h>
#include <ar231x_platform.h>

/*
 * probe link timer - 5 secs
 */
#define LINK_TIMER    (5*HZ)

#define IS_DMA_TX_INT(X)   (((X) & (DMA_STATUS_TI)) != 0)
#define IS_DMA_RX_INT(X)   (((X) & (DMA_STATUS_RI)) != 0)
#define IS_DRIVER_OWNED(X) (((X) & (DMA_TX_OWN))    == 0)

#define AR2313_TX_TIMEOUT (HZ/4)

/*
 * Rings
 */
#define DSC_RING_ENTRIES_SIZE	(AR2313_DESCR_ENTRIES * sizeof(struct desc))
#define DSC_NEXT(idx)	        ((idx + 1) & (AR2313_DESCR_ENTRIES - 1))

#define AR2313_MBGET		2
#define AR2313_MBSET		3
#define AR2313_PCI_RECONFIG	4
#define AR2313_PCI_DUMP		5
#define AR2313_TEST_PANIC	6
#define AR2313_TEST_NULLPTR	7
#define AR2313_READ_DATA	8
#define AR2313_WRITE_DATA	9
#define AR2313_GET_VERSION	10
#define AR2313_TEST_HANG	11
#define AR2313_SYNC		12

#define DMA_RX_ERR_CRC		BIT(1)
#define DMA_RX_ERR_DRIB		BIT(2)
#define DMA_RX_ERR_MII		BIT(3)
#define DMA_RX_EV2		BIT(5)
#define DMA_RX_ERR_COL		BIT(6)
#define DMA_RX_LONG		BIT(7)
#define DMA_RX_LS		BIT(8)	/* last descriptor */
#define DMA_RX_FS		BIT(9)	/* first descriptor */
#define DMA_RX_MF		BIT(10)	/* multicast frame */
#define DMA_RX_ERR_RUNT		BIT(11)	/* runt frame */
#define DMA_RX_ERR_LENGTH	BIT(12)	/* length error */
#define DMA_RX_ERR_DESC		BIT(14)	/* descriptor error */
#define DMA_RX_ERROR		BIT(15)	/* error summary */
#define DMA_RX_LEN_MASK		0x3fff0000
#define DMA_RX_LEN_SHIFT	16
#define DMA_RX_FILT		BIT(30)
#define DMA_RX_OWN		BIT(31)	/* desc owned by DMA controller */

#define DMA_RX1_BSIZE_MASK	0x000007ff
#define DMA_RX1_BSIZE_SHIFT	0
#define DMA_RX1_CHAINED		BIT(24)
#define DMA_RX1_RER		BIT(25)

#define DMA_TX_ERR_UNDER	BIT(1)	/* underflow error */
#define DMA_TX_ERR_DEFER	BIT(2)	/* excessive deferral */
#define DMA_TX_COL_MASK		0x78
#define DMA_TX_COL_SHIFT	3
#define DMA_TX_ERR_HB		BIT(7)	/* hearbeat failure */
#define DMA_TX_ERR_COL		BIT(8)	/* excessive collisions */
#define DMA_TX_ERR_LATE		BIT(9)	/* late collision */
#define DMA_TX_ERR_LINK		BIT(10)	/* no carrier */
#define DMA_TX_ERR_LOSS		BIT(11)	/* loss of carrier */
#define DMA_TX_ERR_JABBER	BIT(14)	/* transmit jabber timeout */
#define DMA_TX_ERROR		BIT(15)	/* frame aborted */
#define DMA_TX_OWN		BIT(31)	/* descr owned by DMA controller */

#define DMA_TX1_BSIZE_MASK	0x000007ff
#define DMA_TX1_BSIZE_SHIFT	0
#define DMA_TX1_CHAINED		BIT(24)	/* chained descriptors */
#define DMA_TX1_TER		BIT(25)	/* transmit end of ring */
#define DMA_TX1_FS		BIT(29)	/* first segment */
#define DMA_TX1_LS		BIT(30)	/* last segment */
#define DMA_TX1_IC		BIT(31)	/* interrupt on completion */

#define RCVPKT_LENGTH(X)	(X  >> 16)	/* Received pkt Length */

#define MAC_CONTROL_RE		BIT(2)	/* receive enable */
#define MAC_CONTROL_TE		BIT(3)	/* transmit enable */
#define MAC_CONTROL_DC		BIT(5)	/* Deferral check */
#define MAC_CONTROL_ASTP	BIT(8)	/* Auto pad strip */
#define MAC_CONTROL_DRTY	BIT(10)	/* Disable retry */
#define MAC_CONTROL_DBF		BIT(11)	/* Disable bcast frames */
#define MAC_CONTROL_LCC		BIT(12)	/* late collision ctrl */
#define MAC_CONTROL_HP		BIT(13)	/* Hash Perfect filtering */
#define MAC_CONTROL_HASH	BIT(14)	/* Unicast hash filtering */
#define MAC_CONTROL_HO		BIT(15)	/* Hash only filtering */
#define MAC_CONTROL_PB		BIT(16)	/* Pass Bad frames */
#define MAC_CONTROL_IF		BIT(17)	/* Inverse filtering */
#define MAC_CONTROL_PR		BIT(18)	/* promiscuous mode (valid frames only) */
#define MAC_CONTROL_PM		BIT(19)	/* pass multicast */
#define MAC_CONTROL_F		BIT(20)	/* full-duplex */
#define MAC_CONTROL_DRO		BIT(23)	/* Disable Receive Own */
#define MAC_CONTROL_HBD		BIT(28)	/* heart-beat disabled (MUST BE SET) */
#define MAC_CONTROL_BLE		BIT(30)	/* big endian mode */
#define MAC_CONTROL_RA		BIT(31)	/* receive all (valid and invalid frames) */

#define MII_ADDR_BUSY		BIT(0)
#define MII_ADDR_WRITE		BIT(1)
#define MII_ADDR_REG_SHIFT	6
#define MII_ADDR_PHY_SHIFT	11
#define MII_DATA_SHIFT		0

#define FLOW_CONTROL_FCE	BIT(1)

#define DMA_BUS_MODE_SWR	BIT(0)	/* software reset */
#define DMA_BUS_MODE_BLE	BIT(7)	/* big endian mode */
#define DMA_BUS_MODE_PBL_SHIFT	8	/* programmable burst length 32 */
#define DMA_BUS_MODE_DBO	BIT(20)	/* big-endian descriptors */

#define DMA_STATUS_TI		BIT(0)	/* transmit interrupt */
#define DMA_STATUS_TPS		BIT(1)	/* transmit process stopped */
#define DMA_STATUS_TU		BIT(2)	/* transmit buffer unavailable */
#define DMA_STATUS_TJT		BIT(3)	/* transmit buffer timeout */
#define DMA_STATUS_UNF		BIT(5)	/* transmit underflow */
#define DMA_STATUS_RI		BIT(6)	/* receive interrupt */
#define DMA_STATUS_RU		BIT(7)	/* receive buffer unavailable */
#define DMA_STATUS_RPS		BIT(8)	/* receive process stopped */
#define DMA_STATUS_ETI		BIT(10)	/* early transmit interrupt */
#define DMA_STATUS_FBE		BIT(13)	/* fatal bus interrupt */
#define DMA_STATUS_ERI		BIT(14)	/* early receive interrupt */
#define DMA_STATUS_AIS		BIT(15)	/* abnormal interrupt summary */
#define DMA_STATUS_NIS		BIT(16)	/* normal interrupt summary */
#define DMA_STATUS_RS_SHIFT	17	/* receive process state */
#define DMA_STATUS_TS_SHIFT	20	/* transmit process state */
#define DMA_STATUS_EB_SHIFT	23	/* error bits */

#define DMA_CONTROL_SR		BIT(1)	/* start receive */
#define DMA_CONTROL_ST		BIT(13)	/* start transmit */
#define DMA_CONTROL_SF		BIT(21)	/* store and forward */


typedef struct {
	volatile unsigned int status;	// OWN, Device control and status.
	volatile unsigned int devcs;	// pkt Control bits + Length
	volatile unsigned int addr;	// Current Address.
	volatile unsigned int descr;	// Next descriptor in chain.
} ar231x_descr_t;



//
// New Combo structure for Both Eth0 AND eth1
//
typedef struct {
	volatile unsigned int mac_control;	/* 0x00 */
	volatile unsigned int mac_addr[2];	/* 0x04 - 0x08 */
	volatile unsigned int mcast_table[2];	/* 0x0c - 0x10 */
	volatile unsigned int mii_addr;	/* 0x14 */
	volatile unsigned int mii_data;	/* 0x18 */
	volatile unsigned int flow_control;	/* 0x1c */
	volatile unsigned int vlan_tag;	/* 0x20 */
	volatile unsigned int pad[7];	/* 0x24 - 0x3c */
	volatile unsigned int ucast_table[8];	/* 0x40-0x5c */

} ETHERNET_STRUCT;

/********************************************************************
 * Interrupt controller
 ********************************************************************/

typedef struct {
	volatile unsigned int wdog_control;	/* 0x08 */
	volatile unsigned int wdog_timer;	/* 0x0c */
	volatile unsigned int misc_status;	/* 0x10 */
	volatile unsigned int misc_mask;	/* 0x14 */
	volatile unsigned int global_status;	/* 0x18 */
	volatile unsigned int reserved;	/* 0x1c */
	volatile unsigned int reset_control;	/* 0x20 */
} INTERRUPT;

/********************************************************************
 * DMA controller
 ********************************************************************/
typedef struct {
	volatile unsigned int bus_mode;	/* 0x00 (CSR0) */
	volatile unsigned int xmt_poll;	/* 0x04 (CSR1) */
	volatile unsigned int rcv_poll;	/* 0x08 (CSR2) */
	volatile unsigned int rcv_base;	/* 0x0c (CSR3) */
	volatile unsigned int xmt_base;	/* 0x10 (CSR4) */
	volatile unsigned int status;	/* 0x14 (CSR5) */
	volatile unsigned int control;	/* 0x18 (CSR6) */
	volatile unsigned int intr_ena;	/* 0x1c (CSR7) */
	volatile unsigned int rcv_missed;	/* 0x20 (CSR8) */
	volatile unsigned int reserved[11];	/* 0x24-0x4c (CSR9-19) */
	volatile unsigned int cur_tx_buf_addr;	/* 0x50 (CSR20) */
	volatile unsigned int cur_rx_buf_addr;	/* 0x50 (CSR21) */
} DMA;

/*
 * Struct private for the Sibyte.
 *
 * Elements are grouped so variables used by the tx handling goes
 * together, and will go into the same cache lines etc. in order to
 * avoid cache line contention between the rx and tx handling on SMP.
 *
 * Frequently accessed variables are put at the beginning of the
 * struct to help the compiler generate better/shorter code.
 */
struct ar231x_private {
	struct net_device *dev;
	int (*rx)(struct sk_buff *skb);

	int version;
	u32 mb[2];

	volatile ETHERNET_STRUCT *phy_regs;
	volatile ETHERNET_STRUCT *eth_regs;
	volatile DMA *dma_regs;
	volatile u32 *int_regs;
	struct ar231x_eth *cfg;

	spinlock_t lock;			/* Serialise access to device */

	/*
	 * RX and TX descriptors, must be adjacent
	 */
	ar231x_descr_t *rx_ring;
	ar231x_descr_t *tx_ring;


	struct sk_buff **rx_skb;
	struct sk_buff **tx_skb;

	/*
	 * RX elements
	 */
	u32 rx_skbprd;
	u32 cur_rx;

	/*
	 * TX elements
	 */
	u32 tx_prd;
	u32 tx_csm;

	/*
	 * Misc elements
	 */
	char name[48];
	struct {
		u32 address;
		u32 length;
		char *mapping;
	} desc;


	struct timer_list link_timer;
	unsigned short phy;			/* merlot phy = 1, samsung phy = 0x1f */
	unsigned short mac;
	unsigned short link;		/* 0 - link down, 1 - link up */
	u16 phyData;

	struct tasklet_struct rx_tasklet;
	int unloading;

	struct phy_device *phy_dev;
	struct mii_bus *mii_bus;
	int oldduplex;
};


/*
 * Prototypes
 */
static int ar231x_init(struct net_device *dev);
#ifdef TX_TIMEOUT
static void ar231x_tx_timeout(struct net_device *dev);
#endif
static int ar231x_restart(struct net_device *dev);
static void ar231x_load_rx_ring(struct net_device *dev, int bufs);
static irqreturn_t ar231x_interrupt(int irq, void *dev_id);
static int ar231x_open(struct net_device *dev);
static int ar231x_start_xmit(struct sk_buff *skb, struct net_device *dev);
static int ar231x_close(struct net_device *dev);
static int ar231x_ioctl(struct net_device *dev, struct ifreq *ifr,
						int cmd);
static void ar231x_init_cleanup(struct net_device *dev);
static int ar231x_setup_timer(struct net_device *dev);
static void ar231x_link_timer_fn(unsigned long data);
static void ar231x_check_link(struct net_device *dev);
#endif							/* _AR2313_H_ */
