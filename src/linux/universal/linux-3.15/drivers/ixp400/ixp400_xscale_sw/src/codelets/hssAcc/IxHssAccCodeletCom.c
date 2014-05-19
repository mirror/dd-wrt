/**
 * @file IxHssAccCodeletCom.c
 *
 * @date 21 May 2002
 *
 * @brief This file contains the common implementation of the HSS Access
 * Codelet.
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
 * @sa IxHssAccCodeletCom.h
 */

/*
 * Put the system defined include files required.
 */


/*
 * Put the user defined include files required.
 */

#include "IxHssAcc.h"

#include "IxHssAccCodelet_p.h"
#include "IxHssAccCodeletCom.h"

#if defined(__wince) && defined(IX_USE_SERCONSOLE)
#include "IxSerConsole.h"
#define printf ixSerPrintf
#endif

BOOL packetisedFlag;
BOOL channelisedFlag;

/*
 * #defines and macros used in this file.
 */

#define SHOW_GEN_STAT(hssPortId, description, field) \
do { \
    printf ("%-30s", description); \
    printf (" %10u", stats[hssPortId].gen.field); \
    printf ("\n"); \
} while (0)

#define SHOW_CHAN_STAT(hssPortId, description, field) \
do { \
    printf ("%-30s", description); \
    printf (" %10u", stats[hssPortId].chan.field); \
    printf ("\n"); \
} while (0)

#define SHOW_PKT_STAT(hssPortId, description, field) \
do { \
    IxHssAccHdlcPort hdlcPortId; \
    printf ("%-30s", description); \
    for (hdlcPortId = 0; hdlcPortId < IX_HSSACC_HDLC_PORT_MAX; \
         hdlcPortId++) \
    { \
        printf (" %10u", stats[hssPortId].pkt[hdlcPortId].field); \
    } \
    printf ("\n"); \
} while (0)

/*
 * Typedefs whose scope is limited to this file.
 */

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

/** Codelet statistics */
IxHssAccCodeletStats stats[IX_HSSACC_HSS_PORT_MAX];

/** the Codelet loopback state */
static BOOL swLoopback = FALSE;
/** the HSS port or external test equipment loopback state */
static BOOL hwLoopback = FALSE;

/** HSS error count */
static unsigned hssErrorCount = 0;

/*
 * Function definition: ixHssAccCodeletLastHssErrorHssPort0Callback
 */

void
ixHssAccCodeletLastHssErrorHssPort0Callback (
    unsigned lastHssError,
    unsigned servicePort)
{
    /* as we are running both channelised and packetised services, the */
    /* servicePort may refer to either one.  for this reason we don't */
    /* interpret the service port.  when running only one service the */
    /* servicePort parameter can be directly related to the service. */

    IxHssAccHssPort hssPortId = IX_HSSACC_HSS_PORT_0;

    /* examine lastHssError */
    switch (lastHssError)
    {
    case IX_HSSACC_NO_ERROR:
        break;

    case IX_HSSACC_TX_FRM_SYNC_ERR:
        stats[hssPortId].gen.txFrmSyncErrors++;
        break;

    case IX_HSSACC_TX_OVER_RUN_ERR:
        stats[hssPortId].gen.txOverRunErrors++;
        break;

    case IX_HSSACC_CHANNELISED_SW_TX_ERR:
        stats[hssPortId].gen.chanSwTxErrors++;
        break;

    case IX_HSSACC_PACKETISED_SW_TX_ERR:
        stats[hssPortId].gen.pktSwTxErrors++;
        break;

    case IX_HSSACC_RX_FRM_SYNC_ERR:
        stats[hssPortId].gen.rxFrmSyncErrors++;
        break;

    case IX_HSSACC_RX_OVER_RUN_ERR:
        stats[hssPortId].gen.rxOverRunErrors++;
        break;

    case IX_HSSACC_CHANNELISED_SW_RX_ERR:
        stats[hssPortId].gen.chanSwRxErrors++;
        break;

    case IX_HSSACC_PACKETISED_SW_RX_ERR:
        stats[hssPortId].gen.pktSwRxErrors++;
        break;

    default:
        stats[hssPortId].gen.unrecognisedErrors++;
        break;
    } /* switch (lastHssError) */
}

/*
 * Function definition: ixHssAccCodeletLastHssErrorHssPort1Callback
 */
void
ixHssAccCodeletLastHssErrorHssPort1Callback (
    unsigned lastHssError,
    unsigned servicePort)
{
    /* as we are running both channelised and packetised services, the */
    /* servicePort may refer to either one.  for this reason we don't */
    /* interpret the service port.  when running only one service the */
    /* servicePort parameter can be directly related to the service. */

    IxHssAccHssPort hssPortId = IX_HSSACC_HSS_PORT_1;

    /* examine lastHssError */
    switch (lastHssError)
    {
    case IX_HSSACC_NO_ERROR:
        break;

    case IX_HSSACC_TX_FRM_SYNC_ERR:
        stats[hssPortId].gen.txFrmSyncErrors++;
        break;

    case IX_HSSACC_TX_OVER_RUN_ERR:
        stats[hssPortId].gen.txOverRunErrors++;
        break;

    case IX_HSSACC_CHANNELISED_SW_TX_ERR:
        stats[hssPortId].gen.chanSwTxErrors++;
        break;

    case IX_HSSACC_PACKETISED_SW_TX_ERR:
        stats[hssPortId].gen.pktSwTxErrors++;
        break;

    case IX_HSSACC_RX_FRM_SYNC_ERR:
        stats[hssPortId].gen.rxFrmSyncErrors++;
        break;

    case IX_HSSACC_RX_OVER_RUN_ERR:
        stats[hssPortId].gen.rxOverRunErrors++;
        break;

    case IX_HSSACC_CHANNELISED_SW_RX_ERR:
        stats[hssPortId].gen.chanSwRxErrors++;
        break;

    case IX_HSSACC_PACKETISED_SW_RX_ERR:
        stats[hssPortId].gen.pktSwRxErrors++;
        break;

    default:
        stats[hssPortId].gen.unrecognisedErrors++;
        break;
    } /* switch (lastHssError) */
}

/*
 * Function definition: ixHssAccCodeletNumHssErrorsUpdate
 */

void
ixHssAccCodeletNumHssErrorsUpdate (
    IxHssAccHssPort hssPortId,
    unsigned numHssErrs)
{
    IX_STATUS status;

    /* if the error count has increased since last invocation */
    if (numHssErrs > hssErrorCount)
    {
        hssErrorCount = numHssErrs;

        /* initiate the retrieval of the last HSS error */
        status = ixHssAccLastErrorRetrievalInitiate (hssPortId);

        /* if there was any problem then update stats */
        if (status != IX_SUCCESS)
        {
            stats[hssPortId].gen.errorRetrievalFails++;
        }
    }
}

/*
 * Function definition: ixHssAccCodeletPktErrorsUpdate
 */

void
ixHssAccCodeletPktErrorsUpdate (
    IxHssAccHssPort hssPortId,
    IxHssAccHdlcPort hdlcPortId,
    IxHssAccPktStatus pktStatus)
{
    /* examine pktStatus */
    switch (pktStatus)
    {
    case IX_HSSACC_PKT_OK:
        break;

    case IX_HSSACC_STOP_SHUTDOWN_ERROR:
        stats[hssPortId].pkt[hdlcPortId].stopShutdownErrors++;
        break;

    case IX_HSSACC_HDLC_ALN_ERROR:
        stats[hssPortId].pkt[hdlcPortId].hdlcAlignErrors++;
        break;

    case IX_HSSACC_HDLC_FCS_ERROR:
        stats[hssPortId].pkt[hdlcPortId].hdlcFcsErrors++;
        break;

    case IX_HSSACC_RXFREE_Q_EMPTY_ERROR:
        stats[hssPortId].pkt[hdlcPortId].rxQueueEmptyErrors++;
        break;

    case IX_HSSACC_HDLC_MAX_FRAME_SIZE_EXCEEDED:
        stats[hssPortId].pkt[hdlcPortId].hdlcMaxSizeErrors++;
        break;

    case IX_HSSACC_HDLC_ABORT_ERROR:
        stats[hssPortId].pkt[hdlcPortId].hdlcAbortErrors++;
        break;

    case IX_HSSACC_DISCONNECT_IN_PROGRESS:
        stats[hssPortId].pkt[hdlcPortId].disconnectErrors++;
        break;

    default:
        stats[hssPortId].pkt[hdlcPortId].unrecognisedErrors++;
        break;
    } /* switch (pktStatus) */
}

/*
 * Function definition: ixHssAccCodeletShow
 */

void
ixHssAccCodeletShow (
    IxHssAccHssPort hssPortId)
{
    /* display general stats */
    printf ("\n================================================\n");
    printf ("IxHssAccCodelet General Stats for HSS port %u:\n",
	    (UINT32) hssPortId);
    printf ("================================================\n");
    SHOW_GEN_STAT(hssPortId, "Port init fails", portInitFails);
    SHOW_GEN_STAT(hssPortId, "Error retrieval fails", errorRetrievalFails);
    SHOW_GEN_STAT(hssPortId, "Tx frame sync errors", txFrmSyncErrors);
    SHOW_GEN_STAT(hssPortId, "Rx frame sync errors", rxFrmSyncErrors);
    SHOW_GEN_STAT(hssPortId, "Tx data over-run errors", txOverRunErrors);
    SHOW_GEN_STAT(hssPortId, "Rx data over-run errors", rxOverRunErrors);
    SHOW_GEN_STAT(hssPortId, "Channelised s/w Tx errors", chanSwTxErrors);
    SHOW_GEN_STAT(hssPortId, "Channelised s/w Rx errors", chanSwRxErrors);
    SHOW_GEN_STAT(hssPortId, "Packetised s/w Tx errors", pktSwTxErrors);
    SHOW_GEN_STAT(hssPortId, "Packetised s/w Rx errors", pktSwRxErrors);
    SHOW_GEN_STAT(hssPortId, "Unrecognised errors", unrecognisedErrors);

    if (TRUE == channelisedFlag)
    {
        /* display Channelised Service stats */
        printf ("\n");
        printf ("IxHssAccCodelet Channelised Service Stats:\n");
        printf ("\n");
        SHOW_CHAN_STAT(hssPortId, "Tx samples", txSamples);
        SHOW_CHAN_STAT(hssPortId, "Tx bytes", txBytes);
        SHOW_CHAN_STAT(hssPortId, "Rx samples", rxSamples);
        SHOW_CHAN_STAT(hssPortId, "Rx bytes", rxBytes);
        SHOW_CHAN_STAT(hssPortId, "Rx idles", rxIdles);
        SHOW_CHAN_STAT(hssPortId, "Rx verify fails", rxVerifyFails);
        SHOW_CHAN_STAT(hssPortId, "Connect fails", connectFails);
        SHOW_CHAN_STAT(hssPortId, "Port enable fails", portEnableFails);
        SHOW_CHAN_STAT(hssPortId, "Port disable fails", portDisableFails);
        SHOW_CHAN_STAT(hssPortId, "Disconnect fails", disconnectFails);
    }

    if (TRUE == packetisedFlag)
    {
        /* display Packetised Service stats */
        printf ("\n");
        printf ("IxHssAccCodelet Packetised Service Stats:\n");
        printf ("\n");
        printf ("%-30s %10s %10s %10s %10s\n",
		"", "Client 0", "Client 1", "Client 2", "Client 3");
        SHOW_PKT_STAT(hssPortId, "Tx packets", txPackets);
        SHOW_PKT_STAT(hssPortId, "Tx bytes", txBytes);
        SHOW_PKT_STAT(hssPortId, "Tx no buffers", txNoBuffers);
        SHOW_PKT_STAT(hssPortId, "Rx packets", rxPackets);
        SHOW_PKT_STAT(hssPortId, "Rx bytes", rxBytes);
        SHOW_PKT_STAT(hssPortId, "Rx no buffers", rxNoBuffers);
        SHOW_PKT_STAT(hssPortId, "Rx idles", rxIdles);
        SHOW_PKT_STAT(hssPortId, "Rx verify fails", rxVerifyFails);
        SHOW_PKT_STAT(hssPortId, "Connect fails", connectFails);
        SHOW_PKT_STAT(hssPortId, "Port enable fails", portEnableFails);
        SHOW_PKT_STAT(hssPortId, "Tx fails", txFails);
        SHOW_PKT_STAT(hssPortId, "Port disable fails", portDisableFails);
        SHOW_PKT_STAT(hssPortId, "Disconnect fails", disconnectFails);
        SHOW_PKT_STAT(hssPortId, "Unreturned Tx mbufs", txBufsInUse);
        SHOW_PKT_STAT(hssPortId, "Unreturned Rx mbufs", rxBufsInUse);
        SHOW_PKT_STAT(hssPortId, "Stop/Shutdown errors", stopShutdownErrors);
        SHOW_PKT_STAT(hssPortId, "HDLC Alignment errors", hdlcAlignErrors);
        SHOW_PKT_STAT(hssPortId, "HDLC Frame Check Sum errors", hdlcFcsErrors);
        SHOW_PKT_STAT(hssPortId, "Rx Queue Empty errors", rxQueueEmptyErrors);
        SHOW_PKT_STAT(hssPortId, "HDLC Max Size errors", hdlcMaxSizeErrors);
        SHOW_PKT_STAT(hssPortId, "HDLC Abort errors", hdlcAbortErrors);
        SHOW_PKT_STAT(hssPortId, "Disconnect In Progress errors", disconnectErrors);
        SHOW_PKT_STAT(hssPortId, "Unrecognised errors", unrecognisedErrors);
    }
}

/*
 * Function definition: ixHssAccCodeletShowReset
 */

void
ixHssAccCodeletShowReset (
    IxHssAccHssPort hssPortId)
{
    /* set entire stats structure to zero */
    ixOsalMemSet (&stats[hssPortId], 0x00, sizeof (stats[hssPortId]));
}

/*
 * Function definition: ixHssAccCodeletCodeletLoopbackGet
 */

BOOL
ixHssAccCodeletCodeletLoopbackGet (void)
{
    return swLoopback;
}

/*
 * Function definition: ixHssAccCodeletCodeletLoopbackSet
 */

void
ixHssAccCodeletCodeletLoopbackSet (
    BOOL codeletLoopback)
{
    swLoopback = codeletLoopback;
}

/*
 * Function definition: ixHssAccCodeletHssLoopbackGet
 */

BOOL
ixHssAccCodeletHssLoopbackGet (void)
{
    return hwLoopback;
}

/*
 * Function definition: ixHssAccCodeletHssLoopbackSet
 */

void
ixHssAccCodeletHssLoopbackSet (
    BOOL hssLoopback)
{
    hwLoopback = hssLoopback;
}


