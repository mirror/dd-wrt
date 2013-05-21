/**
 * @file IxCryptoAcc_p.h
 *
 * @date July-13-2005
 *
 * @brief Private header file for IxCryptoAcc software component
 *
 *
 * Design Notes:
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */


#ifndef IXCRYPTOACC_P_H
#define IXCRYPTOACC_P_H


/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxCryptoAcc.h"


/*
 * #defines for function return types, etc.
 */
 
/* Inline or Non-Inlined function declaration/definition macro */
#ifdef NO_INLINE_APIS
    #define IX_CRYPTO_INLINE  /* empty define */
#else /* else of ifdef NO_INLINE_APIS */
    #define IX_CRYPTO_INLINE IX_OSAL_INLINE
#endif /* end of ifdef NO_INLINE_APIS */



#define IX_CRYPTO_ACC_DELAY_IN_MS   500    /**< Time taken in task delay */
 
#define IX_CRYPTO_ACC_TIMER_COUNT   100    /**< Timer counter to prevent
                                            * deadlock
                                            */
                                            
#define IX_CRYPTO_ACC_NPEQ_DESC_IV      16  /**< Length of the IV stored 
                                             * in the NPEQ descriptor.
                                             */

#define IX_CRYPTO_MAX_CRYPTO_INFO_BYTES 80  /**< Maximum number of bytes 
                                             * needed for NPE cryptographic 
                                             * parameters structure
                                             */
#define IX_CRYPTO_ACC_CCM_MAX_AAD_LEN   IX_CRYPTO_ACC_CCM_AAD_LEN_384 
                                            /**< This should always be maximum
                                             *  of CCM_AAD_LEN_XXX 
                                             */
                 
/* 
 * NPE crypto hw accelration Operation Opcode definition, by default 
 * the transfer mode = 0 (in-place) and Crypt direction = 0 (decrypt)
 *
 * NPE Crypto Hw Acceleration Operation Opcode bit field definition (pls 
 * refer to IxCryptoNpeOperationMode) for details :
 *
 *      DATH CM0Vb
 *          D : Cryption Direction
 *          A : Hmac Enable        
 *          T : Transfer Mode      
 *          H : Hash Enable
 *          C : Crypt Enable        
 *          M : CCM Enable   
 *          V : Verification enable / Key Generation enable
 */
#define IX_CRYPTO_NPE_OP_HASH_GEN_ICV   0x50 /**< One pass hash operation, 
                                              * ICV generation
                                              * (01T1 0000b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place)
                                              */
#define IX_CRYPTO_NPE_OP_HASH_VER_ICV   0x51 /**< One pass hash operation, 
                                              * ICV verification
                                              * (01T1 0001b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place)
                                              */
#define IX_CRYPTO_NPE_OP_HMAC_GEN_ICV   0x10 /**< Two pass authentication,
                                              * ICV generation
                                              * (00T1 0000b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place)
                                              */
#define IX_CRYPTO_NPE_OP_HMAC_VER_ICV   0x11 /**< Two pass authentication,  
                                              * ICV verification
                                              * (00T1 0001b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place)
                                              */
#define IX_CRYPTO_NPE_OP_CRYPT          0x48 /**< Cryption operation 
                                              * (D1T0 1000b)
                                              * D - Cryption direction 
                                              * (0: Decrypt, 1: Encrypt) 
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place) 
                                              */
#define IX_CRYPTO_NPE_OP_CCM_GEN_MIC    0xCC /**< CCM Encrypt with MIC Gen 
                                              * (11T0 1100b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place) 
                                              */

#define IX_CRYPTO_NPE_OP_CCM_VER_MIC    0x4D /**< CCM Decrypt with MIC Ver 
                                              * (01T0 1101b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place) 
                                              */

#define IX_CRYPTO_NPE_OP_ENCRYPT_GEN_KEY      0xC9
                                            /**< Reverse AES key generation */
#define IX_CRYPTO_NPE_OP_ENCRYPT_HMAC_GEN_ICV 0x98 
                                            /**< Combined service, encrypt and
                                             * forward authentication. 
                                             * (10T1 1000b)
                                             * T - Transfer mode 
                                             * (0: in-place, 1: Non in-place) 
                                             */
#define IX_CRYPTO_NPE_OP_HMAC_VER_ICV_DECRYPT 0x19 
                                            /**< Combined service, reverse 
                                             * authentication and decrypt. 
                                             * (00T1 1001b)
                                             * T - Transfer mode 
                                             * (0: in-place, 1: Non in-place) 
                                             */

#define IX_CRYPTO_NPE_OP_CCM_ENABLE_MASK      0x04 /**< CCM Enable */

#define IX_CRYPTO_NPE_OP_CRYPT_ENABLE_MASK    0x08 /**< Crypt enable mask
                                                    * default is disabled, if
                                                    * OR with this mask, the
                                                    * Crypt enable bit will 
                                                    * be set to 1 and become 
                                                    * ENABLED
                                                    */
#define IX_CRYPTO_NPE_OP_TRANSFER_MODE_MASK   0x20 /**< Transfer mode mask
                                                    * default is in-place, if
                                                    * OR with this mask, the
                                                    * transfer mode bit will
                                                    * be set to 1 and become 
                                                    * non in-place transfer 
                                                    * mode
                                                    */
#define IX_CRYPTO_NPE_OP_HMAC_DISABLE_MASK    0x40 /**< HMAC enable mask
                                                    * default is enabled, if
                                                    * OR with this mask, the
                                                    * HMAC enable bit will be 
                                                    * set to 1 and become 
                                                    * DISABLED
                                                    */
#define IX_CRYPTO_NPE_OP_CRYPT_DIR_MASK       0x80 /**< Crypt direction mask,
                                                    * default is decrypt, if
                                                    * OR with this mask, the
                                                    * Crypt dir bit will be 
                                                    * set to 1 and become 
                                                    * encrypt operation
                                                    */


/* 
 * NPE WEP Operation Opcode definition, by default the transfer mode = 0
 * (in-place)
 *
 * NPE WEP Operation Opcode bit field definition (pls refer to
 * IxCryptoNpeOperationMode) for details :
 *
 *      0000 TCBDb
 *          T : Transfer Mode
 *          C : WEP CRC-32 Enable         
 *          B : Combined Service
 *          D : Cryption Direction 
 */
#define IX_CRYPTO_NPE_OP_WEP_DECRYPT    0x00 /**< Decryption
                                              * (0000 T000b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place)
                                              */
#define IX_CRYPTO_NPE_OP_WEP_ENCRYPT    0x01 /**< Encryption
                                              * (0000 T001b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place)
                                              */
#define IX_CRYPTO_NPE_OP_WEP_VER_ICV    0x04 /**< WEP ICV verification
                                              * (0000 T100b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place) 
                                              */
#define IX_CRYPTO_NPE_OP_WEP_GEN_ICV    0x05 /**< WEP ICV generation
                                              * (0000 T101b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place) 
                                              */
#define IX_CRYPTO_NPE_OP_WEP_VER_ICV_DECRYPT 0x02
                                             /**< Combined service, ICV
                                              * verification and encryption. 
                                              * (0000 T010b)
                                              * T - Transfer mode 
                                              * (0: in-place, 1: Non in-place) 
                                              */
#define IX_CRYPTO_NPE_OP_WEP_ENCRYPT_GEN_ICV 0x03 
                                            /**< Combined service, encryption
                                             * and ICV generation
                                             * (0000 T011b)
                                             * T - Transfer mode 
                                             * (0: in-place, 1: Non in-place) 
                                             */

#define IX_CRYPTO_NPE_OP_WEP_TRANSFER_MODE_MASK 0x08 /**< Transfer mode mask
                                                      * default is in-place, if
                                                      * OR with this mask, the
                                                      * transfer mode bit will
                                                      * be set to 1 and become 
                                                      * non in-place transfer 
                                                      * mode
                                                      */
#define IX_CRYPTO_NPE_OP_WEP_CRC_ENABLE_MASK    0x04 /**< CRC enable mask
                                                      * default is disabled, if
                                                      * OR with this mask, the
                                                      * CRC enable bit will be 
                                                      * set to 1 and become 
                                                      * ENABLED
                                                      */
#define IX_CRYPTO_NPE_OP_WEP_COMBINED_SERVICE_ENABLE_MASK 0x02 
                                                     /**< Combined service enable
                                                      * mask, default is disabled,
                                                      * if OR with this mask, the
                                                      * combined service enable bit 
                                                      * will  be set to 1 and 
                                                      * become ENABLED
                                                      */
#define IX_CRYPTO_NPE_OP_WEP_CRYPT_DIR_MASK     0x01 /**< Crypt direction mask,
                                                      * default is reverse
                                                      * direction, if OR
                                                      * OR with this mask, the
                                                      * Crypt dir bit will be 
                                                      * set to 1 and become 
                                                      * encrypt operation or
                                                      * CRC generation 
                                                      * operation
                                                      */


/*
 * NPE cryptographic parameters structure definition
 */
#define IX_CRYPTO_NPE_CRYPTO_PARAM_CRYPTO_CFG_WORD_LEN  4  
                                                /**< Length of crypt config
                                                 * word in bytes 
                                                 */
#define IX_CRYPTO_NPE_CRYPTO_PARAM_HASH_CFG_WORD_LEN    4  
                                                /**< Length of hash config 
                                                 * word in bytes
                                                 */
#define IX_CRYPTO_NPE_CRYPTO_PARAM_SHA1_CHAIN_VAR_LEN   20 
                                                /**< Length of SHA1 
                                                 * chaining variables
                                                 */
#define IX_CRYPTO_NPE_CRYPTO_PARAM_MD5_CHAIN_VAR_LEN    16 
                                                /**< Length of MD5
                                                 * chaining variables
                                                 */
#define IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH      64 
                                                /**< Length of SHA1 and 
                                                 * MD5 data block
                                                 */
#define IX_CRYPTO_NPE_QDESC_IV_LEN_OFFSET  4  
                                                /**< Length of crypt config
                                                 * word in bytes 
                                                 */

                                                 
#define IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP 0  /**< NPE endian mode,
                                             * is set to Big Endian by 
                                             * default 
                                             */
#define IX_CRYPTO_NPE_CFG_BYTE_SWAP      2  /**< NPE endian setting
                                             * bit 0: bit swap
                                             * bit 1: byte swap
                                             * bit 2: word swap
                                             * If bit-n = 0, don't swap.
                                             * else swap
                                             */
#define IX_CRYPTO_MAX_AUTH_LENGTH       0xFFBF /**< Maximum Auth Length
                                                 * check in the perform
                                                 * module
                                                 */
#define IX_CRYPTO_ACC_INVALID_HASH_DATA_LEN  0 /**< Invalid hash data
                                                * length in bytes. Data
                                                * size must be greater
                                                * than 0
                                                */
#define IX_CRYPTO_ACC_MAX_HASH_DATA_LEN 0xFF77 /**< Maximum hash data
                                                * length in bytes.
                                                */
#define IX_CRYPTO_MAX_AUTH_DIGEST_LEN   20     /**< Maximum digest length 
                                                * allowed, SHA-20 bytes, while
                                                * MD5- 16 bytes. Thus the max 
                                                * is 20 bytes
                                                */
#define IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE  0 /**< Transfer mode
                                                       * is in place
                                                       */
#define IX_CRYPTO_NPE_OP_CRYPT_MODE_IS_DISABLED  0 /**< Crypt mode
                                                    * is disabled
                                                    */
#define IX_CRYPTO_NPE_OP_HMAC_MODE_IS_ENABLED    0 /**< HMAC mode
                                                    * is enabled
                                                    */
#define IX_CRYPTO_NPE_OP_WEP_CRC_MODE_IS_DISABLED 0 /**< WEP CRC mode
                                                     * is disabled
                                                     */
#define IX_CRYPTO_NPE_OP_WEP_COMBINED_SERVICE_IS_DISABLED 0 
                                                    /**< WEP combined service
                                                     * is disabled
                                                     */
#define IX_CRYPTO_NPE_OP_CRYPT_IS_FORWARD         1 /**< Forward mode
                                                     * (encryption)
                                                     */

/*
 * NPE crypto configuration word definition 
 */
#define IX_CRYPTO_NPE_CRYPT_CFG_DES       0   /**< DES algorithm */
#define IX_CRYPTO_NPE_CRYPT_CFG_TDEA2     1   /**< TDEA2 algorithn */
#define IX_CRYPTO_NPE_CRYPT_CFG_TDEA3     2   /**< TDEA3 algorithm */
#define IX_CRYPTO_NPE_CRYPT_CFG_AES       1   /**< AES algorithm */
#define IX_CRYPTO_NPE_CRYPT_CFG_AES128    0   /**< AES algotithm with 
                                               * 128-bit key 
                                               */
#define IX_CRYPTO_NPE_CRYPT_CFG_AES192    1   /**< AES algotithm with 
                                               * 192-bit key 
                                               */
#define IX_CRYPTO_NPE_CRYPT_CFG_AES256    2   /**< AES algotithm with 
                                               * 256-bit key 
                                               */
#define IX_CRYPTO_NPE_CRYPT_CFG_ECB       0   /**< ECB mode */
#define IX_CRYPTO_NPE_CRYPT_CFG_CTR       1   /**< CTR mode */                
#define IX_CRYPTO_NPE_CRYPT_CFG_CBC_ENC   2   /**< CBC encryption mode */
#define IX_CRYPTO_NPE_CRYPT_CFG_CBC_DEC   3   /**< CBC decryption mode */
#define IX_CRYPTO_NPE_CRYPT_CFG_CCM_ENC   4   /**< CCM encryption mode */
#define IX_CRYPTO_NPE_CRYPT_CFG_CCM_DEC   5   /**< CCM decryption mode */
#define IX_CRYPTO_NPE_CRYPT_CFG_DECRYPT   0   /**< Decrypt */
#define IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT   1   /**< Encrypt */


/*
 * NPE hashing configuration word definition 
 */
#define IX_CRYPTO_NPE_HASH_CFG_SHA1       0   /**< SHA1 algorithm */
#define IX_CRYPTO_NPE_HASH_CFG_MD5        1   /**< MD5 algorithm */


/*
 * Common definition used in shifting and masking
 */
#define WORD_IN_BYTES               4           /**< word == 4 bytes */
#define BITS_IN_BYTE                8           /**< 8 bits == 1 byte */
#define MSB_BYTE                    3           /**< Most signficant byte 
                                                 * position in number
                                                 */
#define MASK_8_BIT                  0xFF        /**< 8-bit mask */
#define IX_CRYPTO_NPE_OP_MASK       MASK_8_BIT  /**< NPE operation mask */
#define IX_CRYPTO_Q_DESC_ADDR_MASK  0xFFFFFFFC  /**< Q descriptor address
                                                 * mask, word-aligned address
                                                 * The lower 2-bit of the queue
                                                 * descriptor address will
                                                 * be used as status flag, 
                                                 * since the address is word 
                                                 * aligned.
                                                 */
#define IX_CRYPTO_ACC_REV_AES_KEY_VALID 0x80000000 /* MSB bit (bit-31) set */                                                 
/* MD5 initial chaining variables value needed in chaining variables
 * computation
 * Note: The initial values listed below are algorithm dependent. Please
 *       refer to RFC1321 (The MD5 Message-Digest Algorithm) for details
 *       of message digest generation.
 */
#define MD5_CHAIN_VAR {                         \
                        0x01, 0x23, 0x45, 0x67, \
                        0x89, 0xAB, 0xCD, 0xEF, \
                        0xFE, 0xDC, 0xBA, 0x98, \
                        0x76, 0x54, 0x32, 0x10  \
                      }

/* SHA1 initial chaining variables value needed in chaining variables
 * computation
 * Note: The initial values listed below are algorithm dependent. Please
 *       refer to RFC3174 (US Secure Hash Algorithm 1 (SHA1)) for details
 *       of message digest generation.
 */
#define SHA1_CHAIN_VAR {                        \
                        0x67, 0x45, 0x23, 0x01, \
                        0xEF, 0xCD, 0xAB, 0x89, \
                        0x98, 0xBA, 0xDC, 0xFE, \
                        0x10, 0x32, 0x54, 0x76, \
                        0xC3, 0xD2, 0xE1, 0xF0  \
                       }

/*
 * Common definition used in NPE and PKE hash data padding
 */

#define IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE  0x80 /**< Padding value to be  
                                                 * used in hash data block 
                                                 * padding (first byte of
                                                 * padded length only)
                                                 */

#define IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN 1 /**< Length of first pad 
                                                 * byte in byte
                                                 */
                                                 
#define IX_CRYPTO_ACC_HASH_TOTAL_BIT_HIGH_LEN 4 /**< Length of field for
                                                 * total bit (high 32-bit) 
                                                 * for hash data block
                                                 */
                                                 
#define IX_CRYPTO_ACC_HASH_TOTAL_BIT_LOW_LEN 4 /**< Length of field for
                                                 * total bit (low 32-bit) 
                                                 * for hash data block
                                                 */

#define IX_CRYPTO_ACC_HASH_TOTAL_BIT_LEN \
            (IX_CRYPTO_ACC_HASH_TOTAL_BIT_HIGH_LEN \
             + IX_CRYPTO_ACC_HASH_TOTAL_BIT_LOW_LEN) /**< Length of field for
                                                      * total bit of hash data
                                                      * block 
                                                      */                                                      

#define IX_CRYPTO_ACC_HASH_DATA_REMAINDER \
            (IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH \
             - IX_CRYPTO_ACC_HASH_FIRST_PAD_BYTE_LEN \
             - IX_CRYPTO_ACC_HASH_TOTAL_BIT_LEN)  /**< Remainder value for 
                                                   * hash data block, where
                                                   * zero padding is not 
                                                   * needed
                                                   */

/**
 * Bit location and mask for member inIxCryptoNpeCryptCfgWord struct.
 *
 * typedef struct
 * { 
 *   UINT32 reserved0  : 4;    bit[28:31]
 *   UINT32 desKeyWr   : 3;    bit[25:27]
 *   UINT32 desKeyRd   : 3;    bit[22:24]
 *   UINT32 desDataWr  : 3;    bit[19:21]
 *   UINT32 desDataRd  : 3;    bit[16:18]
 *
 *   UINT32 reserved1  : 1;    bit[15]
 *   UINT32 cipherMode : 3;    bit[12:14]
 *
 *   UINT32 cryptAlgo  : 1;    bit[11]
 *   UINT32 encrypt    : 1;    bit[10]
 *   UINT32 cryptMode  : 2;    bit[8:9]
 *   UINT32 keyLength  : 8;    bit[0:7]
 * } IxCryptoNpeCryptCfgWord;
 */
#define IX_CRYPTO_NPE_CRYPT_CFG_RESERVED0_LOC   28   /**< Crypt Cfg Word 
                                                      * reserved0 Shift Location
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_RESERVED0_MASK  0xf  /**< Crypt Cfg Word 
                                                      * reserved0 Mask */ 

#define IX_CRYPTO_NPE_CRYPT_CFG_DESKEYWR_LOC    25   /**< Crypt Cfg Word 
                                                      * desKeyWr Shift Location
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_DESKEYWR_MASK   0x7  /**< Crypt Cfg Word
                                                      * desKeyWr Mask */ 

#define IX_CRYPTO_NPE_CRYPT_CFG_DESKEYRD_LOC    22   /**< Crypt Cfg Word 
                                                      * desKeyRd Shift Location 
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_DESKEYRD_MASK   0x7  /**< Crypt Cfg Word 
                                                      * desKeyRd Mask */ 
                                                 
#define IX_CRYPTO_NPE_CRYPT_CFG_DESDATAWR_LOC   19   /**< Crypt Cfg Word 
                                                      * desDataWr Shift Location
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_DESDATAWR_MASK  0x7  /**< Crypt Cfg Word
                                                      * desDataWr Mask */ 

#define IX_CRYPTO_NPE_CRYPT_CFG_DESDATARD_LOC   16   /**< Crypt Cfg Word 
                                                      * desDataRd Shift Location
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_DESDATARD_MASK  0x7  /**< Crypt Cfg Word 
                                                      * desDataRd Mask */ 

#define IX_CRYPTO_NPE_CRYPT_CFG_RESERVED1_LOC   15   /**< Crypt Cfg Word 
                                                      * reserved1 Shift Location
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_RESERVED1_MASK  0x3  /**< Crypt Cfg Word
                                                      * reserved1 Mask */ 

#define IX_CRYPTO_NPE_CRYPT_CFG_CIPHERMODE_LOC  12   /**< Crypt Cfg Word 
                                                      * cipherMode Shift Location
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_CIPHERMODE_MASK 0x7  /**< Crypt Cfg Word
                                                      * cipherMode Mask */ 

#define IX_CRYPTO_NPE_CRYPT_CFG_CRYPTALGO_LOC   11   /**< Crypt Cfg Word
                                                      * cryptAlgo Shift Location
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_CRYPTALGO_MASK  0x1  /**< Crypt Cfg Word 
                                                      * cryptAlgo Mask */ 

#define IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT_LOC     10   /**< Crypt Cfg Word 
                                                      * encrypt Shift Location*/ 
#define IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT_MASK    0x1  /**< Crypt Cfg Word 
                                                      * encrypt Mask */ 

#define IX_CRYPTO_NPE_CRYPT_CFG_CRYPTMODE_LOC   8    /**< Crypt Cfg Word 
                                                      * cryptMode Shift Location
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_CRYPTMODE_MASK  0x3  /**< Crypt Cfg Word 
                                                      * cryptMode Mask */ 

#define IX_CRYPTO_NPE_CRYPT_CFG_KEYLENGTH_LOC   0    /**< Crypt Cfg Word 
                                                      * keyLength Shift Location
                                                      */ 
#define IX_CRYPTO_NPE_CRYPT_CFG_KEYLENGTH_MASK  0xff /**< Crypt Cfg Word 
                                                      * keyLength Mask */ 

#define IX_CRYPTO_NPE_CRYPT_PARAM_CIPHER_KEY_START_OFFSET 0x4 /**< Start Offset 
                                                               * in bytes of the
                                                               * cipher keys 
                                                               * in the param 
                                                               * structure.
                                                               */


/**
 * Bit location and mask for member inIxCryptoNpeHashCfgWord struct.
 *
 * typedef struct
 * {
 *   UINT32 chnWr        : 2;  bit[30:31]
 *   UINT32 chnRd        : 2;  bit[28:29]
 *   UINT32 hdWr         : 2;  bit[26:27]
 *   UINT32 hdRd         : 2;  bit[24:25]
 *   UINT32 reserved     : 7;  bit[17:23]
 *   UINT32 hashAlgo     : 1;  bit[16]
 *
 *   UINT32 digestLength : 8;  bit[8:15] 
 *   UINT32 CVLength     : 8;  bit[0:7]
 * } IxCryptoNpeHashCfgWord; 
 */
#define IX_CRYPTO_NPE_HASH_CFG_CHNWR_LOC          30   /**< Crypt Cfg Word
                                                        * chnWr Shift Location
                                                        */ 
#define IX_CRYPTO_NPE_HASH_CFG_CHNWR_MASK         0x3  /**< Crypt Cfg Word
                                                        * chnWr Mask */

#define IX_CRYPTO_NPE_HASH_CFG_CHNRD_LOC          28   /**< Crypt Cfg Word 
                                                        * chnRd Shift Location 
                                                        */ 
#define IX_CRYPTO_NPE_HASH_CFG_CHNRD_MASK         0x3  /**< Crypt Cfg Word 
                                                        * chnRd Mask */ 

#define IX_CRYPTO_NPE_HASH_CFG_HDWR_LOC           26   /**< Crypt Cfg Word 
                                                        * hdWr Shift Location */ 
#define IX_CRYPTO_NPE_HASH_CFG_HDWR_MASK          0x3  /**< Crypt Cfg Word
                                                        * hdWr Mask */  

#define IX_CRYPTO_NPE_HASH_CFG_HDRD_LOC           24   /**< Crypt Cfg Word 
                                                        * hdRd Shift Location */ 
#define IX_CRYPTO_NPE_HASH_CFG_HDRD_MASK          0x3  /**< Crypt Cfg Word 
                                                        * hdnRd Mask */

#define IX_CRYPTO_NPE_HASH_CFG_RESERVED_LOC       17   /**< Crypt Cfg Word 
                                                        * reserved Shift 
                                                        * Location */ 
#define IX_CRYPTO_NPE_HASH_CFG_RESERVED_MASK      0x7f /**< Crypt Cfg Word 
                                                        * reserved Mask */ 

#define IX_CRYPTO_NPE_HASH_CFG_HASHALGO_LOC       16   /**< Crypt Cfg Word 
                                                        * hashAlgo Shift 
                                                        * Location */ 
#define IX_CRYPTO_NPE_HASH_CFG_HASHALGO_MASK      0x1  /**< Crypt Cfg Word
                                                        * hashAlgo Mask */ 

#define IX_CRYPTO_NPE_HASH_CFG_DIGESTLENGTH_LOC   8    /**< Crypt Cfg Word
                                                        * digestLength Shift
                                                        * Location */ 
#define IX_CRYPTO_NPE_HASH_CFG_DIGESTLENGTH_MASK  0xff /**< Crypt Cfg Word 
                                                        * digestLength Mask */  

#define IX_CRYPTO_NPE_HASH_CFG_CVLENGTH_LOC       0    /**< Crypt Cfg Word
                                                        * CVLength Shift
                                                        * Location */ 
#define IX_CRYPTO_NPE_HASH_CFG_CVLENGTH_MASK      0xff /**< Crypt Cfg Word
                                                        * CVLength Mask */ 

/**
 * @struct  IxCryptoNpeOperationMode
 * @brief   Data structure for NPE operation mode in Crypto Context 
 *          Database
 *
 * @note    npeOperation is comprised of fields as below (depends on what
 *          operation is chosen during registration):
 *
 *          Crypto Hw Acceleration NPE Opcode :
 *          ---------------------------------
 *          CryptionDirection : 1;  (0: Decrypt; 1: Encrypt) 
 *          HmacEnable        : 1;  (0: Enabled; 1: Disabled)
 *          TransferMode      : 1;  (0: In-place; 1: Seperate src & dest)
 *          HashEnable        : 1;  (0: Disabled; 1: Enabled)
 *          CryptEnable       : 1;  (0: Disabled; 1: Enabled)
 *          CCMEnable         : 1;  (0: Disabled; 1: Enabled)
 *          Reserved0         : 1;
 *          V_GK              : 1;  { 
 *                                   If HashEnable = 1
 *                                      0: ICV verification disabled
 *                                      1: ICV verification enabled
 *                                   Else
 *                                      0: Do not generate reverse AES key
 *                                      1: Generate reverse AES key
 *                                   }
 *
 *          WEP NPE Opcode :
 *          --------------
 *          Reserved          : 4;
 *          TransferMode      : 1;  (0: In-place; 1: Seperate src & dest)
 *          CrcEnable         : 1;  (0: Disabled; 1: Enabled)
 *          CombinedService   : 1;  (0: Enc or CRC only; 1: Combined Service)
 *          CryptionDirection : 1;  (0: Decrypt; 1: Encrypt) 
 *
 */
typedef struct
{
    UINT32 npeOperation      : 8;  /**< NPE operation opcode */
    UINT32 initLength        : 8;  /**< length of initial configuration 
                                    * information transfer in bytes 
                                    */
    UINT32 reserved0         : 8;
    UINT32 reserved1         : 8;
} IxCryptoNpeOperationMode;



/**
 * @struct  IxCryptoNpeCryptCfgWord
 * @brief   Data structure for NPE cryption configuration word in NPE
 *          cryptographic parameters structure.
 * 
 */
typedef struct
{
    UINT32 reserved0  : 4;
    UINT32 desKeyWr   : 3;  /**< Default is 0 (big endian) */
    UINT32 desKeyRd   : 3;  /**< Default is 0 (big endian) */
    UINT32 desDataWr  : 3;  /**< Default is 0 (big endian) */
    UINT32 desDataRd  : 3;  /**< Default is 0 (big endian) */

    UINT32 reserved1  : 1;
    UINT32 cipherMode : 3;  /**< 0: ECB; 1: CTR; 
                             * 2: CBC-Encode 3: CBC-Decode  
			     * 4: CCM-Encode 5: CCM-Decode
                             */
    UINT32 cryptAlgo  : 1;  /**< 0: DES; 1: AES  */
    UINT32 encrypt    : 1;  /**< 0: Decrypt; 1: Encrypt */
    UINT32 cryptMode  : 2;  /**<
                             *      DES     AES
                             * ------------------------------
                             *  0   DES     128-bit key
                             *  1   TDEA2   192-bit key
                             *  2   TDEA3   256-bit key
                             */
    UINT32 keyLength  : 8;  /**< DES/3DES: 6 words; AES: 4 / 6 / 8 words */
} IxCryptoNpeCryptCfgWord;


/**
 * @struct  IxCryptoNpeHashCfgWord
 * @brief   Data structure for NPE hashing configuration word in NPE
 *          cryptographic parameters structure.  
 *
 */
typedef struct
{
    UINT32 chnWr        : 2;  /**< Default is 0 (big endian) */
    UINT32 chnRd        : 2;  /**< Default is 0 (big endian) */
    UINT32 hdWr         : 2;  /**< Default is 0 (big endian) */
    UINT32 hdRd         : 2;  /**< Default is 0 (big endian) */
    UINT32 reserved     : 7;
    UINT32 hashAlgo     : 1;  /**< 0: SHA1; 1: MD5 */

    UINT32 digestLength : 8;  /**< Digest length in words */
    UINT32 CVLength     : 8;  /**< Chaining variables length in words 
                               * SHA1 - 5
                               * MD5 -  4
                               */
} IxCryptoNpeHashCfgWord;


/**
 * @struct  IxCryptoNpeCryptoParam
 * @brief   Data structure for NPE cryptographic parameters structure in
 *          Crypto Context
 *
 */
typedef struct 
{
    UINT8 npeCryptoInfo[IX_CRYPTO_MAX_CRYPTO_INFO_BYTES];
} IxCryptoNpeCryptoParam;


/**
 * @enum    IxCryptoReqType
 * @brief   Type of service request definition
 *
 */
typedef enum 
{
    IX_CRYPTO_HW_ACCL_REQ = 0,   /**< Crypto perform operation request */
    IX_CRYPTO_WEP_REQ,           /**< WEP perform operation request */
    IX_CRYPTO_REQ_TYPE           /**< Maximum value for type of request,
                                  * this is invalid request type
                                  */
} IxCryptoReqType;


/**
 * @struct  IxCryptoAccCryptoCtx
 * @brief   Data structure for Crypto Context Database
 *
 * @note    SPI (Security Parameter Index) is not needed for crypto context
 *          as CTR counter block construction is client's responsibility.
 *          
 */
typedef struct 
{
    IxCryptoNpeCryptoParam *pNpeCryptoParam;    /**< NPE Crypto Param 
                                                 * structure 
                                                 */
    IxCryptoNpeOperationMode npeOperationMode;  /**< NPE operation */
    UINT32 cipherIvLength;              /**< IV length in bytes */
    UINT32 cipherBlockLength;           /**< cipher block length in bytes */
    UINT32 cipherKeyLength;             /**< Key length in bytes */
    UINT32 aadLen;                      /**< Additional Authentication Data
                                         * Length 
                                         */
    UINT32 digestLength;                /**< digest length in bytes */
    IxCryptoReqType reqType;            /**< Type of request indication.
                                         * Could be WEP service request
                                         * or crypto hw accelerator
                                         * service request
                                         */
    UINT32 validAndKeyId;               /**< Valid bit and Key ID 
                                         * MSB bit set if reverse AES key needs
                                         * to be update into crypto context*/
    IxCryptoAccRegisterCompleteCallback registerCallbackFn;
    IxCryptoAccPerformCompleteCallback performCallbackFn;
    UINT32 reqCount;        /**< Accumulated requests issued*/
    UINT32 reqDoneCount;    /**< Accumulated requests have been done */
    BOOL valid;             /**< TRUE: valid; FALSE: invalid */
} IxCryptoAccCryptoCtx;



/**
 * @struct  IxCryptoAccKeyCryptoParam
 * @brief   Data structure for Crypto Param structure needed for hash key
 *          and reverse AES key generation
 *
 */
typedef struct
{
    IxCryptoNpeCryptoParam *pNpeCryptoParam;    /**< NPE Crypto Param
                                                 * structure
                                                 */
    UINT32 keyLength;                           /**< key length in bytes */
    IxCryptoAccHashKeyGenCompleteCallback hashKeyCallbackFn;
} IxCryptoAccKeyCryptoParam;



/**
 * @enum    IxCryptoNpeOperationStatus
 * @brief   Operation mode definition
 *
 */
typedef enum 
{
    IX_CRYPTO_OP_PERFORM = 0,   /**< Crypto perform operation done */
    IX_CRYPTO_OP_REGISTER,      /**< Crypto register operation done */
    IX_CRYPTO_OP_WAIT,          /**< Crypto register operation, not done
                                 * yet
                                 */
    IX_CRYPTO_OP_HASH_GEN_KEY,  /**< Hash key generation done */
    IX_CRYPTO_OP_WEP_PERFORM    /**< Crypto WEP perform operation done */    
} IxCryptoNpeOperationStatus;


/**
 * @struct  IxCryptoNpeQDesc
 * @brief   Data structure for NPE queue descriptor
 *
 */
typedef struct
{
    IxCryptoNpeOperationMode npeOperationMode;    /**< NPE operation */
    UINT8 IV[IX_CRYPTO_ACC_NPEQ_DESC_IV];      /**< IV for CBC mode or
                                                   * CTR IV for CTR mode
                                                   */
    union 
    {
        UINT32 icvAddr;         /**< Address to store ICV */
        UINT32 revAesKeyAddr;   /**< Address to store Rev AES key */
    } address;
    UINT32 srcMbufAddr;         /**< Source mbuf address */
    UINT32 destMbufAddr;        /**< Destination mbuf address, NULL if 
                                 * tansfer mode is in-place
                                 */
    UINT16 authStartOffset;     /**< Authentication start offset */
    UINT16 authLength;          /**< Authentication data length */
    UINT16 cryptStartOffset;    /**< Cryption start offset */
    UINT16 cryptLength;         /**< Cryption data length */
    UINT32 aadAddr;             /**< Additional Authentication Data Addr
                                 * for CCM mode
                                 */
    UINT32 cryptoCtxNpeAddr;    /**< NPE Crypto Param structure address */
} IxCryptoNpeQDesc;


/**
 * @struct  IxCryptoAccQDescriptor
 * @brief   Data structure for access component  queue descriptor
 *
 */
typedef struct
{
    IxCryptoNpeQDesc    npeQDesc;           /**< NPE Q descriptor */
    UINT32 cryptoCtxId;                     /**< Crypto Context ID */
    union
    {
        UINT32 originalIcvOffset;           /**< ICV offset to restore
                                             * ICV value for authentication
                                             * check operation
                                             */
        UINT32 keyId;                       /**< Key Id */
    } value;
    UINT8 integrityCheckValue[IX_CRYPTO_MAX_AUTH_DIGEST_LEN]; /**< ICV */
    IX_OSAL_MBUF *pSrcMbuf;                 /**< Src Mbuf pointer */
    IX_OSAL_MBUF *pDestMbuf;                /**< Dest Mbuf pointer */
    IxCryptoNpeOperationStatus operStatus;  /**< NPE operation status */
} IxCryptoQDescriptor;


/**
 * @struct  IxCryptoAccStats
 * @brief   Data structure for statistics
 * 
 * @note    Collection of statistics for XScale WEP requests are available 
 *          in DEBUG mode only.
 *
 */
typedef struct
{
    UINT32 cryptoSuccessCounter;    /**< Counter for number of crypto requests 
                                     * completed successfully 
                                     */
    UINT32 cryptoFailCounter;       /**< Counter for number of crypto requests 
                                     * failed 
                                     */
    UINT32 qOverflowCounter;        /**< Counter for number of times 
                                     * Crypto Req queue overflow
                                     */
    UINT32 qUnderflowCounter;       /**< Counter for number of times 
                                     * Crypto Done queue underflow
                                     */
    UINT32 qDescAddrInvalidCounter; /**< Counter for number of times 
                                     * Crypto Q descriptor address received is 
                                     * NULL (invalid)
                                     */   
    UINT32 wepNpeSuccessCounter;    /**< Counter for number of NPE WEP requests 
                                     * completed successfully 
                                     */
    UINT32 wepNpeFailCounter;       /**< Counter for number of NPE WEP requests 
                                     * failed 
                                     */                                                                       
    UINT32 wepNpeQOverflowCounter;  /**< Counter for number of times 
                                     * WEP Req NPE queue overflow
                                     */
    UINT32 wepNpeQUnderflowCounter; /**< Counter for number of times 
                                     * WEP Done NPE queue underflow
                                     */                                     
    UINT32 wepNpeQDescAddrInvalidCounter; 
                                    /**< Counter for number of times NPE
                                     * WEP Q descriptor address received is
                                     * NULL (invalid)
                                     */
    UINT32 wepXScaleSuccessCounter; /**< Counter for number of XScale WEP 
                                     * requests completed successfully 
                                     */
    UINT32 wepXScaleFailCounter;    /**< Counter for number of XScale WEP 
                                     * requests failed 
                                     */  
} IxCryptoAccStats;


/**
 * @struct  IxCryptoNpeBuf
 * @brief   Data structure of buffer. This buffer will be passed to NPE instead
 *          of IX_OSAL_MBUF. This structure is a mapping to the structure 
 *          ix_ne in IX_OSAL_MBUF
 *
 */
typedef struct __IxCryptoNpeBuf
{
    struct __IxCryptoNpeBuf *ixp_next;  /**< Pointer to next buffer */
    UINT16 ixp_len;                     /**< Buffer length */
    UINT16 ixp_pkt_len;                 /**< Packet length */
    INT8   *ixp_data;                   /**< Pointer to data buffer in SDRAM */
    UINT32 reserved0;
    UINT32 reserved1;
    UINT32 reserved2;
    UINT32 reserved3;
    UINT32 reserved4;
} IxCryptoNpeBuf;

/**
 * @enum IxCryptoProcessorType
 *
 * @brief Store the co-processor type existence
 *
 */
typedef enum
{
    IX_CRYPTO_DES_COPROCESSOR = 0,    /**< DES */
    IX_CRYPTO_AES_COPROCESSOR,        /**< AES */
    IX_CRYPTO_HASH_COPROCESSOR,       /**< HASH */
    IX_CRYPTO_COPROCESSOR_TYPE       
} IxCryptoCoProcessorType;
    

#endif /* end of #ifndef IXCRYPTOACC_P_H */
