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

