/**
 * @file    IxQMgrInit.c
 *
 * @author Intel Corporation
 * @date    26-Jan-2006
 *
 * @brief:  Provided initialization of the QMgr component and its subcomponents.
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
#include "IxQMgrDispatcher_p.h"
#include "IxQMgrLog_p.h"
#include "IxQMgrQAccess_p.h"
#include "IxQMgrDefines_p.h"
#include "IxQMgrHwQIfIxp400_p.h"

/*
 * Set to true if initialized
 * N.B. global so integration/unit tests can reinitialize
 */
BOOL qMgrIsInitialized = FALSE;

/*
 * Function definitions.
 */
IX_STATUS
ixQMgrInit (void)
{
    if (qMgrIsInitialized)
    {
	IX_QMGR_LOG0("ixQMgrInit: IxQMgr already initialised\n");
	return IX_FAIL;
    }

    /* Initialise the QCfg component */
    ixQMgrQCfgInit ();

    /* Initialise the Dispatcher component */
    ixQMgrDispatcherInit ();

    /* Initialise the Access component */
    ixQMgrQAccessInit ();

    /* Initialization complete */
    qMgrIsInitialized = TRUE;

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrUnload (void)
{
    if (!qMgrIsInitialized)
    {
	return IX_FAIL;
    }

    /* Uninitialise the QCfg component */
    ixQMgrQCfgUninit ();
    /* Uninitialization complete */
    qMgrIsInitialized = FALSE;

    return IX_SUCCESS;
}

void
ixQMgrShow (void)
{
    IxQMgrQCfgStats *qCfgStats = NULL;
    IxQMgrDispatcherStats *dispatcherStats = NULL;
    UINT32 qIndex;
    UINT32 group0IntRegRead, group1IntRegRead;

    qCfgStats = ixQMgrQCfgStatsGet ();
    dispatcherStats = ixQMgrDispatcherStatsGet ();
    ixQMgrHwQIfQInterruptRegRead (IX_QMGR_GROUP_Q0_TO_Q31, &group0IntRegRead);
    ixQMgrHwQIfQInterruptRegRead (IX_QMGR_GROUP_Q32_TO_Q63, &group1IntRegRead);
    
    printf("Generic Stats........\n");
    printf("=====================\n");
    printf("Loop Run Count..........%u\n",dispatcherStats->loopRunCnt);
    printf("Watermark set count.....%d\n", qCfgStats->wmSetCnt);
    printf("===========================================\n");
    printf("On the fly Interrupt Register Stats........\n");
    printf("===========================================\n");
    printf("Queues 0-31 Interrupt Register............0x%08x\n",group0IntRegRead);
    printf("Queues 32-63 Interrupt Register............0x%08x\n",group1IntRegRead);

    printf("==============================================\n");
    printf("Queue Specific Stats........\n");
    printf("============================\n");

    for (qIndex=0; qIndex<IX_QMGR_MAX_NUM_QUEUES; qIndex++)
    {
	if (ixQMgrQIsConfigured(qIndex))
	{
	    ixQMgrQShow(qIndex);
	}
    }

    printf("============================\n");
}

IX_STATUS
ixQMgrQShow (IxQMgrQId qId)
{
    IxQMgrQCfgStats *qCfgStats = NULL;
    IxQMgrDispatcherStats *dispatcherStats = NULL; 

    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
    
    dispatcherStats = ixQMgrDispatcherStatsGet ();
    qCfgStats = ixQMgrQCfgQStatsGet (qId);

    printf("QId %d\n", qId);
    printf("======\n");
    printf("  IxQMgrQCfg Stats\n");
    printf("    Name..................... \"%s\"\n", qCfgStats->qStats[qId].qName);
    printf("    Size in words............ %u\n", qCfgStats->qStats[qId].qSizeInWords);
    printf("    Entry size in words...... %u\n", qCfgStats->qStats[qId].qEntrySizeInWords);
    printf("    Nearly empty watermark... %u\n", qCfgStats->qStats[qId].ne);
    printf("    Nearly full watermark.... %u\n", qCfgStats->qStats[qId].nf);
    printf("    Number of full entries... %u\n", qCfgStats->qStats[qId].numEntries);
    printf("    Sram base address........ 0x%X\n", qCfgStats->qStats[qId].baseAddress);
    printf("    Read pointer............. 0x%X\n", qCfgStats->qStats[qId].readPtr);
    printf("    Write pointer............ 0x%X\n", qCfgStats->qStats[qId].writePtr);

#ifndef NDEBUG
    if (dispatcherStats->queueStats[qId].notificationEnabled)
    {
        char *localEvent = "none ????";
        switch (dispatcherStats->queueStats[qId].srcSel)
        {
            case IX_QMGR_Q_SOURCE_ID_E:
                localEvent = "Empty";
                break;
            case IX_QMGR_Q_SOURCE_ID_NE:
                localEvent = "Nearly Empty";
                break;
            case IX_QMGR_Q_SOURCE_ID_NF:
                localEvent = "Nearly Full";
                break;
            case IX_QMGR_Q_SOURCE_ID_F:
                localEvent = "Full";
                break;
            case IX_QMGR_Q_SOURCE_ID_NOT_E:
                localEvent = "Not Empty";
                break;
            case IX_QMGR_Q_SOURCE_ID_NOT_NE:
                localEvent = "Not Nearly Empty";
                break;
            case IX_QMGR_Q_SOURCE_ID_NOT_NF:
                localEvent = "Not Nearly Full";
                break;
            case IX_QMGR_Q_SOURCE_ID_NOT_F:
                localEvent = "Not Full";
                break;
            default :
                break;
        }
        printf("    Notifications localEvent...... %s\n", localEvent);
    }
    else
    {
        printf("    Notifications............ not enabled\n");
    }
    printf("  IxQMgrDispatcher Stats\n");
    printf("    Callback count................%d\n",
	  dispatcherStats->queueStats[qId].callbackCnt);
    printf("    Priority change count.........%d\n",
	  dispatcherStats->queueStats[qId].priorityChangeCnt);
    printf("    Interrupt no callback count...%d\n",
	  dispatcherStats->queueStats[qId].intNoCallbackCnt);
    printf("    Interrupt lost callback count...%d\n",
	  dispatcherStats->queueStats[qId].intLostCallbackCnt);
#endif

    return IX_SUCCESS;
}




