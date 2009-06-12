//==========================================================================
//
//      dev/acacis_ecos.c
//
//      Ethernet driver for IDT 32438 MAC.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2005 Atheros Communications, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Atheros Communications, Inc.
// Contributors: Atheros Engineering
// Date:         2003-10-22
// Purpose:      
// Description:  AR531X ethernet hardware driver
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Ethernet device driver for IDT 32438

#include <pkgconf/system.h>
#include <pkgconf/io_eth_drivers.h>

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <cyg/kernel/kapi.h>
#endif
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_endian.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_if.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/eth_drv_stats.h>
#include <string.h>

#include "acacia_ecos.h"
#include "acacia_mac.h"
#include "acaciareg.h"

int acacia_debug =  ACACIA_DEBUG_ERROR | ACACIA_DEBUG_INIT;

extern unsigned char * enet_mac_address_get(int unit);


#ifndef CYGPKG_REDBOOT
//#define DEBUG
#endif

typedef struct acacia_priv_data_s {
	int                     enetUnit;
	unsigned char           enet_addr[ETHER_ADDR_LEN];
	acacia_MAC_t            MACInfo;
} acacia_priv_data_t;

/* Ethernet Unit 0 */
acacia_priv_data_t acacia_priv_data0 = {enetUnit: 0};

ETH_DRV_SC(acacia_sc0,
           &acacia_priv_data0,
           "eth0",
           acacia_start,
           acacia_stop,
           acacia_control,
           acacia_can_send,
           acacia_send,
           acacia_recv,
           acacia_deliver,
           acacia_poll,
           acacia_int_vector);

NETDEVTAB_ENTRY(acacia_netdev0, 
                "acacia", 
                acacia_init, 
                &acacia_sc0);

#define ETH_MAX_MTU 1536
#define CYGNUM_PHY_TRAILER_SIZE 4
#define ETHER_SMALL 64

#define ACACIA_RX_BUF_SIZE \
    (ETH_MAX_MTU + CYGNUM_PHY_TRAILER_SIZE)

#define ACACIA_TX_BUF_SIZE \
    (ETH_MAX_MTU)

typedef struct acaciaRxBuf_s {
    struct acaciaRxBuf_s *next;
    char                 data[ACACIA_RX_BUF_SIZE];
} acaciaRxBuf_t;
#define ACACIA_RX_DATA_BUF_COUNT 2*ACACIA_RX_DESC_COUNT_DEFAULT
acaciaRxBuf_t acaciaRxDataBuf[ACACIA_RX_DATA_BUF_COUNT];
acaciaRxBuf_t *acaciaRxFreeList = NULL;
acaciaRxBuf_t *acaciaRxRecvdListHead = NULL;
acaciaRxBuf_t *acaciaRxRecvdListTail = NULL;

typedef struct acaciaTxBuf_s {
    struct acaciaTxBuf_s *next;
    unsigned long        key;
    char                 data[ACACIA_TX_BUF_SIZE];
} acaciaTxBuf_t;
acaciaTxBuf_t acaciaTxDataBuf[ACACIA_TX_DESC_COUNT_DEFAULT];
acaciaTxBuf_t *acaciaTxFreeList = NULL;
unsigned int acaciaTxAvail;

static void acacia_tx_reap(acacia_MAC_t *MACInfo, struct eth_drv_sc *sc);
static void acacia_txbuf_alloc(acacia_MAC_t *MACInfo, acaciaTxBuf_t **txBptr);
static void acacia_rxbuf_free(acaciaRxBuf_t *rxDesc);

char default_mac_address[ETHER_ADDR_LEN] = {0x00, 0x03, 0x7f, 0xe0, 0x02, 0xbf};

/*
 * Standard eCos initialization routine.  Call the upper-level ethernet
 * driver and let it know our MAC address.
 */

static bool
acacia_init(struct cyg_netdevtab_entry *tab)
{
	char *mac_addr;
	struct eth_drv_sc *sc;
	acacia_priv_data_t *acacia_priv;
	acacia_MAC_t *MACInfo;
	int unit;

	ARRIVE();

	sc = (struct eth_drv_sc *)tab->device_instance;
    	acacia_priv = (acacia_priv_data_t *)sc->driver_private;
	unit = acacia_priv->enetUnit;
	
	MACInfo = &acacia_priv->MACInfo;

    	/* Get ethernet's MAC address from board configuration data */
	mac_addr = enet_mac_address_get(unit);
	if (mac_addr) {
		memcpy(MACInfo->enetAddr, mac_addr, ETHER_ADDR_LEN);
	}
	else {
		diag_printf(
		"Couldn't find valid MAC address for enet%d.Using default!\n",
		unit);
		memcpy(MACInfo->enetAddr, default_mac_address, ETHER_ADDR_LEN);
	}

	/* Initialize MACInfo */
	MACInfo->OSinfo = (void *)acacia_priv;
	MACInfo->unit = unit;

	if (unit == 0) {  //eth0
		MACInfo->eth_regs = (ETH_t) KSEG1ADDR(ETH0_VirtualAddress);
		MACInfo->rx_dma_regs =
		(DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress+ 2*DMA_CHAN_OFFSET));
		MACInfo->tx_dma_regs = 
		(DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress +3*DMA_CHAN_OFFSET));
    	}
	else {  //eth1
		MACInfo->eth_regs = (ETH_t) KSEG1ADDR(ETH1_VirtualAddress);
		MACInfo->rx_dma_regs =
		(DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress+4*DMA_CHAN_OFFSET));
		MACInfo->tx_dma_regs = 
		(DMA_Chan_t)KSEG1ADDR((DMA0_VirtualAddress+5*DMA_CHAN_OFFSET));
	}
	
	(sc->funs->eth_drv->init)(sc, MACInfo->enetAddr);

	LEAVE();

	return true;
}

/* Space for Transmit and Receive descriptors */
char acaciaTxDescs[ACACIA_QUEUE_ELE_SIZE * ACACIA_TX_DESC_COUNT_DEFAULT];
char acaciaRxDescs[ACACIA_QUEUE_ELE_SIZE * ACACIA_RX_DESC_COUNT_DEFAULT];

static void
acacia_start(struct eth_drv_sc *sc,
             unsigned char *enet_addr,
             int flags)
{
        acacia_priv_data_t *acacia_priv;
        acacia_MAC_t *MACInfo;
        int i;

        ARRIVE();

        acacia_priv = (acacia_priv_data_t *)sc->driver_private;
        MACInfo = (acacia_MAC_t *)&acacia_priv->MACInfo;

        memcpy(acacia_priv->enet_addr, enet_addr, ETHER_ADDR_LEN);

        /* Attach interrupt handler */
        /* TBDXXX */

        /* Start thread to poll for phy link status changes */
        /* done in acacia_poll */

        /* Setup Transmit Descriptors */
        MACInfo->txDescCount = ACACIA_TX_DESC_COUNT_DEFAULT;
        MACInfo->pTxDescs = acaciaTxDescs;

        /* Setup Transmit buffer free list */
        for (i=0; i<ACACIA_TX_DESC_COUNT_DEFAULT; i++) {
            acaciaTxDataBuf[i].next = acaciaTxFreeList;
            acaciaTxFreeList = &acaciaTxDataBuf[i];
        }
        acaciaTxAvail = ACACIA_TX_DESC_COUNT_DEFAULT;

        /* Setup Receive Descriptors */
        MACInfo->rxDescCount = ACACIA_RX_DESC_COUNT_DEFAULT;
        MACInfo->pRxDescs = acaciaRxDescs;

        /* Setup Receive buffer free list */
        for (i=0; i<ACACIA_RX_DATA_BUF_COUNT; i++) {
            acaciaRxDataBuf[i].next = acaciaRxFreeList;
            acaciaRxFreeList = &acaciaRxDataBuf[i];
        }

        /* Allocate RX/TX Queues */
        if (acacia_mac_AllocateQueues(MACInfo) < 0) {
            ACACIA_PRINT(ACACIA_DEBUG_RESET, ("Queue allocation failed"));
            goto skip_start;
        }

        /* Initialize MAC, DMA and PHY */
        acacia_mac_reset(MACInfo);
        acacia_mac_init(MACInfo);

skip_start:
	LEAVE();
}

static void
acacia_stop(struct eth_drv_sc *sc)
{
	ARRIVE();
	LEAVE();
}

static int
acacia_control(struct eth_drv_sc *sc,
               unsigned long key,
               void *data,
               int data_length)
{
	ARRIVE();
	/* TBDXXX */
	LEAVE();
	return 0;
}

static int
acacia_can_send(struct eth_drv_sc *sc)
{
	return acaciaTxAvail;
}

static void
acacia_send(struct eth_drv_sc *sc,
            struct eth_drv_sg *sg_list,
            int sg_len,
            int total_len,
            unsigned long key)
{
        acacia_priv_data_t *acacia_priv;
	acacia_MAC_t *MACInfo;
        U32 length;
	VIRT_ADDR txDesc;
	acaciaTxBuf_t *TxBuf;
	unsigned char *TxBufData, *TxBufDataStart;
        struct eth_drv_sg *sg_item;
        volatile U32 i, regVal;

	ARRIVE();
        acacia_priv = (acacia_priv_data_t *)sc->driver_private;
        MACInfo = (acacia_MAC_t *)&acacia_priv->MACInfo;

        /* Check if this port is up, else toss packet */
        if (!MACInfo->port_is_up) {
        	/* Though we actually did not transmit any thing, 
                 * Let upper layers know that we're done with this transmit
                 * to avoid unnecessary polling in eth_drv_write() */
                (sc->funs->eth_drv->tx_done)(sc, key, 0);
            	goto dropFrame;
        }
        
	/* Check if we can transport this packet */
        if (total_len > ETH_MAX_MTU) {
            ACACIA_PRINT(ACACIA_DEBUG_ERROR,
                      ("eth%d Tx: length %d too long.  mtu=%d\n",
                       acacia_priv->enetUnit, total_len, ETH_MAX_MTU));


            goto dropFrame;
        }
        
	/* Reap any old, completed Tx descriptors */
        acacia_tx_reap(MACInfo, sc);

        txDesc = MACInfo->txQueue.curDescAddr;
        if (txDesc == MACInfo->txQueue.reapDescAddr) {
            ACACIA_PRINT(ACACIA_DEBUG_ERROR,
                      ("eth%d Tx: cannot get txDesc\n",
                       MACInfo->unit));

            goto dropFrame;
        }

        /* We won't fail now; so consume this descriptor */
        ACACIA_CONSUME_DESC((&MACInfo->txQueue));
        acaciaTxAvail--;

        /* Allocate a transmit data buffer */
        acacia_txbuf_alloc(MACInfo, &TxBuf);
        TxBuf->key = key;
        ACACIA_DESC_SWPTR_SET(txDesc, TxBuf);

        TxBufDataStart = TxBufData = TxBuf->data;

        ACACIA_PRINT(ACACIA_DEBUG_TX, 
		("\nSEND: txDesc=%p   TxBuf=%p   TxBufData=%p  GET=%p\n",
                     txDesc, TxBuf, TxBufData,
                     ACACIA_DESC_SWPTR_GET(txDesc)));

	
	/* Copy and coalesce from sg_list to buf/length */
	length = 0;
	sg_item = sg_list;
	for (i=0; i<sg_len; sg_item++, i++) {
        	char *segment_addr;
	        int segment_len;
        
		segment_addr = (char *)sg_item->buf;
	        segment_len = sg_item->len;

        	length += segment_len;

	        if (length > total_len) {
			ACACIA_PRINT(ACACIA_DEBUG_TX,(
			"acacia_send: scatter/gather err. copied=%d total=%d\n",
			length, total_len));
        		break;
        	}
	        memcpy(TxBufData, segment_addr, segment_len);
        	TxBufData += segment_len;
    	}

 	if (length != total_len) {
        	diag_printf(
			"acacia_send: sg_list too short.  length=%d total=%d\n",
                	 length, total_len);
    	}

	/* Do the padding if necessary */
	while (length < ETHER_SMALL){
	        TxBufData[length] = (char)NULL;
		length++;
	}


	/* set up the TDR for transmission */
	regVal = ( DMAD_iof_m | DMAD_iod_m | (length & DMAD_count_m));
	ACACIA_DESC_CTRLEN_SET(txDesc, regVal);
	
	regVal = (DMAD_TX_DEVCS_oen_m | DMAD_TX_DEVCS_cen_m |
			 DMAD_TX_DEVCS_fd_m | DMAD_TX_DEVCS_ld_m);
	ACACIA_DESC_DEVCS_SET(txDesc, regVal);

	ACACIA_DESC_BUFPTR_SET(txDesc, virt_to_bus(TxBufDataStart));
	ACACIA_DESC_LNKBUF_SET(txDesc, 0);
    	
	/* Alert DMA engine to resume Tx */
	writel(virt_to_bus(txDesc) , &MACInfo->tx_dma_regs->dmadptr);
    	ACACIA_PRINT(ACACIA_DEBUG_TX,
              ("\neth%d Tx: Desc=0x%x, Control=0x%x, DataStart=0x%x, ca=0x%x, length=0x%x\n",
               MACInfo->unit,
               (U32)txDesc,
               ACACIA_DESC_CTRLEN_GET(txDesc),
               TxBufDataStart,
               ACACIA_DESC_BUFPTR_GET(txDesc),
               length));
dropFrame:
	LEAVE();
    return;
}


static void
acacia_recv(struct eth_drv_sc *sc,
            struct eth_drv_sg *sg_list,
            int sg_len)
{
        acaciaRxBuf_t *rxBuf;
        unsigned char *RxBufData;
        struct eth_drv_sg *sg_item;
        int i;

        ARRIVE();

#if defined(DEBUG)
        saved_sg_list = sg_list;
        saved_sg_len = sg_len;
#endif

        rxBuf = acaciaRxRecvdListHead;
        acaciaRxRecvdListHead = acaciaRxRecvdListHead->next;

        if ((char *)sg_list->buf != NULL) {
        	/* Copy out from driver Rx buffer to sg_list */
                RxBufData = rxBuf->data;
                sg_item = sg_list;
                for (i=0; i<sg_len; sg_item++, i++) {
                	char *segment_addr;
                	int segment_len;

	                segment_addr = (char *)sg_item->buf;
        	        segment_len = sg_item->len;

                	memcpy(segment_addr, RxBufData, segment_len);
	                RxBufData += segment_len;
        	}
        }
	else {
            /* Handle according to convention: bit bucket the packet */
        }

        /* Free driver Rx buffer */
        acacia_rxbuf_free(rxBuf);
        LEAVE();
}

static void
acacia_deliver(struct eth_drv_sc *sc)
{
	ARRIVE();
	/* TBDXXX */
	LEAVE();
}

int scanUnit = 0;

static void
acacia_poll(struct eth_drv_sc *sc)
{
        acaciaRxBuf_t *rxBuf, *newRxBuf;
        char *rxBufData;
        int unused_length;
        VIRT_ADDR   rxDesc;
        int length;
        acacia_MAC_t *MACInfo;
        U32 cmdsts, checkFlags;
        acacia_priv_data_t *acacia_priv;

        acacia_priv = (acacia_priv_data_t *)sc->driver_private;
        MACInfo = (acacia_MAC_t *)&acacia_priv->MACInfo;

	if(! MACInfo->port_is_up ) {
	 	if( acacia_mac_init(MACInfo) != OK) {
			scanUnit = (scanUnit ? 0 : 1);
			acacia_mac_start_scan(scanUnit);
			return;
		}
	}

	do {
            for(;;) {
                rxDesc = MACInfo->rxQueue.curDescAddr;

                cmdsts = ACACIA_DESC_STATUS_GET(KSEG1ADDR(rxDesc));
                if ( (cmdsts & (DMAD_f_m | DMAD_d_m)) == 0 ){
			  /*Neither Finish nor Done flag is set */
	                  /* There's nothing left to process in the RX ring */
        	          goto rx_all_done;
                }

                ACACIA_PRINT(ACACIA_DEBUG_RX,
                      ("consume rxDesc %p with cmdsts=0x%x\n",
                       (void *)rxDesc, cmdsts));

                ACACIA_CONSUME_DESC((&MACInfo->rxQueue));

                A_DATA_CACHE_INVAL(rxDesc, ACACIA_DESC_SIZE);

                /*  Process a packet */
                length = ACACIA_DESC_DEVCS_GET(KSEG1ADDR(rxDesc));
                length = ((length >> 16) & 0xFFFF)  - ETH_CRC_LEN;

		checkFlags = ( DMAD_RX_DEVCS_fd_m | DMAD_RX_DEVCS_ld_m |
				 DMAD_RX_DEVCS_rok_m );        
		cmdsts = ACACIA_DESC_DEVCS_GET(KSEG1ADDR(rxDesc));
                if ( ( cmdsts & checkFlags ) == checkFlags) {
			/* Descriptor status indicates "NO errors" */
			rxBuf = ACACIA_DESC_SWPTR_GET(rxDesc);

                    	/*
	                * Allocate a replacement buffer
         	        * We want to get another buffer ready for Rx ASAP.
                	*/
	                newRxBuf = acacia_rxbuf_alloc(MACInfo, &rxBufData,
						 &unused_length);
        	        if(newRxBuf == NULL ) {
                        /*
                         * Give this descriptor back to the DMA engine,
                         * and drop the received packet.
                         */
                        ACACIA_PRINT(ACACIA_DEBUG_ERROR,
                                  ("Can't allocate new rx buf\n"));
                        }
			else {
                        	ACACIA_DESC_BUFPTR_SET(rxDesc, 
						virt_to_bus(rxBufData));
                        	ACACIA_DESC_SWPTR_SET(rxDesc, newRxBuf);
                        }
	    		checkFlags = (unused_length & DMAD_count_m) |
						 DMAD_iof_m | DMAD_iod_m;
                        ACACIA_DESC_CTRLEN_SET(rxDesc, checkFlags);
			ACACIA_DESC_DEVCS_SET(rxDesc, 0);

			/* Clear DMA Status Reg */
			writel(0, &MACInfo->rx_dma_regs->dmas); 
			/* Set the current Rx Desc in dmadptr reg */
                	rxDesc = MACInfo->rxQueue.curDescAddr;
			writel(virt_to_bus(rxDesc),
				&MACInfo->rx_dma_regs->dmadptr); 
 
                        rxDesc = NULL; /* sanity -- cannot use rxDesc now */
                        //sysWbFlush();

                        if (newRxBuf == NULL) {
                        	goto no_rx_bufs;
                        }
			else {
                        	/* Sync data cache w.r.t. DMA */
                        	A_DATA_CACHE_INVAL(rxBuf->data, length);

	                        /* Send the data up the stack */
	                        ACACIA_PRINT(ACACIA_DEBUG_RX,
	                          ("Send data up stack: rxBuf=%p data=%p length=%d\n",
	                          (void *)rxBuf, (void *)rxBuf->data, length));

                                rxBuf->next = NULL;
                                if (acaciaRxRecvdListHead) {
                                	acaciaRxRecvdListTail->next = rxBuf;
                                	acaciaRxRecvdListTail = rxBuf;
                                }
				else {
                                	acaciaRxRecvdListHead = rxBuf;
	                                acaciaRxRecvdListTail = rxBuf;
        	                }
                                (sc->funs->eth_drv->recv)(sc, length);
                    	}
                }
		else {
                	ACACIA_PRINT(ACACIA_DEBUG_ERROR,
                              ("Bad receive.  rxDesc=%p  cmdsts=0x%8.8x\n",
                               (void *)rxDesc, cmdsts));
                }
            }
        } while (readl(&MACInfo->rx_dma_regs->dmas) & (DMAD_f_m|DMAD_d_m));

rx_all_done:
no_rx_bufs:

        return;
}

static int
acacia_int_vector(struct eth_drv_sc *sc)
{
	ARRIVE();

	LEAVE();
	return 0;
}


void *
acacia_rxbuf_alloc(acacia_MAC_t *MACInfo, char **rxBptr, int *rxBSize)
{
        acaciaRxBuf_t *rxbuf;

        if (acaciaRxFreeList) {
            rxbuf = acaciaRxFreeList;
            acaciaRxFreeList = acaciaRxFreeList->next;

            *rxBptr = (char *)rxbuf->data;
            *rxBSize = sizeof(rxbuf->data);
            return (void *)rxbuf;
        }

        DEBUG_PRINTF("acacia_rxbuf_alloc failed!\n");
        *rxBptr = NULL;

        return (void *)NULL;
}

void
acacia_rxbuf_free(acaciaRxBuf_t *rxBuf)
{
        if (rxBuf) {
            rxBuf->next = acaciaRxFreeList;
            acaciaRxFreeList = rxBuf;
        }
}


static void
acacia_txbuf_alloc(acacia_MAC_t *MACInfo, acaciaTxBuf_t **txBptr)
{
        acaciaTxBuf_t *txbuf;

        if (acaciaTxFreeList) {
            txbuf = acaciaTxFreeList;
            acaciaTxFreeList = acaciaTxFreeList->next;

            *txBptr = txbuf;
            return;
        }

        DEBUG_PRINTF("acacia_txbuf_alloc failed!\n");
        *txBptr = NULL;

        return;
}

void
acacia_txbuf_free(acaciaTxBuf_t *txBuf)
{

        if (txBuf) {
            txBuf->next = acaciaTxFreeList;
            acaciaTxFreeList = txBuf;
        }
}

int acaciaUselessReap = 0;
static void
acacia_tx_reap(acacia_MAC_t *MACInfo, struct eth_drv_sc *sc)
{
        ACACIA_QUEUE      *txq;
        VIRT_ADDR         txDesc;
        U32               cmdsts;
        int               reaped;
        acaciaTxBuf_t     *TxBuf;
        unsigned long     key;

        txq = &MACInfo->txQueue;
        reaped = 0;

        // XXXTODO: bug - won't be able to reap once run out of descriptors
        while (1) {
        	txDesc = ACACIA_QUEUE_ELE_NEXT_GET(txq, txq->reapDescAddr);
 	        if (txDesc == txq->curDescAddr) {
        	        break;
            	}

		cmdsts = ACACIA_DESC_STATUS_GET(KSEG1ADDR(txDesc));
                if ((!(cmdsts & DMAD_f_m)) && (!(cmdsts & DMAD_d_m)) ){
                        /*Neither Finish nor Done flag is set */
                        /* DMA has not done with this descripter yet */
       			break;
                }

                TxBuf = (acaciaTxBuf_t *)ACACIA_DESC_SWPTR_GET(txDesc);
                ACACIA_PRINT(ACACIA_DEBUG_TX_REAP,
		 	("TXREAP: cur=%p  reap=%p  txDesc=%p TxBuf=%p\n",
                         txq->curDescAddr, txq->reapDescAddr, txDesc, TxBuf));
                key = TxBuf->key;

                /* Release transmit buffer associated with completed transmit */
                acacia_txbuf_free(TxBuf);
                ACACIA_DESC_SWPTR_SET(txDesc, (void *)0x1bad0dad);
                acaciaTxAvail++;

                /* Let upper layers know that we're done with this transmit */
                (sc->funs->eth_drv->tx_done)(sc, key, 0);

                txq->reapDescAddr = txDesc;
                reaped++;
        }

        if (reaped > 0) {
        	ACACIA_PRINT(ACACIA_DEBUG_TX_REAP,
                     ("reaped=%d    TxAvail=%d\n", reaped, acaciaTxAvail));
        }
	else {
        	acaciaUselessReap++;
        }
}



