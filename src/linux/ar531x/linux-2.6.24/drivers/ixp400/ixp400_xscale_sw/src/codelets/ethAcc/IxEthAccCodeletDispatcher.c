/**
 * @file IxEthAccCodeletDispatcher.c
 *
 * @date 22 April 2002
 *
 * @brief This file contains the implementation of the Ethernet Access 
 * Codelet that implements the QMgr entry point. All the codelet 
 * traffic is configured to run thru the QMgr disaptcher callbacks.
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
#include "IxQMgr.h"
#include "IxEthAccCodelet.h"
#include "IxEthAccCodelet_p.h"

/* Qmgr interrupt */
#define IX_ETH_CODELET_QMGR_IRQ IX_OSAL_IXP400_QM1_IRQ_LVL

/*
 * Variable declarations global to this file only.
 */
PRIVATE IxQMgrDispatcherFuncPtr ixEthAccCodeletDispatcherFunc;
PRIVATE IxOsalMutex ixEthAccCodeletDispatcherPollRunning;
PRIVATE volatile BOOL ixEthAccCodeletDispatcherPollStopTrigger = FALSE;
PRIVATE BOOL ixEthAccCodeletDispatcherInitialized = FALSE;

/**
 * Function definition: ixEthAccCodeletDispatcherPoll
 *
 * This function polls the Queue manager loop.
 *
 */

PRIVATE void 
ixEthAccCodeletDispatcherPoll (void* arg, void** ptrRetObj)
{
    if (ixOsalMutexLock (&ixEthAccCodeletDispatcherPollRunning,
			 IX_OSAL_WAIT_FOREVER) != IX_SUCCESS)
    {
        printf("Dispatcher: Error starting QMgr Dispatcher thread! Failed to lock mutex.\n");
        return;
    }

    ixEthAccCodeletDispatcherPollStopTrigger = FALSE;

    while (1)
    {
        if (ixEthAccCodeletDispatcherPollStopTrigger)
        {
            break;  /* Exit the loop */
        }

	ixOsalYield();

	(*ixEthAccCodeletDispatcherFunc) (IX_QMGR_QUELOW_GROUP);
    }

    ixOsalMutexUnlock (&ixEthAccCodeletDispatcherPollRunning); 

    /* Exit the thread */
}


/**
 * Function definition: ixEthAccCodeletDispatcherStart()
 *
 * This function starts the Queue manager dispatch loop.
 * 
 */

IX_STATUS 
ixEthAccCodeletDispatcherStart(BOOL useInterrupt)
{ 
    IxOsalThread qDispatchThread;
    IxOsalThreadAttr threadAttr;

    threadAttr.name      = "Codelet Q Dispatcher";
    threadAttr.stackSize = 32 * 1024; /* 32kbytes */
    threadAttr.priority  = IX_ETHACC_CODELET_QMR_PRIORITY;

    if (!ixEthAccCodeletDispatcherInitialized)
    {
	/* this should be initialized once */
	ixQMgrDispatcherLoopGet(&ixEthAccCodeletDispatcherFunc);

	ixOsalMutexInit (&ixEthAccCodeletDispatcherPollRunning);
	ixEthAccCodeletDispatcherPollStopTrigger = TRUE;
	ixEthAccCodeletDispatcherInitialized = TRUE;
    }

    if(useInterrupt)	/* Interrupt mode */
    {
	/* 
	 * Hook the QM QLOW dispatcher to the interrupt controller. 
	 */
	if (ixOsalIrqBind(IX_ETH_CODELET_QMGR_IRQ,
			  (IxOsalVoidFnVoidPtr)(ixEthAccCodeletDispatcherFunc),
			  (void *)IX_QMGR_QUELOW_GROUP) != IX_SUCCESS)
	{
	    ixOsalMutexDestroy(&ixEthAccCodeletDispatcherPollRunning);	
	    printf("Dispatcher: Failed to bind to QM1 interrupt\n");
	    return (IX_FAIL);
	}
    }
    else
    {  
	if (ixEthAccCodeletDispatcherPollStopTrigger)
	{
	    /* Polled mode based on a task running a loop */
	    if (ixOsalThreadCreate(&qDispatchThread,
				   &threadAttr,
				   (IxOsalVoidFnVoidPtr) ixEthAccCodeletDispatcherPoll,
				   NULL)	
		!= IX_SUCCESS)
	    {
	    	ixOsalMutexDestroy(&ixEthAccCodeletDispatcherPollRunning);
		printf("Dispatcher: Error spawning Q Dispatcher task\n");
		return (IX_FAIL);
	    }
	    
	    /* Start the thread */
	    if (ixOsalThreadStart(&qDispatchThread) != IX_SUCCESS)
	    {
	    	ixOsalMutexDestroy(&ixEthAccCodeletDispatcherPollRunning);
		printf("Dispatcher: Error failed to start the Q Dispatcher thread\n");
		return IX_FAIL;
	    }
	}
    }

    return (IX_SUCCESS);
}

/**
 * Function definition: ixEthAccCodeletDispatcherStop()
 *
 * This function stops the Queue manager dispatch loop.
 *
 */

IX_STATUS 
ixEthAccCodeletDispatcherStop(BOOL useInterrupt)
{
    if(useInterrupt) /* Interrupt mode */
    {
	/* 
	 * Unhook the QM QLOW dispatcher to the interrupt controller. 
	 */
	if (ixOsalIrqUnbind(IX_ETH_CODELET_QMGR_IRQ) != IX_SUCCESS)
	{
	    printf("Dispatcher: Failed to unbind to QM1 interrupt\n");
	    return (IX_FAIL);
	}
    }
    else
    {
	if (!ixEthAccCodeletDispatcherPollStopTrigger)
	{
	    ixEthAccCodeletDispatcherPollStopTrigger = TRUE;
	    if (ixOsalMutexLock (&ixEthAccCodeletDispatcherPollRunning,
				 IX_OSAL_WAIT_FOREVER) != IX_SUCCESS)
	    {
		printf("Dispatcher: Error stopping QMgr Dispatcher thread!\n");
		return (IX_FAIL);
	    }
	    ixOsalMutexUnlock (&ixEthAccCodeletDispatcherPollRunning);
	    
	    ixOsalMutexDestroy(&ixEthAccCodeletDispatcherPollRunning);
	}
    }

    ixEthAccCodeletDispatcherInitialized = FALSE;

    return (IX_SUCCESS);
}
