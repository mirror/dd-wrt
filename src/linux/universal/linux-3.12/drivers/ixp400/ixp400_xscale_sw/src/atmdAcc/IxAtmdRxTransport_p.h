/**
 * @file IxAtmdRxTransport_p.h
 *
 * @author Intel Corporation
 * @date 17 March 2002
 *
 * @brief IxAtmdAcc Rx Transport
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

#ifndef IXATMDRXTRANSPORT_P_H
#define IXATMDRXTRANSPORT_P_H

/**
* @def IX_ATMDACC_RX_TRANSPORT_EXTERN
* @brief This macro controls the declaration of data in IxAtmdRxTransport.c
*        and their definition as extern in other files
*/
#ifndef IX_ATMDACC_RX_TRANSPORT_EXTERN
#define IX_ATMDACC_RX_TRANSPORT_EXTERN extern
#endif

#include "IxQMgr.h"
#include "IxAtmdAccCtrl.h"
#include "IxAtmdDefines_p.h"
#include "IxAtmdNpe_p.h"
#include "IxAtmdSwQueue_p.h"

/**
* @def IX_ATMDACC_RX_VCQ_ID_VALID
* @brief QueueId range verifications
*/
#define IX_ATMDACC_RX_VCQ_ID_VALID(queueId) \
        ((queueId) == IX_NPE_A_QMQ_ATM_RX_HI || \
        (queueId) == IX_NPE_A_QMQ_ATM_RX_LO)

/**
* @def IX_ATMDACC_RX_FREEQ_ID_VALID
* @brief QueueId range verifications
*/
#define IX_ATMDACC_RX_FREEQ_ID_VALID(queueId) \
        ((queueId) >= IX_NPE_A_QMQ_ATM_RXFREE_MIN || \
        (queueId) <= IX_NPE_A_QMQ_ATM_RXFREE_MAX)

/**
* @def IX_ATMDACC_RX_CALLBACK_ID_VALID
* @brief CallbackId range verifications
*/
#define IX_ATMDACC_RX_CALLBACK_ID_VALID(cbId) \
        ((cbId) < IX_ATM_MAX_RX_STREAMS)

/**
* @def IX_ATMDACC_RX_FREECALLBACK_ID_VALID
* @brief CallbackId range verifications
*/
#define IX_ATMDACC_RX_FREECALLBACK_ID_VALID(cbId) \
        ((cbId) < IX_ATM_MAX_NUM_AAL_OAM_RX_VCS)


/**
* @def IX_ATMDACC_RX_INVALID_CONNID
* @brief Invalid connection Id, used to flag an unused channel
*/
#define IX_ATMDACC_RX_INVALID_CONNID (IX_ATM_MAX_NUM_AAL_OAM_RX_VCS - 1)

/**
* @def IX_ATMDACC_RX_VC_INUSE
* @brief Check if a channel is used using the connectionId value
*/
#define IX_ATMDACC_RX_VC_INUSE(connId) \
        (((connId) / IX_ATM_MAX_NUM_AAL_OAM_RX_VCS) != 0)

/**
* @def IX_ATMDACC_RX_VC_INDEX_GET
* @brief Get the Npe Vc Id from the connection Id
*/
#define IX_ATMDACC_RX_VC_INDEX_GET(connId) \
        ((connId) % IX_ATM_MAX_NUM_AAL_OAM_RX_VCS)

/**
* @def IX_ATMDACC_RX_DISCONNECTING
* @brief Set a connection Id to mark that there is a disconnect in progress
*/
#define IX_ATMDACC_RX_DISCONNECTING(connId) \
        ((connId) + IX_ATM_MAX_NUM_AAL_OAM_RX_VCS)

/**
* @def IX_ATMDACC_RX_DISCONNECTED
* @brief Check if a channel is being disconnected
*/
#define IX_ATMDACC_RX_DISCONNECTED(connId) \
        ((((connId) / IX_ATM_MAX_NUM_AAL_OAM_RX_VCS) & 1) == 0)

/**
* @def IX_ATMDACC_RX_QUEUE_ENTRY_TYPE
* @brief Define the type of an Rx Queue Entry
*/
#define  IX_ATMDACC_RX_QUEUE_ENTRY_TYPE IxAtmdAccNpeDescriptor

/**
* @brief Generate a typedef for a Rx specific s/w queue
*/
IX_ATMDACC_SWQ_TYPE(IX_ATMDACC_RX_QUEUE_ENTRY_TYPE);

/**
* @def IX_ATMDACC_RX_QUEUE
* @brief Define a short form for the type of an Rx Queue
*/
#define IX_ATMDACC_RX_QUEUE IxAtmdAccSwQueueOfType_ ## IX_ATMDACC_RX_QUEUE_ENTRY_TYPE

/**
* @def IX_ATMDACC_RXQ_INIT
* @brief Initialise the Rx queue to the specified size
*/
#define IX_ATMDACC_RXQ_INIT(queue,size,buffer) \
IX_ATMDACC_SWQ_STATIC_INIT((queue), IX_ATMDACC_RX_QUEUE_ENTRY_TYPE, (size),(buffer)); \
IX_ATMDACC_SWQ_HEAD_ADVANCE((queue),IX_ATMDACC_SWQ_SIZE(queue))

/**
* @def IX_ATMDACC_RXQ_INITIALISED
* @brief Check that the Initialisation of the Rx Queue was successfull
*/
#define IX_ATMDACC_RXQ_INITIALISED(queue) \
IX_ATMDACC_SWQ_INITIALISED(queue)

/**
* @def IX_ATMDACC_RXQ_RELEASE_RESOURCES
* @brief Release any memory allocated to an Rx Queue
*/
#define IX_ATMDACC_RXQ_RELEASE_RESOURCES(queue) \
IX_ATMDACC_SWQ_RELEASE_RESOURCES(queue)

/**
* @def IX_ATMDACC_RXQ_SIZE
* @brief return the size of a queue
*/
#define IX_ATMDACC_RXQ_SIZE(queue) \
IX_ATMDACC_SWQ_SIZE(queue)

/**
* @def IX_ATMDACC_RXQ_ENTRY_IDXSET
* @brief Set an entry ditrectly at the specified index in the RX queue
*/
#define IX_ATMDACC_RXQ_ENTRY_IDXSET(queue,index,entry) \
IX_ATMDACC_SWQ_ENTRY_IDXSET(queue,index,entry)

/**
* @def IX_ATMDACC_RXQ_ENTRY_IDXGET
* @brief return an entry ditrectly from the specified index in the RX queue
*/
#define IX_ATMDACC_RXQ_ENTRY_IDXGET(queue,index) \
IX_ATMDACC_SWQ_ENTRY_IDXGET(queue,index)

/**
* @def IX_ATMDACC_RXQ_HEAD_INCR
* @brief Increment the Rx Queue Head
*/
#define IX_ATMDACC_RXQ_HEAD_INCR(queue) \
IX_ATMDACC_SWQ_HEAD_INCR(queue)

/**
* @def IX_ATMDACC_RXQ_HEAD_ADVANCE
* @brief Advance the Rx Queue Head
*/
#define IX_ATMDACC_RXQ_HEAD_ADVANCE(queue, numberOfEntries) \
IX_ATMDACC_SWQ_HEAD_ADVANCE((queue),(numberOfEntries))

/**
* @def IX_ATMDACC_RXQ_HEAD_ENTRY_GET
* @brief return the entry at the Head of teh RX Queue
*/
#define IX_ATMDACC_RXQ_HEAD_ENTRY_GET(queue) \
IX_ATMDACC_SWQ_HEAD_ENTRY_GET(queue)

/**
* @def IX_ATMDACC_RXQ_TAIL_INCR
* @brief Increment the Rx Queue Tail
*/
#define IX_ATMDACC_RXQ_TAIL_INCR(queue) \
IX_ATMDACC_SWQ_TAIL_INCR(queue)

/**
* @def IX_ATMDACC_RXQ_TAIL_DECR
* @brief Decrement the Rx Queue Tail
*/
#define IX_ATMDACC_RXQ_TAIL_DECR(queue) \
IX_ATMDACC_SWQ_TAIL_DECR(queue)

/**
* @def IX_ATMDACC_RXQ_TAIL_ENTRY_GET
* @brief return the entry at the Tail of teh RX Queue
*/
#define IX_ATMDACC_RXQ_TAIL_ENTRY_GET(queue) \
IX_ATMDACC_SWQ_TAIL_ENTRY_GET(queue)

/**
* @def IX_ATMDACC_RXQ_CONSISTENT
* @brief Check the software RX Queue consistency
*
* These conditions should always be true. If not, this means that
* the software RX queue is badly corrupted.
*/
#define IX_ATMDACC_RXQ_CONSISTENT(queue) \
        ((((queue)->head - (queue)->tail) >= 0) && \
         ((queue)->mid == 0) && \
         (((queue)->head - (queue)->tail) <= (queue)->size))

/**
* @def IX_ATMDACC_RXQ_ALL_RECYCLED
* @brief Check in a software RX Queue if descriptors are still used by the Npe
* or waiting in the RxDone queue
*/
#define IX_ATMDACC_RXQ_ALL_RECYCLED(queue) \
        ((queue)->head - (queue)->tail == (queue)->size)

/**
* @def IX_ATMDACC_RXQ_OVERLOADED
* @brief Check if a software RX Queue is overloaded
*/
#define IX_ATMDACC_RXQ_OVERLOADED(queue) \
        ((queue)->tail == (queue)->head)

/**
* @enum IxAtmdAccRxChannelStatus
* @brief Channel status
*/
typedef enum
{
    IX_ATMDACC_RX_CHANNEL_UP = 0,
    IX_ATMDACC_RX_CHANNEL_DOWN,
    IX_ATMDACC_RX_CHANNEL_DOWN_PENDING
} IxAtmdAccRxChannelStatus;

/**
*
* @brief IxAtmdAcc Rx channel information
*
* The information in this structure is used on a per-channel basis.
*
* This structure contains
* @li The channel characteristics (port/vpi/vci ..)
* @li The user callback functions and Id to be used when a mbuf is transmitted
* @li An internal software queue to store incoming Pdus
* @li The channel status
*
*/
typedef struct
{
    IxAtmConnId connId;                                /**< channel Connection Id */
    IxAtmLogicalPort port;                             /**< channel port */
    unsigned int vpi;                                  /**< channel vpi */
    unsigned int vci;                                  /**< channel vci */
    IxQMgrCallbackId callbackId;                       /**< userId passed in a callback */
    IxQMgrQId rxQueueId;                               /**< QMgr RX queue Id */
    IxAtmRxQueueId rxStreamId;                         /**< Atmd RX stream Id */
    IxQMgrQId rxFreeQueueId;                           /**< RX Free queue Id */
    unsigned int rxFreeQueueSize;                      /**< RX Free queue Size */
    IxAtmdAccRxVcRxCallback rxDoneCallback;            /**< user callback */
    IxAtmdAccRxVcFreeLowCallback rxFreeNotification;   /**< user callback */
    IxAtmdAccRxChannelStatus status;                   /**< channel status */
    IxNpeA_AalType npeAalType;                         /**< AAL Traffic type */
    unsigned int cellSize;                             /**< the cell data size 48/52 */
    IxAtmdAccPduStatus pduValidStatusCode;             /**< the value returned to the client if an rx
                                                            PDU is valid */
    IxAtmdAccPduStatus pduInvalidStatusCode;           /**< the value returned to the client if an rx
                                                            PDU is invalid */
    volatile BOOL replenishInProgress;                 /**< This flag is set when
                                                        *     mbufs replenish is in progress. When
                                                        *     set, it is not safe to
                                                        *     force the data recycling during a
                                                        *     @a ixAtmdAccRxDisconnect() step
                                                        */
    void * swQueueBuffer[IX_NPE_A_CHAIN_DESC_COUNT_MAX]; /**< data space for a sw queue. The maximum
                                                        * size of the queue is driven by the number of PDUs
                                                        * the NPE can chain */
    IX_ATMDACC_RX_QUEUE queue;                         /**< a s/w receive queue */
} IxAtmdAccRxVcDescriptor;

#ifndef NDEBUG
/**
*
* @brief AtmdAcc Rx transport stats
*
* The information in this structure is used to log possible
* runTime errors.
*
*/
typedef struct
{
    unsigned int invokeCount;       /**< rx invoke */
    unsigned int failedCount;       /**< rx failures */
    unsigned int spPduCount;        /**< rx pdu received */
    unsigned int ackCount;          /**< rx disable ack received */
    unsigned int maxRxPerDispatch;  /**< max number of entries read from the queue
                                     * during one invoke */
    unsigned int invalidDescCount;  /**< number of invalid descriptors received */
} IxAtmdAccRxDispatchStats;


/**
*
* @brief AtmdAcc Rx error stats
*
* The information in this structure is used to log possible
* runTime errors.
*
*/
typedef struct
{
    unsigned int descriptorOrderErrorCount;   /**< descriptors out of order from npe */
    unsigned int descriptorOrderDisconnectErrorCount;   /**< descriptors out of order
                                                * from npe during disconnect */
    unsigned int mbufMismatchErrorCount;      /**< mbufs from an other channel */
    unsigned int mbufChainErrorCount;         /**< mbuf chaining inconsistent */
} IxAtmdAccRxTransportErrorStats;

#endif

/**
* @brief pool of channel slots
*/
IX_ATMDACC_RX_TRANSPORT_EXTERN IxAtmdAccRxVcDescriptor ixAtmdAccRxVcDescriptor[IX_ATM_MAX_NUM_AAL_OAM_RX_VCS];

/**
* @brief user callback to be used when the threshold level of the RX queues is reached
*/
IX_ATMDACC_RX_TRANSPORT_EXTERN IxAtmdAccRxDispatcher ixAtmdAccRxUserCallback[IX_ATM_MAX_RX_STREAMS];

/**
* @brief qmgr queues used per rx stream
*/
IX_ATMDACC_RX_TRANSPORT_EXTERN IxQMgrQId ixAtmdAccRxQMgrQId[IX_ATM_MAX_RX_STREAMS];

/**
* @brief qmgr buffers used per rx stream
*
* There is an extra entry used as a 0 value to stop iterations through the buffer
*/
IX_ATMDACC_RX_TRANSPORT_EXTERN UINT32 ixAtmdAccRxQMgrBuffer[IX_ATM_MAX_RX_STREAMS][IX_ATMDACC_RX_QUEUE_SIZE + 1];

#ifndef NDEBUG
/**
* @brief stats per rx stream
*/
IX_ATMDACC_RX_TRANSPORT_EXTERN IxAtmdAccRxDispatchStats ixAtmdAccRxDispatchStats[IX_ATM_MAX_RX_STREAMS];


/**
* @brief transport error stats
*/
IX_ATMDACC_RX_TRANSPORT_EXTERN IxAtmdAccRxTransportErrorStats ixAtmdAccRxTransportErrorStats;
#endif

/**
* @brief Rx Free internal callback
*/
void ixAtmdAccRxFreeCallback (IxQMgrQId qid,
                              IxQMgrCallbackId cbId);

/**
* @brief Rx internal callback
*/
void ixAtmdAccRxCallback (IxQMgrQId qid,
                          IxQMgrCallbackId cbId);

/**
* @brief Rx internal channel shutdown processing
*/
void
ixAtmdAccRxShutdownAck (unsigned int npeVcId);

#endif /* IXATMDRXTRANSPORT_P_H */


