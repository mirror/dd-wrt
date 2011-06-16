/*
 ***************************************************************************
 *
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
 *
 **************************************************************************
 */

/**
 *****************************************************************************
 * @file  qat_comms.c
 *
 * @ingroup icp_Qatal
 *
 * @description
 *        This file contains functions for communications with the QAT-AE.
 ****************************************************************************
 */

#include "IxOsal.h"
#include "ix_types.h"
#include "ix_error.h"
#include "qat_comms.h"
#include "qatal_log.h"
#include "qatal_mem.h"
#include "icp_arch_interfaces.h"
#include "icp_qat_fw.h"
#include "icp_rings.h"
#include "cpa.h"
#include "icp_asd_cfg.h"
#include "halAeApi.h"
#include "hal_et_ring.h"
#include "hal_dram.h"
#include "platform.h"

/*
******************************************************************************
*
* Interrupt Control defines
*
*****************************************************************************/

unsigned int g_RingIntrColCtl = 0x80002000;

/* Enable ring coalescing interrupt generation */
#define RING_INT0_COL_CTL_EN        0x80000000
#define RING_INT1_COL_CTL_EN        0x80000000

/* Interrupt Source Masks */
#define RESPRING_SRC_0              0x4444444CUL
#define RESPRING_SRC_123            0x44444444UL

/*  Specifies the region of valid ring addresses */
#define QAT_RING_MASK_BOUNDRY (0xFFFFC000)

#define QAT_RING_INTSRC_LEVELTRGR_SHIFT  3


#define QAT_ICP_RING_CSR_E_STAT (ET_RING_E_STAT_0)

#define QAT_MAX_HARDWARE_RING         (32)

#define QAT_NO_OF_MSG                (256)
#define QAT_MSG_SIZE                 (64)
#define QAT_RING_FREE_MIN            (QAT_MSG_SIZE + 1)



#define QAT_RING_TYPE (1)

#define QAT_RING_BASE_CSR_STRT_BIT (0)
#define QAT_RING_BASE_CSR_END_BIT  (29)

#define QAT_RING_NEAR_WATERMARK_0  (0)

#define QAT_NULL_RING_HANDLE  (void *)0


/* Valid size of all ring size blocks */
#define QAT_VALID_RING_BLOCK_SIZE  (0x4000)

STATIC CpaBoolean qatCoalescedInterruptsEnabled = FALSE;

STATIC int        qatCoalescedInterruptsTimer   = 0;

STATIC int (*asd_bh_schedule)(void) = NULL;


/* A local switch for QAT module collecting statistics */
STATIC qat_comms_statistics_t qatCollectStatistics = QAT_COMMS_STATISTICS_ON;

/*
*******************************************************************************
* Spinlock Macros : locking mechanism to be used
*******************************************************************************
*/
#ifdef ENABLE_SPINLOCK

typedef IxOsalSpinLock SPINLOCK_T;



#define QAT_SPINLOCK_INIT(lock) (void)ixOsalSpinLockInit(&lock, TYPE_IGNORE)
#define QAT_SPINLOCK(lock)      (void)ixOsalSpinLockLockBh(lock)
#define QAT_SPINUNLOCK(lock)    (void)ixOsalSpinLockUnlockBh(lock)

#else

typedef IxOsalSemaphore SPINLOCK_T;

#define QAT_SPINLOCK_INIT(lock)   ixOsalSemaphoreInit(&lock, 1)
#define QAT_SPINLOCK(lock)       { if((*lock) != NULL)     \
                                   ixOsalSemaphoreWait(lock, \
                                                       IX_OSAL_WAIT_FOREVER); }
#define QAT_SPINUNLOCK(lock)      { if((*lock) != NULL)   \
                                    ixOsalSemaphorePost(lock); }

#endif

/*
*******************************************************************************
* Request/Response Ring control Structure
*******************************************************************************
*/

#define QAT_SENT                (0)
#define QAT_RECEIVED            (1)
#define QAT_RETRY               (2)

#define QAT_MSG_SENT_RECEIVE_RETRY    (3)

#define QAT_RING_ALIGNMENT 6


typedef Cpa32U qat_hw_ring_handle;

/**
 *****************************************************************************
 *
 * @description
 *  This structure describes the information associated with a
 *  ME/Host communication pipe this structure is used as to create
 *  ISR callback or dispatch threads communication pipes.
 *
 * @purpose
 *
 *****************************************************************************/
typedef struct qat_s_comm_pipe_info
{
    qat_hw_ring_handle      m_PipeHandle; /**< OUT - ring handle*/
    Cpa32U                  m_PipeId;
 } qat_comm_pipe_info;


/* Struct to contain the ring information for each QAT message type */
typedef struct qat_ring_info_s
{

    /* Stores handles for request rings allocated for this msg type
       Different request rings may be used for different msg priorities */
    qat_hw_ring_handle hRequestRing[QAT_COMMS_PRIORITY_END_DELIMITER];


    /* Stores handle for response ring allocated for this msg type  */
    Cpa8U RequestRingID[QAT_COMMS_PRIORITY_END_DELIMITER];


    /* Response Ring id for request */
    qat_hw_ring_handle hResponseRing;

    /* Stores callback function pointer for all response messages
       of this type  */
    qat_comms_cb_func_t pResponseCb;

    /* Ring ID for Response  */
    Cpa8U ResponseRingID;


} qat_ring_info_t;

/*
*******************************************************************************
*  Struct to contain shadow info for each ring
*******************************************************************************
*/
typedef struct qat_ring_dev_info_s
{
    Cpa32U baseAddr;         /* Base address (virtual) of the ring */
    Cpa32U size;        /* Size of the ring in bytes */
    Cpa32U space;        /* Cache of space available on ring */
    Cpa32U head;        /* Shadow copy of the ring head pointer */
    Cpa32U tail;        /* Shadow copy of the ring tail pointer */
    qat_hw_ring_handle hResponseRing;
    SPINLOCK_T ringLock;    /* Spin lock to protect the ring */
    void * pVirAddr;    /* Virtual address of Ring */
} qat_ring_dev_info_t;


/*
*******************************************************************************
*  Struct to describe ring address
*******************************************************************************
*/

typedef Cpa32U  * Qat_extended_memory_address;

typedef union qat_u_hw_ring_address
{
    Cpa64U value;
   Qat_extended_memory_address pointer;
} qat_ring_addr;


/*
*******************************************************************************
*  Communication Information
*******************************************************************************
*/

#define QAT_MSG_SZ_WORDS (ICP_QAT_FW_RESP_DEFAULT_SZ >> 2)

/* Ring IDs are divided into groups of 32 for the purpose of using
 * bitmask variables to select multiple rings at once, as required
 * by certain Resource Manager API functions.  This macro can be
 * used to set the correct bitmask according to ringId.
 */

/*
 * do {  }while is use to specify start and end of macro
 */
#define RM_RING_MASKS_SET(ringId, lo_mask, hi_mask) \
do {                                                \
    (lo_mask) = (hi_mask) = 0;                      \
    if((ringId) < QAT_MAX_HARDWARE_RING)                               \
    {                                               \
        (lo_mask) |= 1 << (ringId);                 \
    }                                               \
    else                                            \
    {                                               \
        (hi_mask) |= 1 << ((ringId) - QAT_MAX_HARDWARE_RING);          \
    }                                               \
} while(0)


/**
 * MACRO NAME: QAT_BIT_FIELD_MASK32
 *
 * DESCRIPTION: Builds the mask required to extract the bit field from a
 *              32 bit unsigned integer value.
 *
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing
 *            the position of the least significant
 *            bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing
 *            the position of the most significant
 *            bit of the bit field.
 *
 * @Return: Returns a 32 bit mask that will extract the bit field from
 *          a 32 bit unsigned integer value.
 */
#define QAT_BIT_FIELD_MASK32( \
                             arg_FieldLSBBit, \
                             arg_FieldMSBBit \
                           ) \
                           ((ix_bit_mask32)((((Cpa32U)1 <<((arg_FieldMSBBit)\
                             + 1 - (arg_FieldLSBBit))) - \
                               (Cpa32U)1) << (arg_FieldLSBBit)))


/**
 * MACRO NAME: QAT_MAKE_BIT_FIELD32
 *
 * DESCRIPTION: This macro will create a temporary 32 bit value
 *              with the bit field
 *              desired set to the desired value.
 *
 * @Param:  - IN arg_BitFieldValue is the new value of the bit field.
 *            The value can be from 0 to
 *            (1 << (arg_FieldMSBBit + 1 - arg_FieldLSBBit)) - 1.
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing
 *            the position of the least significant
 *            bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing
 *            the position of the most significant
 *            bit of the bit field.
 *
 * @Return: Returns a temporary Cpa32U value that has the bit field set
 *          to the appropriate value.
 */
#define QAT_MAKE_BIT_FIELD32( \
                             arg_BitFieldValue, \
                             arg_FieldLSBBit, \
                             arg_FieldMSBBit \
                           ) \
                   (((Cpa32U)(arg_BitFieldValue) << (arg_FieldLSBBit)) & \
                      QAT_BIT_FIELD_MASK32(arg_FieldLSBBit, arg_FieldMSBBit))


/**
 * MACRO NAME: QAT_GET_BIT_FIELD32
 *
 * DESCRIPTION: Extracts a bit field from 32 bit unsigned integer.
 *              The returned value is normalized in
 *              in the sense that will be right aligned.
 *
 * @Param:  - IN arg_PackedData32 a 32 bit unsigned integer that
 *            contains the bit field of interest.
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing
 *            the position of the least significant
 *            bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing
 *            the position of the most significant
 *            bit of the bit field.
 *
 * @Return: Returns the value of the bit field. The value can be from 0
 *          to (1 << (arg_FieldMSBBit + 1 -
 *          arg_FieldLSBBit)) - 1.
 */
#define QAT_GET_BIT_FIELD32( \
                            arg_PackedData32, \
                            arg_FieldLSBBit, \
                            arg_FieldMSBBit \
                          ) \
                    (((Cpa32U)(arg_PackedData32) & QAT_BIT_FIELD_MASK32   \
                     (arg_FieldLSBBit, arg_FieldMSBBit)) >> \
                     (arg_FieldLSBBit))


/**
 *****************************************************************************
 * @ingroup QAT
 *         Create ring handle.
 *
 * @description
 *         This macro will create a Ring handle based on the passed arguments.
 * @param   arg_RingType IN This is a ix_ring_type value representing the
 *             ring type.
 * @param   arg_MemChannel IN This is a Cpa32U value representing the
 *             Memory Channel.
 * @param   arg_MemType IN This is a Cpa32U value representing the Memory
 *             Type
 * @param   arg_RingId IN This is a Cpa32U value representing the Ring ID
 *
 * @retval  A ix_ring_handle coresponding to the passed values.
 *
 *****************************************************************************/
#define QAT_CREATE_RING_HANDLE( \
                                 arg_RingType, \
                                 arg_MemType, \
                                 arg_MemChannel, \
                                 arg_RingId \
                               ) \
                             ((qat_hw_ring_handle) /* typecast */ \
                             (QAT_MAKE_BIT_FIELD32(arg_RingType, 30U, 31U))| \
                             (QAT_MAKE_BIT_FIELD32(arg_MemType, 10U, 11U)) | \
                             (QAT_MAKE_BIT_FIELD32(arg_MemChannel, 8U, 9U))| \
                             (QAT_MAKE_BIT_FIELD32(arg_RingId, 0U, 7U)))


/**
 *****************************************************************************
 * @ingroup rm_hardware
 *    Retrieve the ring index from a ring handle.
 *
 * @description
 *    These macros should be used for portability in the case the encoding will
 *    change.This macro will retrieve the ring index from a ring handle.
 *
 * @param    arg_hHwRing IN The ring handle whose ring index we need.
 *             Should be of type qat_hw_ring_handle.
 *
 * @retval   Cpa32U value representing the ring index.
 ***********************************************************************/

#define QAT_RING_GET_INDEX( \
                                 arg_hHwRing \
                               ) \
                               QAT_GET_BIT_FIELD32(arg_hHwRing, 0U, 7U)






/**
 * MACRO NAME: QAT_RING_CREATE
 *
 * DESCRIPTION: This macro will create a ICP ring.
 *
 * @Param:  - IN arg_MemType - this is a enumarated data type
 *            representing the ICP ring
 *            memory type.
 * @Param:  - IN arg_MemChannel - this is a enumarated data
 *            type representing the ICP
 *            ring memory channel.
 * @Param:  - IN arg_RingID - this is an unsigned integer value
 *            representing the ICP
 *            ring ID that can have values from 0 to
 *            ICP_MAX_RING_ENTRIES. There will be
 *            no check to see if the ring ID is in
 *            range in order to speed up the put
 *            operation. The caller has to make sure that
 *            the ring ID is in range.
 * @Param:  - IN arg_RingSize - this is a enumarated data
 *            type representing the ICP
 *            ring size.
 * @param   - IN arg_NearFullMark - this is a enumarated data
 *            type representing the
 *            ICP rings near full watermark value.
 * @param   - IN arg_NearEmptyMark - this is a enumarated data
 *            type representing the
 *            ICP rings near empty watermark value.
 * @param   - IN arg_RingBaseAddr - this is a unsigned long
 *            long interger
 *            representing the ICP rings near empty watermark value.
 *
 * @Return: The macro returns an unsigned 32 bit integer representing
 *          the status of the
 *          operation.
 */
#define QAT_RING_CREATE(arg_MemType, arg_MemChannel, arg_RingID, arg_RingSize,\
                arg_NearFullMark, arg_NearEmptyMark, arg_RingBaseAddr) \
             halAe_ETRingInit(arg_RingID, arg_NearFullMark, arg_NearEmptyMark,\
                     arg_RingSize, arg_RingBaseAddr)


/*********************************************************
 ******************* Internal Data ***********************
 *********************************************************/

/*
*******************************************************************************
*  Data stored to control the ring and communication
*******************************************************************************
*/


/* Ring information for each QAT message type */
STATIC qat_ring_info_t qatComms_ringInfo[ICP_ARCH_IF_REQ_DELIMITER];

/* Flag to track the initialisation status of this component */
STATIC CpaBoolean qatComms_Initialised = CPA_FALSE;

/* Ring shadow information */
STATIC qat_ring_dev_info_t qat_ringDevInfo[QAT_MAX_HARDWARE_RING];

/* Ring Mask / Response information */
STATIC Cpa32U respRingMask0_31 = 0;
STATIC Cpa32U numRespRings = 0;
STATIC Cpa32U respRingIds[QAT_MAX_HARDWARE_RING];

/* Array of no. of messages sent,received & retry */
STATIC IxOsalAtomic
       qat_comms_ACK[ICP_ARCH_IF_REQ_DELIMITER][QAT_MSG_SENT_RECEIVE_RETRY];


/* do {  }while is use to specify start and end of macro
   Macro to check if comm is initialised */
#define QAT_COMMS_INITIALISED_CHECK()                                 \
do {                                                                  \
    if ( CPA_TRUE != qatComms_Initialised )                           \
    {                                                                 \
       QATAL_ERROR_LOG                                                \
          ("QAT-Comms API called before QAT-Comms was initialised\n");\
                                                                      \
        return CPA_STATUS_FAIL;                                       \
    }                                                                 \
} while(0)

/*********************************************************
 ******************* External Data ***********************
 *********************************************************/

extern icp_asd_cfg_param_get_cb_func_t g_getCfgParamFunc ;


/*********************************************************
 ******************* Internal functions ******************
 *********************************************************/

void QatComms_RegConfigShow(void);

CpaStatus QAT_comm_pipe_create(qat_comm_pipe_info* , void ** pVirAddr);

CpaStatus QAT_comm_pipe_delete(Cpa8U);

void QatComms_RingDevInfoPrint(Cpa32U ringId);
void QatComms_ACK_DataInit( void );
void QatComms_ACK_ReqInc(Cpa32U ReqType);
void QatComms_ACK_RespInc(Cpa32U RespType );
void QatComms_ACK_RetryInc(Cpa32U RespType );
void QatComms_RingDevInfoCreate(Cpa32U ringId, void * pVirAddr);
CpaStatus QatComms_RingPut(icp_ring_id_t ringId, Cpa32U *pData,
                              Cpa32U lwSizea);



/*********************************************************
 ******************* Internal functions ******************
 *********************************************************/

/*
 * @fn QatComms_ACK_DataInit
 *
 * Initialize no. of messages sent and Received
 */
void
QatComms_ACK_DataInit( void )
{

    int Loop_reqType = 0;

    /* For every ReqType */
    for (Loop_reqType = 0;
         Loop_reqType < ICP_ARCH_IF_REQ_DELIMITER; Loop_reqType++)
    {

        /* Initialize Atomic data structure  */
        ixOsalAtomicSet(0 , &qat_comms_ACK[Loop_reqType][QAT_SENT]);
        ixOsalAtomicSet(0 , &qat_comms_ACK[Loop_reqType][QAT_RECEIVED]);
        ixOsalAtomicSet(0 , &qat_comms_ACK[Loop_reqType][QAT_RETRY]);
    }
}
/*
 * @fn QatComms_ACK_ReqInc
 *
 * Increment no. of messages sent
 */
void
QatComms_ACK_ReqInc(Cpa32U ReqType )
{
    if (ReqType <= ICP_ARCH_IF_REQ_DELIMITER)
    {
        /* Increment Request Type   */
        ixOsalAtomicInc(&qat_comms_ACK[ReqType][QAT_SENT]);
    }
}

/*
 * @fn QatComms_ACK_RespInc
 *
 * Increment no. of messages received
 */
void
QatComms_ACK_RespInc(Cpa32U RespType )
{
    if (RespType <= ICP_ARCH_IF_REQ_DELIMITER)
    {
        /* Increment Response Type   */
        ixOsalAtomicInc(&qat_comms_ACK[RespType][QAT_RECEIVED]);
    }
}
/*
 * @fn QatComms_ACK_RetryInc
 *
 * Increment no. of messages retried
 */
void
QatComms_ACK_RetryInc(Cpa32U RespType )
{
    if (RespType <= ICP_ARCH_IF_REQ_DELIMITER)
    {
        /* Increment Retry Type   */
        ixOsalAtomicInc(&qat_comms_ACK[RespType][QAT_RETRY]);
    }
}


/*
 * @fn QatComms_RequestRingExist
 *
 * Simple function to return CPA_TRUE if ring specified by input parameter
 * exists as a Request Ring
 *
 */
STATIC CpaBoolean
QatComms_RequestRingExists(qat_comm_pipe_info *pRingInfo)
{
    Cpa8U ReqType_Loop_Counter = 0;    /* loop counters */
    Cpa8U Priority_Loop_Counter = 0;

    if (pRingInfo == NULL)        /* If null data ... return False */
    {

        QATAL_ERROR_LOG("QatComms: RequestRingExists() ..Null Data\n");

        return CPA_FALSE;
    }

    /* For every possible entry in ring table */
    for (ReqType_Loop_Counter = 0;
         ReqType_Loop_Counter < ICP_ARCH_IF_REQ_DELIMITER;
         ReqType_Loop_Counter++)
    {
        /* for every possible priority */
        for (Priority_Loop_Counter = 0;
             Priority_Loop_Counter < QAT_COMMS_PRIORITY_END_DELIMITER;
             Priority_Loop_Counter++)
        {
            /* if Ring in input paramter = Curr Request ring */
            if ( pRingInfo->m_PipeId ==
                qatComms_ringInfo[ReqType_Loop_Counter].
                    RequestRingID[Priority_Loop_Counter])
            {
                /* Return TRUE */
                pRingInfo->m_PipeHandle =
                    qatComms_ringInfo[ReqType_Loop_Counter].
                        hRequestRing[Priority_Loop_Counter];
                return CPA_TRUE;
            }
        }
    }

    return CPA_FALSE;
}

/*
 * @fn QatComms_RequestRingCount
 *
 * Simple function to return no of times ring specified by input parameter
 * exists as Request Ring
 *
 */
STATIC Cpa8U
QatComm_RequestRingCount(Cpa8U ref_requestRingId)
{

    Cpa8U count = 0;    /* Counter */
    Cpa8U ReqType_Loop_Counter = 0;  /* Loop Counters */
    Cpa8U Priority_Loop_Counter = 0;

    /* for every Request Ring */
    for (ReqType_Loop_Counter = 0;
         ReqType_Loop_Counter < ICP_ARCH_IF_REQ_DELIMITER;
         ReqType_Loop_Counter++)
    {
        for (Priority_Loop_Counter = 0;
             Priority_Loop_Counter < QAT_COMMS_PRIORITY_END_DELIMITER;
             Priority_Loop_Counter++)
        {
            /* Ringin in input paramter = ringid in request Ring therefor
             * increment counter */
            if ( ref_requestRingId ==
                 qatComms_ringInfo[ReqType_Loop_Counter].
                    RequestRingID[Priority_Loop_Counter])
            {
                count++;
            }
        }
    }

    return count ;
}

/*
 * @fn QatComms_ResponseRingExist
 *
 * Simple function to return CPA_TRUE if ring specified by input parameter
 * exists as a
 * Response Ring
 */
STATIC CpaBoolean
QatComms_ResponseRingExists(qat_comm_pipe_info *pRingInfo)
{
    Cpa8U    ReqType_Loop_Counter = 0;    /* Loop count */

    if (pRingInfo == NULL)        /* If null data ... return False */
    {

        QATAL_ERROR_LOG("QatComms: ResponseRingExists() ..Null Data\n");

        return CPA_FALSE;

    }

    /* for every possibe response ring */
    for (ReqType_Loop_Counter = 0;
         ReqType_Loop_Counter < ICP_ARCH_IF_REQ_DELIMITER;
         ReqType_Loop_Counter++)
    {
        /* If Ringid in inputparamter = curr Response ring Return True */
        if ( pRingInfo->m_PipeId ==
            qatComms_ringInfo[ReqType_Loop_Counter].ResponseRingID)
        {
            pRingInfo->m_PipeHandle =
                qatComms_ringInfo[ReqType_Loop_Counter].hResponseRing;
            return CPA_TRUE;
        }
    }

    return CPA_FALSE;
}
/*
 * @fn QatComms_ResponseRingCount
 *
 * Simple function to return no of times ring specified by input parameter
 * exists as Response Ring
 */
STATIC Cpa8U
QatComm_ResponseRingCount(Cpa8U ref_responseRingId)
{
    Cpa8U count = 0;    /* Counter */
    Cpa8U ReqType_Loop_Counter = 0;  /* ReqType_Loop_Counter Counter */

    /* For every possible Response Ring */
    for (ReqType_Loop_Counter = 0;
         ReqType_Loop_Counter < ICP_ARCH_IF_REQ_DELIMITER;
         ReqType_Loop_Counter++)
    {
        /* If Ringid in inputparamter = curr Response ring increment counter*/
        if ( ref_responseRingId ==
            qatComms_ringInfo[ReqType_Loop_Counter].ResponseRingID)
        {
            count++;
        }
    }

    return count ;
}

/*
 * @fn QatComms_ReqHdrWrite
 *
 * Fills in common request message header for each message sent to QAT
 */
STATIC void
QatComms_ReqHdrWrite(
    icp_arch_if_req_hdr_t *pMsg,
    icp_arch_if_request_t requestType,
    Cpa8U responseRingId)
{

#ifdef ICP_PARAM_CHECK

    if (NULL == pMsg)            /* IF pMsg PTR == null Exit with Error */
    {
        QATAL_ERROR_LOG
           ("QatComms: QatComms_ReqHdrWrite() failed \n");
        return;
    }
#endif

    /* Call Macro build Flags */
    pMsg->flags = ICP_ARCH_IF_FLAGS_BUILD(ICP_ARCH_IF_REQ_VALID_SET,
                                          ICP_ARCH_IF_ET_RING_RESP,
                                          ICP_ARCH_IF_S_RESP);

    /* Set request type and repsonse Ring */
    pMsg->req_type = requestType;
    pMsg->resp_pipe_id = responseRingId;
}


/*
 * @fn QatComms_ResponseMsgHandler
 *
 * Processes response message notification from RM, by reading all
 * messages from the ring and invoking callback function registered
 *  by upper s/w layers
 *
 * This Function assumes that Message type is 64bytes and Ring size
 * is a multiple of 64bytes
 *
 */
STATIC void
QatComms_ResponseMsgHandler(
    Cpa32U arg_PipeId,
    void* arg_pContext)
{
    icp_arch_if_request_t qatNewReqType;
    icp_arch_if_req_hdr_t *pNewReqHeader = NULL;

    qat_ring_info_t *pCallBackRingInfo = 0;

    Cpa32U ringId = arg_PipeId;
    void *msg     = NULL;

    /* extract Ringid information of the response ring */
    qat_ring_dev_info_t *pRingDev = &(qat_ringDevInfo[ringId]);

    arg_pContext = arg_pContext;

#ifdef ICP_DEBUG
    Cpa8U *pMsg =NULL;
    int i = 0;
    ixOsalStdLog("General Response Msg Handler Called ring %d\n",
                             (int)ringId);
#endif

    /*
     * Algorithm:
     * (1) Read Ring Tail CSR
     * (2) Invoke callback per entry
     * (3) Write Ring Head CSR
     *
     * Note: No need to lock as long as the following assumptions hold true:
     * - Response ring is not shared - i.e. this function is the only consumer.
     * - This handler is executed in tasklet context.
     * - This implementation assumes that the ring is not empty when this
     *   handler is invoked.
     */

    /*
     * Step 1: Read Tail CSR
     */
    pRingDev->tail = EAGLETAIL_RING_CSR_READ(ringId, ET_RING_TAIL_OFFSET);

#ifdef ICP_DEBUG
    QATAL_DEBUG_LOG_OPTIONAL("Qatcomms - Before Response Handler \n");
    QatComms_RegConfigShow();
    QatComms_RingDevInfoPrint( ringId);
#endif

    /* Step 2: Invoke the callback per message */
    do
    {
        /* Address of response message at head */
        msg = (void *)((Cpa32U)pRingDev->pVirAddr + (Cpa32U)pRingDev->head);

        pNewReqHeader = (icp_arch_if_req_hdr_t *)msg;

        /* Get newRequest type , then get CallBack function id*/
        qatNewReqType = pNewReqHeader->req_type;

        pCallBackRingInfo  = &(qatComms_ringInfo[qatNewReqType]);

        /* Invoke upper-layer message callback handler if not equal to NULL*/
        if (NULL != pCallBackRingInfo->pResponseCb)
        {

#ifdef ICP_DEBUG
            {
                ixOsalStdLog("Response msg from ring %d\n", (int)ringId);

                pMsg = (Cpa8U *)msg;

                ixOsalStdLog("Got response msg (BE format)...:\n");
                for (i = 0; i < (QAT_MSG_SZ_WORDS * 4); i++)
                {
                    ixOsalStdLog("%02X", pMsg[i]);
                    if (i%4 == 3) ixOsalStdLog(" ");
                }
                ixOsalStdLog("\n");
            }
#endif

            pCallBackRingInfo->pResponseCb(msg, qatNewReqType);

            /* Increment Response Msg done */

            if(QAT_COMMS_STATISTICS_ON == qatCollectStatistics)
            {
                QatComms_ACK_RespInc(qatNewReqType);
            }
        }
        else    /* call Error routine */
        {

            QATAL_ERROR_LOG("No Callback on Response Ring\n");
        }

        /* Advance the head pointer and handle wraparound */
        pRingDev->head = (pRingDev->head + QAT_MSG_SIZE) % pRingDev->size;
    }
    while(pRingDev->head != pRingDev->tail);

    /* Step 3 - Update the head CSR */
    EAGLETAIL_RING_CSR_WRITE(ringId, ET_RING_HEAD_OFFSET, pRingDev->head);
}


/*
 * @fn QatComms_RingDevInfoCreate
 *
 * For a specified Ringid: this builds the shadow information structure
 *
 */
void QatComms_RingDevInfoCreate(Cpa32U ringId, void * pVirAddr)
{
    qat_ring_dev_info_t *pRingDevInfo = NULL;

    pRingDevInfo = &(qat_ringDevInfo[ringId]);

    pRingDevInfo->size = EAGLETAIL_RING_SIZE_TO_BYTES(IX_ICP_RING_SIZE_16K  );


    pRingDevInfo->head = EAGLETAIL_RING_CSR_READ(ringId, ET_RING_HEAD_OFFSET);
    pRingDevInfo->tail = EAGLETAIL_RING_CSR_READ(ringId, ET_RING_TAIL_OFFSET);
    pRingDevInfo->pVirAddr = pVirAddr;

    /* Create lock mechanism and load into ring structure */
    QAT_SPINLOCK_INIT(pRingDevInfo->ringLock);

#ifdef ICP_DEBUG
    ixOsalStdLog("Ring ID: %d\n", ringId);
    ixOsalStdLog("\tAddr      : 0x%08x\n", pRingDevInfo->pVirAddr);
    ixOsalStdLog("\tHead: 0x%x\n", pRingDevInfo->head);
    ixOsalStdLog("\tTail: 0x%x\n", pRingDevInfo->tail);
#endif
}


/*
 * @fn QatComms_RingDevInfoPrint
 *
 * For a specified Ringid: this builds the shadow information structure
 *
 */
void QatComms_RingDevInfoPrint(Cpa32U ringId)
{
    qat_ring_dev_info_t *pRingDevInfo = NULL;
    qat_ring_addr pMemory;
    CpaStatus status = CPA_STATUS_SUCCESS;

#ifdef ICP_DEBUG
    Cpa32U csrVal = 0;
#endif

    pRingDevInfo = &(qat_ringDevInfo[ringId]);

    status = QATAL_MEM_SHARED_VIRT_TO_PHYS((void*)pRingDevInfo->pVirAddr,
                    (Cpa64U *)&pMemory.pointer);

    pRingDevInfo->head = EAGLETAIL_RING_CSR_READ(ringId, ET_RING_HEAD_OFFSET);
    pRingDevInfo->tail = EAGLETAIL_RING_CSR_READ(ringId, ET_RING_TAIL_OFFSET);

#ifdef ICP_DEBUG
    ixOsalStdLog("Ring ID: %d\n", ringId);
    ixOsalStdLog("\tAddr      : 0x%08x\n", pRingDevInfo->pVirAddr);
    ixOsalStdLog("\tAddr_p    : 0x%x\n",(long long int) pMemory.value);
    ixOsalStdLog("\tHead: 0x%x\n", pRingDevInfo->head);
    ixOsalStdLog("\tTail: 0x%x\n", pRingDevInfo->tail);

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_CONFIG +
        (ringId << 2));
    ixOsalStdLog("\tRing  size: 0x%08x\n",csrVal);


#endif
}

/*
 * @fn QatComms_RequestRingCreate
 *
 *  Create a Request Ring for a specified Request type, with specified
 *  priority and ringid and then Allocates the specified request ring
 *
 */

STATIC CpaStatus
QatComms_RequestRingCreate(
    icp_arch_if_request_t qatReqType,
    qat_comms_priority_t priority,
    icp_ring_id_t ringId )
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    qat_comm_pipe_info pipeInfo = {0};
    void * pVirAddr = NULL ;

    /* Initial specifications if Pipe */
    pipeInfo.m_PipeId = ringId;
    pipeInfo.m_PipeHandle = (int)QAT_NULL_RING_HANDLE;

    /* Check if this ring exists as a Request Ring  */
    if (QatComms_RequestRingExists(&pipeInfo) == CPA_FALSE)
    {
        /* If it does not exist as a reqiest ring then call RM to
           create pipe*/
        status = QAT_comm_pipe_create(&pipeInfo, &pVirAddr);


        /* if this failed .. mask out 1st byte and pass remainder of
           return code to ix_rm_error_get_string to display reason */

        if (CPA_STATUS_SUCCESS != status)
        {

            QATAL_ERROR_LOG
                 ("Failed to create request ring Id %d, status = 0x%x\n");
            return CPA_STATUS_FAIL;
        }

        /* Update Ring information structure */
        QatComms_RingDevInfoCreate((Cpa32U) ringId, pVirAddr);

    }
    /* Else .. ring did exist */
    else
    {
#ifdef ICP_DEBUG
    QATAL_DEBUG_LOG_OPTIONAL
          ("QatComms_ReqestRingCreate - Ringid already created \n");
#endif
    }

    /* Assign Pipe and Ring information to the Request/Response Data struct */
    qatComms_ringInfo[qatReqType].hRequestRing[priority] =
               pipeInfo.m_PipeHandle;

    qatComms_ringInfo[qatReqType].RequestRingID[priority] =
               ringId;

    return CPA_STATUS_SUCCESS;
}

/*
 * @fn QatComms_RequestRingDestroy
 *
 * Frees a request ring of a specified reqType and priority
 *
 */
STATIC CpaStatus
QatComms_RequestRingDestroy(
    icp_arch_if_request_t qatReqType,
    qat_comms_priority_t priority)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* get ring handle for a request ring of desired reqType/Priority*/
    qat_hw_ring_handle ringHandle =
        qatComms_ringInfo[qatReqType].hRequestRing[priority];
    Cpa8U requestRingId = 0;
    Cpa8U count = 0;

    /* Get Ringid from ring handle */
    requestRingId  = QAT_RING_GET_INDEX(ringHandle);

    /* How many times is specified request ring used */
    count = QatComm_RequestRingCount(requestRingId);

    /* if count == 0 then does not exist in ring */
    if (0 == count )
    {
        /* If count == 0 ..therefor must be previously deleted */
        return CPA_STATUS_SUCCESS;
    }
    else if (1 == count )
    {
        /* If only used once then it can be deleted */
        /* Delete ring */
        status = QAT_comm_pipe_delete(requestRingId);

        /* If failure maskout 1st byte and call ix_rm_error_get_string
           to display error */
        if (CPA_STATUS_SUCCESS != status)
        {
            QATAL_ERROR_LOG
               ("Failed to delete request ring Id \n");
            return CPA_STATUS_FAIL;
        }
    }

    /* If ring existed only once this will remove all instances of the ring if
       ring existed more than once then this will remove all information about
       the ring for this reqType and priority */
    qatComms_ringInfo[qatReqType].hRequestRing[priority] = \
                                                    (int)QAT_NULL_RING_HANDLE;

    qatComms_ringInfo[qatReqType].RequestRingID[priority] = 0xFF ;
    return CPA_STATUS_SUCCESS;
}

/*
 * @fn QatComms_ResponseRingCreate
 *
 *  Create a Response Ring for a specified Response type and ringid and then
 *  Allocates the specified response ring
 *
 */
STATIC CpaStatus
QatComms_ResponseRingCreate(
    icp_arch_if_request_t qatReqType,
    icp_ring_id_t ringId)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    qat_comm_pipe_info pipeInfo = {0};
    qat_ring_info_t *pRingInfo = &(qatComms_ringInfo[qatReqType]);
    void * pVirAddr = NULL ;

    /* initial Pipe creation paramters */
    pipeInfo.m_PipeId = ringId;

    /* Does Repsonse ring exist with this ring id */
    if (QatComms_ResponseRingExists(&pipeInfo) == CPA_FALSE)
    {
        /* If not Create 1 new response ring with a "Response AEs Handler"
           which will be involded when message arrives on this ring */

        status =
              QAT_comm_pipe_create(&pipeInfo, &pVirAddr);

        /* If ring create failed mask out 1st byte and ix_rm_..
           to display string */
        if (CPA_STATUS_SUCCESS != status)
        {
            QATAL_ERROR_LOG
               ("Failed to create response ring Id \n");
            return CPA_STATUS_FAIL;
        }

        /* Update Ring Info Structure */
        QatComms_RingDevInfoCreate((Cpa32U) ringId, pVirAddr);

        /* Enable response Ring id in Global Response Ring Mask */

        respRingMask0_31 |= (1 << ringId);
        respRingIds[numRespRings++] = ringId;

    }
    else
    {
#ifdef ICP_DEBUG
        QATAL_DEBUG_LOG_OPTIONAL
           ("QatComms_ResponseRingCreate - Ringid already created \n");

#endif
    }
    /* Add ring/pipe info to request/response structre */
    pRingInfo->hResponseRing = pipeInfo.m_PipeHandle;

    pRingInfo->ResponseRingID = ringId ;

    qat_ringDevInfo[ringId].hResponseRing = pipeInfo.m_PipeHandle;

    return CPA_STATUS_SUCCESS;
}

/*
 * @fn QatComms_ResponseRingDestroy
 *
 * Frees a response ring of a specified reqType
 *
 */
STATIC CpaStatus
QatComms_ResponseRingDestroy(
    icp_arch_if_request_t qatReqType)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa8U responseRingId = 0;
    Cpa8U count = 0;

    /* Get Ring handle of specified Response Ring */
    qat_hw_ring_handle ringHandle =
            qatComms_ringInfo[qatReqType].hResponseRing;

    /* Get Ring handle */
    responseRingId  = QAT_RING_GET_INDEX(ringHandle);

    /* If only 1 response ring in structure with this ring id then delete it */
    count = QatComm_ResponseRingCount(responseRingId);
    if (0 == count )
    {
        /* If already does not exist must be previously deleted .therefor ok */
        return status;
    }
    else if (1 == count )
    {
        /* Free 1 response ring */
        status = QAT_comm_pipe_delete(responseRingId);
        if (CPA_STATUS_SUCCESS != status)
        {
                QATAL_ERROR_LOG
                   ("Failed to delete response ring Id \n");
                status = CPA_STATUS_FAIL;
        }
    }

    /*  Remove Response Ring id infor from structure for this reqtype */
    qatComms_ringInfo[qatReqType].hResponseRing = (int)QAT_NULL_RING_HANDLE;
    
    qatComms_ringInfo[qatReqType].ResponseRingID = 0xFF;

    return status;
}

/*
 * @fn QatComms_RingsCreate
 *
 * Allocates all request & response rings used by the supported services
 *
 */
STATIC CpaStatus
QatComms_RingsCreate(void)
{
    icp_arch_if_request_t qatReqType = ICP_ARCH_IF_REQ_DELIMITER;
    CpaStatus status = CPA_STATUS_SUCCESS;

    qatReqType = ICP_ARCH_IF_REQ_QAT_FW_INIT;
    if (CPA_STATUS_SUCCESS != QatComms_RequestRingCreate(
            qatReqType,
            QAT_COMMS_PRIORITY_NORMAL,
            ICP_RING_QATAL_ADMIN_REQUEST ))
    {
        status = CPA_STATUS_FAIL;
        return status;
    }
    if (CPA_STATUS_SUCCESS != QatComms_ResponseRingCreate(
            qatReqType, ICP_RING_QATAL_ADMIN_RESPONSE))
    {
        status = CPA_STATUS_FAIL;
        return status;
    }


    qatReqType = ICP_ARCH_IF_REQ_QAT_FW_ADMIN;
    if (CPA_STATUS_SUCCESS != QatComms_RequestRingCreate(
            qatReqType,
            QAT_COMMS_PRIORITY_NORMAL,
            ICP_RING_QATAL_ADMIN_REQUEST ))
    {
        status = CPA_STATUS_FAIL;
        return status;
    }
    if (CPA_STATUS_SUCCESS != QatComms_ResponseRingCreate(
            qatReqType, ICP_RING_QATAL_ADMIN_RESPONSE))
    {
        status = CPA_STATUS_FAIL;
        return status;
    }


    /* Create PKE request/response rings
     * Note that there is no high-priority ring for PKE
     */

    qatReqType = ICP_ARCH_IF_REQ_QAT_FW_PKE;
    if (CPA_STATUS_SUCCESS !=
        QatComms_RequestRingCreate(
        qatReqType,
        QAT_COMMS_PRIORITY_NORMAL,
        ICP_RING_LAC_PKE_REQUEST))
    {
        status = CPA_STATUS_FAIL;
        return status;
    }
    if (CPA_STATUS_SUCCESS !=
        QatComms_ResponseRingCreate(
        qatReqType,
        ICP_RING_LAC_PKE_RESPONSE))
       {
        status = CPA_STATUS_FAIL;
        return status;
    }


    /* Create LA request/response rings */
    qatReqType = ICP_ARCH_IF_REQ_QAT_FW_LA;
    if (CPA_STATUS_SUCCESS !=
        QatComms_RequestRingCreate(
        qatReqType,
        QAT_COMMS_PRIORITY_HIGH,
        ICP_RING_LAC_LA_HI_REQUEST))
    {
        status = CPA_STATUS_FAIL;
        return status;
    }
    if (CPA_STATUS_SUCCESS !=
        QatComms_RequestRingCreate(
        qatReqType,
        QAT_COMMS_PRIORITY_NORMAL,
        ICP_RING_LAC_LA_LO_REQUEST))
    {
        status = CPA_STATUS_FAIL;
        return status;
    }
    if (CPA_STATUS_SUCCESS !=
        QatComms_ResponseRingCreate(
        qatReqType,
        ICP_RING_LAC_LA_RESPONSE))
    {
        status = CPA_STATUS_FAIL;
        return status;
    }

        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_SRCSEL_0,
               RESPRING_SRC_0);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_SRCSEL_1,
               RESPRING_SRC_123);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_SRCSEL_2,
               RESPRING_SRC_123);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_SRCSEL_3,
               RESPRING_SRC_123);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_REG_0,
               respRingMask0_31);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_EN_0,
               respRingMask0_31);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_REG_1, 0 );
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_EN_1, 0);

    if (qatCoalescedInterruptsEnabled)
    {

        /* Enable ring coalescing */
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_EN_0, 0);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_COL_EN_0,
            respRingMask0_31);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_COL_CTL_0,
            g_RingIntrColCtl);
    }
    else
    {
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_REG_0, respRingMask0_31);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_EN_0, respRingMask0_31);

        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_COL_EN_0, 0);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_COL_EN_1, 0);

        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_COL_CTL_0, 0);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_COL_CTL_1, 0);

    }

#ifdef ICP_DEBUG
    QatComms_RegConfigShow();

#endif

    return status;
}


/*
 * @fn QatComms_RingsDestroy
 *
 * Frees all request & response rings used by the supported services
 *
 */
STATIC CpaStatus
QatComms_RingsDestroy(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    if (qatCoalescedInterruptsEnabled)
    {
       /* Disable ring coalescing */
       EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_COL_CTL_0, 0);
       EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_COL_EN_0, 0);
       EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_EN_0,
         EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_EN_0) & ~respRingMask0_31);
    }

    if (CPA_STATUS_SUCCESS !=
        QatComms_RequestRingDestroy(ICP_ARCH_IF_REQ_QAT_FW_INIT,
                                    QAT_COMMS_PRIORITY_NORMAL))
    {
        status = CPA_STATUS_FAIL;
    }
    if (CPA_STATUS_SUCCESS !=
        QatComms_ResponseRingDestroy(ICP_ARCH_IF_REQ_QAT_FW_INIT))
    {
        status = CPA_STATUS_FAIL;
    }

    if (CPA_STATUS_SUCCESS !=
        QatComms_RequestRingDestroy(ICP_ARCH_IF_REQ_QAT_FW_ADMIN,
                                    QAT_COMMS_PRIORITY_NORMAL))
    {
        status = CPA_STATUS_FAIL;
    }
    if (CPA_STATUS_SUCCESS !=
        QatComms_ResponseRingDestroy(ICP_ARCH_IF_REQ_QAT_FW_ADMIN))
    {
        status = CPA_STATUS_FAIL;
    }


    /* Destroy PKE request/response rings */
    if (CPA_STATUS_SUCCESS !=
        QatComms_RequestRingDestroy(ICP_ARCH_IF_REQ_QAT_FW_PKE,
                                    QAT_COMMS_PRIORITY_NORMAL))
    {
        status = CPA_STATUS_FAIL;
    }
    if (CPA_STATUS_SUCCESS !=
        QatComms_ResponseRingDestroy(ICP_ARCH_IF_REQ_QAT_FW_PKE))
    {
        status = CPA_STATUS_FAIL;
    }

    /* Destroy LA request/response rings */
    if (CPA_STATUS_SUCCESS !=
        QatComms_RequestRingDestroy(ICP_ARCH_IF_REQ_QAT_FW_LA,
                                    QAT_COMMS_PRIORITY_HIGH))
    {
        status = CPA_STATUS_FAIL;
    }
    if (CPA_STATUS_SUCCESS !=
        QatComms_RequestRingDestroy(ICP_ARCH_IF_REQ_QAT_FW_LA,
                                    QAT_COMMS_PRIORITY_NORMAL))
    {
        status = CPA_STATUS_FAIL;
    }
    if (CPA_STATUS_SUCCESS !=
        QatComms_ResponseRingDestroy(ICP_ARCH_IF_REQ_QAT_FW_LA))
    {
        status = CPA_STATUS_FAIL;
    }

    if (CPA_STATUS_SUCCESS !=
        QatComms_RequestRingDestroy(ICP_ARCH_IF_REQ_QAT_FW_IPSEC,
                                    QAT_COMMS_PRIORITY_NORMAL))
    {
        status = CPA_STATUS_FAIL;
    }

    return status;
}

/*
 * @fn QatComms_RingInfoClear
 *
 * Clears all global ring info
 * - Fills the global qatComms_ringInfo and qat_ringDevInfo array
 *   elements with NULL values
 * - Clears response ring table info
 *
 */

STATIC void
QatComms_RingInfoClear(void)
{
    /* wipe with Zero from 1st element in array to size_of_array *
       size_of_struct */
    memset(qat_ringDevInfo, 0,  sizeof(qat_ringDevInfo) );

    memset(qatComms_ringInfo, 0, sizeof(qatComms_ringInfo));

    /* clear the response ring table and mask */
    respRingMask0_31 = 0;

    numRespRings = 0;

    memset(respRingIds, 0, sizeof(respRingIds));

}


/*********************************************************
 ********************* Internal API **********************
 *********************************************************/

/*
 * @fn QatComms_Init
 *
 *  Initize the QAT Communication interface component
 *
 */
CpaStatus
QatComms_Init(void)
{
    icp_asd_cfg_param_get_cb_func_t pAsdGetParamFunc=NULL;
    CpaStatus status1 = CPA_STATUS_SUCCESS;
    CpaStatus status2 = CPA_STATUS_SUCCESS;



    QATAL_DEBUG_LOG_OPTIONAL("Initializing QAT-COMMS ...\n");

    /* If comms already initialized .. output an error and fail */
    if (qatComms_Initialised)
    {
        QATAL_ERROR_LOG
           ("QatComms_Init - QAT Comms already initialised\n");
        return CPA_STATUS_FAIL;
    }

    /* get Coalesced intr  */
    pAsdGetParamFunc = (icp_asd_cfg_param_get_cb_func_t)(g_getCfgParamFunc);
    status1 = (*pAsdGetParamFunc)
        (ICP_ASD_CFG_PARAM_ET_RING_LOOKASIDE_INTERRUPT_COALESCING_ENABLE,
         (icp_asd_cfg_value_t *)&qatCoalescedInterruptsEnabled);

    if (CPA_STATUS_SUCCESS !=  status1)
    {
        QATAL_ERROR_LOG
           ("QatComms_Init - Error Load ASD - Coalescing Enable\n");
        return CPA_STATUS_FAIL;
    }

    /* get Coalesced Timer  */
    status2 =(*pAsdGetParamFunc)
        (ICP_ASD_CFG_PARAM_ET_RING_LOOKASIDE_COALESCE_TIMER_NS,
        ( icp_asd_cfg_value_t *)&qatCoalescedInterruptsTimer);


    if (CPA_STATUS_SUCCESS !=  status2)
    {
        QATAL_ERROR_LOG
           ("QatComms_Init - Error Load ASD - Coalescing Timer\n");
        return CPA_STATUS_FAIL;
    }

    /* If Coalased intr  */
    if (qatCoalescedInterruptsEnabled)
    {
        g_RingIntrColCtl =
                (qatCoalescedInterruptsTimer | RING_INT0_COL_CTL_EN);
        QATAL_DEBUG_LOG_OPTIONAL   \
        ("Initializing QAT-AL ...with Coalesced Interrupts\n");

    }
    else
    {
        QATAL_DEBUG_LOG_OPTIONAL   \
        ("Initializing QAT-AL ... without Coalesced interrupts\n");
    }

    /* Clear the Ring info structure */
    QatComms_RingInfoClear();

    /* Create all the rings and if error distroy rings and exit with error */
    if (CPA_STATUS_SUCCESS != QatComms_RingsCreate())
    {
        /* Free any rings that were successfully created */
        QatComms_RingsDestroy();
        return CPA_STATUS_FAIL;
    }

    /* Initialize message counter */
    QatComms_ACK_DataInit( ) ;

    /* Set comm initialized to True */
    qatComms_Initialised = CPA_TRUE;

    /* If we get here .. then every thing is OK  */
    return CPA_STATUS_SUCCESS;
}

/*
 * @fn atComms_Shutdown
 *
 *  Shutdown the QAT interface component
 *
 */
CpaStatus
QatComms_Shutdown(void)
{
    /* if Communications not in progress .. exit with error */
    if (!qatComms_Initialised)
    {
        QATAL_ERROR_LOG(
                   "QatComms_Shutdown - QAT Comms not initialised\n");
        return CPA_STATUS_FAIL;
    }

    /* Release rings and if error exit with error */
    if (CPA_STATUS_SUCCESS != QatComms_RingsDestroy())
    {
        return CPA_STATUS_FAIL;
    }

    /* Clear ringinfo structure */
    QatComms_RingInfoClear();

    /* Set Communications not initialized to TRUE */
    qatComms_Initialised = CPA_FALSE;

    return CPA_STATUS_SUCCESS;
}
/*
 * @fn QatComms_ReqHdrCreate
 *
 *  Create the header of the message to be sent
 */
CpaStatus
QatComms_ReqHdrCreate(
    void *pMsg,
    icp_arch_if_request_t qatReqType)
{
    qat_hw_ring_handle hResponseRing = (int)QAT_NULL_RING_HANDLE;
    Cpa8U responseRingId = 0;
    qat_ring_info_t *pRingInfo = NULL;

    /* If communication not in progress EXIT with error */
    if (!qatComms_Initialised)
    {
        QATAL_ERROR_LOG
           ("QatComms_ReqHdrCreate - QAT Comms not initialised\n");
        return CPA_STATUS_FAIL;
    }
#ifdef  ICP_PARAM_CHECK

    /* Param checks */
    IX_OSAL_ENSURE_RETURN(NULL != pMsg,
                   "QatComms_ReqHdrCreate - Null pMsg param\n");
    IX_OSAL_ENSURE_RETURN(qatReqType < ICP_ARCH_IF_REQ_DELIMITER,
               "QatComms_ReqHdrCreate - Invalid qatReqType param\n");

#endif

    /* Get ring, based on request type */
    pRingInfo = &(qatComms_ringInfo[qatReqType]);
    hResponseRing = pRingInfo->hResponseRing;

#ifdef  ICP_PARAM_CHECK
    /* If null procedure to cater for response to this message
       exit with err*/
    if (QAT_NULL_RING_HANDLE == hResponseRing)
    {
        QATAL_ERROR_LOG
           ("QatComms_ReqHdrCreate - "
            "Response ring info not set, or invalid qatReqType \n");
        return CPA_STATUS_INVALID_PARAM;
    }
#endif


    /* Get response Ring id */
    responseRingId = QAT_RING_GET_INDEX(hResponseRing);

    /* Fill in request message header */
    QatComms_ReqHdrWrite(pMsg, qatReqType, responseRingId);

    return CPA_STATUS_SUCCESS;
}

/*
 * @fn QatComms_RingPut
 *
 *  Put the specified message of the Ring
 */
CpaStatus QatComms_RingPut(icp_ring_id_t ringId, Cpa32U *pData,
                              Cpa32U lwSize)
{
    qat_ring_dev_info_t *pRingDev = &(qat_ringDevInfo[ringId]);
    Cpa32U trgtAddr = 0;
    Cpa32U byteSize = QAT_MSG_SIZE;
    /* Note: Algo only supports a message size of 64 bytes */
    Cpa32U i = 0;

    /*
     * Note: Current implementation only works for putting a single msg
     * on the ring
     */

#ifdef ICP_PARAM_CHECK
    if (QAT_MSG_SIZE != (lwSize * 4))
    {
        /* If communication is not in progress .. exit with error */
        QATAL_ERROR_LOG
           ("QatComms_RingPut - Message size not configured\n");
        return CPA_STATUS_FAIL;
    }
#else
    /*if the ICP_PARAM_CHECK is off the function's attribute becomes unused
      gentle way of compiler's warning suppresion*/
    lwSize = lwSize;
#endif


    /* lock/reserve slot in Ring structure for update */
    QAT_SPINLOCK(&pRingDev->ringLock);

    /* Check shadow space in ring structure for space ..
       if no space for desired ring*/
    if(pRingDev->space < QAT_RING_FREE_MIN)
    {
        /* read hardware for ring information of Head */
        pRingDev->head = EAGLETAIL_RING_CSR_READ(ringId, ET_RING_HEAD_OFFSET);

        /* Note: Ring may not be empty
           - can't use pure size for the free space */
        if(pRingDev->head > pRingDev->tail)
        {
            pRingDev->space = pRingDev->head - pRingDev->tail;
        }
        else
        {
            pRingDev->space =
                pRingDev->size - (pRingDev->tail - pRingDev->head);
        }
    }

    if(pRingDev->space < QAT_RING_FREE_MIN)
    {
        QAT_SPINUNLOCK(&pRingDev->ringLock);
        return CPA_STATUS_FAIL;
    }

    /* calculate target address */
    trgtAddr = (Cpa32U)pRingDev->pVirAddr + (Cpa32U)pRingDev->tail;

    /* Copy the data into the ring */
    for(i=0; i<byteSize; i+=4)
    {
        *((Cpa32U*)(trgtAddr + i)) = *(pData++);
    }

    /* Update tail position in structure */
    pRingDev->tail = (pRingDev->tail + byteSize) % pRingDev->size;
    pRingDev->space -= byteSize;
    EAGLETAIL_RING_CSR_WRITE(ringId, ET_RING_TAIL_OFFSET, pRingDev->tail);

    /* Unlock Ring slot in shadow structure */
    QAT_SPINUNLOCK(&pRingDev->ringLock);

#ifdef ICP_DEBUG
    QATAL_DEBUG_LOG_OPTIONAL("Qatcomms - Put Info on Ring After\n");

    QatComms_RegConfigShow();
    QatComms_RingDevInfoPrint( ringId);
#endif

    return CPA_STATUS_SUCCESS;
}

/*
 * @fn QatComms_MsgSend
 *
 * Send the specified message with a specified priority and request type
 *
 */
CpaStatus
QatComms_MsgSend(
    void *pMsg,
    icp_arch_if_request_t qatReqType,
    qat_comms_priority_t priority,
    CpaInstanceHandle instanceHandle)
{
    qat_hw_ring_handle hRequestRing = (int)QAT_NULL_RING_HANDLE;
    Cpa8U requestRingId = 0;
    Cpa32U msgSizeInWords = QAT_MSG_SZ_WORDS;
    qat_ring_info_t *pRingInfo = NULL;

#ifdef ICP_DEBUG
    Cpa8U        *pDispMsg = NULL;
    int            i = 0;
#endif

    /* artificial usage of unused parameter */
    instanceHandle = instanceHandle;

#ifdef ICP_PARAM_CHECK
    icp_arch_if_req_hdr_t *pMsgHdr = (icp_arch_if_req_hdr_t *)pMsg;

    if (!qatComms_Initialised)
    {
        /* If communication is not in progress .. exit with error */
        QATAL_ERROR_LOG
           ("QatComms_MsgSend - QAT Comms not initialised\n");
        return CPA_STATUS_FAIL;
    }

    if (NULL == pMsgHdr)
    {
        /* IF Null message ..exit with errror */
        QATAL_ERROR_LOG
           ("QatComms_MsgSend - QAT Msg Hdr not initialised\n");
        return CPA_STATUS_FAIL;
    }

    /* Param checks */
    IX_OSAL_ENSURE_RETURN(NULL != pMsg,
                   "QatComms_MsgSend - Null pMsg param\n");
    IX_OSAL_ENSURE_RETURN(qatReqType < ICP_ARCH_IF_REQ_DELIMITER,
                   "QatComms_MsgSend - Invalid qatReqType param\n");
    IX_OSAL_ENSURE_RETURN((priority > QAT_COMMS_PRIORITY_START_DELIMITER) &&
                   (priority < QAT_COMMS_PRIORITY_END_DELIMITER),
                   "QatComms_MsgSend - Invalid priority param\n");
    IX_OSAL_ENSURE_RETURN((ICP_ARCH_IF_VALID_FLAG_GET(pMsgHdr->flags) != 0),
                   "QatComms_MsgSend - Invalid msg header\n");
#endif

    /* Get ring, based on request type & priority */
    pRingInfo = &(qatComms_ringInfo[qatReqType]);

    /* get request Ring handle */
    hRequestRing = pRingInfo->hRequestRing[priority];


#ifdef ICP_PARAM_CHECK
    if ((NULL == pRingInfo->pResponseCb) ||
        (QAT_NULL_RING_HANDLE == hRequestRing))
    {
        QATAL_ERROR_LOG
           ("QatComms_MsgSend - "
            "Err Request ring info/qatReqType/priority(%d/%d)\n");
        return CPA_STATUS_INVALID_PARAM;
    }
#endif

    /* get request ring id */
    requestRingId  = QAT_RING_GET_INDEX(hRequestRing);

#ifdef ICP_DEBUG
    {

        ixOsalStdLog("\n Writing request msg to ring %d\n",
                             (int)requestRingId);

       pDispMsg = (Cpa8U *)pMsg;

       ixOsalStdLog("Got ReqType...:");
       ixOsalStdLog("%d", qatReqType);
       ixOsalStdLog("\n");
       ixOsalStdLog("Message: ");

       for (i = 0; i < (QAT_MSG_SZ_WORDS * 4); i++)
       {
           ixOsalStdLog("%02X", pDispMsg[i]);
           if (i%4 == 3)
           {
               ixOsalStdLog(" ");
           }
       }
       ixOsalStdLog("\n");
    }
#endif

     /* Send message by putting message on Ring if failure exit with error */
    if(QatComms_RingPut(requestRingId, pMsg, msgSizeInWords)
                                                     != CPA_STATUS_SUCCESS)
    {
        /* Increment Message Retry */
        if(QAT_COMMS_STATISTICS_ON == qatCollectStatistics)
        {
            QatComms_ACK_RetryInc(qatReqType);
        }
        return CPA_STATUS_RETRY;
    }

    /* increment message sent */
    if(QAT_COMMS_STATISTICS_ON == qatCollectStatistics)
    {
        QatComms_ACK_ReqInc(qatReqType);
    }

    return CPA_STATUS_SUCCESS;
}

/*
 * @fn QatComms_ResponseCbSet
 *
 *  Register a callback to handle message responses for a given request type
 *
 */
CpaStatus
QatComms_ResponseCbSet(
    qat_comms_cb_func_t pResponseCb,
    icp_arch_if_request_t qatReqType)
{
    /* If communication not initialized ..exit with error */
    if (!qatComms_Initialised)
    {
        QATAL_ERROR_LOG
           ("QatComms_ResponseCbSet - QAT Comms not initialised\n");
        return CPA_STATUS_FAIL;
    }

    /* If invalid reqType ... exit with error */
    if (qatReqType >= ICP_ARCH_IF_REQ_DELIMITER)
    {
        QATAL_ERROR_LOG
           ("QatComms_ResponseCbSet - "
            "Invalid qatReqType param \n");

        return CPA_STATUS_INVALID_PARAM;
    }


    /* Copy the address of the function to be called when a response of the
       specifed reqtype is received */

    qatComms_ringInfo[qatReqType].pResponseCb = pResponseCb;

    return CPA_STATUS_SUCCESS;
}

/*
 * @fn QatComms_MsgCountGet
 *
 * return to caller the no. of messages sent & received when qatCollectStatistics
 * global variable is non ZERO. Otherwise function returns CPA_STATUS_RESOURCE
 * due to the fact that statistics are not being collected thus statistics are not
 * valid. Additionally all output values for:
 * messages sent/received and number of retries are left untouched
 *
 */

CpaStatus
QatComms_MsgCountGet(
                     icp_arch_if_request_t qatReqType,
                     Cpa32U *pNumSent,
                     Cpa32U *pNumReceived,
                     Cpa32U *pNumRetry)
{
    /* If data specified to receive no. of messages sent = NULL exit
       with error */
    if (NULL == pNumSent)
    {
        QATAL_ERROR_LOG
           ("QatComms_MsgCountGet - "
            "Failed to get message count - pNumSent == NULL \n");

        return CPA_STATUS_FAIL;
    }
    /* If data specified to receive no. of messages received =
       NULL exit with error */
    if (NULL == pNumReceived)
    {
        QATAL_ERROR_LOG
           ("QatComms_MsgCountGet - "
            "Failed to get message count - pNumReceived == NULL \n");

        return CPA_STATUS_FAIL;
    }
    if (NULL == pNumRetry)
    {
        QATAL_ERROR_LOG
           ("QatComms_MsgCountGet - "
            "Failed to get message count - pNumRetry == NULL \n");

        return CPA_STATUS_FAIL;
    }

    if (qatReqType > ICP_ARCH_IF_REQ_DELIMITER)
    {
           QATAL_ERROR_LOG
              ("QatComms_MsgCountGet - "
               "Failed to get message count - ReqType out of Range \n");

           return CPA_STATUS_FAIL;
    }

    if(QAT_COMMS_STATISTICS_ON == qatCollectStatistics)
    {
        /* Extract messages sent from Atomic Item */
        *pNumSent = ixOsalAtomicGet (&qat_comms_ACK[qatReqType][QAT_SENT]);

        /* Extract messages received from Atomic Item */
        *pNumReceived =
            ixOsalAtomicGet (&qat_comms_ACK[qatReqType][QAT_RECEIVED]);

        /* Extract messages retry */
        *pNumRetry =
            ixOsalAtomicGet (&qat_comms_ACK[qatReqType][QAT_RETRY]);
    } 
    else 
    {
        return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}

/*
 * @fn QatComms_RegConfigShow
 *
 * Display Ring information
 *
 */
void
QatComms_RegConfigShow(void)
{
    /* **Caution ** Caution *** Caution ***

       Enabling ICP_DEBUG will casue the following to be
       output. This may cause too much debug information to go to debug console.
       You may need to enable/disable the output of this fuction by hand using
   */

    Cpa32U csrVal = 0;

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_CONFIG +
        (ICP_RING_LAC_LA_RESPONSE << 2));
    QATAL_STD_LOG (
        "\n (ET_RING_CONFIG + (ICP_RING_LAC_LA_RESPONSE << 2)) \n" \
        "Ring CSR: offset: 0x%04x ,value: 0x%08x \n",
        (ET_RING_CONFIG + (ICP_RING_LAC_LA_RESPONSE << 2)) , csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_SRCSEL_0);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_SRCSEL_0 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x\n",
        ET_RING_IA_INT_SRCSEL_0, csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_SRCSEL_1);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_SRCSEL_1 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x \n",
        ET_RING_IA_INT_SRCSEL_1, csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_SRCSEL_2);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_SRCSEL_2 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x \n",
        ET_RING_IA_INT_SRCSEL_2, csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_COL_EN_0);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_COL_EN_0 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x \n",
        ET_RING_IA_INT_COL_EN_0, csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_COL_EN_1);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_COL_EN_1 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x \n",
        ET_RING_IA_INT_COL_EN_1, csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_COL_CTL_0);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_COL_CTL_0 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x \n",
        ET_RING_IA_INT_COL_CTL_0, csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_COL_CTL_1);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_COL_CTL_1 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x \n",
        ET_RING_IA_INT_COL_CTL_1, csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_EN_0);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_EN_0 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x \n",
        ET_RING_IA_INT_EN_0 , csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_EN_1);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_EN_1 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x \n",
        ET_RING_IA_INT_EN_1 , csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_REG_0);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_REG_0 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x \n",
        ET_RING_IA_INT_REG_0 , csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_REG_1);
    QATAL_STD_LOG (
        "\n ET_RING_IA_INT_REG_1 \n" \
        "Ring CSR: offset: 0x%04x, value: 0x%08x \n",
        ET_RING_IA_INT_REG_1 , csrVal
        );

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_E_STAT_0);
    QATAL_STD_LOG (
        "\n ET_RING_E_STAT_0 Ring CSR: E_STAT_0:  0x%08x\n",
        csrVal);

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_E_STAT_1);
    QATAL_STD_LOG (
        "ET_RING_E_STAT_1 Ring CSR: E_STAT_1:  0x%08x\n",
        csrVal);

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_NE_STAT_0);
    QATAL_STD_LOG(
        "ET_RING_NE_STAT_0 Ring CSR: NE_STAT_0: 0x%08x\n",
        csrVal);

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_NF_STAT_0);
    QATAL_STD_LOG (
       "ET_RING_NF_STAT_0 Ring CSR: NF_STAT_0: 0x%08x\n",
       csrVal);

    csrVal = EAGLETAIL_RING_CSR_READ_RAW(ET_RING_F_STAT_0);
    QATAL_STD_LOG(
        "ET_RING_F_STAT_0 Ring CSR: F_STAT_0:  0x%08x\n",
        csrVal);


}

/*
 * @fn QatComms_bh_handler
 *
 * In case of coalaseing interrupts. This is invoked by ASD when a ring
 * has data.
 *
 */
void
QatComms_bh_handler(void* priv_data, int reserved)
{

    Cpa32U csrVal = 0;
    Cpa32U i = 0;

   /* Suppresion of unused argument compilation warning - a no-effect statement 
    * removed by compilers' optimiser 
    */
    priv_data = priv_data;

    /* reserved for the future extensions - 
       will removed by the compiler's optimiser */
    reserved  = reserved;

    /* Read the ring status CSR to determine which rings have data */
    csrVal = ~(EAGLETAIL_RING_CSR_READ_RAW(QAT_ICP_RING_CSR_E_STAT));

    /* If one of our response rings has data to be processed */
    csrVal &= respRingMask0_31;

    if(csrVal != 0)
    {
        /* Invoke the response handler for each ring with data */
        for(i=0; i<numRespRings; ++i)
        {
            if(csrVal & (1 << respRingIds[i]))
            {
                QatComms_ResponseMsgHandler((Cpa32U )respRingIds[i],
                    (void *) 0);
            }
        }
    }

    /* Re-enable interrupts */
    if (qatCoalescedInterruptsEnabled)
    {
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_COL_CTL_0,
            g_RingIntrColCtl);
    }
    else
    {
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_REG_0, respRingMask0_31);
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_EN_0, respRingMask0_31);
    }
}

CpaStatus QatComms_intr(void)
{
    CpaStatus status;

    /* Disable Ring interrupts */
    if (qatCoalescedInterruptsEnabled)
    {
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_COL_CTL_0, 0);
        EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_COL_CTL_0);
    }
    else
    {
        EAGLETAIL_RING_CSR_WRITE_RAW(ET_RING_IA_INT_EN_0, 0);
        EAGLETAIL_RING_CSR_READ_RAW(ET_RING_IA_INT_EN_0);
    }
    status = (CpaStatus)asd_bh_schedule();
    return status;
}

CpaStatus QatComms_bh_schedule_register(QatComms_bh_schedule bhsch)
{
    asd_bh_schedule = bhsch ;

    if (asd_bh_schedule == NULL)
    {
         return CPA_STATUS_FAIL;
    }

    return CPA_STATUS_SUCCESS;

}

/**
 * NAME: QatComms_setQatCollectStatistics
 *
 * DESCRIPTION: This function will set the statistics collection
 *
 * @Param:  - IN collectStatistics QAT_COMMS_STATISTICS_OFF: OFF
 *                                 QAT_COMMS_STATISTICS_ON:  ON
 *
 * @Return: IX_SUCCESS
 *
 */
CpaStatus QatComms_setQatCollectStatistics(const qat_comms_statistics_t
                                           collectStatistics)
{
    qatCollectStatistics = collectStatistics;

    return CPA_STATUS_SUCCESS;
}

/**
 * NAME: QatComms_getQatCollectStatistics
 *
 * DESCRIPTION: This function will get the statistics collection seting
 *
 * @Param:  - (void)
 *
 * @Return: current value  of qatCollectStatistics:
 *                QAT_COMMS_STATISTICS_OFF: OFF
 *                QAT_COMMS_STATISTICS_ON:  ON
 *
 */
qat_comms_statistics_t QatComms_getQatCollectStatistics(void)
{
    return qatCollectStatistics;
}


/**
 * NAME: QAT_hw_ring_delete
 *
 * DESCRIPTION: This function will delete a ring in SRAM/DRAM on the specified
 *          channel.
 *
 * @Param:  - IN arg_RingId - specifies the internal ICP ring Id to be
 *          associated with the ring.
 *
 * @Return: IX_SUCCESS
 *
 */
CpaStatus QAT_comm_pipe_delete (
                                Cpa8U ringId
                                )
{

    if (ringId > 31)
    {
        QATAL_ERROR_LOG("QAT_comm_pipe_delete: Invalid Pipe ID \n");
        return CPA_STATUS_FAIL;
    }

    EAGLETAIL_RING_CSR_WRITE(ringId, ET_RING_HEAD_OFFSET , 0 );
    EAGLETAIL_RING_CSR_WRITE(ringId, ET_RING_TAIL_OFFSET , 0 );
    EAGLETAIL_RING_CSR_WRITE(ringId, ET_RING_BASE, 0 );
    EAGLETAIL_RING_CSR_WRITE(ringId, ET_RING_CONFIG, 0 );

    QATAL_MEM_SPECIAL_FREE (&qat_ringDevInfo[ringId].pVirAddr,   \
                           (QAT_MSG_SIZE * QAT_NO_OF_MSG));

    return CPA_STATUS_SUCCESS;

}

/*
 *
 * NAME: QAT_comm_pipe_create
 *
 * DESCRIPTION: This function creates communication pipes using the ICP pipes
 * for IA/AE communication.
 * The specified pipe id (by the
 * input parameter) the corresponding arg_PipeCount ICP pipes
 * will be created.
 *
 */
CpaStatus QAT_comm_pipe_create(
                                 qat_comm_pipe_info* arg_CommPipeInfo,
                                 void ** pVirAddr
                                 )
{

    Cpa32U channel =  IX_MEMORY_CHANNEL_COHERENT_DRAM; /* coherent DRAM */
    CpaStatus status = CPA_STATUS_SUCCESS;
    qat_ring_addr pMemory;
    Cpa32U * pRingMemory = NULL;
    qat_hw_ring_handle ringHandle = (int)QAT_NULL_RING_HANDLE;
    ix_error err2 = IX_SUCCESS;

    /* initialize pMemory */
    pMemory.value = 0;

    if (arg_CommPipeInfo->m_PipeId > 31)
    {
        QATAL_ERROR_LOG("QAT_comm_pipe_create: Invalid Pipe ID \n");
        return CPA_STATUS_FAIL;
    }

    status =  QATAL_MEM_SPECIAL_ALLOC (&pRingMemory,   \
                                       QAT_MSG_SIZE * QAT_NO_OF_MSG);

    if (pRingMemory == NULL)
    {
        QATAL_ERROR_LOG("QAT_hw_ring_create: mem_alloc_aligned Failed \n");
        return CPA_STATUS_FAIL;
    }

    if (pVirAddr == NULL)
    {
        QATAL_ERROR_LOG("QAT_hw_ring_create:storage of target Failed \n");
        return CPA_STATUS_FAIL;
    }

    *pVirAddr = pRingMemory ;

    /*
     * convert it into a physical address before creating the rings
     */
    status = QATAL_MEM_SHARED_VIRT_TO_PHYS((void*)pRingMemory,
                    (Cpa64U *)&pMemory.pointer);
    if (status!= CPA_STATUS_SUCCESS)
    {
        QATAL_ERROR_LOG("QAT_hw_ring_create: virt_to_phys Failed \n");
        return CPA_STATUS_FAIL;
    }


    /* create the ring handle */
    ringHandle = QAT_CREATE_RING_HANDLE(
            QAT_RING_TYPE,
            QAT_MEMORY,
            channel,
            arg_CommPipeInfo->m_PipeId);

    err2 = QAT_RING_CREATE(QAT_MEMORY, channel, arg_CommPipeInfo->m_PipeId,
                IX_ICP_RING_SIZE_16K , QAT_RING_NEAR_WATERMARK_0,
                QAT_RING_NEAR_WATERMARK_0,  pMemory.value);

    if (err2 != HALAE_SUCCESS)
    {
        ixOsalStdLog      \
        ("QAT_hw_ring_create: QAT_RING_CREATE Failed 0x%08x \n",err2);
        return CPA_STATUS_FAIL;
    }

    arg_CommPipeInfo->m_PipeHandle = ringHandle;


    if(CPA_STATUS_SUCCESS == status)
    { /* ring created */

          /* get the pipe id from the pipe handle */
        (arg_CommPipeInfo->m_PipeId) = QAT_RING_GET_INDEX(
                                         arg_CommPipeInfo->m_PipeHandle);
    }
    else
    {

        QATAL_ERROR_LOG("QAT_comm_pipe_create: QAT_hw_ring_create Failed \n");
        return CPA_STATUS_FAIL;
    }

    return status;
}

