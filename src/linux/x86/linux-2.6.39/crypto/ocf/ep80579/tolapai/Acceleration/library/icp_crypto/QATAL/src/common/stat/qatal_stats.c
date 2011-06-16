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
 * @file qatal_stats.c   Implementation of stats
 *
 * @ingroup Qatal
 *
 ***************************************************************************/


/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/
#include "cpa.h"
#include "qatal_stats.h"
#include "qatal_mem.h"
#include "cpa_types.h"
#include "IxOsal.h"


    

#define BORDER "|"
/**< @ingroup QATAL
 * seperator used for printing stats to standard output*/


#define SEPARATOR "+--------------------------------------------+\n"
/**< @ingroup QATAL
 * seperator used for printing stats to standard output*/


/*
*******************************************************************************
* Static Variables
*******************************************************************************
*/

/* each item of icp_qatal_stats_t = uint_32 */
#define QATAL_NUM_STATS ((sizeof(icp_qatal_stats_t)) / (sizeof(Cpa32U)))

/* array of stats for Qatal -- All items of stats must be of size unint_32, 
   otherwise recoding needed */
static Cpa32U qatal_Stats_Data[ QATAL_NUM_STATS ] ;

/*
*******************************************************************************
* Functions/Procedures
*******************************************************************************
*/

/*
 * @fn Qatal_StatsInit - initialize the stats memory by cleaning it.
 */
void 
Qatal_StatsInit(void)
{
    /* initialize status block - wipe it */
    QATAL_MEM_SET(qatal_Stats_Data, 0, sizeof(qatal_Stats_Data));
}

/*
 * @fn Qatal_StatsShutdown - wipe stats memory
 */
void 
Qatal_StatsShutdown(void)
{
    /* wipe status block after use */
    QATAL_MEM_SET(qatal_Stats_Data, 0, sizeof(qatal_Stats_Data));
}

/*
 * @fn Qatal_StatsInc - Increment desired stat
 */
void
Qatal_StatsInc(Cpa32U offset)
{
    /* Check if with valid range */
    if (offset < QATAL_NUM_STATS)
    {

        /* increment desired stat */
        qatal_Stats_Data[offset] =  qatal_Stats_Data[offset] + 1;
    }
}

/*
 * @fn Qatal_StatsCopyGet  - Copy current stats and place at data pointed to 
       via pointer
 */
void
Qatal_StatsCopyGet(icp_qatal_stats_t *pQatalStats)
{


   /* is ptr valid */
    if (NULL != pQatalStats )
    {    

        QATAL_MEM_CPY(pQatalStats, qatal_Stats_Data, sizeof(qatal_Stats_Data));

    }
}

/*
 * @fn Qatal_StatsGetAddress - Get address of stats data block
 */
void
Qatal_StatsGetAddress(icp_qatal_stats_t **pQatalStats)
{
 
    /* place ptr to stats data into data pointed to by pointer*/
    if (NULL != pQatalStats)
    {    
        *pQatalStats = (icp_qatal_stats_t *)qatal_Stats_Data;
    }
}

/*
 * @fn Qatal_StatsShow - Display current Stats
 */
void
Qatal_StatsShow(void)
{
    icp_qatal_stats_t QatalStats = {0};

    /* Get stats information */
    Qatal_StatsCopyGet(&QatalStats);

    /* build up data output */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT, 
               SEPARATOR
               BORDER "              QAT Access Layer              " BORDER"\n"
               SEPARATOR
               BORDER " No. of times NRBG Test invoked  %10u " BORDER "\n"
               BORDER " No. of Heartbeat Tests invoked  %10u " BORDER "\n"
               BORDER " No. of STATS test invoked       %10u " BORDER "\n"
               BORDER " No. of Starts of AL invoked     %10u " BORDER "\n",
               QatalStats.totalNRGBTestInvoked,
               QatalStats.totalNumHeartbeatInvoked,
               QatalStats.totalNumStatsinvoked,
               QatalStats.totalNumStartALreq, 
               0, 0);
}

