/**
 * @file IxCryptoAccCryptoRegister.c
 *
 * @date October-03-2002
 *
 * @brief  Source file for Crypto Register Module
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


/*
 * System defined include files required.
 */


/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxCryptoAcc_p.h"
#include "IxCryptoAccCryptoRegister_p.h"
#include "IxCryptoAccQAccess_p.h"
#include "IxCryptoAccCCDMgmt_p.h"
#include "IxCryptoAccDescMgmt_p.h"
#include "IxCryptoAccUtilities_p.h"
#include "IxFeatureCtrl.h"

#define IX_CRYPTO_WORD_ALIGNED   0  /**< word-aligned flag */
            
           
/**
 * @fn      ixCryptoRegisterCipherAlgoRegister
 * @brief   Register cipher algorithm for a Crypto Context
 *
 */
IxCryptoAccStatus
ixCryptoRegisterCipherAlgoRegister (
    UINT32 cryptoCtxId,
    IxCryptoAccCtx *pAccCtx,
    UINT32 *npeCryptoParamIndex,
    IxCryptoNpeOperationStatus operStatus)
{
    union
    {
        IxCryptoNpeCryptCfgWord cryptCfgWord;
        UINT32 npeCryptCfgWord;
    } cfgWord;

    UINT32 i;
    UINT32 fwdAesKeyAddr;
    UINT32 npeCryptoConfigWord; 


    /* Initialize Crypt Config word to zero */
    cfgWord.npeCryptCfgWord = 0;

    /* Cipher algorithm switch case options
     * For each case of the cipher algorithm supported, the cipher key
     * length, cipher mode, cipher block length and IV length will be 
     * checked against the cipher algorithm standard and features supported
     * in the access component. If the parameters provided are not 
     * compliant to either one of the source mentioned above, the error 
     * status will be reported to client, and the registration operation is
     * considered failed. NPE Crypt Config word will be constructed too.
     */
    switch (pAccCtx->cipherCtx.cipherAlgo)
    {    
        /* DES algorithm checking, notes: the code for DES fall through
         * to 3DES algorithm
         */
        case IX_CRYPTO_ACC_CIPHER_DES:
        
        /* Triple-DES algorithm checking */
        case IX_CRYPTO_ACC_CIPHER_3DES:       
            /* Check cryption direction */
            if ((IX_CRYPTO_ACC_OP_ENCRYPT == pAccCtx->operation) ||
                (IX_CRYPTO_ACC_OP_ENCRYPT_AUTH == pAccCtx->operation))
            {
                cfgWord.cryptCfgWord.encrypt = IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT;
            }
            else /* decryption */
            {
                cfgWord.cryptCfgWord.encrypt = IX_CRYPTO_NPE_CRYPT_CFG_DECRYPT;
            } /* end of if-else (pAccCtx->operation) */

            /* Check cipher mode */
            if ((IX_CRYPTO_ACC_MODE_ECB == pAccCtx->cipherCtx.cipherMode) ||
                (IX_CRYPTO_ACC_MODE_CBC == pAccCtx->cipherCtx.cipherMode))
            {
                /* Set cipher algorithm */
                cfgWord.cryptCfgWord.cryptAlgo = IX_CRYPTO_NPE_CRYPT_CFG_DES;
                
                /* Check and set key length */
                if (IX_CRYPTO_ACC_CIPHER_DES
                    == pAccCtx->cipherCtx.cipherAlgo)
                {
                    /* Set cipher algorithm type in NPE cryptCfgWord */
                    cfgWord.cryptCfgWord.cryptMode 
                        = IX_CRYPTO_NPE_CRYPT_CFG_DES;
                    
                    if (IX_CRYPTO_ACC_DES_KEY_64 
                        == pAccCtx->cipherCtx.cipherKeyLen)
                    {
                        /* Regardless of the true key length, DES and 3DES 
                         * operation must be set to 3DES key length, 
                         * 192-bit. NPE requirement.
                         */
                        cfgWord.cryptCfgWord.keyLength 
                            = (IX_CRYPTO_ACC_3DES_KEY_192 /
                                  WORD_IN_BYTES);
                    }
                    else /* invalid key length */
                    {
                        return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_KEY_LEN;   
                    } /* end of if-else (pAccCtx->cipherCtx.cipherKeyLen) */
                }
                else /* IX_CRYPTO_ACC_CIPHER_3DES */
                {
                    /* Set cipher algorithm type in NPE cryptCfgWord */
                    cfgWord.cryptCfgWord.cryptMode 
                        = IX_CRYPTO_NPE_CRYPT_CFG_TDEA3;

                    if (IX_CRYPTO_ACC_3DES_KEY_192 
                        == pAccCtx->cipherCtx.cipherKeyLen)
                    {
                         /* Regardless of the true key length, DES and 3DES 
                          * operation must be set to 3DES key length, 
                          * 192-bit. NPE requirement.
                          */
                        cfgWord.cryptCfgWord.keyLength 
                            = (IX_CRYPTO_ACC_3DES_KEY_192 /
                                  WORD_IN_BYTES);
                    }
                    else /* invalid key length */
                    {
                        return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_KEY_LEN;   
                    } /* end of if-else (pAccCtx->cipherCtx.cipherKeyLen) */
                } /* end of if-else (pAccCtx->cipherCtx.cipherAlgo) */

                /* Store the cipherKeyLength assuming it is correct */
                ixCryptoCtx[cryptoCtxId].cipherKeyLength 
                                = IX_CRYPTO_ACC_3DES_KEY_192;
    
                /* Check and set cipher block length */
                if (IX_CRYPTO_ACC_DES_BLOCK_64 
                    == pAccCtx->cipherCtx.cipherBlockLen)
                {
                    ixCryptoCtx[cryptoCtxId].cipherBlockLength 
                        = IX_CRYPTO_ACC_DES_BLOCK_64;
                }
                else /* invalid cipher block length */
                {
                    return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_BLOCK_LEN;   
                } /* end of if-else (pAccCtx->cipherCtx.cipherBlockLen) */
                
                /* Check cipher mode */
                if (IX_CRYPTO_ACC_MODE_CBC 
                    == pAccCtx->cipherCtx.cipherMode)
                {
                    /* Check and set IV length */
                    if (IX_CRYPTO_ACC_DES_IV_64 
                        == pAccCtx->cipherCtx.cipherInitialVectorLen)
                    {
                        ixCryptoCtx[cryptoCtxId].cipherIvLength 
                            = IX_CRYPTO_ACC_DES_IV_64;
                    }
                    else /* invalid IV length */
                    {
                        return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_IV_LEN;   
                    } /* end of if-else 
                       * (pAccCtx->cipherCtx.cipherInitialVectorLen) 
                       */
                    
                    if (IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT 
                        == cfgWord.cryptCfgWord.encrypt)
                    {
                        /* CBC encryption mode */
                        cfgWord.cryptCfgWord.cipherMode 
                            = IX_CRYPTO_NPE_CRYPT_CFG_CBC_ENC;
                    }
                    else /* decryption */
                    {
                        /* CBC decryption mode */
                        cfgWord.cryptCfgWord.cipherMode
                            = IX_CRYPTO_NPE_CRYPT_CFG_CBC_DEC;                    
                    } /* end of if-else (cfgWord.cryptCfgWord.Encrypt) */
                }
                else /* != IX_CRYPTO_ACC_MODE_CBC */
                {
                     /* ECB cipher mode */
                     cfgWord.cryptCfgWord.cipherMode 
                            = IX_CRYPTO_NPE_CRYPT_CFG_ECB;
                } /* end of if-else (pAccCtx->cipherCtx.cipherMode) */
                                
                /* Configure endian mode of NPE, by default is Big Endian
                 * no swapping is required
                 */
                cfgWord.cryptCfgWord.desKeyWr 
                    = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
                cfgWord.cryptCfgWord.desKeyRd 
                    = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;     
                cfgWord.cryptCfgWord.desDataWr
                    = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
                cfgWord.cryptCfgWord.desDataRd 
                    = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
                
                /* Convert to NPE Format (Big Endian) */
                ixCryptoUtilNpeCryptCfgGenerate (
                    &npeCryptoConfigWord, 
                    &cfgWord.cryptCfgWord);

                /* Copy Crypt Config word into NPE Crypto Param structure */
                for ( i = 0; 
                    i < IX_CRYPTO_NPE_CRYPTO_PARAM_CRYPTO_CFG_WORD_LEN; 
                    i++)
                {
                    /* Shift Crypt Config word into the array of UINT8, 
                     * most significant byte is shifted into the array with
                     * smaller index.
                     * Ex : 
                     *   -------------------
                     *  | b0 | b1 | b2 | b3 |
                     *   -------------------
                     *  array[n] = b0
                     *  array[n+1] = b1
                     */
                    (ixCryptoCtx[cryptoCtxId].pNpeCryptoParam)->
                        npeCryptoInfo[*npeCryptoParamIndex + i]
                            = ((npeCryptoConfigWord >> ((MSB_BYTE - i) 
                                  * BITS_IN_BYTE)) & MASK_8_BIT);
                } /* end of for (i) */

                /* Log NPE Crypt Config word in debugging mode */
                IX_CRYPTO_ACC_LOG (
                    IX_OSAL_LOG_LVL_MESSAGE,
                    IX_OSAL_LOG_DEV_STDOUT,
                    "Crypt Cfg word is %x\n",
                    npeCryptoConfigWord,
                    0, 0, 0, 0, 0);
    
                /* Update Npe Crypto Param index to move the pointer in 
                 * NPE Crypto param structure to point end of the Crypto  
                 * Config word the cfg word being copied into the structure
                 */
                *npeCryptoParamIndex += 
                    IX_CRYPTO_NPE_CRYPTO_PARAM_CRYPTO_CFG_WORD_LEN;
              
                /* Copy Cryption key into NPE Crypto Param structure */
                ixOsalMemCopy (
                    &((ixCryptoCtx[cryptoCtxId].pNpeCryptoParam)->
                    npeCryptoInfo[*npeCryptoParamIndex]),
                    pAccCtx->cipherCtx.key.cipherKey,
                    pAccCtx->cipherCtx.cipherKeyLen);
                
                /* Update Npe Crypto Param index to move the pointer in NPE
                 * NPE Crypto param structure to point end of the key after 
                 * the key being copied into the structure
                 * Notes: Regardless of the true key length, DES and 3DES 
                 *        operation must be set to 3DES key length, 
                 *        192-bit. NPE requirement.
                 */
                *npeCryptoParamIndex += IX_CRYPTO_ACC_3DES_KEY_192;
                
                /* Flush NPE Crypto Param structure from cache into SDRAM */
                IX_CRYPTO_DATA_CACHE_FLUSH (
                    ixCryptoCtx[cryptoCtxId].pNpeCryptoParam,
                    IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE);
                
                /* Set Req Type to crypto Hw Accelerator service in crypto 
                 * context
                 */
                ixCryptoCtx[cryptoCtxId].reqType = IX_CRYPTO_HW_ACCL_REQ;
                
                return IX_CRYPTO_ACC_STATUS_SUCCESS;
            }
            else /* CipherMode != CBC or ECB */
            {
                return IX_CRYPTO_ACC_STATUS_CIPHER_MODE_NOT_SUPPORTED;
            } /* end of if-else (pAccCtx->cipherCtx.cipherMode) */
               
        /* AES algorithm */
        case IX_CRYPTO_ACC_CIPHER_AES:
           
            /* Check cryption direction */
            if ((IX_CRYPTO_ACC_OP_ENCRYPT == pAccCtx->operation) ||
                (IX_CRYPTO_ACC_OP_ENCRYPT_AUTH == pAccCtx->operation))
            {
                cfgWord.cryptCfgWord.encrypt = IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT;
            }
            else /* decryption */
            {
                cfgWord.cryptCfgWord.encrypt = IX_CRYPTO_NPE_CRYPT_CFG_DECRYPT;
            } /* end of if-else (pAccCtx->operation) */

            /* Set cipher algorithm */
            cfgWord.cryptCfgWord.cryptAlgo = IX_CRYPTO_NPE_CRYPT_CFG_AES;

            /* Check key length and set crypt mode*/
            switch (pAccCtx->cipherCtx.cipherKeyLen)
            {
                /* AES with 128-bit key */
                case IX_CRYPTO_ACC_AES_KEY_128:
                    
                    /* Set cipher algorithm type in NPE cryptCfgWord */
                    cfgWord.cryptCfgWord.cryptMode 
                        = IX_CRYPTO_NPE_CRYPT_CFG_AES128;

                    break;
                    
                /* AES with 192-bit key */
                case IX_CRYPTO_ACC_AES_KEY_192:

                    /* Set cipher algorithm type in NPE cryptCfgWord */
                    cfgWord.cryptCfgWord.cryptMode 
                        = IX_CRYPTO_NPE_CRYPT_CFG_AES192;
                    
                    break;

                /* AES with 256-bit key */
                case IX_CRYPTO_ACC_AES_KEY_256:

                    /* Set cipher algorithm type in NPE cryptCfgWord */
                    cfgWord.cryptCfgWord.cryptMode 
                        = IX_CRYPTO_NPE_CRYPT_CFG_AES256;
                    
                    break;

                /* invalid key length */
                default:
                    return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_KEY_LEN;
                 
            } /* end of switch case (pAccCtx->cipherCtx.cipherKeyLen) */
 
            /* Store cipherKeyLength assuming it is correct */
            ixCryptoCtx[cryptoCtxId].cipherKeyLength =
                                pAccCtx->cipherCtx.cipherKeyLen;
            
            /* Set key length */
            cfgWord.cryptCfgWord.keyLength 
                = (pAccCtx->cipherCtx.cipherKeyLen /
                      WORD_IN_BYTES);

            /* Check and set cipher block length */
            if (IX_CRYPTO_ACC_AES_BLOCK_128
                == pAccCtx->cipherCtx.cipherBlockLen)
            {
                ixCryptoCtx[cryptoCtxId].cipherBlockLength 
                    = IX_CRYPTO_ACC_AES_BLOCK_128;
            }
            else /* invalid cipher block length */
            {
                return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_BLOCK_LEN;   
            } /* end of if-else (pAccCtx->cipherCtx.cipherBlockLen) */

            switch (pAccCtx->cipherCtx.cipherMode)
            {
                /* case ECB mode */
                case IX_CRYPTO_ACC_MODE_ECB:
                    
                    cfgWord.cryptCfgWord.cipherMode
                        = IX_CRYPTO_NPE_CRYPT_CFG_ECB;
                    
                    break;
                    
                /* case CBC mode */
                case IX_CRYPTO_ACC_MODE_CBC:
                    
                    /* Check and set IV length */
                    if (IX_CRYPTO_ACC_AES_CBC_IV_128
                        == pAccCtx->cipherCtx.cipherInitialVectorLen)
                    {
                        ixCryptoCtx[cryptoCtxId].cipherIvLength 
                            = IX_CRYPTO_ACC_AES_CBC_IV_128;
                    }
                    else /* invalid IV length */
                    {
                        return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_IV_LEN;   
                    } /* end of if-else 
                       * (pAccCtx->cipherCtx.cipherInitialVectorLen)
                       */

                    if (IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT 
                        == cfgWord.cryptCfgWord.encrypt)
                    {
                        /* CBC encryption mode */
                        cfgWord.cryptCfgWord.cipherMode
                            = IX_CRYPTO_NPE_CRYPT_CFG_CBC_ENC;
                    }
                    else /* decryption */
                    {
                        /* CBC decryption mode */
                        cfgWord.cryptCfgWord.cipherMode 
                            = IX_CRYPTO_NPE_CRYPT_CFG_CBC_DEC;
                    } /* end of if-else (CfgWod.cryptCfgWord.Encrypt) */
                    
                    break;

                /* case CTR mode */
                case IX_CRYPTO_ACC_MODE_CTR:
                
                    cfgWord.cryptCfgWord.cipherMode 
                        = IX_CRYPTO_NPE_CRYPT_CFG_CTR;

                    /* Check and set IV length */
                    if (IX_CRYPTO_ACC_AES_CTR_IV_128
                        == pAccCtx->cipherCtx.cipherInitialVectorLen)
                    {
                        ixCryptoCtx[cryptoCtxId].cipherIvLength 
                            = IX_CRYPTO_ACC_AES_CTR_IV_128;
                    }
                    else /* invalid IV length */
                    {
                        return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_IV_LEN;   
                    } /* end of if-else 
                       * (pAccCtx->cipherCtx.cipherInitialVectorLen)
                       */
                       
                    /* Reset encrypt direction in cryptographic config word
                     * to Encrypt regardless of operation (encrypt/decrypt),
                     * this is because of the AES CTR mode special nature.
                     */
                    cfgWord.cryptCfgWord.encrypt 
                        = IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT;
                    
                    break;

                 /* case CCM mode */
                 case IX_CRYPTO_ACC_MODE_CCM:

                    /* Check and set IV length */
                    if (IX_CRYPTO_ACC_AES_CCM_IV_512 
                        == pAccCtx->cipherCtx.cipherInitialVectorLen)
                    {
                        ixCryptoCtx[cryptoCtxId].cipherIvLength 
                            = IX_CRYPTO_ACC_AES_CTR_IV_128;
                    }
                    else /* invalid IV length */
                    {
                        return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_IV_LEN;   
                    } /* end of if-else 
                       * (pAccCtx->cipherCtx.cipherInitialVectorLen)
                       */

                    if (IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT 
                        == cfgWord.cryptCfgWord.encrypt)
                    {
                        /* CCM encryption mode */
                        cfgWord.cryptCfgWord.cipherMode
                            = IX_CRYPTO_NPE_CRYPT_CFG_CCM_ENC;
                    }
                    else /* decryption */
                    {
                        /* CCM decryption mode */
                        cfgWord.cryptCfgWord.cipherMode 
                            = IX_CRYPTO_NPE_CRYPT_CFG_CCM_DEC;
                    } /* end of if-else (CfgWod.cryptCfgWord.Encrypt) */
                    
                    /* Reset encrypt direction in cryptographic config word
                     * to Encrypt regardless of operation (encrypt/decrypt).
                     */
                    cfgWord.cryptCfgWord.encrypt 
                        = IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT;
                    
                    break;

                /* invalid cipher mode */
                default:
                    return IX_CRYPTO_ACC_STATUS_CIPHER_MODE_NOT_SUPPORTED;

            } /* end of switch case (pAccCtx->cipherCtx.cipherMode) */
            
            /* Convert to NPE Format (Big Endian) */
            ixCryptoUtilNpeCryptCfgGenerate (
                &npeCryptoConfigWord, 
                &cfgWord.cryptCfgWord);

            /* Copy Crypt Config word into NPE Crypto Param structure */
            for ( i = 0;
                i < IX_CRYPTO_NPE_CRYPTO_PARAM_CRYPTO_CFG_WORD_LEN; 
                i++)
            {
                /* Shift Crypt Config word into the array of UINT8, 
                 * most significant byte is shifted into the array with
                 * smaller index.
                 * Ex : 
                 *   -------------------
                 *  | b0 | b1 | b2 | b3 |
                 *   -------------------
                 *  array[n] = b0
                 *  array[n+1] = b1
                 */
                ixCryptoCtx[cryptoCtxId].pNpeCryptoParam->
                    npeCryptoInfo[*npeCryptoParamIndex + i]
                        = ((npeCryptoConfigWord >> ((MSB_BYTE - i)
                              * BITS_IN_BYTE)) & MASK_8_BIT);
            } /* end of for (i) */

            /* Log NPE Crypt Config word in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE,
                IX_OSAL_LOG_DEV_STDOUT,
                "Crypt Cfg word is %x\n", 
                npeCryptoConfigWord,
                0, 0, 0, 0, 0);

            /* Update Npe Crypto Param index to move the pointer in NPE
             * Crypto param structure to point end of the Crypto Config
             * word after the cfg word being copied into the structure
             */
            *npeCryptoParamIndex +=
                IX_CRYPTO_NPE_CRYPTO_PARAM_CRYPTO_CFG_WORD_LEN;

            /* Get forward AES key address for reverse AES key
             * computation
             */
            fwdAesKeyAddr = (UINT32)
                &((ixCryptoCtx[cryptoCtxId].pNpeCryptoParam)->
                npeCryptoInfo[*npeCryptoParamIndex]);

            /* Copy Cryption key into NPE Crypto Param structure */
            ixOsalMemCopy (
                &((ixCryptoCtx[cryptoCtxId].pNpeCryptoParam)->
                npeCryptoInfo[*npeCryptoParamIndex]),
                pAccCtx->cipherCtx.key.cipherKey,
                pAccCtx->cipherCtx.cipherKeyLen);

            /* Update Npe Crypto Param index to move the pointer in NPE
             * Crypto param structure to point end of the key after
             * the key being copied into the structure
             */
            *npeCryptoParamIndex += pAccCtx->cipherCtx.cipherKeyLen;

           /* Flush NPE Crypto Param structure from cache into SDRAM */
           IX_CRYPTO_DATA_CACHE_FLUSH (
               ixCryptoCtx[cryptoCtxId].pNpeCryptoParam,
               IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE);

            /* Set Req Type to crypto Hw Accelerator service in crypto 
             * context
             */
            ixCryptoCtx[cryptoCtxId].reqType = IX_CRYPTO_HW_ACCL_REQ;
                
            /* Generate reverse AES key if decryption; otherwise call
             * client's callback, no NPE operation needed
             */
            if (IX_CRYPTO_NPE_CRYPT_CFG_DECRYPT == cfgWord.cryptCfgWord.encrypt)
            {
                /* Generate reverse AES key */
                return ixCryptoRegisterRevAesKeyGenerate (
                           cryptoCtxId,
                           *npeCryptoParamIndex,
                           fwdAesKeyAddr,
                           pAccCtx->cipherCtx.cipherKeyLen,
                           operStatus);
            }
            else
            {
                if (IX_CRYPTO_OP_REGISTER == operStatus)
                {
                    /* Mark valid bit in Crypto Context to indicate the Crypto
                     * Context has been registered successfully
                     */
                    ixCryptoCtx[cryptoCtxId].valid = TRUE;
        
                    /* Call client's callback */
                    ixCryptoCtx[cryptoCtxId].registerCallbackFn(
                    cryptoCtxId,
                    NULL,
                    IX_CRYPTO_ACC_STATUS_SUCCESS);
                }
                return IX_CRYPTO_ACC_STATUS_SUCCESS;
            }

        /* ARC4 algorithm 
         * Note: 
         * i.  ARC4 is a stream cipher, cipher mode is not applicable for 
         *     this algorithm.
         * ii. Encryption key for ARC4 is changed per packet. Thus ARC4 key
         *     is not needed in registration. It will be passed in through API
         *     for every WEP perform request.
         * iii.NPE cryptographic parameters (structure to store crypt config
         *     and keys) is not required for this algorithm. This is because
         *     ARC4 key is a per packet key, and this operation does not 
         *     get executed in coprocessor. Thus crypt config word is not
         *     needed also.
         */
        case IX_CRYPTO_ACC_CIPHER_ARC4:            

            if (IX_CRYPTO_ACC_ARC4_KEY_128 == pAccCtx->cipherCtx.cipherKeyLen)
            {
                /* The first 3 bytes of IV are a  part of ARC4 per packet key
                 * when it is being passed to cryptoAcc through WEP perform API. 
                 * If WEP NPE is used, ARC4 per packet key is passed into NPE via
                 * initialization vactor parameter in NPE queue descriptor.
                 */
                ixCryptoCtx[cryptoCtxId].cipherKeyLength 
                    = IX_CRYPTO_ACC_ARC4_KEY_128;
            }
            else
            {
                /* invalid key length */
                return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_KEY_LEN;   
            } /* end of if-else (pAccCtx->cipherCtx.cipherKeyLen) */
                
            /* Check and set cipher block length */
            if (IX_CRYPTO_ACC_ARC4_BLOCK_8 == pAccCtx->cipherCtx.cipherBlockLen)
            {
                ixCryptoCtx[cryptoCtxId].cipherBlockLength 
                    = IX_CRYPTO_ACC_ARC4_BLOCK_8;
            }
            else /* invalid cipher block length */
            {
                return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_BLOCK_LEN;   
            } /* end of if-else (pAccCtx->cipherCtx.cipherBlockLen) */
                
            /* Set Req Type to WEP NPE service in crypto context
             */
            ixCryptoCtx[cryptoCtxId].reqType = IX_CRYPTO_WEP_REQ;
                
            return IX_CRYPTO_ACC_STATUS_SUCCESS;
        
        /* invalid cipher algo */
        default:
            return IX_CRYPTO_ACC_STATUS_CIPHER_ALGO_NOT_SUPPORTED;

    } /* end of switch case (pAccCtx->cipherCtx.cipherAlgo) */

    /* Note: Normal operation should not reach this point */
    return IX_CRYPTO_ACC_STATUS_FAIL;
} /* end of ixCryptoRegisterCipherAlgoRegister () function */



/**
 * @fn      ixCryptoRegisterAuthAlgoRegister
 * @brief   Register authentication algorithm for a Crypto Context
 *
 *
 */
IxCryptoAccStatus
ixCryptoRegisterAuthAlgoRegister (
    UINT32 cryptoCtxId,
    IxCryptoAccCtx *pAccCtx,
    IX_OSAL_MBUF *pMbufPrimaryChainVar,
    IX_OSAL_MBUF *pMbufSecondaryChainVar,
    UINT32 *npeCryptoParamIndex,
    IxCryptoNpeOperationStatus operStatus)
{
    union
    {
        IxCryptoNpeHashCfgWord hashCfgWord;
        UINT32 npeHashCfgWord;
    } cfgWord;

    UINT32 npeHashConfigWord ;
    IxCryptoAccStatus status = IX_CRYPTO_ACC_STATUS_FAIL;
    UINT32 i;
    UINT32 primaryChainVarAddr;
    UINT32 secondaryChainVarAddr;
    UINT32 npeCryptoParamAddr;
    UINT32 npeCryptoParamIndexStart = *npeCryptoParamIndex;
    UINT32 initLength;
    UINT8 md5ChainVar[IX_CRYPTO_NPE_CRYPTO_PARAM_MD5_CHAIN_VAR_LEN] =
        MD5_CHAIN_VAR;
    UINT8 sha1ChainVar[IX_CRYPTO_NPE_CRYPTO_PARAM_SHA1_CHAIN_VAR_LEN] =
        SHA1_CHAIN_VAR;

    /* Initialize Hash Config word to zero */
    cfgWord.npeHashCfgWord = 0;
    
    /* Mbufs must NOT be NULL if MD5 or SHA1 algorithm. While the mbufs must
     * be NULL if WEP_CRC is selected. This is because mbufs will only be
     * passed back to client in NPE chaining variables computation callback
     */
    switch(pAccCtx->authCtx.authAlgo)
    {
        case IX_CRYPTO_ACC_AUTH_SHA1:
        case IX_CRYPTO_ACC_AUTH_MD5: 

            /* Check if mbuf pointers are NULL */
            if ((NULL == pMbufPrimaryChainVar) ||
                (NULL == pMbufSecondaryChainVar))
            {
                /* mbufs needed in chaining variables computation are not 
                 * avaiable
                 */
                return IX_CRYPTO_ACC_STATUS_FAIL;
            } /* end of if (pMbufPrimaryChainVar && pMbufSecondaryChainVar) */
        
            /* Check if the mbufs provided are chained mbufs */
            if ((NULL != IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pMbufPrimaryChainVar)) ||
                (NULL != IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pMbufSecondaryChainVar)))
            {
                /* chained mbuf, return  error */
                return IX_CRYPTO_ACC_STATUS_FAIL;
            } /* end of if (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pMbufPrimaryChainVar)
               * || IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pMbufSecondaryChainVar))
               */
    
            if ((IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH > 
                 (IX_OSAL_MBUF_MLEN (pMbufPrimaryChainVar))) ||
                (IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH >
                 (IX_OSAL_MBUF_MLEN (pMbufSecondaryChainVar))))
            {
                /* size of mbuf cluster insufficient to hold the data */
                return IX_CRYPTO_ACC_STATUS_FAIL;
            }
            break;

        case IX_CRYPTO_ACC_AUTH_CBC_MAC: 
            /* Check for CCM and WEP CRC is same, fall through */
        case IX_CRYPTO_ACC_AUTH_WEP_CRC:
     
            /* Check if mbuf pointers are NULL, return error if not NULL  */
            if ((NULL != pMbufPrimaryChainVar) ||
                (NULL != pMbufSecondaryChainVar))
            {
                /* mbufs are not needed */
                return IX_CRYPTO_ACC_STATUS_FAIL;

            } /* end of if (pMbufPrimaryChainVar && pMbufSecondaryChainVar) */

             break;
            /* invalid authentication algorithm */
        default:
            return IX_CRYPTO_ACC_STATUS_AUTH_ALGO_NOT_SUPPORTED;            
    }
    
    /* Authentication algorithm switch case options
     * For each case of valid authentication algorithm, hash key length
     * and digest length will be verified against the authentication
     * algorithm standard
     */
    switch (pAccCtx->authCtx.authAlgo)
    {
        /* SHA1 algorithm */
        case IX_CRYPTO_ACC_AUTH_SHA1:
            
            /* Check authentication key length */
            if ((IX_CRYPTO_ACC_SHA1_KEY_160 > pAccCtx->authCtx.authKeyLen) ||
                (IX_CRYPTO_ACC_MAX_AUTH_KEY_LENGTH < pAccCtx->authCtx.authKeyLen))
            {
                /* invalid key length */
                return IX_CRYPTO_ACC_STATUS_AUTH_INVALID_KEY_LEN;
            } /* end of if-else (pAccCtx->authCtx.authAlgo) */

            /* Check authentication digest length, return error if the 
             * digest length is not within the range permitted by the
             * algorithm and the length is not word-aligned 
             */
            if ((IX_CRYPTO_ACC_SHA1_DIGEST_160 
                >= pAccCtx->authCtx.authDigestLen)
                && (IX_CRYPTO_WORD_ALIGNED == 
                (pAccCtx->authCtx.authDigestLen % WORD_IN_BYTES)))
            {
                cfgWord.hashCfgWord.digestLength = 
                    (pAccCtx->authCtx.authDigestLen /
                    WORD_IN_BYTES);
            }
            else /* invalid digest length */
            {
                return IX_CRYPTO_ACC_STATUS_AUTH_INVALID_DIGEST_LEN;
            } /* end of if-else (pAccCtx->authCtx.authDigestLen) */
            
            /* Set authentication algorithm */
            cfgWord.hashCfgWord.hashAlgo = IX_CRYPTO_NPE_HASH_CFG_SHA1;

            /* Set chaining variable length */
            cfgWord.hashCfgWord.CVLength 
                = (IX_CRYPTO_NPE_CRYPTO_PARAM_SHA1_CHAIN_VAR_LEN /
                      WORD_IN_BYTES);
        
            /* Configure endian mode of NPE, by default is Big endian.
             * No swapping is required
             */
            cfgWord.hashCfgWord.chnWr = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
            cfgWord.hashCfgWord.chnRd = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
            cfgWord.hashCfgWord.hdWr = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
            cfgWord.hashCfgWord.hdRd = IX_CRYPTO_NPE_CFG_ENDIAN_NO_SWAP;
            
            /* Set Req Type to crypto Hw Accelerator service in crypto 
             * context
             */
            ixCryptoCtx[cryptoCtxId].reqType = IX_CRYPTO_HW_ACCL_REQ;
                
            break;
        
        /* MD5 algorithm */
        case IX_CRYPTO_ACC_AUTH_MD5:
        
            /* Check authentication key length */
            if ((IX_CRYPTO_ACC_MD5_KEY_128 > pAccCtx->authCtx.authKeyLen) ||
                (IX_CRYPTO_ACC_MAX_AUTH_KEY_LENGTH < pAccCtx->authCtx.authKeyLen))
            {
                /* invalid key length */
                return IX_CRYPTO_ACC_STATUS_AUTH_INVALID_KEY_LEN;
            } /* end of if-else (pAccCtx->authCtx.authAlgo) */
            
            /* Check authentication digest length, return error if the 
             * digest length is not within the range permitted by the
             * algorithm and the length is not word-aligned
             */
            if ((IX_CRYPTO_ACC_MD5_DIGEST_128 
                >= pAccCtx->authCtx.authDigestLen)
                && (IX_CRYPTO_WORD_ALIGNED == 
                (pAccCtx->authCtx.authDigestLen % WORD_IN_BYTES)))
            {
                cfgWord.hashCfgWord.digestLength
                    = (pAccCtx->authCtx.authDigestLen /
                          WORD_IN_BYTES);
            }
            else /* invalid digest length */
            {
                return IX_CRYPTO_ACC_STATUS_AUTH_INVALID_DIGEST_LEN;
            } /* end of if-else (pAccCtx->authCtx.authDigestLen) */
            
            /* Set authentication algorithm */
            cfgWord.hashCfgWord.hashAlgo = IX_CRYPTO_NPE_HASH_CFG_MD5;
            
            /* Set chaining variable length */
            cfgWord.hashCfgWord.CVLength 
                = (IX_CRYPTO_NPE_CRYPTO_PARAM_MD5_CHAIN_VAR_LEN /
                       WORD_IN_BYTES);

            /* Configure endian mode of NPE, MD5 hashing co-processor
             * operate in Little Endian, byte swapping is required
             */
            cfgWord.hashCfgWord.chnWr = IX_CRYPTO_NPE_CFG_BYTE_SWAP;
            cfgWord.hashCfgWord.chnRd = IX_CRYPTO_NPE_CFG_BYTE_SWAP;
            cfgWord.hashCfgWord.hdWr = IX_CRYPTO_NPE_CFG_BYTE_SWAP;
            cfgWord.hashCfgWord.hdRd = IX_CRYPTO_NPE_CFG_BYTE_SWAP;
            
                
            /* Set Req Type to crypto Hw Accelerator service in crypto
             * context
             */
            ixCryptoCtx[cryptoCtxId].reqType = IX_CRYPTO_HW_ACCL_REQ;

            break;

        /* CBC_MAC */
        case IX_CRYPTO_ACC_AUTH_CBC_MAC:
            
           /* Check whether the digest length is valid */
           if (IX_CRYPTO_ACC_CCM_DIGEST_64 !=  
                pAccCtx->authCtx.authDigestLen)
            {
                return IX_CRYPTO_ACC_STATUS_AUTH_INVALID_DIGEST_LEN;
            } /* end of if-else (pAccCtx->authCtx.authDigestLen) */

            /* Save the digest length */
            ixCryptoCtx[cryptoCtxId].digestLength = 
                IX_CRYPTO_ACC_CCM_DIGEST_64;
           
           /* Check whether the aadLen is valid */
           if ( IX_CRYPTO_ACC_CCM_AAD_LEN_384  !=  
                pAccCtx->authCtx.aadLen)
            {
                return IX_CRYPTO_ACC_STATUS_AUTH_INVALID_AAD_LEN;
            } /* end of if-else (pAccCtx->authCtx.authDigestLen) */
            
            /* Save the aadLen */
            ixCryptoCtx[cryptoCtxId].aadLen=IX_CRYPTO_ACC_CCM_AAD_LEN_384;

            /* Set Req Type to crypto Hw Accelerator service in crypto 
             * context
             */
            ixCryptoCtx[cryptoCtxId].reqType = IX_CRYPTO_HW_ACCL_REQ;

            return IX_CRYPTO_ACC_STATUS_SUCCESS;
            
        /* WEP CRC-32 algorithm
         *
         * Note: 
         * i.  WEP_CRC is using CRC-32 checksum algorithm, key is not
         *     required for this operation. Thus WEP_CRC key is not needed 
         *     registration.
         * ii. NPE cryptographic parameters (structure to store hash config
         *     and keys) is not required for this algorithm. This is because
         *     key is not required in NPE operation, and this operation does  
         *     not get executed in coprocessor. Thus hash config word is not
         *     needed also.
         * iii.After parameters checking, the function is forced return in 
         *     case statement as chaining variables computation after that 
         *     are not needed for this algorithm.
         */
        case IX_CRYPTO_ACC_AUTH_WEP_CRC:
            
            /* Check authentication digest length, return error if the 
             * digest length is not permitted by the algorithm 
             */
            if (IX_CRYPTO_ACC_WEP_CRC_DIGEST_32 
                != pAccCtx->authCtx.authDigestLen)
            {
                /* invalid digest length */
                return IX_CRYPTO_ACC_STATUS_AUTH_INVALID_DIGEST_LEN;
            }
            else
            {
                /* Store digest length / ICV length into crypto context */
                ixCryptoCtx[cryptoCtxId].digestLength
                    = IX_CRYPTO_ACC_WEP_CRC_DIGEST_32;
            }/* end of if-else (pAccCtx->authCtx.authDigestLen) */
            
            /* Set Req Type to WEP NPE service in crypto context
             */
            ixCryptoCtx[cryptoCtxId].reqType = IX_CRYPTO_WEP_REQ;
            
            return IX_CRYPTO_ACC_STATUS_SUCCESS;
        
        /* invalid authentication algorithm */
        default:
            return IX_CRYPTO_ACC_STATUS_AUTH_ALGO_NOT_SUPPORTED;            
        
    } /* end of switch case (pAccCtx->authCtx.authAlgo) */
    
    /* Keep a copy of digest length in ixCryptoCtx for future reference. If 
     * the requested operation is IX_CRYPTO_NPE_OP_HMAC_VER_ICV or 
     * IX_CRYPTO_NPE_OP_HMAC_VER_ICV_DECRYPT, the ICV field with the length
     * indicated by this digest length need to be set to zero prior sending 
     * to NPE for processing
     */
    ixCryptoCtx[cryptoCtxId].digestLength = pAccCtx->authCtx.authDigestLen;
    
    /* Convert to NPE Format (Big Endian) */
    ixCryptoUtilNpeHashCfgGenerate (
        &npeHashConfigWord, 
        &cfgWord.hashCfgWord);

    /* Copy Crypt Config word into NPE Crypto Param structure */
    for ( i = 0;
        i < IX_CRYPTO_NPE_CRYPTO_PARAM_HASH_CFG_WORD_LEN;
        i++)
    {
        /* Shift Hash Config word into the array of UINT8,
         * most significant byte is shifted into the array with
         * smaller index.
         * Ex : 
         *   -------------------
         *  | b0 | b1 | b2 | b3 |
         *   -------------------
         *  array[n] = b0
         *  array[n+1] = b1
         */
        ixCryptoCtx[cryptoCtxId].pNpeCryptoParam->
            npeCryptoInfo[*npeCryptoParamIndex + i]
                = ((npeHashConfigWord >> ((MSB_BYTE - i) * BITS_IN_BYTE)) 
                       & MASK_8_BIT);
    } /* end of for (i) */

    /* Log NPE Crypt Config word in debugging mode */
    IX_CRYPTO_ACC_LOG (
        IX_OSAL_LOG_LVL_MESSAGE,
        IX_OSAL_LOG_DEV_STDOUT,
        "Hash Cfg word is %x\n",
        npeHashConfigWord,
        0, 0, 0, 0, 0);

    /* Get the start address for Hashing configuration in NPE Crypto Param
     * structure
     */    
    npeCryptoParamAddr = (UINT32)
       &((ixCryptoCtx[cryptoCtxId].pNpeCryptoParam)->
            npeCryptoInfo[*npeCryptoParamIndex]);
   
    /* Update Npe Crypto Param index to move the pointer in NPE Crypto
     * param structure to point end of the hash config word after 
     * the cfg word being copied into the structure
     */
    *npeCryptoParamIndex += IX_CRYPTO_NPE_CRYPTO_PARAM_HASH_CFG_WORD_LEN;
    
    /* Initialize NPE crypto param structure with algorithm dependent 
     * initial chaining variables value
     */
    if (pAccCtx->authCtx.authAlgo == IX_CRYPTO_ACC_AUTH_MD5)
    {
        /* Copy MD5 initial chaining variable value into NPE Crypto
         * param structure 
         */
        ixOsalMemCopy (&(ixCryptoCtx[cryptoCtxId].pNpeCryptoParam->
           npeCryptoInfo[*npeCryptoParamIndex]),
           md5ChainVar,
           IX_CRYPTO_NPE_CRYPTO_PARAM_MD5_CHAIN_VAR_LEN);        
    }
    else /* IX_CRYPTO_ACC_AUTH_SHA1 */
    {        
        /* Copy SHA1 initial chaining variable value into NPE Crypto
         * param structure 
         */            
        ixOsalMemCopy (&(ixCryptoCtx[cryptoCtxId].pNpeCryptoParam->
            npeCryptoInfo[*npeCryptoParamIndex]),
            sha1ChainVar,
            IX_CRYPTO_NPE_CRYPTO_PARAM_SHA1_CHAIN_VAR_LEN);

    } /* end of if-else (pAccCtx->authCtx.authAlgo) */

    /* Prepare initial chaining variables computation mbuf */
    ixCryptoRegisterChainVarMbufPrepare (
        pMbufPrimaryChainVar,
        pMbufSecondaryChainVar,
        pAccCtx);
        
    /* Get chaining variable address in NPE Crypto Param structure to store
     * primary chaining variables 
     */
    primaryChainVarAddr = (UINT32)
        &((ixCryptoCtx[cryptoCtxId].pNpeCryptoParam)->
            npeCryptoInfo[*npeCryptoParamIndex]);        

    /* Update Npe Crypto Param index (in unit of bytes) for primary 
     * chaining variable, it is used as initLength in Q Descriptor
     */
    *npeCryptoParamIndex += (cfgWord.hashCfgWord.CVLength * WORD_IN_BYTES);
       
    /* Init length for chaining variables computation is the length of
     * hash config word plus length of primary chaining variable
     */
    initLength = *npeCryptoParamIndex - npeCryptoParamIndexStart;

    /* Get chaining address in NPE Crypto Param structure to store the
     * secondary chaining variables. It is stored after the primary 
     * chaining variable
     */ 
    secondaryChainVarAddr = (UINT32)
        &((ixCryptoCtx[cryptoCtxId].pNpeCryptoParam)->
            npeCryptoInfo[*npeCryptoParamIndex]);

    /* Flush NPE Crypto Param structure from cache into SDRAM */
    IX_CRYPTO_DATA_CACHE_FLUSH (
        ixCryptoCtx[cryptoCtxId].pNpeCryptoParam,
        IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE);

    /* Enqueue secondary chaining variable computation request to NPE first
     * to avoid the initial algorithm-dependant keys being overwritten by 
     * primary chaining variable
     */
    status = ixCryptoRegisterChainVariablesGenerate (
                 cryptoCtxId,
                 pMbufSecondaryChainVar,
                 initLength,
                 npeCryptoParamAddr,
                 secondaryChainVarAddr,
                 IX_CRYPTO_OP_WAIT);
        
    /* Check if secondary chaining variable computation function call 
     * returned successfully 
     */
    if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
    {
        /* Enqueue secondary chaining variable computation request to NPE
        * note: initLength is the same for primary and secondary chaining
        * variables computation as the length of secondary chaining
        * variable is not included in the initLength
        */
        return ixCryptoRegisterChainVariablesGenerate (
                   cryptoCtxId,
                   pMbufPrimaryChainVar,
                   initLength,
                   npeCryptoParamAddr,
                   primaryChainVarAddr,
                   operStatus);
    }
    else /* != IX_CRYPTO_ACC_STATUS_SUCCESS */
    {
        return status;
    } /* end of if-else (status) */
} /* end of ixCryptoRegisterAuthAlgoRegister () functon */



/**
 * @fn      ixCryptoRegisterRevAesKeyGenerate
 * @brief   Generate reverse AES key and store the reverse AES key into
 *          Crypto Context
 *
 *
 */
IxCryptoAccStatus
ixCryptoRegisterRevAesKeyGenerate (
    UINT32 cryptoCtxId,
    UINT32 initLength,
    UINT32 fwdAesKeyAddr,
    UINT32 keyLength,
    IxCryptoNpeOperationStatus operStatus)
{
    IxCryptoQDescriptor *pQDesc = NULL;
    IxCryptoAccStatus status;
    INT32 i;
    UINT32 revAesKeyCryptCfg;
    UINT32 keyId;
    UINT32 npeCryptoParamAddr;
    UINT32 revAesKeyAddr;
    UINT32 u32pQDesc = 0;

    /* Construct crypt config for rev-aes-key-gen
     * Note: Cipher mode in crypt config is ignored in Rev-AES key
     *       generation, thus cipher mode is set to ECB by default 
     *       for all cases.
     */
    switch (keyLength)
    {
        case IX_CRYPTO_ACC_AES_KEY_128: /* AES-128 */
            revAesKeyCryptCfg
                = (IX_CRYPTO_NPE_CRYPT_CFG_ECB <<
                    IX_CRYPTO_NPE_CRYPT_CFG_CIPHER_MODE_POS) |
                    (IX_CRYPTO_NPE_CRYPT_CFG_AES <<
                    IX_CRYPTO_NPE_CRYPT_CFG_CIPHER_ALGO_POS) |
                    (IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT <<
                    IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT_POS) |
                    (IX_CRYPTO_NPE_CRYPT_CFG_AES128 <<
                    IX_CRYPTO_NPE_CRYPT_CFG_CRYPT_MODE_POS) |
                    IX_CRYPTO_AES128_KEY_LEN_IN_WORDS;
            break;

        case IX_CRYPTO_ACC_AES_KEY_192: /* AES-192 */
            revAesKeyCryptCfg
                = (IX_CRYPTO_NPE_CRYPT_CFG_ECB <<
                    IX_CRYPTO_NPE_CRYPT_CFG_CIPHER_MODE_POS) |
                    (IX_CRYPTO_NPE_CRYPT_CFG_AES <<
                    IX_CRYPTO_NPE_CRYPT_CFG_CIPHER_ALGO_POS) |
                    (IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT <<
                    IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT_POS) |
                    (IX_CRYPTO_NPE_CRYPT_CFG_AES192 <<
                    IX_CRYPTO_NPE_CRYPT_CFG_CRYPT_MODE_POS) |
                    IX_CRYPTO_AES192_KEY_LEN_IN_WORDS;
            break;

        case IX_CRYPTO_ACC_AES_KEY_256: /* AES-128 */
            revAesKeyCryptCfg
                = (IX_CRYPTO_NPE_CRYPT_CFG_ECB <<
                    IX_CRYPTO_NPE_CRYPT_CFG_CIPHER_MODE_POS) |
                    (IX_CRYPTO_NPE_CRYPT_CFG_AES <<
                    IX_CRYPTO_NPE_CRYPT_CFG_CIPHER_ALGO_POS) |
                    (IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT <<
                    IX_CRYPTO_NPE_CRYPT_CFG_ENCRYPT_POS) |
                    (IX_CRYPTO_NPE_CRYPT_CFG_AES256 <<
                    IX_CRYPTO_NPE_CRYPT_CFG_CRYPT_MODE_POS) |
                    IX_CRYPTO_AES256_KEY_LEN_IN_WORDS;
            break;

        default:
            return IX_CRYPTO_ACC_STATUS_FAIL;
    }

    /* Get Crypto Param for reverse AES key generation */
    status = ixCryptoCCDMgmtKeyCryptoParamGet (&keyId);

    if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
    {
        /* Store key ID for reference in QMgr callback */
        ixCryptoCtx[cryptoCtxId].validAndKeyId 
            = IX_CRYPTO_ACC_REV_AES_KEY_VALID | keyId;
        /* Store Key length for reference in QMgr callback */
        ixKeyCryptoParam[keyId].keyLength = keyLength;
    }
    else /* No Crypto Context is allocated */
    {
        return status;
    } /* end of if-else (status) */

    /* Store crypt config into NPE Crypto Param structure */
    for ( i = 0;
          i < IX_CRYPTO_NPE_CRYPTO_PARAM_CRYPTO_CFG_WORD_LEN;
          i++)
    {
        ixKeyCryptoParam[keyId].pNpeCryptoParam->npeCryptoInfo[i]
            = ((revAesKeyCryptCfg >> ((MSB_BYTE - i)
                * BITS_IN_BYTE)) & MASK_8_BIT);
    } /* end of for (i) */

    /* Log NPE Crypt Config word in debugging mode */
    IX_CRYPTO_ACC_LOG (
        IX_OSAL_LOG_LVL_MESSAGE,
        IX_OSAL_LOG_DEV_STDOUT,
        "Rev AES key Crypt Cfg word is %x\n",
        revAesKeyCryptCfg,
        0, 0, 0, 0, 0);

    /* Copy Cryption key into NPE Crypto Param structure */
    ixOsalMemCopy (
        &((ixKeyCryptoParam[keyId].pNpeCryptoParam)
        ->npeCryptoInfo[IX_CRYPTO_NPE_CRYPTO_PARAM_CRYPTO_CFG_WORD_LEN]),
        (char *) fwdAesKeyAddr,
        keyLength);

    /* Flush NPE Crypto Param structure from cache into SDRAM */
    IX_CRYPTO_DATA_CACHE_FLUSH (
        ixKeyCryptoParam[keyId].pNpeCryptoParam,
        IX_CRYPTO_NPE_CRYPTO_PARAM_SIZE);

     /* Get the start address for cryption configuration in NPE
      * Crypto Paramstructure
      */
     npeCryptoParamAddr = (UINT32) ixKeyCryptoParam[keyId].pNpeCryptoParam;

    /* Get the address to store reverse AES key */
    revAesKeyAddr = (UINT32)
         &((ixKeyCryptoParam[keyId].pNpeCryptoParam)
         ->npeCryptoInfo[IX_CRYPTO_NPE_CRYPTO_PARAM_CRYPTO_CFG_WORD_LEN]);

    /* Get a free Q descriptor from Q descriptors pool
     * Notes: Q descriptor is pushed back to the descriptor pool if the
     *        queue write operation failed. If the queue write operation
     *        pass, the Q descriptor will be processed by NPE and being
     *        release in QMgr dispatcher context through registered
     *        ixCryptoQAccessReqDoneQMgrCallback function
     */
    status = ixCryptoDescMgmtQDescriptorGet (&pQDesc);

    if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
    {
        /* Construct Q descriptor message */
        pQDesc->cryptoCtxId = cryptoCtxId;
        pQDesc->operStatus = operStatus;

        /* Set to reverse AES key generation operation */
        pQDesc->npeQDesc.npeOperationMode.npeOperation
            = IX_CRYPTO_NPE_OP_ENCRYPT_GEN_KEY;

        /* Set length of initial values in Crypto Param structure */
        pQDesc->npeQDesc.npeOperationMode.initLength
            = initLength & MASK_8_BIT;

        /* Offset and length of data */
        pQDesc->npeQDesc.cryptStartOffset = 0;

        /* crypt length is size of a single (dummy) AES data block 
         * + endian conversion
         */
        pQDesc->npeQDesc.cryptLength 
            = IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER(
                (UINT16) IX_CRYPTO_ACC_AES_BLOCK_128);

        /* Convert virtual address to physical address, NPE only takes
         * physical address
         */

        /* NPE Crypto Param structure start address + endian conversion */
        pQDesc->npeQDesc.cryptoCtxNpeAddr
            = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER(
              (UINT32)IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION(npeCryptoParamAddr));

        /* Address to store reverse AES key computation result 
         * + endian conversion
         */ 
        pQDesc->npeQDesc.address.revAesKeyAddr
            = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER(
                (UINT32)IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (revAesKeyAddr));

        /* Flush Q descriptor message from cache */
        IX_CRYPTO_DATA_CACHE_FLUSH (pQDesc, sizeof (IxCryptoQDescriptor));

        /* Convert Q descriptor address to physical address */
        pQDesc = (IxCryptoQDescriptor *)
                    IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION(
                    (UINT32) pQDesc);

        /* Enqueue request to hardware queue */
        u32pQDesc = (UINT32) pQDesc;
        status = ixCryptoQAccessQueueWrite (
                     IX_CRYPTO_ACC_CRYPTO_REQ_Q,
                     &u32pQDesc);

        /* Check if operation enqueued successfully */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
        {
            /* Increment counter in the Crypto Context to keep track the
             * number of requests have been issued
             */
            ixCryptoCtx[cryptoCtxId].reqCount++;
        }
        else /* Queue write operation failed */
        {
            ixCryptoCCDMgmtKeyCryptoParamRelease (keyId);

            /* Convert Q descriptor address back to virtual address
             * before releasing it
             */
            pQDesc = (IxCryptoQDescriptor *)
                         IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION(
                         (UINT32) pQDesc);

            /* Release Q descriptor back to the descriptor pool */
            ixCryptoDescMgmtQDescriptorRelease (pQDesc);
        }/* end of if-else (status) */
    }
    else
    {
        ixCryptoCCDMgmtKeyCryptoParamRelease (keyId);
    } /* end of if (status) for (ixCryptoDescMgmtQDescriptorGet) */

    return status;
} /* end of ixCryptoRegisterRevAesKeyGenerate () function */



/**
 * @fn    ixCryptoRegisterChainVarMbufPrepare
 * @brief Prepare input for NPE to generate initial chaining variables,
 *        the input are stored in the mbufs. Hash key is padded to 64 bytes
 *        and each byte is XORed with 0x36 for primary chaining variables.
 *        (key XORed with ipad, where ipad is the byte 0x36 repeated 64
 *        times). Hash key is padded to 64 bytes and each byte is XORed with
 *        0x5C for secondary chaining variables. (key XORed with opad, where
 *        opad is the byte 0x5C repeated 64 times).Please refer to RFC2104
 *        (HMAC: Keyed-Hashing for Message Authentication) for details.
 *
 *
 */
void
ixCryptoRegisterChainVarMbufPrepare (
    IX_OSAL_MBUF *pMbufPrimaryChainVar,
    IX_OSAL_MBUF *pMbufSecondaryChainVar,
    IxCryptoAccCtx *pAccCtx)
{
    UINT32 i;
    UINT8 * pChainVar1 = IX_OSAL_MBUF_MDATA (pMbufPrimaryChainVar);
    UINT8 * pChainVar2 = IX_OSAL_MBUF_MDATA (pMbufSecondaryChainVar);    
    
    /* Copy hash key into mbuf */
    ixOsalMemCopy (
        pChainVar1,
        pAccCtx->authCtx.key.authKey,
        pAccCtx->authCtx.authKeyLen);    

    ixOsalMemCopy (
        pChainVar2, 
        pAccCtx->authCtx.key.authKey,
        pAccCtx->authCtx.authKeyLen);            

    for (i = 0; i < pAccCtx->authCtx.authKeyLen; i++)
    {
        /* XOR with ipad */
        pChainVar1[i] ^= IX_CRYPTO_HMAC_IPAD_VALUE;
        
        /* XOR with opad */
        pChainVar2[i] ^= IX_CRYPTO_HMAC_OPAD_VALUE;
    } /* end of for (i) */;
    
    /* Pad buffer after hash key in mbuf to 0x36(ipad) directly, skipping 
     * the padding process to pad the hash key into length of 
     * AUTHENTICATION_DATA_BLOCK_LENGTH with zeroes, as the padded bytes in
     * the mbuf needs to be XORed with ipad and whatever number XORed with
     * zeroes will get back the original number again.
     */
    ixOsalMemSet (
        &pChainVar1[pAccCtx->authCtx.authKeyLen],
        IX_CRYPTO_HMAC_IPAD_VALUE,
        (IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH 
        - pAccCtx->authCtx.authKeyLen));
        
    /* Pad buffer after hash key in mbuf to 0x5C(opad) directly, skipping 
     * the padding process to pad the hash key into length of 
     * AUTHENTICATION_DATA_BLOCK_LENGTH with zeroes, as the padded bytes in 
     * the mbuf needs to be XORed with opad and whatever number XORed with
     * zeroes will get back the original number again.
     */
    ixOsalMemSet (
        &pChainVar2[pAccCtx->authCtx.authKeyLen],
        IX_CRYPTO_HMAC_OPAD_VALUE,
        (IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH 
        - pAccCtx->authCtx.authKeyLen));
            
} /* end of ixCryptoRegisterPrimaryChainVarMbufPrepare () function */



/**
 * @fn      ixCryptoRegisterChainVariablesGenerate
 * @brief   Generate initial chaining variables through NPE. The result of 
 *          computation will be stored in IxCryptoNpeCryptoParam, which is 
 *          associated with cryptoCtxId. Only one initial chaining variable 
 *          is computed per each function call.
 *
 */
IxCryptoAccStatus
ixCryptoRegisterChainVariablesGenerate (
    UINT32 cryptoCtxId,
    IX_OSAL_MBUF *pMbufChainVar,
    UINT32 initLength,
    UINT32 npeCryptoParamAddr,
    UINT32 chainVarAddr,
    IxCryptoNpeOperationStatus operStatus)
{
    IxCryptoQDescriptor *pQDesc = NULL;
    IxCryptoAccStatus status;
    UINT32 u32pQDesc = 0;

    /* Get a free Q descriptor from Q descriptors pool
     * Notes: Q descriptor is pushed back to the descriptor pool if the 
     *        queue write operation failed. If the queue write operation 
     *        pass, the Q descriptor will be processed by NPE and being 
     *        release in QMgr dispatcher context through registered
     *        ixCryptoQAccessReqDoneQMgrCallback function
     */
    status =ixCryptoDescMgmtQDescriptorGet (&pQDesc);
    
    if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
    {
        /* Construct Q descriptor message */        
        pQDesc->cryptoCtxId = cryptoCtxId;
        pQDesc->operStatus = operStatus;
        
        /* Store mbuf pointer into Queue descriptor */
        pQDesc->pSrcMbuf = pMbufChainVar;
        
        /* Convert mbuf into NPE format */
        pQDesc->npeQDesc.srcMbufAddr 
            = (UINT32) ixCryptoUtilMbufToNpeFormatConvert(pMbufChainVar, 0);

        /* Set to chaining variables generation operation */
        pQDesc->npeQDesc.npeOperationMode.npeOperation 
            = IX_CRYPTO_NPE_OP_HASH_GEN_ICV; 

        /* Set length of initial values in Crypto Param structure */
        pQDesc->npeQDesc.npeOperationMode.initLength 
            = initLength & MASK_8_BIT;

        /* Offset and length of data + Endian conversion */
        pQDesc->npeQDesc.authStartOffset = 0;
        pQDesc->npeQDesc.authLength 
            = IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER(
                (UINT16) IX_CRYPTO_AUTHENTICATION_DATA_BLOCK_LENGTH);

        /* Convert virtual address to physical address, NPE only takes 
         * physical address 
         */
         
        /*Initial chaining variable address in NPE Crypto param structure 
         *+ Endian conversion
         */
        pQDesc->npeQDesc.address.icvAddr 
            = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER(
              (UINT32)IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (chainVarAddr));

        /* NPE Crypto Param structure start address 
         * + Endian conversion
         */
        pQDesc->npeQDesc.cryptoCtxNpeAddr
            = IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER(
                (UINT32)IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION (
                npeCryptoParamAddr));
 
        /* Flush Q descriptor message from cache */
        IX_CRYPTO_DATA_CACHE_FLUSH (pQDesc, sizeof (IxCryptoQDescriptor));

        /* Convert Q descriptor address to physical address */
        pQDesc = (IxCryptoQDescriptor *)
                     IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION(
                     (UINT32) pQDesc);

        /* Enqueue request to hardware queue */
        u32pQDesc = (UINT32) pQDesc;
        status = ixCryptoQAccessQueueWrite (
                     IX_CRYPTO_ACC_CRYPTO_REQ_Q,
                     &u32pQDesc);

        /* Check if operation enqueued successfully */
        if (IX_CRYPTO_ACC_STATUS_SUCCESS == status)
        {
            /* Increment counter in the Crypto Context to keep track the
             * number of requests have been issued
             */
            ixCryptoCtx[cryptoCtxId].reqCount++;
        }
        else /* Queue write operation failed */
        {
            /* Convert Q descriptor address back to virtual address
             * before releasing it
             */
            pQDesc = (IxCryptoQDescriptor *)
                         IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION(
                         (UINT32) pQDesc);

            /* Convert mbuf back to host format from NPE format */
            pMbufChainVar = ixCryptoUtilMbufFromNpeFormatConvert (
                                pQDesc->pSrcMbuf);
                    
            /* Release Q descriptor back to the descriptor pool */
            ixCryptoDescMgmtQDescriptorRelease (pQDesc);            
        }/* end of if-else (status) */
    } /* end of if (status) for (ixCryptoDescMgmtQDescriptorGet) */
    
    return status;    
} /* end of ixCryptoRegisterChainVariablesGenerate () function */
