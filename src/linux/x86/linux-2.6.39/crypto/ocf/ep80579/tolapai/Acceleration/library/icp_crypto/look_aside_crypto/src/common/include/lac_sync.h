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
 ***************************************************************************
 * @file lac_sync.h
 *
 * @defgroup LacSync     LAC synchronous
 *
 * @ingroup LacCommon
 *
 * Function prototypes and defines for synchronous support
 *
 ***************************************************************************/


#ifndef LAC_SYNC_H
#define LAC_SYNC_H

#include "cpa.h"
#include "cpa_cy_sym.h"
#include "lac_mem.h"
#include "IxOsal.h"


/**
 *****************************************************************************
 * @ingroup LacSync
 *
 * @description
 *      LAC cookie for synchronous support
 *
 *****************************************************************************/
typedef struct lac_sync_op_data_s
{
    IxOsalSemaphore sid;
    /**< Semaphore to signal */
    CpaStatus status;               
    /**< Output - Status of the QAT response */
    CpaBoolean opResult;            
    /**< Output - Verification of the operation/protocol status */
} lac_sync_op_data_t;

#define LAC_PKE_SYNC_CALLBACK_TIMEOUT   (41000)
/**< @ingroup LacSync
 * Timeout waiting for an async callbacks in msecs.
 * This is derived from the max latency of a PKE request  + 1 sec
 */

#define LAC_SYM_SYNC_CALLBACK_TIMEOUT   (1000) 
/**< @ingroup LacSyn
 * Timeout for wait for symmetric response in msecs */

#define LAC_SYN_INITIAL_SEM_VALUE       (0)
/**< @ingroup LacSyn
 * Initial value of the sync waiting semaphore */

/**
 *******************************************************************************
 * @ingroup LacSync
 *      This function allocates a sync op data cookie 
 *      and creates and initialises the OSAL semaphore
 *
 * @param[in] ppSyncCallbackCookie  Pointer to synch op data
 *
 * @retval CPA_STATUS_RESOURCE  Failed to allocate the memory for the cookie.
 * @retval CPA_STATUS_SUCCESS   Success
 * 
 ******************************************************************************/
static __inline
CpaStatus LacSync_CreateSyncCookie(lac_sync_op_data_t **ppSyncCallbackCookie) 
{    
    CpaStatus status = LAC_OS_MALLOC(ppSyncCallbackCookie,                    
                            sizeof(lac_sync_op_data_t));                 
    
    if (CPA_STATUS_SUCCESS == status)                               
    {                                                               
        status = LAC_INIT_SEMAPHORE((*ppSyncCallbackCookie)->sid, 
                            LAC_SYN_INITIAL_SEM_VALUE);
    }

    return status;
}

/**
 *******************************************************************************
 * @ingroup LacSync
 *      This macro frees a sync op data cookie and destroys the OSAL semaphore
 *
 * @param[in] ppSyncCallbackCookie      Pointer to sync op data
 * 
 * @return void
 ******************************************************************************/
static __inline 
CpaStatus LacSync_DestroySyncCookie(lac_sync_op_data_t **ppSyncCallbackCookie)
{    
    CpaStatus status = CPA_STATUS_SUCCESS;

    status = LAC_DESTROY_SEMAPHORE((*ppSyncCallbackCookie)->sid);       
    LAC_OS_FREE(*ppSyncCallbackCookie);                               
    return status;
}

/**
 *****************************************************************************
 * @ingroup LacSync
 *      Function which will wait for a sync callback on a given cookie.
 * 
 * @param[in] pSyncCallbackCookie       Pointer to sync op data
 * @param[in] timeOut                   Time to wait for callback (msec)
 * @param[out] pStatus                  Status returned by the callback
 * @param[out] pOpStatus                Operation status returned by callback.
 *
 * @retval CPA_STATUS_SUCCESS   Success
 * @retval CPA_STATUS_SUCCESS   Fail waiting for a callback 
 *
 *****************************************************************************/
static __inline
CpaStatus 
LacSync_WaitForCallback(lac_sync_op_data_t *pSyncCallbackCookie,
        Cpa32S timeOut, CpaStatus *pStatus, CpaBoolean *pOpStatus)
{   
    CpaStatus status = CPA_STATUS_SUCCESS;

    status = LAC_WAIT_SEMAPHORE(pSyncCallbackCookie->sid, timeOut);

    if (CPA_STATUS_SUCCESS == status)
    {
         *pStatus = pSyncCallbackCookie->status;
         if (NULL != pOpStatus)
         {
             *pOpStatus = pSyncCallbackCookie->opResult;
         }
    }

    return status;
}

/**
 *****************************************************************************
 * @ingroup LacSync
 *      Generic verify callback function.
 * @description
 *      This function is used when the API is called in synchronous mode.
 *      It's assumed the callbackTag holds a lac_sync_op_data_t type
 *      and when the callback is received, this callback shall set the
 *      status element of that cookie structure and kick the sid.
 *      This function may be used directly as a callback function.
 * 
 * @param[in]  callbackTag       Callback Tag
 * @param[in]  status            Status of callback
 * @param[out] pOpdata           Pointer to the Op Data
 * @param[out] opResult          Boolean to indicate the result of the operation
 * 
 * @return void
 *****************************************************************************/
void LacSync_GenVerifyCb(void *callbackTag, CpaStatus status,
                void *pOpdata, CpaBoolean opResult);
 

/**
 *****************************************************************************
 * @ingroup LacSync
 *      Generic flatbuffer callback function.
 * @description
 *      This function is used when the API is called in synchronous mode.
 *      It's assumed the callbackTag holds a lac_sync_op_data_t type
 *      and when the callback is received, this callback shall set the
 *      status element of that cookie structure and kick the sid.
 *      This function may be used directly as a callback function.
 *
 * @param[in]  callbackTag       Callback Tag
 * @param[in]  status            Status of callback
 * @param[in]  pOpdata           Pointer to the Op Data
 * @param[out] pOut              Pointer to the flat buffer
 * 
 * @return void
 *****************************************************************************/
void LacSync_GenFlatBufCb(void *callbackTag, CpaStatus status,
                void *pOpdata, CpaFlatBuffer *pOut);
 
/**
 *****************************************************************************
 * @ingroup LacSync
 *      Generic flatbuffer verify callback function.
 * @description
 *      This function is used when the API is called in synchronous mode.
 *      It's assumed the callbackTag holds a lac_sync_op_data_t type
 *      and when the callback is received, this callback shall set the
 *      status and opResult element of that cookie structure and 
 *      kick the sid.
 *      This function may be used directly as a callback function.
 *
 * @param[in]  callbackTag       Callback Tag
 * @param[in]  status            Status of callback
 * @param[in]  pOpdata           Pointer to the Op Data
 * @param[out] opResult          Boolean to indicate the result of the operation
 * @param[out] pOut              Pointer to the flat buffer
 * 
 * @return void
 *****************************************************************************/
void LacSync_GenFlatBufVerifyCb(void *callbackTag, CpaStatus status,
                void *pOpdata, CpaBoolean opResult, CpaFlatBuffer *pOut);

/**
 *****************************************************************************
 * @ingroup LacSync
 *      Generic dual flatbuffer verify callback function.
 * @description
 *      This function is used when the API is called in synchronous mode.
 *      It's assumed the callbackTag holds a lac_sync_op_data_t type
 *      and when the callback is received, this callback shall set the
 *      status and opResult element of that cookie structure and 
 *      kick the sid.
 *      This function may be used directly as a callback function.
 *
 * @param[in]  callbackTag       Callback Tag
 * @param[in]  status            Status of callback
 * @param[in]  pOpdata           Pointer to the Op Data
 * @param[out] opResult          Boolean to indicate the result of the operation
 * @param[out] pOut0             Pointer to the flat buffer
 * @param[out] pOut1             Pointer to the flat buffer
 * 
 * @return void
 *****************************************************************************/
void LacSync_GenDualFlatBufVerifyCb(void *callbackTag, 
                CpaStatus status, void *pOpdata, CpaBoolean opResult,
                CpaFlatBuffer *pOut0, CpaFlatBuffer *pOut1);

/**
 *****************************************************************************
 * @ingroup LacSync
 *      Generic bufferList callback function.
 * @description
 *      This function is used when the API is called in synchronous mode.
 *      It's assumed the callbackTag holds a lac_sync_op_data_t type
 *      and when the callback is received, this callback shall set the
 *      status and opResult element of that cookie structure and 
 *      kick the sid.
 *      This function may be used directly as a callback function.
 * 
 * @param[in]  callbackTag       Callback Tag
 * @param[in]  status            Status of callback
 * @param[in]  operationType     Operation Type
 * @param[in]  pOpData           Pointer to the Op Data
 * @param[out] pDstBuffer        Pointer to destination buffer list
 * @param[out] opResult          Boolean to indicate the result of the operation
 * 
 * @return void
 *
 *****************************************************************************/
void LacSync_GenBufListVerifyCb(void *callbackTag, CpaStatus status,
                CpaCySymOp operationType, void *pOpData,
                CpaBufferList *pDstBuffer, CpaBoolean opResult);

/**
 *****************************************************************************
 * @ingroup LacSync
 *      Generic wake up function.
 * @description
 *      This function is used when the API is called in synchronous 
 *      mode.
 *      It's assumed the callbackTag holds a lac_sync_op_data_t type
 *      and when the callback is received, this callback shall set 
 *      the status element of that cookie structure and kick the 
 *      sid.
 *      This function maybe called from an async callback. 
 *
 * @param[in] callbackTag       Callback Tag
 * @param[in] status            Status of callback
 * 
 * @return void
 *****************************************************************************/
void LacSync_GenWakeupSyncCaller(void *callbackTag, 
            CpaStatus status);

/**
 *****************************************************************************
 * @ingroup LacSync
 *      Generic wake up verify function.
 * @description
 *      This function is used when the API is called in synchronous 
 *      mode.
 *      It's assumed the callbackTag holds a lac_sync_op_data_t type
 *      and when the callback is received, this callback shall set 
 *      the status element and the opResult of that cookie structure 
 *      and kick the sid.
 *      This function maybe called from an async callback. 
 *
 * @param[in]  callbackTag       Callback Tag
 * @param[in]  status            Status of callback
 * @param[out] opResult          Boolean to indicate the result of the operation
 *  
 * @return void
 *****************************************************************************/
void LacSync_GenVerifyWakeupSyncCaller(void *callbackTag, 
            CpaStatus status, CpaBoolean opResult);

#endif /*LAC_SYNC_H*/
