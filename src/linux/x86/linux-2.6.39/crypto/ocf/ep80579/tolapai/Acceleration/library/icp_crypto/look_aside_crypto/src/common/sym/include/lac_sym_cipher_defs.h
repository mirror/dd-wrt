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
 * @file lac_sym_cipher_defs.h
 *
 * @ingroup LacCipher 
 *
 * @description
 *      This file defines constants for the cipher operations.
 * 
 *****************************************************************************/


/***************************************************************************/

#ifndef LAC_SYM_CIPHER_DEFS_H
#define LAC_SYM_CIPHER_DEFS_H

/* 
******************************************************************************
* Include public/global header files 
******************************************************************************
*/ 

#include "cpa.h"
#include "cpa_cy_sym.h"

/* 
*******************************************************************************
* Include private header files 
*******************************************************************************
*/ 

/***************************************************************************/

/*
 * Constants value for ARC4 algorithm
 */
/* ARC4 algorithm block size */
#define LAC_CIPHER_ARC4_BLOCK_LEN_BYTES        8  
/* ARC4 key matrix size (bytes) */
#define LAC_CIPHER_ARC4_KEY_MATRIX_LEN_BYTES 256  
/* ARC4 256 bytes for key matrix, 2 for i and j and 6 bytes for padding */ 
#define LAC_CIPHER_ARC4_STATE_LEN_BYTES      264  

/* Reserve enough space for max length cipher state
 * (can be IV, counter or RC4 state)  */
#define LAC_CIPHER_STATE_SIZE_MAX  LAC_CIPHER_ARC4_STATE_LEN_BYTES

/* Reserve enough space for max length cipher IV
 * (can be IV or counter, but not RC4 state)  */
#define LAC_CIPHER_IV_SIZE_MAX  ICP_QAT_HW_AES_128_BLK_SZ

/* Reserve enough space for max length cipher key
 * (excluding RC4 key, and F8 mode keys)  */
#define LAC_CIPHER_KEY_SIZE_MAX ICP_QAT_HW_AES_256_KEY_SZ

/*
 * Constants value for NULL algorithm
 */
/* NULL algorithm block size */
#define LAC_CIPHER_NULL_BLOCK_LEN_BYTES        8



/* Macro to check if the Algorithm is AES */
#define LAC_CIPHER_IS_AES(algo)\
    ((algo == CPA_CY_SYM_CIPHER_AES_ECB)||\
    (algo == CPA_CY_SYM_CIPHER_AES_CBC)||\
    (algo == CPA_CY_SYM_CIPHER_AES_CTR)||\
    (algo == CPA_CY_SYM_CIPHER_AES_CCM)||\
    (algo == CPA_CY_SYM_CIPHER_AES_GCM))

/* Macro to check if the Algorithm is DES */        
#define LAC_CIPHER_IS_DES(algo)\
    ((algo == CPA_CY_SYM_CIPHER_DES_ECB) ||\
    (algo == CPA_CY_SYM_CIPHER_DES_CBC))
    
/* Macro to check if the Algorithm is Triple DES */
#define LAC_CIPHER_IS_TRIPLE_DES(algo)\
    ((algo == CPA_CY_SYM_CIPHER_3DES_ECB) ||\
    (algo == CPA_CY_SYM_CIPHER_3DES_CBC)||\
    (algo == CPA_CY_SYM_CIPHER_3DES_CTR))
    
/* Macro to check if the Algorithm is ARC4 */    
#define LAC_CIPHER_IS_ARC4(algo)\
    (algo == CPA_CY_SYM_CIPHER_ARC4)
    
/* Macro to check if the Algorithm is NULL */    
#define LAC_CIPHER_IS_NULL(algo)\
    (algo == CPA_CY_SYM_CIPHER_NULL)
    
/* Macro to check if the Mode is CTR*/    
#define LAC_CIPHER_IS_CTR_MODE(algo)\
    ((algo == CPA_CY_SYM_CIPHER_AES_CTR)||\
    (algo == CPA_CY_SYM_CIPHER_3DES_CTR)||\
    (LAC_CIPHER_IS_CCM(algo))||\
    (LAC_CIPHER_IS_GCM(algo)))
    
/* Macro to check if the Algorithm is ECB */    
#define LAC_CIPHER_IS_ECB_MODE(algo)\
    ((algo == CPA_CY_SYM_CIPHER_AES_ECB)||\
    (algo == CPA_CY_SYM_CIPHER_DES_ECB)||\
    (algo == CPA_CY_SYM_CIPHER_3DES_ECB)||\
    (algo == CPA_CY_SYM_CIPHER_NULL))
   
/* Macro to check if the Algorithm is CBC */    
#define LAC_CIPHER_IS_CBC_MODE(algo)\
    ((algo == CPA_CY_SYM_CIPHER_AES_CBC)||\
    (algo == CPA_CY_SYM_CIPHER_DES_CBC)||\
    (algo == CPA_CY_SYM_CIPHER_3DES_CBC))
    
/* Macro to check if the Algorithm is CCM */    
#define LAC_CIPHER_IS_CCM(algo)\
    (algo == CPA_CY_SYM_CIPHER_AES_CCM)
    
/* Macro to check if the Algorithm is GCM */    
#define LAC_CIPHER_IS_GCM(algo)\
    (algo == CPA_CY_SYM_CIPHER_AES_GCM)
    
#endif /* LAC_CIPHER_DEFS_H */
