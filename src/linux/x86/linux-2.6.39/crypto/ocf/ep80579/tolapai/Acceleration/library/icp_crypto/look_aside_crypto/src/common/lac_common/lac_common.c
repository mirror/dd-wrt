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
 *****************************************************************************
 * @file lac_common.c
 *
 * @ingroup LacCommon
 *****************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_common.h"
#include "lac_hooks.h"
#include "lac_common.h"
#include "lac_mem.h"

/*
*******************************************************************************
* Define public/global function definitions
*******************************************************************************
*/

/**
 ******************************************************************************
 * @ingroup cpaCyCommon
 *****************************************************************************/
CpaStatus
cpaCyGetStatusText(const CpaInstanceHandle instanceHandle,
        CpaStatus errStatus,
        Cpa8S *pStatusText)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    LAC_CHECK_NULL_PARAM(pStatusText);

    switch(errStatus)
    {
        case CPA_STATUS_SUCCESS:
            LAC_COPY_STRING(pStatusText, CPA_STATUS_STR_SUCCESS);
            break;
        case CPA_STATUS_FAIL:
            LAC_COPY_STRING(pStatusText, CPA_STATUS_STR_FAIL);
            break;
        case CPA_STATUS_RETRY:
            LAC_COPY_STRING(pStatusText, CPA_STATUS_STR_RETRY);
            break;
        case CPA_STATUS_RESOURCE:
            LAC_COPY_STRING(pStatusText, CPA_STATUS_STR_RESOURCE);
            break;
        case CPA_STATUS_INVALID_PARAM:
            LAC_COPY_STRING(pStatusText, CPA_STATUS_STR_INVALID_PARAM);
            break;
        case CPA_STATUS_FATAL:
            LAC_COPY_STRING(pStatusText, CPA_STATUS_STR_FATAL);
            break;
        default:
            status = CPA_STATUS_INVALID_PARAM;
            break;
    }

    return status;
}

/**
 ******************************************************************************
 * @ingroup cpaCyCommon
 *****************************************************************************/
CpaStatus
cpaCyStartInstance(CpaInstanceHandle instanceHandle)
{
    return CPA_STATUS_SUCCESS;
}

/**
 ******************************************************************************
 * @ingroup cpaCyCommon
 *****************************************************************************/
CpaStatus
cpaCyStopInstance(CpaInstanceHandle instanceHandle)
{
    return CPA_STATUS_SUCCESS;
}

/**
 ******************************************************************************
 * @ingroup cpaCyCommon
 *****************************************************************************/
CpaStatus
cpaCyInstanceSetNotificationCb( 
    const CpaCyInstanceNotificationCbFunc pInstanceNotificationCb,
    void *pCallbackTag)
{
    return CPA_STATUS_FAIL;
}

/**
 ******************************************************************************
 * @ingroup cpaCyCommon
 *****************************************************************************/
CpaStatus
cpaCyGetNumInstances(Cpa16U *pNumInstances)
{
    LAC_CHECK_NULL_PARAM(pNumInstances);

    *pNumInstances = LAC_QAT_NUM_INSTANCES;

    return CPA_STATUS_SUCCESS;
}

/**
 ******************************************************************************
 * @ingroup cpaCyCommon
 *****************************************************************************/
CpaStatus
cpaCyGetInstances(Cpa16U numInstances,
                  CpaInstanceHandle *pCyInstances)
{
    LAC_CHECK_NULL_PARAM(pCyInstances);

    if (LAC_QAT_NUM_INSTANCES != numInstances)
    {
        LAC_INVALID_PARAM_LOG("numInstances");
        return CPA_STATUS_INVALID_PARAM;
    }

    pCyInstances[0] = CPA_INSTANCE_HANDLE_SINGLE;
    return CPA_STATUS_SUCCESS;
}

/**
 ******************************************************************************
 * @ingroup cpaCyCommon
 *****************************************************************************/
CpaStatus
cpaCyInstanceGetInfo(const CpaInstanceHandle instanceHandle,
                     CpaInstanceInfo *pInstanceInfo)
{
    LAC_CHECK_NULL_PARAM(pInstanceInfo);

    if (CPA_INSTANCE_HANDLE_SINGLE != instanceHandle) 
    {
        LAC_INVALID_PARAM_LOG("instanceHandle");
        return CPA_STATUS_INVALID_PARAM;
    }
    else
    {
        pInstanceInfo->type = LAC_QAT_SERVICES;

        pInstanceInfo->state = ((CPA_TRUE == Lac_IsRunning())?
                                CPA_INSTANCE_STATE_INITIALISED :
                                CPA_INSTANCE_STATE_SHUTDOWN);
        strncpy((char *)pInstanceInfo->name,
                LAC_QAT_INSTANCE_NAME,
                CPA_INSTANCE_MAX_NAME_SIZE_IN_BYTES);

        pInstanceInfo->name[CPA_INSTANCE_MAX_NAME_SIZE_IN_BYTES - 1] = '\0';

        snprintf((char *)pInstanceInfo->version, 
                    CPA_INSTANCE_MAX_NAME_SIZE_IN_BYTES,
                    "%d.%d", CPA_CY_API_VERSION_NUM_MAJOR, 
                    CPA_CY_API_VERSION_NUM_MINOR);
        
        pInstanceInfo->version[CPA_INSTANCE_MAX_VERSION_SIZE_IN_BYTES - 1]
                    = '\0';
    }

    return CPA_STATUS_SUCCESS;
}
