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
 *
 * @file lac_dh_stats.c diffie hellman stats functions
 *
 * @ingroup LacDh

 * @description This file implements functions for Diffie Hellman stats.
 *****************************************************************************/

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

#include "cpa_cy_common.h"
#include "cpa_cy_dh.h"
#include "IxOsal.h"
#include "cpa.h"

#include "lac_common.h"
#include "lac_dh_stats_p.h"

#include "lac_module.h"

/*
********************************************************************************
* Include private header files
********************************************************************************
*/

#define LAC_DH_NUM_STATS (sizeof(CpaCyDhStats) / sizeof(Cpa32U))

/*
********************************************************************************
* Static Variables
********************************************************************************
*/

static IxOsalAtomic lacDhStatsArr[LAC_DH_NUM_STATS];


/*
********************************************************************************
* Define static function definitions
********************************************************************************
*/

/*
********************************************************************************
* Global Variables
********************************************************************************
*/

/*
********************************************************************************
* Define public/global function definitions
********************************************************************************
*/

void
LacDh_StatsClear(void)
{
    LAC_OS_BZERO(
        LAC_CONST_VOLATILE_PTR_CAST(lacDhStatsArr), sizeof(lacDhStatsArr));
}

CpaStatus
cpaCyDhQueryStats(CpaInstanceHandle instanceHandle,
                    CpaCyDhStats *pDhStats)
{
    Cpa32U i = 0;
    LAC_RUNNING_CHECK();
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pDhStats);

    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dh.istat)
    {
        for (i = 0; i < LAC_DH_NUM_STATS; i ++)
        {
            ((Cpa32U *)pDhStats)[i] = ixOsalAtomicGet(&lacDhStatsArr[i]);
        }
    }
    else
    {
        return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}  /* cpaCyDhQueryStats */

void
LacDh_StatsInc(Cpa32U offset, CpaInstanceHandle instanceHandle)
{
    LAC_ASSERT(CPA_INSTANCE_HANDLE_SINGLE == instanceHandle,
        "Invalid accel handle.");
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dh.istat)
    {
        ixOsalAtomicInc(&lacDhStatsArr[ offset / sizeof(Cpa32U) ]);
    }
} /* LacDh_StatIncrement */

void
LacDh_StatsShow(CpaInstanceHandle instanceHandle)
{
    CpaCyDhStats dhStats = {0};

    LAC_ASSERT(CPA_INSTANCE_HANDLE_SINGLE == instanceHandle,
        "Invalid accel handle.");
    (void)cpaCyDhQueryStats(instanceHandle, &dhStats);

    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT, SEPARATOR
        BORDER "            Diffie Hellman Stats            "
        BORDER "\n"
        SEPARATOR, 0, 0, 0, 0, 0, 0);

        /* perform Info */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " DH Phase1 Key Gen Requests:     %10u " BORDER "\n"
        BORDER " DH Phase1 Key Gen Request Err:  %10u " BORDER "\n"
        BORDER " DH Phase1 Key Gen Completed:    %10u " BORDER "\n"
        BORDER " DH Phase1 Key Gen Completed Err:%10u " BORDER "\n"
        SEPARATOR,
        dhStats.numDhPhase1KeyGenRequests,
        dhStats.numDhPhase1KeyGenRequestErrors,
        dhStats.numDhPhase1KeyGenCompleted,
        dhStats.numDhPhase1KeyGenCompletedErrors, 0, 0);

    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " DH Phase2 Key Gen Requests:     %10u " BORDER "\n"
        BORDER " DH Phase2 Key Gen Request Err:  %10u " BORDER "\n"
        BORDER " DH Phase2 Key Gen Completed:    %10u " BORDER "\n"
        BORDER " DH Phase2 Key Gen Completed Err:%10u " BORDER "\n"
        SEPARATOR,
        dhStats.numDhPhase2KeyGenRequests,
        dhStats.numDhPhase2KeyGenRequestErrors,
        dhStats.numDhPhase2KeyGenCompleted,
        dhStats.numDhPhase2KeyGenCompletedErrors, 0, 0);
}

