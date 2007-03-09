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
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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
#if IX_ETH_ACC_RX_FRAME_ETH_Q >= (IX_QMGR_MIN_QUEUPP_QID ) |    	\
 IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q  >=  (IX_QMGR_MIN_QUEUPP_QID) | 	\
 IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q  >=  (IX_QMGR_MIN_QUEUPP_QID) | 	\
 IX_ETH_ACC_TX_FRAME_ENET0_Q >=   (IX_QMGR_MIN_QUEUPP_QID) |  		\
 IX_ETH_ACC_TX_FRAME_ENET1_Q >=   (IX_QMGR_MIN_QUEUPP_QID) | 		\
 IX_ETH_ACC_TX_FRAME_DONE_ETH_Q  >=  (IX_QMGR_MIN_QUEUPP_QID)  
#error "Not all Ethernet Access Queues are betweem 1-31, requires full functionalty Q's unless otherwise validated "
#endif

/**
*
* @typedef  IxEthAccQregInfo
*
* @brief 
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

/*
 * Prototypes for all QM callbacks.
 */

/* 
 * Rx Callbacks 
 */
IX_ETH_ACC_PUBLIC
void  ixEthRxFrameQMCallback(IxQMgrQId, IxQMgrCallbackId);

IX_ETH_ACC_PUBLIC
void  ixEthRxMultiBufferQMCallback(IxQMgrQId, IxQMgrCallbackId);

IX_ETH_ACC_PUBLIC
void  ixEthRxFreeQMCallback(IxQMgrQId, IxQMgrCallbackId);

/* 
 * Tx Callback.
 */
IX_ETH_ACC_PUBLIC
void  ixEthTxFrameQMCallback(IxQMgrQId, IxQMgrCallbackId);

IX_ETH_ACC_PUBLIC
void  ixEthTxFrameDoneQMCallback(IxQMgrQId, IxQMgrCallbackId );


#define IX_ETH_ACC_QM_QUEUE_DISPATCH_PRIORITY (IX_QMGR_Q_PRIORITY_0) /* Highest priority */

/*
 * Queue watermarks
 */
#define IX_ETH_ACC_RX_FRAME_ETH_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_NOT_E   )
#define IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )   
#define IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )
#define IX_ETH_ACC_RX_FREE_BUFF_ENET2_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )
#define IX_ETH_ACC_TX_FRAME_ENET0_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   ) 
#define IX_ETH_ACC_TX_FRAME_ENET1_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )
#define IX_ETH_ACC_TX_FRAME_ENET2_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_E   )
#define IX_ETH_ACC_TX_FRAME_DONE_ETH_Q_SOURCE 		(IX_QMGR_Q_SOURCE_ID_NOT_E   )
