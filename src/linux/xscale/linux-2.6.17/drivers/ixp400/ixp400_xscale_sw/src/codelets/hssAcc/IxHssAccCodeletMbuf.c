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
