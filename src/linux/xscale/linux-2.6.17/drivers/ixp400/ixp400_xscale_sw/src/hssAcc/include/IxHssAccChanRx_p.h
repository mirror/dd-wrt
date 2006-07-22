/**
 * @file IxHssAccChanRx_p.h
 * 
 * @author Intel Corporation
 * @date 11-DEC-2001
 *
 * @brief This file contains the private API of the HSS Channelised Rx
 * module
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
 * @defgroup IxHssAccChanRx_p IxHssAccChanRx_p
 *
 * @brief The private API for the HssAccess Channelised Rx module
 * 
 * @{
 */

#ifndef IXHSSACCCHANRX_P_H
#define IXHSSACCCHANRX_P_H

#include "IxQMgr.h"
#include "IxHssAcc.h"

/**
 * Prototypes for interface functions.
 */

/**
 * #defines for function return types, etc.
 */

/**
 * @fn void ixHssAccChanRxTriggerCallback (IxQMgrQId qId, 
         IxQMgrCallbackId cbId)
 *
 * @brief While a client is connected to the channelised service, this
 * callback will be registered as the handler for the hssSyncQ. This
 * function, when called will execute a read of the hssSyncQ, and in turn
 * callback the client.
 *
 * @param IxQMgrQId qId (in) - The QMgr queue id for the hssSyncQ
 * @param IxQMgrCallbackId cbId (in) - The QMgr callback id.
 *
 * @return void
 */
void 
ixHssAccChanRxTriggerCallback (IxQMgrQId qId, 
			       IxQMgrCallbackId cbId);

/**
 * @fn IX_STATUS ixHssAccChanRxQCheck (IxHssAccHssPort hssPortId,
           BOOL *dataRecvd, 
           unsigned *rxOffset, 
	   unsigned *txOffset, 
	   unsigned *numHssErrs)
 *
 * @brief This function gets called by the service interface to perform a
 * read of the hssSyncQ
 *
 * @param IxHssAccHssPort hssPortId (in) - The HSS port Id. There are two
 * identical ports (0-1).
 * @param BOOL *dataRecvd (out) - The boolean will be set to true if
 * something was successfully read from the hssSyncQ
 * @param unsigned *rxOffset (out) - An offset to indicate to the client
 * where within the receive buffers the NPE has just written the received
 * data to.
 * @param unsigned *txOffset (out) - An offset to indicate to the client
 * from where within the txPtrList the NPE is currently transmitting from.
 * @param unsigned *numHssErrs (out) - The total number of HSS port errors
 * since initial port configuration.
 *
 * @return IX_STATUS
 */
IX_STATUS 
ixHssAccChanRxQCheck (IxHssAccHssPort hssPortId, 
		      BOOL *dataRecvd, 
		      unsigned *rxOffset, 
		      unsigned *txOffset, 
		      unsigned *numHssErrs);

/**
 * @fn void ixHssAccChanRxShow (void)
 *
 * @brief This function will display the current state of the IxHssAcc
 * ChanRx module
 *
 * @return void
 */
void 
ixHssAccChanRxShow (void);

/**
 * @fn void ixHssAccChanRxStatsInit (void)
 *
 * @brief This function will initialise the stats of the IxHssAcc ChanRx
 * module
 *
 * @return void
 */
void 
ixHssAccChanRxStatsInit (void);

/**
 * @fn IX_STATUS ixHssAccChanRxInit (void)
 *
 * @brief This function will initialise the ChanRx module
 *
 * @return 
 *            - IX_SUCCESS Function executed successfully
 *            - IX_FAIL Function failed to execute
 */
IX_STATUS 
ixHssAccChanRxInit (void);

/**
 * @fn IX_STATUS ixHssAccChanRxUninit (void)
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

IX_STATUS ixHssAccChanRxUninit (void);


#endif /* IXHSSACCCHANRX_P_H */

/**
 * @} defgroup IxHssAccChanRx_p
 */
