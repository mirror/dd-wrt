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
 * @file icp_lac_hash_precompute.h   Functions to perform pre-computation of
 * the hash initialiser state for selected authentication algorithms
 * 
 * @ingroup LacHash
 *
 * @description
 *      This header file exports functions to perform hash pre-compute
 *      operations.  This may be used by components outside of LAC. 
 *
 *****************************************************************************/

#ifndef ICP_LAC_HASH_PRECOMPUTE_H
#define ICP_LAC_HASH_PRECOMPUTE_H

#include "cpa.h"
#include "cpa_cy_sym.h"


/**
*******************************************************************************
 * @ingroup LacHash
 *      precomputes for HMAC and AES XCBC/CCM/GCM
 *
 * @description
 *      In this function, one or more requests are sent to the QAT acceleration
 *      engine to get the precomputed result. This function blocks waiting
 *      on the callback for each perform before proceeding. The internal session
 *      is then deregistered. The results are stored in state1 and state2.
 *
 *      NOTE_1: state1 and state2 are treated as working buffers. The memory
 *      allocated must be visible to the accelerator and be the same size as the
 *      block size of the algorithm (even though the precompute result size
 *      may be smaller). e.g SHA384 has a block size of 128 bytes while the
 *      precompute result size is 64 bytes.  For XCBC-MAC, the state2 buffer
 *      must be 48 bytes in size.
 *
 *      NOTE_2: state1 is only used for HMAC precomputes.  For all other 
 *      algorithms, this should be set to NULL.  state2 is used by all
 *      algorithms.
 *
 * @context
 *      This is a synchronous function and it can sleep. It MUST NOT be 
 *      executed in a context that DOES NOT permit sleeping.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in]  hashAlgorithm        Hash Algorithm
 * @param[in]  authModeCtx          Hash Authentication Mode context
 * @param[out] pState1              Pointer to State 1. Refer to Note 1.
 * @param[out] pState2              Pointer to State 2. Refer to Note 2. 
 *
 * @retval CPA_STATUS_SUCCESS        No error
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_RESOURCE       Problem Acquiring system resource
 *
 *****************************************************************************/
CpaStatus
icp_LacHashPrecomputeSynchronous(
    CpaCySymHashAlgorithm hashAlgorithm,
    CpaCySymHashAuthModeSetupData authModeCtx,
    Cpa8U *pState1,
    Cpa8U *pState2);


#endif /*  ICP_LAC_HASH_PRECOMPUTE_H */
