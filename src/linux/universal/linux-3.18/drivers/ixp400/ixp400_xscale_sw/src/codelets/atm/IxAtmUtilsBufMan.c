/*
 * FileName:    IxAtmUtilsCodeletBufMan.c
 * Description:  Utils Buffer Management
 *
 *
 * Design Notes:
 *
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

#include "IxOsal.h"
#include "IxAtmCodelet_p.h"


typedef struct
{
    IX_OSAL_MBUF_POOL *poolPtr;
    unsigned bufSize;
    unsigned numBufs;
} IxAtmUtilsBufManPoolInfo;


/*
 * Forward declarations
 */
void ixAtmUtilsMbufShow (void);

#ifdef  __vxworks
/* vx Works pool sizes */
#define LARGE_POOL_COUNT 4096
#define SMALL_POOL_COUNT 1000
#else
/* linux pool sizes */
#define LARGE_POOL_COUNT 1024
#define SMALL_POOL_COUNT 1000
#endif

#ifdef __wince
#define NUMELEMS(array) (sizeof(array) / sizeof((array)[0]))
#endif

static IxAtmUtilsBufManPoolInfo poolInfo[] =   /* pool information table */
{
    /*
      poolPtr     bufSize  numBufs
      ---------   -----    -------
    */
    {NULL,             64,        SMALL_POOL_COUNT},  /* Small packets, e.g. OAM */
    {NULL,             2048,      LARGE_POOL_COUNT}
};



IX_STATUS
ixAtmUtilsMbufPoolInit(void)
{
    int i, numPools;

    numPools = NUMELEMS (poolInfo);

      for (i = 0; i < numPools; i++)
	{
      poolInfo[i].poolPtr =IX_OSAL_MBUF_POOL_INIT (poolInfo[i].numBufs, poolInfo[i].bufSize, "ixAtmUtils pool");
      if(poolInfo[i].poolPtr == NULL)
	{
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "mbufPoolInit: Null poolPtr returned\n", 0, 0, 0, 0, 0, 0);	  
	}
      }
    
    return IX_SUCCESS;
}

void
ixAtmUtilsMbufGet (UINT32 bufSize, IX_OSAL_MBUF **buf)
{
    IX_OSAL_MBUF_POOL *poolPtr = NULL;
    unsigned i,numPools;

    numPools = NUMELEMS (poolInfo);

    for (i = 0; i < numPools; i++)
    {
        if (poolInfo[i].bufSize >= bufSize)
	{
            poolPtr = poolInfo[i].poolPtr;
            break;
	}
    }

    if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "ixAtmUtilsMbufGet - Invalid bufSize specified\n", 0, 0, 0, 0, 0, 0);
        *buf = NULL;
        return;
    }

    *buf = IX_OSAL_MBUF_POOL_GET (poolPtr);
    
    if (*buf)
    {
        IX_OSAL_MBUF_MLEN(*buf) = bufSize;
    }
    else
    {
	ixOsalLog (IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "ixAtmUtilsMbufGet returning a NULL buffer\n", 0, 0, 0, 0, 0, 0);
	ixAtmUtilsMbufShow ();
    }
}

void
ixAtmUtilsMbufFree(IX_OSAL_MBUF *buf)
{
    IX_OSAL_MBUF_POOL_PUT_CHAIN (buf);
}

void
ixAtmUtilsMbufShow (void)
{

    IX_OSAL_MBUF_POOL *poolPtr = NULL;
    unsigned i,numPools;

    numPools = NUMELEMS (poolInfo);

    for (i = 0; i < numPools; i++)
    {
        poolPtr = poolInfo[i].poolPtr;
        if (poolPtr != NULL)
 	{
            IX_OSAL_MBUF_POOL_SHOW (poolPtr);
	}
    }
}

void
ixAtmUtilsMbufPoolSizeGet (UINT32 bufSize, UINT32 *total)
{
    IX_OSAL_MBUF_POOL *poolPtr = NULL;
    unsigned i,numPools;

    numPools = NUMELEMS (poolInfo);

    for (i = 0; i < numPools; i++)
    {
        if (poolInfo[i].bufSize >= bufSize)
	{
            poolPtr = poolInfo[i].poolPtr;
            break;
	}
    }

    if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR,
	    "ixAtmUtilsMbufPoolSizeGet - Invalid bufSize specified\n", 0, 0, 0, 0, 0, 0);
	*total = 0;
        return;
    }

    *total = poolPtr->totalBufsInPool;
}

void
ixAtmUtilsMbufPoolFreeGet (UINT32 bufSize, UINT32 *avail)
{
    IX_OSAL_MBUF_POOL *poolPtr = NULL;
    unsigned i,numPools;

    numPools = NUMELEMS (poolInfo);

    for (i = 0; i < numPools; i++)
    {
        if (poolInfo[i].bufSize >= bufSize)
	{
            poolPtr = poolInfo[i].poolPtr;
            break;
	}
    }

    if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR,
	    "ixAtmUtilsMbufPoolFreeGet - Invalid bufSize specified\n", 0, 0, 0, 0, 0, 0);
	*avail = 0;
        return;
    }

    *avail = poolPtr->freeBufsInPool;
}
