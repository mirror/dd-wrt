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
 ***************************************************************************
 * @file lac_sym_hash_defs.h 
 *
 * @defgroup LacHashDefs Hash Definitions
 * 
 * @ingroup  LacHash
 *  
 * Constants for hash algorithms
 *
 ***************************************************************************/

#ifndef LAC_SYM_HASH_DEFS_H
#define LAC_SYM_HASH_DEFS_H

/* Constant for MD5 algorithm  */
#define LAC_HASH_MD5_BLOCK_SIZE         64  
/**< @ingroup LacHashDefs  
  * MD5 block size in bytes */
#define LAC_HASH_MD5_DIGEST_SIZE        16  
/**< @ingroup LacHashDefs 
 * MD5 digest length in bytes */
#define LAC_HASH_MD5_STATE_SIZE         16 
/**< @ingroup LacHashDefs 
 * MD5 state size */


/* Constants for SHA1 algorithm  */
#define LAC_HASH_SHA1_BLOCK_SIZE      64  
/**< @ingroup LacHashDefs  
 * SHA1 Block size in bytes */
#define LAC_HASH_SHA1_DIGEST_SIZE     20  
/**< @ingroup LacHashDefs  
 *  SHA1 digest length in bytes */
#define LAC_HASH_SHA1_STATE_SIZE      20  
/**< @ingroup LacHashDefs  
 *  SHA1 state size */


/* Constants for SHA224 algorithm  */
#define LAC_HASH_SHA224_BLOCK_SIZE    64  
/**< @ingroup LacHashDefs  
 *  SHA224 block size in bytes */
#define LAC_HASH_SHA224_DIGEST_SIZE   28  
/**< @ingroup LacHashDefs  
 *  SHA224 digest length in bytes */
#define LAC_HASH_SHA224_STATE_SIZE    32   
/**< @ingroup LacHashDefs  
 * SHA224 state size */


/* Constants for SHA256 algorithm  */
#define LAC_HASH_SHA256_BLOCK_SIZE      64  
/**< @ingroup LacHashDefs  
 *  SHA256 block size in bytes */
#define LAC_HASH_SHA256_DIGEST_SIZE     32  
/**< @ingroup LacHashDefs  
 *  SHA256 digest length */
#define LAC_HASH_SHA256_STATE_SIZE      32  
/**< @ingroup LacHashDefs  
 *  SHA256 state size */


/* Constants for SHA384 algorithm  */
#define LAC_HASH_SHA384_BLOCK_SIZE    128 
/**< @ingroup LacHashDefs  
 *  SHA384 block size in bytes */
#define LAC_HASH_SHA384_DIGEST_SIZE   48  
/**< @ingroup LacHashDefs  
 *  SHA384 digest length in bytes */
#define LAC_HASH_SHA384_STATE_SIZE    64  
/**< @ingroup LacHashDefs  
 *  SHA384 state size */


/* Constants for SHA512 algorithm  */
#define LAC_HASH_SHA512_BLOCK_SIZE    128 
/**< @ingroup LacHashDefs  
 *  SHA512 block size in bytes */
#define LAC_HASH_SHA512_DIGEST_SIZE   64  
/**< @ingroup LacHashDefs  
 *  SHA512 digest length in bytes */
#define LAC_HASH_SHA512_STATE_SIZE    64  
/**< @ingroup LacHashDefs  
 *  SHA512 state size */


/* Constants for XCBC MAC algorithm  */
#define LAC_HASH_XCBC_MAC_BLOCK_SIZE        16
/**< @ingroup LacHashDefs  
 *  XCBC_MAC block size in bytes */
#define LAC_HASH_XCBC_MAC_128_DIGEST_SIZE   16
/**< @ingroup LacHashDefs  
 *  XCBC_MAC_PRF_128 digest length in bytes */


/* constants for AES CCM */
#define LAC_HASH_AES_CCM_BLOCK_SIZE     16
/**< @ingroup LacHashDefs
 *  block size for CBC-MAC part of CCM */
#define LAC_HASH_AES_CCM_DIGEST_SIZE     16
/**< @ingroup LacHashDefs
 *  untruncated size of authentication field */


/* constants for AES GCM */
#define LAC_HASH_AES_GCM_BLOCK_SIZE     16
/**< @ingroup LacHashDefs
 *  block size for Galois Hash 128 part of CCM */
#define LAC_HASH_AES_GCM_DIGEST_SIZE     16
/**< @ingroup LacHashDefs
 *  untruncated size of authentication field */


/* constants for AES GCM ICV allowed sizes */
#define LAC_HASH_AES_GCM_ICV_SIZE_8      8
#define LAC_HASH_AES_GCM_ICV_SIZE_12     12
#define LAC_HASH_AES_GCM_ICV_SIZE_16     16

/* constants for AES CCM ICV allowed sizes */
#define LAC_HASH_AES_CCM_ICV_SIZE_MIN    4
#define LAC_HASH_AES_CCM_ICV_SIZE_MAX    16


/* constants for authentication algorithms */
#define LAC_HASH_IPAD_BYTE  0x36    
/**< @ingroup LacHashDefs  
 *  Ipad Byte */
#define LAC_HASH_OPAD_BYTE  0x5c    
/**< @ingroup LacHashDefs  
 *  Opad Byte */

#define LAC_HASH_IPAD_4_BYTES  0x36363636    
/**< @ingroup LacHashDefs  
 *  Ipad for 4 Bytes */
#define LAC_HASH_OPAD_4_BYTES  0x5c5c5c5c    
/**< @ingroup LacHashDefs  
 *  Opad for 4 Bytes */

#endif /* LAC_SYM_HASH_DEFS_H */
