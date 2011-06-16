/******************************************************************************
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
 *****************************************************************************/

/**
 ***************************************************************************
 * @file lac_sym_queue.c     Functions for sending/queuing symmetric requests
 *
 * @ingroup LacSym
 *
 ***************************************************************************/


/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

#include "cpa.h"
#include "lac_sym_queue.h"
#include "lac_session.h"
#include "lac_sym.h"


/*
*******************************************************************************
* Define public/global function definitions
*******************************************************************************
*/

CpaStatus
LacSymQueue_RequestSend(const CpaInstanceHandle instanceHandle,
                        lac_sym_bulk_cookie_t *pRequest,
                        lac_session_desc_t *pSessionDesc)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    CpaBoolean enqueued = CPA_FALSE;

    /* Enqueue the message instead of sending directly if:
     * (i) a blocking operation is in progress
     * (ii) there are previous requests already in the queue
     */
    if ((CPA_FALSE == pSessionDesc->nonBlockingOpsInProgress) ||
        (NULL != pSessionDesc->pRequestQueueTail))
    {
        LAC_SPINLOCK(&pSessionDesc->requestQueueLock);

        /* Re-check blockingOpsInProgress and pRequestQueueTail in case either
         * changed before the lock was acquired.  The lock is shared with 
         * the callback context which drains this queue
         */
        if ((CPA_FALSE == pSessionDesc->nonBlockingOpsInProgress) ||
            (NULL != pSessionDesc->pRequestQueueTail))
        {
            /* Enqueue the message and exit */
            /* The FIFO queue is made up of a head and tail pointer.
             * The head pointer points to the first/oldest, entry
             * in the queue, and the tail pointer points to the last/newest
             * entry in the queue
             */

            if (NULL != pSessionDesc->pRequestQueueTail)
            {
                /* Queue is non-empty. Add this request to the list */
                pSessionDesc->pRequestQueueTail->pNext = pRequest;
            }
            else
            {
                /* Queue is empty. Initialise the head pointer as well */
                pSessionDesc->pRequestQueueHead = pRequest;
            }
    
            pSessionDesc->pRequestQueueTail = pRequest;

            /* request is queued, don't send to QAT here */
            enqueued = CPA_TRUE;
        }
        LAC_SPINUNLOCK(&pSessionDesc->requestQueueLock);
    }
   
    if (CPA_FALSE == enqueued)
    {
        /* If we send a partial packet request, set the blockingOpsInProgress
         * flag for the session to indicate that subsequent requests must be
         * queued up until this request completes
         *
         * @assumption
         * If we have got here it means that there were no previous blocking
         * operations in progress and, since multiple partial packet requests
         * on a given session cannot be issued concurrently, there should be
         * no need for a critical section around the following code
         */
        if (CPA_CY_SYM_PACKET_TYPE_FULL != pRequest->pOpData->packetType)
        {
            /* Select blocking operations which this reqest will complete */
            pSessionDesc->nonBlockingOpsInProgress = CPA_FALSE;
        }
        
        /* At this point, we're clear to send the request.  For cipher requests,
         * we need to check if the session IV needs to be updated.  This can
         * only be done when no other partials are in flight for this session,
         * to ensure the cipherPartialOpState buffer in the session descriptor
         * is not currently in use
         */
        if (CPA_TRUE == pRequest->updateSessionIvOnSend)
        {
            if(LAC_CIPHER_IS_ARC4(pSessionDesc->cipherAlgorithm))
            {
                memcpy(pSessionDesc->cipherPartialOpState,
                       pSessionDesc->cipherARC4InitialState,
                       LAC_CIPHER_ARC4_STATE_LEN_BYTES);
            }
            else
            {
                memcpy(pSessionDesc->cipherPartialOpState, 
                       pRequest->pOpData->pIv, 
                       pRequest->pOpData->ivLenInBytes);
            }
        }

        /* Send directly to QAT */
        status = QatComms_MsgSend(
            &(pRequest->qatMsg),
            ICP_ARCH_IF_REQ_QAT_FW_LA,
            pSessionDesc->qatSessionPriority,
            instanceHandle);
    }

    return status;
}

