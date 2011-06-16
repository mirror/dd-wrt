/*
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
 */

/**
 *******************************************************************************
 * @file lac_dh_stats_p.h  Definitions and prototypes for Diffie Hellman stats
 *
 * @ingroup LacDh 
 *
 * @lld_start
 *      In the LAC API the stats fields are defined as Cpa32U but
 *      IxOsalAtomic is the type that the atomic API supports. Therefore we
 *      need to define a structure internally with the same fields as the API
 *      stats structure, but each field must be of type IxOsalAtomic.
 *
 *      - <b>Incrementing Statistics:</b>\n
 *      Atomically increment the statistic on the internal stats structure.
 *
 *      - <b>Providing a copy of the stats back to the user:</b>\n
 *      Use atomicGet to read the atomic variable for each stat field in the
 *      local internal stat structure. These values are saved in structure
 *      (as defined by the LAC API) that the client will provide a pointer
 *      to as a parameter.
 *
 *      - <b>Stats Show:</b>\n
 *      Use atomicGet to read the atomic variables for each field in the local
 *      internal stat structure and print to the screen
 *
 *      - <b>Stats Array:</b>\n
 *      A macro is used to get the offset off the stat in the structure. This
 *      offset is passed to a function which uses it to increment the stat
 *      at that offset.
 *
 * @lld_end
 *
 ***************************************************************************/

/******************************************************************************/

#ifndef _LAC_DH_STATS_P_H_
#define _LAC_DH_STATS_P_H_

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_dh.h"

/*
********************************************************************************
* Include private header files
********************************************************************************
*/

/******************************************************************************/

/**
*******************************************************************************
 * Increment a diffie hellman statistic
 *
 * @description
 *      Increment the statistics
 *
 * @param[in] statistic         The field in the statistics structure to be
 *                              incremented
 * @param[in] instanceHandle    accel handle
 *
 * @retval None
 *
 *****************************************************************************/
#define LAC_DH_STAT_INC(statistic, instanceHandle)\
    LacDh_StatsInc(offsetof(CpaCyDhStats, statistic), instanceHandle)

/**
 *******************************************************************************
 *      This function clears the DH statistics for the given accel handle
 * @retval None
 ******************************************************************************/
void
LacDh_StatsClear(void);

/**
 *******************************************************************************
 *      This function increments the given DH stat
 *
 * @param[in] offset            offset of stat field in structure
 * @param[in] instanceHandle    the acceleration handle whose statistics we are
 *                              dealing with
 * @retval None
 ******************************************************************************/
void
LacDh_StatsInc(Cpa32U offset, CpaInstanceHandle instanceHandle);

/**
 *******************************************************************************
 * @ingroup LacDh
 *      This function prints the stats to standard out.
 * @retval None
 ******************************************************************************/
void
LacDh_StatsShow(CpaInstanceHandle instanceHandle);

#endif /* _LAC_DH_STATS_H_ */

