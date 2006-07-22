/**
 * @file        IxEthAccCodeletBufMan.c
 * @author 	Intel Corporation
 * @date        25-08-2004
 *
 * @brief This file contains the implementation of the Ethernet Access 
 * Codelet that implements a buffer management system for the codelet
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


