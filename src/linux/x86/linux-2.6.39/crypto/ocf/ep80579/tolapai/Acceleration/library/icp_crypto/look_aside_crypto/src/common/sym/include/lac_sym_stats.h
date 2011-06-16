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
 * @file lac_sym_stats.h
 *
 * @defgroup LacSymCommon Symmetric Common
 * 
 * @ingroup LacSym
 * 
 * Symetric Common consists of common statistics, buffer and partial packet
 * functionality.
 *
 ***************************************************************************/

/**
 ***************************************************************************
 * @defgroup LacSymStats Statistics
 *
 * @ingroup LacSymCommon
 *
 * definitions and prototypes for LAC symmetric statistics.
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


/***************************************************************************/

#ifndef LAC_SYM_STATS_H
#define LAC_SYM_STATS_H

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_sym.h"
#include "cpa_cy_common.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/


/**
*******************************************************************************
 * @ingroup LacSymStats
 *      increment a symmetric statistic
 *
 * @description
 *      Increment the statistics
 *
 * @param statistic  IN The field in the symmetric statistics structure to be
 *                      incremented
 * @param instanceHandle  IN engine Id Number
 *
 * @retval None
 *
 *****************************************************************************/

#define LAC_SYM_STAT_INC(statistic, instanceHandle)\
    LacSym_StatsInc(offsetof(CpaCySymStats, statistic), instanceHandle)

/**
*******************************************************************************
 * @ingroup LacSymStats
 *      initialises the symmetric stats
 *
 * @description
 *      This function initialises the stats array to 0
 *
 * @retval None
 *
 *****************************************************************************/
void
LacSym_StatsInit(void);


/**
*******************************************************************************
 * @ingroup LacSymStats
 *      shutdown the symmetric stats
 *
 * @description
 *      shutdown the symmetric stats
 *
 * @retval None
 *
 *****************************************************************************/
void
LacSym_StatsShutdown(void);


/**
*******************************************************************************
 * @ingroup LacSymStats
 *      Inrement a stat
 *
 * @description
 *      This function incrementes a stat for a specific engine.
 *
 * @param offset     IN  offset of stat field in structure
 * @param instanceHandle  IN  qat Handle
 *
 * @retval None
 *
 *****************************************************************************/
void
LacSym_StatsInc(Cpa32U offset, CpaInstanceHandle instanceHandle);

/**
*******************************************************************************
 * @ingroup LacSymStats
 *      Copy the contents of the statistics structure for an engine
 *
 * @description
 *      This function copies the symmetric statistics structure for a specific
 *      engine into an address supplied as a parameter.
 *
 * @param instanceHandle  IN     engine Id Number
 * @param pSymStats  OUT stats structure to copy the stats for the into
 *
 * @retval None
 *
 *****************************************************************************/
void
LacSym_StatsCopyGet(CpaInstanceHandle instanceHandle,
                    CpaCySymStats * const pSymStats);


/**
*******************************************************************************
 * @ingroup LacSymStats
 *      print the symmetric stats to standard output
 *
 * @description
 *      The statistics for symmetric are printed to standard output.
 *
 * @retval None
 *
 * @see LacSym_StatsCopyGet()
 *
 *****************************************************************************/
void
LacSym_StatsShow(void);

#endif /*LAC_SYM_STATS_H_*/
