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
 * @file lac_sym_stats.c   Implementation of symmetric stats
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
#include "IxOsal.h"
#include "lac_sym_stats.h"

#include "lac_common.h"

#define LAC_SYM_NUM_STATS (sizeof(CpaCySymStats) / sizeof(Cpa32U))

/*
*******************************************************************************
* Static Variables
*******************************************************************************
*/

/* array of atomic stats for symmetric */
STATIC IxOsalAtomic lacSymStatsArr[LAC_SYM_NUM_STATS];


void
LacSym_StatsInit(void)
{
    LAC_OS_BZERO((void *) LAC_CONST_VOLATILE_PTR_CAST(lacSymStatsArr),
        sizeof(lacSymStatsArr));
}

void
LacSym_StatsShutdown(void)
{
    LAC_OS_BZERO(LAC_CONST_VOLATILE_PTR_CAST(lacSymStatsArr),
        sizeof(lacSymStatsArr));
}

void
LacSym_StatsInc(Cpa32U offset, CpaInstanceHandle instanceHandle)
{
    ixOsalAtomicInc(&lacSymStatsArr[ offset / sizeof(Cpa32U) ]);
}

void
LacSym_StatsCopyGet(CpaInstanceHandle instanceHandle,
                    CpaCySymStats * const pSymStats)
{
    int i = 0;
    LAC_ENSURE(CPA_INSTANCE_HANDLE_SINGLE == instanceHandle, 
                                            "invalid handle\n");
    LAC_ENSURE_NOT_NULL(pSymStats);

    for (i = 0; i < LAC_SYM_NUM_STATS; i ++)
    {
        ((Cpa32U *)pSymStats)[i] = ixOsalAtomicGet(&lacSymStatsArr[i]);
    }
}

void
LacSym_StatsShow(void)
{
    CpaCySymStats symStats = {0};

    LacSym_StatsCopyGet(CPA_INSTANCE_HANDLE_SINGLE, &symStats);

    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT, 
               SEPARATOR
               BORDER "              Symmetric Stats               " BORDER"\n"
               SEPARATOR,
               0, 0, 0, 0, 0, 0);

    /* Session Info */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               BORDER " Sessions Initialized:           %10u " BORDER "\n"
               BORDER " Sessions Removed:               %10u " BORDER "\n"
               BORDER " Session Errors:                 %10u " BORDER "\n"  
               SEPARATOR,
               symStats.numSessionsInitialized,
               symStats.numSessionsRemoved,
               symStats.numSessionErrors,
               0, 0, 0);

    /* Session info */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               BORDER " Symmetric Requests:             %10u " BORDER "\n"
               BORDER " Symmetric Request Errors:       %10u " BORDER "\n"
               BORDER " Symmetric Completed:            %10u " BORDER "\n"
               BORDER " Symmetric Completed Errors:     %10u " BORDER "\n"
               BORDER " Symmetric Verify Failures:      %10u " BORDER "\n"
               SEPARATOR,
               symStats.numSymOpRequests,
               symStats.numSymOpRequestErrors,
               symStats.numSymOpCompleted,
               symStats.numSymOpCompletedErrors,
               symStats.numSymOpVerifyFailures,
               0);
}


