/**
 * @file IxOsalBufferMgt.c
 *
 * @brief Default buffer pool management and buffer management
 *        Implementation.
 *
 * Design Notes:
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
 * OS may choose to use default bufferMgt by defining
 * IX_OSAL_USE_DEFAULT_BUFFER_MGT in IxOsalOsBufferMgt.h  
 */

#include "IxOsal.h"

#define IX_OSAL_BUFFER_FREE_PROTECTION  /* Define this to enable Illegal MBuf Freed Protection*/

/*
 * The implementation is only used when the following
 * is defined.
 */
#ifdef IX_OSAL_USE_DEFAULT_BUFFER_MGT


#define IX_OSAL_MBUF_SYS_SIGNATURE				(0x8BADF00D)
#define IX_OSAL_MBUF_SYS_SIGNATURE_MASK				(0xEFFFFFFF)
#define IX_OSAL_MBUF_USED_FLAG					(0x10000000)
#define IX_OSAL_MBUF_SYS_SIGNATURE_INIT(bufPtr)        		IX_OSAL_MBUF_SIGNATURE (bufPtr) = (UINT32)IX_OSAL_MBUF_SYS_SIGNATURE

/* 
*  This implementation is protect, the buffer pool management's  ixOsalMBufFree 
*  against an invalid MBUF pointer argument that already has been freed earlier 
*  or in other words resides in the free pool of MBUFs. This added feature, 
*  checks the MBUF "USED" FLAG. The Flag tells if the MBUF is still not freed 
*  back to the Buffer Pool.
*  Disable this feature for performance reasons by undef 
*  IX_OSAL_BUFFER_FREE_PROTECTION macro.
*/
#ifdef IX_OSAL_BUFFER_FREE_PROTECTION  /*IX_OSAL_BUFFER_FREE_PROTECTION With Buffer Free protection*/

#define IX_OSAL_MBUF_GET_SYS_SIGNATURE(bufPtr)		(IX_OSAL_MBUF_SIGNATURE (bufPtr)&(IX_OSAL_MBUF_SYS_SIGNATURE_MASK) )
#define IX_OSAL_MBUF_SET_SYS_SIGNATURE(bufPtr)    do {																											\
																									IX_OSAL_MBUF_SIGNATURE (bufPtr)&(~IX_OSAL_MBUF_SYS_SIGNATURE_MASK);\
														    									IX_OSAL_MBUF_SIGNATURE (bufPtr)|=IX_OSAL_MBUF_SYS_SIGNATURE;			\
																									}while(0)

#define IX_OSAL_MBUF_SET_USED_FLAG(bufPtr)   IX_OSAL_MBUF_SIGNATURE (bufPtr)|=IX_OSAL_MBUF_USED_FLAG
#define IX_OSAL_MBUF_CLEAR_USED_FLAG(bufPtr) IX_OSAL_MBUF_SIGNATURE (bufPtr)&=~IX_OSAL_MBUF_USED_FLAG
#define IX_OSAL_MBUF_ISSET_USED_FLAG(bufPtr) (IX_OSAL_MBUF_SIGNATURE (bufPtr)&IX_OSAL_MBUF_USED_FLAG)

#else

#define IX_OSAL_MBUF_GET_SYS_SIGNATURE(bufPtr)	 IX_OSAL_MBUF_SIGNATURE (bufPtr)
#define IX_OSAL_MBUF_SET_SYS_SIGNATURE(bufPtr)   IX_OSAL_MBUF_SIGNATURE (bufPtr) = IX_OSAL_MBUF_SYS_SIGNATURE

#endif /*IX_OSAL_BUFFER_FREE_PROTECTION With Buffer Free protection*/
/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

/* 
 * A unit of 32, used to provide bit-shift for pool
 * management. Needs some work if users want more than 32 pools.
 */
#define IX_OSAL_BUFF_FREE_BITS 32

PRIVATE UINT32 ixOsalBuffFreePools[IX_OSAL_MBUF_MAX_POOLS /
    IX_OSAL_BUFF_FREE_BITS];

PUBLIC IX_OSAL_MBUF_POOL ixOsalBuffPools[IX_OSAL_MBUF_MAX_POOLS];

PRIVATE INT32 ixOsalBuffPoolsInUse = 0;

PRIVATE BOOL ixOsalBuffMgmtInited=FALSE;

PRIVATE IxOsalMutex ixOsalBufferFreePoolsAccess;

#if defined(__linux_user) || defined(__freebsd_user)

PRIVATE IxOsalMutex ixOsalBufferPoolAccess[IX_OSAL_MBUF_MAX_POOLS];

#endif /* __linux_user || __freebsd_user */

#ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY
PRIVATE IX_OSAL_MBUF *
ixOsalBuffPoolMbufInit (UINT32 mbufSizeAligned,
                        UINT32 dataSizeAligned,
                        IX_OSAL_MBUF_POOL *poolPtr);
#endif

PRIVATE IX_OSAL_MBUF_POOL * ixOsalPoolAlloc (VOID);

/*
 * Function definition: ixOsalPoolAlloc
 */

/****************************/

PRIVATE IX_OSAL_MBUF_POOL *
ixOsalPoolAlloc (VOID)
{
    register UINT32 i = 0;

    if (ixOsalBuffMgmtInited == FALSE)
    {
	ixOsalBuffMgmtInited = TRUE;
	ixOsalMutexInit(&ixOsalBufferFreePoolsAccess);
    }
				
    /*
     * Scan for the first free buffer. Free buffers are indicated by 0
     * on the corrsponding bit in ixOsalBuffFreePools. 
     */
    if (ixOsalBuffPoolsInUse >= IX_OSAL_MBUF_MAX_POOLS)
    {
        /*
         * Fail to grab a ptr this time 
         */
        return NULL;
    }

		/*
		 * Lock the shared resource before accessing it
		 */
		ixOsalMutexLock(&ixOsalBufferFreePoolsAccess, IX_OSAL_WAIT_FOREVER);
		 
    while (ixOsalBuffFreePools[i / IX_OSAL_BUFF_FREE_BITS] &
        (1 << (i % IX_OSAL_BUFF_FREE_BITS)))
        i++;
		
    /*
     * Free buffer found. Mark it as busy and initialize. 
     */
    ixOsalBuffFreePools[i / IX_OSAL_BUFF_FREE_BITS] |=
        (1 << (i % IX_OSAL_BUFF_FREE_BITS));

		/*
		 * Unlock the shared resource after accessing it
		 */
		ixOsalMutexUnlock(&ixOsalBufferFreePoolsAccess);
		
    ixOsalMemSet (&ixOsalBuffPools[i], 0, sizeof (IX_OSAL_MBUF_POOL));

    ixOsalBuffPools[i].poolIdx = i;
    ixOsalBuffPoolsInUse++;

    return &ixOsalBuffPools[i];
}


#ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY

/*
 * mbufSize, would be made to align to cache boundary in kernel space,
 * similarly for dataSize.
 */
PRIVATE IX_OSAL_MBUF *
ixOsalBuffPoolMbufInit (UINT32 mbufSize,
                        UINT32 dataSize,
                        IX_OSAL_MBUF_POOL *poolPtr)
{
    UINT8 *dataPtr;
    IX_OSAL_MBUF *realMbufPtr;
    /* Allocate cache-aligned memory for mbuf header */
    
		realMbufPtr = (IX_OSAL_MBUF *) IX_OSAL_BUFF_MEM_ALLOC (mbufSize);
    IX_OSAL_ASSERT (realMbufPtr != NULL);
    ixOsalMemSet (realMbufPtr, 0, mbufSize);

    /* Allocate cache-aligned memory for mbuf data */
    
		dataPtr = (UINT8 *) IX_OSAL_BUFF_MEM_ALLOC (dataSize);
    IX_OSAL_ASSERT (dataPtr != NULL);
    ixOsalMemSet (dataPtr, 0, dataSize);

    /* Fill in mbuf header fields */
    IX_OSAL_MBUF_MDATA (realMbufPtr) = dataPtr;
    IX_OSAL_MBUF_ALLOCATED_BUFF_DATA (realMbufPtr) = (UINT32)dataPtr;

    IX_OSAL_MBUF_MLEN (realMbufPtr) = dataSize;
    IX_OSAL_MBUF_ALLOCATED_BUFF_LEN (realMbufPtr) = dataSize;

    IX_OSAL_MBUF_NET_POOL (realMbufPtr) = (IX_OSAL_MBUF_POOL *) poolPtr;

    IX_OSAL_MBUF_SYS_SIGNATURE_INIT(realMbufPtr);

    /* update some statistical information */
    poolPtr->mbufMemSize += mbufSize;
    poolPtr->dataMemSize += dataSize;

    return realMbufPtr;
}
#endif /* #ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY */

/*
 * Function definition: ixOsalBuffPoolInit
 */
PUBLIC IX_OSAL_MBUF_POOL *
ixOsalPoolInit (UINT32 count, UINT32 size, const char *name)
{

    /* These variables are only used if IX_OSAL_BUFFER_ALLOC_SEPARATELY
     * is defined .
     */
#ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY
				
    UINT32 i, mbufSize, dataSize;
    IX_OSAL_MBUF *currentMbufPtr = NULL;

#else /* IX_OSAL_BUFFER_ALLOC_SEPARATELY */

		VOID *poolBufPtr;
    VOID *poolDataPtr;
    INT32 mbufMemSize;
    INT32 dataMemSize;

#endif /* IX_OSAL_BUFFER_ALLOC_SEPARATELY */

    IX_OSAL_MBUF_POOL *poolPtr = NULL;

		if ((const UINT8 *)name == NULL)
		{
		
				ixOsalLog (IX_OSAL_LOG_LVL_WARNING,
                   IX_OSAL_LOG_DEV_STDOUT,
                   "ixOsalPoolInit(): Failed due to NULL name\n", 0, 0, 0, 0, 0, 0);

				return NULL;
		}
    
    if (count <= 0)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                   IX_OSAL_LOG_DEV_STDOUT,
                   "ixOsalPoolInit(): count = 0 \n", 0, 0, 0, 0, 0, 0);

        return NULL;        
    }

    
    if (strlen (name) > IX_OSAL_MBUF_POOL_NAME_LEN)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            			 IX_OSAL_LOG_DEV_STDOUT,
            			 "ixOsalPoolInit(): "
            			 "ERROR - name length should be no greater than %d  \n",
            			 IX_OSAL_MBUF_POOL_NAME_LEN, 0, 0, 0, 0, 0);

				return NULL;
    }

/* OS can choose whether to allocate all buffers all together (if it 
 * can handle a huge single alloc request), or to allocate buffers 
 * separately by the defining IX_OSAL_BUFFER_ALLOC_SEPARATELY.
 */
#ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY
    /* Get a pool Ptr */
    poolPtr = ixOsalPoolAlloc ();

#if defined(__linux_user) || defined(__freebsd_user)

    ixOsalMutexInit(&ixOsalBufferPoolAccess[poolPtr->poolIdx]);

#endif /* __linux_user || __freebsd_user */

	if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalPoolInit(): " "Fail to Get PoolPtr \n", 0, 0, 0, 0, 0, 0);    
        return NULL;
    }
		
		/*
		 * In Kernel space allocate cache aligned boundary. But in user
		 * space these macros would return the value of the input without
		 * any modification
		 */
		mbufSize = IX_OSAL_MBUF_POOL_SIZE_ALIGN (sizeof (IX_OSAL_MBUF));
    dataSize = IX_OSAL_MBUF_POOL_SIZE_ALIGN(size);
		
    poolPtr->nextFreeBuf = NULL;    
    poolPtr->mbufMemPtr = NULL;    
    poolPtr->dataMemPtr = NULL;
    poolPtr->bufDataSize = dataSize;
    poolPtr->totalBufsInPool = count;
    poolPtr->poolAllocType = IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC;
    strcpy (poolPtr->name, name);

    for (i = 0; i < count; i++)
    {
	    /* create an mbuf */
	    currentMbufPtr = ixOsalBuffPoolMbufInit (mbufSize,
					         														 dataSize,
					         														 poolPtr);

#ifdef IX_OSAL_BUFFER_FREE_PROTECTION
			
/* Set the Buffer USED Flag. If not, ixOsalMBufFree will fail.
   ixOsalMbufFree used here is in a special case whereby, it's 
   used to add MBUF to the Pool. By specification, ixOsalMbufFree 
   deallocates an allocated MBUF from Pool.
*/ 			         
      IX_OSAL_MBUF_SET_USED_FLAG(currentMbufPtr);
#endif                             
	    /* Add it to the pool */
	    ixOsalMbufFree (currentMbufPtr);
	    
			/* 
			 * Flush the pool information to RAM. If caching is done then 
			 * flush information explicitly
			 */
			IX_OSAL_BUFF_FLUSH_INFO(currentMbufPtr, mbufSize);
    }
    
    /*
     * update the number of free buffers in the pool 
     */
    poolPtr->freeBufsInPool = count;

#else 

		/* Otherwise allocate buffers in a continuous block fashion */    
    poolBufPtr = IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC (count, mbufMemSize);
    IX_OSAL_ASSERT (poolBufPtr != NULL);
    poolDataPtr = IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC (count, size, dataMemSize);
    IX_OSAL_ASSERT (poolDataPtr != NULL);

    poolPtr = ixOsalNoAllocPoolInit (poolBufPtr, poolDataPtr, count, size, name);

		if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            			 IX_OSAL_LOG_DEV_STDOUT,
            			 "ixOsalPoolInit(): " "Fail to get pool ptr \n", 0, 0, 0, 0, 0, 0);

				return NULL;
    }

    poolPtr->poolAllocType = IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC;

#endif /* IX_OSAL_BUFFER_ALLOC_SEPARATELY */
    
    return poolPtr;
}

PUBLIC IX_OSAL_MBUF_POOL *
ixOsalNoAllocPoolInit (VOID *poolBufPtr, VOID *poolDataPtr, UINT32 count, UINT32 size, const char *name)
{
    UINT32 i, mbufSize, dataSize;
    IX_OSAL_MBUF *currentMbufPtr = NULL;
    IX_OSAL_MBUF *nextMbufPtr = NULL;
    IX_OSAL_MBUF_POOL *poolPtr = NULL;

    /*
     * check parameters 
     */
    if (poolBufPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
			             IX_OSAL_LOG_DEV_STDOUT,
       			       "ixOsalNoAllocPoolInit(): "
            			 "ERROR - NULL poolBufPtr \n", 0, 0, 0, 0, 0, 0);
				
        return NULL;
    }
		
    if ((const UINT8 *)name == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            		   IX_OSAL_LOG_DEV_STDOUT,
            			 "ixOsalNoAllocPoolInit(): "
            			 "ERROR - NULL name ptr  \n", 0, 0, 0, 0, 0, 0);
        
				return NULL;
    }
		
    if (count <= 0)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            			 IX_OSAL_LOG_DEV_STDOUT,
             			 "ixOsalNoAllocPoolInit(): "
            			 "ERROR - count must > 0   \n", 0, 0, 0, 0, 0, 0);
				
        return NULL;
    }

    if (strlen (name) > IX_OSAL_MBUF_POOL_NAME_LEN)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalNoAllocPoolInit(): "
            "ERROR - name length should be no greater than %d  \n",
            IX_OSAL_MBUF_POOL_NAME_LEN, 0, 0, 0, 0, 0);
        return NULL;
    }

    poolPtr = ixOsalPoolAlloc ();

    if (poolPtr == NULL)
    {
        return NULL;
    }
    
#if defined(__linux_user) || defined(__freebsd_user)

    ixOsalMutexInit(&ixOsalBufferPoolAccess[poolPtr->poolIdx]);

#endif /* __linux_user || __freebsd_user */
        
    mbufSize = IX_OSAL_MBUF_POOL_SIZE_ALIGN (sizeof (IX_OSAL_MBUF));

    /*
     * Clear the mbuf memory area 
     */
    ixOsalMemSet (poolBufPtr, 0, mbufSize * count);

    if (poolDataPtr != NULL)
    {
        dataSize = IX_OSAL_MBUF_POOL_SIZE_ALIGN(size);
     
	/*
         * Clear the data memory area 
         */
        ixOsalMemSet (poolDataPtr, 0, dataSize * count);
    }
    else
    {
        dataSize = 0;
    }

    /*
     * Initialise pool fields 
     */
    strcpy ((poolPtr)->name, name);

    poolPtr->dataMemPtr = poolDataPtr;
    poolPtr->mbufMemPtr = poolBufPtr;
    poolPtr->bufDataSize = dataSize;
    poolPtr->totalBufsInPool = count;
    poolPtr->mbufMemSize = mbufSize * count;
    poolPtr->dataMemSize = dataSize * count;

    currentMbufPtr = (IX_OSAL_MBUF *) poolBufPtr;

    poolPtr->nextFreeBuf = currentMbufPtr;

    for (i = 0; i < count; i++)
    {
        if (i < (count - 1))
        {
            nextMbufPtr =
                (IX_OSAL_MBUF *) ((UINT32) currentMbufPtr + mbufSize);
        }
        else
        {                      
						/* last mbuf in chain */
            nextMbufPtr = NULL;
        }
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (currentMbufPtr) = nextMbufPtr;
        IX_OSAL_MBUF_NET_POOL (currentMbufPtr) = poolPtr;

        IX_OSAL_MBUF_SYS_SIGNATURE_INIT(currentMbufPtr);

        if (poolDataPtr != NULL)
        {
            IX_OSAL_MBUF_MDATA (currentMbufPtr) = poolDataPtr;
            IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(currentMbufPtr) = (UINT32) poolDataPtr;

            IX_OSAL_MBUF_MLEN (currentMbufPtr) = dataSize;
            IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(currentMbufPtr) = dataSize;

            poolDataPtr = (VOID *) ((UINT32) poolDataPtr + dataSize);
        }

        currentMbufPtr = nextMbufPtr;
    }

    /*
     * update the number of free buffers in the pool 
     */
    poolPtr->freeBufsInPool = count;

    poolPtr->poolAllocType = IX_OSAL_MBUF_POOL_TYPE_USER_ALLOC;

    return poolPtr;

}

/* 
 * Get a mbuf ptr from the pool
 */
PUBLIC IX_OSAL_MBUF *
ixOsalMbufAlloc (IX_OSAL_MBUF_POOL * poolPtr)
{

#if !defined(__linux_user) && !defined(__freebsd_user)
    INT32 lock;
#endif
		
    IX_OSAL_MBUF *newBufPtr = NULL;

    /*
     * check parameters 
     */
    if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMbufAlloc(): "
            "ERROR - Invalid Parameter\n", 0, 0, 0, 0, 0, 0);
        return NULL;
    }

#if !defined(__linux_user) && !defined(__freebsd_user)
    lock = ixOsalIrqLock ();
#else
		ixOsalMutexLock(&ixOsalBufferPoolAccess[poolPtr->poolIdx],IX_OSAL_WAIT_FOREVER);
#endif
		
    newBufPtr = poolPtr->nextFreeBuf;
		
    if (newBufPtr)
    {
        poolPtr->nextFreeBuf = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (newBufPtr);
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (newBufPtr) = NULL;

        /*
         * update the number of free buffers in the pool 
         */
        poolPtr->freeBufsInPool--;
    }
    else
    {
        /* Return NULL to indicate to caller that request is denied. */
			#if !defined(__linux_user) && !defined(__freebsd_user)
    		ixOsalIrqUnlock (lock);
			#else
				ixOsalMutexUnlock(&ixOsalBufferPoolAccess[poolPtr->poolIdx]);
			#endif      

        return NULL;
    }

#ifdef IX_OSAL_BUFFER_FREE_PROTECTION
	/* Set Buffer Used Flag to indicate state.*/
    IX_OSAL_MBUF_SET_USED_FLAG(newBufPtr);
#endif

#if !defined(__linux_user) && !defined(__freebsd_user)
    ixOsalIrqUnlock (lock);
#else
		ixOsalMutexUnlock(&ixOsalBufferPoolAccess[poolPtr->poolIdx]);
#endif
		
    return newBufPtr;
}

PUBLIC IX_OSAL_MBUF *
ixOsalMbufFree (IX_OSAL_MBUF * bufPtr)
{

#if !defined(__linux_user) && !defined(__freebsd_user)
    INT32 lock;
#else
    INT32 index;
#endif
		
    IX_OSAL_MBUF_POOL *poolPtr;

    IX_OSAL_MBUF *nextBufPtr = NULL;

    /*
     * check parameters 
     */
    if (bufPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            			 IX_OSAL_LOG_DEV_STDOUT,
            			 "ixOsalMbufFree(): "
            			 "ERROR - Invalid Parameter\n", 0, 0, 0, 0, 0, 0);

        return NULL;
    }

#if !defined(__linux_user) && !defined(__freebsd_user)
    lock = ixOsalIrqLock ();
#else
    index = (((IX_OSAL_MBUF_POOL *)(bufPtr->ix_ctrl.ix_pool))->poolIdx);
		ixOsalMutexLock(&ixOsalBufferPoolAccess[index],IX_OSAL_WAIT_FOREVER);
#endif

#ifdef IX_OSAL_BUFFER_FREE_PROTECTION
	
	/* Prevention for Buffer freed more than once*/
    if(!IX_OSAL_MBUF_ISSET_USED_FLAG(bufPtr))
    {
	   	return NULL;
    }
    IX_OSAL_MBUF_CLEAR_USED_FLAG(bufPtr);
#endif
	
    poolPtr = IX_OSAL_MBUF_NET_POOL (bufPtr);

    /*
     * check the mbuf wrapper signature (if mbuf wrapper was used) 
     */
    if (poolPtr->poolAllocType == IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC)
    {
        IX_OSAL_ENSURE ( (IX_OSAL_MBUF_GET_SYS_SIGNATURE(bufPtr) == IX_OSAL_MBUF_SYS_SIGNATURE),
            "ixOsalBuffPoolBufFree: ERROR - Invalid mbuf signature.");
    }

    nextBufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (bufPtr);

    IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (bufPtr) = poolPtr->nextFreeBuf;
		
    poolPtr->nextFreeBuf = bufPtr;

    /*
     * update the number of free buffers in the pool 
     */
    poolPtr->freeBufsInPool++;

#if !defined(__linux_user) && !defined(__freebsd_user)
    ixOsalIrqUnlock (lock);
#else
		ixOsalMutexUnlock(&ixOsalBufferPoolAccess[index]);
#endif

    return nextBufPtr;
}

PUBLIC VOID
ixOsalMbufChainFree (IX_OSAL_MBUF * bufPtr)
{
    while ((bufPtr = ixOsalMbufFree (bufPtr)));
}

/*
 * Function definition: ixOsalBuffPoolShow
 */
PUBLIC VOID
ixOsalMbufPoolShow (IX_OSAL_MBUF_POOL * poolPtr)
{
    IX_OSAL_MBUF *nextBufPtr;
    INT32 count = 0;

#if !defined(__linux_user) && !defined(__freebsd_user)
    INT32 lock;
#endif
		
    /*
     * check parameters 
     */
    if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalBuffPoolShow(): "
            "ERROR - Invalid Parameter", 0, 0, 0, 0, 0, 0);
        /*
         * return IX_FAIL; 
         */
        return;
    }

#if !defined(__linux_user) && !defined(__freebsd_user)
    lock = ixOsalIrqLock ();
#else
		ixOsalMutexLock(&ixOsalBufferPoolAccess[poolPtr->poolIdx],IX_OSAL_WAIT_FOREVER);
#endif

		count = poolPtr->freeBufsInPool;
    nextBufPtr = poolPtr->nextFreeBuf;
    
#if !defined(__linux_user) && !defined(__freebsd_user)
    ixOsalIrqUnlock (lock);
#else
		ixOsalMutexUnlock(&ixOsalBufferPoolAccess[poolPtr->poolIdx]);
#endif
		
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
        IX_OSAL_LOG_DEV_STDOUT, "=== POOL INFORMATION ===\n", 0, 0, 0,
        0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Pool Name:                   %s\n",
        (UINT32) poolPtr->name, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Pool Allocation Type:        %d\n",
        (UINT32) poolPtr->poolAllocType, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Pool Mbuf Mem Usage (bytes): %d\n",
        (UINT32) poolPtr->mbufMemSize, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Pool Data Mem Usage (bytes): %d\n",
        (UINT32) poolPtr->dataMemSize, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Mbuf Data Capacity  (bytes): %d\n",
        (UINT32) poolPtr->bufDataSize, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Total Mbufs in Pool:         %d\n",
        (UINT32) poolPtr->totalBufsInPool, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Available Mbufs:             %d\n", (UINT32) count, 0,
        0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Next Available Mbuf:         %p\n", (UINT32) nextBufPtr,
        0, 0, 0, 0, 0);

    if (poolPtr->poolAllocType == IX_OSAL_MBUF_POOL_TYPE_USER_ALLOC)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
            IX_OSAL_LOG_DEV_STDOUT,
            "Mbuf Mem Area Start address: %p\n",
            (UINT32) poolPtr->mbufMemPtr, 0, 0, 0, 0, 0);
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
            "Data Mem Area Start address: %p\n",
            (UINT32) poolPtr->dataMemPtr, 0, 0, 0, 0, 0);
    }
}

PUBLIC VOID
ixOsalMbufDataPtrReset (IX_OSAL_MBUF * bufPtr)
{
    IX_OSAL_MBUF_POOL *poolPtr;
    UINT8 *poolDataPtr;

    if (bufPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalBuffPoolBufDataPtrReset"
            ": ERROR - Invalid Parameter\n", 0, 0, 0, 0, 0, 0);
        return;
    }

    poolPtr = (IX_OSAL_MBUF_POOL *) IX_OSAL_MBUF_NET_POOL (bufPtr);

		poolDataPtr = poolPtr->dataMemPtr;

    if (poolPtr->poolAllocType == IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC)
    {
        if (IX_OSAL_MBUF_GET_SYS_SIGNATURE(bufPtr) != IX_OSAL_MBUF_SYS_SIGNATURE)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalBuffPoolBufDataPtrReset"
                ": invalid mbuf, cannot reset mData pointer\n", 0, 0,
                0, 0, 0, 0);
            return;
        }
        IX_OSAL_MBUF_MDATA (bufPtr) = (UINT8*)IX_OSAL_MBUF_ALLOCATED_BUFF_DATA (bufPtr);
    }
    else
    {
        if (poolDataPtr)
        {
            UINT32 bufSize = poolPtr->bufDataSize;
            UINT32 bufDataAddr =
                (UINT32) IX_OSAL_MBUF_MDATA (bufPtr);
            UINT32 poolDataAddr = (UINT32) poolDataPtr;

            /*
             * the pointer is still pointing somewhere in the mbuf payload.
             * This operation moves the pointer to the beginning of the 
             * mbuf payload
             */
            bufDataAddr = ((bufDataAddr - poolDataAddr) / bufSize) * bufSize;
            IX_OSAL_MBUF_MDATA (bufPtr) = &poolDataPtr[bufDataAddr];
        }
        else
        {
            ixOsalLog (IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalBuffPoolBufDataPtrReset"
                ": cannot be used if user supplied NULL pointer for pool data area "
                "when pool was created\n", 0, 0, 0, 0, 0, 0);
            return;
        }
    }

}

/*
 * Function definition: ixOsalBuffPoolUninit
 */
PUBLIC IX_STATUS
ixOsalBuffPoolUninit (IX_OSAL_MBUF_POOL * pool)
{
    if (!pool)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalBuffPoolUninit: NULL ptr \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (pool->freeBufsInPool != pool->totalBufsInPool)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalBuffPoolUninit: need to return all ptrs to the pool first! \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

#if defined(__linux_user) || defined(__freebsd_user)

		ixOsalMutexDestroy(&ixOsalBufferPoolAccess[pool->poolIdx]);		

#endif
		
    if (pool->poolAllocType == IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC)
    {
#ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY
				UINT32 i;
				IX_OSAL_MBUF* pBuf;
				
				pBuf = pool->nextFreeBuf;
				/* Freed the Buffer one by one till all the Memory is freed*/
				for (i= pool->freeBufsInPool; i >0 && pBuf!=NULL ;i--){
						IX_OSAL_MBUF* pBufTemp;
						pBufTemp = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(pBuf);
					
						/* Freed MBUF Data Memory area*/

						IX_OSAL_BUFF_MEM_FREE((VOID *) (IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(pBuf)) );

						/* Freed MBUF Struct Memory area*/

						IX_OSAL_BUFF_MEM_FREE(pBuf);
						pBuf = pBufTemp;
				}
				
#else    	

				IX_OSAL_BUFF_MEM_FREE (pool->mbufMemPtr);
        IX_OSAL_BUFF_MEM_FREE(pool->dataMemPtr);
#endif        
    }

    ixOsalBuffFreePools[pool->poolIdx / IX_OSAL_BUFF_FREE_BITS] &=
        ~(1 << (pool->poolIdx % IX_OSAL_BUFF_FREE_BITS));
    ixOsalBuffPoolsInUse--;
    return IX_SUCCESS;
}

/*
 * Function definition: ixOsalBuffPoolDataAreaSizeGet
 */
PUBLIC UINT32
ixOsalBuffPoolDataAreaSizeGet (UINT32 count, UINT32 size)
{
    UINT32 memorySize;
    memorySize = count * IX_OSAL_MBUF_POOL_SIZE_ALIGN (size);
    return memorySize;
}

/*
 * Function definition: ixOsalBuffPoolMbufAreaSizeGet
 */
PUBLIC UINT32
ixOsalBuffPoolMbufAreaSizeGet (UINT32 count)
{
    UINT32 memorySize;
    memorySize =
        count * IX_OSAL_MBUF_POOL_SIZE_ALIGN (sizeof (IX_OSAL_MBUF));
    return memorySize;
}

/*
 * Function definition: ixOsalBuffPoolFreeCountGet
 */
PUBLIC UINT32 ixOsalBuffPoolFreeCountGet(IX_OSAL_MBUF_POOL * poolPtr)
{

   return poolPtr->freeBufsInPool;

}

#endif /* IX_OSAL_USE_DEFAULT_BUFFER_MGT */
