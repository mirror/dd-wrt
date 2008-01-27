/**
 * @file IxAtmdTxTransport_p.h
 *
 * @author Intel Corporation
 * @date 17 March 2002
 *
 * @brief IxAtmdAcc Tx Transport
 *
 * The functions in this module are carrying the datapath and are time-critical
 *
 * Design Notes:
 * The implementation of this module focus on performances
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

#ifndef IX_ATMDACC_TX_TRANSPORT_P_H
#define IX_ATMDACC_TX_TRANSPORT_P_H

#include "IxQMgr.h"
#include "IxAtmdAccCtrl.h"
#include "IxAtmdNpe_p.h"
#include "IxAtmdDefines_p.h"
#include "IxAtmdSwQueue_p.h"

/**
* @def IX_ATMDACC_TX_TRANSPORT_EXTERN
* @brief This macro controls the declaration of data in IxAtmdTxTransport.c
*        and their definition as extern in other files
*/
#ifndef IX_ATMDACC_TX_TRANSPORT_EXTERN
#define IX_ATMDACC_TX_TRANSPORT_EXTERN extern
#endif



/**
* @def IX_ATMDACC_TX_INVALID_CONNID
* @brief Invalid connection Id, used to flag an unused channel
*/
#define IX_ATMDACC_TX_INVALID_CONNID (IX_ATM_MAX_NUM_AAL_OAM_TX_VCS - 1)

/**
* @def IX_ATMDACC_TX_VC_INUSE
* @brief Check if a channel is used using the connectionId value
*/
#define IX_ATMDACC_TX_VC_INUSE(connId) \
        (((connId) / IX_ATM_MAX_NUM_AAL_OAM_TX_VCS) != 0)

/**
* @def IX_ATMDACC_TX_VC_INDEX_GET
* @brief Get the Npe Vc Id from the connection Id
*/
#define IX_ATMDACC_TX_VC_INDEX_GET(connId) \
        ((connId) % IX_ATM_MAX_NUM_AAL_OAM_TX_VCS)

/**
* @def IX_ATMDACC_TX_DISCONNECTCHECK
* @brief Set a connection Id to mark that there is a disconnect in progress
*/
#define IX_ATMDACC_TX_DISCONNECTCHECK(connId, descConnId) \
        ((descConnId) == ((connId) + IX_ATM_MAX_NUM_AAL_OAM_TX_VCS))
/**
* @def IX_ATMDACC_TX_DISCONNECTING
* @brief Set a connection Id to mark that there is a disconnect in progress
*/
#define IX_ATMDACC_TX_DISCONNECTING(connId) \
        ((connId) + IX_ATM_MAX_NUM_AAL_OAM_TX_VCS)

/**
* @def IX_ATMDACC_TX_DISCONNECTED
* @brief Check if a channel is being disconnected
*/
#define IX_ATMDACC_TX_DISCONNECTED(connId) \
        ((((connId) / IX_ATM_MAX_NUM_AAL_OAM_TX_VCS) & 1) == 0)

/**
* @def IX_ATMDACC_TX_QUEUE_ENTRY_TYPE
* @brief Define the type of an Tx Queue Entry
*/
#define IX_ATMDACC_TX_QUEUE_ENTRY_TYPE IxAtmdAccNpeDescriptor

/**
* @brief Generate a typedef for a Tx specific s/w queue
*/
IX_ATMDACC_SWQ_TYPE(IX_ATMDACC_TX_QUEUE_ENTRY_TYPE);

/**
* @def IX_ATMDACC_TX_QUEUE
* @brief Define a short form for the type of a Tx Queue
*        The short form IX_ATMDACC_TX_QUEUE is defined
*        by using the ## operator to concatenate
*        the fixed string "IxAtmdAccSwQueueOfType_" with the
*        actual type being passed as a macro parameter.
*        The resulting string is identitical with the typedef
*        name created by macro IX_ATMDACC_SWQ_TYPE.
*/
#define IX_ATMDACC_TX_QUEUE IxAtmdAccSwQueueOfType_ ## IX_ATMDACC_TX_QUEUE_ENTRY_TYPE

/**
* @def IX_ATMDACC_TXQ_INIT
* @brief Initialise the Tx queue to the specified size
*/
#define IX_ATMDACC_TXQ_INIT(queue,size,buffer) \
    IX_ATMDACC_SWQ_STATIC_INIT((queue), IX_ATMDACC_TX_QUEUE_ENTRY_TYPE, (size),(buffer)); \
    IX_ATMDACC_SWQ_HEAD_ADVANCE((queue),IX_ATMDACC_SWQ_SIZE(queue))

/**
* @def IX_ATMDACC_TXQ_INITIALISED
* @brief Check that the Initialisation of the Tx Queue was successfull
*/
#define IX_ATMDACC_TXQ_INITIALISED(queue) \
    IX_ATMDACC_SWQ_INITIALISED(queue)

/**
* @def IX_ATMDACC_TXQ_RESET
* @brief Reset the queue to initial empty values
*/
#define IX_ATMDACC_TXQ_RESET(queue) \
    IX_ATMDACC_SWQ_RESET(queue)

/**
* @def IX_ATMDACC_TXQ_RELEASE_RESOURCES
* @brief Release any memory allocated to an Tx Queue
*/
#define IX_ATMDACC_TXQ_RELEASE_RESOURCES(queue) \
    IX_ATMDACC_SWQ_RELEASE_RESOURCES(queue)

/**
* @def IX_ATMDACC_TXQ_SIZE
* @brief return the size of a queue
*/
#define IX_ATMDACC_TXQ_SIZE(queue) \
    IX_ATMDACC_SWQ_SIZE(queue)

/**
* @def IX_ATMDACC_TXQ_ENTRY_IDXSET
* @brief Set an entry ditrectly at the specified index in the Tx queue
*/
#define IX_ATMDACC_TXQ_ENTRY_IDXSET(queue,index,entry) \
    IX_ATMDACC_SWQ_ENTRY_IDXSET(queue,index,entry)

/**
* @def IX_ATMDACC_TXQ_ENTRY_IDXGET
* @brief return an entry ditrectly from the specified index in the Tx queue
*/
#define IX_ATMDACC_TXQ_ENTRY_IDXGET(queue,index) \
    IX_ATMDACC_SWQ_ENTRY_IDXGET(queue,index)

/**
* @def IX_ATMDACC_TXQ_CONSISTENT
* @brief Check the software TX Queue consistency
*
* These conditions should always be true. If not, this means that
* the software TX queue is badly corrupted.
*
*/
#define IX_ATMDACC_TXQ_CONSISTENT(queue) \
       ((((queue)->head - (queue)->tail) >= 0) && \
        (((queue)->head - (queue)->mid)  >= 0) && \
        (((queue)->mid  - (queue)->tail) >= 0) && \
        (((queue)->head - (queue)->mid)  <= (queue)->size) && \
        (((queue)->mid  - (queue)->tail) <= (queue)->size) && \
        (((queue)->head - (queue)->tail) <= (queue)->size))

/**
* @def IX_ATMDACC_TXQ_TAIL
* @brief Get the Tail pointer of a software TX Queue
*/
#define IX_ATMDACC_TXQ_TAIL(queue) \
    IX_ATMDACC_SWQ_TAIL(queue)

/**
* @def IX_ATMDACC_TXQ_TAIL_INCR
* @brief Increment the Tail pointer of a software TX Queue
*
* @note - This action is done during a TxProcessPerform step
*/
#define IX_ATMDACC_TXQ_TAIL_INCR(queue) \
    IX_ATMDACC_SWQ_TAIL_INCR(queue)

/**
* @def IX_ATMDACC_TXQ_HEAD
* @brief Get the Head pointer of a software TX Queue
*/
#define IX_ATMDACC_TXQ_HEAD(queue) \
    IX_ATMDACC_SWQ_HEAD(queue)

/**
* @def IX_ATMDACC_TXQ_HEAD_INCR
* @brief Increment the Head pointer of a software TX Queue
*
* @note - This action is done during a TxDone step
*/
#define IX_ATMDACC_TXQ_HEAD_INCR(queue) \
    IX_ATMDACC_SWQ_HEAD_INCR(queue)

/**
* @def IX_ATMDACC_TXQ_MID
* @brief Get a pointer inside a software TX Queue
*/
#define IX_ATMDACC_TXQ_MID(queue) \
    IX_ATMDACC_SWQ_MID(queue)

/**
* @def IX_ATMDACC_TXQ_MID_INCR
* @brief Increment a pointer of a software TX Queue
*
* @note - This action is done during a TxPduSubmit step
*/
#define IX_ATMDACC_TXQ_MID_INCR(queue) \
    IX_ATMDACC_SWQ_MID_INCR(queue)

/**
* @def IX_ATMDACC_TXQ_MID_DECR
* @brief Rollback a PDU submit from a software TX Queue
*
* @note - This action is done during a TxPduSubmit rollback
*/
#define IX_ATMDACC_TXQ_MID_DECR(queue) \
    IX_ATMDACC_SWQ_MID_DECR(queue)

/**
* @def IX_ATMDACC_TXQ_OVERLOADED
* @brief Check if a software TX Queue is overloaded
*/
#define IX_ATMDACC_TXQ_OVERLOADED(queue) \
        ((queue)->head == (queue)->mid)

/**
* @def IX_ATMDACC_TXQ_SCHEDULE_PENDING
* @brief Check if elements are waiting for scheduling in a software TX Queue
*/
#define IX_ATMDACC_TXQ_SCHEDULE_PENDING(queue) \
        ((queue)->tail != (queue)->mid)

/**
* @def IX_ATMDACC_TXQ_RECYCLE_PENDING
* @brief Check in a software TX Queue if descriptors are still used by the Npe or waiting in the txDone queue
*/
#define IX_ATMDACC_TXQ_RECYCLE_PENDING(queue) \
        (((queue)->head - (queue)->tail) != (queue)->size)

/**
* @def IX_ATMDACC_TXQ_TAIL_ENTRY_GET
* @brief Get a @a IxAtmdAccNpeDescriptor at the tail of a software TX Queue
*/
#define IX_ATMDACC_TXQ_TAIL_ENTRY_GET(queue) \
    IX_ATMDACC_SWQ_TAIL_ENTRY_GET(queue)

/**
* @def IX_ATMDACC_TXQ_HEAD_ENTRY_GET
* @brief Get a @a IxAtmdAccNpeDescriptor at the head of a software TX Queue
*/
#define IX_ATMDACC_TXQ_HEAD_ENTRY_GET(queue) \
    IX_ATMDACC_SWQ_HEAD_ENTRY_GET(queue)

/**
* @def IX_ATMDACC_TXQ_MID_ENTRY_GET
* @brief Get a @a IxAtmdAccNpeDescriptor inside a software TX Queue
*/
#define IX_ATMDACC_TXQ_MID_ENTRY_GET(queue) \
    IX_ATMDACC_SWQ_MID_ENTRY_GET(queue)

/**
* @def IX_ATMDACC_TX_SCHEDULE_TABLE_SIZE
* @brief schedule table size used to build the entries needed
* to fill the tx vc queue over the threshold level. This is set
* to the maximum queue size.
*/
#define IX_ATMDACC_TX_SCHEDULE_TABLE_SIZE  (IX_QMGR_Q_SIZE_INVALID - 1)

/**
*
* @brief IxAtmdAcc Tx channel information
*
* The information in this structure is used on a per-channel basis.
*
* This structure contains
* @li The channel characteristics (port/vpi/vci ..)
* @li The user callback functions and Id to be used when a mbuf is transmitted
* @li An internal software queue to store incoming Pdus, process TX schedule and TX done recycling
* @li The current information about the pdu being transmitted
*
*/
typedef struct
{
    IxAtmConnId connId;                         /**< channel Connection Id */
    IxAtmLogicalPort port;                      /**< channel port */
    unsigned int vpi;                           /**< channel vpi */
    unsigned int vci;                           /**< channel vci */
    IxNpeA_AalType npeAalType;                  /**< the npe service type for this connection */
    unsigned int cellSize;                      /**< the cell size 48/52 for this connection */
    IxAtmSchedulerVcId schedulerVcId;           /**< scheduler Id for this channel */
    IxQMgrCallbackId callbackId;                /**< userId passed in a callback */
    IxAtmdAccTxVcBufferReturnCallback txDoneCallback; /**< user callback */
    IxAtmdAccNpeDescriptor *currentNpeDesc;     /**< current entry being scheduled */
    unsigned int remainingPduCellCount;         /**< counter of remaining cells for this PDU */
    volatile BOOL pduTransmitInProgress;        /**< This flag is set when
                                                            a pdu is being submitted. When
                                                            set, it is not safe to
                                                            force the data recycling during a
                                                            @a ixAtmdAccTxDisconnect() step */
    void * swQueueBuffer[IX_NPE_A_CHAIN_DESC_COUNT_MAX]; /**< data space for a sw queue. The maximum 
                                                        * size of the queue is driven by the number of PDUs 
                                                        * the NPE can chain */
    IX_ATMDACC_TX_QUEUE queue;                  /**< Transmit s/w queue */

#ifndef NDEBUG
    unsigned int txSubmitOverloadedCount;       /**< Counter of overloaded conditions */
    unsigned int txVcPduSubmitFailureCount;     /**< failures during txvcPduSubmit */
#endif
} IxAtmdAccTxVcDescriptor;

/**
* @enum IxAtmdAccPortStatus
* @brief Logical Port status
*/
typedef enum
{
    IX_ATMDACC_PORT_DOWN = 0,
    IX_ATMDACC_PORT_UP,
    IX_ATMDACC_PORT_DOWN_PENDING
} IxAtmdAccPortStatus;

/**
*
* @brief AtmdAcc port information
*
* The information in this structure is used on a per-port basis.
*
* This structure contains
* @li The user callback functions and Id to be used during an event notification
* @li The current information about the queue setup
* @li Flags to avoid reentrancy in Critical code
*
*/
typedef struct
{
    IxAtmdAccPortTxLowCallback txLowCallback; /**< User callback
                                     * invoked when the queue level is going
                                     * low
                                     */
    IxQMgrQId txQueueId;             /**< Tx queue associated
                                     *     with this port
                                     */
    unsigned int txQueueSize;        /**< Tx queue size
                                     */
    unsigned int txQueueThreshold;   /**< Tx queue threshold (0 if not used)
                                     */
    IxAtmScheduleTableEntry scheduleTable[IX_ATMDACC_TX_SCHEDULE_TABLE_SIZE]; /**< 
                                     * schedule table used to build the last entries
                                     * to be filled up to the threshold level
                                     */
    IxAtmdAccPortStatus status;     /**< Logical port status (down or up)
                                     */
    IxAtmdAccTxVcDemandUpdateCallback schDemandUpdate; /**< Atm Scheduling entity
                                     * associated with this port
                                     */
    IxAtmdAccTxVcDemandClearCallback schDemandClear;   /**< Atm Scheduling entity
                                     * associated with this port
                                     */
    IxAtmdAccTxSchVcIdGetCallback schVcIdGet;  /**< Atm Scheduling entity
                                     * associated with this port
                                     */
    volatile BOOL schedulingInProgress;  /**< This flag is set when
                                     * a schedule table is being processed. When
                                     * set, it is not safe to
                                     * force the data recycling during a
                                     * @a ixAtmdAccTxVcTryDisconnect() step
                                     */
    UINT32 qMgrEntries[IX_ATMDACC_TX_SCHEDULE_TABLE_SIZE]; /**<
                                     * array of entries for qMgr burst write 
                                     */
} IxAtmdAccPortDescriptor;


#ifndef NDEBUG
/**
*
* @brief AtmdAcc Tx transport stats
*
* The information in this structure is used to log possible
* runTime errors.
*
*/
typedef struct
{
    unsigned int txProcessInvokeCount;       /**< schedule table submitted */
    unsigned int txProcessFailedCount;       /**< schedule table failure */
    unsigned int idleCellScheduledCount;     /**< idle cells scheduled */
    unsigned int dataCellScheduledCount;     /**< data cells scheduled */
    unsigned int pduScheduledCount;          /**< pdu scheduled */
    unsigned int overScheduledCellCount;     /**< Number of overscheduled cells */
    unsigned int wrongConnIdScheduledCount;  /**< wrong connId scheduled */
    unsigned int disconnectScheduledCount;   /**< scheduled during diconnect */
    unsigned int txProcessUnderscheduledCount; /**< underscheduled */
    unsigned int zeroCellCount; /**< Number of times 0 cells are scheduled by the
                                scheduler */
} IxAtmdAccTxTransportStats;


/**
*
* @brief AtmdAcc txDone transport stats
*
* The information in this structure is used to log possible
* runTime errors.
*
*/
typedef struct
{
    unsigned int invokeCount;        /**< txDone invoke */
    unsigned int failedCount;        /**< txDone failures */
    unsigned int pduCount;           /**< txDone pdu received */
    unsigned int ackCount;           /**< txDone disable ack received */
    unsigned int maxPduPerDispatch;  /**< Maximum number of PDus per dispatch */
    unsigned int descriptorOrderErrorCount; /**< descriptors out of order from npe */
} IxAtmdAccTxDoneDispatchStats;

/**
* @brief txDone stats informations
*/
IX_ATMDACC_TX_TRANSPORT_EXTERN IxAtmdAccTxDoneDispatchStats ixAtmdAccTxDoneDispatchStats;

/**
* @brief tx stats informations
*/
IX_ATMDACC_TX_TRANSPORT_EXTERN IxAtmdAccTxTransportStats ixAtmdAccTxTransportStats[IX_UTOPIA_MAX_PORTS];

#endif /* NDEBUG */

/**
* @brief pool of port information
*/
IX_ATMDACC_TX_TRANSPORT_EXTERN IxAtmdAccPortDescriptor ixAtmdAccPortDescriptor[IX_UTOPIA_MAX_PORTS];

/**
* @brief pool of channel slots
*/
IX_ATMDACC_TX_TRANSPORT_EXTERN IxAtmdAccTxVcDescriptor ixAtmdAccTxVcDescriptor[IX_ATM_MAX_NUM_AAL_OAM_TX_VCS];

/**
* @brief user callback to be used when the threshold level of the txdone queue is reached
*/
IX_ATMDACC_TX_TRANSPORT_EXTERN IxAtmdAccTxDoneDispatcher ixAtmdAccTxDoneDispatcher;

/**
* @brief buffer used to dump the contents of the tx done queue
*
*  This buffer is done to accomodate the tx done burst read. There is an extra entry
*  which is used as a null pointer to mark the end of the data when the full queue
* is read.
*/
IX_ATMDACC_TX_TRANSPORT_EXTERN UINT32 ixAtmdAccTxDoneDispatchBuffer[IX_ATMDACC_TXDONE_QUEUE_SIZE + 1];

/**
* @brief number of ports configured in the system
*/
IX_ATMDACC_TX_TRANSPORT_EXTERN IxAtmLogicalPort ixAtmdAccTxNumPortConfigured;

#endif /* IX_ATMDACC_TX_TRANSPORT_P_H */


