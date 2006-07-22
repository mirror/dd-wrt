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
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
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
