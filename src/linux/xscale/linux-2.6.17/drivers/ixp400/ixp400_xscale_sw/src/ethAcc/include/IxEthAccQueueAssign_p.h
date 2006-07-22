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
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
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
#if IX_ETH_ACC_RX_FREE_NPEA_Q  >=  (IX_QMGR_MIN_QUEUPP_QID) | 	\
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

