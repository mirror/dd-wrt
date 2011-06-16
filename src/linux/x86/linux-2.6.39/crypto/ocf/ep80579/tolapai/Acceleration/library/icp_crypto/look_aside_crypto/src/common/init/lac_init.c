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
 * @file lac_init.c
 *
 * @ingroup LacInit
 *
 * Initialisation and shutdown implementations for LAC
 *
 *****************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_common.h"
#include "IxOsal.h"
#include "qat_comms.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/

#include "icp_asd_cfg.h"
#include "icp_lac_cfg.h"
#include "lac_common.h"
#include "lac_hooks.h"
#include "lac_pke_qat_comms.h"
#include "lac_pke_utils.h"
#include "lac_random.h"
#include "lac_sym_key.h"

#define LAC_SHUTDOWN_RETRY_COUNT     (4)
/**< @ingroup LacInit
 * This value is the number of times we will poll the system to see if there
 * are outstanding requests before we shutdown down */

#define LAC_SHUTDOWN_TIMEOUT_IN_MS   ((40000) / (LAC_SHUTDOWN_RETRY_COUNT))
/**< @ingroup LacInit
 * This indicates the time to wait betweeen polling the current number
 * of requests sent as compared to the responses completed before shutting
 * down the LAC system */



/*
*******************************************************************************
* Static Variables
*******************************************************************************
*/

static IxOsalAtomic lacState = IX_OSAL_ATOMIC_INIT(LAC_COMP_SHUT_DOWN);
/**< @ingroup LacInit
 * State of Look Aside Crypto */
/* @assumption - we're only supporting a single acceleration engine for now.
 * If supporting more than one, the lac init/shutdown state needs to be tracked
 * per engine. We may also need a reference count for LAC.
 */


/*
*******************************************************************************
* Define static function definitions
*******************************************************************************
*/

/**
 ***************************************************************************
 * @ingroup LacInit
 *      Initialise Lac Components
 * 
 * @param[in] lacRandomCacheSize    Random Cache Size
 * @param[in] numSymConcurrentReq   Symmetric concurrent requests
 * @param[in] numAsymConcurrentReq  Asymmetric concurrent requests
 *
 * @retval CPA_STATUS_SUCCESS   Success
 * @return the status of the operation
 * 
 ***************************************************************************/
STATIC CpaStatus
Lac_ComponentsInit(Cpa32U lacRandomCacheSize,
                   Cpa64U numSymConcurrentReq, 
                   Cpa64U numAsymConcurrentReq)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    status = LacMem_InitBuffers(numAsymConcurrentReq);
    LAC_CHECK_STATUS(status);
    
    status = LacSym_Init(numSymConcurrentReq);
    LAC_CHECK_STATUS(status);
    
    
    status = LacSymKey_Init();
    LAC_CHECK_STATUS(status);
    
    status = LacPke_CommsInit(numAsymConcurrentReq);
    LAC_CHECK_STATUS(status);
   
    status = LacDh_Init();
    LAC_CHECK_STATUS(status);

    status = LacRand_InitCacheSize(lacRandomCacheSize);
    LAC_CHECK_STATUS(status);

    status = LacRsa_Init();
    LAC_CHECK_STATUS(status);

    status = LacDsa_Init();
    LAC_CHECK_STATUS(status);

    status = LacPrime_Init(numAsymConcurrentReq);
    LAC_CHECK_STATUS(status);

    status = LacLn_Init();
    LAC_CHECK_STATUS(status);

    return status;
}

/**
 ***************************************************************************
 * @ingroup LacInit
 *      Shutdown Lac Components
 * 
 * @retval CPA_STATUS_SUCCESS   Success 
 * @return CPA_STATUS_FAIL      Failure
 *
 ***************************************************************************/
STATIC CpaStatus
Lac_ComponentsShutdown(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* Attempt best-effort shutdown on all components */

    if (CPA_STATUS_SUCCESS != LacLn_Shutdown())
    {
        status = CPA_STATUS_FAIL;
    }
    
    if (CPA_STATUS_SUCCESS != LacPrime_Shutdown())
    {
        status = CPA_STATUS_FAIL;
    }
 
    if (CPA_STATUS_SUCCESS != LacRsa_Shutdown())
    {
        status = CPA_STATUS_FAIL;
    }

    if (CPA_STATUS_SUCCESS != LacDsa_Shutdown())
    {
        status = CPA_STATUS_FAIL;
    }
                                                                     
    if (CPA_STATUS_SUCCESS != LacDh_Shutdown())
    {
        status = CPA_STATUS_FAIL;
    }

    LacPke_CommsShutdown();

    if (CPA_STATUS_SUCCESS != LacSym_Shutdown())
    {
        status = CPA_STATUS_FAIL;
    }

    if (CPA_STATUS_SUCCESS != LacSymKey_Shutdown())
    {
        status = CPA_STATUS_FAIL;
    }
    
    if (CPA_STATUS_SUCCESS != LacRand_Shutdown())
    {
        status = CPA_STATUS_FAIL;
    }

    LacMem_DestroyBuffers();

    return status;
}

/**
 ***************************************************************************
 * @ingroup LacInit
 *      Check if responses are pending
 * 
 * @param[in] instanceHandle        Instance Handle
 * @param[in,out] pNumRespPending   Indicates if request are pending
 * 
 * @return CPA_STATUS_SUCCESS   Success
 * @return CPA_STATUS_FAIL      Failure
 * 
 ***************************************************************************/
CpaStatus
LacInit_ResponsesPending(CpaInstanceHandle instanceHandle,
              CpaBoolean *pNumRespPending)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa32U lacNumSent = 0;
    Cpa32U lacNumReceived = 0;
    Cpa32U lacNumRetry = 0;
    Cpa32U pkeNumSent = 0;
    Cpa32U pkeNumReceived = 0;
    Cpa32U pkeNumRetry = 0;

    /* Ensure numRespPending is not NULL and initialised to 0 */
    LAC_ENSURE_NOT_NULL(pNumRespPending);
    *pNumRespPending = CPA_TRUE;

    status = QatComms_MsgCountGet(ICP_ARCH_IF_REQ_QAT_FW_LA,
                                  &lacNumSent, &lacNumReceived, &lacNumRetry);

    if (CPA_STATUS_SUCCESS == status)
    {
        status = QatComms_MsgCountGet(ICP_ARCH_IF_REQ_QAT_FW_PKE, 
                                   &pkeNumSent, &pkeNumReceived, &pkeNumRetry);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        *pNumRespPending = ((lacNumSent + pkeNumSent) != 
                            (lacNumReceived + pkeNumReceived));
    }

    return status;
}

/*
*******************************************************************************
* Define public/global function definitions
*******************************************************************************
*/


/**
 ***************************************************************************
 * @ingroup LacInit
 ***************************************************************************/
CpaBoolean
Lac_IsInitialised(void)
{
    if (LAC_COMP_INITIALISED == ixOsalAtomicGet(&lacState))
    {
        return CPA_TRUE;
    }

    return CPA_FALSE;
}

/**
 ***************************************************************************
 * @ingroup LacInit
 ***************************************************************************/
CpaBoolean
Lac_IsRunning(void)
{
    unsigned int currState = ixOsalAtomicGet(&lacState);
    if((LAC_COMP_INITIALISED == currState) ||
       (LAC_COMP_SHUTTING_DOWN == currState))
    {
        return CPA_TRUE;
    }

    return CPA_FALSE;
}


/*******************************************************************/
/**** Init/shutdown entry points for Acceleration System Driver ****/
/*******************************************************************/


/**
 ***************************************************************************
 * @ingroup LacInit
 *      LAC Init from ASD
 ***************************************************************************/
CpaStatus
icp_AsdCfgLacInit(
    CpaInstanceHandle instanceHandle,
    icp_asd_cfg_param_get_cb_func_t getCfgParamFunc)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_asd_cfg_value_t lacRandomCacheSize = 0;
    icp_asd_cfg_value_t numSymConcurrentReq = 0;
    icp_asd_cfg_value_t numAsymConcurrentReq = 0;

    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    LAC_CHECK_NULL_PARAM(getCfgParamFunc);

    if (LAC_COMP_SHUT_DOWN != ixOsalAtomicGet(&lacState))
    {
        LAC_LOG_ERROR("Initialisation Failed due to either the component "
                "being already initialised or failure of a previous "
                "init/shutdown\n");
        return CPA_STATUS_FAIL;
    }

    status = getCfgParamFunc(ICP_ASD_CFG_PARAM_LAC_RANDOM_CACHE_SIZE,
                             &lacRandomCacheSize);
    LAC_CHECK_STATUS(status);

    status = getCfgParamFunc(ICP_ASD_CFG_PARAM_NUM_SYMM_CONCURRENT_REQ,
                             &numSymConcurrentReq);
    LAC_CHECK_STATUS(status);
    
    status = getCfgParamFunc(ICP_ASD_CFG_PARAM_NUM_ASYM_CONCURRENT_REQ,
                             &numAsymConcurrentReq);
    LAC_CHECK_STATUS(status);

    /* change state to be in the process of being initialised */
    ixOsalAtomicSet(LAC_COMP_INITIALISING, &lacState);

    status = Lac_ComponentsInit( lacRandomCacheSize, 
                                numSymConcurrentReq, numAsymConcurrentReq);
    
    if (CPA_STATUS_SUCCESS != status)
    {
        LAC_LOG_ERROR("Failed to initialise LAC components\n");
        /* Attempt to shut-down any components that were initialised */
        (void) Lac_ComponentsShutdown();
        ixOsalAtomicSet(LAC_COMP_SHUT_DOWN, &lacState);
        return CPA_STATUS_FAIL;
    }

    return status;
}


/**
 ***************************************************************************
 * @ingroup LacInit
 *      LAC Start from ASD
 ***************************************************************************/
CpaStatus
icp_AsdCfgLacStart(
    CpaInstanceHandle instanceHandle)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    if (LAC_COMP_INITIALISING != ixOsalAtomicGet(&lacState))
    {
        LAC_LOG_ERROR(
            "icp_AsdCfgLacInit must be called before this function\n");
        return CPA_STATUS_FAIL;
    }

    /* The PKE component does not require a status return value as it cannot
     * fail. All other components can fail and thus return status of the
     * operation.
     */
    status = LacPke_Init();
    LAC_CHECK_STATUS(status);

    /* Initialise random and generate a buffer of random data to use in 
     * synchronous mode. A request to QAT is used to generate the random 
     * numbers so this function can't be called in the init */
    status = LacRand_Init();
    LAC_CHECK_STATUS(status);

    /* change state to initialised */
    ixOsalAtomicSet(LAC_COMP_INITIALISED, &lacState);

    return CPA_STATUS_SUCCESS;
}


/**
 ***************************************************************************
 * @ingroup LacInit
 *      LAC Stop from ASD
 ***************************************************************************/
CpaStatus
icp_AsdCfgLacStop(
    CpaInstanceHandle instanceHandle)
{
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    if (LAC_COMP_INITIALISED != ixOsalAtomicGet(&lacState))
    {
        LAC_LOG_ERROR(
            "icp_AsdCfgLacStart must be called before this function\n");
        return CPA_STATUS_FAIL;
    }

    LacPke_Shutdown();

    /* change state to be in the process of shutting down */
    ixOsalAtomicSet(LAC_COMP_SHUTTING_DOWN, &lacState);

    return CPA_STATUS_SUCCESS;
}


/**
 ***************************************************************************
 * @ingroup LacInit
 *      LAC Shutdown from ASD
 ***************************************************************************/
CpaStatus
icp_AsdCfgLacShutdown(
    CpaInstanceHandle instanceHandle)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    CpaBoolean respPending = CPA_TRUE;
    Cpa32U retryCount = 0;
    Cpa32U lacStateVal = 0;

    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    lacStateVal = ixOsalAtomicGet(&lacState);
    if ((LAC_COMP_SHUTTING_DOWN != lacStateVal) &&
        (LAC_COMP_INITIALISING != lacStateVal))
    {
        LAC_LOG_ERROR("Not in correct state to shutdown\n");
        return CPA_STATUS_FAIL;
    }

    status = LacInit_ResponsesPending(instanceHandle, &respPending);
    
    
    if (CPA_STATUS_SUCCESS == status) 
    {
        if (CPA_TRUE == respPending)
        { 
            LAC_LOG("Pending for outstanding requests");
        }
    }

    while((CPA_TRUE == respPending) && (retryCount < LAC_SHUTDOWN_RETRY_COUNT)
            && (CPA_STATUS_SUCCESS == status))
    {
        /* Sleep As there are still pending requests to be processed */
        ixOsalSleep(LAC_SHUTDOWN_TIMEOUT_IN_MS);
        status = LacInit_ResponsesPending(instanceHandle, &respPending);
        retryCount++;
    }

    if(CPA_TRUE == respPending)
    {
        LAC_LOG_ERROR(
            "Shutting down with requests pending as time out has expired");
    }
    
    /* We will not terminate on an error status as we still need to shutdown
     * as best we can */

    /* Even if there are pending request we will still shutdown. We need to
     * ensure a best effort of responding to all requests */
    status = Lac_ComponentsShutdown();
    LAC_CHECK_STATUS(status);

    /* change state to be shut down */
    ixOsalAtomicSet(LAC_COMP_SHUT_DOWN, &lacState);

    return status;
}

