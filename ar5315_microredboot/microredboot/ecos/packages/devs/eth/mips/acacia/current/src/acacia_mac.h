#ifndef _ACACIA_MAC_H_
#define _ACACIA_MAC_H_

#include "ethacacia.h"

#define MAC_FIFO_TX_THRESHOLD 48

#define virt_to_bus(vaddr) CYGARC_PHYSICAL_ADDRESS(vaddr)
#define PHYS_TO_K1(physaddr) CYGARC_UNCACHED_ADDRESS(physaddr)
#define KSEG1ADDR(vaddr) PHYS_TO_K1(virt_to_bus(vaddr))

extern char DEFAULT_MAC_ADDRESS[];

#define ETH_CRC_LEN       4

typedef unsigned char * VIRT_ADDR;

typedef struct phy_info {
	int     phyAddr;            /* PHY device address: 0-32 */
        int     phySpeed;           /* PHY speed: PHY_10MBS PHY_100MBS */
	int     phyDpx;             /* PHY duplex: PHY_HALF_DPX PHY_FULL_DPX */
} PHY_INFO;




/*****************************************************************
 * Descriptor queue
 */
typedef struct acacia_queue {
    VIRT_ADDR   firstDescAddr;  /* descriptor array address */
    VIRT_ADDR   lastDescAddr;   /* last descriptor address */
    VIRT_ADDR   curDescAddr;    /* current descriptor address */
    VIRT_ADDR   reapDescAddr;   /* current tail of tx descriptors reaped */
    U16      count;          /* number of elements */
} ACACIA_QUEUE;

/* Given a descriptor, return the next one in a circular list */
#define ACACIA_QUEUE_ELE_NEXT_GET(q, descAddr)                          \
        ((descAddr) == (q)->lastDescAddr) ? (q)->firstDescAddr :    \
        (VIRT_ADDR)((U32)(descAddr) + ACACIA_QUEUE_ELE_SIZE)

/* Move the "current descriptor" forward to the next one */
#define ACACIA_CONSUME_DESC(q)    \
         q->curDescAddr = ACACIA_QUEUE_ELE_NEXT_GET(q, q->curDescAddr)

/*****************************************************************
 * Per-ethernet-MAC OS-independent information
 */
typedef struct acacia_MAC_s {
	U32          unit;          /* MAC unit ID */
	ETH_t	    eth_regs;
	DMA_Chan_t  rx_dma_regs;
	DMA_Chan_t  tx_dma_regs;
    	ACACIA_QUEUE    txQueue;       /* Transmit descriptor queue */
    	ACACIA_QUEUE    rxQueue;       /* Receive descriptor queue */
    	U16          txDescCount;      /* Transmit descriptor count */
    	U16          rxDescCount;      /* Receive descriptor count */
    	char            *pTxDescs;     /* Transmit descriptors */
    	char            *pRxDescs;     /* Receive descriptors */
    	bool            aeProcessRst;  /* flag to indicate reset in progress */
    	bool            port_is_up;    /* flag to indicate port is up */
    	void            *OSinfo;       /* OS-dependent per-MAC information */
	PHY_INFO         board;        /* physical interface info */
	unsigned char   enetAddr[ETHER_ADDR_LEN];   /* MAC address */
} acacia_MAC_t;

#define ACACIA_TX_DESC_COUNT_DEFAULT    32     /* Transmit descriptors */
#define ACACIA_RX_DESC_COUNT_DEFAULT    32     /* Receive descriptors */

extern void acacia_mac_start_scan(int unit);
extern int acacia_mac_reset(acacia_MAC_t *MACInfo);
extern int acacia_mac_init(acacia_MAC_t *MACInfo);
extern int acacia_mac_AllocateQueues(acacia_MAC_t *MACInfo);
void *acacia_rxbuf_alloc(acacia_MAC_t *MACInfo, char **rxBptr, int *rxBSize);
void print_dma_rx_status(acacia_MAC_t *MACInfo); 
#endif // _ACACIA_MAC_H_
