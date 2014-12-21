/**
 * @file IxCryptoAccCryptoPerform.c
 *
 * @date 03-October-2002
 *
 * @brief Module to perform crypto services via NPE.
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

/**
 * Put the system defined include files required.
 */


/**
 * Put the user defined include files required.
 */

#include "IxOsal.h"
#include "IxCryptoAcc.h"
#include "IxCryptoAcc_p.h"
#include "IxCryptoAccCryptoPerform_p.h"
#include "IxCryptoAccQAccess_p.h"
#include "IxCryptoAccDescMgmt_p.h"
#include "IxCryptoAccCCDMgmt_p.h"
#include "IxCryptoAccUtilities_p.h"

/**
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

/* CRC initial value for computation */
#define IX_CRYPTO_ACC_CRC_INITIAL_VALUE 0xFFFFFFFF
#define IX_CRYPTO_ACC_CRC_SHIFT_0_BIT       0
#define IX_CRYPTO_ACC_CRC_SHIFT_8_BIT       8
#define IX_CRYPTO_ACC_CRC_SHIFT_16_BIT      16
#define IX_CRYPTO_ACC_CRC_SHIFT_24_BIT      24

/* CRC lookup table */
const UINT32 ixCryptoAccCrcTable[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
    0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
    0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
    0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
    0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
    0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
    0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
    0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
    0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
    0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
    0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
    0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
    0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
    0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
    0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
    0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
    0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
    0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
    0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
    0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
    0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
                             
/* extern IxCryptoAccCryptoCtx ixCryptoCtx[IX_CRYPTO_ACC_MAX_ACTIVE_SA_TUNNELS];
 * is declared in IxCryptoAccCCDMgmt_p.h
 */


/* Function mapping according to the WEP operation */
IxCryptoPerformWepFunction wepFunction[IX_CRYPTO_ACC_NUM_WEP_FUNCTION]
    = {
        ARC4_Crypt,             /* RC4 decryption only */
        ARC4_Crypt,             /* RC4 encryption only */
        ARC4_DecryptWithCRC,    /* RC4 decryption followed by CRC verification */
        ARC4_EncryptWithCRC,    /* CRC generation followed by RC4 encryption */
        CRC_Compute,            /* CRC verfication only */
        CRC_Compute             /* CRC generation only */
    };


/**
 *
 * @fn      ixCryptoPerformBufferOffsetBoundaryCheck
 * @brief   To check the boundary condition of the buffer offset provided 
 *          by the client.
 *
 */

#ifndef NDEBUG
IxCryptoAccStatus 
ixCryptoPerformBufferOffsetBoundaryCheck (
    UINT32 npeOperation,
    IX_OSAL_MBUF *pSrcMbuf,
    IX_OSAL_MBUF *pDestMbuf,
    UINT16 authStartOffset,
    UINT16 authDataLen,
    UINT16 cryptStartOffset,
    UINT16 cryptDataLen,
    UINT16 icvOffset)
{
    IX_OSAL_MBUF *pLengthMbufPtr;
    int length;
    int cipherLen = 0;
    int authLen = 0;
    BOOL opAuth = FALSE;  /* True if authenticating */
    BOOL opCrypt = FALSE; /* True if doing cryption */

    /* Mask the IxCryptoNpeOperationMode HMac bit to obtain the current status 
     * of Authentication */
    opAuth  = (IX_CRYPTO_NPE_OP_HMAC_MODE_IS_ENABLED == 
        (npeOperation & IX_CRYPTO_NPE_OP_HMAC_DISABLE_MASK));
    /* Mask the IxCryptoNpeOperationMode Crypt bit to obtain the current status 
     * of Cryption */
    opCrypt = (IX_CRYPTO_NPE_OP_CRYPT_MODE_IS_DISABLED != 
           (npeOperation & IX_CRYPTO_NPE_OP_CRYPT_ENABLE_MASK));
    
    if (IX_CRYPTO_NPE_OP_CCM_ENABLE_MASK
        == (IX_CRYPTO_NPE_OP_CCM_ENABLE_MASK & npeOperation))
    {
        /* ICV offset should always be at the end of the packet in this
         * mode. There is no difference between GEN_MIC and VER_MIC when
         * calculating icv ptr. Reset authStartOffset and authDataLen 
         * to cryptOffset and cryptDataLen. They are ignored by the NPE
         * when processing request.
         */
        authStartOffset = cryptStartOffset;
        authDataLen = cryptDataLen;
        if(icvOffset < (cryptStartOffset + cryptDataLen))
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "ICV offset should be > (cryptStartOffset+cryptDataLen) \n",
                0, 0, 0, 0, 0, 0);
            return IX_CRYPTO_ACC_STATUS_FAIL;  
        }  
    }

    /* Get total length of chained mbuf - only the source mbuf is chained. The
     * destination mbuf will never be chained
     */
    pLengthMbufPtr = pSrcMbuf;
    length = IX_OSAL_MBUF_MLEN (pSrcMbuf);
    cipherLen = (int)(cryptStartOffset+cryptDataLen);
    authLen = (int)(authStartOffset+authDataLen);

    while (NULL != IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pLengthMbufPtr))
    {
    pLengthMbufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pLengthMbufPtr);
    length += IX_OSAL_MBUF_MLEN (pLengthMbufPtr);
    }   

    /* Check at least one is set, do not support no auth and no crypt */
    if (opAuth||opCrypt) 
    {
    if (opCrypt)
    {
        /* Check if Mbuf is longer than cryptStartOffset+cryptDataLen */
        if (length < cipherLen)
        {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG(
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT, 
            "Source Mbuf is short = %d\ncryptStartOffset+cryptDataLen is %d\n\n",
            length,cipherLen,0,0,0,0);  
        return IX_CRYPTO_ACC_STATUS_FAIL;
        }
    } /* End of Crypt mode check */
    
    if (opAuth)
    {
        /* Check if the AuthDataLen is greater than 65471 or 0xFFBF  
         * refer to NPE FS */
        if (IX_CRYPTO_MAX_AUTH_LENGTH < (UINT32)(authDataLen))
        {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "Authentication data length is \
            %d, greater than IX_CRYPTO_MAX_AUTH_LENGTH of %d",
            authDataLen,IX_CRYPTO_MAX_AUTH_LENGTH,0,0,0,0);
        return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* End of Max auth length check */

        /* Check if Mbuf is longer than authStartOffset+authDataLen */
        if (length <authLen)
        {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG(
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT, 
            "Source Mbuf is short = %d\n \
            authStartOffset+authDataLen is %d\n\n",
            length,authLen,0,0,0,0);  
        return IX_CRYPTO_ACC_STATUS_FAIL;
        }
    } /* End of OpAuth Only Check */

    /* If we are doing both auth and crypt, check relative positions 
     * ((authStartOffset+authDataLen)>=(cryptDataLen+cryptStartOffset))&&
     * ((cryptDataLen+cryptStartOffset)>=cryptStartOffset)&&
     * (cryptStartOffset>=authStartOffset).
     * The simplified version of the relative position check above is used
     * in the actual codes below.
     */
    if (opAuth&&opCrypt)
    {
        if (((authStartOffset+authDataLen)>=(cryptDataLen+cryptStartOffset))&&
        (cryptStartOffset>=authStartOffset))
        {
        return IX_CRYPTO_ACC_STATUS_SUCCESS;
        }
        else /* End of if (relative check) */
        {
        return IX_CRYPTO_ACC_STATUS_FAIL;
        }
    } /* End of if opAuth&&opCrypt */

    /* Check for Destination Mbuf if operationMode is non-inplace */
    if (IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE != 
        (npeOperation & IX_CRYPTO_NPE_OP_TRANSFER_MODE_MASK))
    {
        if (opAuth)
        {
        /* MLEN is used instead of PKT_LEN to ensure all the data will
         * fit into one mbuf.
         */
        if (IX_OSAL_MBUF_MLEN (pDestMbuf) < (UINT32)authLen)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG(
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Dest Mbuf is short = %d\n \
                CryptStartOffset+CryptDataLen is %d\n",
                IX_OSAL_MBUF_MLEN (pDestMbuf),authLen,0,0,0,0);
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* End of DestMbuf length check for authentication*/
        } /* End of authentication mode check */
        if (opCrypt)
        {
        if (IX_OSAL_MBUF_MLEN (pDestMbuf) < (UINT32)cipherLen)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG(
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT,
                "Dest Mbuf is short = %d\n \
                CryptStartOffset+CryptDataLen is %d\n",
                IX_OSAL_MBUF_MLEN (pDestMbuf),cipherLen,0,0,0,0);
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* End of DestMbuf length check for authentication*/
        } /* End of Crypt mode check */
    } /* End of DestMbuf check */
    } /* End of if neither opAuth nor opCrypt */
    else /* Report error when neither opAuth nor opCrypt */
    {
    /* Log error message in debugging mode */
    IX_CRYPTO_ACC_LOG (
        IX_OSAL_LOG_LVL_ERROR,
        IX_OSAL_LOG_DEV_STDERR,
        "Neither authentication nor encryption is enabled",
        0,0,0,0,0,0);
    return IX_CRYPTO_ACC_STATUS_FAIL;
    } /* End of OpAuth || OpCrypt Check */
    /* Otherwise nothing more to do */
    return IX_CRYPTO_ACC_STATUS_SUCCESS;

} /* End of ixCryptoPerformBufferOffsetBoundaryCheck */
#endif /* ifndef NDEBUG */

/**
 * @fn      ixCryptoPerformCipherBlkLengthCheck
 * @brief   To check the data length against the pre-determined cipher block 
 *          length (8 or 16 bytes) stated in the Security Hardware Accelerator
 *          Function Specification.
 *
 */

#ifndef NDEBUG
IxCryptoAccStatus ixCryptoPerformCipherBlkLengthCheck (
    UINT32 npeOperation,
    UINT16 cipherBlkLen,
    UINT16 cryptStartOffset,
    UINT16 cryptDataLen)
{
    /*
     * Need to check:
     * 1. DataLen not 0 and cipherBlkLen not 0 (to protect divide by 0)
     * 2. DataLen is multiple of Cipher Block Length.
     */
    if ((0 != cryptDataLen) && (0 != cipherBlkLen))
    {
        /* No need to check for #2 above if its a CCM operation */
       if (IX_CRYPTO_NPE_OP_CCM_ENABLE_MASK
           == (IX_CRYPTO_NPE_OP_CCM_ENABLE_MASK & npeOperation))
        {
            return IX_CRYPTO_ACC_STATUS_SUCCESS;
        }

    /* Check whether cipherBlkLen is multiple of cryptDataLen */
    if (0 != (cryptDataLen%cipherBlkLen))
    {   
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT,
            "BlkLen not multiple of DataLen!\n \
            cryptDataLen = %d\n cipherBlkLen= %d\n",
            cryptDataLen,cipherBlkLen,0,0,0,0);
        return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_BLOCK_LEN;
    } /* End of Valid Block length check */
    } /* End of (NULL != cryptDataLen)&&(NULL != cipherBlkLen) */
    else /* Return error is either one of DataLen or BlkLen is zero */
    {
        /* Log error message in debugging mode */
        IX_CRYPTO_ACC_LOG (
            IX_OSAL_LOG_LVL_MESSAGE, 
            IX_OSAL_LOG_DEV_STDOUT, 
            "Invalid block length!\ncryptDataLen = %d\n cipherBlkLen= %d\n",
            cryptDataLen,cipherBlkLen,0,0,0,0);
        return IX_CRYPTO_ACC_STATUS_CIPHER_INVALID_BLOCK_LEN;
    }
        
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
} /* End of ixCryptoPerformCipherBlkLengthCheck */

#endif /* ifndef NDEBUG */


/**
 *
 * @fn ixCryptoPerformWepBufferOffsetBoundaryCheck
 * @brief   To check the boundary condition of the buffer offset provided by
 *          the client for WEP service request.
 *
 */
 
#ifndef NDEBUG
IxCryptoAccStatus 
ixCryptoPerformWepBufferOffsetBoundaryCheck (
    UINT32 npeOperation,
    IX_OSAL_MBUF *pSrcMbuf,
    IX_OSAL_MBUF *pDestMbuf,
    UINT16 startOffset,
    UINT16 dataLen,
    UINT16 icvOffset,
    UINT16 icvLen)
{
    IX_OSAL_MBUF *pTempMbufPtr;
    INT32 mbufLen;
    INT32 payloadLen = 0;    
    INT32 digestLen = 0;
    BOOL opWepCrc = FALSE;  /* True if authenticating */

    /* Check WEP data length, both ARC4 data length, data length MUST > 0  */
    if (0 == dataLen)
    {
        return IX_CRYPTO_ACC_STATUS_FAIL;    
    }
    
    /* Mask the IxCryptoNpeOperationMode CrcEnable bit to obtain the 
     * current status of Authentication 
     */
    if ((IX_CRYPTO_NPE_OP_WEP_CRC_MODE_IS_DISABLED != 
        (npeOperation & IX_CRYPTO_NPE_OP_WEP_CRC_ENABLE_MASK))
        || (IX_CRYPTO_NPE_OP_WEP_COMBINED_SERVICE_IS_DISABLED != 
        (npeOperation & IX_CRYPTO_NPE_OP_WEP_COMBINED_SERVICE_ENABLE_MASK)))
    {
        opWepCrc = TRUE;
    }
        
    /* Check data offset with ICV offset if WEP CRC is enabled. 
     * ICV should always be appended after the data payload
     */
    if (opWepCrc)
    {
        /* Check ICV offset */
        if (icvOffset < (startOffset + dataLen))
        {
            return IX_CRYPTO_ACC_STATUS_FAIL;    
        }
        
        /* Update digest length for later use */
        digestLen = (INT32) icvLen;
    }
    
    /* Get total length of chained mbuf - only the source mbuf is chained. The
     * destination mbuf will never be chained
     */
    pTempMbufPtr = pSrcMbuf;
    mbufLen = IX_OSAL_MBUF_MLEN (pSrcMbuf);
    payloadLen = (INT32)(startOffset + dataLen);
    
    while (NULL != IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pTempMbufPtr))
    {
        pTempMbufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pTempMbufPtr);
        mbufLen += IX_OSAL_MBUF_MLEN (pTempMbufPtr);
    }   

    /* Check if operationMode is inplace operation */
    if (IX_CRYPTO_NPE_OP_TRANSFER_MODE_IS_IN_PLACE == 
        (npeOperation & IX_CRYPTO_NPE_OP_WEP_TRANSFER_MODE_MASK))
    {       
        /* Check if Src Mbuf is big enough to hold the data */
        if (mbufLen < (payloadLen + digestLen))
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT, 
                "Source Mbuf is short = %d\npayload length is %d\n\n",
                mbufLen,
                (payloadLen + digestLen),
                0,0,0,0);   
            return IX_CRYPTO_ACC_STATUS_FAIL;
        }
    } /* non in-place */
    else
    {
        /* Check if Src Mbuf is big enough to hold the data */
        if (mbufLen < payloadLen)
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT, 
                "Source Mbuf is short = %d\nPayload length is %d\n\n",
                mbufLen,
                payloadLen,
                0,0,0,0);   
            return IX_CRYPTO_ACC_STATUS_FAIL;
        }
        
        /* MLEN is used instead of PKT_LEN to ensure all the data will
         * fit into one mbuf.
         */
        if (IX_OSAL_MBUF_MLEN (pDestMbuf) < (UINT32)(payloadLen + digestLen))
        {
            /* Log error message in debugging mode */
            IX_CRYPTO_ACC_LOG (
                IX_OSAL_LOG_LVL_MESSAGE, 
                IX_OSAL_LOG_DEV_STDOUT, 
                "Dest Mbuf is short = %d\nPayload length is %d\n",
                IX_OSAL_MBUF_MLEN (pDestMbuf),
                (payloadLen + digestLen),
                0,0,0,0);
            return IX_CRYPTO_ACC_STATUS_FAIL;
        } /* End of DestMbuf length check for authentication*/
    } /* End of if (inplace) */     
    
    return IX_CRYPTO_ACC_STATUS_SUCCESS;
} /* end of ixCryptoPerformWepBufferOffsetBoundaryCheck() */

#endif /* ifndef NDEBUG */


/**
 * @fn CRC_Compute
 * @brief   Computes CRC-32 on a buffer.
 * @note     pSbox will not be used in the function, it is included
 *          only for consistency with others XScale assembly 
 *          functions.     
 *
 */
void CRC_Compute (
    UINT8   *pSbox,
    IX_OSAL_MBUF *pMbuf,
    IxCryptoPerformWepState *pWepState)
{
    UINT8   *pData;
    UINT32  i;
    UINT32  j;

    pData = IX_OSAL_MBUF_MDATA (pMbuf);
    
    for (i = IX_OSAL_MBUF_MLEN(pMbuf); i != 0; i--)
    {
        j = (pWepState->crcValue ^ *pData++) & MASK_8_BIT;
        pWepState->crcValue 
            = pWepState->pCrcTable[j] ^ (pWepState->crcValue >> BITS_IN_BYTE);
    }
} /* end of function CRC_Compute */


/**
 * @fn ixCryptoPerformXScaleWepPerform
 * @brief   Performs ARC4 cryption and/or WEP ICV (32-bit CRC) computation.
 *
 */
IxCryptoAccStatus 
ixCryptoPerformXScaleWepPerform (
    UINT32    wepOperation,
    IX_OSAL_MBUF  *pMbuf,
    UINT16    wepStartOffset,
    UINT16    wepDataLength,
    UINT16    wepIcvOffset,
    UINT8    *pKey,
    UINT8    *pSbox)
{
    UINT8 *pCrc;
    IX_OSAL_MBUF tempMbuf; 
    IX_OSAL_MBUF *pTempMbuf = pMbuf;
    IxCryptoPerformWepState wepState;
    IxCryptoPerformWepFunction wepFn;
    INT32 remainMbufLen = 0;

    /* Get function pointer which match to the operation */
    wepFn = wepFunction[wepOperation];

    /* Initialize the WEP state */
    wepState.pSi = &pSbox[1];
    wepState.sboxIndex = 0;
    wepState.pCrcTable = (UINT32 *) ixCryptoAccCrcTable;
    wepState.crcValue = IX_CRYPTO_ACC_CRC_INITIAL_VALUE;

    /* If ARC4 processing is required (i.e. if this is not a CRC-only
     * operation, initialize the S-box.
     */
    if ((IX_CRYPTO_NPE_OP_WEP_VER_ICV != wepOperation) && 
        (IX_CRYPTO_NPE_OP_WEP_GEN_ICV != wepOperation))
    {
        ARC4_InitSbox (pSbox, pKey);
    }
    
    /* Reset wepIcvOffset relatively to startOffset and data length */
    wepIcvOffset = wepIcvOffset - (wepStartOffset + wepDataLength);
    
    /* Follow the mbuf chain until the start offset is reached. */
    while (IX_OSAL_MBUF_MLEN (pTempMbuf) < wepStartOffset)
    {
        wepStartOffset -= IX_OSAL_MBUF_MLEN (pTempMbuf);
        pTempMbuf = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pTempMbuf);
    }

    /* Set up the first (partial) mbuf to be processed. */
    IX_OSAL_MBUF_MLEN (&tempMbuf)  = IX_OSAL_MBUF_MLEN (pTempMbuf)  - wepStartOffset;
    IX_OSAL_MBUF_MDATA (&tempMbuf) = IX_OSAL_MBUF_MDATA (pTempMbuf) + wepStartOffset;
    IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (&tempMbuf) 
        = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pTempMbuf);
    pTempMbuf = &tempMbuf;

    /* Use the streaming processing functions to process each mbuf in
     * the chain, decrementing wepDataLength until it is less than or
     * equal to the length of the current mbuf.
     */
    while (IX_OSAL_MBUF_MLEN (pTempMbuf) < wepDataLength)
    {
        (*wepFn) (pSbox, pTempMbuf, &wepState);
        wepDataLength -= IX_OSAL_MBUF_MLEN (pTempMbuf);
        pTempMbuf = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pTempMbuf);
    }

    /* Prepare last mbuf */
    tempMbuf = *pTempMbuf;

    /* Handle the last mbuf in the chain. */
    if (0 < wepDataLength)
    {
        remainMbufLen = IX_OSAL_MBUF_MLEN (&tempMbuf) - wepDataLength;
        IX_OSAL_MBUF_MLEN (&tempMbuf) = wepDataLength;
        (*wepFn) (pSbox, &tempMbuf, &wepState);
        IX_OSAL_MBUF_MDATA (&tempMbuf) = IX_OSAL_MBUF_MDATA (&tempMbuf) + wepDataLength;
    }

    /* Do terminal CRC processing if CRC is enabled. */
    if ((IX_CRYPTO_NPE_OP_WEP_CRC_MODE_IS_DISABLED
        != (wepOperation & IX_CRYPTO_NPE_OP_WEP_CRC_ENABLE_MASK))
        || (IX_CRYPTO_NPE_OP_WEP_COMBINED_SERVICE_IS_DISABLED != 
        (wepOperation & IX_CRYPTO_NPE_OP_WEP_COMBINED_SERVICE_ENABLE_MASK)))
    {
        /* Get mbuf pointer which started at/after the last payload */
        pTempMbuf = &tempMbuf;
        
        /* Get CRC pointer in mbuf */
        while (remainMbufLen <= wepIcvOffset)
        {
            wepIcvOffset -= remainMbufLen;
            pTempMbuf = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (pTempMbuf);
            remainMbufLen = IX_OSAL_MBUF_MLEN (pTempMbuf);
        }

        pCrc = IX_OSAL_MBUF_MDATA (pTempMbuf) + wepIcvOffset;

#ifndef __LITTLE_ENDIAN
        /* Since the CRC is always computed assuming little-endian byte order, 
         * the computed CRC must be byte-swapped for big-endian applications.
         */
        wepState.crcValue 
            = (((wepState.crcValue >> IX_CRYPTO_ACC_CRC_SHIFT_24_BIT) 
                & MASK_8_BIT) << IX_CRYPTO_ACC_CRC_SHIFT_0_BIT)
                | (((wepState.crcValue >> IX_CRYPTO_ACC_CRC_SHIFT_16_BIT) 
                & MASK_8_BIT) << IX_CRYPTO_ACC_CRC_SHIFT_8_BIT)
                | (((wepState.crcValue >> IX_CRYPTO_ACC_CRC_SHIFT_8_BIT) 
                & MASK_8_BIT) << IX_CRYPTO_ACC_CRC_SHIFT_16_BIT)
                | (((wepState.crcValue >> IX_CRYPTO_ACC_CRC_SHIFT_0_BIT) 
                & MASK_8_BIT) << IX_CRYPTO_ACC_CRC_SHIFT_24_BIT);
#endif

        /* Get 1's complement of computed CRC value */
        wepState.crcValue = ~wepState.crcValue;
                               
        if (IX_CRYPTO_NPE_OP_CRYPT_IS_FORWARD 
            == (wepOperation & IX_CRYPTO_NPE_OP_WEP_CRYPT_DIR_MASK))
        {
            /* If CRC generation is being done, the 1's-complement of the
             * CRC is written to the location specified by wepCrcOffset.
             * If a combined ARC4 encrypt/CRC operation is being performed,
             * the complemented CRC is first encrypted.
             */            
            if (IX_CRYPTO_NPE_OP_WEP_ENCRYPT_GEN_ICV == wepOperation)
            {
                /* Encrypt the CRC */
                IX_OSAL_MBUF_MDATA (&tempMbuf) = (UINT8 *) &wepState.crcValue;
                IX_OSAL_MBUF_MLEN (&tempMbuf)  
                    = (INT32) IX_CRYPTO_ACC_WEP_CRC_DIGEST_32;
                ARC4_Crypt (pSbox, &tempMbuf, &wepState);
            }
            
            /* Append the CRC */
            ixOsalMemCopy (pCrc, &wepState.crcValue, IX_CRYPTO_ACC_WEP_CRC_DIGEST_32);
        }
        else /* reverse direction */
        {
            /* If CRC verification is being done, the computed CRC is
             * compared to the 1's-complement of the CRC stored at the
             * location wepCrcOffset.  If a combined ARC4 encrypt/CRC
             * operation is being done, the CRC is first decrypted.
             */
            if (IX_CRYPTO_NPE_OP_WEP_VER_ICV_DECRYPT == wepOperation)
            {
                /* Decrypt the CRC */
                IX_OSAL_MBUF_MDATA (&tempMbuf) = pCrc;
                IX_OSAL_MBUF_MLEN (&tempMbuf) 
                    = (INT32) IX_CRYPTO_ACC_WEP_CRC_DIGEST_32;
                ARC4_Crypt (pSbox, &tempMbuf, &wepState);
            }
            
            /* Compare the computed and embedded CRCs */
            if (memcmp (pCrc, 
                &wepState.crcValue,
                IX_CRYPTO_ACC_WEP_CRC_DIGEST_32))
            {
                return IX_CRYPTO_ACC_STATUS_AUTH_FAIL;
            } /* end of if (memcmp) */
        } /* end of if-else (forward direction) */
    } /* end of if CRC enabled */

    return IX_CRYPTO_ACC_STATUS_SUCCESS;   
} /* end of function ixCryptoPerformWepXscalePerform () */

