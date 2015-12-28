/**
* @file ixAtmdRxCfgInfo.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief ATM RX configuration
*
* This module setup the different configuration elements required
* to process the RX traffic
*
* Design Notes:
* This interface is designed to run under the functions of
* ixAtmdAccCfgIf.c
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
* Sytem defined include files
*/

/*
* Put the user defined include files required.
*/

#include "IxOsal.h"
#include "IxAtmdDefines_p.h"
#include "IxAtmdDescMgmt_p.h"
#include "IxAtmdAssert_p.h"
#include "IxAtmdUtil_p.h"
#include "IxAtmdRxTransport_p.h"
#include "IxAtmdRxCfgInfo_p.h"

/**
*
* @struct IxAtmdAccRxFreeQueueTable
*
* @brief Container for Rx free queues which are initialized
*
*/
typedef struct
{
    IxQMgrQId rxFreeQueueId;
    BOOL used;
    unsigned int rxFreeQueueSize;
} IxAtmdAccRxFreeQueueTable;

static IxAtmdAccRxFreeQueueTable ixAtmdAccRxFreeQueueTable[IX_ATM_MAX_NUM_AAL_VCS];
static unsigned int ixAtmdAccRxFreeQueueCount;

#ifndef NDEBUG 
static unsigned int ixAtmdAccRxFreeDummyNotificationErrorCount;
static unsigned int ixAtmdAccRxDummyProcessPerformErrorCount;
static unsigned int ixAtmdAccRxEnableCount;
static unsigned int ixAtmdAccRxDisableCount;
#endif


/* The npeResponse lock will serialise the excution of the functions
* ixAtmdAccRxCfgNpeAtmStatusRead with the IxNpeMh solicited callbacks.
*/
static IxOsalMutex npeResponseLock;
static volatile UINT32 npeRespWordRead = 0;

#define IX_ATMDACC_NPE_RESPONSE_LOCK_INIT() do{\
    IX_STATUS returnStatus = IX_SUCCESS;\
    returnStatus = ixOsalMutexInit (&npeResponseLock);\
    IX_ATMDACC_ENSURE (returnStatus == IX_SUCCESS, "Initailisation Error");\
    IX_ATMDACC_NPE_RESPONSE_LOCK_GET ();\
    } while(0)

#define IX_ATMDACC_NPE_RESPONSE_LOCK_GET() do{\
    ixOsalMutexLock (&npeResponseLock, IX_OSAL_WAIT_FOREVER);\
    } while(0)

#define IX_ATMDACC_NPE_RESPONSE_LOCK_RELEASE() do{\
    ixOsalMutexUnlock (&npeResponseLock);\
    } while(0)


#define IX_ATMDACC_NPE_RESPONSE_LOCK_DESTROY() do{\
    ixOsalMutexDestroy (&npeResponseLock);        \
    } while(0)



/* function prototype */
PRIVATE void
ixAtmdAccRxFreeDummyNotification (IxAtmdAccUserId callbackId);

PRIVATE IX_STATUS
ixAtmdAccRxDummyProcessPerform (IxAtmRxQueueId rxQueueId,
                                                  unsigned int numberOfPdusToProcess,
                                                  unsigned int *numberOfPdusProcessedPtr);

PRIVATE void
ixAtmdAccRxCfgChannelIdReset(IxAtmdAccRxVcDescriptor *vcDescriptorPtr);

PRIVATE void
ixAtmdAccRxCfgNpeAtmStatusReadCallback (IxNpeMhNpeId npeMhNpeId,
                                        IxNpeMhMessage npeMhMessage);

PRIVATE IX_STATUS
ixAtmdAccRxCfgNpeAtmStatusRead (unsigned int * rxFreeUnderflowCount, 
                                unsigned int * rxOverflowCount);

/* ---------------------------------------------------------
* dummy notification
*/
PRIVATE void
ixAtmdAccRxFreeDummyNotification (IxAtmdAccUserId callbackId)
{
/* this function should never get called because the client
*   supplies its own callback. But if a disableInterrupt fails
*   in the qMgr (or any transient state), this function may be used.
*   For code efficiency, it is better to have a dummycallback (bad day
*   scenario) and never test a null pointer in "happy day scenarios"
*/
    IX_ATMDACC_FULL_STATS(ixAtmdAccRxFreeDummyNotificationErrorCount++; );
}

PRIVATE IX_STATUS ixAtmdAccRxDummyProcessPerform (IxAtmRxQueueId rxQueueId,
                                                  unsigned int numberOfPdusToProcess,
                                                  unsigned int *numberOfPdusProcessedPtr)
{
/* this function should never get called because the client
*   supplies its own callback. But if a disableInterrupt fails
*   in the qMgr (or any transient state), this function may be used.
*   For code efficiency, it is better to have a dummycallback (bad day
*   scenario) and never test a null pointer in "happy day scenarios"
*/
    IX_ATMDACC_FULL_STATS( ixAtmdAccRxDummyProcessPerformErrorCount++; );
    return IX_FAIL;
}

PRIVATE void
ixAtmdAccRxCfgChannelIdReset(IxAtmdAccRxVcDescriptor *vcDescriptorPtr)
{
    vcDescriptorPtr->connId    = IX_ATMDACC_RX_INVALID_CONNID;
    vcDescriptorPtr->vci       = NPE_INVALID_VCI;
    vcDescriptorPtr->vpi       = NPE_INVALID_VPI;
    vcDescriptorPtr->port      = NPE_INVALID_LOGICAL_PORT;
    return;
}

/* ---------------------------------------------------------
*   module initialisation
*/
IX_STATUS
ixAtmdAccRxCfgInfoInit (void)
{
    IX_STATUS returnStatusVal = IX_SUCCESS;
    IxQMgrQId rxFreeQueueId;
    IX_STATUS status;
    unsigned int rxFreeQueueSize;
    unsigned int vc;
    unsigned int queueIdx;
    unsigned int totalRxFreeQueueSize;
    unsigned int averageRxFreeQueueSize;

    IX_ATMDACC_NPE_RESPONSE_LOCK_INIT();

    /* stats reset */
    ixAtmdAccRxCfgInfoStatsReset ();

    /* initialise the queue buffers used during Rx read */
    ixOsalMemSet(ixAtmdAccRxQMgrBuffer, 0, sizeof(ixAtmdAccRxQMgrBuffer));

    /* initialise the data structures */
    for (vc = 0; vc < IX_ATM_MAX_NUM_AAL_OAM_RX_VCS; vc++)
    {
        ixOsalMemSet (&ixAtmdAccRxVcDescriptor[vc], 0, sizeof (IxAtmdAccRxVcDescriptor));

        ixAtmdAccRxCfgChannelIdReset(&ixAtmdAccRxVcDescriptor[vc]);

        ixAtmdAccRxVcDescriptor[vc].status = IX_ATMDACC_RX_CHANNEL_DOWN;
        ixAtmdAccRxVcDescriptor[vc].rxQueueId = IX_QMGR_QUEUE_INVALID;
        ixAtmdAccRxVcDescriptor[vc].rxFreeQueueId = IX_QMGR_QUEUE_INVALID;
        ixAtmdAccRxVcDescriptor[vc].rxFreeQueueSize = IX_QMGR_Q_SIZE_INVALID;
        ixAtmdAccRxVcDescriptor[vc].rxFreeNotification = ixAtmdAccRxFreeDummyNotification;
    } /* end of for(vc) */

    /* reset the Free Queue table to a default state */
    for (queueIdx = 0; queueIdx < IX_ATM_MAX_NUM_AAL_VCS; queueIdx++)
    {
        ixAtmdAccRxFreeQueueTable[queueIdx].used = FALSE;
        ixAtmdAccRxFreeQueueTable[queueIdx].rxFreeQueueId = IX_QMGR_QUEUE_INVALID;
        ixAtmdAccRxFreeQueueTable[queueIdx].rxFreeQueueSize = IX_QMGR_Q_SIZE_INVALID;
    }

    /* initialize the queue mapping */
    ixAtmdAccRxQMgrQId[IX_ATM_RX_A] = IX_NPE_A_QMQ_ATM_RX_HI;
    ixAtmdAccRxQMgrQId[IX_ATM_RX_B] = IX_NPE_A_QMQ_ATM_RX_LO;

    /* setup default callback */
    ixAtmdAccRxUserCallback[IX_ATM_RX_A] = ixAtmdAccRxDummyProcessPerform;
    ixAtmdAccRxUserCallback[IX_ATM_RX_B] = ixAtmdAccRxDummyProcessPerform;

    /* set the RxFreeQueue table to empty */
    ixAtmdAccRxFreeQueueCount = 0;

    /* initialise the total size of rx queues */
    totalRxFreeQueueSize = 0;

    /* iterate through the queues Id reserved for RX free queues
    *   to determine the details of the configured VC free queues
    */
    for (rxFreeQueueId = IX_NPE_A_QMQ_ATM_RXFREE_MIN;
        rxFreeQueueId <= IX_NPE_A_QMQ_ATM_RXFREE_MAX;
        rxFreeQueueId++)
    {
        /* this function will fail if there are gaps in the QMgr
        * initialisation, which hould not break this loop. If some
        * queues are not initialized, the number of channels that
        * Atmd will be able to handle will be reduced
        */
        status = ixQMgrQSizeInEntriesGet (rxFreeQueueId, &rxFreeQueueSize);
        if (status == IX_SUCCESS)
        {
            /* set this queue information in the RX Free queue table */
            ixAtmdAccRxFreeQueueTable[ixAtmdAccRxFreeQueueCount].used = FALSE;
            ixAtmdAccRxFreeQueueTable[ixAtmdAccRxFreeQueueCount].rxFreeQueueId = rxFreeQueueId;
            ixAtmdAccRxFreeQueueTable[ixAtmdAccRxFreeQueueCount].rxFreeQueueSize = rxFreeQueueSize;

            /* keep track of how many queues are configured */
            ixAtmdAccRxFreeQueueCount++;

            totalRxFreeQueueSize += rxFreeQueueSize;
        } /* end of if(status) */
    } /* end of for(rxFreeQueueId) */

    /* check the overall number of queues */
    if (ixAtmdAccRxFreeQueueCount == 0)
    {
        /* no rx queue configured ??? */
        returnStatusVal = IX_FAIL;
    }
    else
    {
        /* ensure the size of rx free queues is as expected.
        * This is used to prevent a misconfiguration of the descriptor pool
        */
        averageRxFreeQueueSize = totalRxFreeQueueSize / ixAtmdAccRxFreeQueueCount;
        IX_ATMDACC_ABORT(averageRxFreeQueueSize <= IX_ATMDACC_AVERAGE_RXFREE_QUEUE_SIZE,
            "IX_ATMDACC_AVERAGE_RXFREE_QUEUE_SIZE not consistent with queue configuration");
        /* no way to continue, the system configuration is wrong */
    } /* end of if-else(ixAtmdAccRxFreeQueueCount) */

    return returnStatusVal;
}


/* ---------------------------------------------------------
*   module uninitialisation
*/

IX_STATUS
ixAtmdAccRxCfgInfoUninit (void)
{
    IX_ATMDACC_NPE_RESPONSE_LOCK_DESTROY();
    return IX_SUCCESS;
}




/* ---------------------------------------------------------
* data initialisation
*/
void
ixAtmdAccRxCfgInfoStatsReset (void)
{
    /* initialise stats */
    IX_ATMDACC_FULL_STATS({
    ixAtmdAccRxFreeDummyNotificationErrorCount  = 0;
    ixAtmdAccRxDummyProcessPerformErrorCount    = 0;
    ixAtmdAccRxEnableCount                      = 0;
    ixAtmdAccRxDisableCount                     = 0;
    ixOsalMemSet(ixAtmdAccRxDispatchStats, 0, sizeof(ixAtmdAccRxDispatchStats));
    ixOsalMemSet(&ixAtmdAccRxTransportErrorStats,0,sizeof(ixAtmdAccRxTransportErrorStats));
    });
}

/* ---------------------------------------------------------
*     check that the specified VC is configured
*/
BOOL
ixAtmdAccRxCfgRxVcExists (IxAtmLogicalPort port,
                          unsigned int vpi,
                          unsigned int vci)
{
    BOOL result = FALSE;
    int vc;

    /* iterate through VC until found */
    for (vc = 0; !result && vc < IX_ATM_MAX_NUM_AAL_OAM_RX_VCS; vc++)
    {
        if (IX_ATMDACC_RX_VC_INUSE (ixAtmdAccRxVcDescriptor[vc].connId))
        {
            if (ixAtmdAccRxVcDescriptor[vc].vci == vci
                && ixAtmdAccRxVcDescriptor[vc].vpi == vpi
                && ixAtmdAccRxVcDescriptor[vc].port == port)
            {
                result = TRUE;
            }
        } /* end of if(IX_ATMDACC_RX_VC_INUSE) */
    } /* end of for(vc) */

    return result;
}


/* ---------------------------------------------------------
* allocate a RX AAL channel for this post/vp/vc
*/
IX_STATUS
ixAtmdAccRxCfgChannelGet (IxAtmLogicalPort port,
                          unsigned int vpi,
                          unsigned int vci,
                          IxAtmdAccAalType aalServiceType,
                          unsigned int* npeVcIdPtr,
                          IxAtmConnId* connIdPtr)
{
    unsigned int npeVcid = 0;
    unsigned int limit;
    IX_STATUS returnStatus = IX_SUCCESS;

    switch( aalServiceType )
    {
    case IX_ATMDACC_AAL5:
    case IX_ATMDACC_AAL0_48:
    case IX_ATMDACC_AAL0_52:
  
        /* compute a hash value */
        npeVcid = ixAtmdAccUtilHashVc (port, vpi, vci) % IX_ATM_MAX_NUM_AAL_VCS;

        /* find a free channel, starting from the hash value
        * iterate until the first available channel is found
        *  and stop iterate when all channels are checked
        */
        for (limit = 0;
            (limit < IX_ATM_MAX_NUM_AAL_VCS) &&
            IX_ATMDACC_RX_VC_INUSE (ixAtmdAccRxVcDescriptor[npeVcid].connId);
            limit++)
        {
            npeVcid = npeVcid + 1;
            npeVcid = (npeVcid % IX_ATM_MAX_NUM_AAL_VCS);
        }

        /* check upperbound condition */
        if (limit >= IX_ATM_MAX_NUM_AAL_VCS)
        {
            /* no channel available */
            returnStatus = IX_ATMDACC_BUSY;
        }
        break;
    case IX_ATMDACC_OAM:
        /* allocate a fixed channel outside the hashing range for OAM */
        npeVcid = IX_ATM_MAX_NUM_AAL_OAM_RX_VCS-1;
        break;
    default:
        /* Params were checked on API this should be impossible */
        IX_ATMDACC_ENSURE(0,"Bad serviceType");
        break;
    }

    if (returnStatus == IX_SUCCESS)
    {
        /* the channel is available */
        *npeVcIdPtr = npeVcid;
        /* the connection Id is constructed from the NpeVcId,
        * and an unique ID. This allow to quickly retrieve
        * the channel from a connection ID and also to avoid
        * connection ID garbage from the user. A flag is
        * also merged in the connection ID to quickly detect
        * race conditions (e.g. disconnect in progress). This is
        * done to improve performances of happy day scenarios
        */
        *connIdPtr = npeVcid + (ixAtmdAccUtilUniqueIdGet () * IX_ATM_MAX_NUM_AAL_OAM_RX_VCS);
    } /* end of if-else(limit) */
    return returnStatus;
}



/* ---------------------------------------------------------
*  search the smallest available RxFree queue
*/
IX_STATUS
ixAtmdAccRxCfgFreeQueueGet (IxAtmdAccAalType aalServiceType,
                            unsigned int minimumQueueSize,
                            unsigned int npeVcId)
{
    unsigned int optimumQueueSize = IX_QMGR_Q_SIZE_INVALID;
    unsigned int optimumQueueIdx = IX_QMGR_QUEUE_INVALID;
    unsigned int queueIdx;
    IX_STATUS returnStatus = IX_SUCCESS;

    switch( aalServiceType )
    {
    case IX_ATMDACC_AAL5:
    case IX_ATMDACC_AAL0_48:
    case IX_ATMDACC_AAL0_52:
        /* find the smallest rx free queue */
        for (queueIdx = 0; queueIdx < ixAtmdAccRxFreeQueueCount; queueIdx++)
        {
            /* check if the queue is unused */
            if (ixAtmdAccRxFreeQueueTable[queueIdx].used == FALSE)
            {
                unsigned int qSize = ixAtmdAccRxFreeQueueTable[queueIdx].rxFreeQueueSize;
                /* search the smallest queue
                * - if no size is required, get the smallest queue
                * - if a size is required, get the queue which size is the
                *   closest to the requested queue size
                */
                if (((minimumQueueSize == IX_ATMDACC_DEFAULT_REPLENISH_COUNT) &&
                    (qSize < optimumQueueSize)) ||
                    ((minimumQueueSize != IX_ATMDACC_DEFAULT_REPLENISH_COUNT) &&
                    (qSize >= minimumQueueSize) &&
                    (qSize < optimumQueueSize)))
                {
                /* this queue is the smallest queue larger than
                *   required
                    */
                    optimumQueueSize = qSize;
                    optimumQueueIdx  = queueIdx;
                } /* end of if(minimumQueueSize) */
            } /* end of if(ixAtmdAccRxFreeQueueTable) */
        } /* end of for(queueIdx) */

        if (optimumQueueSize == IX_QMGR_Q_SIZE_INVALID)
        {
            /* queue with appropriate size not found */
            returnStatus = IX_FAIL;
        }
        else
        {
            /* update the internal structure */
            ixAtmdAccRxVcDescriptor[npeVcId].rxFreeQueueId = ixAtmdAccRxFreeQueueTable[optimumQueueIdx].rxFreeQueueId;
            ixAtmdAccRxVcDescriptor[npeVcId].rxFreeQueueSize = optimumQueueSize;
            ixAtmdAccRxFreeQueueTable[optimumQueueIdx].used = TRUE;
        } /* end of if-else(optimumQueueSize) */
        break;
    case IX_ATMDACC_OAM:
        /* Note that the input parameter minimum queue size is ignore doe OAM
         * because only one fixed OAM free queue exists
        */
        ixAtmdAccRxVcDescriptor[npeVcId].rxFreeQueueId = IX_NPE_A_QMQ_OAM_FREE_VC;
        ixAtmdAccRxVcDescriptor[npeVcId].rxFreeQueueSize = IX_ATMDACC_QMGR_OAM_FREE_QUEUE_SIZE;
        break;
    default:
        /* param is checked on API, should never happen */
        IX_OSAL_ENSURE(0,"Bad Service type");
        returnStatus = IX_FAIL;
        break;
    }
    return returnStatus;
}

                                    
/* ---------------------------------------------------------
* set all channels parameters
*/
IX_STATUS
ixAtmdAccRxCfgChannelSet (IxAtmConnId connId,
                          unsigned int npeVcId,
                          IxAtmLogicalPort port,
                          unsigned int vpi,
                          unsigned int vci,
                          IxAtmdAccAalType aalServiceType,
                          IxAtmRxQueueId rxQueueId,
                          IxAtmdAccUserId userId,
                          IxAtmdAccRxVcRxCallback rxDoneCallback)
{
    UINT8 timeLimit = 0;
    unsigned int queueSize = 0;
    unsigned int descCount;
    IX_STATUS returnStatus = IX_SUCCESS;
    IxAtmdAccRxVcDescriptor *vcDescriptor;
    IX_ATMDACC_RX_QUEUE *rxSwQueue;
    IxAtmdAccNpeDescriptor *npeDescriptor;

    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];
    rxSwQueue = &vcDescriptor->queue;

    /* fill the descriptors parameters */
    vcDescriptor->connId = connId;
    vcDescriptor->port = port;
    vcDescriptor->vpi = vpi;
    vcDescriptor->vci = vci;
    vcDescriptor->callbackId = userId;
    vcDescriptor->rxDoneCallback = rxDoneCallback;
    vcDescriptor->rxStreamId = rxQueueId;
    vcDescriptor->rxFreeNotification = ixAtmdAccRxFreeDummyNotification;

    /* service specific initializations */
    switch( aalServiceType )
    {
    case IX_ATMDACC_AAL5:
        vcDescriptor->npeAalType = NPE_AAL5_TYPE;
        vcDescriptor->cellSize = IX_ATM_CELL_PAYLOAD_SIZE;
        vcDescriptor->pduValidStatusCode = IX_ATMDACC_AAL5_VALID;
        vcDescriptor->pduInvalidStatusCode = IX_ATMDACC_AAL5_CRC_ERROR;
        /* the sw queue size is based on the rxFree qmgr queue size
        * (there is 1 descriptor per mbuf) plus the descriptors needed by
        * the NPE to build a 64Kb PDU, plus the descriptors needed to hold
        * a certain amount of PDUs in the RX queue.
        */
        queueSize = vcDescriptor->rxFreeQueueSize + IX_ATMDACC_RX_NUMBER_OF_DESCRIPTORS;

        /* check the sw queue does not reach NPE limits (the NPE is unable
        * to chain more than 256 mbufs. The way to prevent it is to ensure
        * that there is less than 256 mbufs provided to the npe for 1 channel.
        * This is done by limitation of the sw queue size.
        */
        if(queueSize > IX_NPE_A_CHAIN_DESC_COUNT_MAX)
        {
            queueSize = IX_NPE_A_CHAIN_DESC_COUNT_MAX;
        }
        break;
    case IX_ATMDACC_AAL0_48:
        vcDescriptor->npeAalType = NPE_AAL0_48_TYPE;
        vcDescriptor->cellSize = IX_ATM_AAL0_48_CELL_PAYLOAD_SIZE;
        vcDescriptor->pduValidStatusCode = IX_ATMDACC_AAL0_VALID;
        vcDescriptor->pduInvalidStatusCode = IX_ATMDACC_AAL0_VALID;
        timeLimit = IX_ATMDACC_AAL0_RX_TIMEOUT + 1;
        /* the sw queue size is based on the rxFree qmgr queue size
        * ( there is 1 descriptor per mbuf), plus the descriptors needed to hold
        * a certain amount of PDUs in the RX queue. There is no rx chaining,
        */
        queueSize = vcDescriptor->rxFreeQueueSize + IX_ATMDACC_MAX_RX_PDU_PENDING;
        break;
    case IX_ATMDACC_AAL0_52:
        vcDescriptor->npeAalType = NPE_AAL0_52_TYPE;
        vcDescriptor->cellSize = IX_ATM_AAL0_52_CELL_SIZE_NO_HEC;
        vcDescriptor->pduValidStatusCode = IX_ATMDACC_AAL0_VALID;
        vcDescriptor->pduInvalidStatusCode = IX_ATMDACC_AAL0_VALID;
        timeLimit = IX_ATMDACC_AAL0_RX_TIMEOUT + 1;
        /* the sw queue size is based on the rxFree qmgr queue size
        * ( there is 1 descriptor per mbuf), plus the descriptors needed to hold
        * a certain amount of PDUs in the RX queue. There is no rx chaining,
        */
        queueSize = vcDescriptor->rxFreeQueueSize + IX_ATMDACC_MAX_RX_PDU_PENDING;
        break;
    case IX_ATMDACC_OAM:
        vcDescriptor->npeAalType = NPE_OAM_TYPE;
        vcDescriptor->cellSize = IX_ATM_AAL0_52_CELL_SIZE_NO_HEC;
        vcDescriptor->pduValidStatusCode = IX_ATMDACC_OAM_VALID;
        vcDescriptor->pduInvalidStatusCode = IX_ATMDACC_OAM_VALID;
        /* the sw queue size is based on the rxFree qmgr queue size
        * ( there is 1 descriptor per mbuf), plus the descriptors needed to hold
        * a certain amount of PDUs in the RX queue. There is no rx chaining,
        */
        queueSize = vcDescriptor->rxFreeQueueSize + IX_ATMDACC_MAX_RX_PDU_PENDING;
        break;
    default:
        /* Params are checked on API, so this should never happen */
        IX_ATMDACC_ENSURE(0,"Bad service type");
        break;
    }


    /* Initialise the RX Queue */
    IX_ATMDACC_RXQ_INIT(rxSwQueue,queueSize, vcDescriptor->swQueueBuffer);

    /* check if the initialisation is complete */
    if(!IX_ATMDACC_RXQ_INITIALISED(rxSwQueue))
    {
        returnStatus = IX_FAIL;
    }

    if(returnStatus == IX_SUCCESS)
    {
        /* convert from the atmd rx queueId to the qmgr rx queue id */
        returnStatus = ixAtmdAccUtilQmgrQIdGet (vcDescriptor->rxStreamId, &vcDescriptor->rxQueueId);
    }

    if (returnStatus == IX_SUCCESS)
    {
        for (descCount = 0; 
            (returnStatus == IX_SUCCESS) && (descCount < IX_ATMDACC_RXQ_SIZE(rxSwQueue)); 
            descCount++)
        {
        /* get a NPE descriptor and indicate how many descriptors
        * will be required for this queue, to anticipate allocation
            */
            returnStatus = ixAtmdAccDescNpeDescriptorGet (&npeDescriptor);

            IX_ATMDACC_ENSURE (npeDescriptor != 0, "Unable to allocate a descriptor");

            if (returnStatus == IX_SUCCESS)
            {
                /* allocation is sucessful */
                IX_ATMDACC_RXQ_ENTRY_IDXSET(rxSwQueue, descCount, npeDescriptor);

                /* update the descriptor fields */
                npeDescriptor->atmd.connId = connId;
                IX_NPE_A_RXBITFIELD_PORT_SET(npeDescriptor->npe.rx.rxBitField, port);
                IX_ATMDACC_CONVERT_TO_BIG_ENDIAN(UINT32, npeDescriptor->npe.rx.rxBitField);
                npeDescriptor->npe.rx.atmCellHeader = 0;
                IX_NPE_A_ATMCELLHEADER_GFC_SET(npeDescriptor->npe.rx.atmCellHeader, 0);
                IX_NPE_A_ATMCELLHEADER_VPI_SET(npeDescriptor->npe.rx.atmCellHeader, vpi);
                IX_NPE_A_ATMCELLHEADER_VCI_SET(npeDescriptor->npe.rx.atmCellHeader, vci);
                IX_ATMDACC_CONVERT_TO_BIG_ENDIAN(UINT32, npeDescriptor->npe.rx.atmCellHeader);
                npeDescriptor->npe.rx.timeLimit = timeLimit; 

                
            } /* end of if(returnStatus) */
        } /* end of for(descCount) */
    } /* end of if(returnStatus) */

    if (returnStatus != IX_SUCCESS)
    {
        /* rollback if error */
        ixAtmdAccRxCfgChannelReset (npeVcId);
    }

    return returnStatus;
}

/* ----------------------------------------
* disble the rxfree threshold events
*/
IX_STATUS ixAtmdAccRxCfgRxFreeCallbackDisable (unsigned int npeVcId)
{
    IX_STATUS returnStatus = IX_SUCCESS;
    IxAtmdAccRxVcDescriptor *descriptor;

    descriptor = &ixAtmdAccRxVcDescriptor[npeVcId];

    /* check if notifications are enabled */
    if (descriptor->rxFreeNotification != ixAtmdAccRxFreeDummyNotification)
    {
        /* disable interrupts */
        returnStatus = ixQMgrNotificationDisable (descriptor->rxFreeQueueId);
        if (returnStatus != IX_SUCCESS)
        {
            returnStatus = IX_FAIL;
        }
        else
        {
            /* move the queue priority to low */
            returnStatus = ixQMgrDispatcherPrioritySet (descriptor->rxFreeQueueId, 
                IX_QMGR_Q_PRIORITY_2);
            
            if (returnStatus != IX_SUCCESS)
            {
                returnStatus = IX_FAIL;
            }
        }
        
        /* mark the rx free notifications as 
        * disabled (regardless of the qmgr status)
        */
        descriptor->rxFreeNotification = ixAtmdAccRxFreeDummyNotification;
    }

    return returnStatus;
}

/* ----------------------------------------
* reset the channel parameters
*/
IX_STATUS ixAtmdAccRxCfgChannelReset (unsigned int npeVcId)
{
    unsigned int descCount;
    unsigned int queueIdx;
    IX_STATUS returnStatus = IX_SUCCESS;
    IxAtmdAccRxVcDescriptor *descriptor;
    IX_ATMDACC_RX_QUEUE *rxSwQueue;
    BOOL queueFound = FALSE;

    descriptor = &ixAtmdAccRxVcDescriptor[npeVcId];
    rxSwQueue = &descriptor->queue;

    /* Reset the channel and the channel's data to default
    *   (unalloacted) state
    */
    ixAtmdAccRxCfgChannelIdReset(descriptor);

    descriptor->rxQueueId = IX_QMGR_QUEUE_INVALID;

    /* set the rx free queue to idle */
    for (queueIdx = 0; (queueFound == FALSE) && (queueIdx < ixAtmdAccRxFreeQueueCount); queueIdx++)
    {
        if (ixAtmdAccRxFreeQueueTable[queueIdx].rxFreeQueueId == descriptor->rxFreeQueueId)
        {
            ixAtmdAccRxFreeQueueTable[queueIdx].used = FALSE;
            queueFound = TRUE;
        }
    } /* end of for(queueIdx) */

    descriptor->rxFreeQueueId = IX_QMGR_QUEUE_INVALID;
    descriptor->rxFreeQueueSize = IX_QMGR_Q_SIZE_INVALID;
    descriptor->rxFreeNotification = ixAtmdAccRxFreeDummyNotification;

    /* reinitialize the array of descriptor pointers */
    if (IX_ATMDACC_RXQ_INITIALISED(rxSwQueue))
    {
        for (descCount = 0; descCount < IX_ATMDACC_RXQ_SIZE(rxSwQueue); descCount++)
        {
            IxAtmdAccNpeDescriptor *npeDescriptor =
                IX_ATMDACC_RXQ_ENTRY_IDXGET(rxSwQueue,descCount);

            if (npeDescriptor != NULL)
            {
                /* release the NPE descriptor */
                ixAtmdAccDescNpeDescriptorRelease (npeDescriptor);
            }
        } /* end of for(descCount) */
        /* release the array of pointers */
        IX_ATMDACC_RXQ_RELEASE_RESOURCES(rxSwQueue);
    } /* end of if(IX_ATMDACC_RXQ_INITIALISED) */

    /* map the error status */
    if (returnStatus != IX_SUCCESS)
    {
        returnStatus = IX_FAIL;
    }
    return returnStatus;
}

/* ---------------------------------------------------------
* get the NpeVcId for this channel
*/
IX_STATUS
ixAtmdAccRxCfgNpeVcIdGet (IxAtmConnId connId,
                          unsigned int* npeVcIdPtr)
{
    IxAtmdAccRxVcDescriptor *vcDescriptor;
    IX_STATUS returnStatus = IX_SUCCESS;
    unsigned int npeVcId;

    npeVcId = IX_ATMDACC_RX_VC_INDEX_GET (connId);
    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];
    *npeVcIdPtr = npeVcId;

    /* check if the connection ID is valid */
    if (vcDescriptor->connId != connId)
    {
        /* check if the connection ID has been invalidated */
        if (vcDescriptor->connId != IX_ATMDACC_RX_DISCONNECTING (connId))
        {
            /* the connection ID does not correspond with the expected one */
            returnStatus = IX_FAIL;
        }
    } /* end of if(vcDescriptor) */
    return returnStatus;
}

/* ---------------------------------------------------------
*    invalidate the connection Id
*/
IX_STATUS
ixAtmdAccRxCfgConnIdInvalidate (unsigned int rxId)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    IxAtmdAccRxVcDescriptor *vcDescriptor = &ixAtmdAccRxVcDescriptor[rxId];

    /* check if the connId is already invalidated */
    if (!IX_ATMDACC_RX_DISCONNECTED (vcDescriptor->connId))
    {
        vcDescriptor->connId = IX_ATMDACC_RX_DISCONNECTING (vcDescriptor->connId);
        /* connId is now invalidated : a direct comparison of the connId
        *   passed as a parameter and the connId stored in the structure
        *   will fail.
        */
    } /* end of if(IX_ATMDACC_RX_DISCONNECTED) */
    return returnStatus;
}

/* ---------------------------------------------------------
* check if all descriptors are recycled
*/
IX_STATUS
ixAtmdAccRxCfgFreeResourcesCheck (unsigned int npeVcId)
{
    IX_STATUS returnStatus = IX_SUCCESS;
    IxAtmdAccRxVcDescriptor *vcDescriptor;
    IX_ATMDACC_RX_QUEUE *rxSwQueue;

    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];
    rxSwQueue = &vcDescriptor->queue;

    /* check if resources are all free : this is done
    * by checking that the sw queue is in the initial
    * state (the queue is full). When the queue is full,
    * all resources are back and this mean that the
    * channel disconnect is now complete
    */

    if (!IX_ATMDACC_RXQ_ALL_RECYCLED(rxSwQueue))
    {
        returnStatus = IX_ATMDACC_RESOURCES_STILL_ALLOCATED;
    }
    return returnStatus;
}

/* ---------------------------------------------------------
* tell the NPE to start receive on this channel and
* update the lookup table
*/
IX_STATUS
ixAtmdAccRxCfgNpeVcLookupTableUpdate(unsigned int npeVcId)
{
    unsigned int npeParam0;
    unsigned int npeParam1;
    IxAtmdAccRxVcDescriptor* vcDescriptor;
    IX_STATUS returnStatus;
    IxAtmRxQueueId atmdStreamId;

    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];
    ixAtmdAccUtilAtmdQIdGet(vcDescriptor->rxQueueId, &atmdStreamId);
    
    npeParam0 =
        (vcDescriptor->npeAalType << NPE_MSG_RX_ENABLE_TYPE_SHIFT) |
        ((1 - atmdStreamId) << NPE_MSG_RX_ENABLE_RXQ_SHIFT) |
        (vcDescriptor->rxFreeQueueId << NPE_MSG_RX_ENABLE_RXFREEQ_SHIFT) |
        ((npeVcId) << NPE_MSG_RX_ENABLE_VCID_SHIFT);

    npeParam1 =
        (vcDescriptor->port << NPE_MSG_RX_ENABLE_PORT_SHIFT) |
        (vcDescriptor->vpi << NPE_MSG_RX_ENABLE_VPI_SHIFT) |
        (vcDescriptor->vci << NPE_MSG_RX_ENABLE_VCI_SHIFT);
    returnStatus = ixAtmdAccUtilNpeMsgSend(IX_NPE_A_MSSG_ATM_RX_ENABLE,
        npeParam0,
        npeParam1);

    return returnStatus;
}

/* ---------------------------------------------------------
* check if Rx disconnect is in progress
*/
BOOL
ixAtmdAccRxCfgVcIsDisconnecting(unsigned int npeVcId)
{
    IxAtmdAccRxVcDescriptor *vcDescriptor;

    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];

    return IX_ATMDACC_RX_DISCONNECTED (vcDescriptor->connId);
}

/* ---------------------------------------------------------
* check if Rx Traffic is enabled
*/
BOOL
ixAtmdAccRxCfgVcIsEnabled(unsigned int npeVcId)
{
    IxAtmdAccRxVcDescriptor *vcDescriptor;

    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];

    return vcDescriptor->status == IX_ATMDACC_RX_CHANNEL_UP;
}

/* ---------------------------------------------------------
* Enable Rx Traffic
*/
void
ixAtmdAccRxCfgVcEnable (unsigned int npeVcId)
{
    IxAtmdAccRxVcDescriptor *vcDescriptor;

    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];

    vcDescriptor->status = IX_ATMDACC_RX_CHANNEL_UP;

    /* update stats */
    IX_ATMDACC_FULL_STATS( ixAtmdAccRxEnableCount++; );
    
    return;
}

/* ---------------------------------------------------------
* Enable Rx Traffic
*/
void
ixAtmdAccRxCfgVcEnableRollback (unsigned int npeVcId)
{
    IxAtmdAccRxVcDescriptor *vcDescriptor;

    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];

    vcDescriptor->status = IX_ATMDACC_RX_CHANNEL_DOWN;
    
    return;
}

/* ---------------------------------------------------------
* check if Rx Traffic is stopped
*/
BOOL
ixAtmdAccRxCfgVcIsDisabled(unsigned int npeVcId)
{
    IxAtmdAccRxVcDescriptor *vcDescriptor;

    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];

    return vcDescriptor->status == IX_ATMDACC_RX_CHANNEL_DOWN;
}

/* ---------------------------------------------------------
* disable Rx Traffic
*/
void
ixAtmdAccRxCfgVcDisable(unsigned int npeVcId)
{
    IxAtmdAccRxVcDescriptor *vcDescriptor;
    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];

    vcDescriptor->status = IX_ATMDACC_RX_CHANNEL_DOWN_PENDING;

    /* update stats */
    IX_ATMDACC_FULL_STATS( ixAtmdAccRxDisableCount++; );

    return;
}

/* ---------------------------------------------------------
* set a callback for rxfree events
*/
IX_STATUS
ixAtmdAccRxCfgRxFreeCallbackSet (unsigned int npeVcId,
                                 unsigned int thresholdLevel,
                                 IxAtmdAccRxVcFreeLowCallback callback)
{
    IxAtmdAccRxVcDescriptor *vcDescriptor;
    IxQMgrQId rxFreeQueueId;
    IX_STATUS returnStatus = IX_SUCCESS;
    unsigned int numberOfEntries;

    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];
    rxFreeQueueId = vcDescriptor->rxFreeQueueId;

    /* check the threshold level is above the current queue level */
    if (thresholdLevel > (vcDescriptor->rxFreeQueueSize / 2))
    {
        returnStatus = IX_FAIL;
    }
    else
    {
        /* read the number from the queue manager */
        returnStatus = ixQMgrQNumEntriesGet (rxFreeQueueId, &numberOfEntries);
        
        /* read again the number of entries  */
        if (returnStatus == IX_QMGR_WARNING)
        {
            returnStatus = ixQMgrQNumEntriesGet (rxFreeQueueId, &numberOfEntries);
            
            if (returnStatus == IX_QMGR_WARNING)
            {
                /* the queue is empty */
                numberOfEntries = 0; 
                returnStatus = IX_SUCCESS;
            }
        } /* end of if(returnStatus) */
        
        if ((returnStatus != IX_SUCCESS) || (numberOfEntries <= thresholdLevel))
        {
        /* It will fail if the threshold level is above the current queue level 
        * because notifications will not trigger  when the first
        * traffic will be received
            */
            returnStatus = IX_FAIL;
        }
        else
        {
            /* register the user callback */
            vcDescriptor->rxFreeNotification = callback;
            
            if(ixAtmdAccUtilQmgrCallbackSet (rxFreeQueueId,
                thresholdLevel,
                thresholdLevel,
                IX_QMGR_Q_SOURCE_ID_NE,
                ixAtmdAccRxFreeCallback,
                (IxQMgrCallbackId) npeVcId,
                IX_QMGR_Q_PRIORITY_1) != IX_SUCCESS)
            {
                returnStatus = IX_FAIL;
            }
        }
    }

    return returnStatus;
}

/* ---------------------------------------------------------
* set a callback for rx events
*/
IX_STATUS
ixAtmdAccRxCfgRxCallbackSet (IxAtmRxQueueId atmRxQueueId,
                             IxQMgrQId qMgrQId,
                             IxAtmdAccRxDispatcher callback)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* register the new user callback */
    ixAtmdAccRxUserCallback[atmRxQueueId] = callback;

    if(ixAtmdAccUtilQmgrCallbackSet (qMgrQId,
        0,
        0,
        IX_QMGR_Q_SOURCE_ID_NOT_E,
        ixAtmdAccRxCallback,
        (IxQMgrCallbackId) atmRxQueueId,
        IX_QMGR_Q_PRIORITY_0) != IX_SUCCESS)
    {
        returnStatus = IX_FAIL;
    }
    return returnStatus;
}


/* ---------------------------------------------------------
* Reset the callback for rx events
*/
IX_STATUS
ixAtmdAccRxCfgRxCallbackReset (IxAtmRxQueueId atmRxQueueId,
                             IxQMgrQId qMgrQId)
{
    /* register the new user callback */
    ixAtmdAccRxUserCallback[atmRxQueueId] = ixAtmdAccRxDummyProcessPerform;

    if (IX_SUCCESS != ixQMgrNotificationDisable (qMgrQId) )
    {
        return IX_FAIL;
    }
    return IX_SUCCESS;
}




/* ---------------------------------------------------------
* Check if any Vcs exist
*/
BOOL
ixAtmdAccRxCfgRxVcsExist (void)
{
    int vc;
    BOOL returnValue = FALSE;

    /* iterate through VC until found */
    for (vc = 0; returnValue == FALSE && vc < IX_ATM_MAX_NUM_AAL_OAM_RX_VCS; vc++)
    {
        if (IX_ATMDACC_RX_VC_INUSE (ixAtmdAccRxVcDescriptor[vc].connId))
        {
            returnValue = TRUE;
        }
    }
    return returnValue;
}

/* ---------------------------------------------------------
* release all resources in the RxFree queue for this channel
*/
IX_STATUS
ixAtmdAccRxCfgResourcesRelease (unsigned int npeVcId)
{
    IxAtmdAccNpeDescriptor *npeDescriptor;
    UINT32 rxBitField;
    IX_OSAL_MBUF *mbufPtr;
    IX_OSAL_MBUF *mbufPtrTmp;
    IX_STATUS returnStatus = IX_SUCCESS;
    unsigned int qEntry;
    IxAtmdAccRxVcDescriptor *vcDescriptor;
    IX_ATMDACC_RX_QUEUE *rxSwQueue;
   
    /* get the VC descriptor */
    vcDescriptor = &ixAtmdAccRxVcDescriptor[npeVcId];
    rxSwQueue = &vcDescriptor->queue;

    /* test unreachable conditions */
    IX_ATMDACC_ENSURE (npeVcId < IX_ATM_MAX_NUM_AAL_OAM_RX_VCS, "Invalid NpeVcId");
    IX_ATMDACC_ENSURE (IX_ATMDACC_RX_VC_INUSE (vcDescriptor->connId), "Invalid ConnId");

    if ((vcDescriptor->status == IX_ATMDACC_RX_CHANNEL_DOWN) &&
        (vcDescriptor->replenishInProgress == FALSE))
    {
        /* flush the rx free queue */
        do
        {
            /* read the RX free queue entry */
            returnStatus = ixQMgrQRead (vcDescriptor->rxFreeQueueId, &qEntry);
        }
        while (returnStatus == IX_SUCCESS && qEntry != 0);

        /* flush any remaining entry from the rx queue
        */
        while (!IX_ATMDACC_RXQ_ALL_RECYCLED (rxSwQueue))
        {
            /* get the current descriptor */
            npeDescriptor = IX_ATMDACC_RXQ_HEAD_ENTRY_GET (rxSwQueue);

            /* unchain the mbuf (chaining could have started on NPE) */
            mbufPtr = npeDescriptor->atmd.pRootMbuf;
	    mbufPtrTmp = mbufPtr;

	    do {
		IX_ATMDACC_DATA_CACHE_INVALIDATE(mbufPtrTmp, sizeof(mbufPtrTmp));
		IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN(UINT32,IX_OSAL_MBUF_MLEN(mbufPtrTmp));
		
		/* convert the mbuf to virtual address space */
		IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (UINT8 *,IX_OSAL_MBUF_MDATA(mbufPtrTmp)); 
		IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_MDATA(mbufPtrTmp)); 

		/* Check for unchained mbufs */
		if (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtrTmp) != NULL)
		{
		    /* if unchained mbuf exist, then get the next mbuf */
		    IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (IX_OSAL_MBUF *,
							IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtrTmp));
		    IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtrTmp));
		    mbufPtrTmp = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtrTmp);
		}
		else
		{
		    mbufPtrTmp = NULL;
		}

	    } while (mbufPtrTmp != NULL);

            /* move the queue pointer */
            IX_ATMDACC_RXQ_HEAD_INCR (rxSwQueue);

            /* release the mbuf through the user-supplied callback */
            rxBitField = npeDescriptor->npe.rx.rxBitField;
            IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN(UINT32, rxBitField);

            (*vcDescriptor->rxDoneCallback) (
                IX_NPE_A_RXBITFIELD_PORT_GET(rxBitField),
                vcDescriptor->callbackId,
                IX_ATMDACC_MBUF_RETURN,
                IX_ATMDACC_CLP_NOT_SET,
                mbufPtr);
        }            
        
        /* all descriptors should be back */
        /* check if resources are still allocated */
        returnStatus = ixAtmdAccRxCfgFreeResourcesCheck (npeVcId);
        
    } /* end of if(vcDescriptor) */
    else
    {
        /* rxDisconnectAck is not yet received or
        * replenish is in progress, which means that resources
        * are still allocated.
        */
        returnStatus = IX_ATMDACC_RESOURCES_STILL_ALLOCATED;
    }

    return returnStatus;
}

/* ----------------------------------------------
* read status callback
*/
PRIVATE void
ixAtmdAccRxCfgNpeAtmStatusReadCallback (IxNpeMhNpeId npeMhNpeId,
                                        IxNpeMhMessage npeMhMessage)
{
    UINT32 id;

    /* Check NpeId */
    IX_ATMDACC_ENSURE (npeMhNpeId == IX_NPEMH_NPEID_NPEA, "wrong npeMhNpeId");

    /* Check Id */
    id = npeMhMessage.data[0] & NPE_RESP_ID_MASK;
    IX_ATMDACC_ENSURE (id == NPE_ATM_STATUS_READ_EXPECTED_ID, "wrong id");

    npeRespWordRead = npeMhMessage.data[1];

    /* Unblock the reading context */
    IX_ATMDACC_NPE_RESPONSE_LOCK_RELEASE ();
}

/* --------------------------------------------------
* Query the npe ATM status
*/
PRIVATE IX_STATUS
ixAtmdAccRxCfgNpeAtmStatusRead (unsigned int * rxFreeUnderflowCount, 
                                unsigned int * rxOverflowCount)
{
    IX_STATUS returnStatus;
    
    /* get the rx free underflow count */
    returnStatus = ixAtmdAccUtilNpeMsgWithResponseSend(
        IX_NPE_A_MSSG_ATM_STATUS_READ,
        NPE_MSG_RXFREEUNDERFLOW_ID, 
        NPE_IGNORE,
        ixAtmdAccRxCfgNpeAtmStatusReadCallback);
    
    if (returnStatus == IX_SUCCESS)
    {
        /* Wait for ixAtmdAccNpeReadCallback to release */
        IX_ATMDACC_NPE_RESPONSE_LOCK_GET ();
        
        /* copy the response */
        *rxFreeUnderflowCount = npeRespWordRead;
        
        /* get the rx underflow count */
        returnStatus = ixAtmdAccUtilNpeMsgWithResponseSend(
            IX_NPE_A_MSSG_ATM_STATUS_READ,
            NPE_MSG_RXOVERFLOW_ID, 
            NPE_IGNORE,
            ixAtmdAccRxCfgNpeAtmStatusReadCallback);
        
        if (returnStatus == IX_SUCCESS)
        {
            /* Wait for ixAtmdAccNpeReadCallback to release */
            IX_ATMDACC_NPE_RESPONSE_LOCK_GET ();
            
            /* copy the response */
            *rxOverflowCount = npeRespWordRead;
        }
    }
    
    /* map the error */
    if (returnStatus != IX_SUCCESS)
    {
        returnStatus = IX_FAIL;
    }
    
    return returnStatus;
}

/* ---------------------------------------------------------
* display internals
*/
void
ixAtmdAccRxCfgInfoStatsShow (void)
{
    int vc;
    int rxStream;
    IX_STATUS returnStatus;
    unsigned int numEntries;
    unsigned int rxOverflowCount, rxFreeUnderflowCount;
    IxQMgrQId qId;

    printf("\n");

    IX_ATMDACC_FULL_STATS(
    printf ("Rx Free Dummy notification ................ : %10u (should be 0)\n",
        ixAtmdAccRxFreeDummyNotificationErrorCount);
    printf ("Rx Process Dummy notification ............. : %10u (should be 0)\n",
        ixAtmdAccRxDummyProcessPerformErrorCount);
    printf ("Rx Enable ................................. : %10u\n",
        ixAtmdAccRxEnableCount);
    printf ("Rx Disable ................................ : %10u\n",
        ixAtmdAccRxDisableCount);
    printf ("Rx descriptor missing during disconnect ... : %10u (should be 0)\n",
        ixAtmdAccRxTransportErrorStats.descriptorOrderDisconnectErrorCount);
    printf ("Rx descriptor missing ..................... : %10u (should be 0)\n",
        ixAtmdAccRxTransportErrorStats.descriptorOrderErrorCount);
    printf ("Rx descriptor chaining inconsistent ....... : %10u (should be 0)\n",
        ixAtmdAccRxTransportErrorStats.mbufChainErrorCount);
    printf ("Rx descriptor mbuf mismatch ............... : %10u (should be 0)\n",
        ixAtmdAccRxTransportErrorStats.mbufMismatchErrorCount);
    printf("\n");
    );

    ixAtmdAccRxCfgNpeAtmStatusRead(&rxFreeUnderflowCount,
                                   &rxOverflowCount);

    printf ("Rx Free Underflow (Npe) ................... : %10u (should be 0)\n",
        rxFreeUnderflowCount);
    printf ("Rx Overflow (Npe) ......................... : %10u (should be 0)\n",
        rxOverflowCount);
    printf("\n");

    /* iterate through all VCs */
    for (vc = 0; vc < IX_ATM_MAX_NUM_AAL_OAM_RX_VCS; vc++)
    {
        if (IX_ATMDACC_RX_VC_INUSE(ixAtmdAccRxVcDescriptor[vc].connId))
        {
            char *stringAalType = "???????";
            char *stringState   = "???????";

            switch(ixAtmdAccRxVcDescriptor[vc].status)
            {
            case IX_ATMDACC_RX_CHANNEL_UP: 
                stringState = "Up";
                break;
            case IX_ATMDACC_RX_CHANNEL_DOWN:
                stringState = "DOWN";
                break;
            case IX_ATMDACC_RX_CHANNEL_DOWN_PENDING:
                stringState = "DOWN PENDING";
                break;
            }

            switch(ixAtmdAccRxVcDescriptor[vc].npeAalType)
            {
            case IX_NPE_A_AAL_TYPE_5:
                stringAalType = "AAL5   ";
                break;
            case IX_NPE_A_AAL_TYPE_0_48:
                stringAalType = "AAL0_48";
                break;
            case IX_NPE_A_AAL_TYPE_0_52:
                stringAalType = "AAL0_52";
                break;
            case IX_NPE_A_AAL_TYPE_OAM:
                stringAalType = "OAM    ";
                break;
            case IX_NPE_A_AAL_TYPE_INVALID:
            default:
                break;
            }

            /* display information per VC */
            printf ("Rx Channel %2d %s %s connId %8.8x rxFreeQueueId %2d rxQueueId %2d\n",
                vc,
                stringAalType,
                stringState,
                ixAtmdAccRxVcDescriptor[vc].connId,
                ixAtmdAccRxVcDescriptor[vc].rxFreeQueueId,
                ixAtmdAccRxVcDescriptor[vc].rxQueueId);
            printf ("   RxId %u Port %u Vpi %u Vci %u userId %8.8x\n",
                vc,
                ixAtmdAccRxVcDescriptor[vc].port,
                ixAtmdAccRxVcDescriptor[vc].vpi,
                ixAtmdAccRxVcDescriptor[vc].vci,
                ixAtmdAccRxVcDescriptor[vc].callbackId);
            printf ("   Descriptor head %10u >= tail %10u\n",
                ixAtmdAccRxVcDescriptor[vc].queue.head,
                ixAtmdAccRxVcDescriptor[vc].queue.tail);
            numEntries = 0;
            ixAtmdAccRxVcFreeEntriesQuery(ixAtmdAccRxVcDescriptor[vc].connId,
                &numEntries);
            printf ("   Rx Free queue level %10u\n", 
                ixAtmdAccRxVcDescriptor[vc].rxFreeQueueSize - numEntries);

        } /* end of if(IX_ATMDACC_RX_VC_INUSE) */
    } /* end of for(vc) */

    printf("\n");
    for (rxStream = 0; rxStream < IX_ATM_MAX_RX_STREAMS; rxStream++)
    {
        ixAtmdAccUtilQmgrQIdGet(rxStream, &qId);
        printf("Queue %2u:\n", qId);

        IX_ATMDACC_FULL_STATS(
        printf ("   Rx Dispatch invoke ....................... : %10u\n",
            ixAtmdAccRxDispatchStats[rxStream].invokeCount);
        printf ("   Rx Dispatch failed ....................... : %10u (should be 0)\n",
            ixAtmdAccRxDispatchStats[rxStream].failedCount);
        printf ("   Rx SPath Pdu received .................... : %10u\n",
            ixAtmdAccRxDispatchStats[rxStream].spPduCount);
        printf ("   Rx RxDisableAck received ................. : %10u\n",
            ixAtmdAccRxDispatchStats[rxStream].ackCount);
        printf ("   Rx Max Rx per dispatch ................... : %10u\n",
            ixAtmdAccRxDispatchStats[rxStream].maxRxPerDispatch);
        );

        /* get the current number of Queue entries */
        returnStatus = ixQMgrQNumEntriesGet (qId, &numEntries);
        
        /* read again the number of entries  */
        if (returnStatus == IX_QMGR_WARNING)
        {
            returnStatus = ixQMgrQNumEntriesGet (qId, &numEntries);
        }
        
        if (returnStatus != IX_SUCCESS)
        {
            printf ("   Rx Queue level .............,,,,,,,,,..... :        N/A\n");
        }
        else
        {
            printf ("   Rx Queue level ........................... : %10u\n", numEntries);
        } /* end of if-else(returnStatus) */
    } /* end of for(rxStream) */
    printf("\n");
}


/* ---------------------------------------------------------
* display the rx configuration
*/
void
ixAtmdAccRxCfgInfoChannelShow (IxAtmLogicalPort port)
{
    unsigned int vc;

    /* iterate through the VCs for this port */
    for (vc = 0; vc < IX_ATM_MAX_NUM_AAL_OAM_RX_VCS; vc++)
    {
        if (IX_ATMDACC_RX_VC_INUSE (ixAtmdAccRxVcDescriptor[vc].connId))
        {
            if (ixAtmdAccRxVcDescriptor[vc].port == port)
            {
                IX_ATMDACC_RX_QUEUE *rxSwQueue = &ixAtmdAccRxVcDescriptor[vc].queue;
                unsigned int numEntries = 0;
                IX_STATUS returnStatus = 
                    ixQMgrQNumEntriesGet(ixAtmdAccRxVcDescriptor[vc].rxFreeQueueId,
                    &numEntries);

                char *stringAalType = "???????";

                switch(ixAtmdAccRxVcDescriptor[vc].npeAalType)
                {
                case IX_NPE_A_AAL_TYPE_5:
                    stringAalType = "AAL5   ";
                    break;
                case IX_NPE_A_AAL_TYPE_0_48:
                    stringAalType = "AAL0_48";
                    break;
                case IX_NPE_A_AAL_TYPE_0_52:
                    stringAalType = "AAL0_52";
                    break;
                case IX_NPE_A_AAL_TYPE_OAM:
                    stringAalType = "OAM    ";
                    break;
                case IX_NPE_A_AAL_TYPE_INVALID:
                default:
                    break;
                }

                
                if (returnStatus == IX_QMGR_WARNING)
                {
                    returnStatus = ixQMgrQNumEntriesGet(ixAtmdAccRxVcDescriptor[vc].rxFreeQueueId,
                        &numEntries);
                    if (returnStatus == IX_QMGR_WARNING)
                    {
                        numEntries = 0;
                    }
                }

                /* display data about VC */
                printf ("   Rx Channel %2u %s ConnId 0x%8.8x UserId 0x%8.8x\n",
                    vc,
                    stringAalType,
                    ixAtmdAccRxVcDescriptor[vc].connId,
                    ixAtmdAccRxVcDescriptor[vc].callbackId);
                printf ("      Vpi %3u Vci %5u MbfsRx %10u MbfsRxf %10u\n",
                    ixAtmdAccRxVcDescriptor[vc].vpi,
                    ixAtmdAccRxVcDescriptor[vc].vci,
                    rxSwQueue->head - rxSwQueue->size,
                    numEntries);
            }
        }
    }
}
