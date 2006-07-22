/**
 * @file IxHssAccPDM_p.h
 *
 * @author Intel Corporation
 * @date  14  Dec 2001
 *
 * @brief This file contains the private API of the HSS Access Packetised 
 * Descriptor Manager
 *
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
*/

/**
 * @defgroup IxHssAccPDM_p IxHssAccPDM_p
 *
 * @brief IXP400 HSS Access Packetised Descriptor Manager 
 * 
 * @{
 */
#ifndef IXHSSACCPDM_P_H
#define IXHSSACCPDM_P_H 

/*
 * User defined header files 
 */
#include "IxHssAcc.h"
#include "IxNpeA.h"
#include "IxOsal.h"

/**
 * @def IX_HSSACC_PKT_DRV_DMA_MALLOC
 *
 * @brief HssAccess abstraction to the real macro in IxOsal.h
 *
 * @param unsigned [in] size - memory size
 *
 * This macro allocates non-cached memory
 *
 * @return void *
 */
#define IX_HSSACC_PKT_DRV_DMA_MALLOC(size) IX_OSAL_CACHE_DMA_MALLOC(size)

/**
 * @def IX_HSSACC_PKT_DATA_CACHE_INVALIDATE
 *
 * @brief HssAccess abstraction to the real macro in IxOsal.h
 *
 * @param UINT32 * [in] addr - address to operate on
 * @param UINT32 [in] size - the size of the memory to operate on
 *
 * This macro invalidates the memory at addr
 *
 * @return none
 */
#define IX_HSSACC_PKT_DATA_CACHE_INVALIDATE(addr, size) IX_OSAL_CACHE_INVALIDATE(addr, (UINT32)size)

/**
 * @def IX_HSSACC_PKT_DATA_CACHE_FLUSH
 *
 * @brief HssAccess abstraction to the real macro in IxOsal.h
 *
 * @param UINT32 * [in] addr - address to operate on
 * @param UINT32 [in] size - the size of the memory to operate on
 *
 * This macro flushes the memory at addr
 *
 * @return none
 */
#define IX_HSSACC_PKT_DATA_CACHE_FLUSH(addr, size) IX_OSAL_CACHE_FLUSH(addr, (UINT32)size)

/**
 * @def IX_HSSACC_PKT_MMU_PHY_TO_VIRT
 *
 * @brief HssAccess abstraction to the real macro in IxOsal.h
 *
 * @param UINT32 * [in] addr - address to operate on
 *
 * This macro converts a physical address to a virtual one 
 *
 * @return UINT32 *
 */
#define IX_HSSACC_PKT_MMU_PHY_TO_VIRT(addr) IX_OSAL_MMU_PHYS_TO_VIRT(addr)

/**
 * @def IX_HSSACC_PKT_MMU_VIRT_TO_PHY
 *
 * @brief HssAccess abstraction to the real macro in IxOsal.h
 *
 * @param UINT32 * [in] addr - address to operate on
 *
 * This macro converts a virtual address to a physical one 
 *
 * @return UINT32 *
 */
#define IX_HSSACC_PKT_MMU_VIRT_TO_PHY(addr) IX_OSAL_MMU_VIRT_TO_PHYS(addr)

/**
 * @def IX_HSSACC_ENDIAN_MBUF_SWAP
 *
 * @brief Generic endianess conversion macro for MBUF header
 *
 * @param IX_OSAL_MBUF * [in] mbufPtr - pointer to MBUF header to operate on
 *
 * This macro performs endianess conversion on a MBUF header 
 *
 * @return none
 */
#define IX_HSSACC_ENDIAN_MBUF_SWAP(mbufPtr) \
{\
      IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) = (IX_OSAL_MBUF *) (IX_OSAL_SWAP_BE_SHARED_LONG( \
          (UINT32) IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr))); \
      IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(mbufPtr) = (IX_OSAL_MBUF *) (IX_OSAL_SWAP_BE_SHARED_LONG( \
          (UINT32) IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(mbufPtr))); \
      IX_OSAL_MBUF_MDATA(mbufPtr) = (UINT8 *) (IX_OSAL_SWAP_BE_SHARED_LONG( \
          (UINT32) IX_OSAL_MBUF_MDATA(mbufPtr))); \
      IX_OSAL_MBUF_MLEN(mbufPtr) = (INT32) (IX_OSAL_SWAP_BE_SHARED_LONG( \
          (UINT32) IX_OSAL_MBUF_MLEN(mbufPtr))); \
      IX_OSAL_MBUF_PKT_LEN(mbufPtr) = (INT32) (IX_OSAL_SWAP_BE_SHARED_LONG( \
          (UINT32) IX_OSAL_MBUF_PKT_LEN(mbufPtr))); \
}

/**
 * @def IX_HSSACC_ENDIAN_UNCHAINED_MBUF_SWAP
 *
 * @brief Simplified endianess conversion macro for unchained MBUF 
 * header
 *
 * @param IX_OSAL_MBUF * [in] mbufPtr - pointer to MBUF header to operate on
 *
 * This macro performs optimized endianess conversion on an unchained 
 * MBUF header 
 *
 * @return none
 */
#define IX_HSSACC_ENDIAN_UNCHAINED_MBUF_SWAP(mbufPtr) \
{\
      IX_OSAL_MBUF_MLEN(mbufPtr) = IX_OSAL_MBUF_PKT_LEN(mbufPtr) = \
          (INT32) (IX_OSAL_SWAP_BE_SHARED_LONG((UINT32) IX_OSAL_MBUF_MLEN(mbufPtr))); \
      IX_OSAL_MBUF_MDATA(mbufPtr) = (UINT8 *) (IX_OSAL_SWAP_BE_SHARED_LONG( \
          (UINT32) IX_OSAL_MBUF_MDATA(mbufPtr))); \
}

/**
 * @struct IxHssAccPDMDescriptor
 * 
 * @brief The IxHssAccPDMDescriptor (aka descriptor) is used as a pointer to 
 * data of, and to hold info for packets. 
 */
typedef struct 
{
    IxNpeA_NpePacketDescriptor npeDesc;  /**< The NPE Descriptor which the NPE uses 
				              to write packet info to.  */
    IxHssAccHssPort hssPortId;   /**< The hssPortId which will be used to 
				      identify which clients this relates to */
    IxHssAccHdlcPort hdlcPortId; /**< The hdlcPortId which will be used to 
				      identify which clients this relates to */
    BOOL descInUse;              /**< Boolean field which indicates whether 
				      the descriptor is being used or not */ 
    unsigned descIndex;          /**< This descriptors index within its pool */
} IxHssAccPDMDescriptor;


/**
 * @enum IxHssAccPDMPoolType
 * 
 * @brief The IxHssAccPDMPoolType is an enum which is used to indicate 
 * if a Descriptor pool is being used for Rx or Tx  
 */
typedef enum
{
    IX_HSSACC_PDM_TX_POOL, /**< The TX pool */
    IX_HSSACC_PDM_RX_POOL  /**< The RX pool */
} IxHssAccPDMPoolType;


/**
 * @fn IX_STATUS ixHssAccPDMDescGet (IxHssAccHssPort hssPortId, 
                                     IxHssAccHdlcPort hdlcPortId,
				     IxHssAccPDMPoolType poolType, 
				     IxHssAccPDMDescriptor **desc)
 *
 * @brief This is the function which returns a descriptor when requested. 
 *
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number of the client 
 * who is requesting a descriptor is on
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the number of the HDLC port who 
 * is requesting a descriptor is on and it corresponds to the physical E1/T1 
 * channel i.e. 0, 1, 2, 3 
 * @param IxHssAccPDMPoolType poolType (in) -  This is the type of pool from 
 * which the descriptor is requested. hssPortId, hdlcPortId and poolType 
 * uniquely identify a descriptor pool.
 * @param IxHssAccPDMDescriptor **desc (out) - This is the pointer to a 
 * pointer of a descripor into which the descriptors address is written to.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully 
 *                                   due to a resource error
 */
IX_STATUS ixHssAccPDMDescGet (IxHssAccHssPort hssPortId, 
			      IxHssAccHdlcPort hdlcPortId, 
			      IxHssAccPDMPoolType poolType,
			      IxHssAccPDMDescriptor **desc);


/**
 * @fn void ixHssAccPDMDescFree (IxHssAccPDMDescriptor *desc,
                                 IxHssAccPDMPoolType poolType)
 *
 * @brief  This function frees a descriptor when it is no longer needed for 
 * use in the short term.
 *
 * @param IxHssAccPDMDescriptor *desc (in) - This is a pointer to the 
 * descriptor to be freed.
 * @param IxHssAccPDMPoolType poolType (in) -  This is the type of pool from 
 * which the descriptor that is to be freed is from. 
 * @return void 
 */
void ixHssAccPDMDescFree (IxHssAccPDMDescriptor *desc, 
			  IxHssAccPDMPoolType poolType);


/**
 * @fn IX_STATUS ixHssAccPDMInit (void)
 *
 * @brief  This function initialises all resources for all descriptor pools 
 * and descriptors contained in these pools.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully 
 *                                   due to a resource error
 */
IX_STATUS ixHssAccPDMInit (void);


/**
 * @fn unsigned ixHssAccPDMNumDescInUse (IxHssAccHssPort hssPortId, 
                                         IxHssAccHdlcPort hdlcPortId);
 *
 * @brief  This function is used to find out the number of descriptors in a pool 
 * still in use.
 *
 * @param IxHssAccHssPort hssPortId (in) - This is the HSS port number of the client
 * who is being checked
 * @param IxHssAccHdlcPort hdlcPortId (in) - This is the number of the HDLC port of 
 * the client and it corresponds to the physical E1/T1 channel i.e. 0, 1, 2, 3 
 *
 * @return unsigned
 *         - number of descriptors in use for the specified hssPortId/hdlcPortId
 */
unsigned ixHssAccPDMNumDescInUse (IxHssAccHssPort hssPortId, 
				  IxHssAccHdlcPort hdlcPortId);

/**
 * @fn void ixHssAccPDMShow (void);
 *
 * @brief  This function is used to print the stats of the PDM module.
 * 
 * @return void 
 */
void ixHssAccPDMShow (void);


/**
 * @fn void ixHssAccPDMStatsInit (IxHssAccHssPort hssPortId, 
                                  IxHssAccHdlcPort hdlcPortId);
 *
 * @brief  This function is used to initialise the stats of the PDM module.
 * 
 * @param IxHssAccHssPort hssPortId - The hdlcPortId to initailise stats for
 * @param IxHssAccHdlcPort hdlcPortId - The hssPortId to initailise stats for
 * @return void 
 */
void ixHssAccPDMStatsInit (IxHssAccHssPort hssPortId, 
			   IxHssAccHdlcPort hdlcPortId);

/**
 * @fn IX_OSAL_MBUF *ixHssAccPDMMbufToNpeFormatConvert (IX_OSAL_MBUF *mbufPtr)
 *
 * @brief  This function is used to convert chained mbufs to NPE format. This
 * includes virtual to physical address space processing and memory cache
 * processing (flush memory)
 * 
 * @param IX_OSAL_MBUF *mbufPtr - mbuf chain to convert
 *
 * @return IX_OSAL_MBUF * 
 */
IX_OSAL_MBUF *
ixHssAccPDMMbufToNpeFormatConvert (IX_OSAL_MBUF *mbufPtr);

/**
 * @fn IX_OSAL_MBUF *ixHssAccPDMMbufFromNpeFormatConvert (IX_OSAL_MBUF *mbufPtr,
                                                          BOOL invalidateCache)
 *
 * @brief  This function is used to convert chained mbufs from NPE format. This
 * includes physical to virtual address space processing and memory cache
 * processing (invalidate memory)
 * 
 * @param IX_OSAL_MBUF *mbufPtr - mbuf chain to convert
 * @param BOOL invalidateCache - option to invalidate mbuf memory cache area
 *
 * @return IX_OSAL_MBUF * 
 */
IX_OSAL_MBUF *
ixHssAccPDMMbufFromNpeFormatConvert (IX_OSAL_MBUF *mbufPtr, BOOL invalidateCache);


/**
 * @fn IX_STATUS ixHssAccPDMUninit (void)
 *
 * @brief  This function Uninitialises all resources for all descriptor pools 
 * and descriptors contained in these pools.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully 
 *                                   due to a resource error
 */

IX_STATUS ixHssAccPDMUninit (void);


#endif /* IXHSSACCPDM_P_H*/

/**
 * @} defgroup  IxHssAccPDM_p
 */
