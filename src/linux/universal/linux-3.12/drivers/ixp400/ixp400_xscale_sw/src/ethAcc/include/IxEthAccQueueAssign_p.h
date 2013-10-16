/**
 * @file IxEthAccQueueAssign_p.h
 *
 * @author Intel Corporation
 * @date 06-Mar-2002
 *
 * @brief   Mapping from QMgr Q's to internal assignment
 *
 * Design Notes:
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

/**
 * @addtogroup IxEthAccPri
 *@{
 */

/*
 * Os/System dependancies.
 */
#include "IxOsal.h"

/*
 * Intermodule dependancies
 */
#include "IxQMgr.h"
#include "IxQueueAssignments.h"

/* Check range of Q's assigned to this component. */
#if IX_ETH_ACC_RX_FRAME_ETH_Q >= (IX_QMGR_MIN_QUEUPP_QID ) |    	\
 IX_ETH_ACC_RX_FREE_NPEA_Q  >=  (IX_QMGR_MIN_QUEUPP_QID) | 	\
 IX_ETH_ACC_RX_FREE_NPEB_Q  >=  (IX_QMGR_MIN_QUEUPP_QID) | 	\
 IX_ETH_ACC_RX_FREE_NPEC_Q  >=  (IX_QMGR_MIN_QUEUPP_QID) | 	\
 IX_ETH_ACC_TX_NPEA_Q >=   (IX_QMGR_MIN_QUEUPP_QID) |  		\
 IX_ETH_ACC_TX_NPEB_Q >=   (IX_QMGR_MIN_QUEUPP_QID) | 		\
 IX_ETH_ACC_TX_NPEC_Q >=   (IX_QMGR_MIN_QUEUPP_QID) | 		\
 IX_ETH_ACC_TX_DONE_Q  >=  (IX_QMGR_MIN_QUEUPP_QID)  
#error "Not all Ethernet Access Queues are betweem 1-31, requires full functionalty Q's unless otherwise validated "
#endif

/**
*
* @typedef  IxEthAccQregInfo
*
* @brief QMgr registration info structure for queues
*
*/
typedef struct 
{
   IxQMgrQId qId;
   char *qName;
   IxQMgrCallback qCallback;
   IxQMgrCallbackId callbackTag;
   IxQMgrQSizeInWords qSize;
   IxQMgrQEntrySizeInWords qWords; 
   BOOL           qNotificationEnableAtStartup;
   IxQMgrSourceId qConditionSource; 
   IxQMgrWMLevel  AlmostEmptyThreshold;
   IxQMgrWMLevel  AlmostFullThreshold;

} IxEthAccQregInfo;

/**
*
* @typedef  IxEthAccRxQueue
*
* @brief Rx Queue structure
*
*/
typedef struct
{
   int npeCount;
   UINT32 npeId;
   IxQMgrQId qId;
   IxEthDBProperty trafficClass;
   IxQMgrCallbackId callbackTag;
} IxEthAccRxQueue;

#define IX_ETH_ACC_QM_QUEUE_DISPATCH_PRIORITY (IX_QMGR_Q_PRIORITY_0) /* Highest priority */

/**
 *
 * @brief Maximum number of RX queues set to be the maximum number
 * of traffic classes.
 *
 */
#define IX_ETHACC_MAX_RX_QUEUES \
      (IX_ETH_DB_QOS_TRAFFIC_CLASS_7_RX_QUEUE_PROPERTY \
      - IX_ETH_DB_QOS_TRAFFIC_CLASS_0_RX_QUEUE_PROPERTY \
      + 1)

/**
 *
 * @brief Maximum number of RxFree Queues (1 per port)
 *
 */
#define IX_ETHACC_MAX_RXFREE_QUEUES	(IX_ETHNPE_NUM_PHYSICAL_PORTS)

/**
 *
 * @brief Maximum number of Tx Queues (1 per port)
 *
 */
#define IX_ETHACC_MAX_TX_QUEUES		(IX_ETHNPE_NUM_PHYSICAL_PORTS)

/**
 *
 * @brief Maximum number of TxDone Queues (1 shared)
 *
 */
#define IX_ETHACC_MAX_TXDONE_QUEUES	(1)

/**
 *
 * @brief Maximum number of Queues (total of all queues)
 *
 */
#define IX_ETHACC_MAX_QUEUES \
	(IX_ETHACC_MAX_RX_QUEUES + IX_ETHACC_MAX_RXFREE_QUEUES \
	 + IX_ETHACC_MAX_TX_QUEUES + IX_ETHACC_MAX_TXDONE_QUEUES)

/**
 * @brief value used to index the Static queue assignment table
 */
#define IX_ETHACC_TX_QUEUE_ASSIGNMENT_INDEX    (0)
 
/**
 * @brief value used to index the Static queue assignment table
 */
#define IX_ETHACC_RXFREE_QUEUE_ASSIGNMENT_INDEX      (1)

