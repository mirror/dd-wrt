/**
 * @file        IxEthAccCodeletBufMan.c
 * @author 	Intel Corporation
 * @date        25-08-2004
 *
 * @brief This file contains the implementation of the Ethernet Access 
 * Codelet that implements a buffer management system for the codelet
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

/*
 * Put the system defined include files required.
 */
#include "IxOsal.h"

/*
 * Put the user defined include files required.
 */
#include "IxEthAccCodelet.h"
#include "IxEthAccCodelet_p.h"
#include "IxEthDBPortDefs.h"

IX_OSAL_MBUF_POOL *ixEthAccCodeletMBufPool = NULL;

/**
 * Function definition: ixEthAccCodeletMemPoolInit()
 *
 * Initialise Ethernet Access Codelet MBUF pool
 *
 */

IX_STATUS ixEthAccCodeletMemPoolInit(void)
{
    UINT32 bufNo;
    IX_OSAL_MBUF *mBufPtr;

    ixEthAccCodeletMBufPool = IX_OSAL_MBUF_POOL_INIT (IX_ETHACC_CODELET_MBUF_POOL_SIZE,
						      IX_ETHACC_CODELET_PCK_SIZE,
						      "ixEthAccCodeletMBufPool");

    if ( ixEthAccCodeletMBufPool == NULL )
    {
	printf("BufMan: Failed to create the Codelet Buffer Pool\n");
        return IX_FAIL;	
    }

    ixEthAccCodeletFreeCount = 0;

    /* store the mbuf in a linked list */
    for(bufNo = 0;
        bufNo < IX_ETHACC_CODELET_MBUF_POOL_SIZE;
        bufNo++)
    {
       /* Get free mBuf from the pool */
       mBufPtr = IX_OSAL_MBUF_POOL_GET(ixEthAccCodeletMBufPool);
       
       /* Add it to our Receive free queue */
       IX_ETHACC_CODELET_ADD_MBUF_TO_Q_HEAD(ixEthAccCodeletFreeBufQ, mBufPtr);
    }

    return (IX_SUCCESS);
}


/**
 * Function definition: ixEthAccCodeletMemPoolFree()
 *
 * Free Ethernet Access Codelet MBUF pool
 *
 */

IX_STATUS ixEthAccCodeletMemPoolFree(void)
{
    IX_OSAL_MBUF *mBufPtr = NULL;

    if (IX_ETHACC_CODELET_IS_Q_EMPTY(ixEthAccCodeletFreeBufQ))
    {
	printf("ixEthAccCodeletMemPoolFree: Free Buffer Queue is empty!\n");
	return(IX_FAIL);
    }

    while (ixEthAccCodeletFreeBufQ != NULL)
    {
	IX_ETHACC_CODELET_REMOVE_MBUF_FROM_Q_HEAD (ixEthAccCodeletFreeBufQ, mBufPtr);
	IX_OSAL_MBUF_POOL_PUT (mBufPtr);
    }

    if (IX_SUCCESS != IX_OSAL_MBUF_POOL_UNINIT(ixEthAccCodeletMBufPool))
    {
	printf("ixEthAccCodeletMemPoolFree: Failed to free memory pool\n");
	return(IX_FAIL);
    }

    return(IX_SUCCESS);
}


/*
 * Function definition: ixEthAccCodeletMbufChainSizeSet()
 *
 * Reset the size of each mbuf of the chain to the allocated size
 */

void ixEthAccCodeletMbufChainSizeSet(IX_OSAL_MBUF *mBufPtr)
{
    while (mBufPtr)
    {
	/* reset the mBuf length to the allocated value */
	IX_OSAL_MBUF_MLEN(mBufPtr) = IX_ETHACC_CODELET_PCK_SIZE;
	/* move to the next element of the chain */
	mBufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mBufPtr);
    }
}

/**
 * Function definition: ixEthAccCodeletReplenishBuffers()
 *
 * Function to replenish buffers into the Rx free queue. This
 * is done at initialisation in order to receive the first frames.
 */

IX_STATUS ixEthAccCodeletReplenishBuffers(IxEthAccPortId portNo,
					  UINT32 num)
{
    IX_OSAL_MBUF *mBufPtr;
    UINT32 numBufs;

    for(numBufs=0; numBufs<num; numBufs++)
    {
	if(IX_ETHACC_CODELET_IS_Q_EMPTY(ixEthAccCodeletFreeBufQ))
	{
	    printf("BufMan: Buffer queue empty."
		   "Not enough free buffers for port %d!\n", portNo);
	    return (IX_FAIL);
	}
	
	IX_ETHACC_CODELET_REMOVE_MBUF_FROM_Q_HEAD(ixEthAccCodeletFreeBufQ, mBufPtr);
	IX_OSAL_MBUF_MLEN(mBufPtr) = IX_ETHACC_CODELET_PCK_SIZE;
	IX_OSAL_MBUF_PKT_LEN(mBufPtr) = IX_ETHACC_CODELET_PCK_SIZE;

	IX_OSAL_CACHE_INVALIDATE(IX_OSAL_MBUF_MDATA(mBufPtr), 
				     IX_OSAL_MBUF_MLEN(mBufPtr));
	
	if(ixEthAccPortRxFreeReplenish(portNo, mBufPtr) != IX_SUCCESS)
	{
	    printf("BufMan: Error replenishing free queue on port %d\n", portNo);
	    return (IX_FAIL);
	}
    }

    printf("Port %d Rx Free pool has %d buffers\n", portNo, numBufs);
    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletMemPoolFreeRxCb()
 *
 * This callback is used at the end of each ethernet operation.
 * It recovers the buffers used and places them back in the free queue.
 * This way the MBUF pool is free to be used by another ethernet operation.
 *
 */

void ixEthAccCodeletMemPoolFreeRxCB(UINT32 cbTag, 
				    IX_OSAL_MBUF* mBufPtr, 
				    UINT32 reserved)
{
    IX_OSAL_MBUF* mNextPtr;
                               
    while (mBufPtr)
    {
	mNextPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mBufPtr);
        /* return the buffer to the free list */
        IX_ETHACC_CODELET_ADD_MBUF_TO_Q_HEAD(ixEthAccCodeletFreeBufQ, mBufPtr);
        /* move to the next buffer */
	mBufPtr = mNextPtr;
    }
}

/*
 * Function definition: ixEthAccCodeletMultiBufferMemPoolFreeRxCb()
 *
 * This callback is used at the end of each ethernet operation.
 * It recovers the buffers used and places them back in the free queue.
 * This way the MBUF pool is free to be used by another ethernet operation.
 *
 */

void ixEthAccCodeletMultiBufferMemPoolFreeRxCB(UINT32 cbTag, 
				    IX_OSAL_MBUF** mBufPtr)
{

    while (*mBufPtr)
    {
        /* return the buffer (possibly chained) to the free list */
	ixEthAccCodeletMemPoolFreeRxCB(cbTag, 
				       *mBufPtr, 
				       IX_ETH_DB_UNKNOWN_PORT);
        /* next buffer in the array, which is NULL terminated */
	mBufPtr++;
    }
}

/*
 * Function definition: ixEthAccCodeletMemPoolFreeTxCb()
 *
 * This callback is used at the end of each ethernet operation.
 * It recovers the buffers used and places them back in the free queue.
 * This way the MBUF pool is free to be used by another ethernet 
 * operation.
 *
 */

void ixEthAccCodeletMemPoolFreeTxCB(UINT32 cbTag, IX_OSAL_MBUF* mBufPtr)
{
    IX_OSAL_MBUF* mNextPtr;
                               
    while (mBufPtr)
    {
        mNextPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mBufPtr);
        /* return the buffer to the free list */
        IX_ETHACC_CODELET_ADD_MBUF_TO_Q_HEAD(ixEthAccCodeletFreeBufQ, mBufPtr);
        /* move to the next buffer */
        mBufPtr = mNextPtr;
    }
}

/**
 * Function definition: ixEthAccCodeletRecoverBuffers()
 *
 * Recover used buffers (wait until the dispatcher completes 
 * the reception of pending taffic by watching the pool 
 * level).
 *
 */

IX_STATUS ixEthAccCodeletRecoverBuffers(void)
{
    unsigned int count = 0;

    printf("Recovering buffers...\n");

    /* wait 1 second for all mbufs to be stored in linked list */
    while ((count++ < 10) &&
           (IX_ETHACC_CODELET_MBUF_POOL_SIZE != ixEthAccCodeletFreeCount))
    {
        ixOsalSleep(100); /* 100 millisecs */
    }

    printf("%d buffers accounted for\n", ixEthAccCodeletFreeCount);

    if (IX_ETHACC_CODELET_MBUF_POOL_SIZE != ixEthAccCodeletFreeCount)
    {
	return (IX_FAIL);
    }
     
    return (IX_SUCCESS);
}


