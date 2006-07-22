/**
 * file IxHssAccCodeletMbuf.h
 *
 * date 29 May 2002
 *
 * brief This file contains the interface for mbuf pool implementation
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
 * defgroup IsHssAccCodeletMbuf Intel (R) IXP400 Software HSS Access Codelet MBuf Pool (IsHssAccCodeletMbuf) API
 *
 * brief The interface for mbuf pool implementation of the HSS Access Codelet.
 *
 * {
*/

#ifndef IXHSSACCCODELETMBUF_H
#define IXHSSACCCODELETMBUF_H

#include "IxOsal.h"

/*
 * #defines for function return types, etc.
 */

/** preferred netBufLib cluster size is 2K */
#define MBUF_SIZE 2048

/*
 * Put the user defined include files required.
 */

/*
 * Typedefs
 */

/**
 * typedef IxHssAccCodeletMbufPool
 * brief   Pool Id type used to reference a particular mbuf pool
 */

typedef IX_OSAL_MBUF_POOL IxHssAccCodeletMbufPool;

/*
 * Prototypes for interface functions.
 */

/**
 * fn IX_OSAL_MBUF * ixHssAccCodeletMbufGet (
           IxHssAccCodeletMbufPool *poolId,
           unsigned mbufSize)
 *
 * param IxHssAccCodeletMbufPool *poolId (in) - pointer to the pool
 *                                               to get the mbuf from.
 *
 * This routine gets an mbuf from the specified mbuf memory pool
 * 
 * return
 *      - A pointer to the mbuf, or NULL if no mbuf was got from the pool
 */

IX_OSAL_MBUF *
ixHssAccCodeletMbufGet (
    IxHssAccCodeletMbufPool *poolId);


/**
 * fn void ixHssAccCodeletMbufChainFree (
           IX_OSAL_MBUF *pMbuf)
 *
 * param IX_OSAL_MBUF *pMbuf (in) - A pointer to the first mbuf in
                               the mbuf chain to be freed
 *
 * This frees a chain of mbufs back to its memory pool
 * 
 * return
 *      - None
 */

void
ixHssAccCodeletMbufChainFree (
    IX_OSAL_MBUF *pMbuf);


/**
 * fn void ixHssAccCodeletMbufPoolInit (
           IxHssAccCodeletMbufPool **poolIdPtr,
           unsigned numPoolMbufs,
           unsigned poolMbufSize)
 *
 * param IxHssAccCodeletMbufPool **poolIdPtr (out) - A pointer to the mbuf
 * pool pointer created
 * param unsigned numPoolMbufs (in) - The number of mbufs the pool should
 * contain
 * param unsigned poolMbufSize (in) - The size, in bytes, of the mbufs in
 * the pool
 *
 * This creates a memory pool of mbufs.  Use ixHssAccCodeletMbufGet() to
 * get an mbuf from the pool, and ixHssAccCodeletMbufFree() or
 * ixHssAccCodeletMbufChainFree() to return mbufs to the pool
 * 
 * return
 *      - None
 */

void
ixHssAccCodeletMbufPoolInit (
    IxHssAccCodeletMbufPool **poolIdPtr,
    unsigned numPoolMbufs,
    unsigned poolMbufSize);


#endif /* #ifndef IXHSSACCCODELETMBUF_H */

/**} defgroup IxHssAccCodeletMbuf*/

/**} defgroup Codelets*/

