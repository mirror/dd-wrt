/**
 * @file IxAtmdDefines_p.h
 *
 * @author Intel Corporation
 * @date 17 March 2002
 *
 * @brief IxAtmdAcc Defines and tunable onstants
 *
 * This header contains the parameters used to configure atmd
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

#ifndef IXATMDDEFINES_P_H
#define IXATMDDEFINES_P_H

#include "IxAtmTypes.h"
#include "IxNpeA.h"
#include "IxOsal.h"
#include "IxQMgr.h"
#include "IxAtmdAssert_p.h"

#ifdef __vxworks

/* enable function inlining for performances */
#define INLINE __inline__

#else
#ifdef __linux

/* enable function inlining for performances */
#define INLINE __inline__

#else

/* disable function inlining */
#undef INLINE
#define INLINE  /* nothing */

#endif
#endif

/**
*
* @def IX_ATMDACC_PARAM_CHECK
*
* @brief Disable the parameters checking in IxAtmdAcc.
*
* This macro is used to control the input parameter checks in Atmd
* (checking parameters has a significant impact on datapath
* performances). This macro will improve performances
* but increase the risk of errors when using the datapath
* interface during development time.
*/
#ifndef NDEBUG
#define IX_ATMDACC_PARAMS_CHECK( statements ) statements
#else
#if((CPU==SIMSPARCSOLARIS) || (CPU==SIMLINUX))
/* This is to make sure unit test will pass */
#define IX_ATMDACC_PARAMS_CHECK( statements ) statements
#else
#define IX_ATMDACC_PARAMS_CHECK(x)
#endif
#endif

/**
*
* @def IX_ATMDACC_FULL_STATS
*
* @brief Execute statements if NDEBUG not defined.
*
*/
#ifndef NDEBUG
#define IX_ATMDACC_FULL_STATS(statements) statements
#else
#define IX_ATMDACC_FULL_STATS(x)
#endif

/**
*
* @def IX_ATMD_DEBUG_DO
*
* @brief Execute statements if NDEBUG not defined
*
*/
#ifndef NDEBUG
#define IX_ATMD_DEBUG_DO(statements) statements
#else
#define IX_ATMD_DEBUG_DO(x)
#endif



/* ------------------------------------------------------
*  memory data processing (depending on endianness on xscale)
*
* @note These macros can be empty if no conversion is needed
*       and this improve code efficiency and remove compiler
*       warnings about dead code
* -------------------------------------------------------
*/

/**
*
* @def IX_ATMDACC_CONVERT_TO_BIG_ENDIAN
*
* @brief Conversion of a word to big endian, if needed
*
* @note - If the size of an element is not a word, the
*       actual xscale-npe interface has a specific behaviour
*       by which it is necessary to take care about byte ordering
*
* @sa IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN
*/
#define IX_ATMDACC_CONVERT_TO_BIG_ENDIAN(data32Type, data)  \
     (data) = (data32Type) IX_OSAL_SWAP_BE_SHARED_LONG((UINT32) (data))

#define IX_ATMDACC_CONVERT_TO_BIG_ENDIAN16(data)  \
     (data) = (UINT16) IX_OSAL_SWAP_BE_SHARED_SHORT((UINT16) (data))


/**
*
* @def IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN
*
* @brief Conversion of a word from big endian format, if needed
*
* @note - If the size of an element is not a word, the
*       actual xscale-npe interface has a specific behaviour
*       by which it is necessary to take care about byte ordering
*
* @sa IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN
*/
#define IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN(data32Type, data)  \
     (data) = (data32Type) IX_OSAL_SWAP_BE_SHARED_LONG((UINT32) (data))

#define IX_ATMDACC_CONVERT_FROM_BIG_ENDIAN16(data)  \
     (data) = (UINT16) IX_OSAL_SWAP_BE_SHARED_SHORT((UINT16) (data))



/**
*
* @def IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS
*
* @brief Conversion of a virtaul memory address to a
*        physical memory address (npe use physical memeory
*        address)
*
* @note - if no conversion has to apply, define this macro
*        as empty
*
* @sa IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS
*/
#define IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS(data) \
    do { \
    IX_ATMDACC_ENSURE(*((UINT32 *)(data)) >= 0, "force dereference of valid pointer"); \
    (data) = (void *)IX_OSAL_MMU_VIRT_TO_PHYS((UINT32)data); } while(0)


/**
*
* @def IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS
*
* @brief Conversion of a physical memory address to a
*        virtual memory address (npe use physical memeory
*        address)
*
* @note - if no conversion has to apply, define this macro
*        as empty
*
* @sa IX_ATMDACC_CONVERT_TO_PHYSICAL_ADDRESS
*/
#define IX_ATMDACC_CONVERT_TO_VIRTUAL_ADDRESS(data) \
    do { \
    (data) = (void *)IX_OSAL_MMU_PHYS_TO_VIRT((UINT32)data); \
    IX_ATMDACC_ENSURE(*((UINT32 *)(data)) >= 0, "force dereference of valid pointer"); \
    } while (0)

/**
*
* @def IX_ATMDACC_DATA_CACHE_INVALIDATE
*
* @brief Invalidate the xscale cache: next read access
*        to memory will be done from
*
* @note - if no cache has to apply, define this macro
*        as empty
*
* @sa IX_ATMDACC_DATA_CACHE_FLUSH
*/
#define IX_ATMDACC_DATA_CACHE_INVALIDATE(data,size) \
    do { \
    IX_ATMDACC_ENSURE((data) != NULL, "null pointer passed to IX_OSAL_CACHE_INVALIDATE"); \
    IX_ATMDACC_ENSURE(*((UINT32 *)(data)) >= 0, "force dereference of valid pointer"); \
    IX_ATMDACC_ENSURE((size) > 0, "0 size passed to IX_OSAL_CACHE_INVALIDATE"); \
    IX_OSAL_CACHE_INVALIDATE((UINT32)(data),(size)); } while (0)

/**
*
* @def IX_ATMDACC_DATA_CACHE_FLUSH
*
* @brief Flush the xscale cache.to physical memory
*
* @note - if no flush has to apply, define this macro
*        as empty
*
* @sa IX_ATMDACC_DATA_CACHE_INVALIDATE
*/
#define IX_ATMDACC_DATA_CACHE_FLUSH(data,size) \
    do { \
    IX_ATMDACC_ENSURE((data) != NULL, "null pointer passed to IX_OSAL_CACHE_FLUSH"); \
    IX_ATMDACC_ENSURE(*((UINT32 *)(data)) >= 0, "force dereference of valid pointer"); \
    IX_ATMDACC_ENSURE((size) > 0, "0 size passed to IX_OSAL_CACHE_FLUSH"); \
    IX_OSAL_CACHE_FLUSH((UINT32)(data),(size)); } while (0)



/* -------------------------------------------------------
* Queue tuning
* --------------------------------------------------------
*/

/**
* @def IX_ATMDACC_TXQUEUE0_SIZE
* @brief Size of the tx queue for port 0
*/
#define IX_ATMDACC_TXQUEUE0_SIZE 16

#ifdef IX_NPE_MPHYMULTIPORT

/**
* @def IX_ATMDACC_TXQUEUE1_SIZE
* @brief Size of the tx queue for port 1
*/
#define IX_ATMDACC_TXQUEUE1_SIZE 16

/**
* @def IX_ATMDACC_TXQUEUE2_SIZE
* @brief Size of the tx queue for port 2
*/
#define IX_ATMDACC_TXQUEUE2_SIZE 16

/**
* @def IX_ATMDACC_TXQUEUE3_SIZE
* @brief Size of the tx queue for port 3
*/
#define IX_ATMDACC_TXQUEUE3_SIZE 16

/**
* @def IX_ATMDACC_TXQUEUE4_SIZE
* @brief Size of the tx queue for port 4
*/
#define IX_ATMDACC_TXQUEUE4_SIZE 16

/**
* @def IX_ATMDACC_TXQUEUE5_SIZE
* @brief Size of the tx queue for port 5
*/
#define IX_ATMDACC_TXQUEUE5_SIZE 16

/**
* @def IX_ATMDACC_TXQUEUE6_SIZE
* @brief Size of the tx queue for port 6
*/
#define IX_ATMDACC_TXQUEUE6_SIZE 16

/**
* @def IX_ATMDACC_TXQUEUE7_SIZE
* @brief Size of the tx queue for port 7
*/
#define IX_ATMDACC_TXQUEUE7_SIZE 16

/**
* @def IX_ATMDACC_TXQUEUE8_SIZE
* @brief Size of the tx queue for port 8
*/
#define IX_ATMDACC_TXQUEUE8_SIZE 16

/**
* @def IX_ATMDACC_TXQUEUE9_SIZE
* @brief Size of the tx queue for port 9
*/
#define IX_ATMDACC_TXQUEUE9_SIZE 16

/**
* @def IX_ATMDACC_TXQUEUE10_SIZE
* @brief Size of the tx queue for port 10
*/
#define IX_ATMDACC_TXQUEUE10_SIZE 16

/**
* @def IX_ATMDACC_TXQUEUE11_SIZE
* @brief Size of the tx queue for port 11
*/
#define IX_ATMDACC_TXQUEUE11_SIZE 16

#endif /* IX_NPE_MPHYMULTIPORT */

/**
*
* @def IX_ATMDACC_DOUBLE_SIZE_RXFREE_COUNT
*
* @brief Number of Rx Free queues configured with a double size
*
* These queues are more suitable for high rate traffic and reducing the
* interrupt rate.
*
*/
#define IX_ATMDACC_DOUBLE_SIZE_RXFREE_COUNT 0

/**
*
* @def IX_ATMDACC_AVERAGE_RXFREE_QUEUE_SIZE
*
* @brief Average size of a Rx Free queue
*
* This is the average of queues made from 32 entries and
* queues mde from 16 entries. Channel which want to get
* advantages of best queue size can select them using
* the "minimumQueueSize" parameter in @a ixAtmdAccRxVcConnect
*
* @sa ixAtmdAccRxVcConnect
*
*/
#define IX_ATMDACC_AVERAGE_RXFREE_QUEUE_SIZE \
    (((((1 + IX_NPE_A_QMQ_ATM_RXFREE_MAX) - (IX_NPE_A_QMQ_ATM_RXFREE_MIN + IX_ATMDACC_DOUBLE_SIZE_RXFREE_COUNT)) * 16) + \
    (IX_ATMDACC_DOUBLE_SIZE_RXFREE_COUNT * 32)) / \
    (1 + IX_NPE_A_QMQ_ATM_RXFREE_MAX - IX_NPE_A_QMQ_ATM_RXFREE_MIN))

/**
*
* @def IX_ATMDACC_TXDONE_QUEUE_SIZE
*
* @brief Tx Done Queue Size
*/
#define IX_ATMDACC_TXDONE_QUEUE_SIZE 64


/**
*
* @def IX_ATMDACC_RX_QUEUE_SIZE
*
* @brief Rx Queue Size
*/
#define IX_ATMDACC_RX_QUEUE_SIZE     64

/* ------------------------------------------------------
*  Mbuf setup and tuning
* -------------------------------------------------------
*/

/**
*
* @def IX_ATMDACC_AVERAGE_MBUF_LEN
*
* @brief Minimum size of an mbuf
*
* @note - Let the user to specify the size of the mbufs he is likely to
*        use depending on multiple parameters in the client context and
*        implementation
*
* @note - For now we use a magic number (19 cells)
*
*/
#define IX_ATMDACC_AVERAGE_MBUF_LEN (52 * 19)

/**
*
* @def IX_ATMDACC_MAX_PDU_LEN
*
* @brief Maximum size of a pdu
*
* @note - Let the user to specify the size of the pdus he is likely to
*        receive
*
* @note - For now we use a magic number (inherited from PRD requirements
*
*/
#define IX_ATMDACC_MAX_PDU_LEN (65568)

/**
*
* @def IX_ATMDACC_AVERAGE_MBUF_PER_PDU
*
* @brief Maximum number of mbufs in a pdu
*/
#define IX_ATMDACC_AVERAGE_MBUF_PER_PDU \
    (1 + (IX_ATMDACC_MAX_PDU_LEN / IX_ATMDACC_AVERAGE_MBUF_LEN))

/**
*
* @def IX_ATMDACC_MAX_RX_PDU_PENDING
*
* @brief Number of pdu pending in a rx channel
*
* @note - Let the user to specify the number of rx PDUs that
*        are alive in Atmd at any time, this depends on traffic rate
*        queue sizes, and frequency of pdu submit, scheduling, and
*        rx queue processing
*
* @note - For now we use a magic number.dereived from test tuning
*/
#define IX_ATMDACC_MAX_RX_PDU_PENDING 2

/**
*
* @def IX_ATMDACC_RX_NUMBER_OF_DESCRIPTORS
*
* @brief Number of descriptor required per Rx vc
*
* @note - Let the user to specify the number of tx PDUs that
*        are alive in Atmd at any time, this depends on traffic rate
*        queue sizes, and frequency of pdu submit, scheduling, and
*        tx done queue processing.
*
*/
#define IX_ATMDACC_RX_NUMBER_OF_DESCRIPTORS \
    (IX_ATMDACC_MAX_RX_PDU_PENDING * IX_ATMDACC_AVERAGE_MBUF_PER_PDU)

/**
*
* @def IX_ATMDACC_RX_SWQ_SIZE
*
* @brief sw queue size
*
* This evaluation of the sw queue size is done to detect
* bad parameters settings at compile time
*
*/
#define IX_ATMDACC_RX_SWQ_SIZE \
    (IX_ATMDACC_RX_NUMBER_OF_DESCRIPTORS + IX_ATMDACC_AVERAGE_RXFREE_QUEUE_SIZE)

#if IX_ATMDACC_RX_SWQ_SIZE <= 2
#undef IX_ATMDACC_RX_SWQ_SIZE
#define IX_ATMDACC_RX_SWQ_SIZE 2
#else
#if IX_ATMDACC_RX_SWQ_SIZE <= 4
#undef IX_ATMDACC_RX_SWQ_SIZE
#define IX_ATMDACC_RX_SWQ_SIZE 4
#else
#if IX_ATMDACC_RX_SWQ_SIZE <= 8
#undef IX_ATMDACC_RX_SWQ_SIZE
#define IX_ATMDACC_RX_SWQ_SIZE 8
#else
#if IX_ATMDACC_RX_SWQ_SIZE <= 16
#undef IX_ATMDACC_RX_SWQ_SIZE
#define IX_ATMDACC_RX_SWQ_SIZE 16
#else
#if IX_ATMDACC_RX_SWQ_SIZE <= 32
#undef IX_ATMDACC_RX_SWQ_SIZE
#define IX_ATMDACC_RX_SWQ_SIZE 32
#else
#if IX_ATMDACC_RX_SWQ_SIZE <= 64
#undef IX_ATMDACC_RX_SWQ_SIZE
#define IX_ATMDACC_RX_SWQ_SIZE 64
#else
#if IX_ATMDACC_RX_SWQ_SIZE <= 128
#undef IX_ATMDACC_RX_SWQ_SIZE
#define IX_ATMDACC_RX_SWQ_SIZE 128
#else
#if IX_ATMDACC_RX_SWQ_SIZE <= 256
#undef IX_ATMDACC_RX_SWQ_SIZE
#define IX_ATMDACC_RX_SWQ_SIZE 256
#else
#if IX_ATMDACC_RX_SWQ_SIZE <= 512
#undef IX_ATMDACC_RX_SWQ_SIZE
#define IX_ATMDACC_RX_SWQ_SIZE 512
#else
#if IX_ATMDACC_RX_SWQ_SIZE <= 1024
#undef IX_ATMDACC_RX_SWQ_SIZE
#define IX_ATMDACC_RX_SWQ_SIZE 1024
#else
/* the number of descriptor per rx channel exceed a limit
* (The npe cannot chain more than 255 mbufs)
*
* the solution is to reduce the size of the biggest PDU
* or increase the size of the average mbufs. or decrease
* the number of PDU outstanding
*
* see
* IX_ATMDACC_MAX_PDU_LEN
* IX_ATMDACC_AVERAGE_MBUF_LEN
* IX_ATMDACC_MAX_RX_PDU_PENDING
*
*/
#error "Cannot handle so many descriptors per RX channel"

#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#if IX_ATMDACC_RX_SWQ_SIZE > IX_NPE_A_CHAIN_DESC_COUNT_MAX
/* the number of descriptor per rx channel exceed a limit
* (The npe cannot chain more than 255 mbufs)
*
* the solution is to reduce the size of the biggest PDU
* or increase the size of the average mbufs. or decrease
* the number of PDU outstanding
*
* see
* IX_ATMDACC_MAX_PDU_LEN
* IX_ATMDACC_AVERAGE_MBUF_LEN
* IX_ATMDACC_MAX_RX_PDU_PENDING
*
*/
#error "Cannot handle so many descriptors per RX channel"

#endif

/**
*
* @def IX_ATMDACC_MAX_TX_PDU_PENDING
*
* @brief Number of pdu pending in a tx channel
*
* @note - Let the user to specify the number of rx PDUs that
*        are alive in Atmd at any time, this depends on traffic rate
*        queue sizes, and frequency of pdu submit, scheduling, and
*        tx done queue processing
*
* @note - For now we use a magic number.dereived from test tuning
*/
#define IX_ATMDACC_MAX_TX_PDU_PENDING 16

/**
*
* @def IX_ATMDACC_AVERAGE_TXVC_QUEUE_SIZE
*
* @brief Average size of a Tx Vc queue
*/
#ifdef IX_NPE_MPHYMULTIPORT
#define IX_ATMDACC_AVERAGE_TXVC_QUEUE_SIZE \
    ((IX_ATMDACC_TXQUEUE0_SIZE + \
    IX_ATMDACC_TXQUEUE1_SIZE + \
    IX_ATMDACC_TXQUEUE2_SIZE + \
    IX_ATMDACC_TXQUEUE3_SIZE + \
    IX_ATMDACC_TXQUEUE4_SIZE + \
    IX_ATMDACC_TXQUEUE5_SIZE + \
    IX_ATMDACC_TXQUEUE6_SIZE + \
    IX_ATMDACC_TXQUEUE7_SIZE + \
    IX_ATMDACC_TXQUEUE8_SIZE + \
    IX_ATMDACC_TXQUEUE9_SIZE + \
    IX_ATMDACC_TXQUEUE10_SIZE + \
    IX_ATMDACC_TXQUEUE11_SIZE) / 12)
#else
#define IX_ATMDACC_AVERAGE_TXVC_QUEUE_SIZE \
    (IX_ATMDACC_TXQUEUE0_SIZE)
#endif /* IX_NPE_MPHY */

/**
*
* @def IX_ATMDACC_TX_NUMBER_OF_DESCRIPTORS
*
* @brief Number of descriptor per tx vc
*
* @note - Let the user to specify the number of tx PDUs that
*        are alive in Atmd at any time, this depends on traffic rate
*        queue sizes, and frequency of pdu submit, scheduling, and
*        tx done queue processing.
*
* @note - For now we use a magic number. It could be twice the size
*       of the tx vc queue
*/
#define IX_ATMDACC_TX_NUMBER_OF_DESCRIPTORS \
    IX_ATMDACC_MAX_TX_PDU_PENDING

/**
*
* @def IX_ATMDACC_TX_SWQ_SIZE
*
* @brief sw queue size
*
* This evaluation of the sw queue size is done to detect
* bad parameters settings at compile time
*
*/
#define IX_ATMDACC_TX_SWQ_SIZE \
    (IX_ATMDACC_TX_NUMBER_OF_DESCRIPTORS + IX_ATMDACC_AVERAGE_TXVC_QUEUE_SIZE)

#if IX_ATMDACC_TX_SWQ_SIZE < IX_ATMDACC_TXDONE_QUEUE_SIZE
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE IX_ATMDACC_TXDONE_QUEUE_SIZE
#endif

#if IX_ATMDACC_TX_SWQ_SIZE <= 2
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE 2
#else
#if IX_ATMDACC_TX_SWQ_SIZE <= 4
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE 4
#else
#if IX_ATMDACC_TX_SWQ_SIZE <= 8
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE 8
#else
#if IX_ATMDACC_TX_SWQ_SIZE <= 16
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE 16
#else
#if IX_ATMDACC_TX_SWQ_SIZE <= 32
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE 32
#else
#if IX_ATMDACC_TX_SWQ_SIZE <= 64
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE 64
#else
#if IX_ATMDACC_TX_SWQ_SIZE <= 128
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE 128
#else
#if IX_ATMDACC_TX_SWQ_SIZE <= 256
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE 256
#else
#if IX_ATMDACC_TX_SWQ_SIZE <= 512
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE 512
#else
#if IX_ATMDACC_TX_SWQ_SIZE <= 1024
#undef IX_ATMDACC_TX_SWQ_SIZE
#define IX_ATMDACC_TX_SWQ_SIZE 1024
#else
/* the number of descriptor per tx channel exceed a limit
* (The npe cannot chain more than 255 mbufs)
*
* the solutiopn is to reduce the number of tx PDU outstanding
*
* IX_ATMDACC_MAX_TX_PDU_PENDING
*
*/
#error "Cannot handle so many descriptors per TX channel"

#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#if IX_ATMDACC_TX_SWQ_SIZE > IX_NPE_A_CHAIN_DESC_COUNT_MAX
/* the number of descriptor per tx channel exceed a limit
* (The npe cannot chain more than 255 mbufs)
*
* the solutiopn is to reduce the number of tx PDU outstanding
*
* see IX_ATMDACC_MAX_TX_PDU_PENDING
*
*/
#error "Cannot handle so many descriptors per TX channel"

#endif


/**
*
* @def IX_ATMDACC_TX_OAM_SWQ_SIZE
*
* @brief sw OAM queue size
*
* This defines the maximum number of OAM PDUs
* that may be waiting transmission on a
* port at any time.
*/
#define IX_ATMDACC_TX_OAM_SWQ_SIZE 8

/**
*
* @def IX_ATMDACC_MAX_NPE_DESCRIPTORS
*
* @brief Total number of descriptors required in the system
*
* This number estimates the number of descriptors required
* at any time. This is based on
* @li the average tx queue size
* @li the users's requirements in term of pdu sizes and mbuf sizes
* @li the sw queue characteristics
*
*/

#define IX_ATMDACC_MAX_NPE_DESCRIPTORS \
    ((IX_ATMDACC_RX_SWQ_SIZE     * IX_ATM_MAX_NUM_AAL_VCS) + \
     (IX_ATMDACC_TX_SWQ_SIZE     * IX_ATM_MAX_NUM_AAL_VCS) + \
     (IX_ATMDACC_RX_OAM_SWQ_SIZE * IX_ATM_MAX_NUM_OAM_RX_VCS) + \
     (IX_ATMDACC_TX_OAM_SWQ_SIZE * IX_ATM_MAX_NUM_OAM_TX_VCS))

/**
*
* @def IX_ATMDACC_SWQ_MAX_ENTRIES
*
* @brief Number of entries in a s/w queue
*
* @note Let the user to specify max the number of entries that a s/w queue
*       may hold. This is used only to check that requested queue sizes
*       are below this limit.
*
* @note For now we use a magic number, in reality requested queue sizes will
*       be much less than this.
*/
#define IX_ATMDACC_SWQ_MAX_ENTRIES 1024

/**
*
* @def IX_ATMDACC_QMGR_MAX_QUEUE_SIZE
*
* @brief Maximum size of a QMgr queue
*/
#define IX_ATMDACC_QMGR_MAX_QUEUE_SIZE 128

/**
*
* @def IX_ATMDACC_QMGR_OAM_FREE_QUEUE_SIZE
*
* @brief Maximum size of a QMgr queue for OAM Rx free
*/
#define IX_ATMDACC_QMGR_OAM_FREE_QUEUE_SIZE 16

/**
*
* @def IX_ATMDACC_RX_OAM_SWQ_SIZE
*
* @brief sw OAM queue size
* This defines the maximum number of OAM PDUs
* that may be queued awaiting rx handling at any time.
*
* Following the assumption that the maximum possible
* is defined by the size of the rx queue, the size of the
* rx free queue, and 1 pdu in transit in the NPE.
*
* For symetry some attempt is made to relate the size to the
* size of the transmit channels, anticipating OAM loopback.
* However this dimension only takes effect if atmd is configured
* such that the total transmit capacity is less that the maximum
* OAM receive capacity.
*/
#define IX_ATMDACC_MAX_OAM_RX_POSSIBLE \
( IX_ATMDACC_RX_QUEUE_SIZE + \
  IX_ATMDACC_QMGR_OAM_FREE_QUEUE_SIZE + 1)

#define IX_ATMDACC_MAX_OAM_RX_REQUIRED \
(IX_ATMDACC_TX_OAM_SWQ_SIZE * IX_ATM_MAX_NUM_AAL_VCS )

#if (IX_ATMDACC_MAX_OAM_RX_REQUIRED < IX_ATMDACC_MAX_OAM_RX_POSSIBLE)
#define  IX_ATMDACC_RX_OAM_SWQ_SIZE IX_ATMDACC_MAX_OAM_RX_REQUIRED
#else
#define IX_ATMDACC_RX_OAM_SWQ_SIZE IX_ATMDACC_MAX_OAM_RX_POSSIBLE
#endif


/**
 * @def IX_ATMDACC_AAL0_RX_TIMEOUT [ 0 - 255 ]
 *
 *
 * @brief The time in milliseconds during which received AAL0 cells
 * are accumulated into a PDU. An OAM PDU will be passed to the client
 * either when, the rx mbuf is filled or when this timer expires.
 * A value of 0 disables the timer.
 * The timer resolution is 1ms (a value of 1 results in an
 * expiry in the range 0 - 1 ms).
 */
#define IX_ATMDACC_AAL0_RX_TIMEOUT 4 /* with 2K mbufs  timeout will occur if
                                        the rate falls below 4 Mbits/s */





#endif /* IXATMDDEFINES_P_H */


