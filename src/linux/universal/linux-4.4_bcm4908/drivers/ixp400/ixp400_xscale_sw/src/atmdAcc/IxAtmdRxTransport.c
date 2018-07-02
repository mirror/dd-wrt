/**
* @file ixAtmdRxTransport.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief ATM Rx real time interface
*
*
* Design Notes:
*    This code is real-time critical
*
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
* Put the user defined include files required.
*/

#include "IxOsal.h"
#include "IxAtmdAccCtrl.h"
#include "IxAtmdDefines_p.h"
#include "IxAtmdAssert_p.h"
#include "IxAtmdUtil_p.h"

#define IX_ATMDACC_RX_TRANSPORT_EXTERN
#include "IxAtmdRxTransport_p.h"

/* function prototype*/
PRIVATE INLINE IX_STATUS
ixAtmdAccRxQueueEntriesGet(IxQMgrQId qMgrQueueId,
                           unsigned int *numberOfEntriesPtr,
                           unsigned int defaultNumberOfEntries);

PRIVATE void
ixAtmdAccRxProcess (unsigned int physicalAddress);

PRIVATE IX_STATUS
ixAtmdAccRxVcFreeReplenishPossible (IxAtmConnId connId,
                                    IX_OSAL_MBUF *mbufPtr,
                                    unsigned int * spaceInQmgrQueuePtr);

PRIVATE void
ixAtmdAccMbufToDescriptorAttach(IxAtmdAccNpeDescriptor  *npeDescriptor, 
                                IX_OSAL_MBUF *mbufPtr);

PRIVATE IX_STATUS
ixAtmdAccRxVcFreeChainReplenish (IxAtmConnId connId,
                                 IX_OSAL_MBUF *mbufPtr);

#ifndef NDEBUG

PRIVATE void
ixAtmdAccRxAckCheck ( unsigned int npeVcId,
                      IxQMgrQId qId );

PRIVATE void
ixAtmdAccRxPduPtrCheck ( unsigned int physicalAddress, 
                         IxQMgrQId qId );

#endif


/*
* Function definition.
*/

/* ---------------------------------------------------------
* Get the number of entries from a queue and handle the case
* where queues are moving during the operation
*/
PRIVATE INLINE IX_STATUS
ixAtmdAccRxQueueEntriesGet(IxQMgrQId qMgrQueueId,
                           unsigned int *numberOfEntriesPtr,
                           unsigned int defaultNumberOfEntries)
{
    /* read the number from the queue manager */
    IX_STATUS returnStatus = ixQMgrQNumEntriesGet (qMgrQueueId,
        numberOfEntriesPtr);
    
    /* check read success */
    if (returnStatus == IX_QMGR_WARNING)
    {
        /* read again the number from the queue manager */
        returnStatus = ixQMgrQNumEntriesGet (qMgrQueueId,
            numberOfEntriesPtr);
        
        /* cannot get the number of entries : this is because the queue is full */
        if (returnStatus == IX_QMGR_WARNING)
        {
            /* get the number of entries */
            returnStatus = IX_SUCCESS;
            *numberOfEntriesPtr = defaultNumberOfEntries;
        }
        else if (returnStatus != IX_SUCCESS)
        {
            /* map the return status */
            returnStatus = IX_FAIL;
        }
    } /* end of if(returnStatus) */
    else if (returnStatus != IX_SUCCESS)
    {
        /* map the return status */
        returnStatus = IX_FAIL;
    }
    return returnStatus;
}

/* ---------------------------------------------------------
* process a shutdown Ack message from NPE
*/
void
ixAtmdAccRxShutdownAck (unsigned int npeVcId)
{
    if (npeVcId < IX_ATM_MAX_NUM_AAL_OAM_RX_VCS)
    {
        /* get the VC descriptor */
        IxAtmdAccRxVcDescriptor *vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];

        if (vcDescriptor->status == IX_ATMDACC_RX_CHANNEL_DOWN_PENDING)
        {
            vcDescriptor->status = IX_ATMDACC_RX_CHANNEL_DOWN;
        }
    } /* end of if(npeVcId) */
    return;
}

#ifndef NDEBUG
/* ---------------------------------------------------------
* Check a shutdown Ack message from NPE is arriving from 
* the right queue
*/
PRIVATE void
ixAtmdAccRxAckCheck (unsigned int npeVcId,
                     IxQMgrQId qId)
{
    IxAtmdAccRxVcDescriptor *vcDescriptor;
    
    IX_ATMDACC_ABORT(npeVcId < IX_ATM_MAX_NUM_AAL_OAM_RX_VCS, 
        "npeVcId out of range when receiving a RxShutdownAck");

    /* get the VC descriptor */
    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];
    
    IX_ATMDACC_ABORT(qId == vcDescriptor->rxQueueId,
        "Rx Queue Mismatch when receiving a RxShutdownAck");

    return;
}

/* ------------------------------------------------------------
* check a descriptor is from the right queue, check the descriptor order
* and check the mbuf chaining
*/
PRIVATE void
ixAtmdAccRxPduPtrCheck (unsigned int physicalAddress, 
                        IxQMgrQId qId)
{
    unsigned int connId;
    unsigned int rxId;
    IX_OSAL_MBUF *mbufPtr;
    IX_OSAL_MBUF *tempMbufPtr;
    IX_OSAL_MBUF *tempMbufPtr2;
    unsigned int tempHead;
    IxAtmdAccRxVcDescriptor *vcDescriptor;
    IX_ATMDACC_RX_QUEUE *rxSwQueue;
    IxAtmdAccNpeDescriptor *npeDescriptor2;
    
    /* incoming traffic descriptor */
    IxAtmdAccNpeDescriptor *npeDescriptor = (IxAtmdAccNpeDescriptor *) physicalAddress;
    
    IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (npeDescriptor);
    
    /* invalidate the xscale MMU */
    IX_ATMDACC_DATA_CACHE_INVALIDATE(npeDescriptor, sizeof(npeDescriptor->npe.rx));
    
    /* check if the signature of the descriptor is still there 
    * If it is not, it means that this is not a valid pointer
    */
    IX_ATMDACC_ABORT(npeDescriptor->atmd.signature == IX_ATMDACC_DESCRIPTOR_SIGNATURE, "Imvalid pointer returned by NPE");
    
    /* get the connection ID in the desriptor fields */
    connId = npeDescriptor->atmd.connId;
    
    /* get the channel */
    rxId = IX_ATMDACC_RX_VC_INDEX_GET (connId);
    
    /* get the descriptor */
    vcDescriptor = &ixAtmdAccRxVcDescriptor[rxId];
    rxSwQueue = &vcDescriptor->queue;
    
    IX_ATMDACC_ABORT(qId == vcDescriptor->rxQueueId, "Rx Queue Mismatch");
    
    /* check if the incoming descriptor is the extected descriptor */
    if (IX_ATMDACC_RXQ_HEAD_ENTRY_GET (rxSwQueue)->atmd.physicalAddress
        != physicalAddress)
    {
        /* check if we missed a mbuf */
        unsigned int count = 0;
        
        IX_ATMDACC_FULL_STATS(
        if (vcDescriptor->status != IX_ATMDACC_RX_CHANNEL_UP)
        {
            ixAtmdAccRxTransportErrorStats.descriptorOrderDisconnectErrorCount++;
        }
        else
        {
            ixAtmdAccRxTransportErrorStats.descriptorOrderErrorCount++;
        });


        
        /* check the descriptor recycling */
        IX_ATMDACC_ABORT (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), "Rx descriptors queue failure");
        
        do
        {
            /* get the mbuf from the descriptor ring buffer */
            mbufPtr = IX_ATMDACC_RXQ_HEAD_ENTRY_GET (rxSwQueue)->atmd.pRootMbuf;
            
            /* force data to be read from physical memory */
            IX_ATMDACC_DATA_CACHE_INVALIDATE(mbufPtr, sizeof(IX_OSAL_MBUF));
            
            /* unchain the mbuf */
            IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) = NULL;
            
            /* return the mbuf the the user */
            (*vcDescriptor->rxDoneCallback) (vcDescriptor->port,
                vcDescriptor->callbackId,
                IX_ATMDACC_MBUF_RETURN,
                IX_ATMDACC_CLP_NOT_SET,
                mbufPtr);
            
            /* go forward in the queue */
            IX_ATMDACC_RXQ_HEAD_INCR(rxSwQueue);
            
            /* check the descriptor recycling */
            IX_ATMDACC_ENSURE (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), "Rx descriptors queue failure");
            
            count++;
        }
        while (count < IX_ATMDACC_RXQ_SIZE(rxSwQueue) && 
            IX_ATMDACC_RXQ_HEAD_ENTRY_GET (rxSwQueue)->atmd.physicalAddress
            != physicalAddress);
        
        /* check the descriptor recycling */
        IX_ATMDACC_ABORT (IX_ATMDACC_RXQ_HEAD_ENTRY_GET (rxSwQueue)->atmd.physicalAddress
            == physicalAddress, "Recycling order is wrong");
    }
    
    /* check all mbufs of a chain are bind to the descriptors of this channel */
    tempHead = IX_ATMDACC_SWQ_HEAD(rxSwQueue) ;
    tempMbufPtr = npeDescriptor->atmd.pRootMbuf;

    /* check the descriptor recycling */
    IX_ATMDACC_ENSURE (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), "Rx descriptors queue failure");
    
    while (tempMbufPtr)
    {
        /* force data to be read from physical memory */
        IX_ATMDACC_DATA_CACHE_INVALIDATE(tempMbufPtr, sizeof(IX_OSAL_MBUF));

        /* get the mbuf from the descriptor ring buffer */
        npeDescriptor2 = IX_ATMDACC_RXQ_ENTRY_IDXGET(rxSwQueue, tempHead);
        tempMbufPtr2 = npeDescriptor2->atmd.pRootMbuf;
        
        /* check the mbuf order is as-expected */
        IX_ATMDACC_FULL_STATS(
        if (tempMbufPtr2 != tempMbufPtr)
        {
            ixAtmdAccRxTransportErrorStats.mbufMismatchErrorCount++;
        });
        tempMbufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(tempMbufPtr);
        if (tempMbufPtr)
        {
            IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (IX_OSAL_MBUF *, tempMbufPtr);
            IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (tempMbufPtr);
        }
        tempHead = (tempHead + 1) & rxSwQueue->mask;
    }
}
#endif

/* ------------------------------------------------------------
* process a received PDU
* free the asscoiated NPE descriptors, and pass the
* mbuf/chain up to the user via the supplied callback
*/
PRIVATE void
ixAtmdAccRxProcess (unsigned int physicalAddress)
{
    IX_OSAL_MBUF *mbufPtr;
    IxAtmdAccPduStatus pduStatus;
    IxAtmdAccClpStatus clpStatus;
    UINT32 rxBitField;
    UINT32 atmCellHeader;
    unsigned int connId;
    unsigned int rxId;
    unsigned int mbufCount;
    IxAtmdAccRxVcDescriptor *vcDescriptor;
    IX_ATMDACC_RX_QUEUE *rxSwQueue;

    /* incoming traffic descriptor */
    IxAtmdAccNpeDescriptor *npeDescriptor = (IxAtmdAccNpeDescriptor *) physicalAddress;


    IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (npeDescriptor);

    /* invalidate the xscale MMU */
    IX_ATMDACC_DATA_CACHE_INVALIDATE(npeDescriptor, sizeof(npeDescriptor->npe.rx));

    IX_ATMD_DEBUG_DO(
    /* check if the signature of the descriptor is still there 
    * If it is not, it means that this is not a valid pointer
    */
    IX_ATMDACC_ABORT(npeDescriptor->atmd.signature == IX_ATMDACC_DESCRIPTOR_SIGNATURE, 
        "Imvalid pointer returned by NPE");
    );

    /* get the connection ID in the desriptor fields */
    connId = npeDescriptor->atmd.connId;

    /* get the channel */
    rxId = IX_ATMDACC_RX_VC_INDEX_GET (connId);

    /* get the descriptor */
    vcDescriptor = &ixAtmdAccRxVcDescriptor[rxId];
    rxSwQueue = &vcDescriptor->queue;

    /* check the descriptor recycling */
    IX_ATMDACC_ENSURE (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), 
        "Rx descriptors queue failure");
        
    /* check the descriptor recycling is exact : no descriptor is missing */
    IX_ATMDACC_ABORT (IX_ATMDACC_RXQ_HEAD_ENTRY_GET (rxSwQueue)->atmd.physicalAddress
		      == physicalAddress, "Descriptor Recycling order is wrong");
    
    /* get the PDU length information */
    IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (UINT32, npeDescriptor->npe.rx.totalLen);

    /* get the mbuf pointer and process the endianness 
    * and update the pkt header field, and the length of the last mbuf
    */
    mbufPtr = ixAtmdAccUtilRxMbufFromNpeFormatConvert (
        npeDescriptor->atmd.pRootMbuf,
        npeDescriptor->npe.rx.totalLen,
        &mbufCount);

    IX_ATMDACC_ENSURE((vcDescriptor->npeAalType == NPE_AAL5_TYPE) || 
                     ((vcDescriptor->npeAalType != NPE_AAL5_TYPE) && 
		      (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) == NULL)),
                     "Rx PDU has illegal chain on AAL0/OAM");

    IX_ATMDACC_ENSURE(((vcDescriptor->npeAalType == NPE_OAM_TYPE) && 
                       (npeDescriptor->npe.rx.totalLen == vcDescriptor->cellSize)) ||
                      ((vcDescriptor->npeAalType != NPE_OAM_TYPE) && 
                       ((npeDescriptor->npe.rx.totalLen % vcDescriptor->cellSize) == 0)),
                    "Rx PDU has incomplete cells (AALx) or multiple cells (OAM)");



     rxBitField = npeDescriptor->npe.rx.rxBitField;
     IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN(UINT32, rxBitField);

     atmCellHeader = npeDescriptor->npe.rx.atmCellHeader;
     IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN(UINT32, atmCellHeader);

    /* check the channel + connId validity */
    if (vcDescriptor->connId == connId)
    {
        /* the connId is valid */
        /* get the PDU informations : status */


        if (IX_NPE_A_RXBITFIELD_STATUS_GET(rxBitField)
            == NPE_RX_PDU_VALID)
        {
            pduStatus = vcDescriptor->pduValidStatusCode;
        }
        else
        {
            pduStatus = vcDescriptor->pduInvalidStatusCode;
        } /* end of if-else(npeDescriptor) */

        clpStatus = IX_NPE_A_ATMCELLHEADER_CLP_GET(atmCellHeader);

        /* recycle the descriptors */
        IX_ATMDACC_RXQ_HEAD_ADVANCE (rxSwQueue, mbufCount);

        /* check the descriptor queue */
        IX_ATMDACC_ENSURE (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), "Rx descriptors queue failure");

        /* call the receive event for this channel id */
        vcDescriptor->rxDoneCallback (
            IX_NPE_A_RXBITFIELD_PORT_GET(rxBitField),
            vcDescriptor->callbackId, 
            pduStatus, 
            clpStatus, 
            mbufPtr);
    }
    else
    {
        /* this descriptor is related to something old and already disconnected */
        IX_ATMDACC_RXQ_HEAD_ADVANCE (rxSwQueue, mbufCount);
        
        /* check the descriptor queue */
        IX_ATMDACC_ENSURE (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), "Rx descriptors queue failure");
        
        /* release the buffer */
        (*vcDescriptor->rxDoneCallback) (vcDescriptor->port,
            vcDescriptor->callbackId,
            IX_ATMDACC_MBUF_RETURN,
            IX_ATMDACC_CLP_NOT_SET,
            mbufPtr);
    } /* end of if-else(vcDescriptor) */
}


/* ---------------------------------------------------------
* process the entries from the RX queue
*/

PUBLIC IX_STATUS
ixAtmdAccRxDispatch (IxAtmRxQueueId atmdQueueId,
                     unsigned int numberOfPdusToProcess,
                     unsigned int *numberOfPdusProcessedPtr)
{
    IX_STATUS returnStatus = IX_SUCCESS;
    IX_STATUS qmgrStatus = IX_SUCCESS;
    IxQMgrQId qId;
    UINT32 *qEntryPtr;
    UINT32 qEntry;
    UINT32 numberOfEntriesToRead;

    IX_ATMDACC_FULL_STATS( unsigned totalRx = 0; );

    /* check the parameters */
    IX_ATMDACC_PARAMS_CHECK(
    if ((numberOfPdusProcessedPtr == NULL) ||
        (numberOfPdusToProcess == 0)       ||
        (ixAtmdAccUtilQmgrQIdGet (atmdQueueId, &qId) != IX_SUCCESS) ||
        (qId != ixAtmdAccRxQMgrQId[atmdQueueId]))
    {
        /* param check failed */
        IX_ATMDACC_FULL_STATS(
        if(atmdQueueId < IX_ATM_MAX_RX_STREAMS)
        {
            ixAtmdAccRxDispatchStats[atmdQueueId].failedCount++; 
        });
        return IX_FAIL;

    });

    /* get the qId from a stream table 
     * this overwrites the qId that was returned if param checks are enabled
     * but done this way for performance when they are not enabled
     */
    qId = ixAtmdAccRxQMgrQId[atmdQueueId];

    /* update stats */
    IX_ATMDACC_FULL_STATS( ixAtmdAccRxDispatchStats[atmdQueueId].invokeCount++; );
    
    /* initialise the result parameter */
    *numberOfPdusProcessedPtr = 0;
    
    /* iterate until all entries requested are processed */
    do
    {
        /* set the pointer to the beginning of the buffer */
        qEntryPtr = ixAtmdAccRxQMgrBuffer[atmdQueueId];
        
        /* get the number of entries to read */
        if (numberOfPdusToProcess >= IX_ATMDACC_RX_QUEUE_SIZE)
        {
            numberOfEntriesToRead = IX_ATMDACC_RX_QUEUE_SIZE;
        }
        else
        {
            numberOfEntriesToRead = numberOfPdusToProcess;
            qEntryPtr[numberOfEntriesToRead] = 0;
        }
        
        /* read all the queue entries (note that the buffer size is adjusted 
        * and initialised to ensure that there is always a null entry
        * at the end of the buffer. This code assume that a null entry 
        * is stored by qMgr at the end of the input data.
        */
        qmgrStatus = ixQMgrQBurstRead (qId, 
            numberOfEntriesToRead, 
            qEntryPtr);
        
        /* get the first entry from the buffer */
        qEntry = *qEntryPtr;

        /* iterate through all entries until a null entry is found */
        while (qEntry != 0)
        {
            /* check the queue entry type */
            switch(qEntry & NPE_RX_TYPE_MASK)
            {
            case NPE_RX_DESCRIPTOR:
                IX_ATMD_DEBUG_DO(
                /* check the descriptor is from the right queue
                * and check the descriptor order is valid, and mbuf
                * chaining is correct, and try to recover if it
                * is possible to recover, and update statistics.
                */
                ixAtmdAccRxPduPtrCheck(qEntry & NPE_DESCRIPTOR_MASK, qId)
                );
                
                /* process data descriptor */
                ixAtmdAccRxProcess (qEntry & NPE_DESCRIPTOR_MASK);
                
                /* update stats */
                IX_ATMDACC_FULL_STATS( 
                    totalRx++;
                    ixAtmdAccRxDispatchStats[atmdQueueId].spPduCount++; );
                
                /* update the number of pdu processed */
                numberOfPdusToProcess--;
                (*numberOfPdusProcessedPtr)++;
                break;
            case NPE_RX_SHUTDOWN_ACK:
                IX_ATMD_DEBUG_DO(
                    /* check the descriptor ack is from the right queue */
                    ixAtmdAccRxAckCheck((qEntry & NPE_RX_SHUTDOWN_ACK_VCID_MASK) >> NPE_SHUTDOWN_ACK_SHIFT, qId);
                );
                /* process rx shutdown ack */
                ixAtmdAccRxShutdownAck ((unsigned int) ((qEntry & NPE_RX_SHUTDOWN_ACK_VCID_MASK) >> NPE_SHUTDOWN_ACK_SHIFT));
                
                /* update stats */
                IX_ATMDACC_FULL_STATS( ixAtmdAccRxDispatchStats[atmdQueueId].ackCount++; );
                break;
            default:
                /* update stats */
                IX_ATMDACC_FULL_STATS( ixAtmdAccRxDispatchStats[atmdQueueId].invalidDescCount++; );
                returnStatus = IX_FAIL;
                break;
            }
            
            /* get the next entry from the buffer */
            qEntry = *(++qEntryPtr);
        } /* end of while(qEntry) */
    }
    while ((qmgrStatus == IX_SUCCESS) && (numberOfPdusToProcess > 0));

    IX_ATMDACC_FULL_STATS(
    if (totalRx > ixAtmdAccRxDispatchStats[atmdQueueId].maxRxPerDispatch)
    {
        ixAtmdAccRxDispatchStats[atmdQueueId].maxRxPerDispatch = totalRx;

        if (returnStatus != IX_SUCCESS)
        {
            ixAtmdAccRxDispatchStats[atmdQueueId].failedCount++; 
        }
    });
    return returnStatus;
}

/* ---------------------------------------------------------
* queue manager callback to be used to receive data from NPE
*/
void
ixAtmdAccRxCallback (IxQMgrQId qMgrQueueId,
                     IxQMgrCallbackId cbId)
{
    unsigned int dummynumberOfPdusProcessed;
    unsigned int numberOfPdusToProcess = IX_ATMDACC_ALLPDUS;
#ifndef __wince
    IX_STATUS returnStatus;
#endif

    /* check queueId and callbackId values */
    IX_ATMDACC_ENSURE (IX_ATMDACC_RX_VCQ_ID_VALID (qMgrQueueId), "Qmgr invalid qId");

    IX_ATMDACC_ENSURE (IX_ATMDACC_RX_CALLBACK_ID_VALID (cbId), "Qmgr invalid callback Id");

    /* WINCE_PORT NOTE: function pointer comparison across DLLs does not work. */
#ifndef __wince
    /* check the registered callback : if it is the atmdAcc callback, there
    * is no need to read the qmgr level, because the action will be
    * to process all pdus.
    */
    if (ixAtmdAccRxUserCallback[cbId] != ixAtmdAccRxDispatch)
    {
        returnStatus = ixAtmdAccRxQueueEntriesGet(qMgrQueueId,
            &numberOfPdusToProcess,
            IX_QMGR_Q_SIZE64);

        /* check the qmgr status : should not fail */
        IX_ATMDACC_ABORT (returnStatus == IX_SUCCESS, "Qngr status failed");

        if (numberOfPdusToProcess == 0)
        {
            /* nothing to process in this queue */
            return;
        }
    } /* end of if(ixAtmdAccRxUserCallback) */
#endif
    /* invoke the user callback */
    (*ixAtmdAccRxUserCallback[cbId]) ((IxAtmRxQueueId) cbId,
        numberOfPdusToProcess,
        &dummynumberOfPdusProcessed);
}

/* ---------------------------------------------------------
*/
PRIVATE IX_STATUS
ixAtmdAccRxVcFreeReplenishPossible (IxAtmConnId connId,
                                    IX_OSAL_MBUF *mbufPtr,
                                    unsigned int * spaceInQmgrQueuePtr)
{
    IX_STATUS returnStatus = IX_SUCCESS;
    unsigned int numEntriesInQmgrQueue;

    /* get the channel */
    unsigned int rxId = IX_ATMDACC_RX_VC_INDEX_GET (connId);
    IxAtmdAccRxVcDescriptor *vcDescriptor = &ixAtmdAccRxVcDescriptor[rxId];
    IX_ATMDACC_RX_QUEUE *rxSwQueue = &vcDescriptor->queue;
    
    /* check parameters */
    if ((vcDescriptor->connId != connId) ||
        (mbufPtr == NULL)                ||
        (IX_OSAL_MBUF_MLEN(mbufPtr) < vcDescriptor->cellSize)
        )
    {
        returnStatus = IX_FAIL;
    }
    else
    {
        /* check if a descriptor is available */
        if (IX_ATMDACC_RXQ_OVERLOADED (rxSwQueue))
        {
            returnStatus = IX_ATMDACC_BUSY;
        }
        else
        {
            returnStatus = ixAtmdAccRxQueueEntriesGet(vcDescriptor->rxFreeQueueId,
                &numEntriesInQmgrQueue,
                0);
            
            IX_ATMDACC_ENSURE (returnStatus == IX_SUCCESS, "Qngr status failed");
            
            /* map the qmgr error code */
            if (returnStatus == IX_SUCCESS)
            {
                *spaceInQmgrQueuePtr = vcDescriptor->rxFreeQueueSize - numEntriesInQmgrQueue;
                
                /* check the queue is full */
                if (*spaceInQmgrQueuePtr == 0)
                {
                    returnStatus = IX_ATMDACC_BUSY;
                }
            }
        }
    } /* end of if(vcDescriptor) */
    
    return returnStatus;
}

/* ---------------------------------------------------------
* attach a mbuf to a npe descriptor
*/
PRIVATE void
ixAtmdAccMbufToDescriptorAttach(IxAtmdAccNpeDescriptor  *npeDescriptor, 
                                IX_OSAL_MBUF *mbufPtr)
{
    /* - fill the NPE descriptor fields */
    /* - convert the mbuf pointer */
    UINT32 rxBitField = npeDescriptor->npe.rx.rxBitField;
    UINT32 atmCellHeader = npeDescriptor->npe.rx.atmCellHeader;
    IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN(UINT32, rxBitField);
    IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN(UINT32, atmCellHeader);


    IX_NPE_A_ATMCELLHEADER_PTI_SET(atmCellHeader, 0);
    IX_NPE_A_ATMCELLHEADER_CLP_SET(atmCellHeader, 0);
    IX_NPE_A_RXBITFIELD_STATUS_SET(rxBitField, NPE_RX_PDU_VALID);
    npeDescriptor->atmd.pRootMbuf = mbufPtr;
    npeDescriptor->npe.rx.totalLen = 0;
    npeDescriptor->npe.rx.pCurrMbuf = mbufPtr;
    IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS (npeDescriptor->npe.rx.pCurrMbuf);
    IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (IX_OSAL_MBUF *,npeDescriptor->npe.rx.pCurrMbuf);
    /* - convert the mbuf header */
    /* NPE requires len + 1 */
    IX_NPE_A_RXBITFIELD_CURRMBUFSIZE_SET(rxBitField, IX_OSAL_MBUF_MLEN(mbufPtr) + 1);
    IX_ATMDACC_CONVERT_TO_BIG_ENDIAN(UINT32, rxBitField);
    npeDescriptor->npe.rx.rxBitField = rxBitField;
    IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS (IX_OSAL_MBUF_MDATA(mbufPtr));
    IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (UINT8 *,IX_OSAL_MBUF_MDATA(mbufPtr));
    IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (int,IX_OSAL_MBUF_MLEN(mbufPtr));
    npeDescriptor->npe.rx.pCurrMbufData = (unsigned char *)IX_OSAL_MBUF_MDATA(mbufPtr);
    npeDescriptor->npe.rx.currMbufLen = 0;
    IX_ATMDACC_CONVERT_TO_BIG_ENDIAN(UINT32, atmCellHeader);
    npeDescriptor->npe.rx.atmCellHeader = atmCellHeader;        
    npeDescriptor->npe.rx.pNextMbuf = 0;
    npeDescriptor->npe.rx.aal5CrcResidue = 0xffffffff;
    /* flush the xscale MMU */
    IX_ATMDACC_DATA_CACHE_FLUSH(mbufPtr, sizeof(IX_OSAL_MBUF));
}

/* ---------------------------------------------------------
* send a chain of free buffers to NPE
*/
PRIVATE IX_STATUS
ixAtmdAccRxVcFreeChainReplenish (IxAtmConnId connId,
                                 IX_OSAL_MBUF *mbufPtr)
{
    IX_STATUS returnStatus;
    unsigned int spaceInQmgrQueue;
    IxAtmdAccNpeDescriptor *npeDescriptor;
    IX_OSAL_MBUF *mbufPtrNext;
    unsigned int data;

    /* check parameters and if the Rx free queue has available entruies */
    returnStatus = ixAtmdAccRxVcFreeReplenishPossible (connId,
        mbufPtr,
        &spaceInQmgrQueue);

    if (returnStatus == IX_SUCCESS)
    {
        /* get the channel */
        unsigned int rxId = IX_ATMDACC_RX_VC_INDEX_GET (connId);
        IxAtmdAccRxVcDescriptor *vcDescriptor = &ixAtmdAccRxVcDescriptor[rxId];
        IX_ATMDACC_RX_QUEUE *rxSwQueue = &vcDescriptor->queue;

        /* indicate that we are replenishInProgress
        i.e. disconnect is unsafe
        */
        vcDescriptor->replenishInProgress = TRUE;

        do
        {
            /* check if the mbuf length is acceptable */
            if (IX_OSAL_MBUF_MLEN(mbufPtr) < vcDescriptor->cellSize)
            {
                /* this should not happen to the first mbuf
                *  because it is already checked. The remainder of the mbuf is
                *  returned to the user
                */
                break;
            }
            /* check the descriptor queue */
            IX_ATMDACC_ENSURE (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), "Rx descriptors sw queue failure");

            npeDescriptor = IX_ATMDACC_RXQ_TAIL_ENTRY_GET (rxSwQueue);
            IX_ATMDACC_RXQ_TAIL_INCR (rxSwQueue);

            IX_ATMDACC_ENSURE (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), "Rx descriptors sw queue failure");

            /* save the next mbuf */
            mbufPtrNext = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);
            IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) = NULL;

            /* attach an unchained mbuf to a descriptor */
            ixAtmdAccMbufToDescriptorAttach(npeDescriptor, mbufPtr);

            /* flush the xscale MMU */
            IX_ATMDACC_DATA_CACHE_FLUSH(npeDescriptor, sizeof(npeDescriptor->npe.rx));

            /* store the NPE descriptor in the RX free queue */
            data = npeDescriptor->atmd.physicalAddress;
            returnStatus = ixQMgrQWrite (vcDescriptor->rxFreeQueueId, &data);
            IX_ATMDACC_ENSURE (returnStatus == IX_SUCCESS, "Qngr status failed");

            /* get the next element from the chained list */
            mbufPtr = mbufPtrNext;
            /* iterate until one of the conditions
            - no buffer chained
            - rxfree queue is full
            - no more descriptors are available
            */
        }
        while (mbufPtr != 0
            && returnStatus == IX_SUCCESS
            && --spaceInQmgrQueue > 0
            && !IX_ATMDACC_RXQ_OVERLOADED (rxSwQueue));

        /* part of the chanied mbufs are already stored in the RX free queue */
        /* the end of the chained mbufs is returned to the client */
        if (mbufPtr)
        {
            /* pass the end of the chain of mbuf back to the client */
            (*vcDescriptor->rxDoneCallback) (vcDescriptor->port,
                vcDescriptor->callbackId,
                IX_ATMDACC_MBUF_RETURN,
                IX_ATMDACC_CLP_NOT_SET,
                mbufPtr);
        } /* end of if(mbufPtr) */

        vcDescriptor->replenishInProgress = FALSE;
    } /* end of if(returnStatus) */

    return returnStatus;
}

/* ---------------------------------------------------------
* send free buffers to NPE
*/
IX_STATUS
ixAtmdAccRxVcFreeReplenish (IxAtmConnId connId,
                            IX_OSAL_MBUF *mbufPtr)
{
    IX_STATUS returnStatus;
    unsigned int data;
    IxAtmdAccNpeDescriptor *npeDescriptor;
    unsigned int rxId = IX_ATMDACC_RX_VC_INDEX_GET (connId);
    IxAtmdAccRxVcDescriptor *vcDescriptor = &ixAtmdAccRxVcDescriptor[rxId];
    IX_ATMDACC_RX_QUEUE *rxSwQueue = &vcDescriptor->queue;
    IxQMgrQStatus qStatus;

    if ( (vcDescriptor->connId != connId)  ||
         (mbufPtr == NULL) 
       )
    {
        return IX_FAIL;
    }

    IX_ATMDACC_PARAMS_CHECK(
    if ( (IX_OSAL_MBUF_MLEN(mbufPtr) <  vcDescriptor->cellSize) )
    {
        return IX_FAIL;
    });

    /* check for queue full */
    returnStatus = ixQMgrQStatusGet(vcDescriptor->rxFreeQueueId, &qStatus);
    if (qStatus & IX_QMGR_Q_STATUS_F_BIT_MASK)
    {
        /* the rxfree queue is full */
        return IX_ATMDACC_BUSY;
    }

    /* happy day scenario : check for unchained mbufs */
    if ((IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) == NULL)  &&
        (!IX_ATMDACC_RXQ_OVERLOADED (rxSwQueue)) 
        )
    {
        vcDescriptor->replenishInProgress = TRUE;

        /* check the descriptor queue */
        IX_ATMDACC_ENSURE (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), "Rx descriptors sw queue failure");
        
        npeDescriptor = IX_ATMDACC_RXQ_TAIL_ENTRY_GET (rxSwQueue);
        IX_ATMDACC_RXQ_TAIL_INCR (rxSwQueue);
        
        IX_ATMDACC_ENSURE (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), "Rx descriptors sw queue failure");
        
        /* attach an unchained mbuf to a descriptor */
        ixAtmdAccMbufToDescriptorAttach(npeDescriptor, mbufPtr);
        
        /* store the NPE descriptor in the RX free queue */
        data = npeDescriptor->atmd.physicalAddress;

        /* flush the xscale MMU */
        IX_ATMDACC_DATA_CACHE_FLUSH(npeDescriptor, sizeof(npeDescriptor->npe.rx));

        returnStatus = ixQMgrQWrite (vcDescriptor->rxFreeQueueId, &data);

        if (returnStatus != IX_SUCCESS)
        {
            /* roll back */
            IX_ATMDACC_RXQ_TAIL_DECR (rxSwQueue);

            /* check unreachable conditions */
            IX_ATMDACC_ENSURE (IX_ATMDACC_RXQ_CONSISTENT (rxSwQueue), "Rx descriptors sw queue failure");
            
            /* roll back the mbuf */
            IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (UINT8 *, IX_OSAL_MBUF_MDATA(mbufPtr));
            IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (int, IX_OSAL_MBUF_MLEN(mbufPtr));
            IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_MDATA(mbufPtr));

            /* map the error status */
            if (returnStatus == IX_QMGR_Q_OVERFLOW)
            {
                returnStatus = IX_ATMDACC_BUSY;
            }
            else
            {
                returnStatus = IX_FAIL;
            }
        }
        vcDescriptor->replenishInProgress = FALSE;
    }
    else
    {
        /* something is not "happy day" scenario :
        * this can be chained mbufs, sw queue full 
        */
        returnStatus = ixAtmdAccRxVcFreeChainReplenish (connId, mbufPtr);
    } /* end of if(returnStatus) */

    /* return the status to the client */
    return returnStatus;
}

/* ---------------------------------------------------------
* QMgr callback for rx free events
*/
void
ixAtmdAccRxFreeCallback (IxQMgrQId qId,
                         IxQMgrCallbackId cbId)
{
    IxAtmdAccRxVcDescriptor *vcDescriptor;

    /* check callback values and unreachable conditions */
    IX_ATMDACC_ENSURE (IX_ATMDACC_RX_FREEQ_ID_VALID (qId), 
        "Qmgr invalid qId");
    IX_ATMDACC_ENSURE (IX_ATMDACC_RX_FREECALLBACK_ID_VALID (cbId), 
        "Qmgr invalid callback Id");

    /* the channel id is the callback userid */
    vcDescriptor = &ixAtmdAccRxVcDescriptor[cbId];

    /* check callback pointer */
    IX_ATMDACC_ENSURE (vcDescriptor->rxFreeNotification != NULL, 
        "Invalid callback pointer");

    /* invoke the user callback */
    (*vcDescriptor->rxFreeNotification) (vcDescriptor->callbackId);

}

/* ---------------------------------------------------------
*  get the number of free entries in the RX Free queue
*/
PUBLIC IX_STATUS
ixAtmdAccRxVcFreeEntriesQuery (IxAtmConnId connId,
                               unsigned int *numberOfMbufsPtr)
{
    IX_STATUS returnStatus;
    unsigned int rxId;
    unsigned int numberOfEntries;
    IxAtmdAccRxVcDescriptor *vcDescriptor;

    /* get the channel for this connId */
    rxId = IX_ATMDACC_RX_VC_INDEX_GET (connId);
    vcDescriptor = &ixAtmdAccRxVcDescriptor[rxId];

    /* check the connextion Id */
    if ((vcDescriptor->connId != connId) ||
        (numberOfMbufsPtr == NULL))
    {
        returnStatus = IX_FAIL;
    }
    else
    {
        /* check unreachable conditions */
        IX_ATMDACC_ENSURE (IX_ATMDACC_RX_VC_INUSE (vcDescriptor->connId), "Invalid conn ID");

        /* read the number from the queue manager */
        returnStatus = ixAtmdAccRxQueueEntriesGet(vcDescriptor->rxFreeQueueId,
            &numberOfEntries,
            0);

        /* check the qmgr status */
        IX_ATMDACC_ENSURE (returnStatus == IX_SUCCESS, "Qngr status failed");

        /* map the qmgr error code */
        if (returnStatus == IX_SUCCESS)
        {
            *numberOfMbufsPtr = vcDescriptor->rxFreeQueueSize - numberOfEntries;
        } /* end of if-else(returnStatus) */
    } /* end of if-else(vcDescriptor) */

    return returnStatus;
}

/* --------------------------------------------------------- 
*  get the number of used entries in the RX queue 
*/
PUBLIC IX_STATUS
ixAtmdAccRxLevelQuery (IxAtmRxQueueId atmdQueueId,
                       unsigned int *numberOfPdusPtr)
{
    IX_STATUS returnStatus;
    IxQMgrQId qMgrQueueId;

    /* check parameters */
    if (numberOfPdusPtr == NULL)
    {
        returnStatus = IX_FAIL;
    }
    else
    {
        /* convert qIds amd check input parameter */
        returnStatus = ixAtmdAccUtilQmgrQIdGet (atmdQueueId, &qMgrQueueId);
    } /* end of if-else(numberOfPdusPtr) */

    if (returnStatus == IX_SUCCESS)
    {
        /* read the number from the queue manager */
        returnStatus = ixAtmdAccRxQueueEntriesGet (qMgrQueueId,
            numberOfPdusPtr, 
            IX_QMGR_Q_SIZE64);
    } /* end of if(returnStatus) */
    return returnStatus;
}

