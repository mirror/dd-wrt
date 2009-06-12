#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <string.h>

#include "acacia_ecos.h"
#include "acacia_mac.h"
#include "acaciareg.h"
#include "phy.h"
#include "eth.h"

#define IDT_BUS_FREQ 100
#define MII_CLOCK 1250000 /* no more than 2.5MHz */

#define STATION_ADDRESS_HIGH(mac) ((mac[0] << 8) |	\
			           (mac[1]))
#define STATION_ADDRESS_LOW(mac)  ((mac[2] << 24) |	\
				   (mac[3] << 16) |	\
				   (mac[4] << 8)  |	\
				   (mac[5]))

static inline void acacia_halt_tx(acacia_MAC_t *MACInfo )
{
	if (rc32438_halt_dma(MACInfo->tx_dma_regs))
		ACACIA_PRINT(ACACIA_DEBUG_ERROR,(": timeout!\n"));
}

static inline void acacia_halt_rx(acacia_MAC_t *MACInfo )
{
	if (rc32438_halt_dma(MACInfo->rx_dma_regs))
		ACACIA_PRINT(ACACIA_DEBUG_ERROR,(": timeout!\n"));
}

static inline void acacia_start_tx(acacia_MAC_t *MACInfo, volatile U32 td)
{
	rc32438_start_dma(MACInfo->tx_dma_regs, (U32) td);
}

static inline void acacia_start_rx(acacia_MAC_t *MACInfo,  volatile U32 rd)
{
	rc32438_start_dma(MACInfo->rx_dma_regs, (U32) rd);
}

static inline void acacia_chain_tx(acacia_MAC_t *MACInfo, volatile U32 td)
{
	rc32438_chain_dma(MACInfo->tx_dma_regs, (U32) td);
}
static inline void acacia_chain_rx(acacia_MAC_t *MACInfo, volatile U32 rd)
{
	rc32438_chain_dma(MACInfo->rx_dma_regs, (U32) rd);
}


/*
 * Start MII scan for any PHY links
 */
void acacia_mac_start_scan(int unit)
{
	int phyAddr;

	if( unit == 0 ) {
		phyAddr = PHY0_ADDR;
	}
	else {
		phyAddr = PHY1_ADDR;
	}
	phyAddr = (phyAddr << 8) & 0x1F00;
	
	/* Put the MII in Scan mode to continuously read the PHY reg */
	
	/* Select the Register you want to read in scan mode */
	writel ( (phyAddr | (PHY_STATUS1_REG & 0x1F)), IDT_MIIMADDR_REG );
	/* Start the scan */	
	writel (MIIMCMD_SCN, IDT_MIIMCMD_REG ); //start scan
}

/*
 * Reset MAC
 */
int acacia_mac_reset(acacia_MAC_t *MACInfo)
{
        volatile unsigned int i;


        /* Reset eth0 */

	/* Get registers base addresses */
        MACInfo->eth_regs = (ETH_t) KSEG1ADDR(ETH0_VirtualAddress);
        MACInfo->rx_dma_regs = 
               (DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress+ 2*DMA_CHAN_OFFSET));
        MACInfo->tx_dma_regs =
               (DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress +3*DMA_CHAN_OFFSET));
                
        /* Disable DMA */
        acacia_halt_tx(MACInfo);
        acacia_halt_rx(MACInfo);

        /*  Ethernet MAC Initialization
        *  Disable/Reset the Ethernet Interface
        */
        writel(0, &MACInfo->eth_regs->ethintfc);

        for(i = 0xfffff; i>0 ;i--) {
                if(!(readl(&MACInfo->eth_regs->ethintfc)&ETHINTFC_rip_m))
                        break;
        }
        if( i == 0 ) {
                diag_printf("ethernet logic reset - Failed. \n");
                return (ERROR);
        }

        /* Enable Ethernet Interface */
        writel(ETHINTFC_en_m, &MACInfo->eth_regs->ethintfc);

        /* Management Clock Prescaler Divisor */
        /* cgg - changed from clock independent setting:
           writel(40, &MACInfo->eth_regs->ethmcp); */
        writel(((IDT_BUS_FREQ * 1000 * 1000)/MII_CLOCK+1) & ~1,
                &MACInfo->eth_regs->ethmcp);

        /* Reset eth1 */

	MACInfo->eth_regs = (ETH_t) KSEG1ADDR(ETH1_VirtualAddress);
        MACInfo->rx_dma_regs =
                (DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress+4*DMA_CHAN_OFFSET));
        MACInfo->tx_dma_regs =
                 (DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress+5*DMA_CHAN_OFFSET));


        /* Disable DMA */
        acacia_halt_tx(MACInfo);
        acacia_halt_rx(MACInfo);

        /*  Ethernet MAC Initialization
        *  Disable/Reset the Ethernet Interface
        */
        writel(0, &MACInfo->eth_regs->ethintfc);

        for(i = 0xfffff; i>0 ;i--) {
                if(!(readl(&MACInfo->eth_regs->ethintfc)&ETHINTFC_rip_m))
                        break;
        }
        if( i == 0 ) {
                diag_printf("ethernet logic reset - Failed. \n");
                return (ERROR);
        }

        /* Enable Ethernet Interface */
        writel(ETHINTFC_en_m, &MACInfo->eth_regs->ethintfc);

        /* Management Clock Prescaler Divisor */
        writel(((IDT_BUS_FREQ * 1000 * 1000)/MII_CLOCK+1) & ~1,
                &MACInfo->eth_regs->ethmcp);


        /* Reset MII */
        writel ( MIICFG_RESET_VAL, IDT_MIIMCFG_REG );
        sysMsDelay(100);
        /* Take it out of RESET */
        writel ( 0, IDT_MIIMCFG_REG ); /* Clear Reset */
        /* Wait for the PHY link to come up */
        sysMsDelay(100);
	
	idt32438PhyInit();
	
	return OK;	
}

/*
 * Initialize the ACACIA ethernet controller.
 */
int acacia_mac_init(acacia_MAC_t *MACInfo)
{
	volatile U32 regVal;

        regVal = (readl (IDT_MIIMRDD_REG) & 0xffff);
        if ((regVal & PHY_LINK_STATUS) != PHY_LINK_STATUS) {
        	return (ERROR);
        }
        regVal = (readl (IDT_MIIMADDR_REG) & 0xffff) >> 8;

	MACInfo->unit = regVal -1;

	if( idt32438PhyAutoNegotiationComplete(MACInfo) != OK) {
		return (ERROR);
	}

        if (MACInfo->unit == 0) {  //eth0
                MACInfo->eth_regs = (ETH_t) KSEG1ADDR(ETH0_VirtualAddress);
                MACInfo->rx_dma_regs =
                (DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress+ 2*DMA_CHAN_OFFSET));                MACInfo->tx_dma_regs =
                (DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress +3*DMA_CHAN_OFFSET));        }
        else {  //eth1
                MACInfo->eth_regs = (ETH_t) KSEG1ADDR(ETH1_VirtualAddress);
                MACInfo->rx_dma_regs =
                (DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress+4*DMA_CHAN_OFFSET));
                MACInfo->tx_dma_regs =
                (DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress+5*DMA_CHAN_OFFSET));
        }

	/* Set Filtered Multicast, Multicast & Broadcast flags */
	regVal = ( ETHARC_afm_m | ETHARC_am_m | ETHARC_ab_m );
	writel(regVal, &MACInfo->eth_regs->etharc); 
	
	/* Set all Ether station address registers to their initial values */ 
	writel(STATION_ADDRESS_LOW(MACInfo->enetAddr), 
	       &MACInfo->eth_regs->ethsal0); 
	writel(STATION_ADDRESS_HIGH(MACInfo->enetAddr), 
	       &MACInfo->eth_regs->ethsah0);
	
	writel(STATION_ADDRESS_LOW(MACInfo->enetAddr), 
	       &MACInfo->eth_regs->ethsal1); 
	writel(STATION_ADDRESS_HIGH(MACInfo->enetAddr), 
	       &MACInfo->eth_regs->ethsah1);
	
	writel(STATION_ADDRESS_LOW(MACInfo->enetAddr), 
	       &MACInfo->eth_regs->ethsal2); 
	writel(STATION_ADDRESS_HIGH(MACInfo->enetAddr), 
	       &MACInfo->eth_regs->ethsah2);
	
	writel(STATION_ADDRESS_LOW(MACInfo->enetAddr), 
	       &MACInfo->eth_regs->ethsal3); 
	writel(STATION_ADDRESS_HIGH(MACInfo->enetAddr), 
	       &MACInfo->eth_regs->ethsah3); 

	/* Clear the HASH Register : Imperfect Multicast filtering bucket!! */
	writel(0, &MACInfo->eth_regs->ethhash0);
	writel(0, &MACInfo->eth_regs->ethhash1);

	/* Set default value in some of the registers */
	writel(IDT_ETHCLRT_VAL, &MACInfo->eth_regs->ethclrt);
	writel(IDT_ETHMAXF_VAL, &MACInfo->eth_regs->ethmaxf);

	/* Pad Enable, CRC Enable */ 
	regVal = (ETHMAC2_pe_m | ETHMAC2_cen_m);
	writel(regVal, &MACInfo->eth_regs->ethmac2);

	/* clear the DPTR and NDPTR registers */
	writel(0, &MACInfo->rx_dma_regs->dmadptr);
	writel(0, &MACInfo->tx_dma_regs->dmadptr);
	writel(0, &MACInfo->rx_dma_regs->dmandptr);
	writel(0, &MACInfo->tx_dma_regs->dmandptr);

	/* Clear the receive & transmit  DMA 2/3 status register */
	writel(0, &MACInfo->rx_dma_regs->dmas);
	writel(0, &MACInfo->tx_dma_regs->dmas);

	/* Clear the receive & transmit  DMA 2/3 Control register */
	writel(0, &MACInfo->rx_dma_regs->dmac);
	writel(0, &MACInfo->tx_dma_regs->dmac);

	/* Write the default DMA 2/3 Status Mask */
	/* Disable Finish, Done, Halt and Error interrupts in DMA */
	writel(DMA_SMASK_VAL, &MACInfo->rx_dma_regs->dmasm);
	writel(DMA_SMASK_VAL, &MACInfo->tx_dma_regs->dmasm);

	/* Tx DMA Threshold Value */ 
	/* don't transmit until fifo contains 48 bytes */
	writel(MAC_FIFO_TX_THRESHOLD, &MACInfo->eth_regs->ethfifott);
	
	/* setup a RDR for the DMA to handle ethernet receive */
	//acacia_start_rx(MACInfo, virt_to_bus(MACInfo->pRxDescs));
	VIRT_ADDR rxDesc = MACInfo->rxQueue.curDescAddr;
	writel(virt_to_bus(rxDesc), &MACInfo->rx_dma_regs->dmadptr); 

    	/* initialize MAC configuration parameters */
	regVal = readl(&MACInfo->eth_regs->ethmac1) | ETHMAC1_re_m;
	writel(regVal, &MACInfo->eth_regs->ethmac1);

	regVal = readl(&MACInfo->eth_regs->ethmac2) & (~IDT_ETHMAC2_FD);
	regVal |= (MACInfo->board.phyDpx == PHY_FULL_DPX ? IDT_ETHMAC2_FD : 0);
	writel(regVal, &MACInfo->eth_regs->ethmac2);

	/* EtherIPGT: b2b ipg full duplex, IPGR: b2b ipg=IPGR2 0x12 */
	regVal = (MACInfo->board.phyDpx == \
			PHY_FULL_DPX ? IDT_ETHIPGT_FDX : IDT_ETHIPGT_HDX);
	writel(regVal, &MACInfo->eth_regs->ethipgt); 
	
	regVal = (MACInfo->board.phySpeed == \
                 PHY_10MBS ? IDT_ETHIPGR_10MBS : IDT_ETHIPGR_100MBS);
	writel(regVal, &MACInfo->eth_regs->ethipgr); 

	MACInfo->port_is_up = true;
	
	return OK;
}


/******************************************************************************
* acacia_mac_QueueDestroy -- Free all buffers and descriptors associated
* with a queue.
*/
static void
acacia_mac_QueueDestroy(ACACIA_QUEUE *q)
{
    int i;
    int count;
    VIRT_ADDR    descAddr;

    ARRIVE();

    count = q->count;

    for (i=0, descAddr=q->firstDescAddr;
         i<count;
         i++, descAddr=(VIRT_ADDR)((U32)descAddr + ACACIA_QUEUE_ELE_SIZE)) {

        ACACIA_DESC_STATUS_SET(descAddr, 0);
        ACACIA_DESC_CTRLEN_SET(descAddr, 0);
        ACACIA_DESC_BUFPTR_SET(descAddr, (U32)0);
        ACACIA_DESC_LNKBUF_SET(descAddr, (U32)0);

#if 0 /* TBDXXX */
        acacia_rxbuf_free(descAddr); /* Free OS-specific software pointer */
#endif
        ACACIA_DESC_SWPTR_SET(descAddr, NULL);
    }

    LEAVE();
}

static void
acacia_mac_TxQueueDestroy(acacia_MAC_t *MACInfo)
{
    acacia_mac_QueueDestroy(&MACInfo->txQueue);
}

static void
acacia_mac_RxQueueDestroy(acacia_MAC_t *MACInfo)
{
    acacia_mac_QueueDestroy(&MACInfo->rxQueue);
}
/******************************************************************************
* Initialize generic queue data
*/
void
acacia_mac_QueueInit(ACACIA_QUEUE *q, char *pMem, int count)
{
        ARRIVE();
        q->firstDescAddr = pMem;
        q->lastDescAddr = (VIRT_ADDR)((U32)q->firstDescAddr +
                                      (count - 1) * ACACIA_QUEUE_ELE_SIZE);
        q->curDescAddr = q->firstDescAddr;
        q->count = count;
        LEAVE();
}


/******************************************************************************
* acacia_mac_TxQueueCreate - create a circular queue of descriptors for Transmit
*/
static int
acacia_mac_TxQueueCreate(acacia_MAC_t *MACInfo,
                      ACACIA_QUEUE *q,
                      char *pMem,
                      int count)
{
        int         i;
        VIRT_ADDR   descAddr;

        ARRIVE();

        acacia_mac_QueueInit(q, pMem, count);
        q->reapDescAddr = q->lastDescAddr;

        /* Initialize Tx buffer descriptors.  */
        for (i=0, descAddr=q->firstDescAddr;
             i<count;
             i++, descAddr=(VIRT_ADDR)((U32)descAddr + ACACIA_QUEUE_ELE_SIZE))
        {
            /* Initialize the size, BUFPTR, and SWPTR fields */
            ACACIA_DESC_CTRLEN_SET(descAddr, ( DMAD_iof_m | DMAD_iod_m));
            ACACIA_DESC_DEVCS_SET(descAddr, 0);
            ACACIA_DESC_BUFPTR_SET(descAddr, (U32)0);
            ACACIA_DESC_LNKBUF_SET(descAddr, (U32)0);
            ACACIA_DESC_SWPTR_SET(descAddr, (void *)0);
        } /* for each desc */


        ACACIA_PRINT(ACACIA_DEBUG_RESET,
                ("eth%d TxDesc begin = %x, end = %x\n",
                MACInfo->unit,
                (U32)q->firstDescAddr,
                (U32)q->lastDescAddr));

        LEAVE();
        return 0;
}


/******************************************************************************
* acacia_mac_RxQueueCreate - create a circular queue of Rx descriptors
*/
int
acacia_mac_RxQueueCreate(acacia_MAC_t *MACInfo,
                      ACACIA_QUEUE *q,
                      char *pMem,
                      int count)
{
        volatile U32               i, regVal;
        volatile VIRT_ADDR         descAddr;

        ARRIVE();
        
	acacia_mac_QueueInit(q, pMem, count);
        q->reapDescAddr = NULL;

	descAddr = q->firstDescAddr;
        /* Initialize Rx buffer descriptors */
        for (i=0; i<count; i++ ) {
            void *swptr;
            char *rxBuffer;
            int  rxBufferSize;

            swptr = acacia_rxbuf_alloc(MACInfo, &rxBuffer, &rxBufferSize);
            if (swptr == NULL) {
                    ACACIA_PRINT(ACACIA_DEBUG_RESET,
                              ("eth%d RX queue: acacia_rxbuf_alloc failed\n",
                               MACInfo->unit));
                    acacia_mac_QueueDestroy(q);
                    return -1;
            }
	    regVal = (rxBufferSize & DMAD_count_m) | DMAD_iof_m | DMAD_iod_m;
            ACACIA_DESC_CTRLEN_SET(descAddr, regVal);
            ACACIA_DESC_BUFPTR_SET(descAddr, virt_to_bus(rxBuffer));
            ACACIA_DESC_DEVCS_SET(descAddr, 0);
	    ACACIA_DESC_LNKBUF_SET(descAddr, (U32)0);
            ACACIA_DESC_SWPTR_SET(descAddr, swptr);

            descAddr=(VIRT_ADDR)((U32)descAddr + ACACIA_QUEUE_ELE_SIZE);
        } /* for each desc */

        ACACIA_DESC_LNKBUF_SET(q->lastDescAddr,(U32)0);//terminate the Last Desc

        ACACIA_PRINT(ACACIA_DEBUG_RESET,
                  ("eth%d Rxbuf begin = %x, end = %x\n",
                  MACInfo->unit,
                  (U32)q->firstDescAddr,
                  (U32)q->lastDescAddr));

        LEAVE();
        return 0;
}


/******************************************************************************
* acacia_mac_AllocateQueues - Allocate receive and transmit queues
*/
int
acacia_mac_AllocateQueues(acacia_MAC_t *MACInfo)
{
        size_t QMemSize;
        char *pTxBuf = NULL;
        char *pRxBuf = NULL;

        ARRIVE();

        QMemSize = ACACIA_QUEUE_ELE_SIZE * MACInfo->txDescCount;
        pTxBuf = MACInfo->pTxDescs;
        if (pTxBuf == NULL) {
            ACACIA_PRINT(ACACIA_DEBUG_RESET,
                      ("eth%d Failed to allocate TX queue\n", MACInfo->unit));
            goto AllocQFail;
        }

        if (acacia_mac_TxQueueCreate(MACInfo, &MACInfo->txQueue, pTxBuf,
                              MACInfo->txDescCount) < 0)
        {
            ACACIA_PRINT(ACACIA_DEBUG_RESET,
                    ("eth%d Failed to create TX queue\n", MACInfo->unit));
            goto AllocQFail;
        }

        QMemSize = ACACIA_QUEUE_ELE_SIZE * MACInfo->rxDescCount;
        pRxBuf = MACInfo->pRxDescs;
        if (pRxBuf == NULL) {
            ACACIA_PRINT(ACACIA_DEBUG_RESET,
                      ("eth%d Failed to allocate RX queue\n", MACInfo->unit));
            goto AllocQFail;
        }

        if (acacia_mac_RxQueueCreate(MACInfo, &MACInfo->rxQueue, pRxBuf,
                              MACInfo->rxDescCount) < 0)
        {
            ACACIA_PRINT(ACACIA_DEBUG_RESET,
                    ("eth%d Failed to create RX queue\n", MACInfo->unit));
            goto AllocQFail;
        }

        ACACIA_PRINT(ACACIA_DEBUG_RESET,
                ("eth%d Memory setup complete.\n", MACInfo->unit));

        LEAVE();
        return 0;

AllocQFail:
        MACInfo->txDescCount = 0; /* sanity */
        MACInfo->rxDescCount = 0; /* sanity */

        if (pTxBuf) {
            FREE(pTxBuf);
        }
        if (pRxBuf) {
            FREE(pRxBuf);
        }

        LEAVE();
        return -1;
}


/******************************************************************************
*
* acacia_mac_FreeQueues - Free Transmit & Receive queues
*/
void
acacia_mac_FreeQueues(acacia_MAC_t *MACInfo)
{
    acacia_mac_TxQueueDestroy(MACInfo);
    FREE(MACInfo->txQueue.firstDescAddr);

    acacia_mac_RxQueueDestroy(MACInfo);
    FREE(MACInfo->rxQueue.firstDescAddr);
}


void print_dma_rx_status(acacia_MAC_t *MACInfo) 
{
	diag_printf("0x%x=0x%x \n",&MACInfo->rx_dma_regs->dmac,
		    readl(&MACInfo->rx_dma_regs->dmac));
	diag_printf("0x%x=0x%x \n",&MACInfo->rx_dma_regs->dmas,
		    readl(&MACInfo->rx_dma_regs->dmas));
	diag_printf("0x%x=0x%x \n",&MACInfo->rx_dma_regs->dmasm,
		    readl(&MACInfo->rx_dma_regs->dmasm));
	diag_printf("0x%x=0x%x \n",&MACInfo->rx_dma_regs->dmadptr,
		    readl(&MACInfo->rx_dma_regs->dmadptr));
	diag_printf("0x%x=0x%x \n",&MACInfo->rx_dma_regs->dmandptr,
		    readl(&MACInfo->rx_dma_regs->dmandptr));

}

void print_dma_rxbd(DMAD_t rxbd) 
{
	diag_printf("0x%x=0x%x \n",&rxbd->control,readl(&rxbd->control));
	diag_printf("0x%x=0x%x \n",&rxbd->ca,readl(&rxbd->ca));
	diag_printf("0x%x=0x%x \n",&rxbd->devcs,readl(&rxbd->devcs));
	diag_printf("0x%x=0x%x \n",&rxbd->link,readl(&rxbd->link));
}

int poll_count = 0;
int print_interval=50000;

