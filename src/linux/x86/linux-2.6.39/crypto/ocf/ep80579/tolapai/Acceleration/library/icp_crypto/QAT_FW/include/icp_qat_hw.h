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
 *****************************************************************************
 * @file icp_qat_hw.h
 * @defgroup icp_qat_hw_defs ICP QAT HW definitions
 * @ingroup icp_qat_hw
 * @description
 *      This file documents definitions for the QAT HW
 *
 *      It is assumed that all layout described here conform to Big Endian
 *      Mode
 *
 *****************************************************************************/

#ifndef __ICP_QAT_HW_H__
#define __ICP_QAT_HW_H__

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/



/* ========================================================================= */
/*                                                  AUTH SLICE */
/* ========================================================================= */

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Supported Authentication Algorithm types
 * @description
 *      Enumeration which is used to define the authenticate algorithms
 *
 *****************************************************************************/

typedef enum
{
   ICP_QAT_HW_AUTH_ALGO_NULL=0,                    /*!< Null hashing */
   ICP_QAT_HW_AUTH_ALGO_SHA1=1,                    /*!< SHA1 hashing */
   ICP_QAT_HW_AUTH_ALGO_MD5=2,                   /*!< MD5 hashing */
   ICP_QAT_HW_AUTH_ALGO_SHA224=3,                /*!< SHA-224 hashing */
   ICP_QAT_HW_AUTH_ALGO_SHA256=4,                /*!< SHA-256 hashing */
   ICP_QAT_HW_AUTH_ALGO_SHA384=5,                /*!< SHA-384 hashing */
   ICP_QAT_HW_AUTH_ALGO_SHA512=6,                /*!< SHA-512 hashing */
   ICP_QAT_HW_AUTH_ALGO_AES_XCBC_MAC=7,          /*!< AES-XCBC-MAC hashing */
   ICP_QAT_HW_AUTH_ALGO_AES_CBC_MAC=8,             /*!< AES-CBC-MAC hashing */
   ICP_QAT_HW_AUTH_ALGO_AES_F9=9,                /*!< AES F9 hashing */
   ICP_QAT_HW_AUTH_ALGO_GALOIS_128=10,             /*!< Galois 128 bit hashing */
   ICP_QAT_HW_AUTH_ALGO_GALOIS_64=11,             /*!< Galois 64 hashing */
   ICP_QAT_HW_AUTH_ALGO_KASUMI_F9=12,             /*!< Kasumi F9 hashing */
   ICP_QAT_HW_AUTH_ALGO_DELIMITER=13             /**< Delimiter type */
} icp_qat_hw_auth_algo_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Definition of the supported Authentication modes
 * @description
 *      Enumeration which is used to define the authentication slice modes.
 *      The concept of modes is very specific to the QAT implementation. Its
 *      main use is differentiate how the algorithms are used i.e. mode0 SHA1
 *      will configure the QAT Auth Slice to do plain SHA1 hashing while mode1
 *      configures it todo SHA1 HMAC with precomputes and mode2 sets up the
 *      slice todo SHA1 HMAC with no precomputes (uses key directly)
 *
 * @Note
 *      Only some algorithms are valid in some of the modes. If you dont know
 *      what you are doing then refer back to the EAS
 *
 *****************************************************************************/

typedef enum
{
   ICP_QAT_HW_AUTH_MODE0=0,                    /*!< QAT Auth Mode0 configuration */
   ICP_QAT_HW_AUTH_MODE1=1,                    /*!< QAT Auth Mode1 configuration */
   ICP_QAT_HW_AUTH_MODE2=2,                    /*!< QAT AuthMode2 configuration */
   ICP_QAT_HW_AUTH_MODE_DELIMITER=3          /**< Delimiter type */
} icp_qat_hw_auth_mode_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Auth configuration structure
 *
 * @description
 *      Definition of the format of the authentication slice configuration
 *
 *****************************************************************************/
typedef struct icp_qat_hw_auth_config_s
{
   uint32_t config;
   /**< Configuration used for setting up the slice */

   uint32_t reserved;
   /**< Reserved */
} icp_qat_hw_auth_config_t;

/* Private defines */
#define QAT_AUTH_MODE_BITPOS            28
/**< @ingroup icp_qat_hw_defs
 * Starting bit position for indicating the Auth mode */

#define    QAT_AUTH_MODE_MASK              0xF
/**< @ingroup icp_qat_hw_defs
 * Four bit mask used for determing the Auth mode */

#define    QAT_AUTH_ALGO_BITPOS            24
/**< @ingroup icp_qat_hw_defs
 * Starting bit position for indicating the Auth Algo  */

#define QAT_AUTH_ALGO_MASK              0xF
/**< @ingroup icp_qat_hw_defs
 * Four bit mask used for determining the Auth algo */

#define    QAT_AUTH_CMP_BITPOS             16
/**< @ingroup icp_qat_hw_defs
 * Starting bit position for indicating the Auth Compare */

#define QAT_AUTH_CMP_MASK               0x7F
/**< @ingroup icp_qat_hw_defs
 * Seven bit mask used for determing the Auth Compare. */

/**
 ******************************************************************************
 * @ingroup icp_qat_hw_defs
 *
 * @description
 *      Returns the configuration word for the auth slice based on the inputs
 *      of mode, algorithm type and compare length. Note for the digest
 *      generation case the compare length is a dont care value. Also if the
 *      client will be doing the digest validation the compare_length will not
 *      be used
 *
 * @param mode      Authentication mode to use
 * @param algo      Auth Algorithm to use
 * @param cmp_len   The length of the digest if the QAT is to the check
 *
 *****************************************************************************/
#define ICP_QAT_HW_AUTH_CONFIG_BUILD(mode,algo,cmp_len)   \
            ((((mode) & QAT_AUTH_MODE_MASK) << QAT_AUTH_MODE_BITPOS ) | \
             (((algo) & QAT_AUTH_ALGO_MASK) << QAT_AUTH_ALGO_BITPOS ) | \
             (((cmp_len) & QAT_AUTH_CMP_MASK) << QAT_AUTH_CMP_BITPOS ) )

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Auth Counter structure
 *
 * @description
 *      32 bit counter that tracks the number of data bytes passed through
 *      the slice. This is used by the padding logic for some algorithms. Note
 *      only the upper 32 bits are set.
 *
 *****************************************************************************/
typedef struct icp_qat_hw_auth_counter_s
{
   uint32_t counter;
   /**< Counter value */
   uint32_t reserved;
   /**< Reserved */
} icp_qat_hw_auth_counter_t;

/* Private variables */
#define QAT_AUTH_COUNT_MASK         0xFFFFFFFF
/**< @ingroup icp_qat_hw_defs
 * Thirty two bit mask used for determining the Auth count */

#define QAT_AUTH_COUNT_BITPOS      0
/**< @ingroup icp_qat_hw_defs
 * Starting bit position indicating the Auth count.  */

/**
 ******************************************************************************
 * @ingroup icp_qat_hw_defs
 *
 * @description
 *      Macro to build the auth counter quad word
 *
 * @param val      Counter value to set
 *
 *****************************************************************************/
#define ICP_QAT_HW_AUTH_COUNT_BUILD(val)                        \
      (((val) & QAT_AUTH_COUNT_MASK) << QAT_AUTH_COUNT_BITPOS)

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Definition of the common auth parameters
 * @description
 *      This part of the configuration is constant for each service
 *
 *****************************************************************************/
typedef struct icp_qat_hw_auth_setup_s
{
   icp_qat_hw_auth_config_t auth_config;
   /**< Configuration word for the auth slice */
   icp_qat_hw_auth_counter_t auth_counter;
    /**< Auth counter value for this request */
} icp_qat_hw_auth_setup_t;

/* ************************************************************************* */
/* ************************************************************************* */

#define QAT_HW_DEFAULT_ALIGNMENT            8
#define QAT_HW_ROUND_UP(val, n)                (((val) + ((n)-1)) & (~(n-1)))

/* State1 */
#define ICP_QAT_HW_NULL_STATE1_SZ            32
/**< @ingroup icp_qat_hw_defs
 * State1 block size for NULL hashing */
#define ICP_QAT_HW_MD5_STATE1_SZ            16
/**< @ingroup icp_qat_hw_defs
 * State1 block size for MD5 */
#define ICP_QAT_HW_SHA1_STATE1_SZ            20
/**< @ingroup icp_qat_hw_defs
 * Define the state1 block size for SHA1 - Note that for the QAT HW the state
 * is rounded to the nearest 8 byte multiple */
#define ICP_QAT_HW_SHA224_STATE1_SZ         32
/**< @ingroup icp_qat_hw_defs
 * State1 block size for SHA24 */
#define ICP_QAT_HW_SHA256_STATE1_SZ         32
/**< @ingroup icp_qat_hw_defs
 * State1 block size for SHA256 */
#define ICP_QAT_HW_SHA384_STATE1_SZ         64
/**< @ingroup icp_qat_hw_defs
 * State1 block size for SHA512 */
#define ICP_QAT_HW_SHA512_STATE1_SZ         64
/**< @ingroup icp_qat_hw_defs
 * State1 block size for XCBC */
#define ICP_QAT_HW_AES_XCBC_MAC_STATE1      16
/**< @ingroup icp_qat_hw_defs
 * State1 block size for CBC */
#define ICP_QAT_HW_AES_CBC_MAC_STATE1        16
/**< @ingroup icp_qat_hw_defs
 * State1 block size for AES F9 */
#define ICP_QAT_HW_AES_F9_STATE1            32
/**< @ingroup icp_qat_hw_defs
 * State1 block size for Kasumi F9 */
#define ICP_QAT_HW_KASUMI_F9_STATE1         16
/**< @ingroup icp_qat_hw_defs
 * State1 block size for Galois128 */
#define ICP_QAT_HW_GALOIS_128_STATE1        16

/* State2 */
#define ICP_QAT_HW_NULL_STATE2_SZ            32
/**< @ingroup icp_qat_hw_defs
 * State2 block size for NULL hashing */
#define ICP_QAT_HW_MD5_STATE2_SZ            16
/**< @ingroup icp_qat_hw_defs
 * State2 block size for MD5 */
#define ICP_QAT_HW_SHA1_STATE2_SZ            20
/**< @ingroup icp_qat_hw_defs
 * State2 block size for SHA1 - Note that for the QAT HW the state  is rounded
 * to the nearest 8 byte multiple */
#define ICP_QAT_HW_SHA224_STATE2_SZ         32
/**< @ingroup icp_qat_hw_defs
 * State2 block size for SHA224 */
#define ICP_QAT_HW_SHA256_STATE2_SZ         32
/**< @ingroup icp_qat_hw_defs
 * State2 block size for SHA256 */
#define ICP_QAT_HW_SHA384_STATE2_SZ         64
/**< @ingroup icp_qat_hw_defs
 * State2 block size for SHA384 */
#define ICP_QAT_HW_SHA512_STATE2_SZ         64
/**< @ingroup icp_qat_hw_defs
 * State2 block size for SHA512 */
#define ICP_QAT_HW_AES_XCBC_MAC_KEY_SZ      16
/**< @ingroup icp_qat_hw_defs
 * State2 block size for XCBC */
#define ICP_QAT_HW_AES_CBC_MAC_KEY_SZ        16
/**< @ingroup icp_qat_hw_defs
 * State2 block size for CBC */
#define ICP_QAT_HW_AES_CCM_CBC_E_CTR0_SZ    16
/**< @ingroup icp_qat_hw_defs
 * State2 block size for AES Encrypted Counter 0 */
#define ICP_QAT_HW_F9_IK_SZ                    16
/**< @ingroup icp_qat_hw_defs
 * State2 block size for F9 IK */
#define ICP_QAT_HW_F9_FK_SZ                    16
/**< @ingroup icp_qat_hw_defs
 * State2 block size for F9 FK */
#define ICP_QAT_HW_GALOIS_H_SZ                16
/**< @ingroup icp_qat_hw_defs
 * State2 block size for Galois Multiplier H */
#define ICP_QAT_HW_GALOIS_LEN_A_SZ            8
/**< @ingroup icp_qat_hw_defs
 * State2 block size for Galois AAD length */
#define ICP_QAT_HW_GALOIS_E_CTR0_SZ         16
/**< @ingroup icp_qat_hw_defs
 * State2 block size for Galois Encrypted Counter 0 */

/* ************************************************************************* */
/* ************************************************************************* */

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Definition of SHA512 auth algorithm processing struct
 * @description
 *      This structs described the parameters to pass to the slice for
 *      configuring it for SHA512 processing. This is the largest possible
 *      setup block for authentication
 *
 *****************************************************************************/
typedef struct icp_qat_hw_auth_sha512_s
{
   icp_qat_hw_auth_setup_t inner_setup;
   /**< Inner loop configuration word for the slice */

   uint8_t   state1[ICP_QAT_HW_SHA512_STATE1_SZ];
   /**< Slice state1 variable */

   icp_qat_hw_auth_setup_t outer_setup;
   /**< Outer configuration word for the slice */

   uint8_t   state2[ICP_QAT_HW_SHA512_STATE2_SZ];
   /**< Slice state2 variable */

} icp_qat_hw_auth_sha512_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Supported hardware authentication algorithms
 * @description
 *      Common grouping of the auth algorithm types supported by the QAT
 *
 *****************************************************************************/
typedef union icp_qat_hw_auth_algo_blk_u
{
   icp_qat_hw_auth_sha512_t sha512;
   /**< SHA512 Hashing */

} icp_qat_hw_auth_algo_blk_t;

#define ICP_QAT_HW_GALOIS_LEN_A_BITPOS      0
/**< @ingroup icp_qat_hw_defs
 * Bit position of the 32 bit A value in the 64 bit A configuration sent to
 * the QAT */

#define ICP_QAT_HW_GALOIS_LEN_A_MASK      0xFFFFFFFF
/**< @ingroup icp_qat_hw_defs
 * Mask value for A value */

/* ========================================================================= */
/*                                                  BULK SLICE */
/* ========================================================================= */

/* To be defined */

/* ========================================================================= */
/*                                                CIPHER SLICE */
/* ========================================================================= */

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Definition of the supported Cipher Algorithm types
 * @description
 *      Enumeration used to define the cipher algorithms
 *
 *****************************************************************************/

typedef enum
{
   ICP_QAT_HW_CIPHER_ALGO_NULL=0,                /*!< Null ciphering */
   ICP_QAT_HW_CIPHER_ALGO_DES=1,                /*!< DES ciphering */
   ICP_QAT_HW_CIPHER_ALGO_3DES=2,                /*!< 3DES ciphering */
   ICP_QAT_HW_CIPHER_ALGO_AES128=3,             /*!< AES-128 ciphering */
   ICP_QAT_HW_CIPHER_ALGO_AES192=4,             /*!< AES-192 ciphering */
   ICP_QAT_HW_CIPHER_ALGO_AES256=5,             /*!< AES-256 ciphering */
   ICP_QAT_HW_CIPHER_ALGO_RC4=6,                /*!< RC4 ciphering */
   ICP_QAT_HW_CIPHER_ALGO_KASUMI=7,             /*!< Kasumi */
   ICP_QAT_HW_CIPHER_DELIMITER=8                /**< Delimiter type */
} icp_qat_hw_cipher_algo_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Definition of the supported cipher modes of operation
 * @description
 *      Enumeration used to define the cipher slice modes.
 *
 * @Note
 *      Only some algorithms are valid in some of the modes. If you dont know
 *      what you are doing then refer back to the EAS
 *
 *****************************************************************************/

typedef enum
{
   ICP_QAT_HW_CIPHER_ECB_MODE=0,                /*!< ECB mode */
   ICP_QAT_HW_CIPHER_CBC_MODE=1,                /*!< CBC more */
   ICP_QAT_HW_CIPHER_CTR_MODE=2,                /*!< CTR mode */
   ICP_QAT_HW_CIPHER_F8_MODE=3,                    /*!< F8 mode */
   ICP_QAT_HW_CIPHER_MODE_DELIMITER=4             /**< Delimiter type */
} icp_qat_hw_cipher_mode_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Cipher Configuration Struct
 *
 * @description
 *      Configuration data used for setting up the QAT Cipher Slice
 *
 *****************************************************************************/

typedef struct icp_qat_hw_cipher_config_s
{
   uint32_t val;
   /**< Cipher slice configuration */

   uint32_t reserved;
   /**< Reserved */
} icp_qat_hw_cipher_config_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Definition of the cipher direction
 * @description
 *      Enumeration which is used to define the cipher direction to apply
 *
 *****************************************************************************/

typedef enum
{
   /*!< Flag to indicate that encryption is required */
    ICP_QAT_HW_CIPHER_ENCRYPT=0,
   /*!< Flag to indicate that decryption is required */
    ICP_QAT_HW_CIPHER_DECRYPT=1,

} icp_qat_hw_cipher_dir_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Definition of the cipher key conversion modes
 * @description
 *      Enumeration which is used to define if cipher key conversion is needed
 *
 *****************************************************************************/

typedef enum
{
    /*!< Flag to indicate that no key convert is required */
   ICP_QAT_HW_CIPHER_NO_CONVERT=0,
    /*!< Flag to indicate that key conversion is required */
   ICP_QAT_HW_CIPHER_KEY_CONVERT=1,
} icp_qat_hw_cipher_convert_t;

/* Private defines */
#define QAT_CIPHER_MODE_BITPOS           28
/**< @ingroup icp_qat_hw_defs
 * Define for the cipher mode bit position */

#define QAT_CIPHER_MODE_MASK            0xF
/**< @ingroup icp_qat_hw_defs
 * Define for the cipher mode mask (four bits) */

#define QAT_CIPHER_ALGO_BITPOS           24
/**< @ingroup icp_qat_hw_defs
 * Define for the cipher algo bit position */

#define QAT_CIPHER_ALGO_MASK            0xF
/**< @ingroup icp_qat_hw_defs
 * Define for the cipher algo mask (four bits) */

#define QAT_CIPHER_CONVERT_BITPOS        17
/**< @ingroup icp_qat_hw_defs
 * Define the cipher convert key bit position */

#define QAT_CIPHER_CONVERT_MASK         0x1
/**< @ingroup icp_qat_hw_defs
 * Define for the cipher convert key mask (one bit)*/

#define QAT_CIPHER_DIR_BITPOS            16
/**< @ingroup icp_qat_hw_defs
 * Define for the cipher direction bit position */

#define QAT_CIPHER_DIR_MASK             0x1
/**< @ingroup icp_qat_hw_defs
 * Define for the cipher direction mask (one bit) */

#define QAT_CIPHER_MODE_F8_KEY_SZ_MULT    2
/**< @ingroup icp_qat_hw_defs
 * Define for the cipher mode F8 key size */

/**
 ******************************************************************************
 * @ingroup icp_qat_hw_defs
 *
 * @description
 *      Build the cipher configuration field
 *
 * @param mode      Cipher Mode to use
 * @param algo      Cipher Algorithm to use
 * @param convert   Specify if the key is to be converted
 * @param dir      Specify the cipher direction either encrypt or decrypt
 *
 *****************************************************************************/
#define ICP_QAT_HW_CIPHER_CONFIG_BUILD(mode,algo,convert,dir)               \
         ((((mode) & QAT_CIPHER_MODE_MASK) << QAT_CIPHER_MODE_BITPOS )    | \
          (((algo) & QAT_CIPHER_ALGO_MASK) << QAT_CIPHER_ALGO_BITPOS )    | \
          (((convert) & QAT_CIPHER_CONVERT_MASK) <<                         \
                                            QAT_CIPHER_CONVERT_BITPOS )   | \
          (((dir) & QAT_CIPHER_DIR_MASK) << QAT_CIPHER_DIR_BITPOS ) )

#define ICP_QAT_HW_DES_BLK_SZ          8
/**< @ingroup icp_qat_hw_defs
 * Define the block size for DES.
 * This used as either the size of the IV or CTR input value */
#define ICP_QAT_HW_3DES_BLK_SZ         8
/**< @ingroup icp_qat_hw_defs
 * Define the processing block size for 3DES */
#define ICP_QAT_HW_NULL_BLK_SZ         8
/**< @ingroup icp_qat_hw_defs
 * Define the processing block size for NULL */
#define ICP_QAT_HW_AES_128_BLK_SZ      16
/**< @ingroup icp_qat_hw_defs
 * Define the processing block size for AES 128 */
#define ICP_QAT_HW_AES_192_BLK_SZ      16
/**< @ingroup icp_qat_hw_defs
 * Define the processing block size for AES 192 */
#define ICP_QAT_HW_AES_256_BLK_SZ      16
/**< @ingroup icp_qat_hw_defs
 * Define the processing block size for AES 256 */
#define ICP_QAT_HW_KASUMI_BLK_SZ       8
/**< @ingroup icp_qat_hw_defs
 * Define the processing block size for kasumi */
#define ICP_QAT_HW_DES_KEY_SZ          8
/**< @ingroup icp_qat_hw_defs
 * Define the key size for DES */
#define ICP_QAT_HW_3DES_KEY_SZ         24
/**< @ingroup icp_qat_hw_defs
 * Define the key size for 3DES */
#define ICP_QAT_HW_AES_128_KEY_SZ      16
/**< @ingroup icp_qat_hw_defs
 * Define the key size for AES128 */
#define ICP_QAT_HW_AES_192_KEY_SZ      24
/**< @ingroup icp_qat_hw_defs
 * Define the key size for AES192 */
#define ICP_QAT_HW_AES_256_KEY_SZ      32
/**< @ingroup icp_qat_hw_defs
 * Define the key size for AES256 */
#define ICP_QAT_HW_AES_128_F8_KEY_SZ   (ICP_QAT_HW_AES_128_KEY_SZ * \
                                        QAT_CIPHER_MODE_F8_KEY_SZ_MULT)
/**< @ingroup icp_qat_hw_defs
 * Define the key size for AES128 F8 */
#define ICP_QAT_HW_AES_192_F8_KEY_SZ   (ICP_QAT_HW_AES_192_KEY_SZ * \
                                        QAT_CIPHER_MODE_F8_KEY_SZ_MULT)
/**< @ingroup icp_qat_hw_defs
 * Define the key size for AES192 F8 */
#define ICP_QAT_HW_AES_256_F8_KEY_SZ   (ICP_QAT_HW_AES_256_KEY_SZ * \
                                        QAT_CIPHER_MODE_F8_KEY_SZ_MULT)
/**< @ingroup icp_qat_hw_defs
 * Define the key size for AES256 F8 */
#define ICP_QAT_HW_KASUMI_KEY_SZ       16
/**< @ingroup icp_qat_hw_defs
 * Define the key size for Kasumi */
#define ICP_QAT_HW_KASUMI_F8_KEY_SZ    (ICP_QAT_HW_KASUMI_KEY_SZ * \
                                        QAT_CIPHER_MODE_F8_KEY_SZ_MULT)
/**< @ingroup icp_qat_hw_defs
 * Define the key size for Kasumi F8 */
#define ICP_QAT_HW_RC4_KEY_SZ          256
/**< @ingroup icp_qat_hw_defs
 * Define the key size for ARC4 */


/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Definition of AES-256 F8 cipher algorithm processing struct
 * @description
 *      This structs described the parameters to pass to the slice for
 *      configuring it for AES-256 F8 processing
 *
 *****************************************************************************/
typedef struct icp_qat_hw_cipher_aes256_f8_s
{
   icp_qat_hw_cipher_config_t cipher_config;
   /**< Cipher configuration word for the slice set to
    * AES-256 and the F8 mode */

   uint8_t key[ICP_QAT_HW_AES_256_F8_KEY_SZ];
   /**< Cipher key */

} icp_qat_hw_cipher_aes256_f8_t;


/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Supported hardware cipher algorithms
 * @description
 *      Common grouping of the cipher algorithm types supported by the QAT.
 *      This is the largest possible cipher setup block size
 *
 *****************************************************************************/
typedef union icp_qat_hw_cipher_algo_blk_u
{

   icp_qat_hw_cipher_aes256_f8_t aes256_f8;
   /**< AES-256 F8 Cipher */

} icp_qat_hw_cipher_algo_blk_t;


/* ========================================================================= */
/*                                                  NRBG SLICE */
/* ========================================================================= */

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Definition of the supported NRBG configuration modes
 * @description
 *      Enumeration used to define the NRBG modes. Used by clients when
 *      configuring the NRBG for use
 *
 *****************************************************************************/

typedef enum
{
   ICP_QAT_HW_NRBG_DBL=0,                /*!< NRBG Disabled mode */
   ICP_QAT_HW_NRBG_NHT=1,                /*!< NRBG Normal Health Test mode */
   ICP_QAT_HW_NRBG_KAT=4,                /*!< NRBG Known Answer Test mode */
   ICP_QAT_HW_NRBG_DELIMITER=8             /**< Delimiter type */
} icp_qat_hw_nrbg_cfg_mode_t;

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Definition of the supported NRBG KAT (known answer test) modes
 * @description
 *      Enumeration which is used to define the NRBG KAT modes. Used by clients
 *      when configuring the NRBG for testing
 *
 *****************************************************************************/

typedef enum
{
   ICP_QAT_HW_NRBG_NEG_0=0,             /*!< NRBG Neg Zero Test */
   ICP_QAT_HW_NRBG_NEG_1=1,             /*!< NRBG Neg One Test */
   ICP_QAT_HW_NRBG_POS=2,                /*!< NRBG POS Test */
   ICP_QAT_HW_NRBG_POS_VNC=3,             /*!< NRBG POS VNC Test */
   ICP_QAT_HW_NRBG_KAT_DELIMITER=4      /**< Delimiter type */
} icp_qat_hw_nrbg_kat_mode_t;

/* Private Defines */
#define QAT_HW_NRBG_CFG_RSVD_SZ              7
/**< @ingroup icp_qat_hw_defs
 * NRBG config reserved size in bytes. */

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      NRBG mode configuration structure.
 *
 * @description
 *      Definition of the format of the NRBG slice configuration. Used
 *      internally by the QAT FW for configuration of the KAT unit or the
 *      NRBG depending on the slice command i.e. either a set_slice_config or
 *      slice_wr_KAT_type
 *
 *****************************************************************************/

typedef struct icp_qat_hw_nrbg_config_s
{
   uint8_t config;
   /**< Configuration used for setting up the NRBG slice */

   uint8_t reserved[QAT_HW_NRBG_CFG_RSVD_SZ];
   /**< Reserved */
} icp_qat_hw_nrbg_config_t;

/* Private Defines*/
#define QAT_NRBG_CONFIG_MODE_MASK          0x7
/**< @ingroup icp_qat_hw_defs
 * Mask for the NRBG configuration mode. (Three bits) */

#define QAT_NRBG_CONFIG_MODE_BITPOS       5
/**< @ingroup icp_qat_hw_defs
 * NRBG configuration mode bit positions start */

#define QAT_NRBG_KAT_MODE_MASK             0x3
/**< @ingroup icp_qat_hw_defs
 * Mask of two bits for the NRBG known answer test mode */

#define QAT_NRBG_KAT_MODE_BITPOS          6
/**< @ingroup icp_qat_hw_defs
 * NRBG known answer test mode bit positions start */

/**
 ******************************************************************************
 * @ingroup icp_qat_hw_defs
 *
 * @description
 *      Build the configuration byte for the NRBG slice based on the mode
 *
 * @param mode   Configuration mode parameter
 *
 *****************************************************************************/
#define ICP_QAT_HW_NRBG_CONFIG_MODE_BUILD(mode)   \
      (((mode) & QAT_NRBG_CONFIG_MODE_MASK) << QAT_NRBG_CONFIG_MODE_BITPOS)

/**
 ******************************************************************************
 * @ingroup icp_qat_hw_defs
 *
 * @description
 *      Build the configuration byte for the NRBG KAT based on the mode
 *
 * @param mode   Configuration mode parameter
 *
 *****************************************************************************/
#define ICP_QAT_HW_NRBG_KAT_MODE_BUILD(mode)   \
      ((((mode) & QAT_NRBG_KAT_MODE_MASK) << QAT_NRBG_KAT_MODE_BITPOS))

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      NRBG test status structure.
 *
 * @description
 *      Definition of the format of the NRBG slice test status structure. Used
 *      internally by the QAT FW.
 *
 *****************************************************************************/

typedef struct icp_qat_hw_nrbg_test_status_s
{

    uint32_t status;
   /**< Status used for setting up the NRBG slice */

    uint32_t fail_count;
   /**< Comparator fail count */
} icp_qat_hw_nrbg_test_status_t;

#define ICP_QAT_HW_NRBG_TEST_NO_FAILURES              1
/**< @ingroup icp_qat_hw_defs
 * Flag to indicate that there were no Test Failures */

#define ICP_QAT_HW_NRBG_TEST_FAILURES_FOUND          0
/**< @ingroup icp_qat_hw_defs
 * Flag to indicate that there were Test Failures */

#define ICP_QAT_HW_NRBG_TEST_STATUS_VALID              1
/**< @ingroup icp_qat_hw_defs
 * Flag to indicate that there is no valid Test output */

#define ICP_QAT_HW_NRBG_TEST_STATUS_INVALID          0
/**< @ingroup icp_qat_hw_defs
 * Flag to indicate that the Test output is still invalid */

/* Private defines */
#define QAT_NRBG_TEST_FAILURE_FLAG_MASK                0x1
/**< @ingroup icp_qat_hw_defs
 * Mask of one bit used to determine the NRBG Test pass/fail */

#define QAT_NRBG_TEST_FAILURE_FLAG_BITPOS             28
/**< @ingroup icp_qat_hw_defs
 * Flag position to indicate that the NRBG Test status is pass of fail */

#define QAT_NRBG_TEST_STATUS_MASK                      0x1
/**< @ingroup icp_qat_hw_defs
 * Mask of one bit used to determine the NRBG Test staus */

#define QAT_NRBG_TEST_STATUS_BITPOS                   25
/**< @ingroup icp_qat_hw_defs
 * Flag position to indicate the NRBG Test status */

/**
 ******************************************************************************
 * @ingroup icp_qat_hw_defs
 *
 * @description
 *      Extract the fail bit for the NRBG slice
 *
 * @param status   NRBG status value
 *
 *****************************************************************************/

#define ICP_QAT_HW_NRBG_FAIL_FLAG_GET(status)         \
   (((status) >> QAT_NRBG_TEST_FAILURE_FLAG_BITPOS) &    \
                                              QAT_NRBG_TEST_FAILURE_FLAG_MASK)

/**
 ******************************************************************************
 * @ingroup icp_qat_hw_defs
 *
 * @description
 *      Extract the status valid bit for the NRBG slice
 *
 * @param status   NRBG status value
 *
 *****************************************************************************/
#define ICP_QAT_HW_NRBG_STATUS_VALID_GET(status)   \
   (((status) >> QAT_NRBG_TEST_STATUS_BITPOS) & QAT_NRBG_TEST_STATUS_MASK)

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      NRBG entropy counters
 *
 * @description
 *      Definition of the format of the NRBG entropy counters. Used internally
 *      by the QAT FW.
 *
 *****************************************************************************/

typedef struct icp_qat_hw_nrbg_entropy_counts_s
{
   uint64_t raw_ones_count;
   /**< Count of raw ones of entropy */

   uint64_t raw_zeros_count;
   /**< Count of raw zeros of entropy */

   uint64_t cond_ones_count;
   /**< Count of conditioned ones entropy */

   uint64_t cond_zeros_count;
   /**< Count of conditioned zeros entropy */
} icp_qat_hw_nrbg_entropy_counts_t;

/* Private defines */
#define QAT_HW_NRBG_ENTROPY_STS_RSVD_SZ      4
/**< @ingroup icp_qat_hw_defs
 * NRBG entropy status reserved size in bytes */

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      NRBG entropy available status.
 *
 * @description
 *      Definition of the format of the NRBG slice entropy status available.
 *      struct. Used internally by the QAT FW.
 *
 *****************************************************************************/
typedef struct icp_qat_hw_nrbg_entropy_status_s
{
   uint32_t status;
   /**< Entropy status in the NRBG */

    uint8_t reserved[QAT_HW_NRBG_ENTROPY_STS_RSVD_SZ];
   /**< Reserved */
} icp_qat_hw_nrbg_entropy_status_t;

#define ICP_QAT_HW_NRBG_ENTROPY_AVAIL              1
/**< @ingroup icp_qat_hw_defs
 * Flag indicating that entropy data is available in the QAT NRBG slice */

#define ICP_QAT_HW_NRBG_ENTROPY_NOT_AVAIL       0
/**< @ingroup icp_qat_hw_defs
 * Flag indicating that no entropy data is available in the QAT NRBG slice */

/* Private defines */
#define QAT_NRBG_ENTROPY_STATUS_MASK             1
/**< @ingroup icp_qat_hw_defs
 * Mask of one bit used to determine the NRBG Entropy status*/

#define QAT_NRBG_ENTROPY_STATUS_BITPOS             24
/**< @ingroup icp_qat_hw_defs
 * Starting bit position for NRBG Entropy status. */

/**
 ******************************************************************************
 * @ingroup icp_qat_hw_defs
 *
 * @description
 *      Extract the entropy available status bit
 *
 * @param status   NRBG status value
 *
 *****************************************************************************/
#define ICP_QAT_HW_NRBG_ENTROPY_STATUS_GET(status)   \
   (((status) >> QAT_NRBG_ENTROPY_STATUS_BITPOS) & \
                                                QAT_NRBG_ENTROPY_STATUS_MASK)

/**
 *****************************************************************************
 * @ingroup icp_qat_hw_defs
 *      Entropy seed data
 *
 * @description
 *      This type is used for the definition of the entropy generated by a read
 *      of the NRBG slice
 *
 *****************************************************************************/
typedef uint64_t icp_qat_hw_nrbg_entropy;

/* ========================================================================= */
/*                                                STORAGE SLICE */
/* ========================================================================= */

/* To be defined */

#endif /* __ICP_QAT_HW_H__ */

