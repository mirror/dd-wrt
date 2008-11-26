/******************************************************************************
 *
 * Name:        sky2.c
 * Project:     Yukon2 specific functions and implementations
 * Version:     $Revision: 1.38.4.21 $
 * Date:        $Date: 2007/10/15 15:46:03 $
 * Purpose:     The main driver source module
 *
 *****************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect GmbH.
 *	(C)Copyright 2002-2005 Marvell.
 *
 *	Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet 
 *      Server Adapters.
 *
 *	Author: Ralph Roesler (rroesler@syskonnect.de)
 *	        Mirko Lindner (mlindner@syskonnect.de)
 *
 *	Address all question to: linux@syskonnect.de
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	The information in this file is provided "AS IS" without warranty.
 *
 *****************************************************************************/

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include <linux/tcp.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,14)
#include <linux/ip.h>
#endif
/******************************************************************************
 *
 * Local Function Prototypes
 *
 *****************************************************************************/

static void InitPacketQueues(SK_AC *pAC,int Port);
static void GiveTxBufferToHw(SK_AC *pAC,SK_IOC IoC,int Port);
static void GiveRxBufferToHw(SK_AC *pAC,SK_IOC IoC,int Port,SK_PACKET *pPacket);
#ifdef SK_EXTREME
static SK_BOOL HandleReceives(SK_AC *pAC,int Port,SK_U16 Len,SK_U32 FrameStatus,SK_U16 Tcp1,SK_U16 Tcp2,SK_U32 Tist,SK_U16 Vlan, SK_U32 ExtremeCsumResult);
#else
static SK_BOOL HandleReceives(SK_AC *pAC,int Port,SK_U16 Len,SK_U32 FrameStatus,SK_U16 Tcp1,SK_U16 Tcp2,SK_U32 Tist,SK_U16 Vlan);
#endif
static void CheckForSendComplete(SK_AC *pAC,SK_IOC IoC,int Port,SK_PKT_QUEUE *pPQ,SK_LE_TABLE *pLETab,unsigned int Done);
static void UnmapAndFreeTxPktBuffer(SK_AC *pAC,SK_PACKET *pSkPacket,int TxPort);
static SK_BOOL AllocateAndInitLETables(SK_AC *pAC);
static SK_BOOL AllocatePacketBuffersYukon2(SK_AC *pAC);
static void FreeLETables(SK_AC *pAC);
static void FreePacketBuffers(SK_AC *pAC);
static SK_BOOL AllocAndMapRxBuffer(SK_AC *pAC,SK_PACKET *pSkPacket,int Port);
#ifdef CONFIG_SK98LIN_NAPI
static SK_BOOL HandleStatusLEs(SK_AC *pAC,int *WorkDone,int WorkToDo);
#else
static SK_BOOL HandleStatusLEs(SK_AC *pAC);
#endif

extern void	SkGeCheckTimer		(DEV_NET *pNet);
extern void	SkLocalEventQueue(	SK_AC *pAC,
					SK_U32 Class,
					SK_U32 Event,
					SK_U32 Param1,
					SK_U32 Param2,
					SK_BOOL Flag);
extern void	SkLocalEventQueue64(	SK_AC *pAC,
					SK_U32 Class,
					SK_U32 Event,
					SK_U64 Param,
					SK_BOOL Flag);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,19)
/* Need way to schedule device 0 even when it's offline. */
static inline int __netif_rx_schedule_prep(struct net_device *dev)
{
	return !test_and_set_bit(__LINK_STATE_RX_SCHED, &dev->state);
}
#endif
#endif


/******************************************************************************
 *
 * Local Variables
 *
 *****************************************************************************/

#define MAX_NBR_RX_BUFFERS_IN_HW	0x15
static SK_U8 NbrRxBuffersInHW;
#define FLUSH_OPC(le)

/******************************************************************************
 *
 * Global Functions
 *
 *****************************************************************************/

int SkY2Xmit( struct sk_buff *skb, struct SK_NET_DEVICE *dev); 
void FillReceiveTableYukon2(SK_AC *pAC,SK_IOC IoC,int Port);

/*****************************************************************************
 *
 * 	SkY2RestartStatusUnit - restarts teh status unit
 *
 * Description:
 *	Reenables the status unit after any De-Init (e.g. when altering 
 *	the sie of the MTU via 'ifconfig a.b.c.d mtu xxx')
 *
 * Returns:	N/A
 */
void SkY2RestartStatusUnit(
SK_AC  *pAC)  /* pointer to adapter control context */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("==> SkY2RestartStatusUnit\n"));

	/*
	** It might be that the TX timer is not started. Therefore
	** it is initialized here -> to be more investigated!
	*/
	SK_OUT32(pAC->IoBase, STAT_TX_TIMER_INI, HW_MS_TO_TICKS(pAC,10));

	pAC->StatusLETable.Done  = 0;
	pAC->StatusLETable.Put   = 0;
	pAC->StatusLETable.HwPut = 0;
	SkGeY2InitStatBmu(pAC, pAC->IoBase, &pAC->StatusLETable);

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("<== SkY2RestartStatusUnit\n"));
}

/*****************************************************************************
 *
 * 	SkY2RlmtSend - sends out a single RLMT notification
 *
 * Description:
 *	This function sends out an RLMT frame
 *
 * Returns:	
 *	> 0 - on succes: the number of bytes in the message
 *	= 0 - on resource shortage: this frame sent or dropped, now
 *	      the ring is full ( -> set tbusy)
 *	< 0 - on failure: other problems ( -> return failure to upper layers)
 */
int SkY2RlmtSend (
SK_AC          *pAC,       /* pointer to adapter control context           */
int             PortNr,    /* index of port the packet(s) shall be send to */
struct sk_buff *pMessage)  /* pointer to send-message                      */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("=== SkY2RlmtSend\n"));
#if 0
	return -1;   // temporarily do not send out RLMT frames
#endif
	skb_shinfo(pMessage)->nr_frags = (2*MAX_SKB_FRAGS) + PortNr;
	return(SkY2Xmit(pMessage, pAC->dev[PortNr])); // SkY2Xmit needs device
}

/*****************************************************************************
 *
 * 	SkY2AllocateResources - Allocates all required resources for Yukon2
 *
 * Description:
 *	This function allocates all memory needed for the Yukon2. 
 *	It maps also RX buffers to the LETables and initializes the
 *	status list element table.
 *
 * Returns:	
 *	SK_TRUE, if all resources could be allocated and setup succeeded
 *	SK_FALSE, if an error 
 */
SK_BOOL SkY2AllocateResources (
SK_AC  *pAC)  /* pointer to adapter control context */
{
	int CurrMac;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("==> SkY2AllocateResources\n"));

	/*
	** Initialize the packet queue variables first
	*/
	for (CurrMac = 0; CurrMac < pAC->GIni.GIMacsFound; CurrMac++) {
		InitPacketQueues(pAC, CurrMac);
	}

	/* 
	** Get sufficient memory for the LETables
	*/
	if (!AllocateAndInitLETables(pAC)) {
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
			SK_DBGCAT_INIT | SK_DBGCAT_DRV_ERROR,
			("No memory for LETable.\n"));
		return(SK_FALSE);
	}

	/*
	** Allocate and intialize memory for both RX and TX 
	** packet and fragment buffers. On an error, free 
	** previously allocated LETable memory and quit.
	*/
	if (!AllocatePacketBuffersYukon2(pAC)) {
		FreeLETables(pAC);
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
			SK_DBGCAT_INIT | SK_DBGCAT_DRV_ERROR,
			("No memory for Packetbuffers.\n"));
		return(SK_FALSE);
	}

	/* 
	** Rx and Tx LE tables will be initialized in SkGeOpen() 
	**
	** It might be that the TX timer is not started. Therefore
	** it is initialized here -> to be more investigated!
	*/
	SK_OUT32(pAC->IoBase, STAT_TX_TIMER_INI, HW_MS_TO_TICKS(pAC,10));
	SkGeY2InitStatBmu(pAC, pAC->IoBase, &pAC->StatusLETable);

	pAC->MaxUnusedRxLeWorking = MAX_UNUSED_RX_LE_WORKING;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("<== SkY2AllocateResources\n"));

	return (SK_TRUE);
}

/*****************************************************************************
 *
 * 	SkY2FreeResources - Frees previously allocated resources of Yukon2
 *
 * Description:
 *	This function frees all previously allocated memory of the Yukon2. 
 *
 * Returns: N/A
 */
void SkY2FreeResources (
SK_AC  *pAC)  /* pointer to adapter control context */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("==> SkY2FreeResources\n"));

	FreeLETables(pAC);
	FreePacketBuffers(pAC);

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("<== SkY2FreeResources\n"));
}

/*****************************************************************************
 *
 * 	SkY2AllocateRxBuffers - Allocates the receive buffers for a port
 *
 * Description:
 *	This function allocated all the RX buffers of the Yukon2. 
 *
 * Returns: N/A
 */
void SkY2AllocateRxBuffers (
SK_AC    *pAC,   /* pointer to adapter control context */
SK_IOC    IoC,	 /* I/O control context                */
int       Port)	 /* port index of RX                   */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("==> SkY2AllocateRxBuffers (Port %c)\n", Port));

	FillReceiveTableYukon2(pAC, IoC, Port);

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("<== SkY2AllocateRxBuffers\n"));
}

/*****************************************************************************
 *
 * 	SkY2FreeRxBuffers - Free's all allocates RX buffers of
 *
 * Description:
 *	This function frees all RX buffers of the Yukon2 for a single port
 *
 * Returns: N/A
 */
void SkY2FreeRxBuffers (
SK_AC    *pAC,   /* pointer to adapter control context */
SK_IOC    IoC,	 /* I/O control context                */
int       Port)	 /* port index of RX                   */
{
	SK_PACKET     *pSkPacket;
	unsigned long  Flags;   /* for POP/PUSH macros */

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("==> SkY2FreeRxBuffers (Port %c)\n", Port));

	if (pAC->RxPort[Port].ReceivePacketTable   != NULL) {
		POP_FIRST_PKT_FROM_QUEUE(&pAC->RxPort[Port].RxQ_working, pSkPacket);
		while (pSkPacket != NULL) {
			if ((pSkPacket->pFrag) != NULL) {
				pci_unmap_page(pAC->PciDev,
				(dma_addr_t) pSkPacket->pFrag->pPhys,
				pSkPacket->pFrag->FragLen - 2,
				PCI_DMA_FROMDEVICE);

				/* wipe out any rubbish data that may interfere */
				skb_shinfo(pSkPacket->pMBuf)->nr_frags = 0;
				skb_shinfo(pSkPacket->pMBuf)->frag_list = NULL;
				DEV_KFREE_SKB_ANY(pSkPacket->pMBuf);
				pSkPacket->pMBuf        = NULL;
				pSkPacket->pFrag->pPhys = (SK_U64) 0;
				pSkPacket->pFrag->pVirt = NULL;
			}
			PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pSkPacket);
			POP_FIRST_PKT_FROM_QUEUE(&pAC->RxPort[Port].RxQ_working, pSkPacket);
		}
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("<== SkY2FreeRxBuffers\n"));
}

/*****************************************************************************
 *
 * 	SkY2FreeTxBuffers - Free's any currently maintained Tx buffer
 *
 * Description:
 *	This function frees the TX buffers of the Yukon2 for a single port
 *	which might be in use by a transmit action
 *
 * Returns: N/A
 */
void SkY2FreeTxBuffers (
SK_AC    *pAC,   /* pointer to adapter control context */
SK_IOC    IoC,	 /* I/O control context                */
int       Port)	 /* port index of TX                   */
{
	SK_PACKET      *pSkPacket;
	SK_FRAG        *pSkFrag;
	unsigned long   Flags;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("==> SkY2FreeTxBuffers (Port %c)\n", Port));
 
	if (pAC->TxPort[Port][0].TransmitPacketTable != NULL) {
		POP_FIRST_PKT_FROM_QUEUE(&pAC->TxPort[Port][0].TxAQ_working, pSkPacket);
		while (pSkPacket != NULL) {
			if ((pSkFrag = pSkPacket->pFrag) != NULL) {
				UnmapAndFreeTxPktBuffer(pAC, pSkPacket, Port);
			}
			PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->TxPort[Port][0].TxQ_free, pSkPacket);
			POP_FIRST_PKT_FROM_QUEUE(&pAC->TxPort[Port][0].TxAQ_working, pSkPacket);
		}
#ifdef USE_SYNC_TX_QUEUE
		POP_FIRST_PKT_FROM_QUEUE(&pAC->TxPort[Port][0].TxSQ_working, pSkPacket);
		while (pSkPacket != NULL) {
			if ((pSkFrag = pSkPacket->pFrag) != NULL) {
				UnmapAndFreeTxPktBuffer(pAC, pSkPacket, Port);
			}
			PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->TxPort[Port][0].TxQ_free, pSkPacket);
			POP_FIRST_PKT_FROM_QUEUE(&pAC->TxPort[Port][0].TxSQ_working, pSkPacket);
		}
#endif
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("<== SkY2FreeTxBuffers\n"));
}

/*****************************************************************************
 *
 * 	SkY2Isr - handle a receive IRQ for all yukon2 cards
 *
 * Description:
 *	This function is called when a receive IRQ is set. (only for yukon2)
 *	HandleReceives does the deferred processing of all outstanding
 *	interrupt operations.
 *
 * Returns:	N/A
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
SkIsrRetVar SkY2Isr (
int              irq,     /* the irq we have received (might be shared!) */
void            *dev_id)  /* current device id                           */
#else
SkIsrRetVar SkY2Isr (
int              irq,     /* the irq we have received (might be shared!) */
void            *dev_id,  /* current device id                           */
struct  pt_regs *ptregs)  /* not used by our driver                      */
#endif
{
	struct SK_NET_DEVICE  *dev  = (struct SK_NET_DEVICE *)dev_id;
	DEV_NET               *pNet = (DEV_NET*) dev->priv;
	SK_AC                 *pAC  = pNet->pAC;
#ifndef CONFIG_SK98LIN_NAPI
	SK_BOOL                handledStatLE	= SK_FALSE;
	unsigned long          Flags;
#endif

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
		("==> SkY2Isr\n"));

	SK_IN32(pAC->IoBase, B0_Y2_SP_ISRC2, &pAC->InterruptSource);

	if ((pAC->InterruptSource == 0) && (!pNet->NetConsoleMode)){
		SK_OUT32(pAC->IoBase, B0_Y2_SP_ICR, 2);
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
			("No Interrupt\n ==> SkY2Isr\n"));
		return SkIsrRetNone;

	}

#ifdef Y2_RECOVERY
	if (pNet->InRecover) {
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
			("Already in recover\n ==> SkY2Isr\n"));
		SK_OUT32(pAC->IoBase, B0_Y2_SP_ICR, 2);
		return SkIsrRetNone;
	}
#endif

#ifdef CONFIG_SK98LIN_NAPI
	/* Since both boards share one irq, they share one poll routine */
	if (__netif_rx_schedule_prep(pAC->dev[0])) {
		__netif_rx_schedule(pAC->dev[0]);
	}
#else
	handledStatLE = HandleStatusLEs(pAC);

	/* 
	** Check for Special Interrupts 
	*/
	if ((pAC->InterruptSource & ~Y2_IS_STAT_BMU) || pAC->CheckQueue || pNet->TimerExpired) {
		pAC->CheckQueue = SK_FALSE;
		spin_lock_irqsave(&pAC->SetPutIndexLock, Flags);
		SkGeSirqIsr(pAC, pAC->IoBase, pAC->InterruptSource);
		SkEventDispatcher(pAC, pAC->IoBase);
		spin_unlock_irqrestore(&pAC->SetPutIndexLock, Flags);
	}

	/* Speed enhancement for a2 chipsets */
	if (HW_FEATURE(pAC, HWF_WA_DEV_42)) {
		spin_lock_irqsave(&pAC->SetPutIndexLock, Flags);
		SkGeY2SetPutIndex(pAC, pAC->IoBase, Y2_PREF_Q_ADDR(Q_XA1,0), &pAC->TxPort[0][0].TxALET);
		SkGeY2SetPutIndex(pAC, pAC->IoBase, Y2_PREF_Q_ADDR(Q_R1,0), &pAC->RxPort[0].RxLET);
		spin_unlock_irqrestore(&pAC->SetPutIndexLock, Flags);
	}

	/* 
	** Reenable interrupts and signal end of ISR 
	*/
	SK_OUT32(pAC->IoBase, B0_Y2_SP_ICR, 2);

	/*
	** Stop and restart TX timer in case a Status LE was handled
	*/
	if ((HW_FEATURE(pAC, HWF_WA_DEV_43_418)) && (handledStatLE)) {
		SK_OUT8(pAC->IoBase, STAT_TX_TIMER_CTRL, TIM_STOP);
		SK_OUT8(pAC->IoBase, STAT_TX_TIMER_CTRL, TIM_START);
	}
	if (!(IS_Q_EMPTY(&(pAC->TxPort[0][TX_PRIO_LOW].TxAQ_waiting)))) {
		GiveTxBufferToHw(pAC, pAC->IoBase, 0);
	}
	if (!(IS_Q_EMPTY(&(pAC->TxPort[1][TX_PRIO_LOW].TxAQ_waiting)))) {
		GiveTxBufferToHw(pAC, pAC->IoBase, 1);
	}
#endif

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
		("<== SkY2Isr\n"));

	return SkIsrRetHandled;
}	/* SkY2Isr */

/*****************************************************************************
 *
 *	SkY2Xmit - Linux frame transmit function for Yukon2
 *
 * Description:
 *	The system calls this function to send frames onto the wire.
 *	It puts the frame in the tx descriptor ring. If the ring is
 *	full then, the 'tbusy' flag is set.
 *
 * Returns:
 *	0, if everything is ok
 *	!=0, on error
 *
 * WARNING: 
 *	returning 1 in 'tbusy' case caused system crashes (double
 *	allocated skb's) !!!
 */
int SkY2Xmit(
struct sk_buff       *skb,  /* socket buffer to be sent */
struct SK_NET_DEVICE *dev)  /* via which device?        */
{
	DEV_NET         *pNet    = (DEV_NET*) dev->priv;
	SK_AC           *pAC     = pNet->pAC;
	SK_U8            FragIdx = 0;
	SK_PACKET       *pSkPacket;
	SK_FRAG         *PrevFrag;
	SK_FRAG         *CurrFrag;
	SK_PKT_QUEUE    *pWorkQueue;  /* corresponding TX queue */
	SK_PKT_QUEUE    *pWaitQueue; 
	SK_PKT_QUEUE    *pFreeQueue; 
	SK_LE_TABLE     *pLETab;      /* corresponding LETable  */ 
	skb_frag_t      *sk_frag;
	SK_U64           PhysAddr;
	unsigned long    Flags;
	unsigned int     Port;
	int              CurrFragCtr;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
		("==> SkY2Xmit\n"));

	/*
	** Get port and return if no free packet is available 
	*/
	if (skb_shinfo(skb)->nr_frags > MAX_SKB_FRAGS) {
		Port = skb_shinfo(skb)->nr_frags - (2*MAX_SKB_FRAGS);
		skb_shinfo(skb)->nr_frags = 0;
	} else {
		Port = (pAC->RlmtNets == 2) ? pNet->PortNr : pAC->ActivePort;
	}

	if (IS_Q_EMPTY(&(pAC->TxPort[Port][TX_PRIO_LOW].TxQ_free))) {
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
			SK_DBGCAT_DRV_TX_PROGRESS | SK_DBGCAT_DRV_ERROR,
			("Not free packets available for send\n"));
		return 1; /* zero bytes sent! */
	}

	/*
	** Put any new packet to be sent in the waiting queue and 
	** handle also any possible fragment of that packet.
	*/
	pWorkQueue = &(pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_working);
	pWaitQueue = &(pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_waiting);
	pFreeQueue = &(pAC->TxPort[Port][TX_PRIO_LOW].TxQ_free);
	pLETab     = &(pAC->TxPort[Port][TX_PRIO_LOW].TxALET);

	/*
	** Normal send operations require only one fragment, because 
	** only one sk_buff data area is passed. 
	** In contradiction to this, scatter-gather (zerocopy) send
	** operations might pass one or more additional fragments 
	** where each fragment needs a separate fragment info packet.
	*/
	if (((skb_shinfo(skb)->nr_frags + 1) * MAX_FRAG_OVERHEAD) > 
					NUM_FREE_LE_IN_TABLE(pLETab)) {
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
			SK_DBGCAT_DRV_TX_PROGRESS | SK_DBGCAT_DRV_ERROR,
			("Not enough LE available for send\n"));
		return 1; /* zero bytes sent! */
	}
	
	if ((skb_shinfo(skb)->nr_frags + 1) > MAX_NUM_FRAGS) {
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
			SK_DBGCAT_DRV_TX_PROGRESS | SK_DBGCAT_DRV_ERROR,
			("Not even one fragment available for send\n"));
		return 1; /* zero bytes sent! */
	}

	/*
	** Get first packet from free packet queue
	*/
	POP_FIRST_PKT_FROM_QUEUE(pFreeQueue, pSkPacket);
	if(pSkPacket == NULL) {
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
			SK_DBGCAT_DRV_TX_PROGRESS | SK_DBGCAT_DRV_ERROR,
			("Could not obtain free packet used for xmit\n"));
		return 1; /* zero bytes sent! */
	}

	pSkPacket->pFrag = &(pSkPacket->FragArray[FragIdx]);

	/* 
	** map the sk_buff to be available for the adapter 
	*/
	PhysAddr = (SK_U64) pci_map_page(pAC->PciDev,
			virt_to_page(skb->data),
			((unsigned long) skb->data & ~PAGE_MASK),
			skb_headlen(skb),
			PCI_DMA_TODEVICE);
	pSkPacket->pMBuf	  = skb;
	pSkPacket->pFrag->pPhys   = PhysAddr;
	pSkPacket->pFrag->FragLen = skb_headlen(skb);
	pSkPacket->pFrag->pNext   = NULL; /* initial has no next default */
	pSkPacket->NumFrags	  = skb_shinfo(skb)->nr_frags + 1;

	PrevFrag = pSkPacket->pFrag;

	/*
	** Each scatter-gather fragment need to be mapped...
	*/
        for (	CurrFragCtr = 0; 
		CurrFragCtr < skb_shinfo(skb)->nr_frags;
		CurrFragCtr++) {
		FragIdx++;
		sk_frag = &skb_shinfo(skb)->frags[CurrFragCtr];
		CurrFrag = &(pSkPacket->FragArray[FragIdx]);

		/* 
		** map the sk_buff to be available for the adapter 
		*/
		PhysAddr = (SK_U64) pci_map_page(pAC->PciDev,
				sk_frag->page,
		 		sk_frag->page_offset,
		 		sk_frag->size,
		 		PCI_DMA_TODEVICE);

		CurrFrag->pPhys   = PhysAddr;
 		CurrFrag->FragLen = sk_frag->size;
 		CurrFrag->pNext   = NULL;

		/*
		** Add the new fragment to the list of fragments
		*/
		PrevFrag->pNext = CurrFrag;
		PrevFrag = CurrFrag;
	}

	/* 
	** Add packet to waiting packets queue 
	*/
	PUSH_PKT_AS_LAST_IN_QUEUE(pWaitQueue, pSkPacket);
	GiveTxBufferToHw(pAC, pAC->IoBase, Port);
	dev->trans_start = jiffies;
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
		("<== SkY2Xmit(return 0)\n"));
	return (0);
}	/* SkY2Xmit */

#ifdef CONFIG_SK98LIN_NAPI
/*****************************************************************************
 *
 *	SkY2Poll - NAPI Rx polling callback for Yukon2 chipsets
 *
 * Description:
 *	Called by the Linux system in case NAPI polling is activated
 *
 * Returns
 *	The number of work data still to be handled
 *
 * Notes
 *	The slowpath lock needs to be set because HW accesses may
 *	interfere with slowpath events (e.g. TWSI)
 */
int SkY2Poll(
struct net_device *dev,     /* device that needs to be polled */
int               *budget)  /* how many budget do we have?    */
{
	DEV_NET       *pNet         = (DEV_NET*) dev->priv;
	SK_AC	      *pAC          = pNet->pAC;
	int           WorkToDo      = min(*budget, dev->quota);
	int           WorkDone      = 0;
	SK_BOOL       handledStatLE = SK_FALSE;

	handledStatLE = HandleStatusLEs(pAC, &WorkDone, WorkToDo);

	*budget -= WorkDone;
	dev->quota -= WorkDone;

	/* 
	** Check for Special Interrupts 
	*/
	if ((pAC->InterruptSource & ~Y2_IS_STAT_BMU) || pAC->CheckQueue || pNet->TimerExpired) {
		pAC->CheckQueue = SK_FALSE;
		spin_lock(&pAC->SlowPathLock);
		SkGeSirqIsr(pAC, pAC->IoBase, pAC->InterruptSource);
		SkEventDispatcher(pAC, pAC->IoBase);
		spin_unlock(&pAC->SlowPathLock);
	}

	/* Speed enhancement for a2 chipsets */
	if (HW_FEATURE(pAC, HWF_WA_DEV_42)) {
		spin_lock(&pAC->SlowPathLock);
		SkGeY2SetPutIndex(pAC, pAC->IoBase, Y2_PREF_Q_ADDR(Q_XA1,0), &pAC->TxPort[0][0].TxALET);
		SkGeY2SetPutIndex(pAC, pAC->IoBase, Y2_PREF_Q_ADDR(Q_R1,0), &pAC->RxPort[0].RxLET);
		spin_unlock(&pAC->SlowPathLock);
	}

	if(WorkDone < WorkToDo) {
		netif_rx_complete(dev);

		/*
		** Stop and restart TX timer in case a Status LE was handled
		*/
		if ((HW_FEATURE(pAC, HWF_WA_DEV_43_418)) && (handledStatLE)) {
			SK_OUT8(pAC->IoBase, STAT_TX_TIMER_CTRL, TIM_STOP);
			SK_OUT8(pAC->IoBase, STAT_TX_TIMER_CTRL, TIM_START);
		}

		/* 
		** Reenable interrupts
		*/
		SK_OUT32(pAC->IoBase, B0_Y2_SP_ICR, 2);

		if (!(IS_Q_EMPTY(&(pAC->TxPort[0][TX_PRIO_LOW].TxAQ_waiting)))) {
			GiveTxBufferToHw(pAC, pAC->IoBase, 0);
		}
		if (!(IS_Q_EMPTY(&(pAC->TxPort[1][TX_PRIO_LOW].TxAQ_waiting)))) {
			GiveTxBufferToHw(pAC, pAC->IoBase, 1);
		}
	}

	return (WorkDone >= WorkToDo);
}	/* SkY2Poll */
#endif

/******************************************************************************
 *
 *	SkY2PortStop - stop a port on Yukon2
 *
 * Description:
 *	This function stops a port of the Yukon2 chip. This stop 
 *	stop needs to be performed in a specific order:
 * 
 *	a) Stop the Prefetch unit
 *	b) Stop the Port (MAC, PHY etc.)
 *
 * Returns: N/A
 */
void SkY2PortStop(
SK_AC   *pAC,      /* adapter control context                             */
SK_IOC   IoC,      /* I/O control context (address of adapter registers)  */
int      Port,     /* port to stop (MAC_1 + n)                            */
int      Dir,      /* StopDirection (SK_STOP_RX, SK_STOP_TX, SK_STOP_ALL) */
int      RstMode)  /* Reset Mode (SK_SOFT_RST, SK_HARD_RST)               */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("==> SkY2PortStop (Port %c)\n", 'A' + Port));

	/*
	** Stop the HW
	*/
	SkGeStopPort(pAC, IoC, Port, Dir, RstMode);

	/*
	** Move any TX packet from work queues into the free queue again
	** and initialize the TX LETable variables
	*/
	SkY2FreeTxBuffers(pAC, pAC->IoBase, Port);
	pAC->TxPort[Port][TX_PRIO_LOW].TxALET.Bmu.RxTx.TcpWp    = 0;
	pAC->TxPort[Port][TX_PRIO_LOW].TxALET.Bmu.RxTx.MssValue = 0;
	pAC->TxPort[Port][TX_PRIO_LOW].TxALET.BufHighAddr       = 0;
	pAC->TxPort[Port][TX_PRIO_LOW].TxALET.Done              = 0;    
	pAC->TxPort[Port][TX_PRIO_LOW].TxALET.Put               = 0;
#ifndef USE_ASF_DASH_FW
	pAC->GIni.GP[Port].PState = SK_PRT_STOP;
#endif
	/*
	** Move any RX packet from work queue into the waiting queue
	** and initialize the RX LETable variables
	*/
	SkY2FreeRxBuffers(pAC, pAC->IoBase, Port);
	pAC->RxPort[Port].RxLET.BufHighAddr = 0;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("<== SkY2PortStop()\n"));
}

/******************************************************************************
 *
 *	SkY2PortStart - start a port on Yukon2
 *
 * Description:
 *	This function starts a port of the Yukon2 chip. This start 
 *	action needs to be performed in a specific order:
 * 
 *	a) Initialize the LET indices (PUT/GET to 0)
 *	b) Initialize the LET in HW (enables also prefetch unit)
 *	c) Move all RX buffers from waiting queue to working queue
 *	   which involves also setting up of RX list elements
 *	d) Initialize the FIFO settings of Yukon2 (Watermark etc.)
 *	e) Initialize the Port (MAC, PHY etc.)
 *	f) Initialize the MC addresses
 *
 * Returns:	N/A
 */
void SkY2PortStart(
SK_AC   *pAC,   /* adapter control context                            */
SK_IOC   IoC,   /* I/O control context (address of adapter registers) */
int      Port)  /* port to start                                      */
{
	// SK_GEPORT *pPrt = &pAC->GIni.GP[Port];
	SK_HWLE   *pLE;
	SK_U32     DWord;
	SK_U32     PrefetchReg; /* register for Put index */

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("==> SkY2PortStart (Port %c)\n", 'A' + Port));

	/*
	** Initialize the LET indices
	*/
	pAC->RxPort[Port].RxLET.Done                = 0; 
	pAC->RxPort[Port].RxLET.Put                 = 0;
	pAC->RxPort[Port].RxLET.HwPut               = 0;
	pAC->TxPort[Port][TX_PRIO_LOW].TxALET.Done  = 0;    
	pAC->TxPort[Port][TX_PRIO_LOW].TxALET.Put   = 0;
	pAC->TxPort[Port][TX_PRIO_LOW].TxALET.HwPut = 0;
	if (HW_SYNC_TX_SUPPORTED(pAC)) {
		pAC->TxPort[Port][TX_PRIO_LOW].TxSLET.Done  = 0;    
		pAC->TxPort[Port][TX_PRIO_LOW].TxSLET.Put   = 0;
		pAC->TxPort[Port][TX_PRIO_LOW].TxSLET.HwPut = 0;
	}
	
	if (HW_FEATURE(pAC, HWF_WA_DEV_420)) {
		/*
		** It might be that we have to limit the RX buffers 
		** effectively passed to HW. Initialize the start
		** value in that case...
		*/
		NbrRxBuffersInHW = 0;
	}

	/*
	** TODO on dual net adapters we need to check if
	** StatusLETable need to be set...
	** 
	** pAC->StatusLETable.Done  = 0;
	** pAC->StatusLETable.Put   = 0;
	** pAC->StatusLETable.HwPut = 0;
	** SkGeY2InitPrefetchUnit(pAC, pAC->IoBase, Q_ST, &pAC->StatusLETable);
	*/

	/*
	** Initialize the LET in HW (enables also prefetch unit)
	*/
	SkGeY2InitPrefetchUnit(pAC, IoC,(Port == 0) ? Q_R1 : Q_R2,
			&pAC->RxPort[Port].RxLET);
	SkGeY2InitPrefetchUnit( pAC, IoC,(Port == 0) ? Q_XA1 : Q_XA2, 
			&pAC->TxPort[Port][TX_PRIO_LOW].TxALET);
	if (HW_SYNC_TX_SUPPORTED(pAC)) {
		SkGeY2InitPrefetchUnit( pAC, IoC, (Port == 0) ? Q_XS1 : Q_XS2,
				&pAC->TxPort[Port][TX_PRIO_HIGH].TxSLET);
	}


	/*
	** Using new values for the watermarks and the timer for
	** low latency optimization
	*/
	if (pAC->LowLatency) {
		SK_OUT8(IoC, STAT_FIFO_WM, 1);
		SK_OUT8(IoC, STAT_FIFO_ISR_WM, 1);
		SK_OUT32(IoC, STAT_LEV_TIMER_INI, 50);
		SK_OUT32(IoC, STAT_ISR_TIMER_INI, 10);
	}


	/*
	** Initialize the Port (MAC, PHY etc.)
	*/
	if (SkGeInitPort(pAC, IoC, Port)) {
		if (Port == 0) {
			printk("%s: SkGeInitPort A failed.\n",pAC->dev[0]->name);
		} else {
			printk("%s: SkGeInitPort B failed.\n",pAC->dev[1]->name);
		}
	}
	
	if (IS_GMAC(pAC)) {
		/* disable Rx GMAC FIFO Flush Mode */
		SK_OUT8(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), (SK_U8) GMF_RX_F_FL_OFF);
	}

	/*
	** Initialize the MC addresses
	*/
	SkAddrMcUpdate(pAC,IoC, Port);

	SkMacRxTxEnable(pAC, IoC,Port);
				
	SkGeRxCsum(pAC, IoC, Port, SK_TRUE);
	
	GET_RX_LE(pLE, &pAC->RxPort[Port].RxLET);
	RXLE_SET_STACS1(pLE, pAC->CsOfs1);
	RXLE_SET_STACS2(pLE, pAC->CsOfs2);
	RXLE_SET_CTRL(pLE, 0);

	RXLE_SET_OPC(pLE, OP_TCPSTART | HW_OWNER);
	FLUSH_OPC(pLE);
	if (Port == 0) {
		PrefetchReg=Y2_PREF_Q_ADDR(Q_R1,PREF_UNIT_PUT_IDX_REG);
	} else {
		PrefetchReg=Y2_PREF_Q_ADDR(Q_R2,PREF_UNIT_PUT_IDX_REG);
	}
	DWord = GET_PUT_IDX(&pAC->RxPort[Port].RxLET);
	SK_OUT32(IoC, PrefetchReg, DWord);
	UPDATE_HWPUT_IDX(&pAC->RxPort[Port].RxLET);

	pAC->GIni.GP[Port].PState = SK_PRT_RUN;
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("<== SkY2PortStart()\n"));
}

/******************************************************************************
 *
 * Local Functions
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *	InitPacketQueues - initialize SW settings of packet queues
 *
 * Description:
 *	This function will initialize the packet queues for a port.
 *
 * Returns: N/A
 */
static void InitPacketQueues(
SK_AC  *pAC,   /* pointer to adapter control context */
int     Port)  /* index of port to be initialized    */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("==> InitPacketQueues(Port %c)\n", 'A' + Port));
	
	pAC->RxPort[Port].RxQ_working.pHead = NULL;
	pAC->RxPort[Port].RxQ_working.pTail = NULL;
	spin_lock_init(&pAC->RxPort[Port].RxQ_working.QueueLock);
	
	pAC->RxPort[Port].RxQ_waiting.pHead = NULL;
	pAC->RxPort[Port].RxQ_waiting.pTail = NULL;
	spin_lock_init(&pAC->RxPort[Port].RxQ_waiting.QueueLock);
	
	pAC->TxPort[Port][TX_PRIO_LOW].TxQ_free.pHead = NULL;
	pAC->TxPort[Port][TX_PRIO_LOW].TxQ_free.pTail = NULL;
	spin_lock_init(&pAC->TxPort[Port][TX_PRIO_LOW].TxQ_free.QueueLock);

	pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_working.pHead = NULL;
	pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_working.pTail = NULL;
	spin_lock_init(&pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_working.QueueLock);
	
	pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_waiting.pHead = NULL;
	pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_waiting.pTail = NULL;
	spin_lock_init(&pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_waiting.QueueLock);
	
#ifdef USE_SYNC_TX_QUEUE
	pAC->TxPort[Port][TX_PRIO_LOW].TxSQ_working.pHead = NULL;
	pAC->TxPort[Port][TX_PRIO_LOW].TxSQ_working.pTail = NULL;
	spin_lock_init(&pAC->TxPort[Port][TX_PRIO_LOW].TxSQ_working.QueueLock);

	pAC->TxPort[Port][TX_PRIO_LOW].TxSQ_waiting.pHead = NULL;
	pAC->TxPort[Port][TX_PRIO_LOW].TxSQ_waiting.pTail = NULL;
	spin_lock_init(&pAC->TxPort[Port][TX_PRIO_LOW].TxSQ_waiting.QueueLock);
#endif
	
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("<== InitPacketQueues(Port %c)\n", 'A' + Port));
}	/* InitPacketQueues */

/*****************************************************************************
 *
 *	GiveTxBufferToHw - commits a previously allocated DMA area to HW
 *
 * Description:
 *	This functions gives transmit buffers to HW. If no list elements
 *	are available the buffers will be queued. 
 *
 * Notes:
 *       This function can run only once in a system at one time.
 *
 * Returns: N/A
 */
static void GiveTxBufferToHw(
SK_AC   *pAC,   /* pointer to adapter control context         */
SK_IOC   IoC,   /* I/O control context (address of registers) */
int      Port)  /* port index for which the buffer is used    */
{
	SK_HWLE         *pLE;
	SK_PACKET       *pSkPacket;
	SK_FRAG         *pFrag;
	SK_PKT_QUEUE    *pWorkQueue;   /* corresponding TX queue */
	SK_PKT_QUEUE    *pWaitQueue; 
	SK_LE_TABLE     *pLETab;       /* corresponding LETable  */ 
	SK_BOOL          SetOpcodePacketFlag;
	SK_U32           HighAddress;
	SK_U32           LowAddress;
	SK_U16           TcpSumStart = 0; 
	SK_U16           TcpSumWrite = 0;
	SK_U8            OpCode;
	SK_U8            Ctrl;
	unsigned long    Flags;
	unsigned long    LockFlag;
	int              Protocol;
#ifdef NETIF_F_TSO
	SK_U16           Mss;
	int              TcpOptLen;
	int              IpTcpLen;
#endif

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
		("==> GiveTxBufferToHw\n"));

	if (HW_FEATURE(pAC, HWF_WA_DEV_510)) {
		SK_OUT32(pAC->IoBase, TBMU_TEST, TBMU_TEST_BMU_TX_CHK_AUTO_OFF);
	}

	if (IS_Q_EMPTY(&(pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_waiting))) {
		return;
	}

	spin_lock_irqsave(&pAC->TxQueueLock, LockFlag);

	/*
	** Initialize queue settings
	*/
	pWorkQueue = &(pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_working);
	pWaitQueue = &(pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_waiting);
	pLETab     = &(pAC->TxPort[Port][TX_PRIO_LOW].TxALET);

	POP_FIRST_PKT_FROM_QUEUE(pWaitQueue, pSkPacket);
	while (pSkPacket != NULL) {
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
			("\tWe have a packet to send %p\n", pSkPacket));

		/* 
		** the first frag of a packet gets opcode OP_PACKET 
		*/
		SetOpcodePacketFlag	= SK_TRUE;
		pFrag			= pSkPacket->pFrag;

		/* 
		** fill list elements with data from fragments 
		*/
		while (pFrag != NULL) {
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
				("\tGet LE\n"));
#ifdef NETIF_F_TSO

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
			Mss = skb_shinfo(pSkPacket->pMBuf)->gso_size;
#else
			Mss = skb_shinfo(pSkPacket->pMBuf)->tso_size;
#endif

			if (Mss) {
#ifdef SK_EXTREME
				if (!HW_IS_EXT_LE_FORMAT(pAC)) {
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
				       struct iphdr *iph = ip_hdr(pSkPacket->pMBuf);
					struct tcphdr *tcph = tcp_hdr(pSkPacket->pMBuf);
					
					TcpOptLen = ((tcph->doff - 5) * 4);
					IpTcpLen  = ((iph->ihl * 4) + sizeof(struct tcphdr));
#else
					TcpOptLen = ((pSkPacket->pMBuf->h.th->doff - 5) * 4);
					IpTcpLen  = ((pSkPacket->pMBuf->nh.iph->ihl * 4) + 
						sizeof(struct tcphdr));
#endif
					Mss += (TcpOptLen + IpTcpLen + C_LEN_ETHERMAC_HEADER);
#ifdef SK_EXTREME
				}
#endif
			}
			if (pLETab->Bmu.RxTx.MssValue != Mss) {
				pLETab->Bmu.RxTx.MssValue = Mss;
				/* Take a new LE for TSO from the table */
				GET_TX_LE(pLE, pLETab);

#ifdef SK_EXTREME
				if (HW_IS_EXT_LE_FORMAT(pAC)) {
					TXLE_SET_OPC(pLE, OP_MSS | HW_OWNER);
					TXLE_SET_MSSVAL(pLE, pLETab->Bmu.RxTx.MssValue);
				} else {
#endif
					TXLE_SET_OPC(pLE, OP_LRGLEN | HW_OWNER);
					TXLE_SET_LSLEN(pLE, pLETab->Bmu.RxTx.MssValue);
#ifdef SK_EXTREME
				}
#endif

				FLUSH_OPC(pLE) ;
			}
#endif
			GET_TX_LE(pLE, pLETab);
			Ctrl = 0;

			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
				("\tGot empty LE %p idx %d\n", pLE, GET_PUT_IDX(pLETab)));

			SK_DBG_DUMP_TX_LE(pLE);

			LowAddress  = (SK_U32) (pFrag->pPhys & 0xffffffff);
			HighAddress = (SK_U32) (pFrag->pPhys >> 32);

			if (HighAddress != pLETab->BufHighAddr) {
				/* set opcode high part of the address in one LE */
				OpCode = OP_ADDR64 | HW_OWNER;
	
				/* Set now the 32 high bits of the address */
				TXLE_SET_ADDR( pLE, HighAddress);
	
				/* Set the opcode into the LE */
				TXLE_SET_OPC(pLE, OpCode);
	
				/* Flush the LE to memory */
				FLUSH_OPC(pLE);
	
				/* remember the HighAddress we gave to the Hardware */
				pLETab->BufHighAddr = HighAddress;
				
				/* get a new LE because we filled one with high address */
				GET_TX_LE(pLE, pLETab);
			}
	
			/*
			** TCP checksum offload
			*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
			if ((pSkPacket->pMBuf->ip_summed == CHECKSUM_PARTIAL) && 
#else
			if ((pSkPacket->pMBuf->ip_summed == CHECKSUM_HW) && 
#endif
			    (SetOpcodePacketFlag         == SK_TRUE)) {
				Protocol = ((SK_U8)pSkPacket->pMBuf->data[C_OFFSET_IPPROTO] & 0xff);
				/* if (Protocol & C_PROTO_ID_IP) { Ctrl = 0; } */

				// Yukon Extreme B0
				if (HW_FEATURE(pAC, HWF_WA_DEV_510)) {
					if (Protocol & C_PROTO_ID_TCP) {
						Ctrl = CALSUM | WR_SUM | INIT_SUM | LOCK_SUM;
						/* TCP Checksum Calculation Start Position */
						TcpSumStart = C_LEN_ETHERMAC_HEADER + IP_HDR_LEN;
						/* TCP Checksum Write Position */
						TcpSumWrite = TcpSumStart + TCP_CSUM_OFFS;
					} else {
						Ctrl = UDPTCP | CALSUM | WR_SUM | INIT_SUM | LOCK_SUM;
						/* TCP Checksum Calculation Start Position */
						TcpSumStart = ETHER_MAC_HDR_LEN + IP_HDR_LEN;
						/* UDP Checksum Write Position */
						TcpSumWrite = TcpSumStart + UDP_CSUM_OFFS;
					}
				}
				// Yukon 2, Yukon Extreme besides B0
				else {
					if (Protocol & C_PROTO_ID_TCP) {
#ifdef SK_EXTREME
						if (!HW_IS_EXT_LE_FORMAT(pAC)) {
#endif
							Ctrl = CALSUM | WR_SUM | INIT_SUM | LOCK_SUM;
							/* TCP Checksum Calculation Start Position */
							TcpSumStart = C_LEN_ETHERMAC_HEADER + IP_HDR_LEN;
							/* TCP Checksum Write Position */
							TcpSumWrite = TcpSumStart + TCP_CSUM_OFFS;
#ifdef SK_EXTREME
						} else {
							Ctrl = CALSUM;
						}
#endif
					} else {
#ifdef SK_EXTREME
						if (!HW_IS_EXT_LE_FORMAT(pAC)) {
#endif
							Ctrl = UDPTCP | CALSUM | WR_SUM | INIT_SUM | LOCK_SUM;
							/* TCP Checksum Calculation Start Position */
							TcpSumStart = ETHER_MAC_HDR_LEN + IP_HDR_LEN;
							/* UDP Checksum Write Position */
							TcpSumWrite = TcpSumStart + UDP_CSUM_OFFS;
#ifdef SK_EXTREME
						} else {
							Ctrl = CALSUM;
						}
#endif
					}
				}
	
				if ((Ctrl) && (pLETab->Bmu.RxTx.TcpWp != TcpSumWrite)) {
					/* Update the last value of the write position */
					pLETab->Bmu.RxTx.TcpWp = TcpSumWrite;
	
					/* Set the Lock field for this LE: */
					/* Checksum calculation for one packet only */
					TXLE_SET_LCKCS(pLE, 1);
	
					/* Set the start position for checksum. */
					TXLE_SET_STACS(pLE, TcpSumStart);
	
					/* Set the position where the checksum will be writen */
					TXLE_SET_WRICS(pLE, TcpSumWrite);
	
					/* Set the initial value for checksum */
					/* PseudoHeader CS passed from Linux -> 0! */
					TXLE_SET_INICS(pLE, 0);
	
					/* Set the opcode for tcp checksum */
					TXLE_SET_OPC(pLE, OP_TCPLISW | HW_OWNER);
	
					/* Flush the LE to memory */
					FLUSH_OPC(pLE);
	
					/* get a new LE because we filled one with data for checksum */
					GET_TX_LE(pLE, pLETab);
				}
			} /* end TCP offload handling */
	
			TXLE_SET_ADDR(pLE, LowAddress);
			TXLE_SET_LEN(pLE, pFrag->FragLen);
	
			if (SetOpcodePacketFlag){
#ifdef NETIF_F_TSO
				if (Mss) {
					OpCode = OP_LARGESEND | HW_OWNER;
				} else {
#endif
					OpCode = OP_PACKET | HW_OWNER;
#ifdef NETIF_F_TSO
				}
#endif
				SetOpcodePacketFlag = SK_FALSE;
			} else {
				/* Follow packet in a sequence has always OP_BUFFER */
				OpCode = OP_BUFFER | HW_OWNER;
			}

			/* Check if the low address is near the upper limit. */
			CHECK_LOW_ADDRESS(pLETab->BufHighAddr, LowAddress, pFrag->FragLen);

			pFrag = pFrag->pNext;
			if (pFrag == NULL) {
				/* mark last fragment */
				Ctrl |= EOP;
			}
			TXLE_SET_CTRL(pLE, Ctrl);
			TXLE_SET_OPC(pLE, OpCode);
			FLUSH_OPC(pLE);

			SK_DBG_DUMP_TX_LE(pLE);
		}
		
		/* 
		** Remember next LE for tx complete 
		*/
		pSkPacket->NextLE = GET_PUT_IDX(pLETab);
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
			("\tNext LE for pkt %p is %d\n", pSkPacket, pSkPacket->NextLE));

		/* 
		** Add packet to working packets queue 
		*/
		PUSH_PKT_AS_LAST_IN_QUEUE(pWorkQueue, pSkPacket);

		/* 
		** give transmit start command
		*/
		if (HW_FEATURE(pAC, HWF_WA_DEV_42)) {
			spin_lock(&pAC->SetPutIndexLock);
			SkGeY2SetPutIndex(pAC, pAC->IoBase, Y2_PREF_Q_ADDR(Q_XA1,0), &pAC->TxPort[0][0].TxALET);
			spin_unlock(&pAC->SetPutIndexLock);
		} else {
			/* write put index */
			if (Port == 0) { 
				SK_OUT32(pAC->IoBase, 
					Y2_PREF_Q_ADDR(Q_XA1,PREF_UNIT_PUT_IDX_REG), 
					GET_PUT_IDX(&pAC->TxPort[0][0].TxALET)); 
				UPDATE_HWPUT_IDX(&pAC->TxPort[0][0].TxALET);
			} else {
				SK_OUT32(pAC->IoBase, 
					Y2_PREF_Q_ADDR(Q_XA2, PREF_UNIT_PUT_IDX_REG), 
					GET_PUT_IDX(&pAC->TxPort[1][0].TxALET)); 
				UPDATE_HWPUT_IDX(&pAC->TxPort[1][0].TxALET);
			}
		}
	
		if (IS_Q_EMPTY(&(pAC->TxPort[Port][TX_PRIO_LOW].TxAQ_waiting))) {
			break; /* get out of while */
		}
		POP_FIRST_PKT_FROM_QUEUE(pWaitQueue, pSkPacket);
	} /* while (pSkPacket != NULL) */

	spin_unlock_irqrestore(&pAC->TxQueueLock, LockFlag);

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
		("<== GiveTxBufferToHw\n"));
	return;
}	/* GiveTxBufferToHw */

/***********************************************************************
 *
 *	GiveRxBufferToHw - commits a previously allocated DMA area to HW
 *
 * Description:
 *	This functions gives receive buffers to HW. If no list elements
 *	are available the buffers will be queued. 
 *
 * Notes:
 *       This function can run only once in a system at one time.
 *
 * Returns: N/A
 */
static void GiveRxBufferToHw(
SK_AC      *pAC,      /* pointer to adapter control context         */
SK_IOC      IoC,      /* I/O control context (address of registers) */
int         Port,     /* port index for which the buffer is used    */
SK_PACKET  *pPacket)  /* receive buffer(s)                          */
{
	SK_HWLE         *pLE;
	SK_LE_TABLE     *pLETab;
	SK_BOOL         Done = SK_FALSE;  /* at least on LE changed? */
	SK_U32          LowAddress;
	SK_U32          HighAddress;
	SK_U32          PrefetchReg;      /* register for Put index  */
	unsigned        NumFree;
	unsigned        Required;
	unsigned long   Flags;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
	("==> GiveRxBufferToHw(Port %c, Packet %p)\n", 'A' + Port, pPacket));

	pLETab	= &pAC->RxPort[Port].RxLET;

	if (Port == 0) {
		PrefetchReg = Y2_PREF_Q_ADDR(Q_R1, PREF_UNIT_PUT_IDX_REG);
	} else {
		PrefetchReg = Y2_PREF_Q_ADDR(Q_R2, PREF_UNIT_PUT_IDX_REG);
	} 

	if (pPacket != NULL) {
		/*
		** For the time being, we have only one packet passed
		** to this function which might be changed in future!
		*/
		PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pPacket);
	}

	/* 
	** now pPacket contains the very first waiting packet
	*/
	POP_FIRST_PKT_FROM_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pPacket);
	while (pPacket != NULL) {
		if (HW_FEATURE(pAC, HWF_WA_DEV_420)) {
			if (NbrRxBuffersInHW >= MAX_NBR_RX_BUFFERS_IN_HW) {
				PUSH_PKT_AS_FIRST_IN_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pPacket);
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
					("<== GiveRxBufferToHw()\n"));
				return;
			} 
			NbrRxBuffersInHW++;
		}

		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
			("Try to add packet %p\n", pPacket));

		/* 
		** Check whether we have enough listelements:
		**
		** we have to take into account that each fragment 
		** may need an additional list element for the high 
		** part of the address here I simplified it by 
		** using MAX_FRAG_OVERHEAD maybe it's worth to split 
		** this constant for Rx and Tx or to calculate the
		** real number of needed LE's
		*/
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
			("\tNum %d Put %d Done %d Free %d %d\n",
			pLETab->Num, pLETab->Put, pLETab->Done,
			NUM_FREE_LE_IN_TABLE(pLETab),
			(NUM_FREE_LE_IN_TABLE(pLETab))));

		Required = pPacket->NumFrags + MAX_FRAG_OVERHEAD;
		NumFree = NUM_FREE_LE_IN_TABLE(pLETab);
		if (NumFree) {
			NumFree--;
		}

		if (Required > NumFree ) {
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
				SK_DBGCAT_DRV_RX_PROGRESS | SK_DBGCAT_DRV_ERROR,
				("\tOut of LEs have %d need %d\n",
				NumFree, Required));

			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
				("\tWaitQueue starts with packet %p\n", pPacket));
			PUSH_PKT_AS_FIRST_IN_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pPacket);
			if (Done) {
				/*
				** write Put index to BMU or Polling Unit and make the LE's
				** available for the hardware
				*/
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
					("\tWrite new Put Idx\n"));

				SK_OUT32(IoC, PrefetchReg, GET_PUT_IDX(pLETab));
				UPDATE_HWPUT_IDX(pLETab);
			}
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
				("<== GiveRxBufferToHw()\n"));
			return;
		} else {
			if (!AllocAndMapRxBuffer(pAC, pPacket, Port)) {
				/*
				** Failure while allocating sk_buff might
				** be due to temporary short of resources
				** Maybe next time buffers are available.
				** Until this, the packet remains in the 
				** RX waiting queue...
				*/
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
					SK_DBGCAT_DRV_RX_PROGRESS | SK_DBGCAT_DRV_ERROR,
					("Failed to allocate Rx buffer\n"));

				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
					("WaitQueue starts with packet %p\n", pPacket));
				PUSH_PKT_AS_FIRST_IN_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pPacket);
				if (Done) {
					/*
					** write Put index to BMU or Polling 
					** Unit and make the LE's
					** available for the hardware
					*/
					SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
						("\tWrite new Put Idx\n"));
	
					SK_OUT32(IoC, PrefetchReg, GET_PUT_IDX(pLETab));
					UPDATE_HWPUT_IDX(pLETab);
				}
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
					("<== GiveRxBufferToHw()\n"));
				return;
			}
		}
		Done = SK_TRUE;

		LowAddress = (SK_U32) (pPacket->pFrag->pPhys & 0xffffffff);
		HighAddress = (SK_U32) (pPacket->pFrag->pPhys >> 32);
		if (HighAddress != pLETab->BufHighAddr) {
			/* get a new LE for high address */
			GET_RX_LE(pLE, pLETab);

			/* Set now the 32 high bits of the address */
			RXLE_SET_ADDR(pLE, HighAddress);

			/* Set the control bits of the address */
			RXLE_SET_CTRL(pLE, 0);

			/* Set the opcode into the LE */
			RXLE_SET_OPC(pLE, (OP_ADDR64 | HW_OWNER));

			/* Flush the LE to memory */
			FLUSH_OPC(pLE);

			/* remember the HighAddress we gave to the Hardware */
			pLETab->BufHighAddr = HighAddress;
		}

		/*
		** Fill data into listelement
		*/
		GET_RX_LE(pLE, pLETab);
		RXLE_SET_ADDR(pLE, LowAddress);
		RXLE_SET_LEN(pLE, pPacket->pFrag->FragLen);
		RXLE_SET_CTRL(pLE, 0);
		RXLE_SET_OPC(pLE, (OP_PACKET | HW_OWNER));
		FLUSH_OPC(pLE);

		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
			("=== LE filled\n"));

		SK_DBG_DUMP_RX_LE(pLE);

		/* 
		** Remember next LE for rx complete 
		*/
		pPacket->NextLE = GET_PUT_IDX(pLETab);

		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
			("\tPackets Next LE is %d\n", pPacket->NextLE));

		/* 
		** Add packet to working receive buffer queue and get
		** any next packet out of the waiting queue
		*/
		PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->RxPort[Port].RxQ_working, pPacket);
		if (IS_Q_EMPTY(&(pAC->RxPort[Port].RxQ_waiting))) {
			break; /* get out of while processing */
		}
		POP_FIRST_PKT_FROM_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pPacket);
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
		("\tWaitQueue is empty\n"));

	if (Done) {
		/*
		** write Put index to BMU or Polling Unit and make the LE's
		** available for the hardware
		*/
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
			("\tWrite new Put Idx\n"));

		/* Speed enhancement for a2 chipsets */
		if (HW_FEATURE(pAC, HWF_WA_DEV_42)) {
			spin_lock_irqsave(&pAC->SetPutIndexLock, Flags);
			SkGeY2SetPutIndex(pAC, pAC->IoBase, Y2_PREF_Q_ADDR(Q_R1,0), pLETab);
			spin_unlock_irqrestore(&pAC->SetPutIndexLock, Flags);
		} else {
			/* write put index */
			if (Port == 0) { 
				SK_OUT32(IoC, 
					Y2_PREF_Q_ADDR(Q_R1, PREF_UNIT_PUT_IDX_REG), 
					GET_PUT_IDX(pLETab)); 
			} else {
				SK_OUT32(IoC, 
					Y2_PREF_Q_ADDR(Q_R2, PREF_UNIT_PUT_IDX_REG), 
					GET_PUT_IDX(pLETab)); 
			}

			/* Update put index */
			UPDATE_HWPUT_IDX(pLETab);
		}
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
		("<== GiveRxBufferToHw()\n"));
}       /* GiveRxBufferToHw */

/***********************************************************************
 *
 *	FillReceiveTableYukon2 - map any waiting RX buffers to HW
 *
 * Description:
 *	If the list element table contains more empty elements than 
 *	specified this function tries to refill them.
 *
 * Notes:
 *       This function can run only once per port in a system at one time.
 *
 * Returns: N/A
 */
void FillReceiveTableYukon2(
SK_AC   *pAC,   /* pointer to adapter control context */
SK_IOC   IoC,   /* I/O control context                */
int      Port)  /* port index of RX                   */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
		("==> FillReceiveTableYukon2 (Port %c)\n", 'A' + Port));

	if (NUM_FREE_LE_IN_TABLE(&pAC->RxPort[Port].RxLET) >
		pAC->MaxUnusedRxLeWorking) {

		/* 
		** Give alle waiting receive buffers down 
		** The queue holds all RX packets that
		** need a fresh allocation of the sk_buff.
		*/
		if (pAC->RxPort[Port].RxQ_waiting.pHead != NULL) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
			("Waiting queue is not empty -> give it to HW"));
			GiveRxBufferToHw(pAC, IoC, Port, NULL);
		}
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
		("<== FillReceiveTableYukon2 ()\n"));
}	/* FillReceiveTableYukon2 */

/******************************************************************************
 *
 *
 *	HandleReceives - will pass any ready RX packet to kernel
 *
 * Description:
 *	This functions handles a received packet. It checks wether it is
 *	valid, updates the receive list element table and gives the receive
 *	buffer to Linux
 *
 * Notes:
 *	This function can run only once per port at one time in the system.
 *
 * Returns: N/A
 */
static SK_BOOL HandleReceives(
SK_AC  *pAC,          /* adapter control context                     */
int     Port,         /* port on which a packet has been received    */
SK_U16  FrameLength,  /* number of bytes which was actually received */
SK_U32  FrameStatus,  /* MAC frame status word                       */
SK_U16  Tcp1,         /* first hw checksum                           */
SK_U16  Tcp2,         /* second hw checksum                          */
SK_U32  Tist,         /* timestamp                                   */
SK_U16  Vlan          /* Vlan Id                                     */
#ifdef SK_EXTREME
, SK_U32  ExtremeCsumResult
#endif
)
{
	SK_PACKET       *pSkPacket;
	SK_LE_TABLE     *pLETab;
	SK_MBUF         *pRlmtMbuf;  /* buffer for giving RLMT frame */
	struct sk_buff  *pMsg;       /* ptr to message holding frame */
#ifdef __ia64__
	struct sk_buff  *pNewMsg;    /* used when IP aligning        */
#endif
	SK_BOOL         IsGoodPkt;
	SK_BOOL         IsBc;
	SK_BOOL         IsMc;
	SK_EVPARA       EvPara;      /* an event parameter union     */
	SK_I16          LenToFree;   /* must be signed integer       */
	unsigned long   Flags;       /* for spin lock                */
	unsigned int    RlmtNotifier;
	unsigned short  Type;
	int             IpFrameLength;
	int             HeaderLength;
	int             NumBytes; 
	int             Result;
	int             Offset = 0;
	SK_U16          HighVal;
	SK_BOOL         CheckBcMc = SK_FALSE;
#ifdef Y2_SYNC_CHECK
	SK_U16		MyTcp;
#endif

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
		("==> HandleReceives (Port %c)\n", 'A' + Port));

	if (HW_FEATURE(pAC, HWF_WA_DEV_521)) {
		HighVal = (SK_U16)(((FrameStatus) & GMR_FS_LEN_MSK) >> GMR_FS_LEN_SHIFT);

		if ((HighVal > ETH_MAX_LEN) || (FrameStatus == 0x7ffc0001)) {
			FrameStatus = 0;
		}
		else if (HighVal != FrameLength) {
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
					("521: We take pkt len %x stat: %x\n", FrameLength, FrameStatus));

				/*
				** Try to recover the status word. The multicast and broadcast bits
				** from status word are set when we have a valid packet header.
				*/
				FrameStatus = 0;

				/* Set the ReceiveOK bit. */
				FrameStatus |= GMR_FS_RX_OK;

				/* Set now the length into the status word */
				FrameStatus |= (FrameLength << GMR_FS_LEN_SHIFT);

				/* Insure that bit 31 is reset */
				FrameStatus &= ~GMR_FS_LKUP_BIT2;
				CheckBcMc = SK_TRUE;
		}
	}

	/* 
	** Check whether we want to receive this packet 
	*/
	SK_Y2_RXSTAT_CHECK_PKT(FrameLength, FrameStatus, IsGoodPkt);

	/*
	** Take first receive buffer out of working queue 
	*/
	POP_FIRST_PKT_FROM_QUEUE(&pAC->RxPort[Port].RxQ_working, pSkPacket);
	if (pSkPacket == NULL) {
 		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
			SK_DBGCAT_DRV_ERROR,
			("Packet not available. NULL pointer.\n"));
		return(SK_TRUE);
	}

	if (HW_FEATURE(pAC, HWF_WA_DEV_420)) {
		NbrRxBuffersInHW--;
	}

	/* 
	** Verify the received length of the frame! Note that having 
	** multiple RxBuffers being aware of one single receive packet
	** (one packet spread over multiple RxBuffers) is not supported 
	** by this driver!
	*/
	if ((FrameLength > pAC->RxPort[Port].RxBufSize) || 
		(FrameLength > (SK_U16) pSkPacket->PacketLen)) {
		IsGoodPkt = SK_FALSE;
	}

	/* 
	** Initialize vars for selected port 
	*/
	pLETab = &pAC->RxPort[Port].RxLET;

	/* 
	** Reset own bit in LE's between old and new Done index
	** This is not really necessary but makes debugging easier 
	*/
	CLEAR_LE_OWN_FROM_DONE_TO(pLETab, pSkPacket->NextLE);

	/* 
	** Free the list elements for new Rx buffers 
	*/
	SET_DONE_INDEX(pLETab, pSkPacket->NextLE);
	pMsg = pSkPacket->pMBuf;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
		("Received frame of length %d on port %d\n",FrameLength, Port));

	if (!IsGoodPkt) {
		/* 
		** release the DMA mapping 
		*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
 		pci_dma_sync_single(pAC->PciDev,
				(dma_addr_t) pSkPacket->pFrag->pPhys,
				pSkPacket->pFrag->FragLen,
				PCI_DMA_FROMDEVICE);

#else
		pci_dma_sync_single_for_cpu(pAC->PciDev,
				(dma_addr_t) pSkPacket->pFrag->pPhys,
				pSkPacket->pFrag->FragLen,
				PCI_DMA_FROMDEVICE);
#endif

		DEV_KFREE_SKB_ANY(pSkPacket->pMBuf);
		PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pSkPacket);
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
			("<== HandleReceives (Port %c)\n", 'A' + Port));

		/*
		** Sanity check for RxBuffer overruns...
		*/
		LenToFree = FrameLength - (pSkPacket->pFrag->FragLen);
		while (LenToFree > 0) {
			POP_FIRST_PKT_FROM_QUEUE(&pAC->RxPort[Port].RxQ_working, pSkPacket);
			if (HW_FEATURE(pAC, HWF_WA_DEV_420)) {
				NbrRxBuffersInHW--;
			}
			CLEAR_LE_OWN_FROM_DONE_TO(pLETab, pSkPacket->NextLE);
			SET_DONE_INDEX(pLETab, pSkPacket->NextLE);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
			pci_dma_sync_single(pAC->PciDev,
					(dma_addr_t) pSkPacket->pFrag->pPhys,
					pSkPacket->pFrag->FragLen,
					PCI_DMA_FROMDEVICE);
#else
			pci_dma_sync_single_for_device(pAC->PciDev,
					(dma_addr_t) pSkPacket->pFrag->pPhys,
					pSkPacket->pFrag->FragLen,
					PCI_DMA_FROMDEVICE); 
#endif

			DEV_KFREE_SKB_ANY(pSkPacket->pMBuf);
			PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pSkPacket);
			LenToFree = LenToFree - ((SK_I16)(pSkPacket->pFrag->FragLen));
			
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_RX_PROGRESS | SK_DBGCAT_DRV_ERROR,
				("<==HandleReceives (Port %c) drop faulty len pkt(2)\n",'A'+Port));
		}
		return(SK_TRUE);
	} else {
		/* 
		** Release the DMA mapping 
		*/
		pci_unmap_single(pAC->PciDev,
				 pSkPacket->pFrag->pPhys,
				 pAC->RxPort[Port].RxBufSize,
				 PCI_DMA_FROMDEVICE);

		skb_put(pMsg, FrameLength);		/* set message len */
		pMsg->ip_summed = CHECKSUM_NONE;	/* initial default */

#ifdef Y2_SYNC_CHECK
		pAC->FramesWithoutSyncCheck++;
		if (pAC->FramesWithoutSyncCheck > Y2_RESYNC_WATERMARK) {
			if ((Tcp1 != 1) || (Tcp2 != 0)) {
				pAC->FramesWithoutSyncCheck = 0;
				MyTcp = (SK_U16) SkCsCalculateChecksum(
						&pMsg->data[14],
						FrameLength - 14);
				if (MyTcp != Tcp1) {
					/* Queue port reset event */
					SkLocalEventQueue(pAC, SKGE_DRV,
					SK_DRV_RECOVER,Port,-1,SK_FALSE);
				}
			}
		}
#endif

		if (pAC->RxPort[Port].UseRxCsum) {
#ifdef SK_EXTREME
			if (HW_IS_EXT_LE_FORMAT(pAC)) {
				/* Checksum calculation for Yukon Extreme */
				if (CSS_IS_IPV4(ExtremeCsumResult) && CSS_IPV4_CSUM_OK(ExtremeCsumResult)) {
        				pMsg->ip_summed = CHECKSUM_UNNECESSARY;
				} else {
        				pMsg->ip_summed = CHECKSUM_NONE;
				}
			} else {
#endif
				Type = ntohs(*((short*)&pMsg->data[12]));
				if (Type == 0x800) {
					*((char *)&(IpFrameLength)) = pMsg->data[16];
					*(((char *)&(IpFrameLength))+1) = pMsg->data[17];
					IpFrameLength = ntohs(IpFrameLength);
					HeaderLength  = FrameLength - IpFrameLength;
					if (HeaderLength == 0xe) {
						Result = 
						    SkCsGetReceiveInfo(pAC,&pMsg->data[14],Tcp1,Tcp2, Port, IpFrameLength);
						if ((Result == SKCS_STATUS_IP_FRAGMENT) ||
						    (Result == SKCS_STATUS_IP_CSUM_OK)  ||
						    (Result == SKCS_STATUS_TCP_CSUM_OK) ||
						    (Result == SKCS_STATUS_UDP_CSUM_OK)) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
							pMsg->ip_summed = CHECKSUM_COMPLETE;
							pMsg->csum = Tcp1 & 0xffff;
#else
							pMsg->ip_summed = CHECKSUM_UNNECESSARY;
#endif
						} else if ((Result == SKCS_STATUS_TCP_CSUM_ERROR)    ||
						           (Result == SKCS_STATUS_UDP_CSUM_ERROR)    ||
						           (Result == SKCS_STATUS_IP_CSUM_ERROR_UDP) ||
						           (Result == SKCS_STATUS_IP_CSUM_ERROR_TCP) ||
						           (Result == SKCS_STATUS_IP_CSUM_ERROR)) {
							SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
								SK_DBGCAT_DRV_RX_PROGRESS | SK_DBGCAT_DRV_ERROR,
								("skge: CRC error. Frame dropped!\n"));
							DEV_KFREE_SKB_ANY(pMsg);
							PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pSkPacket);
							SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_RX_PROGRESS,
								("<==HandleReceives(Port %c)\n",'A'+Port));
							return(SK_TRUE);
						} else {
							pMsg->ip_summed = CHECKSUM_NONE;
						}
					} /* end if (HeaderLength == valid) */
				} /* end if (Type == 0x800) -> IP frame */
#ifdef SK_EXTREME
			} /* if (HW_IS_EXT_LE_FORMAT(pAC)) */
#endif
		} /* end if (pRxPort->UseRxCsum) */
		
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
			SK_DBGCAT_DRV_RX_PROGRESS,("V"));
		RlmtNotifier = SK_RLMT_RX_PROTOCOL;

		if (CheckBcMc == SK_TRUE) {

			/* Check if this is a broadcast packet */
			if (SK_ADDR_EQUAL(pMsg->data, "\xFF\xFF\xFF\xFF\xFF\xFF")) {

				/* Broadcast packet */
				FrameStatus |= GMR_FS_BC;
			}
			else if (pMsg->data[0] & 0x01) {

				/* Multicast packet */
				FrameStatus |= GMR_FS_MC;
			}
		}

		IsBc = (FrameStatus & GMR_FS_BC) ? SK_TRUE : SK_FALSE;
		SK_RLMT_PRE_LOOKAHEAD(pAC,Port,FrameLength,
					IsBc,&Offset,&NumBytes);
		if (NumBytes != 0) {
			IsMc = (FrameStatus & GMR_FS_MC) ? SK_TRUE : SK_FALSE;
			SK_RLMT_LOOKAHEAD(pAC,Port,&pMsg->data[Offset],
						IsBc,IsMc,&RlmtNotifier);
		}

		if (RlmtNotifier == SK_RLMT_RX_PROTOCOL) {
			SK_DBG_MSG(NULL,SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_RX_PROGRESS,("W"));
			if ((Port == pAC->ActivePort)||(pAC->RlmtNets == 2)) {
				/* send up only frames from active port */
				SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
					SK_DBGCAT_DRV_RX_PROGRESS,("U"));
#ifdef xDEBUG
				DumpMsg(pMsg, "Rx");
#endif
				SK_PNMI_CNT_RX_OCTETS_DELIVERED(pAC,
					FrameLength, Port);
#ifdef __ia64__
				pNewMsg = alloc_skb(pMsg->len, GFP_ATOMIC);
				skb_reserve(pNewMsg, 2); /* to align IP */
				SK_MEMCPY(pNewMsg->data,pMsg->data,pMsg->len);
				pNewMsg->ip_summed = pMsg->ip_summed;
				skb_put(pNewMsg, pMsg->len);
				DEV_KFREE_SKB_ANY(pMsg);
				pMsg = pNewMsg;
#endif
				pMsg->dev = pAC->dev[Port];
				pMsg->protocol = eth_type_trans(pMsg,
					pAC->dev[Port]);
#ifdef CONFIG_SK98LIN_NAPI
				netif_receive_skb(pMsg);
#else
				netif_rx(pMsg);
#endif
				pAC->dev[Port]->last_rx = jiffies;
			} else { /* drop frame */
				SK_DBG_MSG(NULL,SK_DBGMOD_DRV,
					SK_DBGCAT_DRV_RX_PROGRESS,("D"));
				DEV_KFREE_SKB_ANY(pMsg);
			}
		} else { /* This is an RLMT-packet! */
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_RX_PROGRESS,("R"));
			pRlmtMbuf = SkDrvAllocRlmtMbuf(pAC,
				pAC->IoBase, FrameLength);
			if (pRlmtMbuf != NULL) {
				pRlmtMbuf->pNext = NULL;
				pRlmtMbuf->Length = FrameLength;
				pRlmtMbuf->PortIdx = Port;
				EvPara.pParaPtr = pRlmtMbuf;
				SK_MEMCPY((char*)(pRlmtMbuf->pData),
				          (char*)(pMsg->data),FrameLength);

#ifdef CONFIG_SK98LIN_NAPI
				spin_lock_irqsave(&pAC->SlowPathLock, Flags);
#endif
				SkEventQueue(pAC, SKGE_RLMT,
						SK_RLMT_PACKET_RECEIVED,
						EvPara);
				pAC->CheckQueue = SK_TRUE;
#ifdef CONFIG_SK98LIN_NAPI
				spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
#endif
				SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
					SK_DBGCAT_DRV_RX_PROGRESS,("Q"));
			}
			if (pAC->dev[Port]->flags & (IFF_PROMISC | IFF_ALLMULTI)) {
#ifdef __ia64__
				pNewMsg = alloc_skb(pMsg->len, GFP_ATOMIC);
				skb_reserve(pNewMsg, 2); /* to align IP */
				SK_MEMCPY(pNewMsg->data,pMsg->data,pMsg->len);
				pNewMsg->ip_summed = pMsg->ip_summed;
				pNewMsg->len = pMsg->len;
				DEV_KFREE_SKB_ANY(pMsg);
				pMsg = pNewMsg;
#endif
				pMsg->dev = pAC->dev[Port];
				pMsg->protocol = eth_type_trans(pMsg,pAC->dev[Port]);
				netif_rx(pMsg);
				pAC->dev[Port]->last_rx = jiffies;
			} else {
				DEV_KFREE_SKB_ANY(pMsg);
			}
		} /* if packet for rlmt */
		PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->RxPort[Port].RxQ_waiting, pSkPacket);
	} /* end if-else (IsGoodPkt) */

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
		("<== HandleReceives (Port %c)\n", 'A' + Port));
	return(SK_TRUE);

}	/* HandleReceives */

/***********************************************************************
 *
 * 	CheckForSendComplete - Frees any freeable Tx bufffer 
 *
 * Description:
 *	This function checks the queues of a port for completed send
 *	packets and returns these packets back to the OS.
 *
 * Notes:
 *	This function can run simultaneously for both ports if
 *	the OS function OSReturnPacket() can handle this,
 *
 *	Such a send complete does not mean, that the packet is really
 *	out on the wire. We just know that the adapter has copied it
 *	into its internal memory and the buffer in the systems memory
 *	is no longer needed.
 *
 * Returns: N/A
 */
static void CheckForSendComplete(
SK_AC         *pAC,     /* pointer to adapter control context  */
SK_IOC         IoC,     /* I/O control context                 */
int            Port,    /* port index                          */
SK_PKT_QUEUE  *pPQ,     /* tx working packet queue to check    */
SK_LE_TABLE   *pLETab,  /* corresponding list element table    */
unsigned int   Done)    /* done index reported for this LET    */
{
	SK_PACKET       *pSkPacket;
	SK_PKT_QUEUE     SendCmplPktQ = { NULL, NULL, SPIN_LOCK_UNLOCKED };
	SK_BOOL          DoWakeQueue  = SK_FALSE;
	unsigned long    Flags;
	unsigned         Put;
	
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
		("==> CheckForSendComplete(Port %c)\n", 'A' + Port));

	/* 
	** Reset own bit in LE's between old and new Done index
	** This is not really necessairy but makes debugging easier 
	*/
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
		("Clear Own Bits in TxTable from %d to %d\n",
		pLETab->Done, (Done == 0) ?
		NUM_LE_IN_TABLE(pLETab) :
		(Done - 1)));

	spin_lock_irqsave(&(pPQ->QueueLock), Flags);

	CLEAR_LE_OWN_FROM_DONE_TO(pLETab, Done);

	Put = GET_PUT_IDX(pLETab);

	/* 
	** Check whether some packets have been completed 
	*/
	PLAIN_POP_FIRST_PKT_FROM_QUEUE(pPQ, pSkPacket);
	while (pSkPacket != NULL) {
		
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
			("Check Completion of Tx packet %p\n", pSkPacket));
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
			("Put %d NewDone %d NextLe of Packet %d\n", Put, Done,
			pSkPacket->NextLE));

		if ((Put > Done) &&
			((pSkPacket->NextLE > Put) || (pSkPacket->NextLE <= Done))) {
			PLAIN_PUSH_PKT_AS_LAST_IN_QUEUE(&SendCmplPktQ, pSkPacket);
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
				("Packet finished (a)\n"));
		} else if ((Done > Put) &&
			(pSkPacket->NextLE > Put) && (pSkPacket->NextLE <= Done)) {
			PLAIN_PUSH_PKT_AS_LAST_IN_QUEUE(&SendCmplPktQ, pSkPacket);
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
				("Packet finished (b)\n"));
		} else if ((Done == NUMBER_OF_TX_LE-1) && (Put == 0) && (pSkPacket->NextLE == 0)) {
			PLAIN_PUSH_PKT_AS_LAST_IN_QUEUE(&SendCmplPktQ, pSkPacket);
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
				("Packet finished (b)\n"));
			DoWakeQueue = SK_TRUE;
		} else if (Done == Put) {
			/* all packets have been sent */
			PLAIN_PUSH_PKT_AS_LAST_IN_QUEUE(&SendCmplPktQ, pSkPacket);
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
				("Packet finished (c)\n"));
		} else {
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
				("Packet not yet finished\n"));
			PLAIN_PUSH_PKT_AS_FIRST_IN_QUEUE(pPQ, pSkPacket);
			break;
		}
		PLAIN_POP_FIRST_PKT_FROM_QUEUE(pPQ, pSkPacket);
	}
	spin_unlock_irqrestore(&(pPQ->QueueLock), Flags);

	/* 
	** Set new done index in list element table
	*/
	SET_DONE_INDEX(pLETab, Done);
	 
	/*
	** All TX packets that are send complete should be added to
	** the free queue again for new sents to come
	*/
	pSkPacket = SendCmplPktQ.pHead;
	while (pSkPacket != NULL) {
		while (pSkPacket->pFrag != NULL) {
			pci_unmap_page(pAC->PciDev,
					(dma_addr_t) pSkPacket->pFrag->pPhys,
					pSkPacket->pFrag->FragLen,
					PCI_DMA_FROMDEVICE);
			pSkPacket->pFrag = pSkPacket->pFrag->pNext;
		}

		DEV_KFREE_SKB_ANY(pSkPacket->pMBuf);
		pSkPacket->pMBuf	= NULL;
		pSkPacket = pSkPacket->pNext; /* get next packet */
	}

	/*
	** Append the available TX packets back to free queue
	*/
	if (SendCmplPktQ.pHead != NULL) { 
		spin_lock_irqsave(&(pAC->TxPort[Port][0].TxQ_free.QueueLock), Flags);
		if (pAC->TxPort[Port][0].TxQ_free.pTail != NULL) {
			pAC->TxPort[Port][0].TxQ_free.pTail->pNext = SendCmplPktQ.pHead;
			pAC->TxPort[Port][0].TxQ_free.pTail        = SendCmplPktQ.pTail;
			if (pAC->TxPort[Port][0].TxQ_free.pHead->pNext == NULL) {
				netif_wake_queue(pAC->dev[Port]);
			}
		} else {
			pAC->TxPort[Port][0].TxQ_free.pHead = SendCmplPktQ.pHead;
			pAC->TxPort[Port][0].TxQ_free.pTail = SendCmplPktQ.pTail; 
			netif_wake_queue(pAC->dev[Port]);
		}
		if (Done == Put) {
			netif_wake_queue(pAC->dev[Port]);
		}
		if (DoWakeQueue) {
			netif_wake_queue(pAC->dev[Port]);
			DoWakeQueue = SK_FALSE;
		}
		spin_unlock_irqrestore(&pAC->TxPort[Port][0].TxQ_free.QueueLock, Flags);
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
		("<== CheckForSendComplete()\n"));

	return;
}	/* CheckForSendComplete */

/*****************************************************************************
 *
 *	UnmapAndFreeTxPktBuffer
 *
 * Description:
 *      This function free any allocated space of receive buffers
 *
 * Arguments:
 *      pAC - A pointer to the adapter context struct.
 *
 */
static void UnmapAndFreeTxPktBuffer(
SK_AC       *pAC,       /* pointer to adapter context             */
SK_PACKET   *pSkPacket,	/* pointer to port struct of ring to fill */
int          TxPort)    /* TX port index                          */
{
	SK_FRAG	 *pFrag = pSkPacket->pFrag;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
		("--> UnmapAndFreeTxPktBuffer\n"));

	while (pFrag != NULL) {
		pci_unmap_page(pAC->PciDev,
				(dma_addr_t) pFrag->pPhys,
				pFrag->FragLen,
				PCI_DMA_FROMDEVICE);
		pFrag = pFrag->pNext;
	}

	DEV_KFREE_SKB_ANY(pSkPacket->pMBuf);
	pSkPacket->pMBuf	= NULL;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
		("<-- UnmapAndFreeTxPktBuffer\n"));
}

/*****************************************************************************
 *
 * 	HandleStatusLEs
 *
 * Description:
 *	This function checks for any new status LEs that may have been 
  *	received. Those status LEs may either be Rx or Tx ones.
 *
 * Returns:	N/A
 */
static SK_BOOL HandleStatusLEs(
#ifdef CONFIG_SK98LIN_NAPI
SK_AC *pAC,       /* pointer to adapter context   */
int   *WorkDone,  /* Done counter needed for NAPI */
int    WorkToDo)  /* ToDo counter for NAPI        */
#else
SK_AC *pAC)       /* pointer to adapter context   */
#endif
{
	int       DoneTxA[SK_MAX_MACS];
	int       DoneTxS[SK_MAX_MACS];
	int       Port;
	SK_BOOL   handledStatLE = SK_FALSE;
	SK_BOOL   NewDone       = SK_FALSE;
	SK_HWLE  *pLE;
	SK_U16    HighVal;
	SK_U32    LowVal;
	SK_U32    IntSrc;
	SK_U8     OpCode;
	int       i;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
		("==> HandleStatusLEs\n"));

	do {
		if (OWN_OF_FIRST_LE(&pAC->StatusLETable) != HW_OWNER)
			break;

		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
			("Check next Own Bit of ST-LE[%d]: 0x%li \n",
			(pAC->StatusLETable.Done + 1) % NUM_LE_IN_TABLE(&pAC->StatusLETable),
			 OWN_OF_FIRST_LE(&pAC->StatusLETable)));

		while (OWN_OF_FIRST_LE(&pAC->StatusLETable) == HW_OWNER) {
			GET_ST_LE(pLE, &pAC->StatusLETable);
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
				("Working on finished status LE[%d]:\n",
				GET_DONE_INDEX(&pAC->StatusLETable)));
#if 0
			SK_DBG_DUMP_ST_LE(pLE);
#endif
			handledStatLE = SK_TRUE;
			OpCode = STLE_GET_OPC(pLE) & ~HW_OWNER;
			Port = STLE_GET_LINK(pLE);

#ifdef USE_TIST_FOR_RESET
			if (!HW_IS_EXT_LE_FORMAT(pAC) && SK_ADAPTER_WAITING_FOR_TIST(pAC)) {
				/* do we just have a tist LE ? */
				if ((OpCode & OP_RXTIMESTAMP) == OP_RXTIMESTAMP) {
					for (i = 0; i < pAC->GIni.GIMacsFound; i++) {
						if (SK_PORT_WAITING_FOR_ANY_TIST(pAC, i)) {
							/* if a port is waiting for any tist it is done */
							SK_CLR_STATE_FOR_PORT(pAC, i);
							SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DUMP,
								("Got any Tist on port %c (now 0x%X!!!)\n",
								'A' + i, pAC->AdapterResetState));
						}
						if (SK_PORT_WAITING_FOR_SPECIFIC_TIST(pAC, i)) {
							Y2_GET_TIST_LOW_VAL(pAC->IoBase, &LowVal);
							if ((pAC->MinTistHi != pAC->GIni.GITimeStampCnt) ||
								(pAC->MinTistLo < LowVal)) {
								/* time is up now */
								SK_CLR_STATE_FOR_PORT(pAC, i);
								SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DUMP,
									("Got expected Tist on Port %c (now 0x%X)!!!\n",
									'A' + i, pAC->AdapterResetState));
#ifdef Y2_SYNC_CHECK
								pAC->FramesWithoutSyncCheck =
								Y2_RESYNC_WATERMARK;						
#endif
							} else {
								SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DUMP,
									("Got Tist %l:%l on Port %c but still waiting\n",
									pAC->GIni.GITimeStampCnt, pAC->MinTistLo,
									'A' + i));
							}
						}
					}
#ifndef Y2_RECOVERY
					if (!SK_ADAPTER_WAITING_FOR_TIST(pAC)) {
						/* nobody needs tist anymore - turn it off */
						Y2_DISABLE_TIST(pAC->IoBase);
						SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DUMP,
						("Turn off Tist !!!\n"));
					}
#endif
				} else if (OpCode == OP_TXINDEXLE) {
					/*
					 * change OpCode to notify the folowing code
					 * to ignore the done index from this LE
					 * unfortunately tist LEs will be generated only
					 * for RxStat LEs
					 * so in order to get a safe Done index for a
					 * port currently waiting for a tist we have to
					 * get the done index directly from the BMU
					 */
					OpCode = OP_MOD_TXINDEX;
					SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DUMP,
						("Mark unusable TX_INDEX LE!!!\n"));
				} else {
					if (SK_PORT_WAITING_FOR_TIST(pAC, Port)) {
						SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DUMP, 
							("Ignore LE 0x%X on Port %c!!!\n",
							OpCode, 'A' + Port));
						OpCode = OP_MOD_LE;
#ifdef Y2_LE_CHECK
						/* mark entries invalid */
						pAC->LastOpc = 0xFF;
						pAC->LastPort = 3;
#endif
					}
				}
			} /* if (SK_ADAPTER_WAITING_FOR_TIST(pAC)) */
#endif





#ifdef Y2_LE_CHECK
			if (!HW_IS_EXT_LE_FORMAT(pAC) && pAC->LastOpc != 0xFF) {
				/* last opc is valid
				 * check if current opcode follows last opcode
				 */
				if ((((OpCode & OP_RXTIMESTAMP) == OP_RXTIMESTAMP) && (pAC->LastOpc != OP_RXSTAT)) ||
				    (((OpCode & OP_RXCHKS) == OP_RXCHKS) && (pAC->LastOpc != OP_RXTIMESTAMP)) ||
				    ((OpCode == OP_RXSTAT) && (pAC->LastOpc != OP_RXCHKS))) {

					/* opcode sequence broken
					 * current LE is invalid
					 */

					if (pAC->LastOpc == OP_RXTIMESTAMP) {
						/* force invalid checksum */
						pLE->St.StUn.StRxTCPCSum.RxTCPSum1 = 1;
						pLE->St.StUn.StRxTCPCSum.RxTCPSum2 = 0;
						OpCode = pAC->LastOpc = OP_RXCHKS;
						Port = pAC->LastPort;
					} else if (pAC->LastOpc == OP_RXCHKS) {
						/* force invalid frame */
						Port = pAC->LastPort;
						pLE->St.Stat.BufLen = 64;
						pLE->St.StUn.StRxStatWord = GMR_FS_CRC_ERR;
						OpCode = pAC->LastOpc = OP_RXSTAT;
#ifdef Y2_SYNC_CHECK
						/* force rx sync check */
						pAC->FramesWithoutSyncCheck = Y2_RESYNC_WATERMARK;
#endif
					} else if (pAC->LastOpc == OP_RXSTAT) {
						/* create dont care tist */
						pLE->St.StUn.StRxTimeStamp = 0;
						OpCode = pAC->LastOpc = OP_RXTIMESTAMP;
						/* dont know the port yet */
					} else {
#ifdef DEBUG
						SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
							("Unknown LastOpc %X for Timestamp on port %c.\n",
							pAC->LastOpc, Port));
#endif
					}
				}
			}
#endif

			switch (OpCode) {
			case OP_RXSTAT:
#ifdef Y2_RECOVERY
				pAC->LastOpc = OP_RXSTAT;
#endif
				/* 
				** This is always the last Status LE belonging
				** to a received packet -> handle it...
				*/

				HandleReceives(
					pAC,
#ifdef SK_EXTREME
					CSS_GET_PORT(Port),
#else
 					Port,
#endif
					STLE_GET_LEN(pLE),
					STLE_GET_FRSTATUS(pLE),
					pAC->StatusLETable.Bmu.Stat.TcpSum1,
					pAC->StatusLETable.Bmu.Stat.TcpSum2,
					pAC->StatusLETable.Bmu.Stat.RxTimeStamp,
                                        pAC->StatusLETable.Bmu.Stat.VlanId
#ifdef SK_EXTREME
					,Port
#endif
                                        );
#ifdef CONFIG_SK98LIN_NAPI
				if (*WorkDone >= WorkToDo) {
					break;
				}
				(*WorkDone)++;
#endif
				break;
			case OP_RXVLAN:
				/* this value will be used for next RXSTAT */
				pAC->StatusLETable.Bmu.Stat.VlanId = STLE_GET_VLAN(pLE);
				break;
			case OP_RXTIMEVLAN:
				/* this value will be used for next RXSTAT */
				pAC->StatusLETable.Bmu.Stat.VlanId = STLE_GET_VLAN(pLE);
				/* fall through */
			case OP_RXTIMESTAMP:
				/* this value will be used for next RXSTAT */
				pAC->StatusLETable.Bmu.Stat.RxTimeStamp = STLE_GET_TIST(pLE);
#ifdef Y2_RECOVERY
				pAC->LastOpc = OP_RXTIMESTAMP;
				pAC->LastPort = Port;
#endif
				break;
			case OP_RXCHKSVLAN:
				/* this value will be used for next RXSTAT */
				pAC->StatusLETable.Bmu.Stat.VlanId = STLE_GET_VLAN(pLE);
				/* fall through */
			case OP_RXCHKS:
				/* this value will be used for next RXSTAT */
				pAC->StatusLETable.Bmu.Stat.TcpSum1 = STLE_GET_TCP1(pLE);
				pAC->StatusLETable.Bmu.Stat.TcpSum2 = STLE_GET_TCP2(pLE);
#ifdef Y2_RECOVERY
				pAC->LastPort = Port;
				pAC->LastOpc = OP_RXCHKS;
#endif
				break;
			case OP_RSS_HASH:
				/* this value will be used for next RXSTAT */
#if 0
				pAC->StatusLETable.Bmu.Stat.RssHashValue = STLE_GET_RSS(pLE);
#endif
				break;
			case OP_TXINDEXLE:
				/*
				** :;:; TODO
				** it would be possible to check for which queues
				** the index has been changed and call 
				** CheckForSendComplete() only for such queues
				*/
				STLE_GET_DONE_IDX(pLE,LowVal,HighVal);
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
					("LowVal: 0x%x HighVal: 0x%x\n", LowVal, HighVal));

				/*
				** It would be possible to check whether we really
				** need the values for second port or sync queue, 
				** but I think checking whether we need them is 
				** more expensive than the calculation
				*/
				DoneTxA[0] = STLE_GET_DONE_IDX_TXA1(LowVal,HighVal);
				DoneTxS[0] = STLE_GET_DONE_IDX_TXS1(LowVal,HighVal);
				DoneTxA[1] = STLE_GET_DONE_IDX_TXA2(LowVal,HighVal);
				DoneTxS[1] = STLE_GET_DONE_IDX_TXS2(LowVal,HighVal);

				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
					("DoneTxa1 0x%x DoneTxS1: 0x%x DoneTxa2 0x%x DoneTxS2: 0x%x\n",
					DoneTxA[0], DoneTxS[0], DoneTxA[1], DoneTxS[1]));

				NewDone = SK_TRUE;
				break;
#ifdef USE_TIST_FOR_RESET
			case OP_MOD_TXINDEX:
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DUMP,
					("OP_MOD_TXINDEX\n"));
				SK_IN16(pAC->IoBase, Q_ADDR(Q_XA1, Q_DONE), &DoneTxA[0]);
				if (pAC->GIni.GIMacsFound > 1) {
					SK_IN16(pAC->IoBase, Q_ADDR(Q_XA2, Q_DONE), &DoneTxA[1]);
				}
				NewDone = SK_TRUE;
				break;
			case OP_MOD_LE:
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DUMP,
				("Ignore marked LE on port in Reset\n"));
				break;
#endif

			default:
				/* 
				** Have to handle the illegal Opcode in Status LE 
				*/
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
					("Unexpected OpCode\n"));
				break;
			}

#ifdef Y2_RECOVERY
			OpCode = STLE_GET_OPC(pLE) & ~HW_OWNER;
			STLE_SET_OPC(pLE, OpCode);
#else
			/* 
			** Reset own bit we have to do this in order to detect a overflow 
			*/
			STLE_SET_OPC(pLE, SW_OWNER);
#endif
		} /* while (OWN_OF_FIRST_LE(&pAC->StatusLETable) == HW_OWNER) */

		/* 
		** Now handle any new transmit complete 
		*/
		if (NewDone) {
			SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
				("Done Index for Tx BMU has been changed\n"));
			for (Port = 0; Port < pAC->GIni.GIMacsFound; Port++) {
				/* 
				** Do we have a new Done idx ? 
				*/
				if (DoneTxA[Port] != GET_DONE_INDEX(&pAC->TxPort[Port][0].TxALET)) {
					SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
						("Check TxA%d\n", Port + 1));
					CheckForSendComplete(pAC, pAC->IoBase, Port,
						&(pAC->TxPort[Port][0].TxAQ_working),
						&pAC->TxPort[Port][0].TxALET,
						DoneTxA[Port]);
				} else {
					SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
						("No changes for TxA%d\n", Port + 1));
				}
#ifdef USE_SYNC_TX_QUEUE
				if (HW_SYNC_TX_SUPPORTED(pAC)) {
					/* 
					** Do we have a new Done idx ? 
					*/
					if (DoneTxS[Port] !=
						GET_DONE_INDEX(&pAC->TxPort[Port][0].TxSLET)) {
						SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
							SK_DBGCAT_DRV_INT_SRC,
							("Check TxS%d\n", Port));
						CheckForSendComplete(pAC, pAC->IoBase, Port,
							&(pAC->TxPort[Port][0].TxSQ_working),
							&pAC->TxPort[Port][0].TxSLET,
							DoneTxS[Port]);
					} else {
						SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
							SK_DBGCAT_DRV_INT_SRC,
							("No changes for TxS%d\n", Port));
					}
				}
#endif
			}
		}
		NewDone = SK_FALSE;

		/* 
		** Check whether we have to refill our RX table  
		*/
		if (HW_FEATURE(pAC, HWF_WA_DEV_420)) {
			if (NbrRxBuffersInHW < MAX_NBR_RX_BUFFERS_IN_HW) {
				for (Port = 0; Port < pAC->GIni.GIMacsFound; Port++) {
					SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
						("Check for refill of RxBuffers on Port %c\n", 'A' + Port));
					FillReceiveTableYukon2(pAC, pAC->IoBase, Port);
				}
			}
		} else {
			for (Port = 0; Port < pAC->GIni.GIMacsFound; Port++) {
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
					("Check for refill of RxBuffers on Port %c\n", 'A' + Port));
				if (NUM_FREE_LE_IN_TABLE(&pAC->RxPort[Port].RxLET) >= 64) {
					FillReceiveTableYukon2(pAC, pAC->IoBase, Port);
				}
			}
		}
#ifdef CONFIG_SK98LIN_NAPI
		if (*WorkDone >= WorkToDo) {
			break;
		}
#endif
	} while (OWN_OF_FIRST_LE(&pAC->StatusLETable) == HW_OWNER);

	/* 
	** Clear status BMU 
	*/
	if (handledStatLE) {
		SK_OUT32(pAC->IoBase, STAT_CTRL, SC_STAT_CLR_IRQ);

		/* Reread interrupt source */
		SK_IN32(pAC->IoBase, B0_ISRC, &IntSrc);
	}

	return(handledStatLE);
}	/* HandleStatusLEs */

/*****************************************************************************
 *
 *	AllocateAndInitLETables - allocate memory for the LETable and init
 *
 * Description:
 *	This function will allocate space for the LETable and will also  
 *	initialize them. The size of the tables must have been specified 
 *	before.
 *
 * Arguments:
 *	pAC - A pointer to the adapter context struct.
 *
 * Returns:
 *	SK_TRUE  - all LETables initialized
 *	SK_FALSE - failed
 */
static SK_BOOL AllocateAndInitLETables(
SK_AC *pAC)  /* pointer to adapter context */
{
	char           *pVirtMemAddr;
	char           *pAddr;
	dma_addr_t     pPhysMemAddr = 0;
	dma_addr_t     pPhysOrgMemAddr = 0;
	SK_U32         CurrMac;
	unsigned       Size;
	unsigned       Aligned;
	unsigned       Alignment;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("==> AllocateAndInitLETables()\n"));

	/*
	** Determine how much memory we need with respect to alignment
	*/
	Alignment = (SK_U32) LE_TAB_SIZE(NUMBER_OF_ST_LE);
	Size = 0;
	for (CurrMac = 0; CurrMac < pAC->GIni.GIMacsFound; CurrMac++) {
		SK_ALIGN_SIZE(LE_TAB_SIZE(NUMBER_OF_RX_LE), Alignment, Aligned);
		Size += Aligned;
		SK_ALIGN_SIZE(LE_TAB_SIZE(NUMBER_OF_TX_LE), Alignment, Aligned);
		Size += Aligned;
		SK_ALIGN_SIZE(LE_TAB_SIZE(TXS_MAX_LE), Alignment, Aligned);
		Size += Aligned;
	}
	SK_ALIGN_SIZE(LE_TAB_SIZE(NUMBER_OF_ST_LE), Alignment, Aligned);
	Size += Aligned;
	Size += Alignment;
	pAC->SizeOfAlignedLETables = Size;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT, 
			("Need %08x bytes in total\n", Size));
	
	/*
	** Allocate the memory
	*/
	pVirtMemAddr = pci_alloc_consistent(pAC->PciDev, Size, &pPhysMemAddr);
	pAC->pVirtMemAddr = pVirtMemAddr;

	if (pVirtMemAddr == NULL) {
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, 
			SK_DBGCAT_INIT | SK_DBGCAT_DRV_ERROR,
			("AllocateAndInitLETables: kernel malloc failed!\n"));
		return (SK_FALSE); 
	}

	/* 
	** Initialize the memory
	*/
	SK_MEMSET(pVirtMemAddr, 0, Size);
	pPhysOrgMemAddr = pPhysMemAddr;
	pAddr = (void *) (SK_U32) pPhysMemAddr;
	ALIGN_ADDR(pAddr, Alignment); /* Macro defined in skgew.h */
	pPhysMemAddr = (dma_addr_t) (SK_U32) pAddr;
	pVirtMemAddr += pPhysMemAddr - pPhysOrgMemAddr;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("Virtual address of LETab is %8p!\n", pVirtMemAddr));
	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("Phys address of LETab is %8p!\n", (void *) pPhysMemAddr));

	for (CurrMac = 0; CurrMac < pAC->GIni.GIMacsFound; CurrMac++) {
		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
			("RxLeTable for Port %c", 'A' + CurrMac));
		SkGeY2InitSingleLETable(
			pAC,
			&pAC->RxPort[CurrMac].RxLET,
			NUMBER_OF_RX_LE,
			pVirtMemAddr,
			(SK_U32) (pPhysMemAddr & 0xffffffff),
			(SK_U32) (((SK_U64) pPhysMemAddr) >> 32));

		SK_ALIGN_SIZE(LE_TAB_SIZE(NUMBER_OF_RX_LE), Alignment, Aligned);
		pVirtMemAddr += Aligned;
		pPhysMemAddr += Aligned;

		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
			("TxALeTable for Port %c", 'A' + CurrMac));
		SkGeY2InitSingleLETable(
			pAC,
			&pAC->TxPort[CurrMac][0].TxALET,
			NUMBER_OF_TX_LE,
			pVirtMemAddr,
			(SK_U32) (pPhysMemAddr & 0xffffffff),
			(SK_U32) (((SK_U64) pPhysMemAddr) >> 32));

		SK_ALIGN_SIZE(LE_TAB_SIZE(NUMBER_OF_TX_LE), Alignment, Aligned);
		pVirtMemAddr += Aligned;
		pPhysMemAddr += Aligned;

		SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
			("TxSLeTable for Port %c", 'A' + CurrMac));
		SkGeY2InitSingleLETable(
			pAC,
			&pAC->TxPort[CurrMac][0].TxSLET,
			TXS_MAX_LE,
			pVirtMemAddr,
			(SK_U32) (pPhysMemAddr & 0xffffffff),
			(SK_U32) (((SK_U64) pPhysMemAddr) >> 32));

		SK_ALIGN_SIZE(LE_TAB_SIZE(TXS_MAX_LE), Alignment, Aligned);
		pVirtMemAddr += Aligned;
		pPhysMemAddr += Aligned;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,("StLeTable"));

	SkGeY2InitSingleLETable(
		pAC,
		&pAC->StatusLETable,
		NUMBER_OF_ST_LE,
		pVirtMemAddr,
		(SK_U32) (pPhysMemAddr & 0xffffffff),
		(SK_U32) (((SK_U64) pPhysMemAddr) >> 32));

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT, 
		("<== AllocateAndInitLETables(OK)\n"));
	return(SK_TRUE);
}	/* AllocateAndInitLETables */

/*****************************************************************************
 *
 *	AllocatePacketBuffersYukon2 - allocate packet and fragment buffers
 *
 * Description:
 *      This function will allocate space for the packets and fragments
 *
 * Arguments:
 *      pAC - A pointer to the adapter context struct.
 *
 * Returns:
 *      SK_TRUE  - Memory was allocated correctly
 *      SK_FALSE - An error occured
 */
static SK_BOOL AllocatePacketBuffersYukon2(
SK_AC *pAC)  /* pointer to adapter context */
{
	SK_PACKET       *pRxPacket;
	SK_PACKET       *pTxPacket;
	SK_U32           CurrBuff;
	SK_U32           CurrMac;
	unsigned long    Flags; /* needed for POP/PUSH functions */

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("==> AllocatePacketBuffersYukon2()"));

	for (CurrMac = 0; CurrMac < pAC->GIni.GIMacsFound; CurrMac++) {
		/* 
		** Allocate RX packet space, initialize the packets and
		** add them to the RX waiting queue. Waiting queue means 
		** that packet and fragment are initialized, but no sk_buff
		** has been assigned to it yet.
		*/
		pAC->RxPort[CurrMac].ReceivePacketTable = 
			kmalloc((RX_MAX_NBR_BUFFERS * sizeof(SK_PACKET)), GFP_KERNEL);

		if (pAC->RxPort[CurrMac].ReceivePacketTable == NULL) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_INIT | SK_DBGCAT_DRV_ERROR,
				("AllocatePacketBuffersYukon2: no mem RxPkts (port %i)",CurrMac));
			break;
		} else {
			SK_MEMSET(pAC->RxPort[CurrMac].ReceivePacketTable, 0, 
				(RX_MAX_NBR_BUFFERS * sizeof(SK_PACKET)));

			pRxPacket = pAC->RxPort[CurrMac].ReceivePacketTable;

			for (CurrBuff=0;CurrBuff<RX_MAX_NBR_BUFFERS;CurrBuff++) {
				pRxPacket->pFrag = &(pRxPacket->FragArray[0]);
				pRxPacket->NumFrags = 1;
				PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->RxPort[CurrMac].RxQ_waiting, pRxPacket);
				pRxPacket++;
			}
		}

		/*
		** Allocate TX packet space, initialize the packets and
		** add them to the TX free queue. Free queue means that
		** packet is available and initialized, but no fragment
		** has been assigned to it. (Must be done at TX side)
		*/
		pAC->TxPort[CurrMac][0].TransmitPacketTable = 
			kmalloc((TX_MAX_NBR_BUFFERS * sizeof(SK_PACKET)), GFP_KERNEL);

		if (pAC->TxPort[CurrMac][0].TransmitPacketTable == NULL) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_INIT | SK_DBGCAT_DRV_ERROR,
				("AllocatePacketBuffersYukon2: no mem TxPkts (port %i)",CurrMac));
			kfree(pAC->RxPort[CurrMac].ReceivePacketTable);
			return(SK_FALSE);
		} else {
			SK_MEMSET(pAC->TxPort[CurrMac][0].TransmitPacketTable, 0, 
				(TX_MAX_NBR_BUFFERS * sizeof(SK_PACKET)));
		
			pTxPacket = pAC->TxPort[CurrMac][0].TransmitPacketTable;

			for (CurrBuff=0;CurrBuff<TX_MAX_NBR_BUFFERS;CurrBuff++) {
				PUSH_PKT_AS_LAST_IN_QUEUE(&pAC->TxPort[CurrMac][0].TxQ_free, pTxPacket);
				pTxPacket++;
			}
		}
	} /* end for (CurrMac = 0; CurrMac < pAC->GIni.GIMacsFound; CurrMac++) */

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_INIT,
		("<== AllocatePacketBuffersYukon2 (OK)\n"));
	return(SK_TRUE);

}	/* AllocatePacketBuffersYukon2 */

/*****************************************************************************
 *
 *	FreeLETables - release allocated memory of LETables
 *
 * Description:
 *      This function will free all resources of the LETables
 *
 * Arguments:
 *      pAC - A pointer to the adapter context struct.
 *
 * Returns: N/A
 */
static void FreeLETables(
SK_AC *pAC)  /* pointer to adapter control context */
{
	dma_addr_t	pPhysMemAddr;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("==> FreeLETables()\n"));
	
	/*
	** The RxLETable is the first of all LET. 
	** Therefore we can use its address for the input 
	** of the free function.
	*/
	pPhysMemAddr = (((SK_U64) pAC->RxPort[0].RxLET.pPhyLETABHigh << (SK_U64) 32) | 
			((SK_U64) pAC->RxPort[0].RxLET.pPhyLETABLow));

	/* free continuous memory */
	pci_free_consistent(pAC->PciDev, pAC->SizeOfAlignedLETables,
			    pAC->pVirtMemAddr, pPhysMemAddr);

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("<== FreeLETables()\n"));
}	/* FreeLETables */

/*****************************************************************************
 *
 *	FreePacketBuffers - free's all packet buffers of an adapter
 *
 * Description:
 *      This function will free all previously allocated memory of the 
 *	packet buffers.
 *
 * Arguments:
 *      pAC - A pointer to the adapter context struct.
 *
 * Returns: N/A
 */
static void FreePacketBuffers(
SK_AC *pAC)  /* pointer to adapter control context */
{
	int Port;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("==> FreePacketBuffers()\n"));
	
	for (Port = 0; Port < pAC->GIni.GIMacsFound; Port++) {
		kfree(pAC->RxPort[Port].ReceivePacketTable);
		kfree(pAC->TxPort[Port][0].TransmitPacketTable);
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("<== FreePacketBuffers()\n"));
}	/* FreePacketBuffers */

/*****************************************************************************
 *
 * 	AllocAndMapRxBuffer - fill one buffer into the receive packet/fragment
 *
 * Description:
 *	The function allocates a new receive buffer and assigns it to the
 *	the passsed receive packet/fragment
 *
 * Returns:
 *	SK_TRUE - a buffer was allocated and assigned
 *	SK_FALSE - a buffer could not be added
 */
static SK_BOOL AllocAndMapRxBuffer(
SK_AC      *pAC,        /* pointer to the adapter control context */
SK_PACKET  *pSkPacket,  /* pointer to packet that is to fill      */
int         Port)       /* port the packet belongs to             */
{
	struct sk_buff *pMsgBlock;  /* pointer to a new message block  */
	SK_U64          PhysAddr;   /* physical address of a rx buffer */

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
		("--> AllocAndMapRxBuffer (Port: %i)\n", Port));

	pMsgBlock = alloc_skb(pAC->RxPort[Port].RxBufSize, GFP_ATOMIC);
	if (pMsgBlock == NULL) {
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
			SK_DBGCAT_DRV_RX_PROGRESS | SK_DBGCAT_DRV_ERROR,
			("%s: Allocation of rx buffer failed !\n",
			pAC->dev[Port]->name));
		SK_PNMI_CNT_NO_RX_BUF(pAC, pAC->RxPort[Port].PortIndex);
		return(SK_FALSE);
	}
	skb_reserve(pMsgBlock, 8);

	PhysAddr = (SK_U64) pci_map_page(pAC->PciDev,
		virt_to_page(pMsgBlock->data),
		((unsigned long) pMsgBlock->data &
		~PAGE_MASK),
		pAC->RxPort[Port].RxBufSize,
		PCI_DMA_FROMDEVICE);

	pSkPacket->pFrag->pVirt   = pMsgBlock->data;
	pSkPacket->pFrag->pPhys   = PhysAddr;
	pSkPacket->pFrag->FragLen = pAC->RxPort[Port].RxBufSize; /* for correct unmap */
	pSkPacket->pMBuf          = pMsgBlock;	
	pSkPacket->PacketLen      = pAC->RxPort[Port].RxBufSize;

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_RX_PROGRESS,
		("<-- AllocAndMapRxBuffer\n"));

	return (SK_TRUE);
}	/* AllocAndMapRxBuffer */

/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/
