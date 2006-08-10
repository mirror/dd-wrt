/**
 * @file IxEthAccCommon.c
 *
 * @author Intel Corporation
 * @date 12-Feb-2002
 *
 * @brief This file contains the implementation common support routines for the component
 *
 * Design Notes:
 *
 * @par 
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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
 * Component header files 
 */

#include "IxOsal.h"
#include "IxEthAcc.h"
#include "IxEthDB.h"
#include "IxNpeMh.h"
#include "IxEthDBPortDefs.h"
#include "IxFeatureCtrl.h"
#include "IxEthAcc_p.h"
#include "IxEthAccQueueAssign_p.h"

#include "IxEthAccDataPlane_p.h"
#include "IxEthAccMii_p.h"
#include "IxNpeDl.h"	
#include "IxAccCommon.h"

/**
 * @addtogroup IxEthAccPri
 *@{
 */

extern IxEthAccInfo   ixEthAccDataInfo;



/**
 * @brief Stores Rx Queue info for all rx queues
 */
IX_ETH_ACC_PRIVATE
IxEthAccRxQueue ixEthAccRxQueues[IX_ETHACC_MAX_RX_QUEUES+1];

/* storage for the qmgr register base addresses */
UINT32 ixEthAccQMIntEnableBaseAddress;
UINT32 ixEthAccQMIntStatusBaseAddress;

/**
 *
 * @brief Data structure template for RX Queues
 *
 */
IX_ETH_ACC_PRIVATE
IxEthAccQregInfo ixEthAccQmgrRxTemplate =
{ 
    IX_QMGR_QUEUE_INVALID, 	     /**< Queue ID */
    "Eth Rx Q", 
    ixEthRxFrameQMCallback,          /**< Functional callback */
    (IxQMgrCallbackId) 0,	     /**< Callback tag	      */ 
    IX_QMGR_Q_SIZE128,		     /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,	     /**< Queue Entry Sizes - all Q entries are single word entries   */
    TRUE,			     /**< Enable Q notification at startup */
    IX_QMGR_Q_SOURCE_ID_NOT_E,	     /**< Q Condition to drive callback   */
    IX_QMGR_Q_WM_LEVEL0, 	     /**< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL1,	     /**< Q High water mark - needed by NPE */
};

/**
 *
 * @brief Data structure template for RX Free Queues
 *
 */
IX_ETH_ACC_PRIVATE
IxEthAccQregInfo ixEthAccQmgrRxFreeTemplate =
{
    IX_QMGR_QUEUE_INVALID,
    "Eth Rx Free Q",
    ixEthRxFreeQMCallback,
    (IxQMgrCallbackId) 0,
    IX_QMGR_Q_SIZE128,               /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,           /**< Queue Entry Sizes - all Q entries are single word entries   */
    FALSE,                           /**< Disable Q notification at startup */
    IX_QMGR_Q_SOURCE_ID_E,	     /**< Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0,             /***< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL64,            /**< Q High water mark */
};

/**
 *
 * @brief Data structure template for TX Queues
 *
 */
IX_ETH_ACC_PRIVATE
IxEthAccQregInfo ixEthAccQmgrTxTemplate =
{
     IX_QMGR_QUEUE_INVALID,
    "Eth Tx Q",
     ixEthTxFrameQMCallback,
     (IxQMgrCallbackId) 0,
    IX_QMGR_Q_SIZE128,               /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,           /**< Queue Entry Sizes - all Q entries are single word entries   */
    FALSE,                           /**< Disable Q notification at startup */
    IX_QMGR_Q_SOURCE_ID_E,	     /**< Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0,             /**< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL64,            /**< Q High water mark */
};

/**
 *
 * @brief Data structure template for TxDone Queue
 *
 */
IX_ETH_ACC_PRIVATE
IxEthAccQregInfo ixEthAccQmgrTxDoneTemplate =
{
     IX_ETH_ACC_TX_DONE_Q,
    "Eth Tx Done Q",
     ixEthTxFrameDoneQMCallback,
     (IxQMgrCallbackId) 0,
    IX_QMGR_Q_SIZE128,               /**< Allocate Max Size Q */
    IX_QMGR_Q_ENTRY_SIZE1,           /**< Queue Entry Sizes - all Q entries are single word entries   */
    TRUE,                            /**< Enable Q notification at startup */
    IX_QMGR_Q_SOURCE_ID_NOT_E,	     /**< Q Condition to drive callback  */
    IX_QMGR_Q_WM_LEVEL0,             /**< Q Low water mark */
    IX_QMGR_Q_WM_LEVEL2,             /**< Q High water mark - needed by NPE */
};

/**
 * @brief RX Queue size table indexed by relative queue priority
 */
IX_ETH_ACC_PRIVATE const
IxQMgrQSizeInWords ixEthAccRxQueueSizeTable[IX_ETHACC_MAX_RX_QUEUES] =
{
    IX_QMGR_Q_SIZE128, /* highest priority */
    IX_QMGR_Q_SIZE64,  /* second highest */
    IX_QMGR_Q_SIZE32,  /* third highest */
    IX_QMGR_Q_SIZE16,  /* fourth highest */
    IX_QMGR_Q_SIZE16,  /* fifth highest */
    IX_QMGR_Q_SIZE16,  /* sixth highest */
    IX_QMGR_Q_SIZE16,  /* seventh highest */
    IX_QMGR_Q_SIZE16   /* lowest */
};

/**
 * @brief Queue nearly full watermark table indexed by NPE count
 */
IX_ETH_ACC_PRIVATE const
IxQMgrWMLevel ixEthAccQueueNFWatermarkTable[] =
{
    IX_QMGR_Q_WM_LEVEL0,  /* NPE count == 0 (invalid) */ 
    IX_QMGR_Q_WM_LEVEL0,  /* NPE count == 1 */
    IX_QMGR_Q_WM_LEVEL1,  /* NPE count == 2 */
    IX_QMGR_Q_WM_LEVEL2,  /* NPE count == 3 */
    IX_QMGR_Q_WM_LEVEL4,  /* NPE count == 4 */
    IX_QMGR_Q_WM_LEVEL4,  /* NPE count == 5 */
    IX_QMGR_Q_WM_LEVEL8,  /* NPE count == 6 */
    IX_QMGR_Q_WM_LEVEL8   /* NPE count == 7 */
};

/**
 * @brief Static Queue assignment array per NPEs indexed by NPE ID
 */
IX_ETH_ACC_PRIVATE const
UINT8 ixEthAccNpeStaticQueueConfigs[][2] =
{
    /* { TX Queue              , RX Free Queue } */
    { IX_ETH_ACC_TX_NPEA_Q, IX_ETH_ACC_RX_FREE_NPEA_Q},
    { IX_ETH_ACC_TX_NPEB_Q, IX_ETH_ACC_RX_FREE_NPEB_Q},
    { IX_ETH_ACC_TX_NPEC_Q, IX_ETH_ACC_RX_FREE_NPEC_Q}
};

/*
 * Forward declaration of private functions
 */
IX_ETH_ACC_PRIVATE 
IxEthAccStatus ixEthAccQMgrQueueSetup(IxEthAccQregInfo *qInfoDes);
PRIVATE IxEthAccStatus
ixEthAccQMgrQueueUnsetup (IxEthAccQregInfo *qInfoDes);
IX_ETH_ACC_PRIVATE
IxEthAccStatus ixEthAccGetRxQueueList(IxEthAccPortId portId,
				      IxEthAccRxQueue *rxQueues);


/**
 * @fn ixEthAccQMgrQueueSetup(IxEthAccQregInfo*)
 *
 * @brief Setup one queue and its event, and register the callback required 
 * by this component to the QMgr
 *
 * @internal
 */
IX_ETH_ACC_PRIVATE IxEthAccStatus 
ixEthAccQMgrQueueSetup(IxEthAccQregInfo *qInfoDes)
{
    int ret;

    ret = ixQMgrQConfig( qInfoDes->qName,
			qInfoDes->qId,
			qInfoDes->qSize,
			qInfoDes->qWords);

    /* If queue is already configured, continue anyway */
    if(ret != IX_SUCCESS && ret != IX_QMGR_Q_ALREADY_CONFIGURED)
    {
	return IX_ETH_ACC_FAIL;
    }
	
    if ( ixQMgrWatermarkSet( qInfoDes->qId,
			     qInfoDes->AlmostEmptyThreshold,
			     qInfoDes->AlmostFullThreshold
			     ) != IX_SUCCESS)
    {
	return IX_ETH_ACC_FAIL;
    }

    /* 
     * Set dispatcher priority. 
     */
    if ( ixQMgrDispatcherPrioritySet( qInfoDes->qId, 
				      IX_ETH_ACC_QM_QUEUE_DISPATCH_PRIORITY) 
	 != IX_SUCCESS)
    {
	return IX_ETH_ACC_FAIL;
    }
	
    /*
     * Register callbacks for each Q.
     */ 
    if ( ixQMgrNotificationCallbackSet(qInfoDes->qId,
				       qInfoDes->qCallback,
				       qInfoDes->callbackTag) 
	 != IX_SUCCESS )
    {
	return IX_ETH_ACC_FAIL;
    }

    /*
     * Set notification condition for Q
     */  
    if ( qInfoDes->qNotificationEnableAtStartup == TRUE ) 
    {
	if (   ixQMgrNotificationEnable(qInfoDes->qId,
					qInfoDes->qConditionSource)
	       != IX_SUCCESS )
	{
	    return IX_ETH_ACC_FAIL;
	}
    }

    return(IX_ETH_ACC_SUCCESS);
}

/*
 * ixEthAccQMgrQueueUnsetup ()
 * added to the same file
 */

PRIVATE IxEthAccStatus
ixEthAccQMgrQueueUnsetup (IxEthAccQregInfo *qInfoDes)
{
    if (IX_ETH_ACC_SUCCESS != ixQMgrNotificationDisable(qInfoDes->qId))
    {
        IX_ETH_ACC_WARNING_LOG ("Failed to disable the notification for QId=%u\n", 
				(UINT32) qInfoDes->qId, 0, 0, 0, 0, 0);
        return IX_ETH_ACC_FAIL;
    }
    return IX_ETH_ACC_SUCCESS;
}

/**
 * @fn ixEthAccGetRxQueueList(IxEthAccPortId, IxEthAccRxQueue *)
 *
 * @brief Fill in and sort the rx queue array 
 *
 * @li select all Rx queues as configured by ethDB for all ports
 * @li sort the queues by traffic class
 *
 * @param none
 *
 * @return UINT32 (num Rx queues)
 *
 * @internal
 */
IX_ETH_ACC_PRIVATE
IxEthAccStatus ixEthAccGetRxQueueList(IxEthAccPortId portId, IxEthAccRxQueue *rxQueues)
{
    IxEthDBStatus ixEthDBStatus = IX_ETH_DB_SUCCESS;
    IxEthDBProperty ixEthDBTrafficClass = IX_ETH_DB_QOS_TRAFFIC_CLASS_0_RX_QUEUE_PROPERTY;
    IxEthDBPropertyType ixEthDBPropertyType = IX_ETH_DB_INTEGER_PROPERTY;
    UINT32 ixEthDBParameter = 0;
    BOOL completelySorted = FALSE;
    UINT32 rxQueue = 0;
    UINT32 sortIterations = 0;
    IxEthNpeNodeId ixNpeId = IX_ETHNPE_PHYSICAL_ID_TO_NODE(portId);

    /* 
     * Queue Selection step:
     *
     * The following code selects all the queues and fills
     * the RxQueue array
     *
     */

    /* Iterate thru the different priorities */
    for (ixEthDBTrafficClass = IX_ETH_DB_QOS_TRAFFIC_CLASS_0_RX_QUEUE_PROPERTY; 
	 ixEthDBTrafficClass <= IX_ETH_DB_QOS_TRAFFIC_CLASS_7_RX_QUEUE_PROPERTY; 
	 ixEthDBTrafficClass++)
    {
	ixEthDBStatus = ixEthDBFeaturePropertyGet(
				portId,
				IX_ETH_DB_VLAN_QOS,
				ixEthDBTrafficClass,
				&ixEthDBPropertyType,
				(void *)&ixEthDBParameter);

	if (ixEthDBStatus == IX_ETH_DB_SUCCESS)
	{
	    /* This port and QoS class are mapped to 
	     * a RX queue.
	     */
	    if (ixEthDBPropertyType == IX_ETH_DB_INTEGER_PROPERTY)
	    {
		/* search the queue in the list of queues
		 * already used by an other port or QoS
		 */
		for (	rxQueue = 0;
			((rxQueues[rxQueue].npeCount != 0)
			&& (rxQueue < IX_ETHACC_MAX_RX_QUEUES));
			rxQueue++)
		{
		    if (rxQueues[rxQueue].qId == (IxQMgrQId)ixEthDBParameter)
		    {
			/* found an existing setup, update the number of ports 
			 * for this queue if the port maps to
			 * a different NPE.
			 */
			if (rxQueues[rxQueue].npeId != ixNpeId)
			{
			    rxQueues[rxQueue].npeCount++;
			    rxQueues[rxQueue].npeId = ixNpeId;
			}
			/* get the highest traffic class for this queue */
			if (rxQueues[rxQueue].trafficClass > ixEthDBTrafficClass)
			{
			    rxQueues[rxQueue].trafficClass = ixEthDBTrafficClass;
			}
			break;
		    }
		}
		if (rxQueues[rxQueue].npeCount == 0)
		{
		    /* new queue not found in the current list, 
		     * add a new entry.
		     */
		    IX_OSAL_ASSERT(rxQueue < IX_ETHACC_MAX_RX_QUEUES);
		    rxQueues[rxQueue].qId = ixEthDBParameter;
		    rxQueues[rxQueue].npeCount = 1;
		    rxQueues[rxQueue].npeId = ixNpeId;
		    rxQueues[rxQueue].trafficClass = ixEthDBTrafficClass;
		}
	    }
	    else
	    {
		/* unexpected property type (not Integer) */
                IX_ETH_ACC_WARNING_LOG("ixEthAccGetRxQueueList: In Port=%u, unexpected property type returned by EthDB\n", (UINT32) portId, 0, 0, 0, 0, 0);
		/* no point to continue to iterate */
		return (IX_ETH_ACC_FAIL);
	    }
	}
	else
	{
	    /* No Rx queue configured for this port
	     * and this traffic class. Do nothing.
	     */
	}
    }

    /* check there is at least 1 rx queue : there is no point
     * to continue if there is no rx queue configured 
     */
    if (rxQueues[0].npeCount == 0)
    {
        IX_ETH_ACC_WARNING_LOG("ixEthAccGetRxQueueList: In Port=%u, no queues configured, bailing out\n", 
			       (UINT32) portId, 0, 0, 0, 0, 0);
	/* queue configuration error so return queue count 0 */
	return (IX_ETH_ACC_FAIL);
    }

    /* Queue sort step:
     *
     * Re-order the array of queues by decreasing traffic class
     * using a bubble sort. (trafficClass 0 is the lowest 
     * priority traffic, trafficClass 7 is the highest priority traffic)
     *
     * Primary sort order is traffic class
     * Secondary sort order is npeId
     *
     * Note that a bubble sort algorithm is not very efficient when
     * the number of queues grows . However, this is not a very bad choice 
     * considering the very small number of entries to sort. Also, bubble
     * sort is extremely fast when the list is already sorted.
     *
     * The output of this loop is a sorted array of queues.
     *
     */
    sortIterations = 0;
    do
    {
	sortIterations++;
	completelySorted = TRUE;
	for (rxQueue = 0;   (rxQueues[rxQueue+1].npeCount != 0)
	     && (rxQueue < IX_ETHACC_MAX_RX_QUEUES - sortIterations); 
	     rxQueue++)
	{
	    /* compare adjacent elements */
	    if ((rxQueues[rxQueue].trafficClass <
		rxQueues[rxQueue+1].trafficClass)
		|| ((rxQueues[rxQueue].trafficClass ==
		     rxQueues[rxQueue+1].trafficClass)
		    &&(rxQueues[rxQueue].npeId <
		       rxQueues[rxQueue+1].npeId)))
	    {
		/* swap adjacent elements */
		int npeCount = rxQueues[rxQueue].npeCount;
		UINT32 npeId = rxQueues[rxQueue].npeId;
		IxQMgrQId qId = rxQueues[rxQueue].qId;
		IxEthDBProperty trafficClass = rxQueues[rxQueue].trafficClass;
		rxQueues[rxQueue].npeCount = rxQueues[rxQueue+1].npeCount;
		rxQueues[rxQueue].npeId = rxQueues[rxQueue+1].npeId;
		rxQueues[rxQueue].qId = rxQueues[rxQueue+1].qId;
		rxQueues[rxQueue].trafficClass = rxQueues[rxQueue+1].trafficClass;
		rxQueues[rxQueue+1].npeCount = npeCount;
		rxQueues[rxQueue+1].npeId = npeId;
		rxQueues[rxQueue+1].qId = qId;
		rxQueues[rxQueue+1].trafficClass = trafficClass;
		completelySorted = FALSE;
	    }
	}
    }
    while (!completelySorted);

    return (IX_ETH_ACC_SUCCESS);
}

/**
 * @fn ixEthAccQMgrQueuesConfig(void)
 *
 * @brief Setup all the queues and register all callbacks required 
 * by this component to the QMgr
 *
 * Rx queues configuration is driven by QoS setup. 
 * Many Rx queues may be required when QoS is enabled (this depends
 * on IxEthDB setup and the images being downloaded). The configuration 
 * of the rxQueues is done in many steps as follows:
 *
 * @li select all Rx queues as configured by ethDB for all ports
 * @li sort the queues by traffic class
 * @li build the priority dependency for all queues
 * @li fill the configuration for all rx queues
 * @li configure all statically configured queues
 * @li configure all dynamically configured queues
 *
 * @param none
 *
 * @return IxEthAccStatus
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccQMgrQueuesConfig(IxEthAccPortId portId)
{
    IxEthAccQregInfo qInfoDes;
    IxEthAccQregInfo rxFreeQInfoDes;
    IxEthAccQregInfo txQInfoDes;
    UINT32 rxQueue = 0;
    /* Rx queue list stored globally */
    IxEthAccRxQueue *rxQueues = ixEthAccRxQueues;
    IxEthAccStatus ret = IX_ETH_ACC_SUCCESS;
    IxEthNpeNodeId ixNpeId = IX_ETHNPE_PHYSICAL_ID_TO_NODE(portId);


    /* Configure the TxDone Queue
     * Note that the TxDone queue is shared by all ports
     * ixEthAccQMgrQueueSetup makes sure it is not configured again
     */ 
    memcpy(&qInfoDes, &ixEthAccQmgrTxDoneTemplate, sizeof(qInfoDes));
    /* setup the TxDone Queue watermark level from the static table */
    qInfoDes.AlmostFullThreshold = ixEthAccQueueNFWatermarkTable[ixEthAccDataInfo.npeCount];
    ret = ixEthAccQMgrQueueSetup(&qInfoDes);
 
	if(ret) return (ret);

    /* Configure the TX and RxFree Queues for this port
     */ 
    memcpy(&rxFreeQInfoDes, &ixEthAccQmgrRxFreeTemplate, sizeof(rxFreeQInfoDes));
    ixEthAccPortData[portId].ixEthAccRxData.rxFreeQueueSource \
        = rxFreeQInfoDes.qConditionSource;
    rxFreeQInfoDes.callbackTag = portId;

    memcpy(&txQInfoDes, &ixEthAccQmgrTxTemplate, sizeof(txQInfoDes));
    ixEthAccPortData[portId].ixEthAccTxData.txQueueSource \
        = txQInfoDes.qConditionSource;
    txQInfoDes.callbackTag = portId;

    ixEthAccPortData[portId].ixEthAccRxData.rxFreeQueue \
        = rxFreeQInfoDes.qId \
        = ixEthAccNpeStaticQueueConfigs[ixNpeId][IX_ETHACC_RXFREE_QUEUE_ASSIGNMENT_INDEX];
    ixEthAccPortData[portId].ixEthAccTxData.txQueue \
        = txQInfoDes.qId \
        = ixEthAccNpeStaticQueueConfigs[ixNpeId][IX_ETHACC_TX_QUEUE_ASSIGNMENT_INDEX];

    ret = ixEthAccQMgrQueueSetup(&rxFreeQInfoDes);

    if(ret) return (ret);
    
	ret = ixEthAccQMgrQueueSetup(&txQInfoDes);

    if(ret) return (ret);
            

    /* Get RX queue list */
    ret = ixEthAccGetRxQueueList(portId, rxQueues);    

    if(ret) return (ret);

    /* get RX queue template for the queues */
    memcpy(&qInfoDes, &ixEthAccQmgrRxTemplate, sizeof(qInfoDes));

    /* go through all the queues configuring with proper parameters:
     * - queue ID (from rxQueues list)
     * - queue size (larger size for higher priority queues)
     * - Almost full watermark (based on npe count for queue)
     */
    for (rxQueue = 0;
	 (rxQueues[rxQueue].npeCount != 0) && (ret == IX_ETH_ACC_SUCCESS);
	 rxQueue++)
    {
	/* copy the local priority qId into dataplane struct and add to bitmask */
	ixEthAccDataInfo.rxQueues[rxQueue] = rxQueues[rxQueue].qId;
	ixEthAccDataInfo.rxQueuesIntMask |= (1 << (rxQueues[rxQueue].qId));

	/* setup the Rx Queue ID */
	qInfoDes.qId = rxQueues[rxQueue].qId;
	
	/* setup the Rx Queue size from static table */
	qInfoDes.qSize = ixEthAccRxQueueSizeTable[rxQueue];
	
	/* setup the Rx Queue watermark level from the static table */
	qInfoDes.AlmostFullThreshold = ixEthAccQueueNFWatermarkTable[rxQueues[rxQueue].npeCount];

	/* Set the callback tag in Rx queue storage
         * Used for callback registration */
	rxQueues[rxQueue].callbackTag = qInfoDes.callbackTag;

	/* configure this queue */
      ret = ixEthAccQMgrQueueSetup(&qInfoDes);
	

    }
    /* set invalid last entry in dataplane struct */
    ixEthAccDataInfo.rxQueues[rxQueue] = IX_QMGR_MAX_NUM_QUEUES;

    /* notify EthDB that queue initialization is complete and traffic class allocation is frozen */
    ixEthDBFeaturePropertySet(portId,
        IX_ETH_DB_VLAN_QOS,
        IX_ETH_DB_QOS_QUEUE_CONFIGURATION_COMPLETE,
        NULL /* ignored */);

    return (ret);				 
}

/* @ixEthUnconfigQueues
 *
 * @params none
 *
 * @fn - unconfigures the queues configured in ixEthAccInit
 *
 * @return IxEthAccStatus
 */

IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccQueuesUnconfig (void)
{
    IxEthAccStatus ret = IX_ETH_ACC_SUCCESS;
    IxEthAccQregInfo qInfoDes;
    IxEthAccPortId portId;
    UINT32 rxQueue = 0;
    /* Rx queue list stored globally */
    IxEthAccRxQueue *rxQueues = ixEthAccRxQueues;

    /* Get RX queue list */
    ret = ixEthAccGetRxQueueList(0, rxQueues);    
    if(ret) return (ret);

    /* un configure the Rx queues */
    for (rxQueue = 0;
	 (rxQueues[rxQueue].npeCount != 0) && (ret == IX_ETH_ACC_SUCCESS);
	 rxQueue++)
    {
	memset(&qInfoDes, 0, sizeof(IxEthAccQregInfo));
	qInfoDes.qId = rxQueues[rxQueue].qId;
	ret = ixEthAccQMgrQueueUnsetup (&qInfoDes);
    }

    /* clear the rxQueues data structure */
    memset(rxQueues, 0, sizeof(ixEthAccRxQueues));

    /* un configure the TxDone queue */
    ret = ixEthAccQMgrQueueUnsetup(&ixEthAccQmgrTxDoneTemplate);

    /* un configure the RxFree and Tx queues */
    for (portId = 0;
         portId < IX_ETH_ACC_NUM_PORTS;
        ++portId)
    {
        ixQMgrNotificationDisable(ixEthAccPortData[portId].ixEthAccTxData.txQueue);
        ixQMgrNotificationDisable(ixEthAccPortData[portId].ixEthAccRxData.rxFreeQueue);
    }

    return ret;
}



/**
 * @fn ixEthAccQMgrRxQEntryGet(UINT32 *rxQueueEntries)
 *
 * @brief Add and return the total number of entries in all Rx queues
 *
 * @param UINT32 rxQueueEntries[in] number of entries in all queues
 *
 * @return void
 *
 * @note Rx queues configuration is driven by Qos Setup. There is a 
 * variable number of rx queues which are set at initialisation. 
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC
void ixEthAccQMgrRxQEntryGet(UINT32 *numRxQueueEntries)
{
    UINT32 rxQueueLevel;
    IxEthAccRxQueue *rxQueuePtr;

    *numRxQueueEntries = 0;

    /* iterate thru rx queues */
    for (rxQueuePtr = ixEthAccRxQueues;
	 rxQueuePtr->npeCount != 0;
	 ++rxQueuePtr)
    {
	/* retrieve the rx queue level */
	rxQueueLevel = 0;
	ixQMgrQNumEntriesGet(rxQueuePtr->qId, &rxQueueLevel);
	(*numRxQueueEntries) += rxQueueLevel;
    }
}

/**
 * @fn ixEthAccQMgrRxNotificationEnable(void)
 *
 * @brief Enable AQM notification for all rx queues.
 *
 * @return IxEthAccStatus
 *
 */
IX_ETH_ACC_PUBLIC
void ixEthAccQMgrRxNotificationEnable()
{
    UINT32 intEnableReg;

    intEnableReg = IX_OSAL_READ_LONG(ixEthAccQMIntEnableBaseAddress);
    IX_OSAL_WRITE_LONG(ixEthAccQMIntEnableBaseAddress,
        (intEnableReg | ixEthAccDataInfo.rxQueuesIntMask)); 
}

/**
 * @fn ixEthAccQMgrRxNotificationDisable(void)
 *
 * @brief Disable AQM notification for all rx queues.
 *
 * @return IxEthAccStatus
 *
 */
IX_ETH_ACC_PUBLIC
void ixEthAccQMgrRxNotificationDisable()
{
    UINT32 intEnableReg;

    intEnableReg = IX_OSAL_READ_LONG(ixEthAccQMIntEnableBaseAddress);
    IX_OSAL_WRITE_LONG(ixEthAccQMIntEnableBaseAddress,
        (intEnableReg & ~(ixEthAccDataInfo.rxQueuesIntMask)));
    IX_OSAL_WRITE_LONG(ixEthAccQMIntStatusBaseAddress,
        ixEthAccDataInfo.rxQueuesIntMask); 
}

/**
 * @fn ixEthAccQMgrRxCallbacksRegister
 *
 * @brief Change the callback registered to all rx queues.
 *
 * @param IxQMgrCallback ixQMgrCallback[in] QMgr callback to register
 * @param IxQMgrCallbackId ixQMgrCallbackTag[in] callback tag to register
 *
 * @return IxEthAccStatus
 *
 * @note The user may decide to use different Rx mechanisms
 * (e.g. receive many frames at the same time , or receive
 *  one frame at a time, depending on the overall application
 *  performances). A different QMgr callback is registered. This
 *  way, there is no excessive pointer checks in the datapath.
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccQMgrRxCallbacksRegister(IxQMgrCallback ixQMgrCallback,
                                               IxQMgrCallbackId ixQMgrCallbackTag)
{
    IxEthAccRxQueue *rxQueuePtr;
    IxEthAccStatus ret = IX_ETH_ACC_SUCCESS;
    IxQMgrCallbackId callbackTag = ixQMgrCallbackTag;
    
    /* parameter check */
    if (NULL == ixQMgrCallback)
    {
	ret = IX_ETH_ACC_FAIL;
    }
    
    /* iterate thru rx queues */
    for (rxQueuePtr = ixEthAccRxQueues;
	 (rxQueuePtr->npeCount != 0)
	     && (ret == IX_ETH_ACC_SUCCESS);
	 ++rxQueuePtr)
    {
        /* if no tag given, use stored tag */
        if(ixQMgrCallbackTag == 0)
            callbackTag = rxQueuePtr->callbackTag;
	/* register the rx callback for all queues */
	ret = ixQMgrNotificationCallbackSet(rxQueuePtr->qId,
					     ixQMgrCallback,
					     callbackTag);
    }
    return(ret);
}

/**
 * @fn ixEthAccSingleEthNpeCheck(IxEthAccPortId portId)
 *
 * @brief Check the hardware and microcode exists for this port
 *
 * @param IxEthAccPortId portId[in] port
 *
 * @return IxEthAccStatus
 *
 * @note The following conditions must all be true to pass:
 *  - The port's associated NPE exists
 *  - An ethernet NPE image is loaded
 *  - For ports 1-3 on NPEB, the correct 4port image is loaded
 *    and the (1-3 ports) are enabled in hardware
 *  - The Ethernet coprocessor exists for the port
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccSingleEthNpeCheck(IxEthAccPortId physicalId)
{
    UINT8 functionalityId;

    IxEthNpeNodeId npeId = IX_ETHNPE_PHYSICAL_ID_TO_NODE(physicalId);
    IxEthNpePortId portId = IX_ETHNPE_PHYSICAL_ID_TO_PORT(physicalId);
    
    if (IX_SUCCESS != ixNpeDlLoadedImageFunctionalityGet(npeId, &functionalityId))
    {
        return IX_ETH_ACC_FAIL;
    }
    else
    {
        /* If not IXP42X A0 stepping, proceed to check for existence of NPEs and ethernet coprocessors */ 
        if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 != 
            (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
            || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
        {
            switch(npeId)
            {
              case IX_NPEDL_NPEID_NPEA: 
                if ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA) ==
                     IX_FEATURE_CTRL_COMPONENT_DISABLED) ||
                     ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA_ETH) ==
                     IX_FEATURE_CTRL_COMPONENT_DISABLED) || 
                     (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
                     IX_FEATURE_CTRL_COMPONENT_DISABLED)))
                {
                    return IX_ETH_ACC_FAIL;
                }
                break;

              case IX_NPEDL_NPEID_NPEB: 
                if ( (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB) ==
                     IX_FEATURE_CTRL_COMPONENT_DISABLED) )
                {
                    return IX_ETH_ACC_FAIL;
                }
                if (portId == 0)
                {
                    if( ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
                        IX_FEATURE_CTRL_COMPONENT_DISABLED)
                    {
                        return IX_ETH_ACC_FAIL;	    
                    }
                }
                else /* ports 1-3 */
                {
                    if( ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB_ETH) ==
                        IX_FEATURE_CTRL_COMPONENT_DISABLED)
                    {
                        return IX_ETH_ACC_FAIL;	    
                    }

                }
                break;

              case IX_NPEDL_NPEID_NPEC: 
                if ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC) ==
                     IX_FEATURE_CTRL_COMPONENT_DISABLED) ||
                     ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) ==
                     IX_FEATURE_CTRL_COMPONENT_DISABLED) ||
                     (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
                     IX_FEATURE_CTRL_COMPONENT_DISABLED)))
                {
                    return IX_ETH_ACC_FAIL;
                }
                break;

              default: /* invalid NPE */
                return IX_ETH_ACC_FAIL;
            }
        }
        return IX_ETH_ACC_SUCCESS;
    }
}

/**
 * @fn ixEthAccStatsShow(void)
 *
 * @brief Displays all EthAcc stats
 *
 * @return void
 *
 */
void ixEthAccStatsShow(IxEthAccPortId portId)
{
    ixEthAccMdioShow();

    printf("\nPort %u\nUnicast MAC : ", portId);
    ixEthAccPortUnicastAddressShow(portId);
    ixEthAccPortMulticastAddressShow(portId);
    printf("\n");

    ixEthAccDataPlaneShow();
}

/**
 * This function is responsible to activate Flag Bus status from AQM to
 * NPE. Only Rx Queue, RxFree and TxDone Queue are involved.
 * Note: user is required to disable IRQ before calling this function.
 */
IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccQMStatusUpdate(IxEthAccPortId portId)
{  
   UINT32 rxFreeId = 0; 
   UINT32 qEntry = 0;
   UINT32 rxQueue = 0;
   IxEthAccRxQueue *rxQueues = ixEthAccRxQueues;
   IxEthAccStatus ret = IX_ETH_ACC_SUCCESS;

   if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
   {
       return (IX_ETH_ACC_FAIL);
   }

    IX_ETH_ACC_VALIDATE_PORT_ID(portId);

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("EthAcc: Eth %d: Cannot reupdate QM Status.\n",(INT32) portId,0,0,0,0,0);
        return IX_ETH_ACC_FAIL ;
    } 

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
 
   /* 
    * (1) Read an entry from TxDone Queue.
    * (2) If there is any entry, write the entry back to TxDone Queue. 
    */
    if (ixQMgrQRead(IX_ETH_ACC_TX_DONE_Q, &qEntry) == IX_SUCCESS)
    { 
      /* Avoid qEntry=0 (NULL) to be written to HwQ. */
      if(qEntry > 0)
      {
       if (ixQMgrQWrite(IX_ETH_ACC_TX_DONE_Q, &qEntry) != IX_SUCCESS)
       {
         return (IX_ETH_ACC_FAIL);        
       } 
      }
    }

    /* 
    * (1) Read an entry from RxFree Queue, then
    * (2) If thre is any entry, write the entry back to RxFree Queue. 
    */ 
    rxFreeId = IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId);
    
    if (ixQMgrQRead(rxFreeId, &qEntry) == IX_SUCCESS)
    {
       /* Avoid qEntry=0 (NULL) to be written to HwQ. */
       if(qEntry > 0)
       {
        if (ixQMgrQWrite(rxFreeId, &qEntry) != IX_SUCCESS)
        {
          return (IX_ETH_ACC_FAIL);        
        } 
       }
    }
   
   /* 
    * (1) Read an entry from Rx Queue.
    * (2) If there is any entry, write the entry back to Rx Queue. 
    *  Note: For QoS enabled image, there are more than 1 RxQ.
    */
    /* Get RX queue list */
    ret = ixEthAccGetRxQueueList(portId, rxQueues);    

    if(ret) return (ret);

    for (rxQueue = 0;
	 (rxQueues[rxQueue].npeCount != 0);
	 rxQueue++)
    {
      if (ixQMgrQRead(rxQueues[rxQueue].qId, &qEntry) == IX_SUCCESS)
      {
        /* Avoid qEntry=0 (NULL) to be written to HwQ. */
        if(qEntry > 0)
        {
	 if (ixQMgrQWrite(rxQueues[rxQueue].qId, &qEntry) != IX_SUCCESS)
         {
           return (IX_ETH_ACC_FAIL);        
         }
        }
      }	       
    }

   return (IX_ETH_ACC_SUCCESS);
}

/**
 * @fn ixEthHssAccCoExistInit(void)
 *
 * @brief Check ethernet&hss co-existence services initialized and init mutex
 *
 * @return IxEthAccStatus
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthHssAccCoExistInit(void)
{
    UINT8 functionalityId;
    IX_STATUS retStatus;

    retStatus = ixNpeDlLoadedImageFunctionalityGet(IX_NPEDL_NPEID_NPEA, &functionalityId);

    /* Check for parameter error */
    if (IX_NPEDL_PARAM_ERR ==  retStatus)
    {
        return IX_ETH_ACC_FAIL;
    }    

    /* Check if NPE image is downloaded to NPE A. If NPE image is not downloaded
     * in NPE A, ixNpeDlLoadedImageFunctionalityGet returns IX_FAIL.If this is the case,
     * we can return IX_ETH_ACC_SUCCESS to not interrupt the following
     * steps in ixEthAccInit()  
     */
    if (IX_FAIL == retStatus)
    {
        return IX_ETH_ACC_SUCCESS;
    }

    /*
     * To enable Ethernet & HSS co-existence feature in NPE A, check the functionality 
     * of downloaded NPE image when the functionality id is either 0x00900000
     * (HSS channelized + learning/filtering support) or 0x00910000
     * (HSS channelized + header conversion support)
     */
    if ((functionalityId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(
         IX_NPEDL_NPEIMAGE_NPEA_ETH_MACFILTERLEARN_HSSCHAN_COEXIST) ||
         functionalityId == IX_FUNCTIONID_FROM_NPEIMAGEID_GET(
         IX_NPEDL_NPEIMAGE_NPEA_ETH_HDRCONV_HSSCHAN_COEXIST)) &&
         ixEthHssAccCoexistEnable != TRUE)
    {
    	/* If service is initialized and the common mutex is not initialized, initialize it */
    	if (TRUE != ixEthHssComMutexInitDone)
    	{
           if (ixOsalMutexInit(&ixEthHssCoexistLock) != IX_SUCCESS)
           {
              IX_ETH_ACC_WARNING_LOG("ixEthHssAccCoExistInit: Common co-exist mutex init failed\n", 0, 0, 0, 0, 0, 0);
              return IX_ETH_ACC_FAIL;
           }

           ixEthHssComMutexInitDone = TRUE;
    	}

        ixEthHssAccCoexistEnable = TRUE;
    }

    return IX_ETH_ACC_SUCCESS;
}

/**
 * @fn ixEthHssAccCoExistUninit(void)
 *
 * @brief Check ethernet&hss co-existence services uninitialized
 *
 * @return IxEthAccStatus
 *
 * @internal
 */
IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthHssAccCoExistUninit(void)
{
    /*
     * Destroy common mutex initialized for Eth+HSS co-existence services.
     * Check whether already destroyed in the HSS component
     */

    if (FALSE != ixEthHssAccCoexistEnable)
    {
       if (FALSE != ixEthHssComMutexInitDone)
       {
          if (ixOsalMutexDestroy (&ixEthHssCoexistLock) != IX_SUCCESS)
          {
              IX_ETH_ACC_WARNING_LOG("ixEthHssAccCoExistUninit: Common co-exist mutex destroy failed\n", 0, 0, 0, 0, 0, 0);

              return IX_ETH_ACC_FAIL;
          }

          ixEthHssComMutexInitDone = FALSE;
       }

       ixEthHssAccCoexistEnable = FALSE;
    }

    return IX_ETH_ACC_SUCCESS;
}

