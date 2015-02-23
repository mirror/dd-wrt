/**
 * @file IxHssAccCodeletPkt.c
 *
 * @date 21 May 2002
 *
 * @brief This file contains the packetised implementation of the HSS
 * Access Codelet.
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
 * @sa IxHssAccCodelet.h
 * @sa IxHssAccCodeletPkt.h
 */

/*
 * Put the system defined include files required.
 */


/*
 * Put the user defined include files required.
 */

#include "IxHssAcc.h"
#include "IxOsal.h"

#include "IxHssAccCodelet_p.h"
#include "IxHssAccCodeletCom.h"
#include "IxHssAccCodeletPkt.h"
#include "IxHssAccCodeletMbuf.h"

/*
 * #defines and macros used in this file.
 */

#define MESSAGE_Q_SIZE (IX_HSSACC_CODELET_PKT_NUM_RX_BUFS)
#define IX_HSSACC_CODELET_PKT_56K_HDLC_CLIENT IX_HSSACC_HDLC_PORT_2

/*
 * Typedefs whose scope is limited to this file.
 */

typedef struct
{
    IX_OSAL_MBUF *buffer;
    unsigned numHssErrs;
    IxHssAccPktStatus pktStatus;
} CallbackParams;

typedef enum
{
    RxCallback,
    RxFreeLowCallback,
    TxDoneCallback
} MessageType;

typedef struct
{
    MessageType type;
    CallbackParams params;
} Message;

typedef struct
{
    IxHssAccHssPort hssPortId;
    IxHssAccHdlcPort hdlcPortId;
    IxOsalThread threadId;
    IxOsalSemaphore messageSem;
    Message messageQ[MESSAGE_Q_SIZE];
    unsigned qHead;
    unsigned qTail;
    /** packetised TX sample data - source of data for TX */
    UINT8 pktTxSampleData;
    /** packetised RX sample data - to verify against TX data */
    UINT8 pktRxSampleData;
    /** packetised mbuf pools */
    IxHssAccCodeletMbufPool *mbufPoolPtr;
    BOOL mbufPoolInitialised;
    unsigned staleRawBytesRecieved;
} ClientInfo;

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

static ClientInfo clientInfo
    [IX_HSSACC_HSS_PORT_MAX]
    [IX_HSSACC_HDLC_PORT_MAX];

static BOOL verify = TRUE;

/*
 * Extern function prototypes.
 */

/*
 * Static function prototypes.
 */
PRIVATE unsigned
mbufsCount (IX_OSAL_MBUF *buffer);

PRIVATE void
ixHssAccCodeletPktRxCallback (
    IX_OSAL_MBUF *buffer,
    unsigned numHssErrs,
    IxHssAccPktStatus pktStatus,
    IxHssAccPktUserId rxUserId);

PRIVATE void
ixHssAccCodeletPktRxCallbackProcess (
    IX_OSAL_MBUF *buffer,
    unsigned numHssErrs,
    IxHssAccPktStatus pktStatus,
    ClientInfo *pClientInfo);

PRIVATE void 
ixHssAccCodeletPktRxFreeLowCallback (IxHssAccPktUserId rxFreeLowUserId);

PRIVATE void
ixHssAccCodeletPktRxFreeLowCallbackProcess (ClientInfo *pClientInfo);

PRIVATE void
ixHssAccCodeletPktTxDoneCallback (
    IX_OSAL_MBUF *buffer,
    unsigned numHssErrs,
    IxHssAccPktStatus pktStatus,
    IxHssAccPktUserId txDoneUserId);

PRIVATE void
ixHssAccCodeletPktTxDoneCallbackProcess (
    IX_OSAL_MBUF *buffer,
    unsigned numHssErrs,
    IxHssAccPktStatus pktStatus,
    ClientInfo *pClientInfo);

PRIVATE IX_STATUS
ixHssAccCodeletPktThreadMain (
    void* arg,
    void** ptrRetObj);




/*
 * Function definition: mbufsCount
 */

PRIVATE
unsigned
mbufsCount (
    IX_OSAL_MBUF *buffer)
{
    unsigned count;

    for (count = 0; buffer != NULL;
	 buffer = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(buffer), count++)
        ;

    return count;
}

/**
 * @fn void ixHssAccCodeletPktRxCallback (
           IX_OSAL_MBUF *buffer,
           unsigned numHssErrs,
           IxHssAccPktStatus pktStatus,
           IxHssAccPktUserId rxUserId)
 *
 * @param IX_OSAL_MBUF *buffer (in) - the mbuf containing the received packet.
 * @param unsigned numHssErrs (in) - the total number of HSS port errors
 * since initial port configuration.
 * @param IxHssAccPktStatus pktStatus (in) - indicates the status of
 * the received packet.
 * @param IxHssAccPktUserId rxUserId (in) - the value supplied during
 * ixHssAccPktPortConnect(), a pointer to the client's ClientInfo
 * structure.
 *
 * This function is of type IxHssAccPktRxCallback, the prototype of the
 * clients function to accept notification of packetised rx.
 *
 * This function is registered through ixHssAccPktPortConnect().  Received
 * packets are passed back to the client in the form of mbufs via this
 * callback.  The mbuf passed back to the client could contain a chain of
 * mbufs, depending on the packet size received.
 * <P>
 * The callback updates RX statistics and returns the mbuf(s) to the RX
 * pool.  If the HSS is as loopback then any received data is verified.
 */

PRIVATE
void
ixHssAccCodeletPktRxCallback (
    IX_OSAL_MBUF *buffer,
    unsigned numHssErrs,
    IxHssAccPktStatus pktStatus,
    IxHssAccPktUserId rxUserId)
{
    ClientInfo *pClientInfo = (ClientInfo *)rxUserId;
    Message *pMessage;

    /* add message to the head of the message queue */
    pMessage = &pClientInfo->messageQ[pClientInfo->qHead++];
    pClientInfo->qHead %= NUMELEMS(pClientInfo->messageQ);

    /* fill in the message */
    pMessage->type = RxCallback;
    pMessage->params.buffer = buffer;
    pMessage->params.numHssErrs = numHssErrs;
    pMessage->params.pktStatus = pktStatus;

    /* wake up the message processing thread */
    (void) ixOsalSemaphorePost (&pClientInfo->messageSem);
}

/*
 * Function definition: ixHssAccCodeletPktRxCallbackProcess
 */

PRIVATE
void
ixHssAccCodeletPktRxCallbackProcess (
    IX_OSAL_MBUF *buffer,
    unsigned numHssErrs,
    IxHssAccPktStatus pktStatus,
    ClientInfo *pClientInfo)
{
    IX_STATUS status ; 
    IX_OSAL_MBUF *rxBuffer;
    IxHssAccHssPort hssPortId = pClientInfo->hssPortId;
    IxHssAccHdlcPort hdlcPortId = pClientInfo->hdlcPortId;
    unsigned wordIndex;
    unsigned byteIndex;
    UINT32 value;
    UINT8 expectedValue;

    /* for each buffer in the RX buffer chain */
    for (rxBuffer = buffer;
         (rxBuffer != NULL) && (pktStatus == IX_HSSACC_PKT_OK);
         rxBuffer = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(rxBuffer))
    {
        /* to allow for caching, we request a cache invalidate after rx */
        IX_OSAL_CACHE_INVALIDATE (IX_OSAL_MBUF_MDATA(rxBuffer),
            IX_OSAL_MBUF_MLEN(rxBuffer));

        /* if hardware is performing a loopback then verify the RX data */
        if (ixHssAccCodeletHssLoopbackGet () && verify)
        {
            /* verify buffer contents */
            for (wordIndex = 0; wordIndex < (UINT32)(IX_OSAL_MBUF_MLEN(rxBuffer) / 4);
                 wordIndex++)
            {
                value = ((UINT32 *)IX_OSAL_MBUF_MDATA(rxBuffer))[wordIndex];

                /* if this word is an idle pattern update idle stats */
                if ((value == IX_HSSACC_CODELET_PKT_IDLE_PATTERN) ||
                    (value == IX_HSSACC_CODELET_PKT_RAW_IDLE_PATTERN))
                {
                    stats[hssPortId].pkt[hdlcPortId].rxIdles += 4;
                    continue;
                }

                /* verify each byte of the word */
                for (byteIndex = 0; byteIndex < 4; byteIndex++)
                {
                    /* get/update the value we are expecting to receive */
                    expectedValue = pClientInfo->pktRxSampleData++;

                    if (((UINT8 *)&value)[byteIndex] != expectedValue)
                    {
                        if (pClientInfo->staleRawBytesRecieved <
                            IX_HSSACC_CODELET_PKT_RAW_STALE_BYTES_MAX)
                        {
                            /* In the first raw-mode packet recieved after the
                             *   HDLC port is enabled, it is normal to see a
                             *   few strange bytes at the start of the packet.
                             *   This is stale data from NPE buffers.  We can
                             *   safely ignore these bytes
                             */
                             pClientInfo->staleRawBytesRecieved++;
                             pClientInfo->pktRxSampleData--;
                        }
                        else
                        {
                            stats[hssPortId].pkt[hdlcPortId].rxVerifyFails++;
                        }
                    }
                }
            } /* for (wordIndex ... */
        } /* if (ixHssAccCodeletHssLoopbackGet ()) */

        /* let higher priority tasks run (i.e. channelised service) */
        ixOsalYield();
    } /* for (rxBuffer ... */

    stats[hssPortId].pkt[hdlcPortId].rxBufsInUse -= mbufsCount (buffer);

    /* if codelet is performing a loopback then re-transmit buffer */
    if (ixHssAccCodeletCodeletLoopbackGet () &&
        (pktStatus == IX_HSSACC_PKT_OK))
    {
        stats[hssPortId].pkt[hdlcPortId].txBufsInUse +=
            mbufsCount (buffer);

        /* transmit the received buffer */
        status = ixHssAccPktPortTx (hssPortId, hdlcPortId, buffer);

        /* if there was any problem then free buffer and update stats */
        if (status != IX_SUCCESS)
        {
            stats[hssPortId].pkt[hdlcPortId].txBufsInUse -=
                mbufsCount (buffer);
            ixHssAccCodeletMbufChainFree (buffer);

            stats[hssPortId].pkt[hdlcPortId].txFails++;
        }
    }
    else
    {
        /* free the buffer */
        ixHssAccCodeletMbufChainFree (buffer);
    }

    if (pktStatus == IX_HSSACC_PKT_OK)
    {
        /* update RX stats */
        stats[hssPortId].pkt[hdlcPortId].rxPackets++;
        stats[hssPortId].pkt[hdlcPortId].rxBytes += IX_OSAL_MBUF_PKT_LEN(buffer);
    }

    /* update error stats */
    ixHssAccCodeletNumHssErrorsUpdate (hssPortId, numHssErrs);
    ixHssAccCodeletPktErrorsUpdate (hssPortId, hdlcPortId, pktStatus);

    /* if we're not stopping or disconnecting replenish the rxfree q */
    if ((pktStatus != IX_HSSACC_STOP_SHUTDOWN_ERROR) &&
        (pktStatus != IX_HSSACC_DISCONNECT_IN_PROGRESS))
    {
        ixHssAccCodeletPktRxFreeLowCallbackProcess (pClientInfo);
    }
}

/**
 * @fn void ixHssAccCodeletPktRxFreeLowCallback (
           IxHssAccPktUserId rxFreeLowUserId)
 *
 * @param IxHssAccPktUserId rxFreeLowUserId (in) - the value supplied
 * during ixHssAccPktPortConnect(), a pointer to the client's ClientInfo
 * structure.
 *
 * This function is of type IxHssAccPktRxFreeLowCallback, the prototype of
 * the clients function to accept notification of requirement of more Rx
 * Free buffers.
 *
 * This function is registered through ixHssAccPktPortConnect().  The
 * callback is invoked when more mbufs are required to receive packets
 * into.
 * <P>
 * The callback will provide additional mbufs by calling
 * ixHssAccPktPortRxFreeReplenish().
 */

PRIVATE
void
ixHssAccCodeletPktRxFreeLowCallback (
    IxHssAccPktUserId rxFreeLowUserId)
{
    ClientInfo *pClientInfo = (ClientInfo *)rxFreeLowUserId;
    Message *pMessage;

    /* add message to the head of the message queue */
    pMessage = &pClientInfo->messageQ[pClientInfo->qHead++];
    pClientInfo->qHead %= NUMELEMS(pClientInfo->messageQ);

    /* fill in the message */
    pMessage->type = RxFreeLowCallback;

    /* wake up the message processing thread */
    (void) ixOsalSemaphorePost (&pClientInfo->messageSem);
}

/*
 * Function definition: ixHssAccCodeletPktRxFreeLowCallbackProcess
 */

PRIVATE
void
ixHssAccCodeletPktRxFreeLowCallbackProcess (
    ClientInfo *pClientInfo)
{
    IX_STATUS status = IX_SUCCESS;
    IX_OSAL_MBUF *rxBuffer;
    IxHssAccHssPort hssPortId = pClientInfo->hssPortId;
    IxHssAccHdlcPort hdlcPortId = pClientInfo->hdlcPortId;

    /* provide as many Rx buffers as we can, i.e. until we receive: */
    /*   a resource error - means IxHssAcc is out of resources */
    /*   a queue write overflow - means the RxFree queue is full */
    do
    {
        /* get an RX buffer */
        rxBuffer = ixHssAccCodeletMbufGet (
            pClientInfo->mbufPoolPtr);

        /* if we got a buffer ok */
        if (rxBuffer != NULL)
        {
            /* IMPORTANT: IxHssAcc component needs to know the capacity */
            /* of the mbuf - we tell it via the mbuf's m_len field */
            IX_OSAL_MBUF_MLEN(rxBuffer) = IX_HSSACC_CODELET_PKT_BUFSIZE;

            /* give the Rx buffer to the HssAcc component */
            status = ixHssAccPktPortRxFreeReplenish (
                hssPortId, hdlcPortId, rxBuffer);

            /* if the HssAcc component couldn't accept the Rx buffer */
            if (status != IX_SUCCESS)
            {
                /* free the buffer */
                (void) ixHssAccCodeletMbufChainFree (rxBuffer);

                /* if the error was other than expected */
                if ((status != IX_HSSACC_RESOURCE_ERR) &&
                    (status != IX_HSSACC_Q_WRITE_OVERFLOW))
                {
                    stats[hssPortId].pkt[hdlcPortId].replenishFails++;
                }
            }
            else
            {
                stats[hssPortId].pkt[hdlcPortId].rxBufsInUse++;
            }
        }
        else /* no rx buffers available */
        {
            stats[hssPortId].pkt[hdlcPortId].rxNoBuffers++;
        }
    } while ((rxBuffer != NULL) && (status == IX_SUCCESS));
}

/**
 * @fn void ixHssAccCodeletPktTxDoneCallback (
           IX_OSAL_MBUF *buffer,
           unsigned numHssErrs,
           IxHssAccPktStatus pktStatus,
           IxHssAccPktUserId txDoneUserId)
 *
 * @param IX_OSAL_MBUF *buffer (in) - the mbuf containing the transmitted
 * packet.
 * @param unsigned numHssErrs (in) - the total number of HSS port errors
 * since initial port configuration.
 * @param IxHssAccPktStatus pktStatus (in) - indicates the status of
 * the transmitted packet.
 * @param IxHssAccPktUserId txDoneUserId (in) - the value supplied during
 * ixHssAccPktPortConnect(), a pointer to the client's ClientInfo
 * structure.
 *
 * This function is of type IxHssAccPktTxDoneCallback, the prototype of the
 * clients function to accept notification of completion with Tx buffers.
 *
 * This function is registered through ixHssAccPktPortConnect().  The
 * callback is invoked to return a transmitted packet back to the client.
 * <P>
 * The callback updates TX statistics and returns the mbuf(s) to the TX
 * pool.
 */

PRIVATE
void
ixHssAccCodeletPktTxDoneCallback (
    IX_OSAL_MBUF *buffer,
    unsigned numHssErrs,
    IxHssAccPktStatus pktStatus,
    IxHssAccPktUserId txDoneUserId)
{
    ClientInfo *pClientInfo = (ClientInfo *)txDoneUserId;
    Message *pMessage;

    /* add message to the head of the message queue */
    pMessage = &pClientInfo->messageQ[pClientInfo->qHead++];
    pClientInfo->qHead %= NUMELEMS(pClientInfo->messageQ);

    /* fill in the message */
    pMessage->type = TxDoneCallback;
    pMessage->params.buffer = buffer;
    pMessage->params.numHssErrs = numHssErrs;
    pMessage->params.pktStatus = pktStatus;

    /* wake up the message processing thread */
    (void) ixOsalSemaphorePost (&pClientInfo->messageSem);
}

/*
 * Function definition: ixHssAccCodeletPktTxDoneCallbackProcess
 */

PRIVATE
void
ixHssAccCodeletPktTxDoneCallbackProcess (
    IX_OSAL_MBUF *buffer,
    unsigned numHssErrs,
    IxHssAccPktStatus pktStatus,
    ClientInfo *pClientInfo)
{
    IxHssAccHssPort hssPortId = pClientInfo->hssPortId;
    IxHssAccHdlcPort hdlcPortId = pClientInfo->hdlcPortId;

    if (pktStatus == IX_HSSACC_PKT_OK)
    {
        /* update TX stats */
        stats[hssPortId].pkt[hdlcPortId].txPackets++;
        stats[hssPortId].pkt[hdlcPortId].txBytes += IX_OSAL_MBUF_PKT_LEN(buffer);
    }

    /* update error stats */
    ixHssAccCodeletNumHssErrorsUpdate (hssPortId, numHssErrs);
    ixHssAccCodeletPktErrorsUpdate (hssPortId, hdlcPortId, pktStatus);

    /* free the buffer */
    stats[hssPortId].pkt[hdlcPortId].txBufsInUse -= mbufsCount (buffer);
    ixHssAccCodeletMbufChainFree (buffer);
}

/*
 * Function definition: ixHssAccCodeletPktThreadMain
 */

PRIVATE
IX_STATUS
ixHssAccCodeletPktThreadMain (
    void* arg,
    void** ptrRetObj)
{
    ClientInfo *pClientInfo = (ClientInfo *)arg;
    Message *pMessage;

    while (1)
    {
        (void) ixOsalSemaphoreWait (
            &pClientInfo->messageSem, IX_OSAL_WAIT_FOREVER);

        pMessage = &pClientInfo->messageQ[pClientInfo->qTail++];
        pClientInfo->qTail %= NUMELEMS(pClientInfo->messageQ);

        switch (pMessage->type)
        {
        case RxCallback:
            ixHssAccCodeletPktRxCallbackProcess (
                pMessage->params.buffer,
                pMessage->params.numHssErrs,
                pMessage->params.pktStatus,
                pClientInfo);
            break;

        case RxFreeLowCallback:
            ixHssAccCodeletPktRxFreeLowCallbackProcess (
                pClientInfo);
            break;

        case TxDoneCallback:
            ixHssAccCodeletPktTxDoneCallbackProcess (
                pMessage->params.buffer,
                pMessage->params.numHssErrs,
                pMessage->params.pktStatus,
                pClientInfo);
            break;
        } /* switch (pMessage->type) */
    } /* while (1) */

    return IX_SUCCESS;
}

/*
 * Function definition: ixHssAccCodeletPacketisedServiceStart
 */

void
ixHssAccCodeletPacketisedServiceStart (
    IxHssAccHssPort hssPortId,
    IxHssAccHdlcPort hdlcPortId)
{
    IX_STATUS status;
    BOOL hdlcFraming;
    IxHssAccHdlcMode hdlcMode;
    BOOL hdlcBitInvert;
    unsigned blockSizeInWords;
    UINT32 rawIdleBlockPattern;
    IxHssAccPktHdlcFraming hdlcTxFraming;
    IxHssAccPktHdlcFraming hdlcRxFraming;
    unsigned frmFlagStart;
    ClientInfo *pClientInfo = &clientInfo[hssPortId][hdlcPortId];
    IxOsalThreadAttr ixHssAccCodeletPktThreadAttr;

    /* initialise client info structure for this client */
    pClientInfo->hssPortId = hssPortId;
    pClientInfo->hdlcPortId = hdlcPortId;

    /* initialise the tx/rx sample data values (to port number) */
    pClientInfo->pktTxSampleData = hdlcPortId + 1;
    pClientInfo->pktRxSampleData = hdlcPortId + 1;

    /****************/
    /* START THREAD */
    /****************/

    /* initialise message queue to empty */
    pClientInfo->qHead = 0;
    pClientInfo->qTail = 0;

    /* initialise the rx semaphore */
    (void) ixOsalSemaphoreInit (
        &pClientInfo->messageSem, IX_HSSACC_CODELET_SEM_UNAVAILABLE);

    /* create the thread for processing callbacks */
    /* when running both packetised and channelised services, the */
    /* channelised service needs to be serviced in a higher priority */
    /* thread (high) than the packetised service (low) */
    ixHssAccCodeletPktThreadAttr.name = "HSS Codelet Pkt CB";
    ixHssAccCodeletPktThreadAttr.stackSize = 10240;
    ixHssAccCodeletPktThreadAttr.priority = IX_HSSACC_CODELET_THREAD_PRI_MEDIUM;
    (void) ixOsalThreadCreate (
        &pClientInfo->threadId,                            /* threadId */
	&ixHssAccCodeletPktThreadAttr,                     /* threadAttr */
	(IxOsalVoidFnVoidPtr)ixHssAccCodeletPktThreadMain, /* entryPoint */
	pClientInfo);                                      /* arg */

    /* start the thread for processing callbacks */
    (void) ixOsalThreadStart (
	&pClientInfo->threadId);    /* threadId */		        

    /********************/
    /* ALLOCATE BUFFERS */
    /********************/

    /* initialise the pool */
    if (!pClientInfo->mbufPoolInitialised)
    {
        ixHssAccCodeletMbufPoolInit (
            &pClientInfo->mbufPoolPtr,
            IX_HSSACC_CODELET_PKT_NUM_BUFS,
            IX_HSSACC_CODELET_PKT_BUFSIZE);

        pClientInfo->mbufPoolInitialised = TRUE;
    }

    /**********************/
    /* CONNECT TO SERVICE */
    /**********************/

    /* HDLC framing = TRUE/FALSE (clients 0 and 2 will use HDLC mode, */
    /* clients 1 and 3 will use RAW mode) */
    hdlcFraming = (hdlcPortId % 2 == 0 ? TRUE : FALSE);


    if (hdlcPortId == IX_HSSACC_CODELET_PKT_56K_HDLC_CLIENT)
    {
	/* Client 2 is configured to run in 56Kbps packetised HDLC */
	/* mode here. CAS bit is set to be in the LSB position with */
	/* bit polarity '1' */
	hdlcMode.hdlc56kMode = TRUE;
	hdlcMode.hdlc56kEndian = IX_HSSACC_56KE_BIT_0_UNUSED;
	hdlcMode.hdlc56kUnusedBitPolarity0 = FALSE;

	/* Bit inversion is enabled for client 2 */
	hdlcBitInvert = TRUE;
    }
    else
    {
	/* Client 0, 1 and 3 are running in 64Kbps packetised mode. */
	/* hdlcMode.hdlc56kEndian and hdlcMode.hdlc56kUnusedBitPolarity0 */ 
	/* values are ignored by HSS access component in 64Kbps mode */
	hdlcMode.hdlc56kMode = FALSE;
	hdlcMode.hdlc56kEndian = IX_HSSACC_56KE_BIT_7_UNUSED;
	hdlcMode.hdlc56kUnusedBitPolarity0 = TRUE;

	/* No bit inversion for client 0, 1 and 3 */
	hdlcBitInvert = FALSE;
    }

    /* Raw mode block size = 4096 words (i.e. 16K) */
    blockSizeInWords = (IX_HSSACC_CODELET_PKT_TX_PKTSIZE / 4);

    /* Raw mode idle pattern = 0x5F5F5F5F */
    rawIdleBlockPattern = IX_HSSACC_CODELET_PKT_RAW_IDLE_PATTERN;

    /* HDLC idle transmission type = flags */
    hdlcTxFraming.hdlcIdleType = IX_HSSACC_HDLC_IDLE_FLAGS;
    hdlcRxFraming.hdlcIdleType = IX_HSSACC_HDLC_IDLE_FLAGS;

    /* HDLC data endianness = lsb endian */
    hdlcTxFraming.dataEndian = IX_HSSACC_LSB_ENDIAN;
    hdlcRxFraming.dataEndian = IX_HSSACC_LSB_ENDIAN;

    /* CRC type (CRC-16 or CRC-32) = CRC-32 */
    hdlcTxFraming.crcType = IX_HSSACC_PKT_32_BIT_CRC;
    hdlcRxFraming.crcType = IX_HSSACC_PKT_32_BIT_CRC;

    /* Number of flags inserted at start of frame = 1 */
    frmFlagStart = 1;

    /* connect this client to the Packetised Service */
    status = ixHssAccPktPortConnect (
        hssPortId,                           /* hssPortId */
        hdlcPortId,                          /* hdlcPortId */
        hdlcFraming,                         /* hdlcFraming */
        hdlcMode,			     /* hdlcMode */
        hdlcBitInvert,			     /* hdlcBitInvert */
        blockSizeInWords,                    /* blockSizeInWords */
        rawIdleBlockPattern,                 /* rawIdleBlockPattern */
        hdlcTxFraming,                       /* hdlcTxFraming */
        hdlcRxFraming,                       /* hdlcRxFraming */
        frmFlagStart,                        /* frmFlagStart */
        ixHssAccCodeletPktRxCallback,        /* rxCallback */
        (IxHssAccPktUserId) pClientInfo,     /* rxUserId */
        ixHssAccCodeletPktRxFreeLowCallback, /* rxFreeLowCallback */
        (IxHssAccPktUserId) pClientInfo,     /* rxFreeLowUserId */
        ixHssAccCodeletPktTxDoneCallback,    /* txDoneCallback */
        (IxHssAccPktUserId) pClientInfo);    /* txDoneUserId */

    /* if there was any problem then update stats */
    if (status != IX_SUCCESS)
    {
        stats[hssPortId].pkt[hdlcPortId].connectFails++;
    }

    /*********************/
    /* SUPPLY RX BUFFERS */
    /*********************/

    /* invoke the free request callback */
    ixHssAccCodeletPktRxFreeLowCallbackProcess (pClientInfo);

    /*****************/
    /* START SERVICE */
    /*****************/

    /* this counter should be reset at port enable */
    pClientInfo->staleRawBytesRecieved = 0;

    /* start the Packetised Service for this client */
    status = ixHssAccPktPortEnable (hssPortId, hdlcPortId);

    /* if there was any problem then update stats */
    if (status != IX_SUCCESS)
    {
        stats[hssPortId].pkt[hdlcPortId].portEnableFails++;
    }
}

/*
 * Function definition: ixHssAccCodeletPacketisedServiceRun
 */

void
ixHssAccCodeletPacketisedServiceRun (
    IxHssAccHssPort hssPortId,
    IxHssAccHdlcPort hdlcPortId)
{
    IX_STATUS status;
    IX_OSAL_MBUF *txBuffer = NULL;
    IX_OSAL_MBUF *txBufferChain = NULL;
    IX_OSAL_MBUF *lastBuffer = NULL;
    UINT32 txBytes;
    UINT32 bytes;
    unsigned byteIndex;
    ClientInfo *pClientInfo = &clientInfo[hssPortId][hdlcPortId];

    /*********************/
    /* TRANSMIT A PACKET */
    /*********************/

    /* if the codelet is acting as data source/sink */
    if (!ixHssAccCodeletCodeletLoopbackGet ())
    {
        /* initialise the number of bytes we want to transmit */
        txBytes = IX_HSSACC_CODELET_PKT_TX_PKTSIZE;

        do
        {
            /* bytes to transmit in this buffer */
            bytes = (txBytes > IX_HSSACC_CODELET_PKT_BUFSIZE ?
                     IX_HSSACC_CODELET_PKT_BUFSIZE : txBytes);

            /* get a TX buffer */
            txBuffer = ixHssAccCodeletMbufGet (
                pClientInfo->mbufPoolPtr);

            /* if we got a buffer ok */
            if (txBuffer != NULL)
            {
                stats[hssPortId].pkt[hdlcPortId].txBufsInUse++;

                /* set the data for the current buffer */
                IX_OSAL_MBUF_MLEN(txBuffer) = bytes;

                /* if we're verifying rx data then fill in tx buffer */
                if (verify)
                {
                    for (byteIndex = 0; byteIndex < (UINT32) IX_OSAL_MBUF_MLEN(txBuffer);
                         byteIndex++)
                    {
                        /* get/update the value to transmit */
                        ((UINT8 *)IX_OSAL_MBUF_MDATA(txBuffer))[byteIndex] =
                            pClientInfo->pktTxSampleData++;
                    }
                }

                /* to allow for caching, request a cache flush before tx */
                IX_OSAL_CACHE_FLUSH (
                    IX_OSAL_MBUF_MDATA(txBuffer), IX_OSAL_MBUF_MLEN(txBuffer));
 
                /* chain the buffer to the end of the current chain */
                if (txBufferChain == NULL)
                {
                    txBufferChain = txBuffer;

                    /* set packet header for buffer */
                    IX_OSAL_MBUF_PKT_LEN(txBufferChain) = 0;
                }
                else
                {
                    IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(lastBuffer) = txBuffer;
                }

                IX_OSAL_MBUF_PKT_LEN(txBufferChain) += IX_OSAL_MBUF_MLEN(txBuffer);
                IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(txBuffer) = NULL;
                lastBuffer = txBuffer;

                /* decrement bytes left to transmit */
                txBytes -= bytes;
            }
            else /* no tx buffers available */
            {
                stats[hssPortId].pkt[hdlcPortId].txNoBuffers++;
            }

            /* let higher priority tasks run (i.e. channelised service) */
            ixOsalYield();
        } while ((txBytes > 0) && (txBuffer != NULL));
    } /* if (!ixHssAccCodeletCodeletLoopbackGet ()) */

    /* if we have a buffer to transmit */
    if (txBufferChain != NULL)
    {
        /* transmit the packet */
        status = ixHssAccPktPortTx (hssPortId, hdlcPortId, txBufferChain);

        /* if there was any problem then return buffer and update stats */
        if (status != IX_SUCCESS)
        {
            stats[hssPortId].pkt[hdlcPortId].txBufsInUse -=
                mbufsCount (txBufferChain);
            ixHssAccCodeletMbufChainFree (txBufferChain);

            stats[hssPortId].pkt[hdlcPortId].txFails++;
        }
    }

    /********************/
    /* RECEIVE A PACKET */
    /********************/

    /* nothing to do here - RX happens via callback routines */
}

/*
 * Function definition: ixHssAccCodeletPacketisedServiceStop
 */

void
ixHssAccCodeletPacketisedServiceStop (
    IxHssAccHssPort hssPortId,
    IxHssAccHdlcPort hdlcPortId)
{
    IX_STATUS status;
    unsigned elapsedMilliseconds;
    ClientInfo *pClientInfo = &clientInfo[hssPortId][hdlcPortId];

    /****************/
    /* STOP SERVICE */
    /****************/

    /* stop the Packetised Service for the client */
    status = ixHssAccPktPortDisable (hssPortId, hdlcPortId);

    /* if there was any problem then update stats */
    if (status != IX_SUCCESS)
    {
        stats[hssPortId].pkt[hdlcPortId].portDisableFails++;
    }

    /**********************/
    /* DISCONNECT SERVICE */
    /**********************/

    /* disconnect the Packetised Service for the client */
    status = ixHssAccPktPortDisconnect (hssPortId, hdlcPortId);

    /* if the service is in the process of disconnnecting */
    if (status == IX_HSSACC_PKT_DISCONNECTING)
    {
        elapsedMilliseconds = 0;

        /* wait for a maximum of 250ms */
        while (elapsedMilliseconds < 250)
        {
            /* if the disconnect is complete then exit with success */
            if (ixHssAccPktPortIsDisconnectComplete (
                hssPortId, hdlcPortId))
            {
                status = IX_SUCCESS;
                break;
            }

            /* wait for 50ms */
            ixOsalSleep (50);
            elapsedMilliseconds += 50;
        }
    }

    /* if there was any problem then update stats */
    if (status != IX_SUCCESS)
    {
        stats[hssPortId].pkt[hdlcPortId].disconnectFails++;
    }

    /***************/
    /* STOP THREAD */
    /***************/

    /* wait for thread to finish processing messages */
    while (pClientInfo->qTail != pClientInfo->qHead)
    {
        /* wait for 50ms */
        ixOsalSleep (50);
    }
}

/*
 * Function definition: ixHssAccCodeletPacketisedVerifySet
 */

void
ixHssAccCodeletPacketisedVerifySet (
    BOOL verifyOn)
{
    verify = verifyOn;
}



