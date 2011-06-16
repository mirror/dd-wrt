/***************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 ***************************************************************************/

/**
 *****************************************************************************
 * @file qat_comms.h
 *
 * @defgroup icp_QatComms  QAT Comms API
 *
 * @ingroup icp_Qatal
 *
 * @description
 *      This file documents the external interfaces for interfacing with the QAT
 *
 *****************************************************************************/

#ifndef QAT_COMMS_H
#define QAT_COMMS_H

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/
#include "cpa.h"
#include "icp_services.h"

#include "IxOsal.h"

#include "qat_comms_statistics.h"
/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/

/*
*******************************************************************************
* define OS specific macros
*******************************************************************************
*/


/**
 *****************************************************************************
 * @ingroup icp_QatComms
 *      Request priority
 * @description
 *      Enumeration of priority of the request to be given to the API. 
 *      Currently two levels - HIGH and NORMAL are supported. HIGH priority
 *      requests will be prioritized on a "best-effort" basis over requests 
 *      that are marked with a NORMAL priority. HIGH priority is only supported
 *      for certain QAT services.
 *
 *****************************************************************************/
typedef enum
{
    QAT_COMMS_PRIORITY_START_DELIMITER = 0, /**< enum start delimiter */
    QAT_COMMS_PRIORITY_NORMAL,              /**< Normal/Default priority */
    QAT_COMMS_PRIORITY_HIGH,                /**< High priority */
    QAT_COMMS_PRIORITY_END_DELIMITER        /**< enum end delimiter */
} qat_comms_priority_t;


/**
 *****************************************************************************
 * @ingroup icp_QatComms
 *      Definition of callback function for response messages
 *
 * @description
 *      This data structure specifies the prototype for the callback function.
 *      The callback function is registered by the application using
 *      the QatComms_ResponseCbSet() call.
 *
 * @context
 *      The function will be invoked from an interrupt bottom-half context
 *
 * @param pRespMsg      IN     A pointer to the 64-byte response message
 *
 * @param qatReqType    IN     QAT service type of the response message
 *
 * @retval
 *      None
 *
 * @see
 *      QatComms_ResponseCbSet(), QatComms_MsgSend()
 *
 *****************************************************************************/
typedef void (*qat_comms_cb_func_t)(void *pRespMsg,
                                        icp_arch_if_request_t qatReqType);


/**
 *******************************************************************************
 * @ingroup icp_QatComms
 *      Init the QAT interface component
 *
 * @description
 *      Initialse this component and allocate necessary resources.
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 *
 *****************************************************************************/
CpaStatus
QatComms_Init(void);


/**
 *******************************************************************************
 * @ingroup icp_QatComms
 *      Register a callback to handle message responses for a given request type
 *
 * @description
 *      Register a callback to handle message responses for a given request type.
 *
 * @param pResponseCb    IN     Message response callback function pointer
 *                              Use NULL to de-register existing callback.
 *
 * @param qatReqType     IN     QAT request type for the request.
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 *
 * @pre @ref QatComms_Init() must have been called already
 *
 *****************************************************************************/
CpaStatus
QatComms_ResponseCbSet(
    qat_comms_cb_func_t pResponseCb,
    icp_arch_if_request_t qatReqType);

/**
 *******************************************************************************
 * @ingroup icp_QatComms
 *      Create the header for a QAT request
 *
 * @description
 *      Fills in the common ICP S interface request header.
 *
 * @param pMsg           IN     A pointer to the 64-byte QAT request message
 *
 * @param qatReqType     IN     QAT request type for the request.
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 * @retval ICP_E_INVALID_PARAM  Invalid parameter
 *
 * @pre @ref QatComms_Init() must have been called already
 *
 *****************************************************************************/
CpaStatus
QatComms_ReqHdrCreate(
    void *pMsg,
    icp_arch_if_request_t qatReqType);

/**
 *******************************************************************************
 * @ingroup icp_QatComms
 *      Send a message to the QAT
 *
 * @description
 *      Selects request and response ring, fills in the common ICP S interface
 *      request header, and sends the message to the QAT.
 *
 * @param pMsg           IN     A pointer to the 64-byte QAT request message
 *                              The message contents will be copied to ring
 *                              memory, so the memory for this message (pMsg)
 *                              can be freed when this function returns
 *
 * @param qatReqType     IN     QAT request type for the request. Used to
 *                              select correct request ring.
 *
 * @param priority       IN     Priority for the request. Used to
 *                              select the correct request ring. If not
 *                              applicable, specify Normal priority.
 *
 * @param instanceHandle    IN     Acceleration engine to which the message will 
 *                              be sent. Used to select the request ring.
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 * @retval ICP_E_RETRY          Request ring full, try again
 * @retval ICP_E_INVALID_PARAM  Invalid parameter
 *
 * @pre @ref QatComms_Init() must have been called already
 *
 * @pre A callback handler must have been registered for the requestType
 *      using @ref QatComms_ResponseCbSet()
 *
 * @pre A valid message header must have been inserted in the message by
 *      using @ref QatComms_ReqHdrCreate()
 *
 * @note Memory must be allocated for the message prior to calling this
 *       function. As the contents of the message are copied onto the ring it
 *       can be allocated as a variable on the stack. Unused fields in the
 *       message should be initialised to 0.
 *
 *****************************************************************************/
CpaStatus
QatComms_MsgSend(
    void *pMsg,
    icp_arch_if_request_t qatReqType,
    qat_comms_priority_t priority,
    CpaInstanceHandle instanceHandle);

/**
 *******************************************************************************
 * @ingroup icp_QatComms
 *      Shutdown the QAT interface component
 *
 * @description
 *      Shutdown the QAT interface component.  Free any resources allocated by
 *      this component.
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 *
 * @pre @ref QatComms_Init() must have been called already
 *
 * @post This component can not be used to exchange messages with the QAT after
 *       QatComms_Shutdown() is called.  QatComms_Init() may be used to
 *       re-initialise the component again.
 *
 *****************************************************************************/
CpaStatus
QatComms_Shutdown(void);

/**
 *******************************************************************************
 * @ingroup icp_QatComms
 *      Query no of request & Responses
 *
 * @description
 *      Queries the no. of requests, responses and retries per reqtype
 *
 * @retval ICP_E_NO_ERROR       Function executed successfully.
 * @retval ICP_E_FAIL           Function failed.
 *
 *
 *****************************************************************************/
CpaStatus 
QatComms_MsgCountGet(
                     icp_arch_if_request_t qatReqType, 
                     Cpa32U *pNumSent, 
                     Cpa32U *pNumRecieved,
					 Cpa32U *pNumRetry);


/**
 *****************************************************************************
 * @ingroup icp_QatalCfg
 *      QAT Access Layer - Status and stats of Firmware
 * 
 * @description
 *      This function will query the Firmware and if problem or lockup or 
 *      timeout return Fail. If all is successful it will return success plus 
 *      no. commands sent and received by the Firmware
 * 
 * @context
 *      This function may sleep, and so MUST NOT be called in interrupt
 *      context.
 *
 * @assumptions
 *      System is up and running
 *
 * @sideEffects
 *      None
 * @blocking
 *      This function is synchronous and blocking.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * @param	Cpa32U *pNumSent       IN  Ptr to data to store no. of command sent
 * @param   Cpa32U *pNumReceived   IN  Ptr to data to store no. of command received
 * 
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * 
 * @pre
 *      The firmware is up and running.
 * @post
 *      None
 *
 *****************************************************************************/
CpaStatus Qatal_FWCountGet(Cpa32U *pNumSent, Cpa32U *pNumReceived );



/**
 *******************************************************************************
 * @ingroup icp_QatComms
 *      This is the QAT_comms ISR 
 *
 * @description
 *      This is the QAT_comms Interrupt Service Routine 
 *
 * @retval CPA_STATUS_SUCCESS       Function executed successfully.
 *
 *
 *****************************************************************************/
CpaStatus QatComms_intr(void);



/**
 *******************************************************************************
 * @ingroup icp_QatComms
 *      This is the QAT_comms Bh Handler
 *
 * @description
 *      This is used by ASD to bind the bottom half interrupt to Ring Interrupt processing
 *
 * @retval void
 *
 *
 *****************************************************************************/
void 
QatComms_bh_handler(void* priv_data, int );


/**
 *******************************************************************************
 * @ingroup icp_QatComms
 *      This is the QAT_comms Bh Handler Register
 *
 * @description
 *      This is used by ASD to register the function bind the bottom half interrupt 
 *      to Ring Interrupt processing
 *
 * @retval CPA_STATUS_SUCCESS       Function executed successfully.
 *
 *
 *****************************************************************************/
typedef int (*QatComms_bh_schedule) (void);


CpaStatus QatComms_bh_schedule_register(QatComms_bh_schedule bhsch);

#endif /* __QAT_COMMS_H__ */
