/**
 * @File    IxAtmCodeletSwLb.c
 *
 * @date    20-May-2002
 *
 * @brief   Rx and Tx Transport for IXP400 Atm Codelet (IxAtmCodelet)
 *
 * Functions implementated in this file:
 *   ixAtmSwLbInit
 *   ixAtmSwLbChannelsProvision
 *   ixAtmSwLbStatsGet (IxAtmCodeletStats *stats)
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

/*
 * User defined include files.
 */
#if defined(__wince) && defined(IX_USE_SERCONSOLE)
	#include "IxSerConsole.h"
	#define printf ixSerPrintf
	#define gets ixSerGets
#endif


#include "IxOsal.h"
#include "IxAtmCodelet_p.h"
#include "IxAtmCodelet.h"
#include "IxAtmdAcc.h"

/*
 * #defines and macros
 */

/*
 * Typedefs
 */
typedef struct {
    IxAtmConnId connId;
    IxAtmdAccAalType aalType;
    UINT32 bytesPerCell;
} VcInfo;

/*
 * This structure defines a software queue that stores mbufs.
 * NOTE: The head and tail pointers are incremented each time an mbuf is added
 * to the head or removed from the tail. The range of values that the head and
 * tail pointers can assume are in the range 0....2^32. The size of this queue
 * _must_ be a power of 2. The mask is set to (size - 1). Whenever the head or
 * tail pointers are used they are masked with this mask. The following is an
 * example of how this works(size = 128, 26 elements in the queue):
 * mask = 127   (0x007f)
 * head = 35535 (0x8acf)
 * tail = 35509 (0x8ab5)
 * numElementsInQueue = (head   & mask  ) - (tail   & mask  )
 *                    = (0x8acf & 0x007f) - (0x8ab5 & 0x007f)
 *                    = (0x4f)            - (0x35)
 *                    = 79 - 53
 * numElementsInQueue = 26
 */

#define IX_ATMCODELET_SW_QUEUE_SIZE 1024

typedef struct
{
    volatile UINT32 head; /* Points to the head of the queue */
    volatile UINT32 tail; /* Points to the tail of the queue */
    UINT32 size;          /* The size of the queue           */
    UINT32 mask;          /* Head and tail mask              */
    IX_OSAL_MBUF *array[IX_ATMCODELET_SW_QUEUE_SIZE];
} IxAtmCodeletSwLbSwMbufQ;

/*
 * Static variables
 */
static VcInfo ixAtmSwLbTxVcInfoTable[IX_ATM_MAX_NUM_VC];
static VcInfo ixAtmSwLbRxVcInfoTable[IX_ATM_MAX_NUM_VC];

static IxAtmLogicalPort ixAtmSwLb_port       = 0; 
static UINT32     ixAtmSwLb_vci        = IX_ATMCODELET_START_VCI;
static UINT32     ixAtmSwLb_vpi        = IX_ATMCODELET_START_VPI;
static UINT32     ixAtmSwLb_channelIdx = 0;


/* Software queue underflow/overflow statistics counters */
static UINT32 ixAtmSwLbChannelRxSwqOvfl;
static UINT32 ixAtmSwLbChannelRxSwqUndl;

static UINT32 swLbRxToTxRatio = 0;

/*
 * mbufs are stored in this queue. This queue is filled with Transmit Done
 * buffers and is drained to fill Recieve Free queues
 */
static IxAtmCodeletSwLbSwMbufQ loopbackSwq;

/*
 * Statistics.
 */
static IxAtmCodeletStats *ixAtmSwLbStats = NULL;

/*
 * Function prototypes
 */

/* Reset stats counters */
PRIVATE void
ixAtmSwLbStatsReset (void);

/* Replenish the Rx Free queues with buffers */
void
ixAtmSwLbRxFreeLowReplenish (IxAtmdAccUserId userId);

/* Called when a PDU is recieved */
PRIVATE void
ixAtmSwLbRxCallback (IxAtmLogicalPort port,
		     IxAtmdAccUserId userId,
		     IxAtmdAccPduStatus pduStatus,
		     IxAtmdAccClpStatus clp,
		     IX_OSAL_MBUF *mbufPtr);

PRIVATE void
ixAtmSwLbRxInvalidate(IX_OSAL_MBUF * mbufPtr);


/* Return buffers to the software queue */
PRIVATE void
ixAtmSwLbTxDoneCallback (IxAtmdAccUserId userId,
			 IX_OSAL_MBUF *mbufPtr);

/* Check if the queue is full */
PRIVATE BOOL 
ixAtmQueueFullQuery (IxAtmCodeletSwLbSwMbufQ *s);

/* Check if the queue is empty */
PRIVATE BOOL 
ixAtmQueueEmptyQuery (IxAtmCodeletSwLbSwMbufQ *s);

/* Remove an entry from the queue */
PRIVATE IX_OSAL_MBUF * 
ixAtmMBufQueueGet (IxAtmCodeletSwLbSwMbufQ *s);

/* Place a buffer on the software queue */
PRIVATE void 
ixAtmSwLbMBufQueuePut (IxAtmCodeletSwLbSwMbufQ *s,
		       IX_OSAL_MBUF *buf);

/* Place an Mbuf chain on the queue */
PRIVATE void 
ixAtmChainQueuePut (IxAtmCodeletSwLbSwMbufQ *s,
		    IX_OSAL_MBUF *mbufPtr);

/*
 * Function definitions
 */

/* --------------------------------------------------------------
   Initialise the Atm Software Loopback codelet.
   -------------------------------------------------------------- */
IX_STATUS
ixAtmSwLbInit (IxAtmCodeletStats *stats, UINT32 rxToTxRatio)
{
    int i;
    IX_OSAL_MBUF *buf;
    UINT32 mbufCount;

    ixAtmSwLbStats = stats;

    /* Reset statistics */
    ixAtmSwLbStatsReset ();

    swLbRxToTxRatio = rxToTxRatio;

    IX_ATMCODELET_LOG("Using Rx:Tx ratio %d\n",swLbRxToTxRatio);


    for( i=0; i < IX_ATM_MAX_NUM_VC; i++)
    {
	ixAtmSwLbRxVcInfoTable[i].aalType = IX_ATMDACC_MAX_SERVICE_TYPE;
	ixAtmSwLbTxVcInfoTable[i].aalType = IX_ATMDACC_MAX_SERVICE_TYPE;        
    }

    /* Clear the Sw Q */
    ixOsalMemSet(&loopbackSwq, 0, sizeof(loopbackSwq));

    loopbackSwq.size = IX_ATMCODELET_SW_QUEUE_SIZE;
    loopbackSwq.mask = IX_ATMCODELET_SW_QUEUE_SIZE - 1;

    for (mbufCount=0; mbufCount<loopbackSwq.size; mbufCount++)
    {	    
        /* Get the first buffer */
        ixAtmUtilsMbufGet( IX_ATMCODELET_MBUF_SIZE, &buf);
        IX_OSAL_ASSERT(buf != 0);
        ixAtmSwLbMBufQueuePut (&loopbackSwq, buf);
    }

    return IX_SUCCESS;
}

/* --------------------------------------------------------------
   Remove all provisioned channels.
   -------------------------------------------------------------- */
IX_STATUS
ixAtmSwLbChannelsRemove( void )
{
    IX_STATUS retval;
    UINT32 i;

    retval = ixAtmUtilsAtmAllVcsDisconnect ();
    
    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to remove channels\n");
    }

    for(i=0; i < ixAtmSwLb_channelIdx; i++)
    {
	ixAtmSwLbRxVcInfoTable[i].aalType = IX_ATMDACC_MAX_SERVICE_TYPE;
	ixAtmSwLbTxVcInfoTable[i].aalType = IX_ATMDACC_MAX_SERVICE_TYPE;        

    }  

    while(!ixAtmQueueEmptyQuery (&loopbackSwq))
    {
       ixAtmUtilsMbufFree (ixAtmMBufQueueGet (&loopbackSwq));                
    }


    ixAtmSwLb_port           = 0;
    ixAtmSwLb_vci        = IX_ATMCODELET_START_VCI;
    ixAtmSwLb_vpi        = IX_ATMCODELET_START_VPI;
    ixAtmSwLb_channelIdx = 0;

    return retval;

}




/* --------------------------------------------------------------
   Provision channels specifying the number of ports and channels.
   -------------------------------------------------------------- */
IX_STATUS
ixAtmSwLbChannelsProvision (UINT32 numPorts,
			    UINT32 numChannels,
			    IxAtmdAccAalType aalType)
{
    IX_STATUS retval;
    UINT32 i;
    IxAtmServiceCategory atmServiceCat = IX_ATM_UBR; /* Set to default */

    /* Parameter validation */
    if ((numPorts == 0) || (numChannels == 0))
    {
	IX_ATMCODELET_LOG("Invalid parameters to channesls provision \n");
	return IX_FAIL;
    }

    /* N.B. channelIdx is static */
    for (i=0; i<numChannels; i++,ixAtmSwLb_channelIdx++)
    {

	/* Save the aalType and bytesPerCell in the Rx & Tx Vc Info tables */
	ixAtmSwLbRxVcInfoTable[ixAtmSwLb_channelIdx].aalType = aalType;
	ixAtmSwLbTxVcInfoTable[ixAtmSwLb_channelIdx].aalType = aalType;
	if (aalType == IX_ATMDACC_AAL0_52)
	{
	    ixAtmSwLbTxVcInfoTable[ixAtmSwLb_channelIdx].bytesPerCell 
                = IX_ATM_AAL0_52_CELL_SIZE_NO_HEC;
	    ixAtmSwLbRxVcInfoTable[ixAtmSwLb_channelIdx].bytesPerCell 
                = IX_ATM_AAL0_52_CELL_SIZE_NO_HEC;
	}
	else
	{
	    ixAtmSwLbTxVcInfoTable[ixAtmSwLb_channelIdx].bytesPerCell 
                = IX_ATM_AAL0_48_CELL_PAYLOAD_SIZE;
	    ixAtmSwLbRxVcInfoTable[ixAtmSwLb_channelIdx].bytesPerCell
                = IX_ATM_AAL0_48_CELL_PAYLOAD_SIZE;
	}

	/* Verify whether ATM codelet is going to use Real-time VCs for
	 ( UTOPIA/Remote loopback mode. Otherwise set to UBR VCs by default */
	if (ixAtmUtilsAtmRtVcsGet())
	{
	    if (IX_ATMCODELET_8VCS > i)
	    {
		/* 1st 8VCs setup with VBR */
		atmServiceCat = IX_ATM_VBR;
                IX_ATMCODELET_LOG("Setting up VBR: ");
	    }
	    else if ((IX_ATMCODELET_8VCS <= i) && 
		     (IX_ATMCODELET_16VCS > i))
	    {
		/* 2nd 8VCs setup with CBR */
		atmServiceCat = IX_ATM_CBR;
                IX_ATMCODELET_LOG("Setting up CBR: ");
	    }
	    else 
	    {
		/* Balance is 16 UBR VCs */
		atmServiceCat = IX_ATM_UBR;
                IX_ATMCODELET_LOG("Setting up UBR: ");
	    }
	}
	else
	{
	    IX_ATMCODELET_LOG("Setting up UBR: ");
	}

	/* Use a different port per channel, depending on the maximum
	 * number of ports configured
	 */
	retval = ixAtmUtilsAtmVcRegisterConnect(ixAtmSwLb_port,
						 ixAtmSwLb_vpi,
						 ixAtmSwLb_vci,
						 aalType,
						 atmServiceCat,
						 IX_ATM_RX_A,
						 ixAtmSwLbRxCallback,
						 IX_ATMCODELET_RX_FREE_Q_LOW_THRESHOLD,
						 ixAtmSwLbTxDoneCallback,
						 ixAtmSwLbRxFreeLowReplenish,
						 ixAtmSwLb_channelIdx,
						 &ixAtmSwLbRxVcInfoTable[ixAtmSwLb_channelIdx].connId,
						 &ixAtmSwLbTxVcInfoTable[ixAtmSwLb_channelIdx].connId);
	
	if (retval != IX_SUCCESS)
	{
	    IX_ATMCODELET_LOG("Sw loopback VC setup failed for port %2u vpi %3u vci %5u\n", 
			      ixAtmSwLb_port, ixAtmSwLb_vpi, ixAtmSwLb_vci);
	    return IX_FAIL;
	}
	else
	{
	    IX_ATMCODELET_LOG("Sw loopback VC configured for port %2u vpi %3u vci %5u\n", 
			      ixAtmSwLb_port, ixAtmSwLb_vpi, ixAtmSwLb_vci);
	}

	/* Wrap the port back to 0 when numPortss used already */
	ixAtmSwLb_port++;
	if (ixAtmSwLb_port >= (IxAtmLogicalPort)numPorts)
        {
	    ixAtmSwLb_port = 0;
        }

	/* use the next vci */
	ixAtmSwLb_vci++;
    }

    return IX_SUCCESS;
}

/*
 * Replenish buffers to the RxFree queue. Buffers are taken from
 * the software queue.
 */
void
ixAtmSwLbRxFreeLowReplenish (IxAtmdAccUserId userId)
{
    IX_OSAL_MBUF *mBuf;
    IX_STATUS retval;
    IxAtmConnId connId;
    UINT32 numFreeEntries;
    UINT32 cnt;

    connId = ixAtmSwLbRxVcInfoTable[userId].connId;

    /* Get the number of free entries in the RX Free queue */
    retval = ixAtmdAccRxVcFreeEntriesQuery (connId, &numFreeEntries);
    if (retval != IX_SUCCESS)
    {
	IX_ATMCODELET_LOG("Failed to query depth of Rx Free Q for connection %u\n",
			  connId);
	return;
    }


    /* Replenish Rx buffers  */
    for (cnt=0; cnt<numFreeEntries; cnt++)
    {
	if (ixAtmQueueEmptyQuery (&loopbackSwq))
	{
	    ixAtmSwLbChannelRxSwqUndl++;
	    ixAtmUtilsMbufGet(IX_ATMCODELET_MBUF_SIZE, &mBuf);

	    if (mBuf == NULL)
	    {
		IX_ATMCODELET_LOG("Failed to get rx free mbuffers\n");
		return;
	    }
	}
	else
	{
	    mBuf = ixAtmMBufQueueGet (&loopbackSwq);
	}

	/* 
	 * Set the number of bytes of data in this MBUF to the number of
	 * payloads that will fit evenly into IX_ATMCODELET_MBUF_SIZE.
	 */
	IX_OSAL_MBUF_MLEN(mBuf) = 
	    (IX_ATMCODELET_MBUF_SIZE / ixAtmSwLbRxVcInfoTable[userId].bytesPerCell) *
	    ixAtmSwLbRxVcInfoTable[userId].bytesPerCell;

        ixAtmSwLbRxInvalidate(mBuf);

	/* Send free buffers to NPE */
	retval = ixAtmdAccRxVcFreeReplenish (connId, mBuf);

	if (retval != IX_SUCCESS)
	{
	    /* Free the allocated buffer */
	    ixAtmUtilsMbufFree(mBuf);
	    IX_ATMCODELET_LOG("Failed to pass Rx free buffers to Atmd \n");
	    return;
	}
	/* Update stats */
	ixAtmSwLbStats->rxFreeBuffers++;
    }
}

/* --------------------------------------------------------------
   Return the stats.
   -------------------------------------------------------------- */
void
ixAtmSwLbStatsGet (IxAtmCodeletStats *stats)
{
    *stats = *ixAtmSwLbStats;
}


/* --------------------------------------------------------------
   Reset counters.
   -------------------------------------------------------------- */
PRIVATE void
ixAtmSwLbStatsReset (void)
{
    if (NULL != ixAtmSwLbStats)
    {
	ixAtmSwLbStats->txPdus = 0;
	ixAtmSwLbStats->txBytes = 0;
	ixAtmSwLbStats->rxPdus = 0;
	ixAtmSwLbStats->rxBytes = 0;
	ixAtmSwLbStats->txDonePdus = 0;
	ixAtmSwLbStats->rxFreeBuffers = 0;
	ixAtmSwLbStats->txPdusSubmitFail = 0;
	ixAtmSwLbStats->txPdusSubmitBusy = 0;
	ixAtmSwLbStats->rxPdusInvalid = 0;
    }
}

/* --------------------------------------------------------------
   The calback is called when the TxDone queue reaches a
   specified level. The mubf passed to this function is recycled
   by placing it on the the softwre queue.
   -------------------------------------------------------------- */
PRIVATE void
ixAtmSwLbTxDoneCallback (IxAtmdAccUserId userId,
			 IX_OSAL_MBUF *mbufPtr)
{
    ixAtmSwLbStats->txDonePdus++;

    if (ixAtmQueueFullQuery (&loopbackSwq))
    {
	ixAtmSwLbChannelRxSwqOvfl++;
	ixAtmUtilsMbufFree(mbufPtr);
    }
    else
    {
	ixAtmChainQueuePut (&loopbackSwq,
			    mbufPtr);
    }
}

/* --------------------------------------------------------------
   The callback is called when an Atm Packet has been recieved. If
   the PDU is invalid or it is a buffer returned during a
   dissconnect by IxAtmdAcc then return the buffer to the vxWorks
   pool. Otherwise transmit the PDU; this is where the loopback
   occurs. 1 PDU is transmitted for every rxToTxRatio PDUs
   received.
   -------------------------------------------------------------- */
PRIVATE void
ixAtmSwLbRxCallback (IxAtmLogicalPort port,
		     IxAtmdAccUserId userId,
		     IxAtmdAccPduStatus pduStatus,
		     IxAtmdAccClpStatus clp,
		     IX_OSAL_MBUF *mbufPtr)
{
    IX_STATUS retval = IX_FAIL;
    static UINT32 rxCount = 0;
    UINT32 sizeInCells;
    UINT32 rxBytes = 0;

    if (mbufPtr == NULL)
    {
	IX_ATMCODELET_LOG ("rxCallback called with NULL mbufPtr\n");
	return;
    }

    rxBytes = IX_OSAL_MBUF_PKT_LEN(mbufPtr);

    /* Size of the data transmitted in CELLS */
    sizeInCells = rxBytes/ixAtmSwLbRxVcInfoTable[userId].bytesPerCell;

    if (pduStatus == IX_ATMDACC_MBUF_RETURN)
    {
	/* Return this buffer to the pool */
	ixAtmUtilsMbufFree (mbufPtr);
	return;
    }
    
    if ((pduStatus != IX_ATMDACC_AAL0_VALID) && (pduStatus != IX_ATMDACC_AAL5_VALID))
    {	
	ixAtmSwLbStats->rxPdusInvalid++;
	/* Return buffer to the pool */
        ixAtmUtilsMbufFree(mbufPtr);
	return;
    }

    /* Update stats */
    ixAtmSwLbStats->rxPdus++;
    ixAtmSwLbStats->rxBytes += rxBytes;
	
    /* Only send 1 PDU for every rxToTxRatio PDUs received */
    if (rxCount++ == 0)
    {
	/* Tx the Rx data, including the trailer */
	retval = ixAtmdAccTxVcPduSubmit (ixAtmSwLbTxVcInfoTable[userId].connId,
					 mbufPtr,
					 IX_ATMCODELET_DEFAULT_CLP,
					 sizeInCells );

	if (retval != IX_SUCCESS)
	{
	    ixAtmSwLbStats->txPdusSubmitFail++;
	    ixAtmUtilsMbufFree (mbufPtr);
	}
    }
    else
    {
	/* Do not process this Pdu. Free the buffer */
        ixAtmUtilsMbufFree (mbufPtr);	
    }

    if (rxCount == swLbRxToTxRatio)
    {
	rxCount = 0;
    }

    if (retval == IX_SUCCESS)
    {
	/* Update status */
	ixAtmSwLbStats->txPdus++;
	ixAtmSwLbStats->txBytes += rxBytes;
    }

    return;
}


/* ---------------------------------------------------
*/
PRIVATE void
ixAtmSwLbRxInvalidate(IX_OSAL_MBUF * mbufPtr)
{
    while (mbufPtr != NULL)
    {
      IX_OSAL_CACHE_INVALIDATE(IX_OSAL_MBUF_MDATA(mbufPtr), IX_OSAL_MBUF_MLEN(mbufPtr));
      mbufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);
    }
    return;
}




/* --------------------------------------------------------------
   Software queue manipulation functions.
   -------------------------------------------------------------- */
PRIVATE BOOL 
ixAtmQueueFullQuery (IxAtmCodeletSwLbSwMbufQ *s)
{
    return (s->head - s->tail == s->size);
}

PRIVATE BOOL 
ixAtmQueueEmptyQuery (IxAtmCodeletSwLbSwMbufQ *s)
{
    return (s->head == s->tail);
}

PRIVATE IX_OSAL_MBUF * 
ixAtmMBufQueueGet (IxAtmCodeletSwLbSwMbufQ *s)
{
    IX_OSAL_ASSERT (s->head != s->tail);
    return (s->array[s->tail++ & s->mask]);
}

PRIVATE void 
ixAtmSwLbMBufQueuePut (IxAtmCodeletSwLbSwMbufQ *s, IX_OSAL_MBUF *buf)
{
    IX_OSAL_ASSERT (s->head - s->tail != s->size);
    s->array[s->head++ & s->mask] = buf;
}

PRIVATE void 
ixAtmChainQueuePut (IxAtmCodeletSwLbSwMbufQ *s,
		    IX_OSAL_MBUF *mbufPtr)
{
    IX_OSAL_MBUF *buf;

    do
    {
	if (ixAtmQueueFullQuery(s))
	{
	    ixAtmUtilsMbufFree(mbufPtr);
	    break;
	}
	else
	{
	    buf = mbufPtr;
	    mbufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);
	    IX_OSAL_ASSERT(IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(buf) == 0);
	    ixAtmSwLbMBufQueuePut (s, buf);
	}
    }
    while (mbufPtr);
}




         
