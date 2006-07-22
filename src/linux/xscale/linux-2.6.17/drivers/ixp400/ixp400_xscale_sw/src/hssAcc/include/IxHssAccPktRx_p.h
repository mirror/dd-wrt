/**
 * @file IxHssAccPktRx_p.h
 *
 * @author Intel Corporation
 * @date  14 Dec 2001
 *
 * @brief This file contains the private API of the HSS Packetised Rx module
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
 * @defgroup IxHssAccPktRx_p IxHssAccPktRx_p
 *
 * @brief IXP400 HSS Access Packet Rx Module
 * 
 * @{
 */

#ifndef IXHSSACCPKTRX_P_H
#define IXHSSACCPKTRX_P_H

#include "IxHssAcc.h"
#include "IxNpeA.h"
#include "IxQMgr.h" 
#include "IxOsal.h"

/**
 * Prototypes for interface functions.
 */

/**
 * @fn void ixHssAccPktRxFreeLowCallback (IxQMgrQId qId, 
                                          IxQMgrCallbackId cbId);
 *
 * @brief This function is the Callback run by the RxFreeQMQ, and is called
 * when the RxFree QMQ reaches a NearlyEmpty threshold.
 *
 * @param IxQMgrQId qId (in)-  The IxQMQ Id, this is the ID number of the queue.
 * @param IxQMgrCallbackId cbId (in) - The IxQMQ callback ID. This Id 
 * uniquely identifies a callback function.
 * @return void 
 */
void 
ixHssAccPktRxFreeLowCallback (IxQMgrQId qId, 
			      IxQMgrCallbackId cbId);


/**
 * @fn void ixHssAccPktRxCallback (IxQMgrQId qId, 
                                   IxQMgrCallbackId cbId);
 *
 * @brief This function is the Callback run by the RxQMQ, and is called 
 * when the Rx QMQ reaches a NotEmpty condition.
 *
 * @param IxQMgrQId qId (in) -  The IxQMQ Queue Id, this is the ID number of 
 * the queue
 * @param IxQMgrCallbackId cbId (in) - The IxQMQ callback ID
 * @return void 
 */
void 
ixHssAccPktRxCallback (IxQMgrQId qId, 
		       IxQMgrCallbackId cbId);


/**
 * @fn IX_STATUS ixHssAccPktRxFreeReplenish (IxHssAccHssPort hssPortId, 
                                             IxHssAccHdlcPort hdlcPortId, 
					     IX_OSAL_MBUF *buffer);
 *
 * @brief This funtion is called to fill one of the Rx Free QMQ with 
 * descriptors containing mbufs.  
 * 
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number on 
 * which the HDLC port of the client that is calling this function is on
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the HDLC port number of 
 * the client that is calling this function is on. 
 * @param IX_OSAL_MBUF *buffer -  The client supplied mbuf that is to be added to a 
 * ixHssAccPDMDescriptor before the ixHssAccPDMDescriptor is placed on the 
 * Rx Free QMQ.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully due
 *                          to a parameter error
 * 
 */
IX_STATUS 
ixHssAccPktRxFreeReplenish (IxHssAccHssPort hssPortId, 
			    IxHssAccHdlcPort hdlcPortId, 
			    IX_OSAL_MBUF *buffer);


/**
 * @fn void ixHssAccPktRxShow (void);
 *
 * @brief This function is called to display the stats for the RX module.
 *
 * @return void 
 */
void 
ixHssAccPktRxShow (void);


/**
 * @fn void ixHssAccPktRxStatsInit (void); 
 *
 * @brief This function is called to resets all the stats for the RX module.
 * 
 * @return void 
 */
void 
ixHssAccPktRxStatsInit (void);

/**
 * @fn IX_STATUS ixHssAccPktRxInit (void)
 *
 * @brief This function will initialise the PktRx module
 *
 * @return 
 *            - IX_SUCCESS Function executed successfully
 *            - IX_FAIL Function failed to execute
 */
IX_STATUS 
ixHssAccPktRxInit (void);


/**
 * @fn IX_STATUS ixHssAccPktRxUninit (void)
 *
 * @brief  This function Uninitialises all resources for all descriptor pools 
 * and descriptors contained in these pools.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully 
 *                                   due to a resource error
 */

IX_STATUS ixHssAccPktRxUninit (void);

#endif /* IXHSSACCPKTRX_P_H */

/**
 * @} defgroup IxHssAccPktRx_p
 */
