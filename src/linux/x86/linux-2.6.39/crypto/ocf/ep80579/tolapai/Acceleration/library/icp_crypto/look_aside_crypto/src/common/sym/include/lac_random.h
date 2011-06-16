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
 * @file lac_random.h
 *
 * @defgroup LacSym_Random     Random
 * 
 * @ingroup LacSym
 *
 * This file contains prototypes of random bits/number generator functions
 * 
 * @lld_start
 * @lld_overview 
 * The RNG (Random Number Generator) allows two different types of operation. 
 * An asynchronous operation consisting in sending a DRBG_GET_RANDOM message 
 * to QAT and a synchronous operation.
 * At initialisation time a cache of random numbers is generated to be used
 * in synchronous mode. A system of ping pong buffer is then used to realise 
 * the synchronous operation. While available data is in one of the buffer
 * we give these to the user as required. When there is no more fresh data
 * in this buffer we use the second buffer to give new data and in the same time
 * regenerate the first buffer by sending a message to QAT.
 * An entropy testing is also done by LAC to ensure the randomness of the data.
 * If a problem occurs the DRBG will be disable and a message will be send to
 * QAT to explain the problem.
 * 
 * @lld_dependencies
 * - \ref LacSymQat "QAT Comms": sending messages to request rings.
 * - \ref LacMem "Memory": For memory pool functions
 * 
 * @lld_initialisation
 * - The main symmetric callback function (LacSymQat_SymRespHandler()) is 
 *          registered.  
 * - Each RAND subcomponent populates the Response Handler table with its 
 *          individual response handler for the random bits/number feature. 
 *       This means that when responses come in from QAT the command id for 
 *       DRBG_GET_RANDOM shall be equated with the individual response handler 
 *       for the random bits/number feature (@see LacRand_ProcessCallback()).
 * - All associated statistics are reset.     
 * 
 * @lld_module_algorithms
 *
 * @lld_process_context
 *
 * @locking 
 * Feature has been designed to be thread safe and re-entrant.
 * However the initialisation and shutdown functions are not re-entrant.
 * A spin lock is use to ensure the correctness of the generated random numbers
 * in the synchronous mode and avoiding giving twice the same results to the 
 * user.
 * 
 * @lld_end
 * 
 *****************************************************************************/

/*****************************************************************************/

#ifndef LAC_RANDOM_H
#define LAC_RANDOM_H

/* 
******************************************************************************
* Include public/global header files 
******************************************************************************
*/ 

#include "cpa.h"
#include "cpa_cy_rand.h"
#include "cpa_cy_common.h"
#include "lac_sym.h"

/* 
*******************************************************************************
* Include private header files 
*******************************************************************************
*/ 

/*****************************************************************************/

/**
 ****************************************************************************
 * @ingroup LacSym_Random 
 *      Print the symmetric stats to standard output
 *
 * @description
 *      For each engine this function copies the stats using the function
 *      cpaCyRandQueryStats. It then prints contents of this structure to
 *      standard output
 *
 * @param[in]    instanceHandle         Acceleration Handle / Engine Id
 *
 * @see cpaCyRandQueryStats()
 *
 *****************************************************************************/
void 
LacRand_StatsShow(CpaInstanceHandle instanceHandle);

/**
 ****************************************************************************
 * @ingroup LacSym_Random 
 *      Initialise the size of the cache used for the synchronous random 
 *      generation
 *
 * @description
 *      The size of the cache is initialised to the value of 
 *      ICP_ASD_CFG_PARAM_LAC_RANDOM_CACHE_SIZE (defined in ASD) 
 *
 * @param[in]    lacRandomCacheSize     Size of the cache
 *
 *****************************************************************************/
CpaStatus 
LacRand_InitCacheSize(Cpa32U lacRandomCacheSize);

CpaStatus
LacRand_DisableDrbg(lac_sym_random_cookie_t *pCookie);

void
LacRand_ProcessCallback(icp_qat_fw_la_cmd_id_t commandId,
                        void *pOpaqueData,
                        icp_qat_fw_comn_flags cmnRespFlags);

#endif /* LAC_RANDOM_H */
