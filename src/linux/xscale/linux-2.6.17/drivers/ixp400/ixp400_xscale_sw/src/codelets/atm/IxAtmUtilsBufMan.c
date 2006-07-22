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
