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
 * @file lac_sym_qat_hash_defs_lookup.h 
 *
 * @defgroup LacSymQatHashDefsLookup  Hash Defs Lookup
 *
 * @ingroup  LacSymQatHash
 *
 * API to be used for the hash defs lookup table.
 *
 *****************************************************************************/

#ifndef LAC_SYM_QAT_HASH_DEFS_LOOKUP_P_H
#define LAC_SYM_QAT_HASH_DEFS_LOOKUP_P_H


#include "cpa.h"
#include "cpa_cy_sym.h"

/**
******************************************************************************
 * @ingroup LacSymQatHashDefsLookup
 *      Starting Hash algorithm     
 * @description
 *      This define points to the first available hash algorithm    
 *****************************************************************************/
#define CPA_CY_HASH_ALG_START    CPA_CY_SYM_HASH_MD5

/**
******************************************************************************
 * @ingroup LacSymQatHashDefsLookup
 *      Finishing Hash algorithm     
 * @description
 *      This define points to the last available hash algorithm    
 *****************************************************************************/
#define CPA_CY_HASH_ALG_END      CPA_CY_SYM_HASH_AES_GCM

/***************************************************************************/

/**
******************************************************************************
 * @ingroup LacSymQatHashDefsLookup
 *      hash algorithm specific structure     
 * @description
 *      This structure contain constants specific to an algorithm.    
 *****************************************************************************/
typedef struct lac_sym_qat_hash_alg_info_s
{
    Cpa32U    digestLength;   /**< Digest length in bytes */
    Cpa32U    blockLength;    /**< Block length in bytes */
    Cpa8U     *initState;     /**< Initialiser state for hash algorithm */
    Cpa32U    stateSize;      /**< size of above state in bytes */
} lac_sym_qat_hash_alg_info_t;


/**
******************************************************************************
 * @ingroup LacSymQatHashDefsLookup
 *      hash qat specific structure
 * @description
 *      This structure contain constants as defined by the QAT for an 
 *      algorithm.
 *****************************************************************************/
typedef struct lac_sym_qat_hash_qat_info_s
{
    Cpa32U    algoEnc;                    /**< QAT Algorithm encoding */
    Cpa32U    authCounter;                /**< Counter value for Auth */
    Cpa32U    state1Length;               /**< QAT state1 length in bytes */
    Cpa32U    state2Length;               /**< QAT state2 length in bytes */
} lac_sym_qat_hash_qat_info_t;


/**
******************************************************************************
 * @ingroup LacSymQatHashDefsLookup
 *      hash defs structure
 * @description
 *      This type contains pointers to the hash algorithm structure and
 *      to the hash qat specific structure
 *****************************************************************************/
typedef struct lac_sym_qat_hash_defs_s
{
    lac_sym_qat_hash_alg_info_t *algInfo;
    /**< pointer to hash info structure */
    lac_sym_qat_hash_qat_info_t *qatInfo;
    /**< pointer to hash QAT info structure */
} lac_sym_qat_hash_defs_t;


/**
*******************************************************************************
 * @ingroup LacSymQatHashDefsLookup
 *      initialise the hash lookup table
 *
 * @description
 *      This function initialises the digest lookup table.
 *
 * @note 
 *      This function does not have a corresponding shutdown function.
 *
 * @return None
 *
 *****************************************************************************/
void
LacSymQat_HashLookupInit(void);


/**
*******************************************************************************
 * @ingroup LacSymQatHashDefsLookup
 *      get hash algorithm specific structure from lookup table
 *
 * @description
 *      This function looks up the hash lookup array for a structure
 *      containing data specific to a hash algorithm. The hashAlgorithm enum
 *      value MUST be in the correct range prior to calling this function.
 *
 * @param[in]  hashAlgorithm     Hash Algorithm
 * @param[out] ppHashAlgInfo     Hash Alg Info structure
 *
 * @return None
 *
 *****************************************************************************/
void
LacSymQat_HashAlgLookupGet(CpaCySymHashAlgorithm hashAlgorithm,
                           lac_sym_qat_hash_alg_info_t **ppHashAlgInfo);


/**
*******************************************************************************
 * @ingroup LacSymQatHashDefsLookup
 *      get hash defintions from lookup table.
 *
 * @description
 *      This function looks up the hash lookup array for a structure
 *      containing data specific to a hash algorithm. This includes both 
 *      algorithm specific info and qat specific infro. The hashAlgorithm enum
 *      value MUST be in the correct range prior to calling this function.
 *
 * @param[in]  hashAlgorithm     Hash Algorithm
 * @param[out] ppHashDefsInfo    Hash Defs structure
 *
 * @return void
 *
 *****************************************************************************/
void
LacSymQat_HashDefsLookupGet(CpaCySymHashAlgorithm hashAlgorithm,
                            lac_sym_qat_hash_defs_t **ppHashDefsInfo);


#endif /* LAC_SYM_QAT_HASH_DEFS_LOOKUP_P_H */
