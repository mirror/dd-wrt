/**
 * @file IxCryptoAccUtilities_p.h
 *
 * @date October-03-2002
 *
 * @brief Header file for the utilities module.
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


#ifndef IXCRYPTOACCUTILITIES_P_H
#define IXCRYPTOACCUTILITIES_P_H

/*
 * Os/System dependancies.
 */
#include "IxOsal.h"
#include "IxCryptoAcc_p.h"


/**
 * @def IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER
 *
 * @brief Convert word into network order (Big Endian).
 *
 * @param UINT32 [inout] data - word to be converted.
 *
 * @return: none
 *
 */ 
#define IX_CRYPTO_CONVERT_WORD_TO_NETWORK_ORDER(data) \
        IX_OSAL_SWAP_BE_SHARED_LONG(data)


/**
 * @def IX_CRYPTO_CONVERT_WORD_TO_HOST_ORDER
 *
 * @brief Convert word into host order (Little Endian or Big Endian). 
 *
 * @param UINT32 [inout] data - words to be converted.
 *
 * @return none
 *
 */
#define IX_CRYPTO_CONVERT_WORD_TO_HOST_ORDER(data) \
        IX_OSAL_SWAP_BE_SHARED_LONG(data)

/**
 * @def IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER
 *
 * @brief Convert short word into network order (Big Endian).
 *
 * @param UINT16 [inout] data - short word to be converted.
 *
 * @return: none
 *
 */ 
#define IX_CRYPTO_CONVERT_SHORT_TO_NETWORK_ORDER(data) \
        IX_OSAL_SWAP_BE_SHARED_SHORT(data) 

/**
 * @def IX_CRYPTO_CONVERT_SHORT_TO_HOST_ORDER
 *
 * @brief Convert short word into host order (Little Endian or Big Endian). 
 *
 * @param UINT16 [inout] data - short word to be converted.
 *
 * @return none
 *
 */
#define IX_CRYPTO_CONVERT_SHORT_TO_HOST_ORDER(data) \
        IX_OSAL_SWAP_BE_SHARED_SHORT(data)

/**
 * @def IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION
 *
 * @brief To convert logical/virtual address to physical address. It will
 *        return a virtual address for the provided  physical address.
 *
 * @note Example of usage x = IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION(x)
 * 
 * @param UINT32 [in] address - virtual address to be translated.
 *
 * @return virtual address for the provided  physical address.
 *
 */
#define IX_CRYPTO_VIRTUAL_TO_PHYSICAL_TRANSLATION(address) \
             IX_OSAL_MMU_VIRT_TO_PHYS(address)


/**
 * @def IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION
 *
 * @brief To convert physical address to logical/virtual address. It will
 *        return a physical address for the provided virtual.
 *
 * @note Example of usage: x = IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION(x)
 *
 * @param UINT32 [in] address - physical address to be translated.
 *
 * @return physical address for the provided virtual address
 *
 */
#define IX_CRYPTO_PHYSICAL_TO_VIRTUAL_TRANSLATION(address) \
             IX_OSAL_MMU_PHYS_TO_VIRT(address)


/**
 * @def IX_CRYPTO_ACC_DRV_DMA_MALLOC
 *
 * @brief To allocate memory for driver use, that will be shared between
 *        XSCale and NPE.
 *
 * @note Example of usage: x = IX_CRYPTO_ACC_DRV_DMA_MALLOC(x)
 *
 * @param UINT32 [in] size - memory size in bytes to be allocated.
 *
 * @return  void * Pointer to memory that can be used between XScale and NPE's.
 *
 */
#define IX_CRYPTO_ACC_DRV_DMA_MALLOC(size) \
             IX_OSAL_CACHE_DMA_MALLOC(size)
             

/**
 * @def IX_CRYPTO_ACC_DRV_DMA_FREE
 *
 * @brief To free the memory allocated through IX_CRYPTO_ACC_DRV_DMA_MALLOC
 *        function.
 *
 * @note Example of usage: IX_CRYPTO_ACC_DRV_DMA_FREE(pData, size)
 *
 * @param UINT8 [in] *pData - memory block pointer to be freed.
 *
 * @return None
 *
 */
#define IX_CRYPTO_ACC_DRV_DMA_FREE(pData) \
             IX_OSAL_CACHE_DMA_FREE(pData)

             
/**
 * @def IX_CRYPTO_DATA_CACHE_INVALIDATE
 *
 * @brief To invalidate a cache range.
 *
 * @param UINT32 [in] address - cache address to be invalidated.
 * @param UINT32 [in] size - the size in bytes of cache to be invalidated.
 *
 * @return none
 *
 */
#define IX_CRYPTO_DATA_CACHE_INVALIDATE(address, size) \
             IX_OSAL_CACHE_INVALIDATE(address, size)


/**
 * @def IX_CRYPTO_DATA_CACHE_FLUSH
 *
 * @brief To flush a cache range to physical memory. 
 *
 * @param UINT32 [in] address - cache address to be flushed to the physical 
 *	  memory.
 * @param UINT32 [in] size - the size in bytes of cache to be flushed to the 
 *	  physical memory.
 *
 * @return none
 *
 */
#define IX_CRYPTO_DATA_CACHE_FLUSH(address, size) \
             IX_OSAL_CACHE_FLUSH(address, size)



/**
 * @fn ixCryptoUtilMbufOffsetToAddressConvert
 *
 * @brief To convert offset to data block of mbuf into 32-bit address.
 *
 * @param IX_OSAL_MBUF [in] *pMbuf - pointer to the mbuf.
 * @param INT32 [in] offset - offset of the mbuf.
 * @param BOOL [in] useDiffBuf - TRUE if non in-place operation
 *                               FALSE if in-place operation 
 *
 * @return UINT32 address of data pointer in mbuf.
 *
 *
 */
UINT32 
ixCryptoUtilMbufOffsetToAddressConvert (
    IX_OSAL_MBUF *pMbuf,
    INT32 offset,
    BOOL useDiffBuf);
                                        

/**
 * @fn ixCryptoUtilMbufToNpeFormatConvert
 *
 * @brief To convert chained mbuf to NPE format, including endianess
 *        conversion (to Big Endian) and virtual address to physical address
 *        conversion.  And then flush the data to physical memory. Besides 
 *        that, shift the ixp_data buffer pointer in NPE shared structure to 
 *        right based on shiftOffset. All the mbuf fields (buffer control
 *        structure) will remain the same, only those converted values will
 *        be assigned into NPE shared structure in IX_OSAL_MBUF.
 *
 * @param IX_OSAL_MBUF [in] *pMbuf - pointer to the mbuf data block.
 * @param UINT16 [in] shiftOffset - shift offset for ixp_data buffer pointer
 *
 * @return IxCryptoNpeBuf* - the converted NPE buffer is returned.
 *
 */
IxCryptoNpeBuf*
ixCryptoUtilMbufToNpeFormatConvert (
    IX_OSAL_MBUF *pMbuf,
    UINT16 shiftOffset);


/**
 * @fn ixCryptoUtilMbufFromNpeFormatConvert
 *
 * @brief Conversion of addresses or control information in buffer is not needed
 *        as the addresses / control information never get converted. NPE 
 *        shared structure information also do no need to be converted back, as 
 *        the information is duplicated from the control structure in buffer. 
 *        This function needs to invalidate the cache.
 *
 * @param IX_OSAL_MBUF [in] *pMbuf - pointer to the mbuf data block.
 *
 * @return IX_OSAL_MBUF* - mbuf is returned.
 *
 * 
 */
IX_OSAL_MBUF*
ixCryptoUtilMbufFromNpeFormatConvert (IX_OSAL_MBUF *pMbuf);


/**
 * @fn ixCryptoUtilNpeCryptCfgGenerate
 *
 * @brief To generate Npe Crypt Cfg word that will be written to NPE.  
 *
 * @param IxCryptoNpeCryptCfgWord [in] *pNpeCryptCfg - pointer to the 
 *        IxCryptoNpeCryptCfgWord struct.
 * @param UINT32 [out] *pNpeCryptCfgWord - NPE Crypt Cfg Word
 *
 * 
 */
void
ixCryptoUtilNpeCryptCfgGenerate (
    UINT32 *pNpeCryptCfgWord, 
    IxCryptoNpeCryptCfgWord *pNpeCryptCfg);


/**
 * @fn ixCryptoUtilNpeHashCfgGenerate
 *
 * @brief To generate Npe Hash Cfg word that will be written to NPE.  
 *
 * @param IxCryptoNpeHashCfgWord [in] *pNpeHashCfg - pointer to the 
 *        IxCryptoNpeHashCfgWord struct.
 * @param UINT32 [out] *pNpeHashCfgWord - NPE Hash Cfg Word
 *
 * 
 */
void
ixCryptoUtilNpeHashCfgGenerate (
    UINT32 *pNpeHashCfgWord, 
    IxCryptoNpeHashCfgWord *pNpeHashCfg);


/**
 * @def IX_CRYPTO_ACC_LOCK
 *
 * @brief Locks out IRQs
 *
 * @return UINT32 lockVal - lock key used by IX_CRYPTO_ACC_UNLOCK to unlock 
 *         IRQs later
 *
 */
#define IX_CRYPTO_ACC_LOCK(lockVal) lockVal = ixOsalIrqLock();


/**
 * @def IX_CRYPTO_ACC_UNLOCK
 *
 * @brief Unlock IRQs
 *
 * @param UINT32 [in] lockVal - lock key previously obtained with 
 *        IX_CRYPTO_ACC_LOCK
 *
 * @return None
 *
 */
#define IX_CRYPTO_ACC_UNLOCK(lockVal) ixOsalIrqUnlock(lockVal);


/**
 * @def IX_CRYPTO_ACC_LOG
 *
 * @brief logs a formatted message
 *
 * @param IxOsalLogLevel [in] level - identifier prefix for the message.
 * @param IxOsalLogDevice [in] device - output device
 * @param char [in] format - format string, similar to printf().
 * @param int [in] a - first argument to display.
 * @param int [in] b - second argument to display.
 * @param int [in] c - third argument to display.
 * @param int [in] d - fourth argument to display.
 * @param int [in] e - fifth argument to display.
 * @param int [in] f - sixth argument to display.
 *
 * @return none
 *
 */
#ifndef NDEBUG
#define IX_CRYPTO_ACC_LOG(level, device, format, a, b, c, d, e, f) \
             (ixOsalLog (level, device, format, a, b, c, d, e, f))
#else
#define IX_CRYPTO_ACC_LOG(level, device, format, a, b, c, d, e, f) /* do nothing */
#endif /* def NDEBUG */


/**
 * @def IX_CRYPTO_ACC_IX_NE_SHARED_LEN_MASK
 *
 * @brief Bit mask for ix_ne->ixp_len (buffer length) in IX_OSAL_MBUF. Buffer
 *        length in NPE shared structure is 16 bit only. Therefore upper 16-bit
 *        of mbuf length in IX_OSAL_MBUF will be discarded, the lower 16-bit 
 *        will be assigned into the buffer length of NPE shared structure.
 *
 */
#define IX_CRYPTO_ACC_IX_NE_SHARED_LEN_MASK   0xFFFF


/**
 * @def IX_CRYPTO_ACC_IX_NE_SHARED_PKT_LEN_MASK
 *
 * @brief Bit mask for ix_ne->ixp_pkt_len (packet length) in IX_OSAL_MBUF. 
 *        Packet length in NPE shared structure is 16 bit only. Therefore upper 
 *        16-bit of mbuf packet length in IX_OSAL_MBUF will be discarded, the 
 *        lower 16-bit will be assigned into the packet length of NPE shared 
 *        structure.
 *
 */
#define IX_CRYPTO_ACC_IX_NE_SHARED_PKT_LEN_MASK   0xFFFF


/**
 * @def IX_CRYPTO_ACC_IX_NE_SHARED
 *
 * @brief Extract NPE shared structure address from IX_OSAL_MBUF
 *
 * @param IX_OSAL_MBUF [in] pMbuf - Mbuf pointer 
 *
 * @return NPE shared structure pointer
 *
 */
#define IX_CRYPTO_ACC_IX_NE_SHARED(pMbuf) ((IxCryptoNpeBuf *)(&(pMbuf)->ix_ne))
            

/**
 * @def IX_CRYPTO_ACC_IX_NE_SHARED_NEXT
 *
 * @brief Extract next buffer pointer of NPE shared structure from IX_OSAL_MBUF
 *
 * @param IX_OSAL_MBUF [in] pMbuf - Mbuf pointer 
 *
 * @return NPE next buffer pointer
 *
 */
#define IX_CRYPTO_ACC_IX_NE_SHARED_NEXT(pMbuf) \
            (((IxCryptoNpeBuf *)(&(pMbuf)->ix_ne))->ixp_next)


/**
 * @def IX_CRYPTO_ACC_IX_NE_SHARED_DATA
 *
 * @brief Extract data buffer pointer of NPE shared structure from IX_OSAL_MBUF
 *
 * @param IX_OSAL_MBUF [in] pMbuf - Mbuf pointer 
 *
 * @return NPE data buffer pointer
 *
 */
#define IX_CRYPTO_ACC_IX_NE_SHARED_DATA(pMbuf) \
            (((IxCryptoNpeBuf *)(&(pMbuf)->ix_ne))->ixp_data)
            
            
/**
 * @def IX_CRYPTO_ACC_IX_NE_SHARED_LEN
 *
 * @brief Extract buffer length of NPE shared structure from IX_OSAL_MBUF
 *
 * @param IX_OSAL_MBUF [in] pMbuf - Mbuf pointer 
 *
 * @return NPE buffer length
 *
 */
#define IX_CRYPTO_ACC_IX_NE_SHARED_LEN(pMbuf) \
            (((IxCryptoNpeBuf *)(&(pMbuf)->ix_ne))->ixp_len)


/**
 * @def IX_CRYPTO_ACC_IX_NE_SHARED_PKT_LEN
 *
 * @brief Extract packet length of NPE shared structure from IX_OSAL_MBUF
 *
 * @param IX_OSAL_MBUF [in] pMbuf - Mbuf pointer 
 *
 * @return Packet length
 *
 */
#define IX_CRYPTO_ACC_IX_NE_SHARED_PKT_LEN(pMbuf) \
            (((IxCryptoNpeBuf *)(&(pMbuf)->ix_ne))->ixp_pkt_len)            

#endif /* ndef IXCRYPTOACCUTILITIES_P_H */
