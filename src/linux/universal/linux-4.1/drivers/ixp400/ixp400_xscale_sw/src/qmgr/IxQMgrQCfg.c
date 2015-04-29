/**
 * @file    QMgrQCfg.c
 *
 * @author Intel Corporation
 * @date    26-Jan-2006
 * 
 * @brief   This modules provides an interface for setting up the static
 * configuration of HwQ queues. This file contains the following
 * functions:
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

/*
 * System defined include files.
 */

/*
 * User defined include files.
 */
 
#include "IxOsal.h"
#include "IxQMgr_sp.h"
#include "IxQMgrQCfg_p.h"
#include "IxQMgrDefines_p.h"
#include "IxQMgrHwQIfIxp400_p.h"

/*
 * #defines and macros used in this file.
 */
#define IX_QMGR_MIN_ENTRY_SIZE_IN_WORDS 16

/* Size of SRAM in a qmgr memory map block in bytes
 */
#define IX_QMGR_HWQ_SRAM_SIZE_IN_BYTES 0x4000

/*
 * Check that qId is a valid queue identifier. This is provided to
 * make the code easier to read.
 */
#define IX_QMGR_QID_IS_VALID(qId) \
(((qId) >= (IX_QMGR_MIN_QID)) && ((qId) <= (IX_QMGR_MAX_QID)))

/*
 * Typedefs whose scope is limited to this file.
 */

/*
 * This struct describes an HwQ queue.
 * N.b. bufferSizeInWords and qEntrySizeInWords are stored in the queue
 * as these are requested by Access in the data path. sizeInEntries is
 * not required by the data path so it can be calculated dynamically.
 * 
 */
typedef struct
{
    char qName[IX_QMGR_MAX_QNAME_LEN+1];       /* Textual description of a queue*/
    IxQMgrQSizeInWords qSizeInWords;           /* The number of words in the queue */
    IxQMgrQEntrySizeInWords qEntrySizeInWords; /* Number of words per queue entry*/
    BOOL isConfigured;                         /* This flag is TRUE if the queue has
                                                *   been configured
                                                */
} IxQMgrCfgQ;

/*
 * This struct contains the essential parameters 
 * required to reconfigure a queue using the proper 
 * config parameters from IxQMgrCfgQ structure.
 */
typedef struct
{
    UINT32 freeSramAddress; /* Actual SRAM base address of a configured queue */
}IxQMgrRecfgQ;

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */

extern UINT32 * ixQMgrHwQIfQueAccRegAddr[]; 

/* Store data required to inline read and write access
 */
IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[IX_QMGR_MAX_NUM_QUEUES];

/* 
 * Store data required to configure a queue
 */
static IxQMgrCfgQ cfgQueueInfo[IX_QMGR_MAX_NUM_QUEUES];

/* 
 * Store data required to re-configure a queue
 */
static IxQMgrRecfgQ reCfgQueueInfo[IX_QMGR_MAX_NUM_QUEUES]; 

/*
 * This flag is used to check whether the call is being made for 
 * Config or Reconfig purpose 
 */
BOOL reCfgFlag = FALSE;

/* This pointer holds the starting address of HwQ SRAM not used by
 * the HwQ queues.
 */
static UINT32 freeSramAddress[IX_QMGR_MEM_MAP_BLOCK_MAX] = {0};

/* 4 words of zeroed memory for inline access */
static UINT32 zeroedPlaceHolder[4] = { 0, 0, 0, 0 };

static BOOL cfgInitialized = FALSE;

/* 
 * Mutex variable used during configuartion of a queue 
 */
static IxOsalMutex ixQMgrQCfgMutex;

/* 
 * Mutex variable used during Un-configuartion of a queue 
 */
static IxOsalMutex ixQMgrQUncfgMutex;

/*
 * Statistics
 */
static IxQMgrQCfgStats stats;

/*
 * Function declarations
 */
PRIVATE BOOL
watermarkLevelIsOk (IxQMgrQId qId, IxQMgrWMLevel level);

PRIVATE BOOL
qSizeInWordsIsOk (IxQMgrQSizeInWords qSize);

PRIVATE BOOL
qEntrySizeInWordsIsOk (IxQMgrQEntrySizeInWords entrySize);

/*
 * Function definitions.
 */
void
ixQMgrQCfgInit (void)
{
    UINT32 qIndex, blockIndex;
    
    for (qIndex=0; qIndex < IX_QMGR_MAX_NUM_QUEUES;qIndex++)
    {
	/* info for code inlining */
	ixQMgrHwQIfQueAccRegAddr[qIndex] = zeroedPlaceHolder;

	/* info for code inlining */
	ixQMgrQInlinedReadWriteInfo[qIndex].qReadCount = 0;
	ixQMgrQInlinedReadWriteInfo[qIndex].qWriteCount = 0;
	ixQMgrQInlinedReadWriteInfo[qIndex].qAccRegAddr = zeroedPlaceHolder;
	ixQMgrQInlinedReadWriteInfo[qIndex].qUOStatRegAddr = zeroedPlaceHolder;
	ixQMgrQInlinedReadWriteInfo[qIndex].qUflowStatBitMask = 0;
	ixQMgrQInlinedReadWriteInfo[qIndex].qOflowStatBitMask = 0;
	ixQMgrQInlinedReadWriteInfo[qIndex].qEntrySizeInWords = 0;
	ixQMgrQInlinedReadWriteInfo[qIndex].qSizeInEntries = 0;
	ixQMgrQInlinedReadWriteInfo[qIndex].qConfigRegAddr = zeroedPlaceHolder;
   }

    /* Initialise the HwQIf component */
    ixQMgrHwQIfInit ();
   
    /* Reset all queues to have queue name = NULL, entry size = 0 and
     * isConfigured = false
     */
    for (qIndex=0; qIndex < IX_QMGR_MAX_NUM_QUEUES;qIndex++)
    {
	strcpy (cfgQueueInfo[qIndex].qName, "");
	cfgQueueInfo[qIndex].qSizeInWords = 0;
	cfgQueueInfo[qIndex].qEntrySizeInWords = 0;
	cfgQueueInfo[qIndex].isConfigured = FALSE;
 
        reCfgQueueInfo[qIndex].freeSramAddress = 0;

	/* Statistics */
	stats.qStats[qIndex].isConfigured = FALSE;
	stats.qStats[qIndex].qName = cfgQueueInfo[qIndex].qName;
    }

    /* Statistics */
    stats.wmSetCnt = 0;

    for (blockIndex = 0; blockIndex < IX_QMGR_MEM_MAP_BLOCK_MAX; blockIndex++)
    {
        ixQMgrHwQIfInternalSramBaseAddressGet (blockIndex, &freeSramAddress[blockIndex]);
	freeSramAddress[blockIndex] -= (blockIndex * IX_QMGR_MEM_MAP_BLOCK_SIZE);
    }
    
    ixOsalMutexInit(&ixQMgrQCfgMutex);
    ixOsalMutexInit(&ixQMgrQUncfgMutex);

    cfgInitialized = TRUE;
}

void
ixQMgrQCfgUninit (void)
{

    UINT32 qIndex, blockIndex;
    UINT8 retVal = IX_SUCCESS;
    cfgInitialized = FALSE;

    ixOsalMutexDestroy (&ixQMgrQCfgMutex);
    ixOsalMutexDestroy (&ixQMgrQUncfgMutex);

    for (qIndex = 0; qIndex < IX_QMGR_MAX_NUM_QUEUES; qIndex++)
    {
        ixOsalSleep(1);

        retVal = ixQMgrNotificationDisable (qIndex);
        /* Reset the qMgr Notification Callback to NULL */
        if(IX_SUCCESS != retVal && IX_QMGR_Q_NOT_CONFIGURED != retVal)
        {
            IX_QMGR_LOG_WARNING1("Failed to Disable Notification the queue."
                                 ".qid - %d\n", qIndex);
	}

        cfgQueueInfo[qIndex].isConfigured = FALSE;
    }
    
    for (blockIndex = 0; blockIndex < IX_QMGR_MEM_MAP_BLOCK_MAX; blockIndex++)
    {
        freeSramAddress[blockIndex] = 0;
    }

    /* Uninitialise the HwQIf component */
    ixQMgrHwQIfUninit ();
}

IX_STATUS
ixQMgrQConfig (char *qName,
	      IxQMgrQId qId,
	      IxQMgrQSizeInWords qSizeInWords,
	      IxQMgrQEntrySizeInWords qEntrySizeInWords)
{
    UINT32 hwQLocalBaseAddress;
    UINT32 block;
    UINT32 *pSramBaseAddress;
    
    if (!cfgInitialized)
    {
        return IX_FAIL;
    }
     
    /* Check for wild card parameters to reconfigure a queue */
    if(IX_QMGR_QID_IS_VALID(qId) && (NULL == qName) && 
       (IX_QMGR_Q_SIZE_INVALID == qSizeInWords) &&
       (IX_QMGR_Q_ENTRY_SIZE_INVALID == qEntrySizeInWords))
    {
        if (reCfgQueueInfo[qId].freeSramAddress == 0)
        {
            return IX_QMGR_Q_NOT_CONFIGURED;
        }
     
        if (cfgQueueInfo[qId].isConfigured)
        {
            return IX_QMGR_Q_ALREADY_CONFIGURED;
        }
        
        ixOsalMutexLock(&ixQMgrQCfgMutex, IX_OSAL_WAIT_FOREVER);
        
        qName = cfgQueueInfo[qId].qName;
        qSizeInWords = cfgQueueInfo[qId].qSizeInWords;
        qEntrySizeInWords = cfgQueueInfo[qId].qEntrySizeInWords;
        pSramBaseAddress = &reCfgQueueInfo[qId].freeSramAddress; 
       
        reCfgFlag = TRUE;
 
        /* Write the config register */
        ixQMgrHwQIfQueCfgWrite (qId,
	  		        qSizeInWords,
			        qEntrySizeInWords,
			        pSramBaseAddress);

        /* store pre-computed information in the same cache line
         * to facilitate inlining of QRead and QWrite functions 
         * in IxQMgr.h
         */
        ixQMgrQInlinedReadWriteInfo[qId].qReadCount = 0;
        ixQMgrQInlinedReadWriteInfo[qId].qWriteCount = 0;
        ixQMgrQInlinedReadWriteInfo[qId].qEntrySizeInWords = qEntrySizeInWords;
        ixQMgrQInlinedReadWriteInfo[qId].qSizeInEntries = 
	                 (UINT32)qSizeInWords / (UINT32)qEntrySizeInWords;

        /* The queue is now configured */
        cfgQueueInfo[qId].isConfigured = TRUE;
        
        ixOsalMutexUnlock(&ixQMgrQCfgMutex);
    }
    else
    {
        /* Validate parameters */
        if (!IX_QMGR_QID_IS_VALID(qId))
        {
	    return IX_QMGR_INVALID_Q_ID; 
        }
    
        else if (NULL == qName)
        {
	    return IX_QMGR_PARAMETER_ERROR;
        }
    
        else if (strlen (qName) > IX_QMGR_MAX_QNAME_LEN)
        {
	    return IX_QMGR_PARAMETER_ERROR;
        }

        else if (!qSizeInWordsIsOk (qSizeInWords))
        {
	    return IX_QMGR_INVALID_QSIZE;
        }

        else if (!qEntrySizeInWordsIsOk (qEntrySizeInWords))
        {
	    return IX_QMGR_INVALID_Q_ENTRY_SIZE;
        }
    
        else if (cfgQueueInfo[qId].isConfigured)
        {
	    return IX_QMGR_Q_ALREADY_CONFIGURED;
        }
   
        ixOsalMutexLock(&ixQMgrQCfgMutex, IX_OSAL_WAIT_FOREVER);

        pSramBaseAddress = freeSramAddress;
        reCfgQueueInfo[qId].freeSramAddress = *pSramBaseAddress;
        
        /* Write the config register */
        ixQMgrHwQIfQueCfgWrite (qId,
	    		        qSizeInWords,
			        qEntrySizeInWords,
			        freeSramAddress);

        strcpy (cfgQueueInfo[qId].qName, qName);
        cfgQueueInfo[qId].qSizeInWords = qSizeInWords;
        cfgQueueInfo[qId].qEntrySizeInWords = qEntrySizeInWords;

        /* store pre-computed information in the same cache line
         * to facilitate inlining of QRead and QWrite functions 
         * in IxQMgr.h
         */
        ixQMgrQInlinedReadWriteInfo[qId].qReadCount = 0;
        ixQMgrQInlinedReadWriteInfo[qId].qWriteCount = 0;
        ixQMgrQInlinedReadWriteInfo[qId].qEntrySizeInWords = qEntrySizeInWords;
        ixQMgrQInlinedReadWriteInfo[qId].qSizeInEntries = 
	    	    (UINT32)qSizeInWords / (UINT32)qEntrySizeInWords;

        /* Determine which memory map block the queue is in
         */
        block = qId / IX_QMGR_NUM_QUEUES_PER_MEM_MAP_BLOCK;
 
        /* Calculate the new freeSramAddress from the size of the queue
         * currently being configured.
         */
        freeSramAddress[block] += (qSizeInWords * IX_QMGR_NUM_BYTES_PER_WORD);

        /* Get the virtual SRAM address */
        ixQMgrHwQIfBaseAddressGet (&hwQLocalBaseAddress);

        /* Determine if configuring and allocating sram space to this queue means that 
         * the amount of sram space used has not exceeded the total amount of sram available
         */ 
        IX_OSAL_ASSERT((freeSramAddress[block] - 
                       (hwQLocalBaseAddress + (IX_QMGR_QUEBUFFER0_SPACE_OFFSET))) <=
	               IX_QMGR_QUE_BUFFER_SPACE_SIZE);
    
        /* The queue is now configured */
        cfgQueueInfo[qId].isConfigured = TRUE;
        
        ixOsalMutexUnlock(&ixQMgrQCfgMutex);
    }
#ifndef NDEBUG
    /* Update statistics */
    stats.qStats[qId].isConfigured = TRUE;
    stats.qStats[qId].qName = cfgQueueInfo[qId].qName;
#endif
    return IX_SUCCESS;
}

IX_STATUS 
ixQMgrQUnconfig (IxQMgrQId qId)
{
    volatile UINT32 *cfgAddress = NULL;
    UINT8 retVal = IX_SUCCESS;

    if (!cfgInitialized)
    {
        return IX_FAIL;
    }
    
    /* Validate the qId parameter */
    if (!IX_QMGR_QID_IS_VALID(qId))
    {
	return IX_QMGR_INVALID_Q_ID;
    }
    else if (!cfgQueueInfo[qId].isConfigured)
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
   
    ixOsalMutexLock(&ixQMgrQUncfgMutex, IX_OSAL_WAIT_FOREVER);

    /* Disable the Notification to the queue */
    retVal = ixQMgrNotificationDisable (qId);
    if(IX_SUCCESS != retVal && IX_QMGR_Q_NOT_CONFIGURED != retVal)
    {
       IX_QMGR_LOG_WARNING1("Failed to Disable Notification the queue - %d\n", qId);
    }
    
    /* Calculate and Reset the config register value */
    cfgAddress = (UINT32*)(hwQBaseAddress +
			IX_QMGR_Q_CONFIG_ADDR_GET(qId));

    ixQMgrHwQIfWordWrite (cfgAddress, 0); 

    /* Reset the stored pre-computed information in the cache line */
    ixQMgrQInlinedReadWriteInfo[qId].qReadCount = 0;
    ixQMgrQInlinedReadWriteInfo[qId].qWriteCount = 0;
    ixQMgrQInlinedReadWriteInfo[qId].qEntrySizeInWords = 0;
    ixQMgrQInlinedReadWriteInfo[qId].qSizeInEntries = 0;

    cfgQueueInfo[qId].isConfigured = FALSE;
 
    ixOsalMutexUnlock(&ixQMgrQUncfgMutex);

#ifndef NDEBUG
    /* Update statistics */
    stats.qStats[qId].isConfigured = FALSE;
    stats.qStats[qId].qName = "" ;
#endif

    return IX_SUCCESS;
}

IxQMgrQSizeInWords
ixQMgrQSizeInWordsGet (IxQMgrQId qId)
{
    if (!ixQMgrQIsConfigured(qId))
    {
        return IX_QMGR_Q_SIZE_INVALID;
    }
    /* No parameter checking as this is used on the data path */
    return (cfgQueueInfo[qId].qSizeInWords);
}

IX_STATUS
ixQMgrQSizeInEntriesGet (IxQMgrQId qId,
			 unsigned *qSizeInEntries)
{
    if (!ixQMgrQIsConfigured(qId))
    {
        return IX_QMGR_Q_NOT_CONFIGURED;
    }

    if(NULL == qSizeInEntries)
    {
        return IX_QMGR_PARAMETER_ERROR;
    }

    *qSizeInEntries = (UINT32)(cfgQueueInfo[qId].qSizeInWords) /
        (UINT32)cfgQueueInfo[qId].qEntrySizeInWords;

    return IX_SUCCESS;
}

IxQMgrQEntrySizeInWords
ixQMgrQEntrySizeInWordsGet (IxQMgrQId qId)
{
    if (!ixQMgrQIsConfigured(qId))
    {
        return IX_QMGR_Q_ENTRY_SIZE_INVALID;
    }
    /* No parameter checking as this is used on the data path */
    return (cfgQueueInfo[qId].qEntrySizeInWords);
}

IX_STATUS
ixQMgrWatermarkSet (IxQMgrQId qId,
		    IxQMgrWMLevel ne,
		    IxQMgrWMLevel nf)
{    
    IxQMgrQStatus qStatusOnEntry;/* The queue status on entry/exit */
    IxQMgrQStatus qStatusOnExit; /* to this function               */

    if (!ixQMgrQIsConfigured(qId))
    {
        return IX_QMGR_Q_NOT_CONFIGURED;
    }

    if (!watermarkLevelIsOk (qId, ne))
    {
	return IX_QMGR_INVALID_Q_WM;
    }

    if (!watermarkLevelIsOk (qId, nf))
    {
	return IX_QMGR_INVALID_Q_WM;
    }

    /* Get the current queue status */
    ixQMgrHwQIfQueStatRead (qId, &qStatusOnEntry);

#ifndef NDEBUG
    /* Update statistics */
    stats.wmSetCnt++;
#endif

    ixQMgrHwQIfWatermarkSet (qId,
			    ne,
			    nf);

    /* Get the current queue status */
    ixQMgrHwQIfQueStatRead (qId, &qStatusOnExit);
  
    /* If the status has changed return a warning */
    if (qStatusOnEntry != qStatusOnExit)
    {
	return IX_QMGR_WARNING;
    }

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrAvailableSramAddressGet (UINT32 *address,
			       unsigned *sizeOfFreeRam)
{ 
    return ixQMgrAvailableSramAddressInBlockGet(IX_QMGR_MEM_MAP_BLOCK_0,
                                                address,
						sizeOfFreeRam);
}

IX_STATUS
ixQMgrAvailableSramAddressInBlockGet (IxQMgrMemMapBlock block,
                                      UINT32 *address,
			              UINT32 *sizeOfFreeRam)
{
    UINT32 hwQLocalBaseAddress;

    if ((NULL == address)||(NULL == sizeOfFreeRam)) 
    {
	return IX_QMGR_PARAMETER_ERROR;
    }
    if (block >= IX_QMGR_MEM_MAP_BLOCK_MAX)
    {
        return IX_QMGR_PARAMETER_ERROR;
    }
    if (!cfgInitialized)
    {
	return IX_FAIL;
    }

    /* Get the virtual SRAM address */
    ixQMgrHwQIfBaseAddressGet (&hwQLocalBaseAddress);
    
    *address = freeSramAddress[block];

    /* 
     * Calculate the size in bytes of free sram 
     * i.e. current free SRAM virtual pointer from
     *      (base + total size)
     */
    *sizeOfFreeRam = 
	(hwQLocalBaseAddress +
	IX_QMGR_HWQ_SRAM_SIZE_IN_BYTES) -
	freeSramAddress[block];
    
    if (0 == *sizeOfFreeRam)
    {
	return IX_QMGR_NO_AVAILABLE_SRAM;
    }
    return IX_SUCCESS;
}

BOOL
ixQMgrQIsConfigured (IxQMgrQId qId)
{
    if (!IX_QMGR_QID_IS_VALID(qId))
    {
	return FALSE;
    }

    return cfgQueueInfo[qId].isConfigured;
}

IxQMgrQCfgStats*
ixQMgrQCfgStatsGet (void)
{
    return &stats;
}

IxQMgrQCfgStats*
ixQMgrQCfgQStatsGet (IxQMgrQId qId)
{
    UINT32 ne;
    UINT32 nf;
    UINT32 baseAddress;
    UINT32 readPtr;
    UINT32 writePtr;

    if (!ixQMgrQIsConfigured(qId))
    {
        return (IxQMgrQCfgStats*)IX_QMGR_Q_NOT_CONFIGURED;
    }

    stats.qStats[qId].qSizeInWords = cfgQueueInfo[qId].qSizeInWords;
    stats.qStats[qId].qEntrySizeInWords = cfgQueueInfo[qId].qEntrySizeInWords;
    
    if (IX_SUCCESS != ixQMgrQNumEntriesGet (qId, &stats.qStats[qId].numEntries))
    {
        if (IX_QMGR_WARNING != ixQMgrQNumEntriesGet (qId, &stats.qStats[qId].numEntries))
        {
	   IX_QMGR_LOG_WARNING1("Failed to get the number of entries in queue.... %d\n", qId);
        }
    }

    ixQMgrHwQIfQueCfgRead (qId,
			   stats.qStats[qId].numEntries,
			   &baseAddress,
			   &ne,
			   &nf,
			   &readPtr,
			   &writePtr);
        
    stats.qStats[qId].baseAddress = baseAddress;
    stats.qStats[qId].ne = ne;
    stats.qStats[qId].nf = nf;
    stats.qStats[qId].readPtr = readPtr;
    stats.qStats[qId].writePtr = writePtr;

    return &stats;
}

/* 
 * Static function definitions
 */

PRIVATE BOOL
watermarkLevelIsOk (IxQMgrQId qId, IxQMgrWMLevel level)
{
    UINT32 qSizeInEntries;

    switch (level)
    {
	case IX_QMGR_Q_WM_LEVEL0: 
	case IX_QMGR_Q_WM_LEVEL1: 
	case IX_QMGR_Q_WM_LEVEL2: 
	case IX_QMGR_Q_WM_LEVEL4: 
	case IX_QMGR_Q_WM_LEVEL8: 
	case IX_QMGR_Q_WM_LEVEL16:
	case IX_QMGR_Q_WM_LEVEL32:
	case IX_QMGR_Q_WM_LEVEL64:
	    break;
	default:
	    return FALSE;
    }

    /* Check watermark is not bigger than the qSizeInEntries */
    ixQMgrQSizeInEntriesGet(qId, &qSizeInEntries);

    if ((UINT32)level > qSizeInEntries)
    {
	return FALSE;
    }

    return TRUE;
}

PRIVATE BOOL
qSizeInWordsIsOk (IxQMgrQSizeInWords qSize)
{
    BOOL status;

    switch (qSize)
    {	
	case IX_QMGR_Q_SIZE16:
	case IX_QMGR_Q_SIZE32:
	case IX_QMGR_Q_SIZE64:
	case IX_QMGR_Q_SIZE128:
	    status = TRUE;
	    break;
	default:
	    status = FALSE;
	    break;
    }

    return status;
}

PRIVATE BOOL
qEntrySizeInWordsIsOk (IxQMgrQEntrySizeInWords entrySize)
{
    BOOL status;

    switch (entrySize)
    {
	case IX_QMGR_Q_ENTRY_SIZE1:
	case IX_QMGR_Q_ENTRY_SIZE2:
	case IX_QMGR_Q_ENTRY_SIZE4:
	    status = TRUE;
	    break;
	default:
	    status = FALSE;
	    break;
    }

    return status;
}
    
