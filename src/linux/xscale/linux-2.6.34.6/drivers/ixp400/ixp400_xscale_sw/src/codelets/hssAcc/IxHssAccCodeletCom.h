/**
 * file IxHssAccCodeletCom.h
 *
 * date 21 May 2002
 *
 * brief This file contains the interface for common implementation of the
 * HSS Access Codelet.
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
 * sa IxHssAccCodelet.h
 */

/**
 * ingroup IxHssAccCodelet
 *
 * defgroup IxHssAccCodeletCom Intel (R) IXP400 Software HSS Access Codelet Common (IxHssAccCodeletCom) API
 *
 * brief The interface for common implementation of the HSS Access Codelet.
 *
 * {
*/

#ifndef IXHSSACCCODELETCOM_H
#define IXHSSACCCODELETCOM_H

#include "IxHssAcc.h"

/*
 * #defines for function return types, etc.
 */

/**
 * ingroup IxHssAccCodeletCom                
 *
 * def IX_HSSACC_CODELET_THREAD_PRI_HIGH
 *
 * brief High thread priority value
 *  
 */
#define IX_HSSACC_CODELET_THREAD_PRI_HIGH 90

/**
 * ingroup IxHssAccCodeletCom                
 *
 * def IX_HSSACC_CODELET_THREAD_PRI_MEDIUM
 *
 * brief Medium thread priority value
 *  
 */
#define IX_HSSACC_CODELET_THREAD_PRI_MEDIUM 160

/**
 * ingroup IxHssAccCodeletCom                
 *
 * def IX_HSSACC_CODELET_SEM_UNAVAILABLE
 *
 * brief Semaphore unavailable value
 *  
 */
#define IX_HSSACC_CODELET_SEM_UNAVAILABLE 0

/**
 * ingroup IxHssAccCodeletCom
 *
 * brief Type definition structure for general statistics
 */
typedef struct
{
    UINT32 portInitFails;
    UINT32 errorRetrievalFails;
    /* HSS errors */
    UINT32 txFrmSyncErrors;
    UINT32 rxFrmSyncErrors;
    /* HSS errors, per Service Port */
    UINT32 txOverRunErrors;
    UINT32 rxOverRunErrors;
    /* HSS Channelised errors */
    UINT32 chanSwTxErrors;
    UINT32 chanSwRxErrors;
    /* HSS Packetised errors, per Service Port */
    UINT32 pktSwTxErrors;
    UINT32 pktSwRxErrors;
    UINT32 unrecognisedErrors;
} GeneralStats;

/**
 * ingroup IxHssAccCodeletCom
 *
 * brief Type definition structure for channelised  statistics
 */
typedef struct
{
    UINT32 txSamples;
    UINT32 txBytes;
    UINT32 rxSamples;
    UINT32 rxBytes;
    UINT32 rxIdles;
    UINT32 rxVerifyFails;
    UINT32 connectFails;
    UINT32 portEnableFails;
    UINT32 portDisableFails;
    UINT32 disconnectFails;
} ChannelisedStats;

/**
 * ingroup IxHssAccCodeletCom
 *
 * brief Type definition structure for Packetised statistics
 */
typedef struct
{
    UINT32 txPackets;
    UINT32 txBytes;
    UINT32 txNoBuffers;
    UINT32 rxPackets;
    UINT32 rxBytes;
    UINT32 rxNoBuffers;
    UINT32 rxIdles;
    UINT32 rxVerifyFails;
    UINT32 connectFails;
    UINT32 portEnableFails;
    UINT32 txFails;
    UINT32 replenishFails;
    UINT32 portDisableFails;
    UINT32 disconnectFails;
    UINT32 txBufsInUse;
    UINT32 rxBufsInUse;
    UINT32 stopShutdownErrors;
    UINT32 hdlcAlignErrors;
    UINT32 hdlcFcsErrors;
    UINT32 rxQueueEmptyErrors;
    UINT32 hdlcMaxSizeErrors;
    UINT32 hdlcAbortErrors;
    UINT32 disconnectErrors;
    UINT32 unrecognisedErrors;
} PacketisedStats;

/**
 * ingroup IxHssAccCodeletCom
 *
 * brief Type definition structure for HSS Access Codelet statistics
 */
typedef struct
{
    GeneralStats gen;
    ChannelisedStats chan;
    PacketisedStats pkt[IX_HSSACC_HDLC_PORT_MAX];
} IxHssAccCodeletStats;

/** 
 * ingroup IxHssAccCodeletCom
 * Codelet statistics 
 */
extern IxHssAccCodeletStats stats[IX_HSSACC_HSS_PORT_MAX];

/*
 * Prototypes for interface functions.
 */

/**
 * ingroup IxHssAccCodeletCom
 *
 * fn void ixHssAccCodeletLastHssErrorHssPort0Callback (
           unsigned lastHssError,
           unsigned servicePort)
 *
 * param unsigned lastHssError (in) - the last HSS error registered by the
 * NPE.
 * param unsigned servicePort (in) - this is the NPE service port number -
 * 0-3 for Packetised Services, 0 for Channelised Service.
 *
 * return void
 *
 * This function is of type IxHssAccLastErrorCallback, the prototype of the
 * clients function to accept notification of the last error from port 0.
 *
 * This function is registered through ixHssAccPortInit().  It will be
 * called in response to a call to ixHssAccLastErrorRetrievalInitiate().
 * <P>
 * This callback will record the last error, so that errors can be
 * accumulated and later reported.
 */
void
ixHssAccCodeletLastHssErrorHssPort0Callback (
    unsigned lastHssError,
    unsigned servicePort);

/**
 * ingroup IxHssAccCodeletCom
 *
 * fn void ixHssAccCodeletLastHssErrorHssPort1Callback (
           unsigned lastHssError,
           unsigned servicePort)
 *
 * param unsigned lastHssError (in) - the last HSS error registered by the
 * NPE.
 * param unsigned servicePort (in) - this is the NPE service port number -
 * 0-3 for Packetised Services, 0 for Channelised Service.
 *
 * return void
 *
 * This function is of type IxHssAccLastErrorCallback, the prototype of the
 * clients function to accept notification of the last error from HSS port 
 * 1. 
 *
 * This function is registered through ixHssAccPortInit().  It will be
 * called in response to a call to ixHssAccLastErrorRetrievalInitiate().
 * <P>
 * This callback will record the last error, so that errors can be
 * accumulated and later reported.
 */
void
ixHssAccCodeletLastHssErrorHssPort1Callback (
    unsigned lastHssError,
    unsigned servicePort);

/**
 * ingroup IxHssAccCodeletCom
 *
 * fn void ixHssAccCodeletNumHssErrorsUpdate (
           IxHssAccHssPort hssPortId,
           unsigned numHssErrs)
 * 
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * param unsigned numHssErrs (in) - The total number of HSS port errors
 * since initial port configuration.
 *
 * return void
 *
 * This function will check the number of HSS errors.  If the HSS error
 * count has increased since the last invocation of this function then
 * ixHssAccLastErrorRetrievalInitiate() is invoked to query the last error.
 */

void
ixHssAccCodeletNumHssErrorsUpdate (
    IxHssAccHssPort hssPortId,
    unsigned numHssErrs);

/**
 * ingroup IxHssAccCodeletCom
 *
 * fn void ixHssAccCodeletPktErrorsUpdate (
           IxHssAccHssPort hssPortId,
           IxHssAccHdlcPort hdlcPortId,
           IxHssAccPktStatus pktStatus)
 * 
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * param IxHssAccHdlcPort hdlcPortId (in) - the HDLC port ID (0,1,2,3).
 * param IxHssAccPktStatus pktStatus (in) - Indicates the status of
 * packets passed to the client.
 *
 * return void
 *
 * This function will update the packet status error stats.
 */

void
ixHssAccCodeletPktErrorsUpdate (
    IxHssAccHssPort hssPortId,
    IxHssAccHdlcPort hdlcPortId,
    IxHssAccPktStatus pktStatus);

#endif /* IXHSSACCCODELETCOM_H */
/**} defgroup IxHssAccCodeletCom*/

/**} defgroup Codelets*/
