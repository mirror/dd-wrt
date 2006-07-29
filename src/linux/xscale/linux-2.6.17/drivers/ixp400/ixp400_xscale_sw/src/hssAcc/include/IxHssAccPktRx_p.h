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
 * IXP400 SW Release Crypto version 2.1
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
