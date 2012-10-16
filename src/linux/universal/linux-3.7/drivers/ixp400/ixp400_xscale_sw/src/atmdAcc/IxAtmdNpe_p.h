/**
 * @file IxAtmdNpe_p.h
 *
 * @author Intel Corporation
 * @date 17 March 2002
 *
 * @brief IxAtmdAcc Npe structures and constants
 *
 * This file contains the NPE data structures and constants which are
 * visible from Xscale.
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

#ifndef IXATMDNPE_P_H
#define IXATMDNPE_P_H

#include "IxAtmTypes.h"
#include "IxNpeA.h"

/**
 *
 * @brief Structure used to pass information to the NPE
 *
 * This structure is divided in 2 oarts
 * @li the structure filled by Xcsale and used by NPE
 * @li the structure used by IxAtmdAcc to retrieve the channel
 *     associated with this descriptor
 *
 */
typedef struct
{
    /** This structure is the NPE - Xscale
    * interface, it is used in both rx and tx transmission
    */
    union
    {
        IxNpeA_RxAtmVc rx;
        IxNpeA_TxAtmVc tx;
    } npe;

    /**
    * @struct atmd_
    * @brief structure used by IxAtmdAcc only
    */
    struct atmd_ {
#ifndef NDEBUG
    unsigned int signature;/**< Used by Xscale to check that the pointer
                           * on this part of memory is a
                           */
#endif

    unsigned int totalCell; /**< Used by Xscale when scheduling cells
                            * for the PDU (to remember the number of
                            * cells scheduled)
                            */
    IxAtmConnId connId;    /**< Used by Xscale to retrieve the channel
                            * associated with this descriptor, and detect
                            * possible channel mismatches
                            */
    unsigned int physicalAddress; /**< Used by Xscale to identify the
                            * physical address of this descriptor
                            * without permanent conversion from a logical
                            * address
                            */
    IX_OSAL_MBUF *pRootMbuf;
    } atmd;     /**< part of the descriptor used by IxAtmdAcc */
} IxAtmdAccNpeDescriptor;

#ifndef NDEBUG
#define IX_ATMDACC_DESCRIPTOR_SIGNATURE    0x55aa55aa
#endif

#define NPE_INVALID_LOGICAL_PORT           (IX_UTOPIA_MAX_PORTS+1)
#define NPE_INVALID_VPI                    (IX_ATM_MAX_VPI + 1)
#define NPE_INVALID_VCI                    (IX_ATM_MAX_VCI + 1)

#define NPE_AAL5_TYPE        IX_NPE_A_AAL_TYPE_5
#define NPE_AAL0_48_TYPE     IX_NPE_A_AAL_TYPE_0_48
#define NPE_AAL0_52_TYPE     IX_NPE_A_AAL_TYPE_0_52
#define NPE_OAM_TYPE         IX_NPE_A_AAL_TYPE_OAM
#define NPE_IGNORE           0

#define NPE_SUCCESS          0
#define NPE_RX_PDU_VALID     0

#define NPE_TX_IDLECELL_ADDRESS 0
#define NPE_TX_CELLTYPE_MASK 0x3
#define NPE_TX_CELLCOUNT_OFFSET 2
#define NPE_TX_MAXCELLS_PER_QENTRY 15
#define NPE_TX_DATACELL 0
#define NPE_TX_IDLECELL 1

/* NPE qmgr entries */
#define NPE_DESCRIPTOR_MASK 0xffffffc0

#define NPE_ADDR_ALIGN (2*IX_OSAL_CACHE_LINE_SIZE)  /* align to 64-byte */
#define NPE_SHUTDOWN_ACK_SHIFT     24

#define NPE_TX_SHUTDOWN_ACK 0x00000008
#define NPE_TX_SHUTDOWN_ACK_MASK 0x0000000c
#define NPE_TX_SHUTDOWN_ACK_PORT_MASK 0xff000000

#define NPE_RX_TYPE_MASK              0x0000000c
#define NPE_RX_SHUTDOWN_ACK           0x00000008
#define NPE_RX_SHUTDOWN_ACK_VCID_MASK 0xff000000
#define NPE_RX_DESCRIPTOR             0x00000000

/* NPE Status */
#define NPE_MSG_RXFREEUNDERFLOW_ID    0
#define NPE_MSG_RXOVERFLOW_ID         4

/* NPE fifo */

#define NPE_MSG_COMMAND_BIT_OFFSET    24
#define NPE_RESP_REQ_ON               0x00FF0000
#define NPE_RESP_REQ_OFF              0x00000000

#define NPE_RESP_ID_MASK              0xFF000000
#define NPE_RESP_STATUS_MASK          0x000000FF
#define NPE_RESP_OFFSET_MASK          0x000000FF

#define NPE_UT_STATUS_UPLOAD_EXPECTED_ID \
    (IX_NPE_A_MSSG_ATM_UTOPIA_STATUS_UPLOAD << NPE_MSG_COMMAND_BIT_OFFSET)

#define NPE_UT_CONFIG_LOAD_EXPECTED_ID \
    (IX_NPE_A_MSSG_ATM_UTOPIA_CONFIG_LOAD << NPE_MSG_COMMAND_BIT_OFFSET)

#define NPE_UT_STATUS_READ_EXPECTED_ID \
    (IX_NPE_A_MSSG_ATM_UTOPIA_STATUS_READ << NPE_MSG_COMMAND_BIT_OFFSET)

#define NPE_ATM_STATUS_READ_EXPECTED_ID \
    (IX_NPE_A_MSSG_ATM_STATUS_READ << NPE_MSG_COMMAND_BIT_OFFSET)

#define NPE_MSG_RX_ENABLE_RXQ_SHIFT     20
#define NPE_MSG_RX_ENABLE_RXQ_MASK      0x00f00000
#define NPE_MSG_RX_ENABLE_TYPE_SHIFT    16
#define NPE_MSG_RX_ENABLE_TYPE_MASK     0x000f0000
#define NPE_MSG_RX_ENABLE_RXFREEQ_SHIFT  8
#define NPE_MSG_RX_ENABLE_RXFREEQ_MASK  0x0000ff00
#define NPE_MSG_RX_ENABLE_VCID_SHIFT     0
#define NPE_MSG_RX_ENABLE_VCID_MASK     0x000000ff

#define NPE_MSG_RX_ENABLE_PORT_SHIFT    24
#define NPE_MSG_RX_ENABLE_PORT_MASK     0xff000000
#define NPE_MSG_RX_ENABLE_VPI_SHIFT     16
#define NPE_MSG_RX_ENABLE_VPI_MASK      0x00ff0000
#define NPE_MSG_RX_ENABLE_VCI_SHIFT      0
#define NPE_MSG_RX_ENABLE_VCI_MASK      0x0000ffff

#endif /* IXATMDNPE_P_H */


