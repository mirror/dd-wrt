/**
 * file IxHssAccCodeletPkt.h
 *
 * date 21 May 2002
 *
 * brief This file contains the interface for packetised implementation
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
 * defgroup IxHssAccCodeletPkt Intel (R) IXP400 Software HSS Access Codelet Packetised (IxHssAccCodeletPkt) API
 *
 * brief The interface for packetised implementation of the HSS Access Codelet.
 *
 * {
*/

#ifndef IXHSSACCCODELETPKT_H
#define IXHSSACCCODELETPKT_H

#include "IxHssAccCodeletMbuf.h"

/*
 * #defines for function return types, etc.
 */

/** num of TX mbufs per client */
#define IX_HSSACC_CODELET_PKT_NUM_TX_BUFS    (32)

/** num of RX mbufs per client */
#define IX_HSSACC_CODELET_PKT_NUM_RX_BUFS    (32)

/** mbuf size */
#define IX_HSSACC_CODELET_PKT_BUFSIZE (MBUF_SIZE)

/** TX and RX mbufs per client */
#define IX_HSSACC_CODELET_PKT_NUM_BUFS \
    (IX_HSSACC_CODELET_PKT_NUM_TX_BUFS + \
     IX_HSSACC_CODELET_PKT_NUM_RX_BUFS)

/** TX packet size (i.e. 8 mbufs) */
#define IX_HSSACC_CODELET_PKT_TX_PKTSIZE \
    (IX_HSSACC_CODELET_PKT_BUFSIZE * 8)

/** Packetised idle pattern (tx'ed when service is disabled) */
#define IX_HSSACC_CODELET_PKT_IDLE_PATTERN     (0x7F7F7F7F)

/** Raw mode idle pattern (tx'ed when no data - HDLC tx's ones or flags) */
#define IX_HSSACC_CODELET_PKT_RAW_IDLE_PATTERN (0x5F5F5F5F)

/** Number of stale bytes at start of first raw mode packet which
    can be safely ignored if they occur */
#define IX_HSSACC_CODELET_PKT_RAW_STALE_BYTES_MAX (128)


/*
 * Prototypes for interface functions.
 */

#endif /* IXHSSACCCODELETPKT_H */

/**} degroup IxHssAccCodeletPkt */

/**} degroup Codelets */
