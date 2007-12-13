/**
 * file IxHssAccCodelet_p.h
 *
 * date 26 Mar 2002
 *
 * brief This file contains the internal functions for the HSS Access Codelet.
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
#ifndef IXHSSACCCODELET_P_H
#define IXHSSACCCODELET_P_H

#include "IxHssAcc.h"

/*
 * Define
 */

/*
 *  Enumeration
 */

/**< Type of services offered by codelet */
typedef enum
{
    IX_HSSACC_CODELET_PKT_SERV_ONLY = 1,    /**< Packetised service only */
    IX_HSSACC_CODELET_CHAN_SERV_ONLY,       /**< Channelised service only */
    IX_HSSACC_CODELET_PKT_CHAN_SERV         /**< Packetised and channelised service */
} IxHssAccCodeletOperation;  

/**< HSS Port supported by codelet */
typedef enum
{
    IX_HSSACC_CODELET_HSS_PORT_0_ONLY = 1,    /**< HSS Port 0 Only */
    IX_HSSACC_CODELET_HSS_PORT_1_ONLY,        /**< HSS Port 1 Only */
    IX_HSSACC_CODELET_DUAL_PORTS              /**< Both HSS Port 0 & 1 */
} IxHssAccCodeletPortMode; 

/**< Codelet [verifies / not verifies] traffic received in HSS loopback more */
typedef enum
{
    IX_HSSACC_CODELET_VERIFY_ON = 1,    /**< Codelet verifies traffic received */
    IX_HSSACC_CODELET_VERIFY_OFF        /**< Codelet does not verify traffic received */
} IxHssAccCodeletVerifyMode; 

/*
 * Prototypes for interface functions.
 */

/**
 * fn IX_STATUS ixHssAccCodeletInit (void)
 *
 * This function initializes and starts NPE-A, NpeMh, QMgr and HssAcc 
 */
IX_STATUS
ixHssAccCodeletInit (void);


/**
 * fn ixHssAccCodeletServiceStart (IxHssAccCodeletPortMode portMode)
 *
 * param IxHssAccCodeletPortMode portMode (in) - port mode of code.
 *        Refer to description in IxHssAccCodelet.h  
 *
 * This function is the launch-point for the codelet.  It assumes the
 * environment has been setup.  It initialises and creates a thread to run the
 * IxHssAcc services for each HSS port based on the two input.
 */
void
ixHssAccCodeletServiceStart (IxHssAccCodeletPortMode portMode);

/**
 * fn void ixHssAccCodeletConfigure (
           IxHssAccHssPort hssPortId)
 *
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 *
 * For each top-level scenario the HSS port needs to be configured
 * appropriately.
 * <P>
 * The Codelet will simulate five simultaneous clients, four using the
 * Packetised Service and one using the Channelised Service.  Each
 * packetised client will use a subset of the timeslots available in the
 * T1/E1 stream for that client.  The channelised client will use timeslots
 * from the remaining available timeslots from each of the four T1/E1
 * streams.  Two packetised clients will use RAW mode while two will use
 * HDLC mode.  The channelised client will use some timeslots to carry
 * voice at 64Kbps and other timeslots to carry voice at 56Kbps (employing
 * "bit-stealing").
 * <P>
 * The HSS Port needs to be configured to allow the HSS to interface to the
 * particular framer and protocol being used. The configuration options and
 * their values are shown below.  Some options are relevant to TX and
 * others to both TX and RX.  These values have been initially chosen based
 * on the HSS co-processor default settings.  These values will get written
 * to the HSSTXPCR, HSSRXPCR, HSSTXFCR, and HSSRXFCR registers of the HSS
 * co-processor.
 * <UL>
 *   <LI> Frame sync type = <B>active low</B>          (HSSTXPCR/HSSRXPCR)
 *   <LI> Frame sync output en. = <B>output rising</B> (HSSTXPCR/HSSRXPCR)
 *   <LI> Frame sync clock edge = <B>rising</B>        (HSSTXPCR/HSSRXPCR)
 *   <LI> Data clock edge = <B>falling</B>             (HSSTXPCR/HSSRXPCR)
 *   <LI> Clock direction = <B>output</B>              (HSSTXPCR/HSSRXPCR)
 *   <LI> Frame usage = <B>enabled</B>                 (HSSTXPCR/HSSRXPCR)
 *   <LI> Data rate = <B>clock rate</B>                (HSSTXPCR/HSSRXPCR)
 *   <LI> Data polarity = <B>same</B>                  (HSSTXPCR/HSSRXPCR)
 *   <LI> Data endianness = <B>lsb endian</B>          (HSSTXPCR/HSSRXPCR)
 *   <LI> Drain mode = <B>normal</B>                   (HSSTXPCR)
 *   <LI> FBit usage = <B>data</B>                     (HSSTXPCR/HSSRXPCR)
 *   <LI> Data enable = <B>data</B>                    (HSSTXPCR)
 *   <LI> 56K type = <B>low</B>                        (HSSTXPCR)
 *   <LI> Unassigned type = <B>low</B>                 (HSSTXPCR)
 *   <LI> FBit type = <B>fifo</B>                      (HSSTXPCR)
 *   <LI> 56K endianness = <B>bit 7 unused</B>         (HSSTXPCR)
 *   <LI> 56K selection = <B>32/8 data</B>             (HSSTXPCR)
 *   <LI> Frame offset = <B>0</B>                      (HSSTXFCR/HSSRXFCR)
 *   <LI> Frame size = <B>1024 (4 * 32 * 8)</B>        (HSSTXFCR/HSSRXFCR)
 * </UL>
 * The following configuration parameters are related to the scenarios we
 * are running:
 * <UL>
 *   <LI> HssChannelized Number of Channels = <B>16</B>
 *   <LI> HssPacketized Number of Packet-Pipes = <B>4</B>
 *   <LI> HssChannelized Idle Pattern = <B>0x7F</B>
 *   <LI> Loopback = <B>TRUE</B>/<B>FALSE</B> (depends on scenario)
 *   <LI> HssPacketized Pipes 0-3 Idle Patterns = <B>0x7F7F7F7F</B>
 *   <LI> Clock speed = <B>8192</B>
 * </UL>
 * The timeslots of each of the four T1/E1 streams will be assigned as
 * follows:
 * <UL>
 *   <LI> 4 timeslots for 64K channelised <B>(8, 15, 24, 31)</B>
 *   <LI> 4 timeslots for 56K channelised <B>(4, 12, 20, 28)</B>
 *   <LI> 16 timeslots for HDLC <B>(1-2, 5-6, 9-10, 13-14, 17-18, 21-22,
 * 25-26, 29-30)</B>
 *   <LI> 6 timeslots unassigned <B>(3, 7, 11, 19, 23, 27)</B>
 * </UL>
 * The Codelet will provide ixHssAccCodeletLastHssErrorCallback() as the
 * IxHssAccLastErrorCallback during configuration.
 * ixHssAccLastErrorRetrievalInitiate() will be called whenever an increase
 * in the number of HSS errors is detected.  This will initiate a callback
 * to the Codelet's ixHssAccCodeletLastHssErrorCallback().
 */
void
ixHssAccCodeletConfigure (
    IxHssAccHssPort hssPortId);

/**
 * fn void ixHssAccCodeletPacketisedServiceStart (
           IxHssAccHssPort hssPortId,
           IxHssAccHdlcPort hdlcPortId)
 *
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * param IxHssAccHdlcPort hdlcPortId (in) - the port id (0,1,2,3) to start
 * the service on.
 * 
 * First create both TX and RX mbuf pools.  Each client will create a pool
 * of 32 mbufs of size 2K for TX and 32 mbufs of size 2K for RX.  For
 * sending packets larger than 2K, mbufs will be chained together.
 * Similarly the NPE will chain together mbufs buffers as necssary to
 * receive packets larger than 2K.  Both the TX and RX mbufs will be
 * recycled so that the pools just circulate the same mbufs over and over.
 * <P>
 * Next connect the client to one of the four available HDLC ports by
 * invoking ixHssAccPktPortConnect().  The connection parameters will be
 * specified as follows.  Where indicated, the values will get written to
 * the TxCtxt and RxCtxt registers of the HDLC co-processor.
 * <UL>
 *   <LI> HDLC framing = <B>TRUE/FALSE</B> (clients 0 and 2 will use HDLC
 * mode, clients 1 and 3 will use RAW mode).
 *   <LI> Raw mode block size = <B>4096 words</B> (i.e. 16K)
 *   <LI> Raw mode idle pattern = <B>0x5F5F5F5F</B>
 *   <LI> HDLC idle transmission type = <B>flags</B> (TxCtxt/RxCtxt)
 *   <LI> HDLC data endianness = <B>lsb endian</B> (TxCtxt/RxCtxt
 *   <LI> CRC type (CRC-16 or CRC-32) = <B>CRC-32</B> (TxCtxt/RxCtxt)
 *   <LI> Number of flags inserted at start of frame = <B>1</B> (TxCtxt)
 * </UL>
 * <P>
 * The method ixHssAccCodeletPktRxCallback() will be supplied as the
 * IxHssAccPktRxCallback to receive packets.  A pointer to a structure
 * identifying the client will be passed as the rxUserId parameter.
 * <P>
 * The method ixHssAccCodeletPktRxFreeLowCallback() will be supplied as the
 * IxHssAccPktRxFreeLowCallback to provide RX buffers on request.  A
 * pointer to a structure identifying the client will be passed as the
 * rxFreeLowUserId.
 * <P>
 * The method ixHssAccCodeletPktTxDoneCallback() will be supplied as the
 * IxHssAccPktTxDoneCallback to be passed back a packet after it has been
 * transmitted.  A pointer to a structure identifying the client will be
 * passed as the txDoneUserId.
 * <P>
 * After connecting to the service, the RX buffers will be passed to the
 * IxHssAcc component by calling ixHssAccPktPortRxFreeReplenish() to make
 * them available for receiving packets into.
 * <P>
 * Finally the Packetised Service will be started and the RX flow will be
 * enabled by invoking ixHssAccPktPortEnable().
 */
void
ixHssAccCodeletPacketisedServiceStart (
    IxHssAccHssPort hssPortId,
    IxHssAccHdlcPort hdlcPortId);

/**
 * fn void ixHssAccCodeletPacketisedServiceRun (
           IxHssAccHssPort hssPortId,
           IxHssAccHdlcPort hdlcPortId)
 *
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * param IxHssAccHdlcPort hdlcPortId (in) - the port id (0,1,2,3) to run
 * the service on.
 *
 * Packets will be transmitted by calling ixHssAccPktPortTx().  Buffers
 * will be taken from the TX pool and chained together to create a buffer
 * for sending.
 * <P>
 * Both the clients using HDLC mode and the clients using RAW mode will
 * send and receive packets of 16Kb in size.
 * <P>
 * In the transmit-done callback, ixHssAccCodeletPktTxDoneCallback(), the
 * transmitted packets will be returned to the TX buffer pool to be reused.
 * This callback will also maintain statistics of packets transmitted and
 * number of HSS errors.
 * <P>
 * Packets will be received via the callback routine
 * ixHssAccCodeletPktRxCallback().  Received packets will be returned to
 * the RX buffer pool to be reused.  This callback will also maintain
 * statistics of packets received and number of HSS errors.
 * <P>
 * When operating as data source/sink packets will be created and
 * transmitted, and the contents of received packets will be verified 
 * against those transmitted (the HSS is as loopback).
 */
void
ixHssAccCodeletPacketisedServiceRun (
    IxHssAccHssPort hssPortId,
    IxHssAccHdlcPort hdlcPortId);

/**
 * fn void ixHssAccCodeletPacketisedServiceStop (
           IxHssAccHssPort hssPortId,
           IxHssAccHdlcPort hdlcPortId)
 *
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 * param IxHssAccHdlcPort hdlcPortId (in) - the port id (0,1,2,3) to stop
 * the service on.
 *
 * Stop the Packetised Service and disable the RX flow by calling
 * ixHssAccPktPortDisable().
 * <P>
 * Disconnect the client from the HDLC port by invoking
 * ixHssAccPktPortDisconnect().
 */
void
ixHssAccCodeletPacketisedServiceStop (
    IxHssAccHssPort hssPortId,
    IxHssAccHdlcPort hdlcPortId);

/**
 * fn void ixHssAccCodeletPacketisedVerifySet (
           BOOL verifyOn)
 *
 * param BOOL verifyOn (in) - whether to verify data or not.
 *
 * This function sets the verification of data on or off.  Verification of
 * data can be turned off to improve performance.
 */
void
ixHssAccCodeletPacketisedVerifySet (
    BOOL verifyOn);

/**
 * fn void ixHssAccCodeletChannelisedServiceConfigure (
           IxHssAccHssPort hssPortId)
 *
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 *
 * First create both the TX and RX buffers.
 * <P>
 * For controlling the RX and TX of data, a frequent sampling rate will be
 * used.  This allows flexibility in deciding when to RX and TX data.  The
 * common configuration for the Channelised Service will be as follows:
 * <UL>
 *   <LI> Trigger rate = <B>1ms</B>
 *   <LI> Bytes per timeslot trigger = <B>8</B> (8 bytes @ 1ms => 64Kbps)
 *   <LI> Number of channels = <B>16</B>
 *   <LI> Bytes per sample = <B>44 bytes</B> (5.5ms sampling rate)
 * </UL>
 * These parameters are example values that would be typical of a voice
 * over ATM application.  A voice sample of 44 bytes plus an AAL2 header
 * will fit into the 48 byte payload of an ATM cell.  A trigger rate of 1ms
 * ensures that we are very responsive to handling voice samples.  A
 * trigger rate of 5ms or 6ms would seem more appropriate, but missing a
 * trigger due to busy CPU means a much longer delay before the data gets
 * processed on the next trigger.
 * <P>
 * A latency factor is introduced for the RX path to give the client
 * sufficient time to process the data received from the HSS port.  For
 * example, the client may wish to retransmit the data over an ATM network.
 * The higher this latency, the longer it will be before data received from
 * the HSS port is overwritten.  The RX configuration will be as follows:
 * <UL>
 *   <LI> RX latency factor = <B>4</B> (=> 3 * 5.5ms before data is
 * over-written)
 *   <LI> RX buffer size per channel = <B>176 bytes</B> (bytes per sample
 * times latency factor, i.e. 44 * 4)
 *   <LI> RX buffer size for all channels = <B>2816 bytes</B> (buffer size
 * per channel times number of channels, i.e. 176 * 16)
 * </UL>
 * <P>
 * A latency factor is introduced for the TX path to absorb the jitter
 * introduced by the network the data is coming from.  The HSS port
 * transmits data at a constant rate, however packets don't arrive at a
 * constant rate from a data network.  The TX configuration will be as
 * follows:
 * <UL>
 *   <LI> TX latency factor = <B>8</B> (=> 7 * 5.5ms before TX data runs
 * out)
 *   <LI> TX buffer size per channel = <B>352 bytes</B> (bytes per sample
 * times latency factor, i.e. 44 * 8)
 *   <LI> TX buffer size for all channels = <B>5632 bytes</B> (buffer size
 * per channel times number of channels, i.e. 352 * 16)
 * </UL>
 * <P>
 * For the TX service an array of pointers to samples is also required.
 * This array will contain a pointer to each 44-byte sample in the TX
 * buffer, i.e. 5632 / 44 = 128 pointers (or in other words the number of
 * channels times the latency factor, i.e. 16 * 8).
 * <P>
 * Next connect the client to the TX/RX NPE Channelised Service by invoking
 * ixHssAccChanConnect().  The connection parameters will be as follows:
 * <UL>
 *   <LI> Bytes per timeslot trigger = <B>8</B>
 *   <LI> RX circular buffer = <B>RX buffer</B>
 *   <LI> Number of RX bytes per timeslot = <B>176</B> (RX buffer size per
 * channel)
 *   <LI> TX pointer list = <B>TX pointer array</B>
 *   <LI> Number of TX pointer lists = <B>8</B> (latency factor)
 *   <LI> Number of TX bytes per block = <B>44</B> (bytes per sample)
 * </UL>
 * <P>
 * The client will supply ixHssAccCodeletChanRxCallback() as the
 * IxHssAccChanRxCallback parameter to handle the actual TX/RX of
 * channelised data.
 * <P>
 * Note, before starting the service the Codelet will need to prepare the
 * data to be transmitted.
 */
void
ixHssAccCodeletChannelisedServiceConfigure (
    IxHssAccHssPort hssPortId);

/**
 * fn void ixHssAccCodeletChannelisedServiceStart (
           IxHssAccHssPort hssPortId)
 *
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 *
 * Start the Channelised Service and enable the TX/RX flows by
 * invoking ixHssAccChanPortEnable().
 */
void
ixHssAccCodeletChannelisedServiceStart (
    IxHssAccHssPort hssPortId);

/**
 * fn void ixHssAccCodeletChannelisedServiceRun (
           IxHssAccHssPort hssPortId)
 *
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 *
 * Transmit traffic via the TX buffer and receive traffic via the RX
 * buffer.  No action needs to be taken to transmit or receive data.  Once
 * the Channelised Service is started the NPE will automatically transmit
 * data from the TX buffer and receive data into the RX buffer.
 * <P>
 * The callback routine ixHssAccCodeletChanRxCallback() will be called at
 * the sampling rate to tell the Codelet where data is currently being
 * transmitted from the TX buffer and received into the RX buffer.  This
 * callback will maintain statistics of data transmitted and received and
 * number of HSS errors.
 * <P>
 * When the Codelet is acting as data source/sink the data in the TX buffer
 * will be prepared for transmission.  This will involve writing a unique,
 * changing bit pattern to each channel.  The data in the RX buffer will be
 * verified to ensure it matches the data transmitted (the HSS is as loopback).
 * <P>
 * For transmitting data, the strategy used will be to write tx data to
 * slots we know the NPE has finished transmitting from.  As the NPE
 * transmits data and moves on to the next set of pointers, we will fill in
 * new data in the set of pointers just transmitted.  In this way the NPE
 * will always be transmitting new data.  Note, this is an artificial
 * scenario as we always have tx data available without any delay.  In a
 * real application data should be transmitted when available, while
 * ensuring that the NPE isn't too far behind (causing latency) or too
 * close behind (trying to transmit when there is no data ready).  Also, as
 * we are creating the tx data ourselves we can simply use one large tx
 * buffer and pass pointers to this buffer to the NPE.
 * <P>
 * For receiving data, the NPE tells us where data is currently being
 * received to.  As we want to handle voice data as samples, we wait for a
 * full sample to be received before processing it.
 */
void
ixHssAccCodeletChannelisedServiceRun (
    IxHssAccHssPort hssPortId);

/**
 * fn void ixHssAccCodeletChannelisedServiceStop (
           IxHssAccHssPort hssPortId)
 *
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 *
 * Stop the Channelised Service and disable the TX/RX flows by calling
 * ixHssAccChanPortDisable().
 * <P>
 * Disconnect the client from the TX/RX NPE Channelised Service by invoking
 * ixHssAccChanDisconnect().
 */
void
ixHssAccCodeletChannelisedServiceStop (
    IxHssAccHssPort hssPortId);

/**
 * fn void ixHssAccCodeletChannelisedVerifySet (
           BOOL verifyOn)
 *
 * param BOOL verifyOn (in) - whether to verify data or not.
 *
 * This function sets the verification of data on or off.  Verification of
 * data can be turned off to improve performance.
 */
void
ixHssAccCodeletChannelisedVerifySet (
    BOOL verifyOn);

/**
 * fn void ixHssAccCodeletShow (
           IxHssAccHssPort hssPortId)
 *
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 *
 * Display statistics for the specified HSS port.
 */
void
ixHssAccCodeletShow (
    IxHssAccHssPort hssPortId);

/**
 * fn void ixHssAccCodeletShowReset (
           IxHssAccHssPort hssPortId)
 *
 * param IxHssAccHssPort hssPortId (in) - the HSS port ID (0 or 1).
 *
 * Reset the statistics for the specified HSS port.
 */
void
ixHssAccCodeletShowReset (
    IxHssAccHssPort hssPortId);

/**
 * fn BOOL ixHssAccCodeletCodeletLoopbackGet (void)
 *
 * This function returns the Codelet loopback state.
 */

BOOL
ixHssAccCodeletCodeletLoopbackGet (void);

/**
 * fn void ixHssAccCodeletCodeletLoopbackSet (
           BOOL codeletLoopback)
 *
 * param BOOL codeletLoopback (in) - the Codelet loopback state.
 *
 * This function sets the Codelet loopback state.
 */

void
ixHssAccCodeletCodeletLoopbackSet (
    BOOL codeletLoopback);

/**
 * fn BOOL ixHssAccCodeletHssLoopbackGet (void)
 *
 * This function returns the HSS loopback state.
 */

BOOL
ixHssAccCodeletHssLoopbackGet (void);

/**
 * fn void ixHssAccCodeletHssLoopbackSet (
           BOOL hssLoopback)
 *
 * param BOOL hssLoopback (in) - the HSS loopback state.
 *
 * This function sets the HSS loopback state.
 */

void
ixHssAccCodeletHssLoopbackSet (
    BOOL hssLoopback);


#endif /* IXHSSACCCODELET_P_H */

/** } defgroup IxHssAccCodelet*/

/** } defgroup Codelets*/
