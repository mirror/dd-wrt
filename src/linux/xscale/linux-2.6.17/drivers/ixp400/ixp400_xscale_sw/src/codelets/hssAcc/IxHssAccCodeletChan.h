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
