/**
 * @file IxHssAccCodeletMbuf.c
 *
 * @date 29 May 2002
 *
 * @brief This file contains the mbuf pool implementation of the HSS Access
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
 * @sa IxHssAccCodeletMbuf.h
 */

/*
 * Put the system defined include files required.
 */

/*
 * Put the user defined include files required.
 */

#include "IxOsal.h"
#include "IxHssAccCodeletMbuf.h"

#if defined(__wince) && defined(IX_USE_SERCONSOLE)
#include "IxSerConsole.h"
#define printf ixSerPrintf
#endif

/*
 * Function definition: ixHssAccCodeletMbufGet
 */

IX_OSAL_MBUF *
ixHssAccCodeletMbufGet (
    IxHssAccCodeletMbufPool *poolId)
{
    IX_OSAL_MBUF *newMbuf;

    newMbuf = IX_OSAL_MBUF_POOL_GET(poolId); 

    IX_OSAL_ENSURE ((newMbuf != NULL),
	            "ixHssAccCodeletMbufGet - cannot get a new buffer from pool\n");

    return newMbuf;
}


/*
 * Function definition: ixHssAccCodeletMbufChainFree
 */

void
ixHssAccCodeletMbufChainFree (
    IX_OSAL_MBUF *pMbuf)
{
    IX_OSAL_MBUF_POOL_PUT_CHAIN(pMbuf);
}


/*
 * Function definition: ixHssAccCodeletMbufPoolInit
 */

void
ixHssAccCodeletMbufPoolInit (
    IxHssAccCodeletMbufPool **poolIdPtr,
    unsigned numPoolMbufs,
    unsigned poolMbufSize)
{
    *poolIdPtr = IX_OSAL_MBUF_POOL_INIT(numPoolMbufs, poolMbufSize, 
 			           "HssAcc Codelet Pool");

    IX_OSAL_ASSERT (*poolIdPtr != NULL);
}
