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
