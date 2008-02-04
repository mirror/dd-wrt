/**
 * @file IxDmaAcc_p.h
 *
 * @date 18 October 2002
 *
 * @brief Descriptor pool access, initialise and allocation for DMA access layer
 *
 * @par
 * IXP400 SW Release version 2.4
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

#ifndef IXDMAACC_P_H
#define IXDMAACC_P_H

#include "IxQMgr.h"
#include "IxQueueAssignments.h"

#define IX_DMA_MAX_TRANSFER_LENGTH 65535  /**< Maximum length for Dma transfer
                                           */
#define IX_DMA_MAX_REQUEST           16   /**< Maximum number of entries in the
                                               descriptor pool */
#define IX_DMA_CALLBACK_ID_DMADONE    0   /**< Callback Id for Dma Done
                                           */
/**
* @brief Masks for Dma Transfer Mode
*/
/**
* @def IX_DMA_MODE_INC_INC
* @brief Mask for Addressing Mode
*        Bit 28 : Source Increment Mode = 0
*        Bit 20 : Destination Increment Mode = 0
*                       31       23       15        7      0
*                        |--x---- |--x---- |------- |------|
*           Binary       00000000 00000000 00000000 00000000
*            Hex              00       00       00      00
*/
#define IX_DMA_MODE_INC_INC  0x00000000

/**
* @def IX_DMA_MODE_INC_FIX
* @brief Mask for Addressing Mode
*        Bit 28 : Source Increment Mode  = 0
*        Bit 20 : Destination Fixed Mode = 1
*                       31       23       15        7      0
*                        |--x---- |--x---- |------- |------|
*           Binary       00000000 00010000 00000000 00000000
*            Hex              00       10       00      00
*/
#define IX_DMA_MODE_INC_FIX  0x00100000

/**
* @def IX_DMA_MODE_FIX_INC
* @brief Mask for Addressing Mode
*        Bit 28 : Source Increment Mode = 1
*        Bit 20 : Destination Fixed Mode = 0
*                       31       23       15        7      0
*                        |--x---- |--x---- |------- |------|
*           Binary       00010000 00000000 00000000 00000000
*            Hex              10       00       00      00
*/
#define IX_DMA_MODE_FIX_INC  0x10000000

/**
* @def IX_DMA_MODE_FIX_FIX
* @brief Mask for Addressing Mode
*        Bit 28 : Source Increment Mode = 1
*        Bit 20 : Destination Fixed Mode = 1
*                       31       23       15        7      0
*                        |--x---- |--x---- |------- |------|
*           Binary       00010000 00010000 00000000 00000000
*            Hex              10       10       00      00
*/
#define IX_DMA_MODE_FIX_FIX  0x10100000

/**
* @def IX_DMA_MODE_COPY_CLEAR
* @brief Mask for Transfer Mode Copy and Clear Source
*        Bit 19,27   = 0
*        Bit 18,26   = 0
*                       31       23       15        7      0
*                        |---xx-- |---xx-- |------- |------|
*           Binary       00000000 00000000 00000000 00000000
*            Hex              00       00       00      00
*/
#define IX_DMA_MODE_COPY_CLEAR  0x00000000

/**
* @def IX_DMA_MODE_COPY
* @brief Mask for Transfer Mode Copy only
*        Bit 19,27   = 0
*        Bit 18,26   = 1
*                       31       23       15        7      0
*                        |---xx-- |---xx-- |------- |------|
*           Binary       00000100 00000100 00000000 00000000
*            Hex              04       04       00      00
*/
#define IX_DMA_MODE_COPY  0x04040000

/*
* @def IX_DMA_MODE_COPY_BYTE_SWAP
* @brief Mask for Transfer Mode Copy and Byte Swap
*        Bit 19,27   = 1
*        Bit 18,26   = 0
*                       31       23       15        7      0
*                        |---xx-- |---xx-- |------- |------|
*           Binary       00001000 00001000 00000000 00000000
*            Hex              08       08       00      00
*/
#define IX_DMA_MODE_COPY_BYTE_SWAP  0x08080000

/*
* @def IX_DMA_MODE_COPY_REVERSE
* @brief Mask for Transfer Mode Copy and Byte Reverse
*        Bit 19,27   = 1
*        Bit 18,26   = 1
*                       31       23       15        7      0
*                        |---xx-- |---xx-- |------- |------|
*           Binary       00001100 00001100 00000000 00000000
*            Hex              0C       0C       00      00
*/
#define IX_DMA_MODE_COPY_REVERSE  0x0C0C0000

/*
* @def IX_DMA_MODE_TRANSWIDTH_32_32
* @brief Mask for Transfer Width
*        Bit 25,24 Source 32 bits      = 1,1
*        Bit 17,16 Destination 32 bits = 1,1
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000011 00000011 00000000 00000000
*            Hex              03       03       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_32_32   0x03030000

/*
* @def IX_DMA_MODE_TRANSWIDTH_32_16
* @brief Mask for Transfer Width
*        Bit 25,24 Source 32 bits      = 1,1
*        Bit 17,16 Destination 16 bits = 1,0
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000011 00000010 00000000 00000000
*            Hex              03       02       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_32_16   0x03020000

/*
* @def IX_DMA_MODE_TRANSWIDTH_32_8
* @brief Mask for Transfer Width
*        Bit 25,24 Source 32 bits     = 1,1
*        Bit 17,16 Destination 8 bits = 0,1
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000011 00000001 00000000 00000000
*            Hex              03       01       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_32_8    0x03010000

/*
* @def IX_DMA_MODE_TRANSWIDTH_16_32
* @brief Mask for Transfer Width
*        Bit 25,24 Source 16 bits      = 1,0
*        Bit 17,16 Destination 32 bits = 1,1
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000010 00000011 00000000 00000000
*            Hex              02       03       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_16_32   0x02030000

/*
* @def IX_DMA_MODE_TRANSWIDTH_16_16
* @brief Mask for Transfer Width
*        Bit 25,24 Source 16 bits      = 1,0
*        Bit 17,16 Destination 16 bits = 1,0
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000010 00000010 00000000 00000000
*            Hex              02       02       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_16_16   0x02020000

/*
* @def IX_DMA_MODE_TRANSWIDTH_16_8
* @brief Mask for Transfer Width
*        Bit 25,24 Source 16 bits      = 1,0
*        Bit 17,16 Destination 8 bits = 0,1
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000010 00000001 00000000 00000000
*            Hex              02       01       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_16_8    0x02010000

/*
* @def IX_DMA_MODE_TRANSWIDTH_8_32
* @brief Mask for Transfer Width
*        Bit 25,24 Source 8 bits      = 0,1
*        Bit 17,16 Destination 32 bits = 1,1
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000001 00000011 00000000 00000000
*            Hex              01       03       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_8_32    0x01030000

/*
* @def IX_DMA_MODE_TRANSWIDTH_8_16
* @brief Mask for Transfer Width
*        Bit 25,24 Source 8 bits      = 0,1
*        Bit 17,16 Destination 16 bits = 1,0
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000001 00000010 00000000 00000000
*            Hex              01       02       00      00
*/

#define IX_DMA_MODE_TRANSWIDTH_8_16    0x01020000

/*
* @def IX_DMA_MODE_TRANSWIDTH_8_8
* @brief Mask for Transfer Width
*        Bit 25,24 Source 8 bits      = 0,1
*        Bit 17,16 Destination 8 bits = 0,1
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000001 00000001 00000000 00000000
*            Hex              01       01       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_8_8    0x01010000

/*
* @def IX_DMA_MODE_TRANSWIDTH_32_BURST
* @brief Mask for Transfer Width
*        Bit 25,24 Source 32 bits      = 1,1
*        Bit 17,16 Destination Burst = 0,0
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000011 00000000 00000000 00000000
*            Hex              03       00       00      00
*/

#define IX_DMA_MODE_TRANSWIDTH_32_BURST   0x03000000

/*
* @def IX_DMA_MODE_TRANSWIDTH_16_BURST
* @brief Mask for Transfer Width
*        Bit 25,24 Source 16 bits      = 1,0
*        Bit 17,16 Destination Burst   = 0,0
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000010 00000000 00000000 00000000
*            Hex              02       00       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_16_BURST    0x02000000

/*
* @def IX_DMA_MODE_TRANSWIDTH_8_BURST
* @brief Mask for Transfer Width
*        Bit 25,24 Source 8 bits     = 0,1
*        Bit 17,16 Destination Burst = 0,0
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000001 00000000 00000000 00000000
*            Hex              01       00       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_8_BURST    0x01000000

/*
* @def IX_DMA_MODE_TRANSWIDTH_BURST_32
* @brief Mask for Transfer Width
*        Bit 25,24 Source Burst        = 0,0
*        Bit 17,16 Destination 32 bits = 1,1
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000000 00000011 00000000 00000000
*            Hex              00       03       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_BURST_32   0x00030000

/*
* @def IX_DMA_MODE_TRANSWIDTH_BURST_16
* @brief Mask for Transfer Width
*        Bit 25,24 Source Burst        = 0,0
*        Bit 17,16 Destination 16 bits = 1,0
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000000 00000010 00000000 00000000
*            Hex              00       02       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_BURST_16   0x00020000

/*
* @def IX_DMA_MODE_TRANSWIDTH_BURST_8
* @brief Mask for Transfer Width
*        Bit 25,24 Source Burst      = 0,0
*        Bit 17,16 Destination 8 bits = 0,1
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000000 00000001 00000000 00000000
*            Hex              00       01       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_BURST_8    0x00010000

/*
* @def IX_DMA_MODE_TRANSWIDTH_BURST_BURST
* @brief Mask for Transfer Width
*        Bit 25,24 Source Burst      = 0,0
*        Bit 17,16 Destination Burst = 0,0
*                       31       23       15        7      0
*                        |-----xx |-----xx |------- |------|
*           Binary       00000000 00000000 00000000 00000000
*            Hex              00       00       00      00
*/
#define IX_DMA_MODE_TRANSWIDTH_BURST_BURST  0x00000000

/**
 * @def IX_DMA_ACC_MEMFREE
 *
 * @brief To free the memory allocated through IX_OSAL_CACHE_DMA_MALLOC 
 *        function.
 *
 * @note Example of usage: IX_DMA_ACC_MEMFREE(pData)
 *
 * @param UINT8 [in] *pData - memory block pointer to be freed.
 *
 */
#ifdef __vxworks

#define IX_DMA_ACC_MEMFREE(pData) cacheDmaFree (pData)

#else

#define IX_DMA_ACC_MEMFREE(pData) (pData) 	  /* Note: Do nothing in
                                                   * Linux platform. Will be
                                                   * ported to Linux platform.
                                                   */
#endif /* def __vxworks */



/**
 * @enum IxDmaReturnStatus
 * @brief Dma return status definitions
 */
typedef enum
{
    IX_DMA_DM_SUCCESS = IX_SUCCESS,  /**< DMA Transfer Success */
    IX_DMA_DM_FAIL = IX_FAIL,        /**< DMA Transfer Fail */
    IX_DMA_DM_FIFO_FULL,             /**< DMA Descriptor Pool FIFO full */
    IX_DMA_DM_FIFO_EMPTY,            /**< DMA Descriptor Pool FIFO empty */
    IX_DMA_DM_INDEX_CORRUPTED        /**< DMA Descriptor Pool Index Corrupted */
} IxDmaDescMgrStatus;

/**
 * @struct IxDmaNpeQDescriptor
 * @brief Structure for storing descriptor parameters
 */
typedef struct {
    UINT32 sourceAddress;      /**< Source address for dma transfer
                                 */
    UINT32 destinationAddress; /**< Destination address for dma transfer
                                 */
    UINT32 operationMode;      /**< Operation mode for dma transfer
                                 */
    IxDmaAccDmaCompleteCallback pDmaCallback;
                               /**< Pointer to client callback for dma transfer
                                 */
    UINT32             pad0;   /**pad bytes added to make it align to size of*/ 
    UINT32             pad1;   /** cache line */ 
    UINT32             pad2;
    UINT32             pad3;
      
} IxDmaNpeQDescriptor;

/**
 * @struct IxDmaDescriptorPool
 * @brief Structure for storing the descriptor pointers
 */
typedef struct {
    IxDmaNpeQDescriptor **pDmaNpeQDescriptor;
                           /**< Pointer to array of descriptors
                             */
    UINT32 size;           /**< Size of descriptor pool
                             */
    UINT32 allocatedCnt;   /**< Counter for descriptors in use
                             */
    UINT32 head;           /**< Ring buffer Index Head
                             */
    UINT32 tail;           /**< Ring buffer Index Tail
                             */
} IxDmaDescriptorPool;

/**
 * @struct  IxDmaAccStats
 * @brief   Data structure for statistics
 *
 */
typedef struct
{
    UINT32 successCnt;        /**< Counter for number of requests
                               * completed successfully
                               */
    UINT32 failCnt;           /**< Counter for number of requests
                               * failed
                               */
    UINT32 qOverflowCnt;      /**< Counter for number of times
                               * queue overflow
                               * (Writing DMA Request Q returns a Fail)
                               */
    UINT32 qUnderflowCnt;     /**< Counter for number of times
                               * queue underflow
                               * (Reading DMA Done Q returns a Fail)
                               */
    UINT32 qDescAddrInvalidCnt; /**< Counter for number of times
                                 * Q descriptor address received
                                 * as NULL value (invalid)
                                 */
} IxDmaAccStats;

/**
 * @brief Allocate and initialize the descriptor pool
 * @param None
 * @return @li IX_SUCCESS Notification that descriptor pool initialization is
 *         @li            succesful
 * @return @li IX_FAIL 	 Error initializing descriptor pool
 */

IX_STATUS
ixDmaAccDescriptorPoolInit (void);


IX_STATUS ixDmaAccDescriptorPoolUninit(void);
/**
 * @brief Return pointer to descriptor entry
 * @param pDescriptor Pointer to descriptor within an array
 * @return @li IX_DMA_DM_SUCCESS Free descriptor pointer succesfully returned
 * @return @li IX_DMA_DM_FAIL Invalid descriptor address
 * @return @li IX_DMA_DM_FIFO_FULL Descriptor get request when FIFO is full
 * @return @li IX_DMA_DM_INDEX_CORRUPTED Index for Desc Pool corrupted
 */
IxDmaDescMgrStatus
ixDmaAccDescriptorGet (IxDmaNpeQDescriptor **pDescriptor);

/**
 * @brief Free the oldest entry in the descriptor pool
 * @param pDescriptor Pointer to descriptor
 * @return @li IX_DMA_DM_SUCCESS Free descriptor pointer succesfully returned
 * @return @li IX_DMA_DM_FAIL Invalid descriptor address
 * @return @li IX_DMA_DM_FIFO_EMPTY Descriptor free request when FIFO is empty
 * @return @li IX_DMA_DM_INDEX_CORRUPTED Index for Desc Pool corrupted
 */

IxDmaDescMgrStatus
ixDmaAccDescriptorFree (IxDmaNpeQDescriptor *pDescriptor);

/**
 * @brief       Validate parameters for DMA transfer
 * This function will validate parameters for DMA transfer.
 *
 * @param ixDmaSourceAddr	    Starting address of DMA source.
 *                              Must be a valid IXP400 memory map address.
 * @param ixDmaDestinationAddr	Starting address of DMA destination.
 *                              Must be a valid IXP400 memory map address.
 * @param ixDmaTransferLength	The size of DMA data transfer.
 *                              The range must be from 1-65535 bytes
 * @param ixDmaTransferMode	    The DMA transfer mode
 * @param ixDmaAddressingMode	The DMA addressing mode
 * @param ixTransferWidth	    The DMA transfer width
 *
 * @return @li IX_DMA_SUCCESS 	Notification that the DMA request is succesful
 * @return @li IX_DMA_FAIL 	    IxDmaAcc not yet initialised or some internal
 *                              error has occured
 * @return @li IX_DMA_INVALID_TRANSFER_WIDTH Transfer width is not valid
 * @return @li IX_DMA_INVALID_TRANSFER_LENGTH Transfer length outside of valid
 *                                            range
 * @return @li IX_DMA_INVALID_TRANSFER_MODE Transfer Mode not valid
 * @return @li IX_DMA_REQUEST_FIFO_FULL IxDmaAcc request queue is full
 */
IxDmaReturnStatus
ixDmaAccParamsValidate(
                        UINT32 SourceAddr,
                        UINT32 DestinationAddr,
                        UINT16 TransferLength,
                        IxDmaTransferMode TransferMode,
                        IxDmaAddressingMode AddressingMode,
                        IxDmaTransferWidth TransferWidth);
/**
 * @brief This callback is registered with the queue manager for
 *        notification of Dma transfer done event
 *        Q manager calls this function when Queue is Not Empty
 * @param qId    Queue Identifier for Dma done
 * @param cbId   Callback Identifier for Dma done
 *
 * @return none
 */
void
ixDmaTransferDoneCallback (IxQMgrQId qId, IxQMgrCallbackId cbId);

/**
 * @brief Show descriptor pool statistics
 * @param None
 * @return None
 */
void
ixDmaAccDescPoolShow (void);

/**
 * @brief Free memory allocated to descriptor pool
 * @param None
 * @return None
 */
void
ixDmaAccDescPoolFree(void);
#endif /* IXDMAACC_P_H */


