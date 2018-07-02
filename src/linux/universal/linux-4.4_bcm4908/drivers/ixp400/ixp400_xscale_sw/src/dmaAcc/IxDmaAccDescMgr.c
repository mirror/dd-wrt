/*
 * @file    IxDmaAccDescMgr.c
 *
 * @date    18 October 2002
 *
 * @brief   IXP400 DMA Access Driver Component(IxDma) allocate and initialize
 *          descriptor pool
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
 * User defined include files listed below
 */
#include "IxDmaAcc.h"
#include "IxDmaAcc_p.h"
#include "IxOsal.h"
/*
 * System defined include files listed below
 */
/*
 *  Local Variables
 */
static IxDmaDescriptorPool ixDmaDescriptorPool;/**< Instantiation of the
                                              descriptor pool structure */
static BOOL ixDmaPoolInitDone = FALSE;     /**< PoolInitFlag initialised to
                                                value of false */
static IxOsalMutex dmaDescMgmtLock;            /**< Protect critical sections
                                                in this module */
static UINT8 *ixDmaRawMemoryPtr = NULL;   /**< Pointer to memory allocation for
                                               descriptor pool */
                                               
static UINT32 ixDmaLock;                                               

PRIVATE UINT8 *ixDmaAccDescRawMemoryPtr = NULL;


/***********************************************************************
 * @fn ixDmaAccDescriptorPoolInit();
 * @param None
 * @brief Allocate and initialise descriptor pool
 * @return IX_SUCCESS : Initialisation succeeded
 * @return IX_FAIL : Initialisation Failed : done already or malloc
 *                   not succesful
 ***********************************************************************/
PUBLIC IX_STATUS
ixDmaAccDescriptorPoolInit(void)
{
    UINT32 count;          /* Index for For-Loop */
    UINT32 RawMemorySize;  /* Temporary storage for size of descriptor pool */
    UINT32 DescriptorSize; /* Temporary storage for size of descriptor */
    IX_STATUS status=IX_FAIL;          /* Default return value to IX_FAIL */

    
    /*  Exit this function if Pool Init has been done before */
    if(FALSE == ixDmaPoolInitDone)
    {

        if (IX_SUCCESS != ixOsalMutexInit (&dmaDescMgmtLock))
        {
            /* Log error message in debugging mode */
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            		   IX_OSAL_LOG_DEV_STDERR,
                       "Descriptor mutex initialization failed.\n",
                       0, 0, 0, 0, 0, 0);

            /* Mutex initialization failed */
            return IX_FAIL;
        } /* end of if (ixOsalMutexInit) */

       
        /*  get the size of one descriptor */
        DescriptorSize = sizeof(IxDmaNpeQDescriptor);

        /*  calculate the actual memory required to store the descriptors */
        RawMemorySize=(IX_DMA_MAX_REQUEST*DescriptorSize);

        /* get the memory required for the descriptors */
        ixDmaRawMemoryPtr = (UINT8 *)
                        IX_OSAL_CACHE_DMA_MALLOC(RawMemorySize);

        ixDmaAccDescRawMemoryPtr = ixDmaRawMemoryPtr;

        /* get the memory for the pool of pointers to the descriptors */
        ixDmaDescriptorPool.pDmaNpeQDescriptor
                        = (IxDmaNpeQDescriptor **) IX_OSAL_CACHE_DMA_MALLOC(
                          IX_DMA_MAX_REQUEST*sizeof(IxDmaNpeQDescriptor*));

        if (NULL != (ixDmaDescriptorPool.pDmaNpeQDescriptor)
                        && (ixDmaRawMemoryPtr != NULL))
        {
            /* Zero the space allocated for buffer that holds pointers to
               descriptors */
            ixOsalMemSet( ixDmaDescriptorPool.pDmaNpeQDescriptor,
                    0,
                    IX_DMA_MAX_REQUEST*sizeof(IxDmaNpeQDescriptor*));
            /* Zero the space allocated for descriptor storage */
            ixOsalMemSet( ixDmaRawMemoryPtr,
                    0,
                    RawMemorySize);
            /* Assign the allocated memory for descriptors to the buffer */
            for(count = 0; count < IX_DMA_MAX_REQUEST; count++)
            {
                ixDmaDescriptorPool.pDmaNpeQDescriptor[count]
                      = (IxDmaNpeQDescriptor *) ixDmaRawMemoryPtr;
                ixDmaRawMemoryPtr += DescriptorSize;
            } /* end of for(count) */
            /* Initialize the buffer parameter Size */
	        ixDmaDescriptorPool.size = IX_DMA_MAX_REQUEST;
            /* Initialize the ring buffer head index */
	        ixDmaDescriptorPool.head = 0;
            /* Initialize the ring buffer tail index */
	        ixDmaDescriptorPool.tail = 0;
            /* Initialize the allocated descriptor counter */
	        ixDmaDescriptorPool.allocatedCnt = 0;

            /* Set flag to prevent initialisation occuring a second time */
	        ixDmaPoolInitDone = TRUE;
            
           
           /* Set return status to Success */
            status = IX_SUCCESS;
            
      } /* end of if ((ixDmaDescriptorPool.pDmaNpeQDescriptor != NULL)
             && (ixDmaRawMemoryPtr != NULL))*/
    } /* end of if(!ixDmaPoolInitDone) */
    return (status);
} /* end of function ixDmaAccDescriptorPoolInit(void) */


IX_STATUS ixDmaAccDescriptorPoolUninit(void)
{
    if(TRUE == ixDmaPoolInitDone)
    {
        IX_OSAL_CACHE_DMA_FREE(ixDmaAccDescRawMemoryPtr);
        IX_OSAL_CACHE_DMA_FREE (ixDmaDescriptorPool.pDmaNpeQDescriptor);
        ixOsalMutexDestroy(&dmaDescMgmtLock);
        ixDmaPoolInitDone = FALSE;
        ixDmaAccDescRawMemoryPtr = NULL;
    }
    return IX_SUCCESS;
}


/***********************************************************************
 * @fn ixDmaAccDescriptorGet();
 * @param IxDmaNpeQDescriptor **pDescriptor
 * @brief Return pointer to descriptor entry
 * @return IX_DMA_DM_SUCCESS Descriptor pointer succesfully returned
 * @return IX_DMA_DM_FAIL Invalid descriptor address
 * @return IX_DMA_DM_FIFO_FULL Descriptor get request when FIFO is full
 * @return IX_DMA_DM_INDEX_CORRUPTED Index for Desc Pool corrupted
 * Descriptor pointer pDescriptor is a local variable defined in
 * ixDmaAccDmaTransfer which should not be NULL
 ***********************************************************************/
IxDmaDescMgrStatus
ixDmaAccDescriptorGet(IxDmaNpeQDescriptor **pDescriptor)
{
    /* Lock critical section that is using descriptor pool */
       ixDmaLock=ixOsalIrqLock();

    /* Check if there are no more free descriptors to give */
    if (ixDmaDescriptorPool.allocatedCnt >= ixDmaDescriptorPool.size)
    {
        /* Unlock critical section that is using descriptor pool */
         ixOsalIrqUnlock(ixDmaLock);
       
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
        		   "\nixDmaAccDescriptorGet : Descriptor pool full.",
        		   0,0,0,0,0,0);
        return IX_DMA_DM_FIFO_FULL;
    } /* end of if (ixDmaDescriptorPool.allocatedCnt) */

    /* Check for illegal index value in head */
    if(ixDmaDescriptorPool.head >= ixDmaDescriptorPool.size)
    {
        /* Unlock critical section that is using descriptor pool */
        ixOsalIrqUnlock(ixDmaLock);

        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
        		   "\nixDmaAccDescriptorGet : Descriptor pool buffer index corrupted.",
        		   0,0,0,0,0,0);
        return IX_DMA_DM_INDEX_CORRUPTED;
    } /* end of if(ixDmaDescriptorPool.head) */

    /* Increment descriptors allocated counter */
    ixDmaDescriptorPool.allocatedCnt++;

    /* Assign to pDescriptor : a pointer to an available descriptor */
    *pDescriptor =
        ixDmaDescriptorPool.pDmaNpeQDescriptor[ixDmaDescriptorPool.head];
   
    /* Increment the descriptor pool head */
    if(ixDmaDescriptorPool.head < ixDmaDescriptorPool.size)
    {
        ixDmaDescriptorPool.head++;
    } /* end of if(ixDmaDescriptorPool.head) */

    /* Descriptor pool head rolls over to 0 when exceeds pool size */
    if(ixDmaDescriptorPool.head == ixDmaDescriptorPool.size)
    {
        ixDmaDescriptorPool.head=0;
    } /* end of if(ixDmaDescriptorPool.head) */

    /* Unlock critical section that is using descriptor pool */
    ixOsalIrqUnlock(ixDmaLock);
    
    return(IX_DMA_DM_SUCCESS);
} /* end of function ixDmaAccDescriptorGet */

/***********************************************************************
 * @fn ixDmaAccDescriptorFree();
 * @param IxDmaNpeQDescriptor *pDescriptor
 * @brief Free the oldest descriptor entry in the pool
 * @return IX_DMA_DM_SUCCESS Descriptor pointer succesfully freed
 * @return IX_DMA_DM_FAIL Invalid descriptor address
 * @return IX_DMA_DM_FIFO_EMPTY Descriptor free request when FIFO is empty
 * @return IX_DMA_DM_INDEX_CORRUPTED Index for Desc Pool corrupted
 * Check for Null Descriptor Pointer pDescriptor is done in ixDmaTransferDoneCallback
 ***********************************************************************/

IxDmaDescMgrStatus
ixDmaAccDescriptorFree(IxDmaNpeQDescriptor *pDescriptor)
{
    /* Lock critical section that is using descriptor pool */
      ixDmaLock=ixOsalIrqLock();

    /* Check if there are no descriptors to free */
    if (ixDmaDescriptorPool.allocatedCnt < 1)
    {
        /* Unlock critical section that is using descriptor pool */
        ixOsalIrqUnlock(ixDmaLock);

        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
        		   "\nixDmaAccDescriptorFree : No descriptors to free",
        		   0,0,0,0,0,0);
        return IX_DMA_DM_FIFO_EMPTY;
    }

    /* Check for illegal index value in tail */
    /* Check that returned descriptor is the oldest entry in the queue */
    if((ixDmaDescriptorPool.tail >= ixDmaDescriptorPool.size) || 
       (pDescriptor !=ixDmaDescriptorPool.pDmaNpeQDescriptor[ixDmaDescriptorPool.tail]))
    {
        /* Unlock critical section that is using descriptor pool */
        ixOsalIrqUnlock(ixDmaLock);
        
        /* Log error message in debugging mode */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        		   IX_OSAL_LOG_DEV_STDERR,
        		   "\nixDmaAccDescriptorFree : Descriptor pool index corrupted",
        		   0,0,0,0,0,0);
        return IX_DMA_DM_INDEX_CORRUPTED;
    } /* end if(dixDmaDescriptorPool.tail) */

    /* Decrement descriptors allocated counter */
    ixDmaDescriptorPool.allocatedCnt--;

    /* Increment the descriptor pool tail */
    if(ixDmaDescriptorPool.tail < ixDmaDescriptorPool.size)
    {
        ixDmaDescriptorPool.tail++;
    }

    /* Descriptor pool tail rolls over to 0 when exceeds pool size */
    if(ixDmaDescriptorPool.tail == ixDmaDescriptorPool.size)
    {
        ixDmaDescriptorPool.tail=0;
    }
    /* Unlock critical section that is using descriptor pool */
       ixOsalIrqUnlock(ixDmaLock); 

    return(IX_DMA_DM_SUCCESS);
} /* end of function ixDmaAccDescriptorFree */

/***********************************************************************
 * @fn ixDmaAccDescPoolShow();
 * @param none
 * @brief Show descriptor pool statistics
 * @return none
 ***********************************************************************/
void
ixDmaAccDescPoolShow(void)
{
    UINT32 max=ixDmaDescriptorPool.size;  /* Maximum number of entries for
                                             descriptor pool */
    UINT32 count=0;                       /* Counter for for loop */
    IxDmaNpeQDescriptor *descriptorPtr;   /* Temporary descriptor pointer */
    /* If descriptor pool is not initialized then show no statistics */
    if(TRUE == ixDmaPoolInitDone)
    {   
        printf("\nThe statistic of the descriptor pool is given as follow:\n");

        printf("\nDMA Descriptor Pool Statistics");
        printf("\n------------------------------\n");
        /* Show content of each descriptor in the pool */
       
  
        for (count=0;count<max;count++)
        {
            descriptorPtr=ixDmaDescriptorPool.pDmaNpeQDescriptor[count];
            printf("\nDescriptor #%d",count);
            printf("  Source : %x",
                    descriptorPtr->sourceAddress);
            printf("  Dest : %x",
                    descriptorPtr->destinationAddress);
            printf("  Mode : %x",
                    descriptorPtr->operationMode);
        }  /* end for(count) */
        
       
	printf("\nOverall Statistic for the descriptor pool:\n");
	printf("\n------------------------------\n");
	/* Show statistics for the descriptor pool */
	printf("\nBuffer Head  : %d",ixDmaDescriptorPool.head);
	printf("    Tail : %d",ixDmaDescriptorPool.tail);
	printf("\nDMA descriptors available : %d",
               (ixDmaDescriptorPool.size-ixDmaDescriptorPool.allocatedCnt));
	printf("\nDMA descriptors used      : %d",
               (ixDmaDescriptorPool.allocatedCnt));
	printf("\nDMA descriptors pool size : %d",
               (ixDmaDescriptorPool.size));
        
    }
    else
    {
        printf("\nDescriptor pool is not initialized.");
    } /* end of if-else (ixDmaPoolInitDone) */
    
   
    
    return;
} /* end of function ixDmaAccDescPoolShow() */


/* end of file IxDmaAccDescMgr.c */

















