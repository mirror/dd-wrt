/**
* @file ixAtmdTxCfgInfo.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief ATM TX data accessors
*
* Access the data required to process  the TX traffic
*
* Design Notes:
*    Should run under the ixAtmdAccTxCfgIf lock
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

#include "IxAtmdAccCtrl.h"
#include "IxAtmdDefines_p.h"
#include "IxAtmdAssert_p.h"
#include "IxAtmdTxCfgInfo_p.h"
#include "IxAtmdTxTransport_p.h"
#include "IxAtmdPortMgmt_p.h"
#include "IxAtmdDescMgmt_p.h"
#include "IxAtmdUtil_p.h"

#ifndef NDEBUG
static unsigned int ixAtmdAccTxLowDefaultCallbackCount = 0;
static unsigned int ixAtmdAccTxDoneDefaultCallbackCount = 0;
#endif

/* Function prototype */
PRIVATE void
ixAtmdAccTxLowDummyCallback (IxAtmLogicalPort port,
                             unsigned int numberOfRemainingCells);

PUBLIC IX_STATUS
ixAtmdAccTxDoneDummyCallback (unsigned int numberOfPdusToProcess,
                              unsigned int *numberOfPdusProcessedPtr);

/* --------------------------------------------------------
* Tx notification default callback
*/
PRIVATE void
ixAtmdAccTxLowDummyCallback (IxAtmLogicalPort port,
                             unsigned int numberOfRemainingCells)
{
    IX_ATMDACC_FULL_STATS(ixAtmdAccTxLowDefaultCallbackCount++; );
}

/* --------------------------------------------------------
* Tx Done default callback
*/
PUBLIC IX_STATUS
ixAtmdAccTxDoneDummyCallback (unsigned int numberOfPdusToProcess,
                              unsigned int *numberOfPdusProcessedPtr)
{

    IX_ATMDACC_FULL_STATS( ixAtmdAccTxDoneDefaultCallbackCount++; );
    return IX_FAIL;
}

/* ---------------------------------------------------------
* basic initialisation entry point for tx
*/
IX_STATUS
ixAtmdAccTxCfgInfoInit (void)
{
    unsigned int vc;
    IxAtmLogicalPort portIndex;
    unsigned int txQueueSize;
    unsigned int numTxVcQueues;
    IxQMgrQId txQueueId;
    IX_STATUS returnStatus = IX_SUCCESS;
    IX_STATUS qmgrStatus = IX_SUCCESS;
    unsigned int totalTxQueueSize;
    unsigned int averageTxQueueSize;

    /* initialize the tx done buffer used during burstRead from tx done queue */
    ixOsalMemSet(ixAtmdAccTxDoneDispatchBuffer, 0, sizeof(ixAtmdAccTxDoneDispatchBuffer));

    /* initialise the data structures */
    for (vc = 0; vc < IX_ATM_MAX_NUM_AAL_OAM_TX_VCS; vc++)
    {
        ixOsalMemSet (&ixAtmdAccTxVcDescriptor[vc], 0, sizeof (IxAtmdAccTxVcDescriptor));
        ixAtmdAccTxVcDescriptor[vc].connId = IX_ATMDACC_TX_INVALID_CONNID;
        ixAtmdAccTxVcDescriptor[vc].port   = NPE_INVALID_LOGICAL_PORT;
        ixAtmdAccTxVcDescriptor[vc].vpi    = NPE_INVALID_VPI;
        ixAtmdAccTxVcDescriptor[vc].vci    = NPE_INVALID_VCI;
    } /* end of for(vc) */

    for (portIndex = 0; portIndex < IX_UTOPIA_MAX_PORTS; portIndex++)
    {
        ixAtmdAccPortDescriptor[portIndex].txLowCallback    = ixAtmdAccTxLowDummyCallback;
        ixAtmdAccPortDescriptor[portIndex].txQueueId        = IX_QMGR_QUEUE_INVALID;
        ixAtmdAccPortDescriptor[portIndex].status           = IX_ATMDACC_PORT_DOWN;
        ixAtmdAccPortDescriptor[portIndex].schDemandUpdate  = ixAtmdAccTxDummyDemandUpdate;
        ixAtmdAccPortDescriptor[portIndex].schDemandClear   = ixAtmdAccTxDummyDemandClear;
        ixAtmdAccPortDescriptor[portIndex].schVcIdGet       = ixAtmdAccTxDummyVcIdGet;
        ixAtmdAccPortDescriptor[portIndex].txQueueSize      = IX_QMGR_Q_SIZE_INVALID;
    } /* end of for(portIndex) */

    /* set the default tx done handler */
    numTxVcQueues = 0;
    ixAtmdAccTxDoneDispatcher = ixAtmdAccTxDoneDispatch;

    /* start detecting the configured tx vc queues */
    totalTxQueueSize = 0;

    for (portIndex = 0;
        (qmgrStatus == IX_SUCCESS) &&
        (portIndex < IX_UTOPIA_MAX_PORTS);
        portIndex++)
    {
        txQueueId = portIndex + IX_NPE_A_QMQ_ATM_TX0;
        qmgrStatus = ixQMgrQSizeInEntriesGet (txQueueId, &txQueueSize);

        if(qmgrStatus == IX_SUCCESS)
        {
            /* set the queue size and the default queue threshold */
            numTxVcQueues++;
            ixAtmdAccPortDescriptor[portIndex].txQueueId = txQueueId;
            ixAtmdAccPortDescriptor[portIndex].txQueueSize = txQueueSize;
            totalTxQueueSize += txQueueSize;

        }
    } /* end of for(portIndex) */

    if (numTxVcQueues > 0)
    {
    /* ensure the size of tx free queues is as expected.
    * This is used to prevent a misconfiguration of the descriptor pool
    */
        averageTxQueueSize = totalTxQueueSize / numTxVcQueues;

        /* no way to continue, the system configuration is wrong */
        IX_ATMDACC_ABORT(averageTxQueueSize <= IX_ATMDACC_AVERAGE_TXVC_QUEUE_SIZE,
            "IX_ATMDACC_AVERAGE_TXVC_QUEUE_SIZE not consistent with queue configuration");

        returnStatus = ixAtmdAccPortMgmtNumTxVcQueuesSet (numTxVcQueues);
    } /* end of if(numTxVcQueues) */
    else
    {
        returnStatus = IX_FAIL;
    } /* end of if-else(numTxVcQueues) */

    return returnStatus;
}

/* ------------------------------------------------------------
* reset all config params to initial state
*/
void
ixAtmdAccTxCfgInfoStatsReset (void)
{
    IX_ATMDACC_FULL_STATS( 
    ixAtmdAccTxLowDefaultCallbackCount = 0;
    ixAtmdAccTxDoneDefaultCallbackCount = 0;
    ixOsalMemSet (&ixAtmdAccTxTransportStats, 0, sizeof(ixAtmdAccTxTransportStats));
    ixOsalMemSet (&ixAtmdAccTxDoneDispatchStats, 0, sizeof(ixAtmdAccTxDoneDispatchStats));
    );
}

/* --------------------------------------------------------
* check if the specific port, vpi, and vci is confifured
*/
BOOL
ixAtmdAccTxCfgVcConfigured (IxAtmLogicalPort port,
                            unsigned int vpi,
                            unsigned int vci)
{
    /* assume that we will not find a match */
    BOOL result = FALSE;
    int vc;

    /* iterate through the VCs searching for a match */
    for (vc = 0; result == FALSE && vc < IX_ATM_MAX_NUM_AAL_OAM_TX_VCS; vc++)
    {
        /* only test used VCs */
        if (IX_ATMDACC_TX_VC_INUSE (ixAtmdAccTxVcDescriptor[vc].connId))
        {
            /* check if it matches */
            if (ixAtmdAccTxVcDescriptor[vc].vci == vci
                && ixAtmdAccTxVcDescriptor[vc].vpi == vpi && ixAtmdAccTxVcDescriptor[vc].port == port)
            {
                /* yes, matched so return the connection Id */
                result = TRUE;
            }
        } /* end of if(IX_ATMDACC_TX_VC_INUSE) */
    } /* end of for(vc) */
    return result;
}

/* ------------------------------------------------------
* check if the specified port is idle
*/
BOOL
ixAtmdAccTxCfgPortVcsExist (IxAtmLogicalPort port)
{
    BOOL result = FALSE;
    int vc;

    /* check if already existing */
    for (vc = 0; !result && (vc < IX_ATM_MAX_NUM_AAL_OAM_TX_VCS); vc++)
    {
        if (IX_ATMDACC_TX_VC_INUSE (ixAtmdAccTxVcDescriptor[vc].connId))
        {
            if (ixAtmdAccTxVcDescriptor[vc].port == port)
            {
                result = TRUE;
            }
        }
    } /* end of for(vc) */
    return result;
}

/* ------------------------------------------------------
* check if all connections are idle
*/
BOOL
ixAtmdAccTxCfgVcsExist (void)
{
    BOOL result = FALSE;
    int vc;

    /* check if already existing */
    for (vc = 0; !result && (vc < IX_ATM_MAX_NUM_AAL_OAM_TX_VCS); vc++)
    {
        if (IX_ATMDACC_TX_VC_INUSE (ixAtmdAccTxVcDescriptor[vc].connId))
        {
            result = TRUE;
        }
    }
    return result;
}

/* ------------------------------------------------
* Get a free AAL tx channel and allocate a new connection id
*/
IX_STATUS
ixAtmdAccTxCfgFreeChannelGet (IxAtmdAccAalType aalServiceType, IxAtmConnId * connIdPtr)
{
/* assume that if we dont find any free tx descriptors
then the system is overloaded
    */
    int txVc=0;
    IX_STATUS returnStatus = IX_ATMDACC_BUSY;

    switch( aalServiceType )
    {
    case IX_ATMDACC_AAL5:
    case IX_ATMDACC_AAL0_48:
    case IX_ATMDACC_AAL0_52:

        /* iterate through the Tx descriptor table looking for a free entry */
        txVc=0;
        while (returnStatus != IX_SUCCESS && txVc < IX_ATM_MAX_NUM_AAL_VCS)
        {
            if (!IX_ATMDACC_TX_VC_INUSE (ixAtmdAccTxVcDescriptor[txVc].connId))
            {
                returnStatus = IX_SUCCESS;
            }
            else
            {
                /* this entry is not free so step on to the next */
                txVc++;
            } /* end of if-else(IX_ATMDACC_TX_VC_INUSE) */
        } /* end of while(returnStatus) */
        break;
    case IX_ATMDACC_OAM:
        txVc=IX_ATM_MAX_NUM_AAL_OAM_TX_VCS;
        while (returnStatus != IX_SUCCESS &&  
               --txVc >= IX_ATM_MAX_NUM_AAL_VCS)
        {
            if (!IX_ATMDACC_TX_VC_INUSE (ixAtmdAccTxVcDescriptor[txVc].connId))
            {
                returnStatus = IX_SUCCESS;
            }
        } /* end of while(returnStatus) */
        break;
    default:
        /* Params are checked on API this should not happen */
        IX_ATMDACC_ENSURE(0,"Bad service type");
        break;
    }

    if( returnStatus == IX_SUCCESS )
    {
        IxAtmConnId newConnId = txVc + (ixAtmdAccUtilUniqueIdGet () * IX_ATM_MAX_NUM_AAL_OAM_TX_VCS);
        *connIdPtr = newConnId;
    }
    return returnStatus;
}



/* --------------------------------------------------------
*  initialise channel variables, allocate and initialise
*  NPE descriptors for the transmit software queue.
*/
IX_STATUS
ixAtmdAccTxCfgChannelSet (IxAtmConnId connId,
                          IxAtmSchedulerVcId schedulerVcId,
                          IxAtmLogicalPort port,
                          unsigned int vpi,
                          unsigned int vci,
                          IxAtmdAccAalType aalServiceType,
                          IxAtmdAccUserId callbackId,
                          IxAtmdAccTxVcBufferReturnCallback txDoneCallback)
{
    unsigned int descCount;
    UINT8 npeAalType = IX_NPE_A_AAL_TYPE_INVALID;
    IX_STATUS returnStatus = IX_SUCCESS;
    unsigned int txId;
    IxAtmdAccTxVcDescriptor *vcDescriptor = NULL;
    IX_ATMDACC_TX_QUEUE *txSwQueue = NULL;
    IxAtmdAccPortDescriptor *portDescriptor = NULL;
    unsigned int requiredSwQueueSize = 0;
    unsigned int actualSwQueueSize = 0;
    unsigned int cellSize = 0;
    txId = IX_ATMDACC_TX_VC_INDEX_GET (connId);
    vcDescriptor = &ixAtmdAccTxVcDescriptor[txId];
    txSwQueue = &vcDescriptor->queue;
    portDescriptor = &ixAtmdAccPortDescriptor[port];
    /* default queue size for all service types, maybe overridden later */
    requiredSwQueueSize = ((int)portDescriptor->txQueueSize - IX_ATMDACC_AVERAGE_TXVC_QUEUE_SIZE) + IX_ATMDACC_TX_SWQ_SIZE;

    /* perform service specific operations */
    /* convert the atmd service type to an Npe aalType */
    /* set the cell size */
    switch(aalServiceType)
    {
    case IX_ATMDACC_AAL5:
        npeAalType = NPE_AAL5_TYPE;
        cellSize = IX_ATM_CELL_PAYLOAD_SIZE;
        break;
    case IX_ATMDACC_AAL0_48:
        npeAalType = NPE_AAL0_48_TYPE;
        cellSize = IX_ATM_AAL0_48_CELL_PAYLOAD_SIZE;
        break;
    case IX_ATMDACC_AAL0_52:
        npeAalType = NPE_AAL0_52_TYPE;
        cellSize = IX_ATM_AAL0_52_CELL_SIZE_NO_HEC;
        break;
    case IX_ATMDACC_OAM:
        npeAalType = NPE_OAM_TYPE;
        cellSize = IX_ATM_AAL0_52_CELL_SIZE_NO_HEC;
        requiredSwQueueSize = ((int)portDescriptor->txQueueSize - IX_ATMDACC_AVERAGE_TXVC_QUEUE_SIZE) + IX_ATMDACC_TX_OAM_SWQ_SIZE;
        break;
    default:
        returnStatus = IX_FAIL;
        break;
    }

    if( returnStatus == IX_SUCCESS )
    {
        /* Initialise the TX Queue */
        IX_ATMDACC_TXQ_INIT(txSwQueue, requiredSwQueueSize,vcDescriptor->swQueueBuffer);

        if(!IX_ATMDACC_TXQ_INITIALISED (txSwQueue))
        {
            returnStatus = IX_FAIL;
        }

        if (returnStatus == IX_SUCCESS)
        {
            /* discover what size queue was actually created for us,...
            * it will be greater than or equal to what we requested
            */
            actualSwQueueSize = IX_ATMDACC_TXQ_SIZE(txSwQueue);

            /* fill the parameters */
            vcDescriptor->schedulerVcId = schedulerVcId;
            vcDescriptor->connId = connId;
            vcDescriptor->port = port;
            vcDescriptor->vpi = vpi;
            vcDescriptor->vci = vci;
            vcDescriptor->npeAalType = npeAalType;
            vcDescriptor->cellSize = cellSize;
            vcDescriptor->callbackId = callbackId;
            vcDescriptor->txDoneCallback = txDoneCallback;
            vcDescriptor->currentNpeDesc = 0;
            vcDescriptor->remainingPduCellCount = 0;
            IX_ATMDACC_FULL_STATS(
            vcDescriptor->txSubmitOverloadedCount = 0;      
            vcDescriptor->txVcPduSubmitFailureCount = 0; );

            /* update the descriptor fields */
            for (descCount = 0; returnStatus == IX_SUCCESS && descCount < actualSwQueueSize; descCount++)
            {
                IxAtmdAccNpeDescriptor *npeDescriptor;

                /* get a NPE descriptor 
                */
                returnStatus = ixAtmdAccDescNpeDescriptorGet (&npeDescriptor);

                if (returnStatus == IX_SUCCESS)
                {
                    IX_ATMDACC_TXQ_ENTRY_IDXSET (txSwQueue, descCount, npeDescriptor);

                    /*fill the NPE descriptor */
                    npeDescriptor->atmd.connId = connId;
                    npeDescriptor->npe.tx.port = port;
                    npeDescriptor->npe.tx.atmCellHeader = 0;
                    IX_NPE_A_ATMCELLHEADER_VPI_SET(npeDescriptor->npe.tx.atmCellHeader, vpi);
                    IX_NPE_A_ATMCELLHEADER_VCI_SET(npeDescriptor->npe.tx.atmCellHeader, vci);
                    IX_ATMDACC_CONVERT_TO_BIG_ENDIAN( UINT32, npeDescriptor->npe.tx.atmCellHeader);
                    npeDescriptor->npe.tx.aalType = npeAalType;
                } /* end of if(returnStatus) */
            } /* end of for(descCount) */
        } /* end of if(returnStatus) */
    }
    /* rollback if error */
    if (returnStatus != IX_SUCCESS)
    {
        ixAtmdAccTxCfgChannelReset (txId);
    }
    return returnStatus;
}

/* -------------------------------------------------
* reset the vc channel structure
*/
IX_STATUS ixAtmdAccTxCfgChannelReset (unsigned int txId)
{
    unsigned int descCount;
    unsigned int queueSize;
    IX_STATUS returnStatus = IX_SUCCESS;
    IxAtmdAccTxVcDescriptor *vcDescriptor = &ixAtmdAccTxVcDescriptor[txId];
    IX_ATMDACC_TX_QUEUE *txSwQueue = &vcDescriptor->queue;

    /* reset the descriptor parameters */
    vcDescriptor->connId = IX_ATMDACC_TX_INVALID_CONNID;        /* store an invalid value */
    vcDescriptor->port = NPE_INVALID_LOGICAL_PORT;        /* store an invalid value */
    vcDescriptor->vpi = NPE_INVALID_VPI;        /* store an invalid value */
    vcDescriptor->vci = NPE_INVALID_VCI;

    IX_ATMDACC_TXQ_RESET (txSwQueue);

    /* if any descriptors exist then release them */
    if (IX_ATMDACC_TXQ_INITIALISED (txSwQueue))
    {
        queueSize = IX_ATMDACC_TXQ_SIZE (txSwQueue);

        for (descCount = 0; descCount < queueSize; descCount++)
        {
            IxAtmdAccNpeDescriptor *npeDescriptor =
                IX_ATMDACC_TXQ_ENTRY_IDXGET (txSwQueue, descCount);

            if (npeDescriptor != NULL)
            {
                ixAtmdAccDescNpeDescriptorRelease (npeDescriptor);
            }
        } /* end of for(descCount) */
        /* release the  memory used by the tx software NPE descriptor pointer queue */
        IX_ATMDACC_TXQ_RELEASE_RESOURCES (txSwQueue);
    } /* end of if(IX_ATMDACC_TXQ_INITIALISED) */
    return returnStatus;
}

/* -------------------------------------------------
* get the index in the pool of vcs
*/
IX_STATUS
ixAtmdAccTxCfgIndexGet (IxAtmConnId connId,
                        unsigned int *txId)
{
    IxAtmdAccTxVcDescriptor *vcDescriptor;
    IX_STATUS returnStatus = IX_SUCCESS;

    *txId = IX_ATMDACC_TX_VC_INDEX_GET (connId);
    vcDescriptor = &ixAtmdAccTxVcDescriptor[*txId];

    /* check if the connection ID is valid */
    if (vcDescriptor->connId != connId)
    {
        /* check if the connection ID has been invalidated */
        if (!IX_ATMDACC_TX_DISCONNECTCHECK (connId, vcDescriptor->connId))
        {
            /* the connection ID does not correspond with the expected one */
            returnStatus = IX_FAIL;
        }
    } /* end of if(vcDescriptor) */
    return returnStatus;
}

/* ----------------------------------------------
* Invalidate the connId in the descriptor at this
* index in the tx descriptor pool
*/
void
ixAtmdAccTxCfgConnIdInvalidate (unsigned int txId)
{

    IxAtmdAccTxVcDescriptor *vcDescriptor = &ixAtmdAccTxVcDescriptor[txId];

    /* check if the connId is already invalidated */
    if (!IX_ATMDACC_TX_DISCONNECTED (vcDescriptor->connId))
    {
        vcDescriptor->connId = IX_ATMDACC_TX_DISCONNECTING (vcDescriptor->connId);
        /* connId is now invalidated : a direct comparison of the connId
        * passed as a parameter and the connId stored in the structure
        * will fail
        */
    }
    return;
}

/* --------------------------------------------------
* check if any tx descriptors are still in use
*/
BOOL
ixAtmdAccTxCfgFreeResourcesCheck (unsigned int txId)
{
    BOOL result = TRUE;
    IxAtmdAccTxVcDescriptor *vcDescriptor = &ixAtmdAccTxVcDescriptor[txId];
    IX_ATMDACC_TX_QUEUE *txSwQueue = &vcDescriptor->queue;

    /* check if resources are all transmitted */
    if ((ixAtmdAccPortDescriptor[vcDescriptor->port].status == IX_ATMDACC_PORT_DOWN_PENDING) ||
        IX_ATMDACC_TXQ_SCHEDULE_PENDING (txSwQueue) ||
        IX_ATMDACC_TXQ_RECYCLE_PENDING (txSwQueue))
    {
        /* resources are not all free: everything is not transmitted OR 
        * tx done is not complete 
        */
        result = FALSE;
    } /* end of if(ixAtmdAccPortDescriptor) */
    return result;
}

/* ---------------------------------------------------
* store the user callback, and regsiter the corresponding
* Atmd callback with Qmgr.
*/
IX_STATUS
ixAtmdAccTxCfgTxDoneCallbackRegister (unsigned int thresholdLevel,
                                      IxAtmdAccTxDoneDispatcher newCallback)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* check the threshold is less than the queue size */
    if (thresholdLevel > (IX_ATMDACC_TXDONE_QUEUE_SIZE / 2))
    {
        returnStatus = IX_FAIL;
    }
    else
    {
    /* store the new callback before registering
    * Atmd with Qmgr because this enables interrupts
    */
        ixAtmdAccTxDoneDispatcher = newCallback;
        
        /* register with Qmgr */
        if(ixAtmdAccUtilQmgrCallbackSet (IX_NPE_A_QMQ_ATM_TX_DONE,
            thresholdLevel,
            /*  N.B. The NPE reserves the High Watermark for its operation. But it must
                     be set by the Xscale. */
            IX_NPE_A_TXDONE_QUEUE_HIGHWATERMARK,
            IX_QMGR_Q_SOURCE_ID_NOT_NE,
            ixAtmdAccTxDoneCallBack,
            (IxQMgrCallbackId)IX_NPE_A_QMQ_ATM_TX_DONE,
            IX_QMGR_Q_PRIORITY_1) != IX_SUCCESS)
        {
            returnStatus = IX_FAIL;
        }
    }
    return returnStatus;
}


/* ---------------------------------------------------
* Disable Notification for IX_NPE_A_QMQ_ATM_TX_DONE Queue.
*/
void
ixAtmdAccTxCfgTxDoneCallbackUnregister (void)
{
    ixQMgrNotificationDisable (IX_NPE_A_QMQ_ATM_TX_DONE);
}



/* -----------------------------------------------------
* store the user callback, and regsiter the corresponding
* Atmd callback with Qmgr.
*/
IX_STATUS
ixAtmdAccTxCfgTxCallbackRegister (IxAtmLogicalPort port,
                                  unsigned int newThresholdLevel,
                                  IxAtmdAccPortTxLowCallback newCallback)
{
    IX_STATUS returnStatus = IX_SUCCESS;
    IxQMgrQId txQueueId = ixAtmdAccPortDescriptor[port].txQueueId;

    /* check the threshold is less than the queue size */
    if (newThresholdLevel > (ixAtmdAccPortDescriptor[port].txQueueSize / 2))
    {
        returnStatus = IX_FAIL;
    }
    else
    {
    /* store the new callback before registering
    * Atmd with Qmgr because this enables interrupts
    */
        ixAtmdAccPortDescriptor[port].txLowCallback = newCallback;
        ixAtmdAccPortDescriptor[port].txQueueThreshold = newThresholdLevel;
        
        /* register with Qmgr */
        if(ixAtmdAccUtilQmgrCallbackSet (txQueueId,
            newThresholdLevel,
            newThresholdLevel,
            IX_QMGR_Q_SOURCE_ID_NE,
            ixAtmdAccTxLowCallBack,
            (IxQMgrCallbackId) port,
            IX_QMGR_Q_PRIORITY_0) != IX_SUCCESS)
        {
            returnStatus = IX_FAIL;
        }
    }

    return returnStatus;
}


/* ---------------------------------------------------
* Disable Notification for IX_NPE_A_QMQ_ATM_TX_DONE Queue.
*/
void
ixAtmdAccTxCfgTxCallbackUnregister (IxAtmLogicalPort port)
{
    IxQMgrQId txQueueId = ixAtmdAccPortDescriptor[port].txQueueId;
    ixQMgrNotificationDisable (txQueueId);
}



/* ------------------------------------------------------
* dummy scheduler clear callback
*/
void
ixAtmdAccTxDummyDemandClear (IxAtmLogicalPort port,
                             int vcId)
{
    /* nothing to do because the dummy scheduler has no state */
    return;
}

/* -------------------------------------------------------
* get the schedluer Id from the dummy scheduler
*/
IX_STATUS
ixAtmdAccTxDummyVcIdGet (IxAtmLogicalPort port,
                         unsigned int vpi,
                         unsigned int vci,
                         IxAtmConnId connId,
                         int *vcId)
{
    /* for the dummy scheduler we use the
    Tx index as the scheduler Id
    */
    *vcId = IX_ATMDACC_TX_VC_INDEX_GET (connId);
    return IX_SUCCESS;
}

/* --------------------------------------------------------
* store the user scheduler callbacks, for the specified port
*/
void
ixAtmdAccTxCfgSchCallbackRegister (IxAtmLogicalPort port,
                                   IxAtmdAccTxVcDemandUpdateCallback demandUpdate,
                                   IxAtmdAccTxVcDemandClearCallback demandClear,
                                   IxAtmdAccTxSchVcIdGetCallback vcIdGet)
{
    ixAtmdAccPortDescriptor[port].schDemandUpdate = demandUpdate;
    ixAtmdAccPortDescriptor[port].schDemandClear = demandClear;
    ixAtmdAccPortDescriptor[port].schVcIdGet = vcIdGet;
    return;
}



/* --------------------------------------------------------
* Reset the scheduler callbacks, for the specified port
*/
void
ixAtmdAccTxCfgSchCallbackUnregister (IxAtmLogicalPort port)
{
    /* Load dummy callbacks to unregister */
    ixAtmdAccPortDescriptor[port].schDemandUpdate = ixAtmdAccTxDummyDemandUpdate;
    ixAtmdAccPortDescriptor[port].schDemandClear  = ixAtmdAccTxDummyDemandClear;
    ixAtmdAccPortDescriptor[port].schVcIdGet      = ixAtmdAccTxDummyVcIdGet;
}



/* ----------------------------------------------------------
* force the NPE tx queue to contain ony idle cells for
* this channel
*/
IX_STATUS
ixAtmdAccTxCfgPortResourcesRelease (IxAtmConnId connId)
{
    unsigned int txId;
    IxAtmLogicalPort port;
    IxAtmdAccTxVcDescriptor *vcDescriptor;
    IX_ATMDACC_TX_QUEUE *txSwQueue;
    IxAtmdAccNpeDescriptor *npeDescriptor;
    IX_OSAL_MBUF *mbufPtr;
    UINT32 qEntry;
    IX_STATUS returnStatus = IX_SUCCESS;
    unsigned int entryIndex = 0;

    txId = IX_ATMDACC_TX_VC_INDEX_GET (connId);
    vcDescriptor = &ixAtmdAccTxVcDescriptor[txId];
    txSwQueue = &vcDescriptor->queue;
    port = vcDescriptor->port;

    if ((ixAtmdAccPortDescriptor[vcDescriptor->port].status == IX_ATMDACC_PORT_DOWN) &&
        (vcDescriptor->pduTransmitInProgress == FALSE) &&
        (ixAtmdAccPortDescriptor[port].schedulingInProgress == FALSE) &&
        (IX_ATMDACC_TXQ_SCHEDULE_PENDING (txSwQueue) || IX_ATMDACC_TXQ_RECYCLE_PENDING (txSwQueue)))
    {        
        while ((returnStatus == IX_SUCCESS) &&
            (ixQMgrQPeek (ixAtmdAccPortDescriptor[port].txQueueId,
            entryIndex,
            &qEntry)== IX_SUCCESS))
        {
            IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN (UINT32, qEntry);
            
            if ((qEntry == 0) || ((qEntry & NPE_TX_CELLTYPE_MASK) == NPE_TX_IDLECELL))
            {
                /* idle cell or empty entry, nothing to do */
            }
            else
            {
                /* it must be a data cell */
                
                npeDescriptor = (IxAtmdAccNpeDescriptor *) (qEntry & NPE_DESCRIPTOR_MASK);
                IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS (npeDescriptor);
                
                /* check if the cell belong to this channel */
                if (npeDescriptor->atmd.connId == connId)
                {
                    /* force this entry to be an idle cell */
                    qEntry &= ~NPE_TX_CELLTYPE_MASK;
                    qEntry |= NPE_TX_IDLECELL;
                    IX_ATMDACC_CONVERT_TO_BIG_ENDIAN (UINT32, qEntry);
                    
                    /* write the queue entry */
                    returnStatus = ixQMgrQPoke (ixAtmdAccPortDescriptor[port].txQueueId,
                        entryIndex,
                        &qEntry);
                } /* end of if(npeDescriptor) */
            } /* end of if-else(qEntry) */
            /* check the next entry */
            entryIndex++;
        }
        
        /* flush any pending PDUs by making it appear
        * that they have been scheduled
        */
        while (IX_ATMDACC_TXQ_SCHEDULE_PENDING (txSwQueue))
        {
            IX_ATMDACC_TXQ_TAIL_INCR (txSwQueue);
        }
        /* flush any pending PDUs by making it appear
        * that they have been transmitted
        */
        while (IX_ATMDACC_TXQ_RECYCLE_PENDING (txSwQueue))
        {
            npeDescriptor = IX_ATMDACC_TXQ_HEAD_ENTRY_GET (txSwQueue);
            mbufPtr = ixAtmdAccUtilMbufFromNpeFormatConvert (npeDescriptor->atmd.pRootMbuf,
							     FALSE);
            IX_ATMDACC_TXQ_HEAD_INCR (txSwQueue);
            vcDescriptor->txDoneCallback (vcDescriptor->callbackId, mbufPtr);
        }            
    } /* end of if(ixAtmdAccPortDescriptor) */
    return returnStatus;
}

/* ----------------------------------------------------------
* Handle port state changes initiated by PortMgmt
*/
IX_STATUS
ixAtmdAccTxPortStateChangeHandler(IxAtmLogicalPort port, IxAtmdAccPortState requestedState)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    switch (ixAtmdAccPortDescriptor[port].status)
    {
    case IX_ATMDACC_PORT_UP:
        if (requestedState == IX_ATMD_PORT_DISABLED)
        {
        /* set the state to be "down pending". When the acknowledge message will
        * be received through the tx done queue, the state will be changed
        * to "down"
            */
            ixAtmdAccPortDescriptor[port].status = IX_ATMDACC_PORT_DOWN_PENDING;
            if (ixAtmdAccUtilNpeMsgSend(IX_NPE_A_MSSG_ATM_TX_DISABLE,
                port,
                NPE_IGNORE) != IX_SUCCESS)
            {
                /* rollback the state change */
                ixAtmdAccPortDescriptor[port].status = IX_ATMDACC_PORT_UP;
                returnStatus = IX_FAIL;
            } /* end of if(ixAtmdAccUtilNpeMsgSend) */
        } /* end of if(requestedState) */
        else if (requestedState == IX_ATMD_PORT_ENABLED)
        {
            returnStatus = IX_ATMDACC_WARNING;
        }
        break;

    case IX_ATMDACC_PORT_DOWN_PENDING:
        if (requestedState == IX_ATMD_PORT_ENABLED)
        {
            returnStatus = IX_FAIL;
        }
        else if (requestedState == IX_ATMD_PORT_DISABLED)
        {
            returnStatus = IX_ATMDACC_WARNING;
        }
        break;

    case IX_ATMDACC_PORT_DOWN:
        if (requestedState == IX_ATMD_PORT_ENABLED)
        {
            ixAtmdAccPortDescriptor[port].status = IX_ATMDACC_PORT_UP;
            /* Are we guaranteed this will succeed */
            if (ixAtmdAccUtilNpeMsgSend(IX_NPE_A_MSSG_ATM_TX_ENABLE,
                port,
                NPE_IGNORE) != IX_SUCCESS)
            {
                /* rollback the state change */
                ixAtmdAccPortDescriptor[port].status = IX_ATMDACC_PORT_DOWN;
                returnStatus = IX_FAIL;
            } /* end of if(ixAtmdAccUtilNpeMsgSend) */
        } /* end of if(requestedState) */
        else if (requestedState == IX_ATMD_PORT_DISABLED)
        {
            returnStatus = IX_ATMDACC_WARNING;
        }
        break;

    default:
        IX_ATMDACC_ENSURE(0,"\nInvalid port state");
        returnStatus = IX_FAIL;
        break;
    } /* end of switch(ixAtmdAccPortDescriptor) */
    return returnStatus;
}

/* ----------------------------------------------------------
* Handle port setup initiated by PortMgmt
*/
void
ixAtmdAccTxPortSetupNotifyHandler(unsigned int numPort)
{
    ixAtmdAccTxNumPortConfigured = numPort;
}

/* -------------------------------------------------
* Check that the port is enabled
*/
BOOL
ixAtmdAccTxPortEnabledQuery(IxAtmLogicalPort port)
{
    if (ixAtmdAccPortDescriptor[port].status == IX_ATMDACC_PORT_UP)
    {
        return TRUE;
    }
    return FALSE;
}

/* -------------------------------------------------
* Check that the port is disabled
*/
BOOL
ixAtmdAccTxPortDisabledQuery(IxAtmLogicalPort port)
{
    if (ixAtmdAccPortDescriptor[port].status == IX_ATMDACC_PORT_DOWN)
    {
        return TRUE;
    }
    return FALSE;
}

/* -------------------------------------------------
* regsiter with the scheduler and obtain
* a scheduler Id
*/
IX_STATUS
ixAtmdAccTxCfgSchVcIdGet (IxAtmLogicalPort port,
                          unsigned int vpi,
                          unsigned int vci,
                          IxAtmConnId connId,
                          IxAtmSchedulerVcId *schedulerVcIdPtr)
{
    return (*ixAtmdAccPortDescriptor[port].schVcIdGet) (port,
        vpi,
        vci,
        connId,
        schedulerVcIdPtr);
}

/* --------------------------------------------------
* Clear scheduler demand for this connection
*/
void
ixAtmdAccTxCfgVcDemandCancel (IxAtmConnId connId,
                              IxAtmLogicalPort port,
                              IxAtmSchedulerVcId schedulerVcId)
{
    unsigned int txId;
    IxAtmdAccTxVcDescriptor *vcDescriptor;

    txId = IX_ATMDACC_TX_VC_INDEX_GET (connId);
    vcDescriptor = &ixAtmdAccTxVcDescriptor[txId];

    /* check if the connId has been invalidated
    * in this case demand will already have been cleared
    * so we do nothing
    */
    if(connId == vcDescriptor->connId)
    {
        /* invoke the scheduler demand clear callback */
        (*ixAtmdAccPortDescriptor[port].schDemandClear) (port,
            schedulerVcId);
    }
    return;
}

/* --------------------------------------------------
* Clear scheduler demand for this connection
*/
void
ixAtmdAccTxCfgVcDemandClear (IxAtmConnId connId)
{
    unsigned int txId;
    IxAtmdAccTxVcDescriptor *vcDescriptor;

    txId = IX_ATMDACC_TX_VC_INDEX_GET (connId);
    vcDescriptor = &ixAtmdAccTxVcDescriptor[txId];

    /* check if the connId has been invalidated
    * in this case demand will already have been cleared
    * so we do nothing
    */
    if(connId != vcDescriptor->connId)
    {
        /* invoke the scheduler demand clear callback */
        (*ixAtmdAccPortDescriptor[vcDescriptor->port].schDemandClear) (vcDescriptor->port,
            vcDescriptor->schedulerVcId);
    }
    return;
}


/* ---------------------------------------------------------
* display the tx configuration
*/
void
ixAtmdAccTxCfgInfoStatsShow (void)
{
    int port;
    unsigned int vc;
    IX_STATUS returnStatus;
    unsigned int numEntries;

    printf("\n");
    IX_ATMDACC_FULL_STATS(
    printf ("Tx Low Default callback count .. : %10u (should be 0)\n",
        ixAtmdAccTxLowDefaultCallbackCount);
    printf("\n");
    );

    /* iterate through the VCs displaying interesting things */
    for (vc = 0; vc < IX_ATM_MAX_NUM_AAL_OAM_TX_VCS; vc++)
    {
        if (IX_ATMDACC_TX_VC_INUSE (ixAtmdAccTxVcDescriptor[vc].connId))
        {
            IX_ATMDACC_TX_QUEUE *txSwQueue = &ixAtmdAccTxVcDescriptor[vc].queue;
            char *stringAalType = "???????";

            switch(ixAtmdAccTxVcDescriptor[vc].npeAalType)
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

            /* display data about VC */
            printf ("Tx Channel %u %s connId %8.8x schedulerId %d\n", 
                vc,
                stringAalType,
                ixAtmdAccTxVcDescriptor[vc].connId,
                ixAtmdAccTxVcDescriptor[vc].schedulerVcId);
            printf ("   TxId %2u Port %2u Vpi %3u Vci %5u userId %8.8x\n",
                vc,
                ixAtmdAccTxVcDescriptor[vc].port,
                ixAtmdAccTxVcDescriptor[vc].vpi,
                ixAtmdAccTxVcDescriptor[vc].vci,
                ixAtmdAccTxVcDescriptor[vc].callbackId);

            printf ("   Descriptor head %10u >= mid %10u >= tail %10u\n",
                txSwQueue->head,
                txSwQueue->mid,
                txSwQueue->tail);

            IX_ATMDACC_FULL_STATS(
            printf ("   Tx Submit Overflow %10u (should be 0)\n",
                ixAtmdAccTxVcDescriptor[vc].txSubmitOverloadedCount);
            printf ("   Tx Submit rejected %10u (should be 0)\n",
                ixAtmdAccTxVcDescriptor[vc].txVcPduSubmitFailureCount);
            );        

            if (IX_ATMDACC_TXQ_OVERLOADED(txSwQueue))
            {
                printf("     Sw queue is full\n");
            }
            if (IX_ATMDACC_TXQ_SCHEDULE_PENDING(txSwQueue))
            {
                printf("     %u pdus need more scheduling\n", 
                    txSwQueue->mid - txSwQueue->tail);
            }
            if (IX_ATMDACC_TXQ_RECYCLE_PENDING(txSwQueue))
            {
                printf("     %u Pdus still in use in the NPE or txDone queue\n",
                    txSwQueue->size - (txSwQueue->head - txSwQueue->tail));
            }
        } /* end of if(IX_ATMDACC_TX_VC_INUSE) */
    } /* end of for(vc) */
    printf("\n");

    for (port = 0; port < IX_UTOPIA_MAX_PORTS; port++)
    {
        char *statusString = "?";

        switch (ixAtmdAccPortDescriptor[port].status)
        {
        case IX_ATMDACC_PORT_DOWN:
            statusString = "Down";
            break;
        case IX_ATMDACC_PORT_UP:
            statusString = "Up";
            break;
        case IX_ATMDACC_PORT_DOWN_PENDING:
            statusString = "Down pending";
            break;
        } /* end of switch(ixAtmdAccPortDescriptor) */

        printf ("Port %2u status %s TxQueue %2u TxThreshold %2u\n",
            port,
            statusString,
            ixAtmdAccPortDescriptor[port].txQueueId,
            ixAtmdAccPortDescriptor[port].txQueueThreshold);

        IX_ATMDACC_FULL_STATS(
        /* display port stats */
        if ((ixAtmdAccTxTransportStats[port].dataCellScheduledCount == 0) &&
            (ixAtmdAccTxTransportStats[port].idleCellScheduledCount == 0))
        {
            printf("   No Tx traffic\n");
        }
        else
        {
            printf ("   Tx Process invoke .............. : %10u\n",
                ixAtmdAccTxTransportStats[port].txProcessInvokeCount);
            printf ("   Tx Process failed .............. : %10u (should be 0)\n",
                ixAtmdAccTxTransportStats[port].txProcessFailedCount);
            printf ("   Tx Pdu scheduled ............... : %10u\n",
                ixAtmdAccTxTransportStats[port].pduScheduledCount);
            printf ("   Tx Data Cell Scheduled ......... : %10u\n",
                ixAtmdAccTxTransportStats[port].dataCellScheduledCount);
            printf ("   Tx Idle cell Scheduled.......... : %10u\n",
                ixAtmdAccTxTransportStats[port].idleCellScheduledCount);
            printf ("   Tx Scheduled during disconnect . : %10u\n",
                ixAtmdAccTxTransportStats[port].disconnectScheduledCount);
            printf ("   Tx Over Scheduled .............. : %10u (should be 0)\n",
                ixAtmdAccTxTransportStats[port].overScheduledCellCount);
            printf ("   Tx Zero cell Scheduled.......... : %10u (should be 0)\n",
                ixAtmdAccTxTransportStats[port].zeroCellCount);
            printf ("   Tx UnderScheduled............... : %10u (should be 0)\n",
                ixAtmdAccTxTransportStats[port].txProcessUnderscheduledCount);
            printf ("   Tx Wrong connId Scheduled....... : %10u (should be 0)\n",
                ixAtmdAccTxTransportStats[port].wrongConnIdScheduledCount);
        });

        /* get the current number of entries */
        returnStatus = ixQMgrQNumEntriesGet (ixAtmdAccPortDescriptor[port].txQueueId, &numEntries);
        
        /* read again the number of entries  */
        if (returnStatus == IX_QMGR_WARNING)
        {
            returnStatus = ixQMgrQNumEntriesGet (ixAtmdAccPortDescriptor[port].txQueueId, &numEntries);
            
        } /* end of if(returnStatus) */
        if (returnStatus != IX_SUCCESS)
        {
            printf ("   Tx Queue level ................. : N/A\n");
        }
        else
        {
            printf ("   Tx Queue level ................. : %10u\n", numEntries);
        } /* end of if-else(returnStatus) */
    } /* end of for(port) */

    IX_ATMDACC_FULL_STATS(
    printf ("Tx Done queue ............................. : %10u\n",
        IX_NPE_A_QMQ_ATM_TX_DONE);
    printf ("Tx Done Dispatch invoke ................... : %10u\n",
        ixAtmdAccTxDoneDispatchStats.invokeCount);
    printf ("Tx Done Dispatch failed ................... : %10u (should be 0)\n",
        ixAtmdAccTxDoneDispatchStats.failedCount);
    printf ("Tx Done descriptors missing ............... : %10u (should be 0)\n",
        ixAtmdAccTxDoneDispatchStats.descriptorOrderErrorCount);
    printf ("Tx Done Pdu received    ................... : %10u\n",
        ixAtmdAccTxDoneDispatchStats.pduCount);
    printf ("Tx Done TxDisableAck received ............. : %10u\n",
        ixAtmdAccTxDoneDispatchStats.ackCount);
    printf ("Tx Done Max pdu per dispatch .............. : %10u\n",
        ixAtmdAccTxDoneDispatchStats.maxPduPerDispatch);
    printf ("Tx Done Default callback count ............ : %10u (should be 0)\n",
        ixAtmdAccTxDoneDefaultCallbackCount);
    );

    returnStatus = ixQMgrQNumEntriesGet (IX_NPE_A_QMQ_ATM_TX_DONE, &numEntries);
    
    /* read again the number of entries, if needed */
    if (returnStatus == IX_QMGR_WARNING)
    {
        returnStatus = ixQMgrQNumEntriesGet (IX_NPE_A_QMQ_ATM_TX_DONE, &numEntries);
    }
    
    if (returnStatus == IX_QMGR_WARNING)
    {
        printf ("Tx Done Queue level ........................  N/A\n");
    }
    else
    {
        printf ("Tx Done Queue level ....................... : %10u\n", numEntries);
    }
    printf("\n");
}

/* ---------------------------------------------------------
* display the tx configuration
*/
void
ixAtmdAccTxCfgInfoPortShow (IxAtmLogicalPort port)
{
    char *statusString = "?";
    
    switch (ixAtmdAccPortDescriptor[port].status)
    {
    case IX_ATMDACC_PORT_DOWN:
        statusString = "DOWN";
        break;
    case IX_ATMDACC_PORT_UP:
        statusString = "Up";
        break;
    case IX_ATMDACC_PORT_DOWN_PENDING:
        statusString = "DOWN PENDING";
        break;
    } /* end of switch(ixAtmdAccPortDescriptor) */
    
    printf ("Port %2u status %s\n",
        port,
        statusString);
}

/* ---------------------------------------------------------
* display the tx configuration
*/
void
ixAtmdAccTxCfgInfoChannelShow (IxAtmLogicalPort port)
{
    unsigned int vc;

    /* iterate through the VCs for this port */
    for (vc = 0; vc < IX_ATM_MAX_NUM_AAL_OAM_TX_VCS; vc++)
    {
        if (IX_ATMDACC_TX_VC_INUSE (ixAtmdAccTxVcDescriptor[vc].connId))
        {
            if (ixAtmdAccTxVcDescriptor[vc].port == port)
            {
                IX_ATMDACC_TX_QUEUE *txSwQueue = &ixAtmdAccTxVcDescriptor[vc].queue;

                char *stringAalType = "???????";

                switch(ixAtmdAccTxVcDescriptor[vc].npeAalType)
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

                
                /* display data about VC */
                printf ("   Tx Channel %2u %s ConnId 0x%8.8x UserId 0x%8.8x\n",
                    vc,
                    stringAalType,
                    ixAtmdAccTxVcDescriptor[vc].connId,
                    ixAtmdAccTxVcDescriptor[vc].callbackId);
                /* display data about VC */
                printf ("      Vpi %3u Vci %5u PDUsTx %10u PDUsTxd %10u\n",
                    ixAtmdAccTxVcDescriptor[vc].vpi,
                    ixAtmdAccTxVcDescriptor[vc].vci,
                    txSwQueue->mid,
                    txSwQueue->tail);
            }
        }
    }
}
