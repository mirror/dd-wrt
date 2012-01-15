
/**
 * @file IxHssAccPDM.c
 *
 * @author Intel Corporation
 * @date 14 Dec 2001
 *
 * @brief This file contains the implementation of the private API for 
 * the Packetised Descriptor Manager.
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
*/

/*
 * Put the user defined include files required.
 */
#include "IxOsal.h"
#include "IxHssAccPDM_p.h"
#include "IxHssAcc.h"
#include "IxHssAccError_p.h"
#include "IxHssAccCommon_p.h"

/*
 * #defines and macros used in this file.
 */
#define IX_HSSACC_PKT_NUM_DESCS_PER_POOL   16
#define IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT 2
#define IX_HSSACC_PKT_NUM_POOLS_PER_HSS (IX_HSSACC_HDLC_PORT_MAX *\
                                        IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT)
#define IX_HSSACC_PKT_NUM_DESCS_PER_HSS (IX_HSSACC_PKT_NUM_DESCS_PER_POOL *\
                                        IX_HSSACC_PKT_NUM_POOLS_PER_HSS)
#define IX_HSSACC_PKT_MAX_NUM_POOLS  (IX_HSSACC_HSS_PORT_MAX *\
                                     IX_HSSACC_PKT_NUM_POOLS_PER_HSS)
#define IX_HSSACC_PKT_MAX_NUM_DESCS (IX_HSSACC_PKT_NUM_DESCS_PER_POOL *\
                                     IX_HSSACC_PKT_MAX_NUM_POOLS)


/*
 * Typedefs whose scope is limited to this file.
 */
typedef struct
{
    unsigned gets;
    unsigned frees;
    unsigned maxInUse;
    unsigned errs;
} IxHssAccPDMStats;

typedef struct
{
    IxHssAccPDMStats stats;
    unsigned freeIndex;
    IxHssAccPDMDescriptor *descPool[IX_HSSACC_PKT_NUM_DESCS_PER_POOL]; 
} IxHssAccPDMPoolInfo;

/*
 * Variable declarations global to this file only.  
   Externs are followed by static variables.
 */
IxHssAccPDMPoolInfo ixHssAccPDMPoolInfo[IX_HSSACC_PKT_MAX_NUM_POOLS];
static UINT32 maxNumPools = IX_HSSACC_PKT_NUM_POOLS_PER_HSS; /* assume only HSS port 0 
								is available */


/*
 * Flag used by IxHssAccPDMInit and IxHssAccPDMUninit functions
 */
PRIVATE BOOL ixHssAccMemoryAllocdAndChecked = FALSE;
/*
 * Pointer to memory allocated by IxHssAccPDMInit()
 */
PRIVATE UINT8 *ixHssAccMallocData           = NULL;



/**
 * Static function prototypes.
 */
PRIVATE void 
ixHssAccPDMPoolStatsInit (unsigned index1);

/**
 * Function definition: ixHssAccPDMPoolStatsInit
 */
PRIVATE void 
ixHssAccPDMPoolStatsInit (unsigned index1)
{
    ixOsalMemSet (&(ixHssAccPDMPoolInfo[index1].stats), 0, 
		  sizeof (IxHssAccPDMStats));
}

/**
 * Function definition: ixHssAccPDMInit 
 */
IX_STATUS
ixHssAccPDMInit ()
{
    unsigned index1;
    unsigned count;
    unsigned descMallocSize = sizeof (IxHssAccPDMDescriptor);
    unsigned totalMallocSize;
    static BOOL memoryAllocdAndChecked = FALSE;
    UINT32 maxNumDescs;
    UINT8 *mallocData;
    UINT8 *nextDescAddr;
    IxHssAccPDMPoolInfo *poolInfo = &ixHssAccPDMPoolInfo[0];
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering" 
		      " ixHssAccPDMInit \n");
   

    memoryAllocdAndChecked = ixHssAccMemoryAllocdAndChecked;

    /* create the memory first */
    if (!memoryAllocdAndChecked)
    {
	/* each descriptor will need to be cache page aligned */
	if ((descMallocSize % IX_OSAL_CACHE_LINE_SIZE) != 0)
	{
	    descMallocSize += (IX_OSAL_CACHE_LINE_SIZE - 
				 (descMallocSize % IX_OSAL_CACHE_LINE_SIZE));
	}

	if (hssPortMax == IX_HSSACC_SINGLE_HSS_PORT) /* allocating mem for HSS port 0 only */
	{
	    maxNumDescs = IX_HSSACC_PKT_NUM_DESCS_PER_HSS;
	    maxNumPools = IX_HSSACC_PKT_NUM_POOLS_PER_HSS;
	}
	else /* allocating mem for both HSS ports */
	{
	    maxNumDescs = IX_HSSACC_PKT_MAX_NUM_DESCS;
	    maxNumPools = IX_HSSACC_PKT_MAX_NUM_POOLS;
	}

	totalMallocSize = descMallocSize * maxNumDescs;
	mallocData = (UINT8 *)IX_HSSACC_PKT_DRV_DMA_MALLOC(totalMallocSize);

	/* check that valid memory was received */
	if (mallocData == NULL)
	{
	    IX_HSSACC_REPORT_ERROR ("ixHssAccPDMInit:"
				    "Failed to get memory for descriptor pool\n");
	    return IX_HSSACC_RESOURCE_ERR;
	}

    ixHssAccMallocData = mallocData;


	nextDescAddr = mallocData;
	for (index1 = 0; index1 < maxNumPools; index1++)
	{
	    /* initialise pool stats */
	    ixHssAccPDMPoolStatsInit (index1);

	    /* initialise the pool info values */
	    for (count = 0; count < IX_HSSACC_PKT_NUM_DESCS_PER_POOL; count++)
	    {
		poolInfo->descPool[count] = (IxHssAccPDMDescriptor *) nextDescAddr;
		poolInfo->descPool[count]->descIndex = count; /* used in freeing */
		poolInfo->descPool[count]->descInUse = FALSE;
		poolInfo->descPool[count]->hssPortId = 
		    index1 / IX_HSSACC_PKT_NUM_POOLS_PER_HSS;
		poolInfo->descPool[count]->hdlcPortId = 
		    (index1 / IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT) % IX_HSSACC_HDLC_PORT_MAX;
		nextDescAddr += descMallocSize;
		
	    }
	    poolInfo->freeIndex = 0;
	    poolInfo++;   
	}

	memoryAllocdAndChecked = TRUE;

    ixHssAccMemoryAllocdAndChecked = memoryAllocdAndChecked;

    }
    else
    {
	/* may want to do a PDMInit a second time - just init desc contents */
	for (index1 = 0, poolInfo = &ixHssAccPDMPoolInfo[0]; 
	     index1 < maxNumPools; index1++)
	{
	    /* initialise pool stats */
	    ixHssAccPDMPoolStatsInit (index1);

	    /* initialise the pool info values */
	    for (count = 0; count < IX_HSSACC_PKT_NUM_DESCS_PER_POOL; count++)
	    {
		poolInfo->descPool[count]->descInUse = FALSE;
		poolInfo->descPool[count]->hssPortId = 
		    index1 / IX_HSSACC_PKT_NUM_POOLS_PER_HSS;
		poolInfo->descPool[count]->hdlcPortId = 
		    (index1 / IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT) % IX_HSSACC_HDLC_PORT_MAX;
		
	    }
	    poolInfo->freeIndex = 0;
	    poolInfo++;   
	}
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
		      "ixHssAccPDMInit \n");
    return status;
}



/**
 * Function definition: ixHssAccPDMUninit
 */
IX_STATUS
ixHssAccPDMUninit (void)
{
    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering" " ixHssAccPDMUninit \n");

    /* check if uninitialised already done first */
    if (ixHssAccMemoryAllocdAndChecked)
    {
        /* Free the memory alloted by initialisation*/
        IX_OSAL_CACHE_DMA_FREE (ixHssAccMallocData);
        ixHssAccMallocData = NULL;

        /* Reset the flag to FALSE */
        ixHssAccMemoryAllocdAndChecked = FALSE;
    }
    else
    {
        IX_HSSACC_REPORT_ERROR ("ixHssAccPDMUninit:" "Called with "
                                " ixHssAccMemoryAllocdAndChecked == FALSE\n");
        return IX_FAIL;
    }

    IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting " "ixHssAccPDMUninit \n");
    return IX_SUCCESS;
}



/**
 * Function definition: ixHssAccPDMDescGet 
 */
IX_STATUS 
ixHssAccPDMDescGet (IxHssAccHssPort hssPortId, 
		    IxHssAccHdlcPort hdlcPortId, 
		    IxHssAccPDMPoolType poolType,
		    IxHssAccPDMDescriptor **desc)
{
    /* This function may be called from within an ISR */
    unsigned poolIndex = (hssPortId * IX_HSSACC_PKT_NUM_POOLS_PER_HSS) +
	(hdlcPortId * IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT) + poolType;
    IxHssAccPDMPoolInfo *poolInfo = &ixHssAccPDMPoolInfo[poolIndex];
    unsigned descsInUse;
    IX_STATUS status = IX_SUCCESS;
    IX_HSSACC_DEBUG_OFF (char errMsg[120]);
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering"
					   " ixHssAccPDMDescGet\n"));

    /*
     * Check that the freeIndexs dereference is "free" 
     * if so set the desc parameter to point to the free ptr 
     */
    if (!(poolInfo->descPool[poolInfo->freeIndex]->descInUse))
    {
	/*
	 * Descriptor pools are statically allocated for each client. 
	 * The hssPortId, the hdlcPortId and the poolType are used to 
	 * index into the array of pools.
	 */
	*desc = poolInfo->descPool[poolInfo->freeIndex];
	(*desc)->descInUse = TRUE;
	poolInfo->stats.gets++;
	descsInUse = poolInfo->stats.gets - poolInfo->stats.frees;
	/* Update the Max value if maxInUse has been broken*/
	if (poolInfo->stats.maxInUse < descsInUse)
	{
	    poolInfo->stats.maxInUse = descsInUse;
	}
       /*
	* Is the freeIndex at the end of the array? 
	* if not, just increment it, if it is, 
	* move freeIndex to start of the array 
	* Do this by calculating the base address of the 
	* array to the current pointer 
	* and comparing it to the total num of descriptors in the pool.
	*/
	poolInfo->freeIndex = ++(poolInfo->freeIndex) % 
	    IX_HSSACC_PKT_NUM_DESCS_PER_POOL;
    }
    else
    {
	poolInfo->stats.errs++;
	IX_HSSACC_DEBUG_OFF (
	    IX_HSSACC_REPORT_ERROR_WITH_ARG ("ixHssAccPDMDescGet: freeIndex desc in use - "
					     "Hdlc[%d] - Out of descriptors\n",
					     hdlcPortId,
					     0, 0, 0, 0, 0));
	return IX_HSSACC_RESOURCE_ERR;
    }
    
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
					   "ixHssAccPDMDescGet\n"));
    return status;
}


/**
 * Function definition: ixHssAccPDMDescFree 
 */
void 
ixHssAccPDMDescFree (IxHssAccPDMDescriptor *desc, 
		     IxHssAccPDMPoolType poolType)
{
    /* This function may be called from within an ISR */
    unsigned int index1;
    unsigned poolIndex = (desc->hssPortId * IX_HSSACC_PKT_NUM_POOLS_PER_HSS) +
	(desc->hdlcPortId * IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT) + poolType;
    IxHssAccPDMPoolInfo *poolInfo = &ixHssAccPDMPoolInfo[poolIndex];
       
    /* Freeing of descriptors:
       When freeing descriptors it is important to consider that one 
       descriptor received contains a chain of mbufs, and these mbufs 
       will have come from a single descriptor each.  So rather than 
       freeing just a single descriptor it is necessary to free the 
       following 'chaincount' descriptors also. This done as follows:
       Go from the first desc index forward the num of mbuf that was 
       on the first desc as follows... 
       Start at the first desc pulled off the Q, the index of this in 
       the array is firstDescIndex and go forward the number of mbufs
       chained to that descriptor, marking free that number of descriptors,
       because that is the number of descriptors used by this packet.
       The mod in the array index is to enable wrap around */
    for (index1 = desc->descIndex;
	 index1 <= (desc->descIndex + (desc->npeDesc.chainCount)); index1++)
    {
	if (poolInfo->descPool[index1 % IX_HSSACC_PKT_NUM_DESCS_PER_POOL]->descInUse)
	{
	    poolInfo->descPool[index1 % IX_HSSACC_PKT_NUM_DESCS_PER_POOL]->descInUse = 
		FALSE;
	    poolInfo->stats.frees++;
	}
	else
	{
	    poolInfo->stats.errs++;
	}
    }    
}


/**
 * Function definition: ixHssAccPDMNumDescInUse
 */
unsigned 
ixHssAccPDMNumDescInUse (IxHssAccHssPort hssPortId, 
			 IxHssAccHdlcPort hdlcPortId)
{
    /* This function may be called from within an ISR */
    unsigned index1 = 0;
    unsigned descsInUse = 0;
    unsigned poolIndex = (hssPortId * IX_HSSACC_PKT_NUM_POOLS_PER_HSS) +
	(hdlcPortId * IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT);
    IxHssAccPDMPoolInfo *poolInfo = &ixHssAccPDMPoolInfo[poolIndex];
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Entering"
					   " ixHssAccPDMNumDescInUse\n"));
        
    /* While we havent gone past the max number of desc in the array AND  
     * the desc at this index isnt in use, AND the desc at the same index 
     * in the next pool arent in use increment the index by 1.  
     * The 2 adjacent pools belong to the same client, one for Tx and 1 for Rx.
     * So eventually we will either find an in use descriptor or we will 
     * reach the array end. */
    while (index1 < IX_HSSACC_PKT_NUM_DESCS_PER_POOL)
    {
	/* check tx pool */
	if (poolInfo->descPool[index1]->descInUse)
	{
	    descsInUse++;
	}
	/* check rx pool */
	if ((poolInfo + 1)->descPool[index1]->descInUse)
	{
	    descsInUse++;
	}

	index1++;
    }
    
    IX_HSSACC_DEBUG_OFF (IX_HSSACC_TRACE0 (IX_HSSACC_FN_ENTRY_EXIT, "Exiting "
					   "ixHssAccPDMNumDescInUse\n"));

    return descsInUse;
}

IX_OSAL_MBUF *
ixHssAccPDMMbufToNpeFormatConvert (IX_OSAL_MBUF *mbufPtr)
{
    IX_OSAL_MBUF *mbufChainPtr;
    IX_OSAL_MBUF *mbufTemp;
    int iterationCount;

    mbufChainPtr = mbufPtr;

    /* convert the pointer to the first mbuf */
    mbufPtr = (IX_OSAL_MBUF *) IX_HSSACC_PKT_MMU_VIRT_TO_PHY (mbufPtr);

    /* iterate through the mbuf chain */
    iterationCount = 0;
    while (mbufChainPtr)
    {
        iterationCount++;

        /* check about possible mbuf infinite chain or loop */

        /* save the next pointer */
        mbufTemp = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr);

        /* convert the current mbuf header */
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr) = (IX_OSAL_MBUF *)
            IX_HSSACC_PKT_MMU_VIRT_TO_PHY (
	        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr));
	
        IX_OSAL_MBUF_MDATA(mbufChainPtr) = (UINT8 *)
            IX_HSSACC_PKT_MMU_VIRT_TO_PHY (IX_OSAL_MBUF_MDATA(mbufChainPtr));

        /* endian conversion for the mbuf header */
	IX_HSSACC_ENDIAN_MBUF_SWAP(mbufChainPtr);

        /* force data to be stored in physical memory */
        IX_HSSACC_PKT_DATA_CACHE_FLUSH(mbufChainPtr, sizeof(*mbufChainPtr));

        /* next element */
        mbufChainPtr = mbufTemp;
    }
    return (mbufPtr);
}

IX_OSAL_MBUF *
ixHssAccPDMMbufFromNpeFormatConvert (IX_OSAL_MBUF *mbufPtr,
                                     BOOL invalidateCache)
{
    IX_OSAL_MBUF *mbufChainPtr;
    int iterationCount;

    /* convert the pointer to the first mbuf */
    mbufPtr = (IX_OSAL_MBUF *) IX_HSSACC_PKT_MMU_PHY_TO_VIRT (mbufPtr);

    mbufChainPtr = mbufPtr;
    /* iterate through the mbuf chain */
    iterationCount = 0;
    while (mbufChainPtr)
    {
        iterationCount++;

        /* check about possible mbuf infinite chain or loop */

        if (invalidateCache)
        {
            /* force data to be read from physical memory */
            IX_HSSACC_PKT_DATA_CACHE_INVALIDATE(mbufChainPtr, 
                                                sizeof(*mbufChainPtr));
        }

        /* endian conversion for the mbuf header */
	IX_HSSACC_ENDIAN_MBUF_SWAP(mbufChainPtr);

        /* convert the current mbuf header */
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr) =
	    (IX_OSAL_MBUF *) IX_HSSACC_PKT_MMU_PHY_TO_VIRT (
	        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr));
        IX_OSAL_MBUF_MDATA(mbufChainPtr) = 
	    (UINT8 *) IX_HSSACC_PKT_MMU_PHY_TO_VIRT (IX_OSAL_MBUF_MDATA(mbufChainPtr));
	
        /* next element */
        mbufChainPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufChainPtr);
    }
    return (mbufPtr);
}

/**
 * Function definition: ixHssAccPDMShow
 */
void ixHssAccPDMShow (void)
{
    IxHssAccHssPort hssPortId;
    IxHssAccHdlcPort hdlcPortId;
    IxHssAccPDMPoolType poolType;
    unsigned poolIndex;
    unsigned availIndex;
    unsigned avail = 0;
    IxHssAccPDMPoolInfo *poolInfo = &ixHssAccPDMPoolInfo[0];

    /* check that the pools have been setup */
    if (poolInfo->descPool[0] == NULL)
    {
	return;
    }

    printf ("\nixHssAccPDMShow:\n");
    printf ("     Pool           Total  Avail BlkSize  Freelist    Gets    Frees   MaxOut     Errs\n");
    printf ("=====================================================================================\n");

    for (poolIndex = 0; poolIndex < maxNumPools;
	poolIndex++, avail = 0)
    {
	hssPortId = (IxHssAccHssPort) (poolIndex / IX_HSSACC_PKT_NUM_POOLS_PER_HSS);
	hdlcPortId = (IxHssAccHdlcPort) ((poolIndex / IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT) %
                                          IX_HSSACC_HDLC_PORT_MAX);
	poolType = (IxHssAccPDMPoolType) (poolIndex % IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT);

	for (availIndex = 0; availIndex < IX_HSSACC_PKT_NUM_DESCS_PER_POOL;
	     availIndex++)
	{
	    if (poolInfo->descPool[availIndex]->descInUse == FALSE)
	    {
		avail++;
	    }
	}

	printf (" HSS[%d] HDLC[%d] %s %5d %5d",
		hssPortId,
		hdlcPortId,
		poolType == IX_HSSACC_PDM_TX_POOL ? "TX" : "RX",
		IX_HSSACC_PKT_NUM_DESCS_PER_POOL,
		avail);

	printf (" %7d    %p %8d %8d %8d %8d\n",
		(int) sizeof (IxHssAccPDMDescriptor),
#if ((CPU!=SIMSPARCSOLARIS) && (CPU!=SIMLINUX))
		poolInfo->descPool[poolInfo->freeIndex],
#else
		(UINT32 *) 0x0, /* don't want to see the addr on UT */
#endif
		poolInfo->stats.gets,
		poolInfo->stats.frees,
		poolInfo->stats.maxInUse,
		poolInfo->stats.errs);

	poolInfo++;
    }
}


/**
 * Function definition: ixHssAccPDMStatsInit
 */
void 
ixHssAccPDMStatsInit (IxHssAccHssPort hssPortId, 
		      IxHssAccHdlcPort hdlcPortId)
{
    unsigned i;
    unsigned poolIndex = (hssPortId * IX_HSSACC_PKT_NUM_POOLS_PER_HSS) + 
	(hdlcPortId * IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT);

    for (i = 0; i < IX_HSSACC_PKT_NUM_POOLS_PER_CLIENT; i++)
    {
	/* initialise all pools for this client */
	ixHssAccPDMPoolStatsInit (poolIndex + i);
    }
}
