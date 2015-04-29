/*
 * Ethernet driver for the Micrel/Kendin KS8695 (Centaur)
 *
 * (C) 2006 Andrew Victor
 *
 */

#ifndef KS8695_ETHERNET
#define KS8695_ETHERNET

/* .... Hardware Descriptors ................................................ */

struct rx_descriptor {
	unsigned long status;
	unsigned long length;
	unsigned long addr;
	unsigned long next;
};

#define RDES_OWN	(1 << 31)	/* Ownership */
#define RDES_FS		(1 << 30)	/* First Descriptor */
#define RDES_LS		(1 << 29)	/* Last Descriptor */
#define RDES_IPE	(1 << 28)	/* IP Checksum error */
#define RDES_TCPE	(1 << 27)	/* TCP Checksum error */
#define RDES_UDPE	(1 << 26)	/* UDP Checksum error */
#define RDES_ES		(1 << 25)	/* Error summary */
#define RDES_MF		(1 << 24)	/* Multicast Frame */
#define RDES_RE		(1 << 19)	/* MII Error reported */
#define RDES_TL		(1 << 18)	/* Frame too Long */
#define RDES_RF		(1 << 17)	/* Runt Frame */
#define RDES_CE		(1 << 16)	/* CRC error */
#define RDES_FT		(1 << 15)	/* Frame Type */
#define RDES_FLEN	(0x7ff)		/* Frame Length */

#define RDES_RER	(1 << 25)	/* Receive End of Ring */
#define RDES_RBS	(0x7ff)		/* Receive Buffer Size */


struct tx_descriptor {
	unsigned long ownership;
	unsigned long status;
	unsigned long addr;
	unsigned long next;
};

#define TDES_OWN	(1 << 31)	/* Ownership */

#define TDES_IC		(1 << 31)	/* Interrupt on Completion */
#define TDES_FS		(1 << 30)	/* First Segment */
#define TDES_LS		(1 << 29)	/* Last Segment */
#define TDES_IPCKG	(1 << 28)	/* IP Checksum generate */
#define TDES_TCPCKG	(1 << 27)	/* TCP Checksum generate */
#define TDES_UDPCKG	(1 << 26)	/* UDP Checksum generate */
#define TDES_TER	(1 << 25)	/* Transmit End of Ring */
#define TDES_TBS	(0x7ff)		/* Transmit Buffer Size */


/* .... ..................................................................... */

#define MAX_RX_DESC	16		/* number of receive descriptors */
#define MAX_TX_DESC	8		/* number of transmit descriptors */
#define MAX_RXBUF_SIZE	0x600		/* 1518 rounded-up */

struct ks8695_buffer
{
	struct sk_buff *skb;
	dma_addr_t dma;
	unsigned long length;
};


struct ks8695eth_priv
{
	struct device *dev;
	struct net_device_stats stats;			/* statistics */
	unsigned long irqs;				/* IRQ bitset */

	/* Transmit */
	struct tx_descriptor *txdma;			/* Tx DMA descriptors */
	dma_addr_t txdma_phys;				/* TX DMA descriptors (phys address) */
	unsigned int tx_head;				/* descriptor index (add) */
	unsigned int tx_tail;				/* descriptor index (remove) */
	spinlock_t tx_lock;
	struct ks8695_buffer txSkb[MAX_TX_DESC];	/* packets being transmitted */

	/* Receive */
	struct rx_descriptor *rxdma;			/* Rx DMA descriptors */
	dma_addr_t rxdma_phys;				/* Rx DMA descriptors (phys address) */
	unsigned int rx_idx;				/* descriptor index */
	struct ks8695_buffer rxSkb[MAX_RX_DESC];
};

#endif
