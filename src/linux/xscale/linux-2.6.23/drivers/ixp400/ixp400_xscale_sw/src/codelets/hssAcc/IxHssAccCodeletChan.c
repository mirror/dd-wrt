/**
 * @file IxHssAccCodeletChan.c
 *
 * @date 21 May 2002
 *
 * @brief This file contains the channelised implementation of the HSS
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
 * @sa IxHssAccCodeletChan.h
 */

/*
 * Put the system defined include files required.
 */

/*
 * Put the user defined include files required.
 */

#include "IxOsal.h"
#include "IxHssAcc.h"

#include "IxHssAccCodelet.h"
#include "IxHssAccCodeletChan.h"
#include "IxHssAccCodeletCom.h"
#include "IxHssAccCodeletConfig.h"

#if defined(__wince) && defined(IX_USE_SERCONSOLE)
#include "IxSerConsole.h"
#define printf ixSerPrintf
#endif

/*
 * #defines and macros used in this file.
 */

#define MESSAGE_Q_SIZE \
    (IX_HSSACC_CODELET_CHAN_RX_BUFSIZE_PERCHAN / \
     IX_HSSACC_CODELET_CHAN_BYTES_PER_TS_TRIG)

#define MEMSET_AND_FLUSH(s, c, n) \
do { \
    ixOsalMemSet (s, c, n); \
    IX_OSAL_CACHE_FLUSH (s, n); \
} while (0)

/*
 * Typedefs whose scope is limited to this file.
 */

typedef struct
{
    unsigned rxOffset;
    unsigned txOffset;
    unsigned numHssErrs;
} CallbackParams;

typedef enum
{
    RxCallback
} MessageType;

typedef struct
{
    MessageType type;
    CallbackParams params;
} Message;

typedef struct
{
    /** are we receiving non-idle data? */
    BOOL receivingNonIdleData;
    /** channelised TX sample data - source of data for TX */
    UINT8 txSampleData;
    /** channelised RX sample data - to verify against TX data */
    UINT8 rxSampleData;
} ChannelInfo;

/** channelised RX buffers */
typedef UINT8 (*RxBuffers)
    [IX_HSSACC_CODELET_CHAN_NUM_CHANS]
    [IX_HSSACC_CODELET_CHAN_RX_BUFSIZE_PERCHAN];

/** channelised TX buffers */
typedef UINT8 (*TxBuffers)
    [IX_HSSACC_CODELET_CHAN_NUM_CHANS]
    [IX_HSSACC_CODELET_CHAN_TX_LATENCY_FACTOR]
    [IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE];

/** channelised TX pointer lists */
typedef UINT8 *(*TxPointers)
    [IX_HSSACC_CODELET_CHAN_TX_LATENCY_FACTOR]
    [IX_HSSACC_CODELET_CHAN_NUM_CHANS];

typedef struct
{
    IxHssAccHssPort hssPortId;
    IxOsalThread threadId;
    IxOsalSemaphore messageSem;
    Message messageQ[MESSAGE_Q_SIZE];
    unsigned qHead;
    unsigned qTail;
    /** the last offset we TX'ed from */
    int lastTxOffset;
    /** the last offset we RX'ed to */
    int lastRxOffset;
    /** the next offset we will RX to */
    int nextRxOffset;
    RxBuffers rxBuffers;
    TxBuffers txBuffers;
    TxPointers txPointers;
    /** set if we have a full set of samples Rx'ed */
    BOOL readyToLoopback;
    /** for recording channelised rx callback parameters */
    ChannelInfo channelInfo[IX_HSSACC_CODELET_CHAN_NUM_CHANS];
} ClientInfo;

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

static ClientInfo clientInfo
    [IX_HSSACC_HSS_PORT_MAX];

static BOOL verify = TRUE;

/*
 * Extern function prototypes.
 */

/*
 * Static function prototypes.
 */
PRIVATE void
ixHssAccCodeletChannelisedDataSampleCreate (
    IxHssAccHssPort hssPortId,
    unsigned channelIndex,
    UINT8 *sample);

PRIVATE void
ixHssAccCodeletChannelisedDataSampleTransmit (
    IxHssAccHssPort hssPortId,
    unsigned channelIndex,
    UINT8 *sample);

PRIVATE void
ixHssAccCodeletChannelisedDataTransmit (
    IxHssAccHssPort hssPortId,
    unsigned txOffset);

PRIVATE void
ixHssAccCodeletChannelisedDataSampleVerify (
    IxHssAccHssPort hssPortId,
    unsigned channelIndex,
    UINT8 *sample);

PRIVATE void
ixHssAccCodeletChannelisedDataSampleReceive (
    IxHssAccHssPort hssPortId,
    unsigned channelIndex,
    UINT8 *sample);

PRIVATE void
ixHssAccCodeletChannelisedDataReceive (
    IxHssAccHssPort hssPortId,
    unsigned rxOffset);

PRIVATE void
ixHssAccCodeletChanRxCallback (
    IxHssAccHssPort hssPortId,
    unsigned rxOffset,
    unsigned txOffset,
    unsigned numHssErrs);

PRIVATE void
ixHssAccCodeletChanRxCallbackProcess (
    IxHssAccHssPort hssPortId,
    unsigned rxOffset,
    unsigned txOffset,
    unsigned numHssErrs);

PRIVATE IX_STATUS
ixHssAccCodeletChanThreadMain (
    void* arg,
    void** ptrRetObj);


/**
 * @fn void ixHssAccCodeletChannelisedDataSampleCreate (
           IxHssAccHssPort hssPortId,
           unsigned channelIndex,
           UINT8 *sample)
 *
 * @param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * @param unsigned channelIndex (in) - the channel (0-31) to create the
 * data sample for.
 * @param UINT8 *sample (out) - a pointer to a data sample of size
 * CHAN_BYTES_PER_SAMPLE.
 *
 * This function creates a data sample for the specified port/channel.  The
 * sample is filled with an incrementing byte value.  The byte value begins
 * as the channel number and increases with each invocation:
 *
 * <TABLE>
 * <TR><TD>         <TD>1st Value<TD>2nd Value<TD>3rd Value<TD>...</TR>
 * <TR><TD>Channel 0<TD>     0x01<TD>     0x02<TD>     0x03<TD>...</TR>
 * <TR><TD>Channel 1<TD>     0x02<TD>     0x03<TD>     0x04<TD>...</TR>
 * <TR><TD>Channel 2<TD>     0x03<TD>     0x04<TD>     0x05<TD>...</TR>
 * <TR><TD>      ...<TD>      ...<TD>      ...<TD>      ...<TD>...</TR>
 * </TABLE>
 */

PRIVATE
void
ixHssAccCodeletChannelisedDataSampleCreate (
    IxHssAccHssPort hssPortId,
    unsigned channelIndex,
    UINT8 *sample)
{
    unsigned byteIndex;
    ClientInfo *pClientInfo = &clientInfo[hssPortId];
    ChannelInfo *pChannelInfo = &pClientInfo->channelInfo[channelIndex];

    /* if the codelet is acting as data source/sink */
    if (!ixHssAccCodeletCodeletLoopbackGet ())
    {
        /* for each byte in the data sample */
        for (byteIndex = 0;
             byteIndex < IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE;
             byteIndex++)
        {
            /* get/update the value to transmit */
            sample[byteIndex] = pChannelInfo->txSampleData++;
        }
    }
    else /* codelet is performing loopback, but nothing to loopback */
    {
        /* transmit an idle pattern */
        /* to allow for caching, we request a cache flush after memset */
        MEMSET_AND_FLUSH (
            sample, IX_HSSACC_CODELET_CHAN_IDLE_PATTERN,
            IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE);
    }
}

/**
 * @fn void ixHssAccCodeletChannelisedDataSampleTransmit (
           IxHssAccHssPort hssPortId,
           unsigned channelIndex,
           UINT8 *sample)
 *
 * @param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * @param unsigned channelIndex (in) - the channel (0-31) to transmit the
 * data sample on.
 * @param UINT8 *sample (in) - a pointer to a data sample of size
 * CHAN_BYTES_PER_SAMPLE.
 *
 * This function transmits a data sample on the specified port/channel.  It
 * maintains the current TX offset for each channel and stores the sample
 * pointer in the appropriate pointer list.  This function also updates TX
 * statistics.
 */

PRIVATE
void
ixHssAccCodeletChannelisedDataSampleTransmit (
    IxHssAccHssPort hssPortId,
    unsigned channelIndex,
    UINT8 *sample)
{
    ClientInfo *pClientInfo = &clientInfo[hssPortId];
    UINT8 *temp = NULL;

    /* to allow for caching, we request a cache flush before tx */
    IX_OSAL_CACHE_FLUSH (
        sample, IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE);

    /* update the pointer in the pointer list to point to the sample */
    /* endianess conversion on txPointers */
    temp = (UINT8 *) IX_OSAL_MMU_VIRT_TO_PHYS((UINT32)sample);
    (*pClientInfo->txPointers)[pClientInfo->lastTxOffset][channelIndex] =
        (UINT8 *) IX_OSAL_SWAP_BE_SHARED_LONG((UINT32) temp);

    IX_OSAL_CACHE_FLUSH (
	&(*pClientInfo->txPointers)[pClientInfo->lastTxOffset][channelIndex],
	sizeof((*pClientInfo->txPointers)[pClientInfo->lastTxOffset][channelIndex]));

    /* update TX stats */
    stats[hssPortId].chan.txSamples++;
    stats[hssPortId].chan.txBytes +=
        IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE;
}

/**
 * @fn void ixHssAccCodeletChannelisedDataTransmit (
           IxHssAccHssPort hssPortId,
           unsigned txOffset)
 *
 * @param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * @param unsigned txOffset (in) - an offset indicating from where within
 * the txPtrList the NPE is currently transmitting from or will transmit
 * from next.
 *
 * This function examines the txOffset parameter to determine if data needs
 * to be transmitted to the NPE or not.  If data needs to be transmitted
 * then data samples are created if necessary and transmitted for each
 * channel.
 */

PRIVATE
void
ixHssAccCodeletChannelisedDataTransmit (
    IxHssAccHssPort hssPortId,
    unsigned txOffset)
{
    unsigned channelIndex;
    UINT8 *txSample;
    UINT8 *rxSample;
    ClientInfo *pClientInfo = &clientInfo[hssPortId];

    /* the NPE tells us where it is currently transmitting from or will */
    /* transmit from next.  We want to update the txPtrList once we know */
    /* the NPE has completed transmitted it.  We can only be sure that */
    /* the NPE has completed transmitted the txPtrList corresponding to */
    /* (txOffset - 2), as the NPE may still be transmitting from */
    /* (txOffset - 1). */

    /* if we last transmitted to 2 slots prior to txOffset then we do */
    /* nothing, otherwise we transmit data.  In this way we will keep */
    /* updating the tx data as soon as we know the NPE has finished */
    /* transmitting it. */
    if ((pClientInfo->lastTxOffset + 2) %
        (UINT32) IX_HSSACC_CODELET_CHAN_TX_LATENCY_FACTOR == txOffset)
    {
        return;
    }

    /* increment our TX offset to remember where we last transmitted to */
    pClientInfo->lastTxOffset++;
    pClientInfo->lastTxOffset %= IX_HSSACC_CODELET_CHAN_TX_LATENCY_FACTOR;

    /* for each channel */
    for (channelIndex = 0; 
         channelIndex < IX_HSSACC_CODELET_CHAN_NUM_CHANS;
         channelIndex++)
    {
        /* get the pointer to the data sample for this offset/channel */
        txSample =
            (*pClientInfo->txBuffers)
                  [channelIndex][pClientInfo->lastTxOffset];

        /* codelet will loop back last data samples received */
        if (ixHssAccCodeletCodeletLoopbackGet () &&
            pClientInfo->readyToLoopback)
        {
            /* get pointer to the data sample for this offset/channel */
            rxSample =
                &(*pClientInfo->rxBuffers)
                    [channelIndex][pClientInfo->lastRxOffset];

            /* loopback rx data to tx data */

            /* note: important to copy data here, if we add rx sample */
            /* pointer to the tx pointer list then we are relying on the */
            /* rx sample not being overwritten before sample is tx'ed */
            /* - not safe when rx and tx latencies differ */
            ixOsalMemCopy (
                txSample, rxSample,
                IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE);
        }
        else  /* we create new data to transmit */
        {
            /* fill in the data sample */
            ixHssAccCodeletChannelisedDataSampleCreate (
                hssPortId, channelIndex, txSample);
        }

        /* transmit the data sample */
        ixHssAccCodeletChannelisedDataSampleTransmit (
            hssPortId, channelIndex, txSample);
    } /* for (channelIndex ... */
}

/**
 * @fn void ixHssAccCodeletChannelisedDataSampleVerify (
           IxHssAccHssPort hssPortId,
           unsigned channelIndex,
           UINT8 *sample)
 *
 * @param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * @param unsigned channelIndex (in) - the channel (0-31) to verify the
 * data sample for.
 * @param UINT8 *sample (in) - a pointer to a data sample of size
 * CHAN_BYTES_PER_SAMPLE.
 *
 * This function verifies the contents of a data sample for the specified
 * port/channel.  On transmit the sample is filled with an incrementing
 * byte value.  The byte value begins as the channel number and increases
 * with each invocation:
 * 
 * <TABLE>
 * <TR><TD>         <TD>1st Value<TD>2nd Value<TD>3rd Value<TD>...</TR>
 * <TR><TD>Channel 0<TD>     0x01<TD>     0x02<TD>     0x03<TD>...</TR>
 * <TR><TD>Channel 1<TD>     0x02<TD>     0x03<TD>     0x04<TD>...</TR>
 * <TR><TD>Channel 2<TD>     0x03<TD>     0x04<TD>     0x05<TD>...</TR>
 * <TR><TD>      ...<TD>      ...<TD>      ...<TD>      ...<TD>...</TR>
 * </TABLE>
 */

PRIVATE
void
ixHssAccCodeletChannelisedDataSampleVerify (
    IxHssAccHssPort hssPortId,
    unsigned channelIndex,
    UINT8 *sample)
{
    unsigned byteIndex;
    UINT8 expectedValue;
    ClientInfo *pClientInfo = &clientInfo[hssPortId];
    ChannelInfo *pChannelInfo = &pClientInfo->channelInfo[channelIndex];
    IxHssAccTdmSlotUsage tsUsage =
        ixHssAccCodeletChannelisedTimeslotGet (channelIndex);

    /* if the timeslot is assigned to 56K voice then bit 7 is unused */
    UINT32 mask = (tsUsage == IX_HSSACC_TDMMAP_VOICE56K ? 0x7F : 0xFF);

    /* for each byte in the data sample */
    for (byteIndex = 0;
         byteIndex < IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE;
         byteIndex++)
    {
        /* if we haven't started receiving non-idle data yet */
        if (!pChannelInfo->receivingNonIdleData)
        {
            /* check to see if we're receiving the idle pattern */
            if ((sample[byteIndex] & mask) ==
                (IX_HSSACC_CODELET_CHAN_IDLE_PATTERN & mask))
            {
                stats[hssPortId].chan.rxIdles++;
                continue;
            }

            /* not receiving idle pattern, i.e. receiving non-idle data */
            pChannelInfo->receivingNonIdleData = TRUE;
        }

        /* get/update the value we are expecting to receive */
        expectedValue = pChannelInfo->rxSampleData++;

        /* verify that the data is as expected */
        if ((sample[byteIndex] & mask) != (expectedValue & mask))
        {
            stats[hssPortId].chan.rxVerifyFails++;
        }
    } /* for (byteIndex ... */
}

/**
 * @fn void ixHssAccCodeletChannelisedDataSampleReceive (
           IxHssAccHssPort hssPortId,
           unsigned channelIndex,
           UINT8 *sample)
 *
 * @param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * @param unsigned channelIndex (in) - the channel (0-31) to receive the
 * data sample from.
 * @param UINT8 *sample (out) - a pointer to a data sample of size
 * CHAN_BYTES_PER_SAMPLE.
 *
 * This function handles a data sample received from the specified
 * port/channel.  If the hardware is performing a loopback then the data
 * sample is verified.  If the codelet is performing a loopback then the
 * data sample is re-transmitted on the same channel.  This function also
 * updates RX statistics.
 */

PRIVATE
void
ixHssAccCodeletChannelisedDataSampleReceive (
    IxHssAccHssPort hssPortId,
    unsigned channelIndex,
    UINT8 *sample)
{
    /* to allow for caching, we request a cache invalidate after rx */
    IX_OSAL_CACHE_INVALIDATE (
        sample, IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE);

    /* if the hardware is performing a loopback then verify the data */
    if (ixHssAccCodeletHssLoopbackGet () && verify)
    {
        ixHssAccCodeletChannelisedDataSampleVerify (
            hssPortId, channelIndex, sample);
    }

    /* update RX stats */
    stats[hssPortId].chan.rxSamples++;
    stats[hssPortId].chan.rxBytes +=
        IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE;
}

/**
 * @fn void ixHssAccCodeletChannelisedDataReceive (
        
           IxHssAccHssPort hssPortId,
           unsigned rxOffset)
 *
 * @param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * @param unsigned rxOffset (in) - an offset indicating where within the
 * receive buffers the NPE is currently receiving data into.
 * 
 * This function examines the rxOffset parameter to determine if a full
 * data sample has been received by the NPE or not.  If a full data sample
 * has been received by the NPE then it is handled.
 */

PRIVATE
void
ixHssAccCodeletChannelisedDataReceive (
    IxHssAccHssPort hssPortId,
    unsigned rxOffset)
{
    unsigned channelIndex;
    UINT8 *sample;
    ClientInfo *pClientInfo = &clientInfo[hssPortId];

    /* update rxOffset to allow calculation of the offset difference */
    if (rxOffset < (UINT32) pClientInfo->nextRxOffset)
    {
        rxOffset += IX_HSSACC_CODELET_CHAN_RX_BUFSIZE_PERCHAN;
    }

    /* if we haven't received a full data sample yet then return */
    if ((rxOffset - pClientInfo->nextRxOffset) <
        IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE)
    {
        return;
    }

    /* save nextRxOffset then update with next offset we will receive to */
    pClientInfo->lastRxOffset = pClientInfo->nextRxOffset;
    pClientInfo->nextRxOffset += IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE;
    pClientInfo->nextRxOffset %= IX_HSSACC_CODELET_CHAN_RX_BUFSIZE_PERCHAN;

    /* for each channel */
    for (channelIndex = 0;
         channelIndex < IX_HSSACC_CODELET_CHAN_NUM_CHANS;
         channelIndex++)
    {
        /* get the pointer to the data sample for this offset/channel */
        sample = &(*pClientInfo->rxBuffers)
            [channelIndex][pClientInfo->lastRxOffset];

        /* process the received data sample */
        ixHssAccCodeletChannelisedDataSampleReceive (
            hssPortId, channelIndex, sample);
    }

    /* we have received samples for every channel */
    pClientInfo->readyToLoopback = TRUE;
}

/**
 * @fn void ixHssAccCodeletChanRxCallback (
           IxHssAccHssPort hssPortId,
           unsigned rxOffset,
           unsigned txOffset,
           unsigned numHssErrs)
 *
 * @param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * @param unsigned rxOffset (in) - an offset indicating where within the
 * receive buffers the NPE is currently receiving data into.
 * @param unsigned txOffset (in) - an offset indicating from where within
 * the txPtrList the NPE is either currently transmitting from or will
 * transmit from next.
 * @param unsigned numHssErrs (in) - The total number of HSS port errors
 * since initial port configuration.
 *
 * This function is of type IxHssAccChanRxCallback, the prototype of the
 * clients function to accept notification of channelised rx.
 *
 * This function is registered through ixHssAccChanConnect().  It handles
 * the transmission and receipt of data samples if and when necessary.  It
 * also updates the HSS error count.
 */

PRIVATE
void
ixHssAccCodeletChanRxCallback (
    IxHssAccHssPort hssPortId,
    unsigned rxOffset,
    unsigned txOffset,
    unsigned numHssErrs)
{
    ClientInfo *pClientInfo = &clientInfo[hssPortId];
    Message *pMessage;

    /* add message to the head of the message queue */
    pMessage = &pClientInfo->messageQ[pClientInfo->qHead++];
    pClientInfo->qHead %= NUMELEMS(pClientInfo->messageQ);

    /* fill in the message */
    pMessage->type = RxCallback;
    pMessage->params.rxOffset = rxOffset;
    pMessage->params.txOffset = txOffset;
    pMessage->params.numHssErrs = numHssErrs;

    /* wake up the message processing thread */
    (void) ixOsalSemaphorePost (&pClientInfo->messageSem);
}

/*
 * Function definition: ixHssAccCodeletChanRxCallbackProcess
 */

PRIVATE
void
ixHssAccCodeletChanRxCallbackProcess (
    IxHssAccHssPort hssPortId,
    unsigned rxOffset,
    unsigned txOffset,
    unsigned numHssErrs)
{
    /* receive data */
    ixHssAccCodeletChannelisedDataReceive (hssPortId, rxOffset);

    /* transmit data */
     ixHssAccCodeletChannelisedDataTransmit (hssPortId, txOffset);

    /* update error stats */
    ixHssAccCodeletNumHssErrorsUpdate (hssPortId, numHssErrs);
}

/*
 * Function definition: ixHssAccCodeletChanThreadMain
 */

PRIVATE
IX_STATUS
ixHssAccCodeletChanThreadMain (
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
            ixHssAccCodeletChanRxCallbackProcess (
                pClientInfo->hssPortId,
                pMessage->params.rxOffset,
                pMessage->params.txOffset,
                pMessage->params.numHssErrs);
            break;
        }
    } /* while (1) */

    return IX_SUCCESS;
}

/*
 * Function definition: ixHssAccCodeletChannelisedServiceConfigure
 */

void
ixHssAccCodeletChannelisedServiceConfigure (
    IxHssAccHssPort hssPortId)
{
    IX_STATUS status;
    unsigned bytesPerTSTrigger;
    UINT8 *rxCircular;
    unsigned numRxBytesPerTS;
    UINT32 *txPtrList;
    unsigned numTxPtrLists;
    unsigned numTxBytesPerBlk;
    IxHssAccChanRxCallback rxCallback;
    unsigned channelIndex;
    unsigned i;
    ClientInfo *pClientInfo = &clientInfo[hssPortId];
    ChannelInfo *pChannelInfo;
    IxOsalThreadAttr ixHssAccCodeletChanThreadAttr;

    /* initialise client infor structure for this client */
    pClientInfo->hssPortId = hssPortId;
    pClientInfo->lastTxOffset = -1;
    pClientInfo->nextRxOffset = 0;        /* 1st rxOffset we receive to */
    pClientInfo->lastRxOffset = -1;       /* initially invalid */ 
    pClientInfo->readyToLoopback = FALSE; /* no samples Rx'ed yet */

    for (channelIndex = 0;
         channelIndex < IX_HSSACC_CODELET_CHAN_NUM_CHANS;
         channelIndex++)
    {
        pChannelInfo = &pClientInfo->channelInfo[channelIndex];

        /* not receiving non-idle data yet */
        pChannelInfo->receivingNonIdleData = FALSE;

        /* initialise the tx/rx sample data values (to channel number) */
        pChannelInfo->txSampleData = channelIndex + 1;
        pChannelInfo->rxSampleData = channelIndex + 1;
    }

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
    ixHssAccCodeletChanThreadAttr.name = "HSS Codelet Chan CB";
    ixHssAccCodeletChanThreadAttr.stackSize = 10240;
    ixHssAccCodeletChanThreadAttr.priority = IX_HSSACC_CODELET_THREAD_PRI_HIGH;
    (void) ixOsalThreadCreate (
	&pClientInfo->threadId,                             /* threadId */
	&ixHssAccCodeletChanThreadAttr,                     /* threadAttr */
	(IxOsalVoidFnVoidPtr)ixHssAccCodeletChanThreadMain, /* startRoutine */
	pClientInfo);                                       /* arg */ 

    /* start the thread for processing callbacks */
    (void) ixOsalThreadStart (
	&pClientInfo->threadId);    /* threadId */		

    /********************/
    /* ALLOCATE BUFFERS */
    /********************/

    /* allocate channelised RX buffers */
    if (pClientInfo->rxBuffers == 0)
    {
        pClientInfo->rxBuffers = IX_OSAL_CACHE_DMA_MALLOC(
            sizeof (*pClientInfo->rxBuffers));

        if (pClientInfo->rxBuffers == 0)
        {
	    printf("Failed to allocated Rx buffers\n");
            return;
        }
    }

    /* to allow for caching, we request a cache flush after memset */
    MEMSET_AND_FLUSH (pClientInfo->rxBuffers, 0x00,
                      sizeof (*pClientInfo->rxBuffers));

    /* allocate channelised TX buffers */
    if (pClientInfo->txBuffers == 0)
    {
        pClientInfo->txBuffers = IX_OSAL_CACHE_DMA_MALLOC(
            sizeof (*pClientInfo->txBuffers));

        if (pClientInfo->txBuffers == 0)
        {
	    printf("Failed to allocated Tx buffers\n");
	    return;
        }
    }

    /* to allow for caching, we request a cache flush after memset */
    MEMSET_AND_FLUSH (pClientInfo->txBuffers, 0x00,
                      sizeof (*pClientInfo->txBuffers));

    /* allocate channelised TX pointer lists */
    if (pClientInfo->txPointers == 0)
    {
        pClientInfo->txPointers = IX_OSAL_CACHE_DMA_MALLOC(
            sizeof (*pClientInfo->txPointers));

        if (pClientInfo->txPointers == 0)
        {
	    printf("Failed to allocated Tx pointers\n");
            return;
        }
    }

    /* to allow for caching, we request a cache flush after memset */
    MEMSET_AND_FLUSH (pClientInfo->txPointers, 0x00,
                      sizeof (*pClientInfo->txPointers));

    /**********************/
    /* CONNECT TO SERVICE */
    /**********************/

    /* Bytes per timeslot trigger = 8 */
    bytesPerTSTrigger = IX_HSSACC_CODELET_CHAN_BYTES_PER_TS_TRIG;

    /* RX circular buffer = RX buffer */
    rxCircular = (UINT8 *)(*pClientInfo->rxBuffers);

    /* Number of RX bytes per timeslot = 176 (RX buf size per channel) */
    numRxBytesPerTS = IX_HSSACC_CODELET_CHAN_RX_BUFSIZE_PERCHAN;

    /* TX pointer list = TX pointer array */
    txPtrList = (UINT32 *)(*pClientInfo->txPointers);

    /* Number of TX pointer lists = 8 (latency factor) */
    numTxPtrLists = IX_HSSACC_CODELET_CHAN_TX_LATENCY_FACTOR;

    /* Number of TX bytes per block = 44 (bytes per sample) */
    numTxBytesPerBlk = IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE;

    /* Receive callback */
    rxCallback = ixHssAccCodeletChanRxCallback;

    /* connect this client to the Channelised Service */
    status = ixHssAccChanConnect (
        hssPortId,         /* hssPortId */
        bytesPerTSTrigger, /* bytesPerTSTrigger */
        rxCircular,        /* rxCircular */
        numRxBytesPerTS,   /* numRxBytesPerTS */
        txPtrList,         /* txPtrList */
        numTxPtrLists,     /* numTxPtrLists */
        numTxBytesPerBlk,  /* numTxBytesPerBlk */
        rxCallback);       /* rxCallback */

    /* if there was any problem then update stats */
    if (status != IX_SUCCESS)
    {
        stats[hssPortId].chan.connectFails++;
        return;
    }

    /*******************/
    /* PREPARE TX DATA */
    /*******************/

    /* prepare data for transmit */
    for (i = 1; i <= IX_HSSACC_CODELET_CHAN_TX_LATENCY_FACTOR; i++)
    {
        /* by passing in txOffsets from 1 to the tx latency factor, we */
        /* can reuse our transmit function to initially fill up the tx */
        /* buffer with data */
        ixHssAccCodeletChannelisedDataTransmit (hssPortId, i);
    }
}

/*
 * Function definition: ixHssAccCodeletChannelisedServiceStart
 */

void
ixHssAccCodeletChannelisedServiceStart (
    IxHssAccHssPort hssPortId)
{
    IX_STATUS status;

    /*****************/
    /* START SERVICE */
    /*****************/

    /* start the Channelised Service for this client */
    status = ixHssAccChanPortEnable (hssPortId);

    /* if there was any problem then update stats */
    if (status != IX_SUCCESS)
    {
        stats[hssPortId].chan.portEnableFails++;
        return;
    }
}

/*
 * Function definition: ixHssAccCodeletChannelisedServiceRun
 */

void
ixHssAccCodeletChannelisedServiceRun (
    IxHssAccHssPort hssPortId)
{
}

/*
 * Function definition: ixHssAccCodeletChannelisedServiceStop
 */

void
ixHssAccCodeletChannelisedServiceStop (
    IxHssAccHssPort hssPortId)
{
    IX_STATUS status;
    ClientInfo *pClientInfo = &clientInfo[hssPortId];

    /****************/
    /* STOP SERVICE */
    /****************/

    /* stop the Channelised Service for the client */
    status = ixHssAccChanPortDisable (hssPortId);

    /* if there was any problem then update stats */
    if (status != IX_SUCCESS)
    {
        stats[hssPortId].chan.portDisableFails++;
        return;
    }

    /**********************/
    /* DISCONNECT SERVICE */
    /**********************/

    /* disconnect the Channelised Service for the client */
    status = ixHssAccChanDisconnect (hssPortId);

    /* if there was any problem then update stats */
    if (status != IX_SUCCESS)
    {
        stats[hssPortId].chan.disconnectFails++;
        return;
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
 * Function definition: ixHssAccCodeletChannelisedVerifySet
 */

void
ixHssAccCodeletChannelisedVerifySet (
    BOOL verifyOn)
{
    verify = verifyOn;
}
