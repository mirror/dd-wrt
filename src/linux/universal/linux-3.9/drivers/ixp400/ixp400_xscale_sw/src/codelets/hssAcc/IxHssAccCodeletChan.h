/**
 * file IxHssAccCodeletChan.h
 *
 * date 21 May 2002
 *
 * brief This file contains the interface for channelised implementation
 * of the HSS Access Codelet.
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
 * defgroup IxHssAccCodeletChan Intel (R) IXP400 Software HSS Access Codelet Channelized (IxHssAccCodeletChan) 
 *
 * brief The interface for channelised implementation of the HSS Access Codelet.
 *
 * {
*/

#ifndef IXHSSACCCODELETCHAN_H
#define IXHSSACCCODELETCHAN_H

/*
 * #defines for function return types, etc.
 */

/**  
 * ingroup IxHssAccCodeletChan
 *
 * def IX_HSSACC_CODELET_CHAN_BYTES_PER_TS_TRIG
 *
 * brief trigger rate of 1ms 
 */
#define IX_HSSACC_CODELET_CHAN_BYTES_PER_TS_TRIG  (8)

/** 
 * ingroup IxHssAccCodeletChan
 * 
 * def IX_HSSACC_CODELET_CHAN_NUM_CHANS
 * 
 * brief number of channels 
 */
#define IX_HSSACC_CODELET_CHAN_NUM_CHANS         (16)

/**
 * ingroup IxHssAccCodeletChan
 *
 * def IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE 
 * 
 * brief bytes per sample of data 
 */
#define IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE  (44)

/** 
 * ingroup IxHssAccCodeletChan
 *
 * def IX_HSSACC_CODELET_CHAN_RX_LATENCY_FACTOR
 *
 * brief RX latency factor 
 */
#define IX_HSSACC_CODELET_CHAN_RX_LATENCY_FACTOR  (4)

/**
 * ingroup IxHssAccCodeletChan
 *
 * def IX_HSSACC_CODELET_CHAN_RX_BUFSIZE_PERCHAN
 *
 * brief RX buffer size per channel 
 */
#define IX_HSSACC_CODELET_CHAN_RX_BUFSIZE_PERCHAN \
    (IX_HSSACC_CODELET_CHAN_RX_LATENCY_FACTOR * \
     IX_HSSACC_CODELET_CHAN_BYTES_PER_SAMPLE)

/** 
 * ingroup IxHssAccCodeletChan
 *
 * def IX_HSSACC_CODELET_CHAN_TX_LATENCY_FACTOR
 *
 * brief TX latency factor 
 */
#define IX_HSSACC_CODELET_CHAN_TX_LATENCY_FACTOR  (8)

/** 
 * ingroup IxHssAccCodeletChan
 *
 * def IX_HSSACC_CODELET_CHAN_IDLE_PATTERN  
 * 
 * brief Channelised idle pattern 
 */
#define IX_HSSACC_CODELET_CHAN_IDLE_PATTERN    (0x7F)

/*
 * Prototypes for interface functions.
 */

#endif /* IXHSSACCCODELETCHAN_H */

/**} defgroup IxHssAccCodeletChan*/

/**} defgroup Codelets*/
