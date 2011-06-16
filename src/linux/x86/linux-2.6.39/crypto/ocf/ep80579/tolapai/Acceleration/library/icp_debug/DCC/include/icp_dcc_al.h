/*******************************************************************************
 * 
 * @par
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
 *****************************************************************************/
#if !defined(__ICP_DCC_AL_H__)
#define __ICP_DCC_AL_H__

#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */

/**
 *****************************************************************************
 * 
 * @defgroup DCCAL Debug Common Component Access Layer Interface APIs
 *
 * @ingroup DCCLibrary
 *
 * @description
 *         This file contains the exported function definitions of 
 *         Debug Common Component used by the Access Layer Components
 *      
 * 
 *****************************************************************************/

/* Include Header Files */
#include "icp.h"
#include "dcc/icp_dcc_common.h"


/*===========================================================================*/

/* Component Version */

/* Acceleration Software Interface */

/**
 *****************************************************************************
 * @ingroup DCCAL
 *      Register the Software Version Information with DCC
 * 
 * @description
 *      This function is used to register the Software version information 
 *      with DCC.
 *
 * @context
 *      This function does not sleep and therefore may be called from 
 *      interrupt context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is non-blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pSoftwareVersion: IN: Pointer to storage containing the software 
 *                               version information to be registered with DCC.
 * @param  pModuleId: OUT: Module ID returned by the DCC for the 
 *                         package/component registering its version info   
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_RESOURCE Error related to system resource
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
icp_status_t
icp_DccComponentVersionRegister (
        icp_dcc_ver_info_t *pSoftwareVersion, 
        uint32_t *pModuleId 
    );


/*============================================================================*/

/* Liveness */

/* Acceleration Software interface */
/**
 *****************************************************************************
 * @ingroup DCCAL
 *     Register Thread response Verification Handler
 * 
 * @description
 *      Firmware will register response verification handler with DCC 
 *      in this format
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes 
 *
 * @threadSafe
 *      Yes
 *
 * @param  pThreadId: IN: Thread id whose response status is returned
 * @param  pThreadStatus: IN/OUT: The system threads execution status:
 *                           ICP_DCC_THREAD_ID_LIVE or ICP_DCC_THREAD_ID_DEAD 
 * @param  pUsrPrivData: IN: pointer to user private data 
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see 
 *
 *****************************************************************************/
typedef icp_status_t (*icp_DccLivenessVerificationHandler) (
        icp_dcc_thread_id_t *pThreadId,
        icp_dcc_thread_status_t *pThreadStatus,
        const void *pUsrPrivData
    );

/**
 *****************************************************************************
 * @ingroup DCCAL
 *      Register Acceleration Software Thread response 
 *      Verification Memory Location
 * 
 * @description
 *      This API will be used by the Acceleration Software components to 
 *      register the memory location location of the threads with DCC. 
 *      For ASU, corresponding access layer components will allocate memory 
 *      and patch it to the PPS and then will register the location with DCC. 
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pThreadId: IN: The thread id whose response is being verified
 * @param  pLivenessLocation: IN: The thread id response Update location
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_RESOURCE Error related to system resource
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see 
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessLocationRegister (
        icp_dcc_thread_id_t *pThreadId,
        uint32_t *pLivenessLocation
    );

/**
 *****************************************************************************
 * @ingroup DCCAL
 *        Un-Register Acceleration Software Layer & ASU Thread response 
 *        Verification Memory Location
 * 
 * @description
 *        This API will be used by the access layer components to un-register 
 *        the memory location of each thread of execution with the DCC.
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pThreadId: IN: The thread id whose response location is to be 
 *                        unregistered  
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see 
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessLocationUnregister (
        icp_dcc_thread_id_t *pThreadId
    );

/**
 *****************************************************************************
 * @ingroup DCCAL
 *      Register Thread response Verification Handler
 * 
 * @description
 *      This API will be used by the Acceleration Software components to 
 *      register the livenes verification handler firmware.
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pThreadId: IN: The thread id whose response is being verified
 * @param  pLivenessVerificationHandler: IN: callback of thread to verify response
 * @param  pUsrPrivData: IN: pointer to user private data 
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_RESOURCE Error related to system resource
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see icp_DccLivenessVerify @ icp_dcc.h  
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessVerificationHandlerRegister (
        icp_dcc_thread_id_t *pThreadId,
        icp_DccLivenessVerificationHandler pLivenessVerificationHandler,
        const void *pUsrPrivData
    );

/**
 ******************************************************************************
 * @ingroup DCCAL
 *      Un-Register Thread response Verification Handler 
 * 
 * @description
 *      This API will be used by the Acceleration Software components to 
 *      un-register response verification handler for firmware.
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pThreadId: IN: The thread id whose response callback is unregistered
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see icp_DccLivenessVerify @ icp_dcc.h
 *
 *****************************************************************************/
icp_status_t
icp_DccLivenessVerificationHandlerUnregister (
        icp_dcc_thread_id_t *pThreadId
    );

/*============================================================================*/

/* DUMP */

/* AL Interface */

/**
 *****************************************************************************
 * @ingroup DCCAL
 *        Acceleration Software Modules Data dump handler
 * 
 * @description
 *        Acceleration Software Modules register their data dump handler with
 *        DCC in this format.
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pDataDump: IN/OUT: Data Dump information from the module
 * @param  pDataDumpSize: IN/OUT: The actual size of dump data present in 
 *                                the buffer.
 * @param  pUsrPrivData: IN: pointer to user private data 
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
typedef icp_status_t (*icp_DccDataDumpHandler)(
        void *pDataDump,
        uint32_t *pDataDumpSize,
        const void *pUsrPrivData);

/**
 *****************************************************************************
 * @ingroup DCCAL
 *        Register Acceleration Software Module Data dump handler with DCC
 * 
 * @description
 *        Acceleration Software Modules register their data dump handler with
 *        DCC using this function.
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  moduleId: IN: Module ID of the access layer module registering the
 *                       dump handler.
 * @param  dataDumpSize: IN: The actual size of dump data present in the buffer.
 * @param  dataDumpHandler: IN: Data dump handler
 * @param  pUsrPrivData: IN: pointer to user private data 
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_RESOURCE Error related to system resource
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 * @see  icp_DccDataDumpHandler 
 *
 *****************************************************************************/
icp_status_t
icp_DccDataDumpHandlerRegister (
        uint32_t moduleId,
        uint32_t dataDumpSize,
        icp_DccDataDumpHandler dataDumpHandler,
        const void *pUsrPrivData
    );

/**
 *****************************************************************************
 * @ingroup DCCAL
 *        Un-Register the Data dump handler registered with DCC
 * 
 * @description
 *        Acceleration Software Modules un-register the registered data dump 
 *        handler from DCC using this function.
 *
 *
 * @context
 *      This function may sleep and therefore may not be called from interrupt
 *      context.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  moduleId: IN: Module ID of the acceleration software module 
 *                       unregistering the data dump handler.
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_INVALID_PARAM Invalid parameter passed 
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
icp_status_t
icp_DccDataDumpHandlerUnregister (
        uint32_t moduleId
    );

/*============================================================================*/

/* Software Error Notification (SEN) */


/* Acceleration Software Interface */
/**
 *****************************************************************************
 * @ingroup DCCAL
 *      Notify DCC of Software Error Notification (SEN) event
 * 
 * @description
 *      This API is used by the Acceleration Software to notify any Software 
 *      Error Notification (referred to as SEN hereafter) messages to DCC. 
 *      This function will call the SEN event handler. If no SEN event handler
 *      is registered, then only Logger API is called.
 *
 * @context
 *      This function runs in the context of the calling function. This can be
 *      called from interrupt context also.
 *
 * @assumptions
 *      None
 *
 * @sideEffects
 *      None
 *
 * @blocking
 *      This function is blocking.
 *      
 * @reentrant
 *      Yes
 *
 * @threadSafe
 *      Yes
 *
 * @param  pSenMsg: IN: Details of the occured Software Error 
 *
 * @retval  ICP_STATUS_SUCCESS If successful
 * @retval  ICP_STATUS_NULL_PARAM One or more parameters is null
 * @retval  ICP_STATUS_FAIL Unspecified error 
 *
 * @pre
 *      None
 *
 * @post
 *      None.
 *
 *****************************************************************************/
icp_status_t
icp_DccSenNotify (
        icp_dcc_sen_msg_t *pSenMsg
    );


#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__ICP_DCC_AL_H__) */
