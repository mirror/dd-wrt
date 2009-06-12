//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
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
/*
 * Ethernet driver for Atheros' ae531x ethernet MAC.
 */

/*#if defined(linux)
#include <linux/config.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <asm/io.h>

#include "ar531xlnx.h"
#endif 
*/
#if defined(__ECOS)
#include "ae531xecos.h"
#endif

#include "ae531xreg.h"
#include "ae531xmac.h"
#include "ae531xPhy.h"

int ae531x_debug = AE531X_DEBUG_ERROR;

/*
 * These externs are for functions that this layer relies on
 * that have OS-dependent implementations.
 */
extern UINT8 *macAddrGet(ae531x_MAC_t *MACInfo);

/* Forward references to local functions */
static void ae531x_QueueDestroy(AE531X_QUEUE *q);


/******************************************************************************
*
* ae531x_ReadMacReg - read AE MAC register
*
* RETURNS: register value
*/
UINT32
ae531x_ReadMacReg(ae531x_MAC_t *MACInfo, UINT32 reg)
{
    volatile UINT32 addr = MACInfo->macBase+reg;
    UINT32 data;

    data = RegRead(addr);
    return data;
}


/******************************************************************************
*
* ae531x_WriteMacReg - write AE MAC register
*
* RETURNS: N/A
*/
void
ae531x_WriteMacReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 data)
{
    volatile UINT32 addr = MACInfo->macBase+reg;

    RegWrite(data, addr);
}


/******************************************************************************
*
* ae531x_SetMacReg - set bits in AE MAC register
*
* RETURNS: N/A
*/
void
ae531x_SetMacReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 val)
{
    volatile UINT32 addr = MACInfo->macBase+reg;
    UINT32 data = RegRead(addr);

    data |= val;
    RegWrite(data, addr);
}


/******************************************************************************
*
* ae531x_ClearMacReg - clear bits in AE MAC register
*
* RETURNS: N/A
*/
void
ae531x_ClearMacReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 val)
{
    volatile UINT32 addr = MACInfo->macBase+reg;
    UINT32 data = RegRead(addr);

    data &= ~val;
    RegWrite(data, addr);
}


/******************************************************************************
*
* ae531x_ReadDmaReg - read AE DMA register
*
* RETURNS: register value
*/
UINT32
ae531x_ReadDmaReg(ae531x_MAC_t *MACInfo, UINT32 reg)
{
    volatile UINT32 addr = MACInfo->dmaBase+reg;
    UINT32 data = RegRead(addr);

    return data;
}


/******************************************************************************
*
* ae531x_WriteDmaReg - write AE DMA register
*
* RETURNS: N/A
*/
void
ae531x_WriteDmaReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 data)
{
    volatile UINT32 addr = MACInfo->dmaBase+reg;

    RegWrite(data, addr);
}


/******************************************************************************
 *
 * ae531x_AckIntr - clear interrupt bits in the status register.
 * Note: Interrupt bits are *cleared* by writing a 1.
 */
void
ae531x_AckIntr(ae531x_MAC_t *MACInfo, UINT32 data)
{
      ae531x_WriteDmaReg(MACInfo, DmaStatus, data);
}


/******************************************************************************
*
* ae531x_SetDmaReg - set bits in an AE DMA register
*
* RETURNS: N/A
*/
void
ae531x_SetDmaReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 val)
{
    volatile UINT32 addr = MACInfo->dmaBase+reg;
    UINT32 data = RegRead(addr);

    data |= val;
    RegWrite(data, addr);
}


/******************************************************************************
*
* ae531x_ClearDmaReg - clear bits in an AE DMA register
*
* RETURNS: N/A
*/
void
ae531x_ClearDmaReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 val)
{
    volatile UINT32 addr = MACInfo->dmaBase+reg;
    UINT32 data = RegRead(addr);

    data &= ~val;
    RegWrite(data, addr);
}


/******************************************************************************
*
* ae531x_ReadMiiReg - read PHY registers via AE MAC Mii addr/data registers
*
* RETURNS: register value
*/
UINT32
ae531x_ReadMiiReg(UINT32 phyBase, UINT32 reg)
{
    UINT32 data;
    volatile UINT32 addr = phyBase+reg;

    data = RegRead(addr);
    return data;
}


/******************************************************************************
*
* ae531x_WriteMiiReg - write PHY registers via AE MAC Mii addr/data registers
*
* RETURNS: N/A
*/
void
ae531x_WriteMiiReg(UINT32 phyBase, UINT32 reg, UINT32 data)
{
    volatile UINT32 addr = phyBase+reg;

    RegWrite(data, addr);
}


/******************************************************************************
*
* ae531x_MiiRead - read AE Mii register
*
* RETURNS: register value
*/
UINT16
ae531x_MiiRead(UINT32 phyBase, UINT32 phyAddr, UINT8 reg)
{
    volatile UINT32 addr;
    UINT16 data;

    addr = ((phyAddr << MiiDevShift) & MiiDevMask) | ((reg << MiiRegShift) & MiiRegMask);

    ae531x_WriteMiiReg(phyBase, MacMiiAddr, addr );
    do {
        /* nop */
    } while ((ae531x_ReadMiiReg(phyBase, MacMiiAddr ) & MiiBusy) == MiiBusy);

    data = ae531x_ReadMiiReg(phyBase, MacMiiData) & 0xFFFF;

    return data;
}


/******************************************************************************
*
* ae531x_MiiWrite - write AE Mii register
*
* RETURNS: N/A
*/
void
ae531x_MiiWrite(UINT32 phyBase, UINT32 phyAddr, UINT8 reg, UINT16 data)
{
    volatile UINT32 addr;

    ae531x_WriteMiiReg(phyBase, MacMiiData, data );

    addr = ((phyAddr << MiiDevShift) & MiiDevMask) |
        ((reg << MiiRegShift) & MiiRegMask) | MiiWrite;
    ae531x_WriteMiiReg(phyBase, MacMiiAddr, addr );

    do {
        /* nop */
    } while ((ae531x_ReadMiiReg(phyBase, MacMiiAddr ) & MiiBusy) == MiiBusy);
}


/*******************************************************************************
* ae531x_BeginResetMode - enter a special "reset mode" in which
*    -no interrupts are expected from the device
*    -the device will not transmit nor receive
*    -attempts to send or receive will return with an error and
*    -the device will be reset at the next convenient opportunity.
*/
void
ae531x_BeginResetMode(ae531x_MAC_t *MACInfo)
{
    /* Set the reset flag */
    MACInfo->aeProcessRst = 1;
}


/*******************************************************************************
* ae531x_EndResetMode - exit the special "reset mode" entered
* earlier via a call to ae531x_BeginResetMode.
*/
void
ae531x_EndResetMode(ae531x_MAC_t *MACInfo)
{
    MACInfo->aeProcessRst = 0;
}


/*******************************************************************************
* ae531x_IsInResetMode - determine whether or not the device is
* currently in "reset mode" (i.e. that a device reset is pending)
*/
BOOL
ae531x_IsInResetMode(ae531x_MAC_t *MACInfo)
{
    return MACInfo->aeProcessRst;
}


/******************************************************************************
*
* ae531x_DmaRxStart - Start Rx
*
* RETURNS: N/A
*/
static void
ae531x_DmaRxStart(ae531x_MAC_t *MACInfo)
{
    ae531x_SetDmaReg(MACInfo, DmaControl, DmaRxStart);
    sysWbFlush();
    sysMsDelay(10);
    ae531x_WriteDmaReg(MACInfo, DmaRxBaseAddr, virt_to_bus(MACInfo->pRxDescs));
    ae531x_SetDmaReg(MACInfo, DmaControl, DmaRxStart);
}


/******************************************************************************
*
* ae531x_DmaRxStop - Stop Rx
*
* RETURNS: N/A
*/
void
ae531x_DmaRxStop(ae531x_MAC_t *MACInfo)
{
    ae531x_ClearDmaReg(MACInfo, DmaControl, DmaRxStart);
    sysWbFlush();
}


/******************************************************************************
*
* ae531x_DmaTxStart - Start Tx
*
* RETURNS: N/A
*/
void
ae531x_DmaTxStart(ae531x_MAC_t *MACInfo)
{
    /* Init the hw Tx queue */
    ae531x_SetDmaReg(MACInfo, DmaControl, DmaTxStart);
    sysWbFlush();
    sysMsDelay(10);
    ae531x_WriteDmaReg(MACInfo, DmaTxBaseAddr, virt_to_bus(MACInfo->pTxDescs));
    ae531x_SetDmaReg(MACInfo, DmaControl, DmaTxStart);
}


/******************************************************************************
*
* ae531x_DmaTxStop - Stop Tx
*
* RETURNS: N/A
*/
void
ae531x_DmaTxStop(ae531x_MAC_t *MACInfo)
{
    ae531x_ClearDmaReg(MACInfo, DmaControl, DmaTxStart);
    sysWbFlush();
}


/******************************************************************************
*
* ae531x_DmaIntEnable - Enable DMA interrupts
*
* RETURNS: N/A
*/
void
ae531x_DmaIntEnable(ae531x_MAC_t *MACInfo)
{
    ae531x_WriteDmaReg(MACInfo, DmaIntrEnb, DmaIntEnable);
}


/******************************************************************************
*
* ae531x_DmaIntDisable - Disable DMA interrupts
*
* RETURNS: N/A
*/
void
ae531x_DmaIntDisable(ae531x_MAC_t *MACInfo)
{
    ae531x_WriteDmaReg(MACInfo, DmaIntrEnb, DmaIntDisable);
}


/******************************************************************************
*
* ae531x_DmaIntClear - Clear DMA interrupts
*
* RETURNS: N/A
*/
static void
ae531x_DmaIntClear(ae531x_MAC_t *MACInfo)
{
    /* clear all interrupt requests */
    ae531x_WriteDmaReg(MACInfo, DmaStatus,
                      ae531x_ReadDmaReg(MACInfo, DmaStatus));  
}


/******************************************************************************
* Initialize generic queue data
*/
void
ae531x_QueueInit(AE531X_QUEUE *q, char *pMem, int count)
{
    ARRIVE();
    q->firstDescAddr = pMem;
    q->lastDescAddr = (VIRT_ADDR)((UINT32)q->firstDescAddr +
                                  (count - 1) * AE531X_QUEUE_ELE_SIZE);
    q->curDescAddr = q->firstDescAddr;
    q->count = count;
    LEAVE();
}


/******************************************************************************
* ae531x_TxQueueCreate - create a circular queue of descriptors for Transmit
*/
static int
ae531x_TxQueueCreate(ae531x_MAC_t *MACInfo,
                  AE531X_QUEUE *q,
                  char *pMem,
                  int count)
{
    int         i;
    VIRT_ADDR   descAddr;

    ARRIVE();

    ae531x_QueueInit(q, pMem, count);
    q->reapDescAddr = q->lastDescAddr;

    /* Initialize Tx buffer descriptors.  */
    for (i=0, descAddr=q->firstDescAddr;
         i<count;
         i++, descAddr=(VIRT_ADDR)((UINT32)descAddr + AE531X_QUEUE_ELE_SIZE))
    {
        /* Update the size, BUFPTR, and SWPTR fields */

        AE531X_DESC_STATUS_SET(descAddr, 0);
        AE531X_DESC_CTRLEN_SET(descAddr, 0);

        AE531X_DESC_BUFPTR_SET(descAddr, (UINT32)0);
        AE531X_DESC_LNKBUF_SET(descAddr, (UINT32)0);
        AE531X_DESC_SWPTR_SET(descAddr, (void *)0);
    } /* for each desc */

    /* Make the queue circular */
    AE531X_DESC_CTRLEN_SET(q->lastDescAddr,
                       DescEndOfRing|AE531X_DESC_CTRLEN_GET(q->lastDescAddr));

    AE531X_PRINT(AE531X_DEBUG_RESET,
            ("eth%d Txbuf begin = %x, end = %x\n",
            MACInfo->unit,
            (UINT32)q->firstDescAddr,
            (UINT32)q->lastDescAddr));

    LEAVE();
    return 0;
}


/******************************************************************************
* ae531x_RxQueueCreate - create a circular queue of Rx descriptors
*/
int
ae531x_RxQueueCreate(ae531x_MAC_t *MACInfo,
                  AE531X_QUEUE *q,
                  char *pMem,
                  int count)
{
    int               i;
    VIRT_ADDR         descAddr;

    ARRIVE();

    ae531x_QueueInit(q, pMem, count);
    q->reapDescAddr = NULL;


    /* Initialize Rx buffer descriptors */
    for (i=0, descAddr=q->firstDescAddr;
         i<count;
         i++, descAddr=(VIRT_ADDR)((UINT32)descAddr + AE531X_QUEUE_ELE_SIZE))
    {
        void *swptr;
        char *rxBuffer;
        int  rxBufferSize;

        swptr = ae531x_rxbuf_alloc(MACInfo, &rxBuffer, &rxBufferSize);
        if (swptr == NULL) {
                AE531X_PRINT(AE531X_DEBUG_RESET,
                          ("eth%d RX queue: ae531x_rxbuf_alloc failed\n",
                           MACInfo->unit));
                ae531x_QueueDestroy(q);
                return -1;
        }
        AE531X_DESC_SWPTR_SET(descAddr, swptr);

        AE531X_DESC_STATUS_SET(descAddr, DescOwnByDma);
        AE531X_DESC_CTRLEN_SET(descAddr, rxBufferSize);
        AE531X_DESC_BUFPTR_SET(descAddr, virt_to_bus(rxBuffer));
        AE531X_DESC_LNKBUF_SET(descAddr, (UINT32)0);
    } /* for each desc */

    /* Make the queue circular */
    AE531X_DESC_CTRLEN_SET(q->lastDescAddr,
                       DescEndOfRing|AE531X_DESC_CTRLEN_GET(q->lastDescAddr));

    AE531X_PRINT(AE531X_DEBUG_RESET,
              ("eth%d Rxbuf begin = %x, end = %x\n",
              MACInfo->unit,
              (UINT32)q->firstDescAddr,
              (UINT32)q->lastDescAddr));

    LEAVE();
    return 0;
}


/******************************************************************************
* ae531x_QueueDestroy -- Free all buffers and descriptors associated 
* with a queue.
*/
static void
ae531x_QueueDestroy(AE531X_QUEUE *q)
{
    int i;
    int count;
    VIRT_ADDR    descAddr;

    ARRIVE();

    count = q->count;

    for (i=0, descAddr=q->firstDescAddr;
         i<count;
         i++, descAddr=(VIRT_ADDR)((UINT32)descAddr + AE531X_QUEUE_ELE_SIZE)) {

        AE531X_DESC_STATUS_SET(descAddr, 0);
        AE531X_DESC_CTRLEN_SET(descAddr, 0);
        AE531X_DESC_BUFPTR_SET(descAddr, (UINT32)0);
        AE531X_DESC_LNKBUF_SET(descAddr, (UINT32)0);

#if 0 /* TBDXXX */
        ae531x_rxbuf_free(descAddr); /* Free OS-specific software pointer */
#endif
        AE531X_DESC_SWPTR_SET(descAddr, NULL);
    }

    LEAVE();
}

static void
ae531x_TxQueueDestroy(ae531x_MAC_t *MACInfo)
{
    ae531x_QueueDestroy(&MACInfo->txQueue);
}

static void
ae531x_RxQueueDestroy(ae531x_MAC_t *MACInfo)
{
    ae531x_QueueDestroy(&MACInfo->rxQueue);
}


/******************************************************************************
* ae531x_AllocateQueues - Allocate receive and transmit queues
*/
int
ae531x_AllocateQueues(ae531x_MAC_t *MACInfo)
{
    size_t QMemSize;
    char *pTxBuf = NULL;
    char *pRxBuf = NULL;

    ARRIVE();

    QMemSize = AE531X_QUEUE_ELE_SIZE * MACInfo->txDescCount;
    pTxBuf = MACInfo->pTxDescs;
    if (pTxBuf == NULL) {
        AE531X_PRINT(AE531X_DEBUG_RESET,
                  ("eth%d Failed to allocate TX queue\n", MACInfo->unit));
        goto AllocQFail;
    }

    if (ae531x_TxQueueCreate(MACInfo, &MACInfo->txQueue, pTxBuf,
                          MACInfo->txDescCount) < 0)
    {
        AE531X_PRINT(AE531X_DEBUG_RESET,
                ("eth%d Failed to create TX queue\n", MACInfo->unit));
        goto AllocQFail;
    }

    QMemSize = AE531X_QUEUE_ELE_SIZE * MACInfo->rxDescCount;
    pRxBuf = MACInfo->pRxDescs;
    if (pRxBuf == NULL) {
        AE531X_PRINT(AE531X_DEBUG_RESET,
                  ("eth%d Failed to allocate RX queue\n", MACInfo->unit));
        goto AllocQFail;
    }

    if (ae531x_RxQueueCreate(MACInfo, &MACInfo->rxQueue, pRxBuf,
                          MACInfo->rxDescCount) < 0)
    {
        AE531X_PRINT(AE531X_DEBUG_RESET,
                ("eth%d Failed to create RX queue\n", MACInfo->unit));
        goto AllocQFail;
    }

    AE531X_PRINT(AE531X_DEBUG_RESET,
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
* ae531x_FreeQueues - Free Transmit & Receive queues
*/
void
ae531x_FreeQueues(ae531x_MAC_t *MACInfo)
{
    ae531x_TxQueueDestroy(MACInfo);
    FREE(MACInfo->txQueue.firstDescAddr);

    ae531x_RxQueueDestroy(MACInfo);
    FREE(MACInfo->rxQueue.firstDescAddr);
}

/******************************************************************************
*
* ae531x_DmaReset - Reset DMA and TLI controllers
*
* RETURNS: N/A
*/
void
ae531x_DmaReset(ae531x_MAC_t *MACInfo)
{
    int           i;
    UINT32        descAddr;
    unsigned long oldIntrState;

    ARRIVE();

    /* Disable device interrupts prior to any errors during stop */
    intDisable(oldIntrState);

    /* Disable MAC rx and tx */
    ae531x_ClearMacReg(MACInfo, MacControl, (MacRxEnable | MacTxEnable));

    /* Reset dma controller */
    ae531x_WriteDmaReg(MACInfo, DmaBusMode, DmaResetOn);

    /* Delay 2 usec */
    sysUDelay(2);

    /* Flush the rx queue */
    descAddr = (UINT32)MACInfo->rxQueue.firstDescAddr;
    MACInfo->rxQueue.curDescAddr = MACInfo->rxQueue.firstDescAddr;
    for (i=0;
         i<(MACInfo->rxDescCount);
         i++, descAddr += AE531X_QUEUE_ELE_SIZE) {
            AE531X_DESC_STATUS_SET(descAddr, DescOwnByDma);
    }

    /* Flush the tx queue */
    descAddr = (UINT32)MACInfo->txQueue.firstDescAddr;
    MACInfo->txQueue.curDescAddr = MACInfo->txQueue.firstDescAddr;
    MACInfo->txQueue.reapDescAddr = MACInfo->txQueue.lastDescAddr;
    for (i=0;
         i<(MACInfo->txDescCount);
         i++, descAddr += AE531X_QUEUE_ELE_SIZE) {
            AE531X_DESC_STATUS_SET (descAddr, 0);
    }

    /* Set init register values  */
    ae531x_WriteDmaReg(MACInfo, DmaBusMode, DmaBusModeInit);

    /* Install the first Tx and Rx queues on the device */
    ae531x_WriteDmaReg(MACInfo, DmaRxBaseAddr,
		       virt_to_bus((UINT32)MACInfo->rxQueue.firstDescAddr));
    ae531x_WriteDmaReg(MACInfo, DmaTxBaseAddr,
                      virt_to_bus((UINT32)MACInfo->txQueue.firstDescAddr));
    ae531x_WriteDmaReg(MACInfo, DmaControl, DmaStoreAndForward);
    ae531x_WriteDmaReg(MACInfo, DmaIntrEnb, DmaIntDisable);

    AE531X_PRINT(AE531X_DEBUG_RESET,
              ("eth%d: DMA RESET!\n", MACInfo->unit));

    /* Turn on device interrupts -- enable most errors */
    ae531x_DmaIntClear(MACInfo);    /* clear interrupt requests  */
    ae531x_DmaIntEnable(MACInfo);   /* enable interrupts */

    ae531x_EndResetMode(MACInfo);
    intEnable(oldIntrState);

    LEAVE();
}


/******************************************************************************
*
* ae531x_MACAddressSet - Set the ethernet address
*
* Sets the ethernet address from the given address field
*
* RETURNS: void
*/
static void
ae531x_MACAddressSet(ae531x_MAC_t *MACInfo)
{
    unsigned int    data;
    UINT8 *macAddr;

    ARRIVE();
        
    macAddr = macAddrGet(MACInfo);

    /* set our MAC address  */
    data = (macAddr[5]<<8) | macAddr[4];
    ae531x_WriteMacReg(MACInfo, MacAddrHigh, data );

    data = (macAddr[3]<<24) | (macAddr[2]<<16) | (macAddr[1]<<8) | macAddr[0];
    ae531x_WriteMacReg(MACInfo, MacAddrLow, data );

    AE531X_PRINT(AE531X_DEBUG_RESET,
              ("eth%d Verify MAC address %8.8X %8.8X \n",
               MACInfo->unit,
               ae531x_ReadMacReg(MACInfo, MacAddrLow),
               ae531x_ReadMacReg(MACInfo, MacAddrHigh)));

    AE531X_PRINT(AE531X_DEBUG_RESET,
              ("  sb = %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
               0xff&macAddr[0],
               0xff&macAddr[1],
               0xff&macAddr[2],
               0xff&macAddr[3],
               0xff&macAddr[4],
               0xff&macAddr[5]));
    LEAVE();
}


/******************************************************************************
*
* ae_SetMACFromPhy - read Phy settings and update Mac
*                    with current duplex and speed.
*
* RETURNS:
*/
static void
ae531x_SetMACFromPhy(ae531x_MAC_t *MACInfo)
{
    UINT32  macCtl;
    BOOL    fullDuplex;

    ARRIVE();

    /* Get duplex mode from Phy */
    fullDuplex = phyIsFullDuplex(MACInfo->unit);

    /* Flag is set for full duplex mode, else cleared */
    macCtl = ae531x_ReadMacReg(MACInfo, MacControl);

    if (fullDuplex) {
        /* set values of control registers */
        macCtl &= ~MacDisableRxOwn;
        macCtl |= MacFullDuplex;
        ae531x_WriteMacReg(MACInfo, MacControl, macCtl);
        ae531x_WriteMacReg(MACInfo, MacFlowControl, MacFlowControlInitFdx);
    } else {
        /* set values of control registers */
        ae531x_WriteMacReg(MACInfo, MacFlowControl, MacFlowControlInitHdx);
        macCtl |= MacDisableRxOwn;
        macCtl &= ~MacFullDuplex;
        ae531x_WriteMacReg(MACInfo, MacControl, macCtl);
    }

    LEAVE();
}


/******************************************************************************
* ae531x_MACReset -- sets MAC address and duplex.
*/
void
ae531x_MACReset(ae531x_MAC_t *MACInfo)
{
    ae531x_MACAddressSet(MACInfo);

    ae531x_SetMACFromPhy(MACInfo);
}


/******************************************************************************
* ae531x_EnableComm -- enable Transmit and Receive
*/
void
ae531x_EnableComm(ae531x_MAC_t *MACInfo)
{
    ae531x_SetMacReg(MACInfo, MacControl, (MacRxEnable | MacTxEnable));

    ae531x_DmaRxStart(MACInfo);     /* start receiver  */
    ae531x_DmaTxStart(MACInfo);     /* start transmitter */
}


/******************************************************************************
* ae531x_DisableComm -- disable Transmit and Receive
*/
void
ae531x_DisableComm(ae531x_MAC_t *MACInfo)
{
    ae531x_ClearMacReg(MACInfo, MacControl, (MacRxEnable | MacTxEnable));
}


/******************************************************************************
* ae531x_reset -- Cold reset ethernet interface
*/
void
ae531x_reset(ae531x_MAC_t *MACInfo)
{
    UINT32 mask = 0;
    UINT32 regtmp;
   
    if (MACInfo->unit == 0) {
        mask = RESET_ENET0 | RESET_EPHY0;
#ifdef CYGPKG_HAL_MIPS_AR5312
    } else {
        mask = RESET_ENET1 | RESET_EPHY1;
#ifdef TWISTED_ENET_MACS
        mask |= RESET_ENET0 | RESET_EPHY0;
#endif
#endif
    }

#ifdef CYGPKG_HAL_MIPS_AR2316
    /* Enable Arbitration for Ethernet bus */
    regtmp = sysRegRead(AR2316_AHB_ARB_CTL);
    regtmp |= ARB_ETHERNET;
    sysRegWrite(AR2316_AHB_ARB_CTL, regtmp);

    /* Put into reset */
    regtmp = sysRegRead(AR531X_RESET);
    sysRegWrite(AR531X_RESET, regtmp | mask);
    sysMsDelay(10);

    /* Pull out of reset */
    regtmp = sysRegRead(AR531X_RESET);
    sysRegWrite(AR531X_RESET, regtmp & ~mask);
    sysMsDelay(10);

    regtmp = sysRegRead(AR2316_IF_CTL);
    regtmp |= IF_TS_LOCAL;
    sysRegWrite(AR2316_IF_CTL, regtmp);

    /* Enable global swapping so this looks like a normal BE system */
    regtmp = sysRegRead(AR2316_ENDIAN_CTL);
#if 0
    regtmp |= CONFIG_WLAN;
    regtmp |= (CONFIG_PCIAHB | CONFIG_PCIAHB_BRIDGE);
#endif /* 0 */
    regtmp &= ~CONFIG_ETHERNET;
    sysRegWrite(AR2316_ENDIAN_CTL, regtmp);

#if 0
    {
	int timeout;

	/* Wakeup the through the  WMAC Sleep Control register in the 
	   PCI address space */
	regtmp = sysRegRead(PCI_MAC_SCR);
	regtmp = (regtmp & ~PCI_MAC_SCR_SLMODE_M) | (PCI_MAC_SCR_SLM_FWAKE << PCI_MAC_SCR_SLMODE_S);
	sysRegWrite(PCI_MAC_SCR, regtmp);
	timeout = 0x100000;
	while (sysRegRead(PCI_MAC_PCICFG) & PCI_MAC_PCICFG_SPWR_DN) {
	    if (--timeout == 0) break;
	}
    }
#endif /* 0 */
#endif /* CYGPKG_HAL_MIPS_AR2316 */

#ifdef CYGPKG_HAL_MIPS_AR5312
    /* Put into reset */
    regtmp = sysRegRead(AR531X_RESET);
    sysRegWrite(AR531X_RESET, regtmp | mask);
    sysMsDelay(15);

    /* Pull out of reset */
    regtmp = sysRegRead(AR531X_RESET);
    sysRegWrite(AR531X_RESET, regtmp & ~mask);
    sysUDelay(25);

    /* Enable */
    if (MACInfo->unit == 0) {
        mask = ENABLE_ENET0;
    } else {
        mask = ENABLE_ENET1;
#ifdef TWISTED_ENET_MACS
        mask |= ENABLE_ENET0;
#endif /* TWISTED_ENET_MACS */
    }
    regtmp = sysRegRead(AR531X_ENABLE);
    sysRegWrite(AR531X_ENABLE, regtmp | mask);
#endif /* CYGPKG_HAL_MIPS_AR5312 */
}

#if defined(CYGPKG_HAL_MIPS_AR5312) || defined(CYGPKG_HAL_MIPS_AR2312)
/******************************************************************************
 * ae531x_enablePhy1 -- This sets bit 0 of the local configuration
 * register so we can use ethernet 1.
 */

void
ae531x_enablePhy1(ae531x_MAC_t *MACInfo)
{
    UINT32 data;
    
    data = RegRead(AR5312_LOCAL_BUS_CONFIG);
    RegWrite (data & ~AR5312_LBCONFIG_LOCAL_BUS, AR5312_LOCAL_BUS_CONFIG);
}
#endif /* AR5312 || AR2312 */

/******************************************************************************
* ae531x_unitLinkLost -- Called from PHY layer to notify the MAC layer
* that there are no longer any live links associated with a MAC.
*/
void
ae531x_unitLinkLost(int ethUnit)
{
    AE531X_PRINT(AE531X_DEBUG_LINK_CHANGE,
             ("enet%d link down\n", ethUnit));
}


/******************************************************************************
* ae531x_unitLinkGained -- Called from PHY layer to notify the MAC layer
* that there are 1 or more live links associated with a MAC.
*/
void
ae531x_unitLinkGained(int ethUnit)
{
    AE531X_PRINT(AE531X_DEBUG_LINK_CHANGE,
             ("enet%d link up\n", ethUnit));
}
