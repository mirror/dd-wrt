/**
 * @file IxHssAccCommon_p.h
 * 
 * @author Intel Corporation
 * @date 10-DEC-2001
 *
 * @brief This file contains the private API of the HSS Access Common
 * module
 *
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

/**
 * @defgroup IxHssAccCommon_p IxHssAccCommon_p
 *
 * @brief The private API for the HssAccess Common module
 * 
 * @{
 */

#ifndef IXHSSACCCOMMON_P_H
#define IXHSSACCCOMMON_P_H

#include "IxHssAcc.h"
#include "IxNpeMh.h"

#include "IxNpeA.h"
#include "IxOsal.h"
/*
 * Global variables
 */
extern IxHssAccHssPort hssPortMax; /**< Number of HSS ports available */

/*
 * Typedefs
 */
 /**
*
* @typedef  IxHssAccNpeBuffer
*
* @brief NPE Shared region structure for hssAcc of IX_OSAL_BUF buffer
*
*/
typedef struct 
{
    UINT32  *ixp_next;      /* Ptr to next buffer */
    UINT16  ixp_len;        /* Buffer length */
    UINT16  ixp_pkt_len;    /* Packet length */
    UINT8   *ixp_data;      /* Ptr to data buffer in SDRAM */
    UINT8   status;
    UINT8   error_count;
    UINT16  reserved0;      /* reserved field */
    UINT32  reserved1;      /* reserved field */
    UINT32  reserved2;      /* reserved field */
    UINT32  reserved3;      /* reserved field */
    UINT32  reserved4;      /* reserved field */

} IxHssAccNpeBuffer;

/**
 * #defines for function return types, etc.
 */

/**
 * @def IX_HSSACC_MAX_CHAN_TIMESLOTS
 *
 * @brief The number of timeslots supported for the channelised
 * service
 */


#ifdef IX_NPE_HSS_MPHY4PORT 
#define IX_HSSACC_MAX_CHAN_TIMESLOTS 16
#else
#define IX_HSSACC_MAX_CHAN_TIMESLOTS 32 
#endif



/**
 * @def IX_HSSACC_BYTES_PER_WORD
 *
 * @brief Number of bytes per word
 */
#define IX_HSSACC_BYTES_PER_WORD 4

/**
 * @def IX_HSSACC_LUT_BITS_PER_TS
 *
 * @brief The number of bits each HSS timeslot consumes in the HSS Co-p LUT
 */
#define IX_HSSACC_LUT_BITS_PER_TS     2

/**
 * @def IX_HSSACC_LUT_BITS_PER_WORD
 *
 * @brief The number of bits per HSS Co-p LUT word entry
 */
#define IX_HSSACC_LUT_BITS_PER_WORD  32

/**
 * @def IX_HSSACC_LUT_WORDS_PER_LUT
 *
 * @brief The total number of words in the HSS Co-p LUT to represent all
 * timeslots with the HSS TDM stream
 */
#define IX_HSSACC_LUT_WORDS_PER_LUT  ((IX_HSSACC_TSLOTS_PER_HSS_PORT * IX_HSSACC_LUT_BITS_PER_TS) / IX_HSSACC_LUT_BITS_PER_WORD)

/**
 * @def IX_HSSACC_SINGLE_HSS_PORT
 *
 * @brief The max number of ports available when only HSS port 0 is 
 * enabled
 */
#define IX_HSSACC_SINGLE_HSS_PORT  IX_HSSACC_HSS_PORT_1

/**
 * @def IX_HSSACC_DUAL_HSS_PORTS
 *
 * @brief The max number of ports available when both HSS ports are 
 * enabled
 */
#define IX_HSSACC_DUAL_HSS_PORTS  IX_HSSACC_HSS_PORT_MAX

/**
 * @def IX_HSSACC_LUT_TS_MASK
 *
 * @brief The mask used to extract HSS timeslot usage from HSS Co-p LUT 
 * word entry
 */
#define IX_HSSACC_LUT_TS_MASK  	     0x00000003

/* ------------------------------------------
   The following are HSS Co-p related defines
   ------------------------------------------ */
/**
 * @def IX_HSSACC_COM_HSSPCR_FTYPE_OFFSET
 *
 * @brief The HSS co-processor register bit offset for FTYPE in
 * HSSTXPCR/HSSRXPCR
 */
#define IX_HSSACC_COM_HSSPCR_FTYPE_OFFSET    30

/**
 * @def IX_HSSACC_COM_HSSPCR_FENABLE_OFFSET
 *
 * @brief The HSS co-processor register bit offset for FENABLE in
 * HSSTXPCR/HSSRXPCR
 */
#define IX_HSSACC_COM_HSSPCR_FENABLE_OFFSET  28

/**
 * @def IX_HSSACC_COM_HSSPCR_FEDGE_OFFSET
 *
 * @brief The HSS co-processor register bit offset for FEDGE in
 * HSSTXPCR/HSSRXPCR
 */
#define IX_HSSACC_COM_HSSPCR_FEDGE_OFFSET    27

/**
 * @def IX_HSSACC_COM_HSSPCR_DEDGE_OFFSET
 *
 * @brief The HSS co-processor register bit offset for DEDGE in
 * HSSTXPCR/HSSRXPCR
 */
#define IX_HSSACC_COM_HSSPCR_DEDGE_OFFSET    26

/**
 * @def IX_HSSACC_COM_HSSPCR_CLKDIR_OFFSET
 *
 * @brief The HSS co-processor register bit offset for CLKDIR in
 * HSSTXPCR/HSSRXPCR
 */
#define IX_HSSACC_COM_HSSPCR_CLKDIR_OFFSET   25

/**
 * @def IX_HSSACC_COM_HSSPCR_FRAME_OFFSET
 *
 * @brief The HSS co-processor register bit offset for FRAME in
 * HSSTXPCR/HSSRXPCR
 */
#define IX_HSSACC_COM_HSSPCR_FRAME_OFFSET    24

/**
 * @def IX_HSSACC_COM_HSSPCR_HALF_OFFSET
 *
 * @brief The HSS co-processor register bit offset for HALF in
 * HSSTXPCR/HSSRXPCR
 */
#define IX_HSSACC_COM_HSSPCR_HALF_OFFSET     21

/**
 * @def IX_HSSACC_COM_HSSPCR_DPOL_OFFSET
 *
 * @brief The HSS co-processor register bit offset for DPOL in
 * HSSTXPCR/HSSRXPCR
 */
#define IX_HSSACC_COM_HSSPCR_DPOL_OFFSET     20

/**
 * @def IX_HSSACC_COM_HSSPCR_BITEND_OFFSET
 *
 * @brief The HSS co-processor register bit offset for BITEND in
 * HSSTXPCR/HSSRXPCR
 */
#define IX_HSSACC_COM_HSSPCR_BITEND_OFFSET   19

/**
 * @def IX_HSSACC_COM_HSSPCR_ODRAIN_OFFSET
 *
 * @brief The HSS co-processor register bit offset for ODRAIN in HSSTXPCR
 */
#define IX_HSSACC_COM_HSSPCR_ODRAIN_OFFSET   18

/**
 * @def IX_HSSACC_COM_HSSPCR_FBIT_OFFSET
 *
 * @brief The HSS co-processor register bit offset for FBIT in
 * HSSTXPCR/HSSRXPCR 
 */
#define IX_HSSACC_COM_HSSPCR_FBIT_OFFSET     17

/**
 * @def IX_HSSACC_COM_HSSPCR_ENABLE_OFFSET
 *
 * @brief The HSS co-processor register bit offset for ENABLE in HSSTXPCR
 */
#define IX_HSSACC_COM_HSSPCR_ENABLE_OFFSET   16

/**
 * @def IX_HSSACC_COM_HSSPCR_56KTYPE_OFFSET
 *
 * @brief The HSS co-processor register bit offset for 56KTYPE in HSSTXPCR
 */
#define IX_HSSACC_COM_HSSPCR_56KTYPE_OFFSET  13

/**
 * @def IX_HSSACC_COM_HSSPCR_UTYPE_OFFSET
 *
 * @brief The HSS co-processor register bit offset for UTYPE in HSSTXPCR
 */
#define IX_HSSACC_COM_HSSPCR_UTYPE_OFFSET    11

/**
 * @def IX_HSSACC_COM_HSSPCR_FBTYPE_OFFSET
 *
 * @brief The HSS co-processor register bit offset for FBTYPE in HSSTXPCR
 */
#define IX_HSSACC_COM_HSSPCR_FBTYPE_OFFSET   10

/**
 * @def IX_HSSACC_COM_HSSPCR_56KEND_OFFSET
 *
 * @brief The HSS co-processor register bit offset for 56KEND in HSSTXPCR
 */
#define IX_HSSACC_COM_HSSPCR_56KEND_OFFSET   9

/**
 * @def IX_HSSACC_COM_HSSPCR_56KSEL_OFFSET
 *
 * @brief The HSS co-processor register bit offset for 56KSEL in HSSTXPCR
 */
#define IX_HSSACC_COM_HSSPCR_56KSEL_OFFSET   8

/**
 * @def IX_HSSACC_COM_HSSCCR_HFIFO_OFFSET
 *
 * @brief The HSS co-processor register bit offset for HFIFO in HSSCCR
 */
#define IX_HSSACC_COM_HSSCCR_HFIFO_OFFSET    26

/**
 * @def IX_HSSACC_COM_HSSCCR_LBACK_OFFSET
 *
 * @brief The HSS co-processor register bit offset for LBACK in HSSCCR
 */
#define IX_HSSACC_COM_HSSCCR_LBACK_OFFSET    25

/**
 * @def IX_HSSACC_COM_HSSCCR_COND_OFFSET
 *
 * @brief The HSS co-processor register bit offset for COND in HSSCCR
 */
#define IX_HSSACC_COM_HSSCCR_COND_OFFSET     24

/**
 * @def IX_HSSACC_COM_HSSCLKCR_MAIN_OFFSET
 *
 * @brief The HSS co-processor register bit offset for MAIN in HSSCLKCR
 */
#define IX_HSSACC_COM_HSSCLKCR_MAIN_OFFSET   22 

/**
 * @def IX_HSSACC_COM_HSSCLKCR_NUM_OFFSET
 *
 * @brief The HSS co-processor register bit offset for NUM in HSSCLKCR
 */
#define IX_HSSACC_COM_HSSCLKCR_NUM_OFFSET    12

/**
 * @def IX_HSSACC_COM_HSSCLKCR_DENOM_OFFSET
 *
 * @brief The HSS co-processor register bit offset for DENOM in HSSCLKCR
 */
#define IX_HSSACC_COM_HSSCLKCR_DENOM_OFFSET  0
 
/**
 * @def IX_HSSACC_COM_HSSFCR_OFFSET_OFFSET
 *
 * @brief The HSS co-processor register bit offset for OFFSET in HSSFCR
 */
#define IX_HSSACC_COM_HSSFCR_OFFSET_OFFSET   16

/**
 * @def IX_HSSACC_COM_HSSFCR_OFFSET_MAX
 *
 * @brief The HSS co-processor register max value for OFFSET in HSSFCR
 */
#define IX_HSSACC_COM_HSSFCR_OFFSET_MAX   1023

/**
 * @def IX_HSSACC_COM_HSSFCR_SIZE_OFFSET
 *
 * @brief The HSS co-processor register bit offset for SIZE in HSSFCR
 */
#define IX_HSSACC_COM_HSSFCR_SIZE_OFFSET     0

/**
 * @def IX_HSSACC_COM_HSSFCR_SIZE_MAX
 *
 * @brief The HSS co-processor register max value for SIZE in HSSFCR
 */
#define IX_HSSACC_COM_HSSFCR_SIZE_MAX     1023

/**
 * @def IX_HSSACC_IX_NE_SHARED_SHORT_MASK
 *
 * @brief Mask used for sixteen bit (short) values.
 */
 #define IX_HSSACC_IX_NE_SHARED_SHORT_MASK      ((UINT16)0xFFFF)



/**
 * @def IX_HSSACC_ENUM_INVALID
 *
 * @brief Mechanism to validate the upper (MAX) and lower (0) bounds 
 * of a positive enumeration
 *
 * @param int [in] VALUE - the integer value to test
 * @param int [in] MAX - the maximum value to test against
 *
 * This macro returns TRUE if the bounds are invalid and FALSE if
 * they are okay. NOTE: MAX will be an invalid value, so check >=
 *
 * @return none
 */
#define IX_HSSACC_ENUM_INVALID(VALUE, MAX) ((((VALUE) < 0) || ((VALUE) >= (MAX))) ? TRUE : FALSE)



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
 * @def IX_HSSACC_IX_NE_SHARED(bufPtr)
 *
 * @brief Macro to extract the NPE shared region address 
 * from address of the OS dependant region of IX_OSAL_BUF buffer
 *
 * @param int [in] bufPtr - The address of the OS dependant region 
 * of IX_OSAL_BUF buffer
 *
 * @return (IxHssAccNpeBuffer *) - Pointer to the NPE shared region 
 * of IX_OSAL_BUF buffer.
 */
#define IX_HSSACC_IX_NE_SHARED(bufPtr) \
    ((IxHssAccNpeBuffer *)&((bufPtr)->ix_ne))

/**
 * @def IX_HSSACC_IX_OSAL_MBUF_FROM_IX_NE(ix_ne_ptr)
 *
 * @brief Macro to extract the OS dependant region address 
 * from the address of the NPE shared region of IX_OSAL_BUF buffer
 *
 * @param int [in] ix_ne_ptr - the address of the NPE shared region 
 * of IX_OSAL_BUF buffer
 *
 * @return (IX_OSAL_MBUF *) - Pointer to the OS dependant region
 * of IX_OSAL_BUF buffer.
 */
#define IX_HSSACC_IX_OSAL_MBUF_FROM_IX_NE(ix_ne_ptr) \
    ((IX_OSAL_MBUF *)((UINT8 *)(ix_ne_ptr) - offsetof(IX_OSAL_MBUF, ix_ne)))

/**
 * @def IX_HSSACC_IX_NE_SHARED_NEXT(bufPtr)
 *
 * @brief Macro to extract the pointer to the NPE shared region of  
 * the next IX_OSAL_BUF buffer in the chain from the address of the
 * IX_OSAL_BUF buffer
 *
 * @param int [in] bufPtr - the address of the IX_OSAL_BUF buffer
 *
 * @return (UINT32 *) - Pointer to the NPE shared region of the next
 * IX_OSAL_BUF buffer in the chain.
 */
#define IX_HSSACC_IX_NE_SHARED_NEXT(bufPtr) \
    IX_HSSACC_IX_NE_SHARED(bufPtr)->ixp_next

/**
 * @def IX_HSSACC_IX_NE_SHARED_LEN(bufPtr)
 *
 * @brief Macro to extract the length of the buffer from the address
 * of the IX_OSAL_BUF buffer. 
 *
 * @param int [in] bufPtr - the address of the IX_OSAL_BUF buffer
 *
 * @return UINT16  - Length of the IX_OSAL_BUF buffer
 */
#define IX_HSSACC_IX_NE_SHARED_LEN(bufPtr) \
    IX_HSSACC_IX_NE_SHARED(bufPtr)->ixp_len

/**
 * @def IX_HSSACC_IX_NE_SHARED_PKT_LEN(bufPtr)
 *
 * @brief Macro to extract the length of the packet from the address
 * of the IX_OSAL_BUF buffer.
 *
 * @param int [in] bufPtr - the address of the IX_OSAL_BUF buffer
 *
 * @return UINT16 - Length of the packet (composed of chained IX_OSAL_BUF 
 * buffers) stored in the NPE shared region of the IX_OSAL_BUF buffer.
 */
#define IX_HSSACC_IX_NE_SHARED_PKT_LEN(bufPtr) \
    IX_HSSACC_IX_NE_SHARED(bufPtr)->ixp_pkt_len

/**
 * @def IX_HSSACC_IX_NE_SHARED_DATA(bufPtr)
 *
 * @brief Macro to extract the data area pointer in the NPE shared region 
 * from the address of the IX_OSAL_BUF buffer
 *
 * @param int [in] bufPtr - the address of the IX_OSAL_BUF buffer
 *
 * @return (UINT8 *) - Pointer to the data.
 */
#define IX_HSSACC_IX_NE_SHARED_DATA(bufPtr) \
    IX_HSSACC_IX_NE_SHARED(bufPtr)->ixp_data

/**
 * @def IX_HSSACC_IX_NE_SHARED_STATUS(bufPtr)
 *
 * @brief Macro to extract the status in the NPE shared region from 
 * the address of the IX_OSAL_BUF buffer.
 *
 * @param int [in] bufPtr - the address of the IX_OSAL_BUF buffer
 *
 * @return UINT8 - The status is returned.
 */
#define IX_HSSACC_IX_NE_SHARED_STATUS(bufPtr) \
    IX_HSSACC_IX_NE_SHARED(bufPtr)->status

/**
 * @def IX_HSSACC_IX_NE_SHARED_ERR_CNT(bufPtr)
 *
 * @brief Macro to extract the error count in the NPE shared region 
 * from the address of the IX_OSAL_BUF buffer.
 *
 * @param int [in] bufPtr - the address of the IX_OSAL_BUF buffer
 *
 * @return UINT8 - The error count is returned.
 */
#define IX_HSSACC_IX_NE_SHARED_ERR_CNT(bufPtr) \
    IX_HSSACC_IX_NE_SHARED(bufPtr)->error_count

/**
 * @def IX_HSSACC_IX_NE_SHARED_CACHE_FLUSH(bufPtr)
 *
 * @brief HssAccess abstraction to the real macro in IxOsal.h
 *
 * @param UINT32 * [in] bufPtr - address to operate on
 *
 * This macro flushes the NPE shared region of the IX_OSAL_BUF buffer
 * pointed to by bufPtr
 *
 * @return none
 */
#define IX_HSSACC_IX_NE_SHARED_CACHE_FLUSH(bufPtr) \
  do { \
      IX_OSAL_CACHE_FLUSH(IX_HSSACC_IX_NE_SHARED(bufPtr), \
                              sizeof(IxHssAccNpeBuffer)); \
    } \
  while(0)

/**
 * @def IX_HSSACC_IX_NE_SHARED_CACHE_INVALIDATE(bufPtr)
 *
 * @brief HssAccess abstraction to the real macro in IxOsal.h
 *
 * @param UINT32 * [in] bufPtr - address of the IX_OSAL_BUF buffer
 *
 * This macro invalidates the NPE shared region of the IX_OSAL_BUF buffer  
 *
 * @return none
 */
#define IX_HSSACC_IX_NE_SHARED_CACHE_INVALIDATE(bufPtr) \
  do { \
      IX_OSAL_CACHE_INVALIDATE(IX_HSSACC_IX_NE_SHARED(bufPtr), \
                                   sizeof(IxHssAccNpeBuffer)); \
    } \
  while(0)

/**
 * @def IX_HSSACC_IX_NE_ENDIAN_SWAP(bufPtr)
 *
 * @brief Generic endianess conversion macro for NPE shared region of IX_OSAL_BUF buffer
 *
 * @param IX_OSAL_MBUF * [in] bufPtr - Pointer to the IX_OSAL_BUF buffer.
 *
 * @return none
 */
#define IX_HSSACC_IX_NE_ENDIAN_SWAP(bufPtr) \
{\
      IX_HSSACC_IX_NE_SHARED_NEXT(bufPtr) = (UINT32 *) (IX_OSAL_SWAP_BE_SHARED_LONG( \
          (UINT32) IX_HSSACC_IX_NE_SHARED_NEXT(bufPtr))); \
      IX_HSSACC_IX_NE_SHARED_DATA(bufPtr) = (UINT8 *) (IX_OSAL_SWAP_BE_SHARED_LONG( \
          (UINT32) IX_HSSACC_IX_NE_SHARED_DATA(bufPtr))); \
      IX_HSSACC_IX_NE_SHARED_LEN(bufPtr) = (UINT16) (IX_OSAL_SWAP_BE_SHARED_SHORT( \
          (UINT16) IX_HSSACC_IX_NE_SHARED_LEN(bufPtr))); \
     IX_HSSACC_IX_NE_SHARED_PKT_LEN(bufPtr) = (UINT16) (IX_OSAL_SWAP_BE_SHARED_SHORT( \
          (UINT16) IX_HSSACC_IX_NE_SHARED_PKT_LEN(bufPtr))); \
      IX_HSSACC_IX_NE_SHARED_STATUS(bufPtr) = (UINT8) IX_HSSACC_IX_NE_SHARED_STATUS(bufPtr); \
      IX_HSSACC_IX_NE_SHARED_ERR_CNT(bufPtr) = (UINT8) IX_HSSACC_IX_NE_SHARED_ERR_CNT(bufPtr); \
}

/**
 * @def IX_HSSACC_MBUF_TO_IX_NE_SWAP(bufPtr)
 *
 * @brief Generic macro for copying relevant fields from the OS dependant region
 * to the NPE shared region of IX_OSAL_BUF buffer.
 *
 * @param IX_OSAL_MBUF * [in] bufPtr - Pointer to the IX_OSAL_BUF buffer
 *
 * This macro does the following
 * - Virtual to Physical address conversion is performed on the m_Next and m_Data 
 *   values and then copied to the corresponding fields in the NPE shared region 
 *   of the IX_OSAL_BUF buffer
 * - Copies the m_Len and m_PktHdr.len fields of the OS dependant region to the 
 *  ixp_len and ixp_pkt_len fields of the NPE shared region of the IX_OSAL_BUF buffer 
 *
 * @return none
 */
#define IX_HSSACC_MBUF_TO_IX_NE_SWAP(bufPtr) \
{\
    IX_OSAL_MBUF *nextBufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(bufPtr); \
    if (nextBufPtr) {\
        IX_HSSACC_IX_NE_SHARED_NEXT(bufPtr) = (UINT32 *) \
             IX_HSSACC_PKT_MMU_VIRT_TO_PHY( IX_HSSACC_IX_NE_SHARED(nextBufPtr)); \
    } \
    else \
        IX_HSSACC_IX_NE_SHARED_NEXT(bufPtr) = NULL; \
\
    IX_HSSACC_IX_NE_SHARED_DATA(bufPtr) = (UINT8 *) \
         IX_HSSACC_PKT_MMU_VIRT_TO_PHY( IX_OSAL_MBUF_MDATA(bufPtr)); \
\
    IX_HSSACC_IX_NE_SHARED_LEN(bufPtr) = (UINT16) (IX_OSAL_MBUF_MLEN(bufPtr) & IX_HSSACC_IX_NE_SHARED_SHORT_MASK) ; \
\
    IX_HSSACC_IX_NE_SHARED_PKT_LEN(bufPtr) = (UINT16) (IX_OSAL_MBUF_PKT_LEN(bufPtr) & IX_HSSACC_IX_NE_SHARED_SHORT_MASK) ; \
}

/**
 * @def IX_HSSACC_MBUF_FROM_IX_NE_SWAP(bufPtr)
 *
 * @brief Generic macro for copying relevant fields from the NPE shared region
 * to the OS dependant region of IX_OSAL_BUF buffer.
 *
 * @param IX_OSAL_MBUF * [in] bufPtr - Pointer to the IX_OSAL_BUF buffer
 *
 * This macro does the following
 *  - Takes the ixp_next value from the NPE shared region, converts the physical 
 *  address to virtual address, calculates the address of the IX_OSAL_BUF from 
 *  this address and then copies it to the m_Next field of the OS dependant region.
 *  If the ixp_next field was NULL, the m_Next field is also set to NULL.
 *  - Takes the ixp_data value from the NPE shared region, converts the physical 
 *  address to virtual address and copies it to the m_Data field of the OS dependant
 *  region.
 *  - Copies the ixp_len and ixp_pkt_len fields of the NPE shared region to the m_Len
 *  and m_PktHdr.len fields of the OS dependant region of the IX_OSAL_BUF buffer 
 *
 * @return none
 */
#define IX_HSSACC_MBUF_FROM_IX_NE_SWAP(bufPtr) \
{\
    UINT32 *nextIxNePtr; \
    nextIxNePtr = (UINT32 *) IX_HSSACC_IX_NE_SHARED_NEXT(bufPtr) ; \
    if (nextIxNePtr) {\
        nextIxNePtr = (UINT32 *) IX_HSSACC_PKT_MMU_PHY_TO_VIRT(nextIxNePtr) ;\
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(bufPtr) = IX_HSSACC_IX_OSAL_MBUF_FROM_IX_NE(nextIxNePtr); \
    } \
    else \
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(bufPtr) = (IX_OSAL_MBUF *) 0; \
    IX_OSAL_MBUF_MDATA(bufPtr) = (UINT8 *) IX_HSSACC_PKT_MMU_PHY_TO_VIRT (\
        IX_HSSACC_IX_NE_SHARED_DATA(bufPtr)); \
    IX_OSAL_MBUF_MLEN(bufPtr) = (UINT32) IX_HSSACC_IX_NE_SHARED_LEN(bufPtr); \
    IX_OSAL_MBUF_PKT_LEN(bufPtr) = (UINT32) IX_HSSACC_IX_NE_SHARED_PKT_LEN(bufPtr); \
}


/*--------------------------------------------------------------------------
 * Queue Manager Queue entry bit field masks
 *------------------------------------------------------------------------*/

/**
 * @def IX_HSSACC_QM_Q_ADDR_MASK
 *
 * @brief Macro to mask the Address field of the Queue Manager Entry
 */
#define IX_HSSACC_QM_Q_ADDR_MASK       0xFFFFFFE0


/**
 * @def IX_HSSACC_QM_Q_CHAN_NUM_MASK
 *
 * @brief Macro to mask the Hdlc Channel number of the Queue Manager Entry
 */
#define IX_HSSACC_QM_Q_CHAN_NUM_MASK   0x0000001F   
    

/**
 * Prototypes for interface functions.
 */

/**
 * @fn IX_STATUS ixHssAccComPortInit (IxHssAccHssPort hssPortId, 
           IxHssAccConfigParams *configParams, 
	   IxHssAccTdmSlotUsage *tdmMap, 
	   IxHssAccLastErrorCallback lastErrorCallback)
 *
 * @brief This function takes the client specified parameters, configures
 * them appropriately and communicates them to NPE-A
 * 
 * @param IxHssAccHssPort hssPortId (in) - The HSS port Id. There are two
 * identical ports (0-1). 
 * @param IxHssAccConfigParams *configParams (in) - Pointer to a structure
 * containing HSS configuration parameters - clock characteristics, data
 * frame characteristics, number of HDLC ports, total channelised slots
 * etc.
 * @param IxHssAccTdmSlotUsage *tdmMap (in) - A pointer to an array of size
 * IX_HSSACC_TSLOTS_PER_HSS_PORT, defining the slot usage over the HSS port
 * @param IxHssAccLastErrorCallback lastErrorCallback (in) - Function
 * pointer to be called back by the NPE Message Handler to pass the last
 * HSS error to the client. The client will be notified of the total number
 * of HSS errors through the callbacks registered for the packetised and
 * channelised services. If the client observes this number increasing, it
 * may initiate the retrieval of the last error through the
 * ixHssAccLastErrorRetrievalInitiate interface. This interface will send a
 * read request to the NPE. When the NPE responds (in a different context),
 * the NPE Message Handler will pass the data read to the client through
 * this interface.
 *
 * Several configurables need to be written to the NPE. The IxNpeMh is used
 * to write these values. For each message sent to IxNpeMh, a response is
 * requested. This response will come back in a different context. This
 * config function will wait between sends for valid responses. If an
 * invalid one is received, the config will return an error to the client.
 *
 * @return IX_STATUS */
IX_STATUS 
ixHssAccComPortInit (IxHssAccHssPort hssPortId, 
		     IxHssAccConfigParams *configParams, 
		     IxHssAccTdmSlotUsage *tdmMap, 
		     IxHssAccLastErrorCallback lastErrorCallback);

/**
 * @fn IX_STATUS ixHssAccComLastHssErrGet (IxHssAccHssPort hssPortId)
 *
 * @brief The service interface calls this function to read the last HSS
 * error. The last error will be fed back to the client through the client
 * callback interface in the context of the NPE Message Handler.
 *
 * @param IxHssAccHssPort hssPortId (in) - The HSS port Id. There are two
 * identical ports (0-1). 
 *
 * @return IX_STATUS
 */
IX_STATUS 
ixHssAccComLastHssErrGet (IxHssAccHssPort hssPortId);

/**
 * @fn IX_STATUS ixHssAccComNpeCmdMsgSend (IxNpeMhMessage message, 
           BOOL reqResp, 
	   unsigned npeMsgId)
 *
 * @brief This function sends a command to the NPE through the ixNpeMh. If
 * reqResp is TRUE, this function will BLOCK until a response is received
 * back from the NPE via the NpeMh.
 *
 * @param IxNpeMhMessage message (in) - The Npe message to send

 * @param BOOL reqResp (in) - This value specifies whether or not a
 * response is required
 * @param unsigned npeMsgId (in) - The Npe message
 * identifier
 *
 * @return IX_STATUS
 */
IX_STATUS 
ixHssAccComNpeCmdMsgSend (IxNpeMhMessage message, 
			  BOOL reqResp, 
			  unsigned npeMsgId);

/**
 * @fn void ixHssAccComNpeCmdMsgCreate (unsigned byte0, 
          unsigned byte1, 
	  unsigned byte2, 
	  unsigned byte3,
	  unsigned data,
	  IxNpeMhMessage *pMessage)
 *
 * @brief This function creates an NPE HSS command
 *
 * @param unsigned byte0 (in) - Byte value to be assigned byte position 0
 * @param unsigned byte1 (in) - Byte value to be assigned byte position 1
 * @param unsigned byte2 (in) - Byte value to be assigned byte position 2
 * @param unsigned byte3 (in) - Byte value to be assigned byte position 3
 * @param unsigned data (in) - Data word to be sent with the command
 * @param IxNpeMhMessage *pMessage (out) - The Npe message to write to
 *
 * @return void
 */
void 
ixHssAccComNpeCmdMsgCreate (unsigned byte0, 
			    unsigned byte1, 
			    unsigned byte2, 
			    unsigned byte3,
			    unsigned data,
			    IxNpeMhMessage *pMessage);

/**
 * @fn UINT32 ixHssAccComTdmToNpeVoiceChanTranslate (IxHssAccHssPort hssPortId,
 	  UINT32 tdmSlotId) 
 *
 * @brief This function translates a TDM slot (0-127) to NPE voice channel 
 * Id (0-31) on the specified HSS port. This interface performs no error 
 * checking on the tdmSlotId passed in by client. Hence, before this 
 * interface is called, client should ensure that the tdmSlotId passed 
 * in has been configured as a channelised timeslot on the specified HSS 
 * port.
 *
 * @param IxHssAccHssPort hssPortId (in) - The HSS port Id. There are two
 * identical ports (0-1). Only port 0 will be supported.
 * @param UINT32 tdmSlotId (in) - TDM slot that is configured as
 * channelised timeslot (0-127)
 *
 * @return UINT32
 */
UINT32 
ixHssAccComTdmToNpeVoiceChanTranslate (IxHssAccHssPort hssPortId,
				       UINT32 tdmSlotId); 

/**
 * @fn BOOL ixHssAccComIsChanTimeslot (IxHssAccHssPort hssPortId,
 	  UINT32 tdmSlotId)
 *
 * @brief This function checks whether the TDM slot on the specified HSS
 * port is a channelised timeslot
 *
 * @param IxHssAccHssPort hssPortId (in) - The HSS port Id. There are two
 * identical ports (0-1). Only port 0 will be supported.
 * @param UINT32 tdmSlotId (in) - The TDM slot Id (0-127).
 *
 * @return
 *	   - TRUE The tdmSlotId is a channelised timeslot
 *	   - FALSE The tdmSlotId is a not channelised timeslot
 */
BOOL 
ixHssAccComIsChanTimeslot (IxHssAccHssPort hssPortId,
			   UINT32 tdmSlotId);

/**
 * @fn UINT32 ixHssAccComNumChanTimeslotGet (IxHssAccHssPort hssPortId)
 *
 * @brief This function returns number of channelised timeslots
 * configured for the specified HSS port
 *
 * @param IxHssAccHssPort hssPortId (in) - The HSS port Id. There are two
 * identical ports (0-1). Only port 0 will be supported.
 *
 * @return 
 *         - UINT32 The number of channelised timeslots the HSS port has
 */
UINT32 
ixHssAccComNumChanTimeslotGet (IxHssAccHssPort hssPortId);


/**
 * @fn void ixHssAccComShow (void)
 *
 * @brief This function will display the current state of the IxHssAcc Com
 * module
 *
 * @return void
 */
void 
ixHssAccComShow (void);

/**
 * @fn void ixHssAccComStatsInit (void)
 *
 * @brief This function will initialise the stats of the IxHssAcc Com module
 *
 * @return void
 */
void 
ixHssAccComStatsInit (void);

/**
 * @fn IX_STATUS ixHssAccComInit (void)
 *
 * @brief This function will initialise the IxHssAcc Com module
 *
 * @return
 *          - IX_SUCCESS Function executed successfully
 *          - IX_FAIL Function failed to execute
 */
IX_STATUS 
ixHssAccComInit (void);


/**
 * @fn IX_STATUS ixHssAccComPortUninit (IxHssAccHssPort hssPortId)
 *
 * @brief  This function Uninitialises all resources for the specified port.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully 
 *                                   due to a resource error
 */

IX_STATUS ixHssAccComPortUninit (IxHssAccHssPort hssPortId);


/**
 * @fn IX_STATUS ixHssAccComUninit ()
 *
 * @brief  This function Uninitialises all resources for HssAcc module.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 *         - IX_HSSACC_RESOURCE_ERR The function did not execute successfully 
 *                                   due to a resource error
 */

IX_STATUS ixHssAccComUninit (void);

/**
 * @fn IX_OSAL_MBUF *
    ixHssAccComMbufFromNpeFormatConvert (UINT32 qEntry, BOOL invalidateCache, UINT32 *chainCount)
 *
 * @brief Copies relevant fields from NPE shared region to OS dependant region of IX_OSAL_BUF buffer
 * so that the client can process the data.
 * 
 * This function copies the relevant fields from the NPE shared region to the OS dependant
 * region of the IX_OSAL_BUF buffer. If needed the cache is invalidated to force data to be read 
 * from physical memory. If the IX_OSAL_BUF buffers are chained the process is repeated for each
 * of the buffers in the chain. The function returns the address of the root IX_OSAL_BUF buffer 
 * and updates the chain count in chainCount.
 *                           
 * @param UINT32 qEntry (in) - The queue entry taken from the queue is passed to this function.
 * @param BOOL invalidateCache (in) - BOOL value specifies whether to invalidate the cache or not.
 * @param UINT32 *chainCount (out) - Returns the number of chained buffers
 *
 * @return (IX_OSAL_MBUF *) - Pointer to the IX_OSAL_BUF buffer
 */
IX_OSAL_MBUF *
ixHssAccComMbufFromNpeFormatConvert (UINT32 qEntry, BOOL invalidateCache, UINT32 *chainCount);

/**
 * @fn IX_STATUS ixHssAccComMbufToNpeFormatConvert (IX_OSAL_MBUF *bufPtr, UINT32 * chainCount, UINT32 *npeAddr)
 *
 * @brief This function copies the relevant fields from OS dependant region to the NPE shared region of the
 * IX_OSAL_BUF buffer so that the NPE can process the buffers.
 *
 * The function copies the relevant fields from the OS dependant region to th NPE shared region of the 
 * IX_OSAL_BUF buffer. Also endian conversion of the NPE shared region is performed. The above process is
 * repeat for each of the buffers in the chain. 
 * The function returns the number of buffers in the chain in chainCount and the address of the NPE 
 * shared region of the root IX_OSAL_BUF buffer in npeAddr.
 *
 * @param IX_OSAL_MBUF *bufPtr (in) - The address of the root IX_OSAL_BUF buffer is passed to the function.
 * @param UINT32 *chainCount (out) - Returns the number of chained buffers
 * @param UINT32 *npeAddr (out) - Returns the address of the NPE shared region of the root IX_OSAL_BUF 
 * buffer.
 *
 * @return 
 *         - IX_SUCCESS The function executed successfully
 *         - IX_FAIL The function did not execute successfully
 */
IX_STATUS 
ixHssAccComMbufToNpeFormatConvert (IX_OSAL_MBUF *bufPtr, UINT32 * chainCount, UINT32 *npeAddr); 

#endif /* IXHSSACCCOMMON_P_H */

/**
 * @} defgroup IxHssAccCommon_p
 */
