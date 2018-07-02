/**
 * @file IxDmaAccCodelet.c
 *
 * @author Intel Corporation
 * @date 18 November 2002
 *
 * @brief This file contains the implementation of the Dma Access Codelet.
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
#include "IxQMgr.h"
#include "IxNpeDl.h"
#include "IxDmaAcc.h"
#include "IxDmaAccCodelet.h"
#include "IxDmaAccCodelet_p.h"

/*
 * Defines
 */
/* Sleep timer is set to 1ms */
#define IX_CLIENT_SLEEP_IN_MS  1

/* XScale tick is 66.666666 MHz */
#define IX_DMA_CODELET_XSCALE_TICK IX_OSAL_IXP400_TIME_STAMP_RESOLUTION/1000000

/* Decimal point */
#define IX_DMA_CODELET_DECIMAL_POINT 3

/* 8 bits in a byte */
#define IX_DMA_CODELET_EIGHT_BITS 8

/* Max limit for CallBack time out */
#define IX_DMA_CODELET_MAX_TIME_OUT 10000

/*
 * Variable declarations global to this file only.
 */
static BOOL ixDmaAccCodeletInitialised = FALSE;
static IxOsalThread ixDmaAccCodeletDispatchId;

static UINT8 *ixDmaAccCodeletSrcBlock;
             /**< Pointer to memory allocation for storing DMA source test patterns */
static UINT8 *ixDmaAccCodeletDestBlock;
             /**< Pointer to memory allocation for storing DMA destination test patterns */

/* Check to see whether CallBack is returned */
BOOL ixDmaAccCodeletCallBackReturnFlag = FALSE;

/* Counter used for stop timer */
UINT32 ixDmaAccCodeletLoopCount = 0;

/* Structure to store start and stop time of PERFORMANCE_NUM_LOOP 
   (i.e. 100 runs) */
IxDmaAccCodeletTimeStore ixDmaAccCodeletTimeStore;

/* Used to check the status of the callback */
IX_STATUS ixDmaAccCodeletCallBackStatus;

/* Messages for DmaTransfer Modes */
char *ixDmaAccCodeletMsgTM[] = {
        "IX_DMA_COPY_CLEAR     ",
        "IX_DMA_COPY           ",
        "IX_DMA_COPY_BYTE_SWAP ",
        "IX_DMA_COPY_REVERSE   "
};

/* Messages for DmaAddress Modes */
char *ixDmaAccCodeletMsgAM[] = {
        "INC SRC INC DEST  ",
        "INC SRC FIX DEST  ",
        "FIX SRC INC DEST  "
};

/* Messages for DmaTransfer Widths */
char *ixDmaAccCodeletMsgTW[] = {
        "IX_DMA_32_SRC_32_DST ",
        "IX_DMA_32_SRC_16_DST ",
        "IX_DMA_32_SRC_8_DST ",
        "IX_DMA_16_SRC_32_DST ",
        "IX_DMA_16_SRC_16_DST ",
        "IX_DMA_16_SRC_8_DST ",
        "IX_DMA_8_SRC_32_DST ",
        "IX_DMA_8_SRC_16_DST ",
        "IX_DMA_8_SRC_8_DST ",
        "IX_DMA_8_SRC_BURST_DST ",
        "IX_DMA_16_SRC_BURST_DST ",
        "IX_DMA_32_SRC_BURST_DST ",
        "IX_DMA_BURST_SRC_8_DST ",
        "IX_DMA_BURST_SRC_16_DST ",
        "IX_DMA_BURST_SRC_32_DST ",
        "IX_DMA_BURST_SRC_BURST_DST "
};


/*
 * Static function prototypes.
 */
PRIVATE void ixDmaAccCodeletShow(void);
PRIVATE void ixDmaAccCodeletTestPatternReset(void);
PRIVATE void ixDmaAccCodeletDispatcherPoll(void* argUnused, void** ptrRetObjUnused);
PRIVATE IX_STATUS ixDmaAccCodeletDispatcherStart(BOOL useInterrupt);
PRIVATE IX_STATUS ixDmaAccCodeletNpeInit(IxNpeDlNpeId npeId);
PRIVATE void ixDmaAccCodeletCallback(IX_STATUS status);

PRIVATE IxQMgrDispatcherFuncPtr ixDmaAccCodeletDispatcherFunc ;

PRIVATE void ixDmaAccCodeletReportAverageTime(UINT16 tLength);

PRIVATE void ratioPrintf (int decimalPoint, UINT64 param1, UINT64 param2);

/*
 * Function definition: ixDmaAccCodeletInit()
 * See header file for documentation.
 */
IX_STATUS ixDmaAccCodeletInit(IxNpeDlNpeId npeId)
{
    UINT32 counter;

    /* Block reinitialisation if already done so */
    if(ixDmaAccCodeletInitialised)
    {
        printf("\nDma codelet already initialised");
	    return(IX_FAIL);
    }

    /* get the memory for source area */
    ixDmaAccCodeletSrcBlock = (UINT8*) IX_OSAL_CACHE_DMA_MALLOC(IX_DMA_CODELET_TEST_MAXLENGTH);

    /* get the memory for destination area */
    ixDmaAccCodeletDestBlock = (UINT8*) IX_OSAL_CACHE_DMA_MALLOC(IX_DMA_CODELET_TEST_MAXLENGTH);

    /* Initialise Queue Manager */
    printf("\nInitialising Queue Manager...");
    if (ixQMgrInit() != IX_SUCCESS)
    {
	    printf("\nError initialising queue manager!");
	    return (IX_FAIL);
    }
    /* Start the Queue Manager dispatcher loop :
       Parameter is TRUE for Interrupt mode else poll mode */
#ifdef __linux
    if(ixDmaAccCodeletDispatcherStart(TRUE) != IX_SUCCESS)
#else
    if(ixDmaAccCodeletDispatcherStart(FALSE) != IX_SUCCESS)
#endif
    {
	    printf("\nError starting queue manager dispatch loop!");
	    return (IX_FAIL);
    }
    /* Initialise NPE and download Image */
    printf("\nInitialising NPE %d...", npeId);
    if(ixDmaAccCodeletNpeInit(npeId) != IX_SUCCESS)
    {
	    printf("\nError initialising NPE %d!", npeId);
	    return (IX_FAIL);
    }

    /***********************************************************************
     * System initialisation done. Now initialise Dma Access component.
     ***********************************************************************/
    if (ixDmaAccInit(npeId) != IX_SUCCESS)
    {
	    printf("\nError initialising Dma access driver!");
	    return (IX_FAIL);
    }

    /* initialize the start and stop time array */
    for (counter = 0; counter < PERFORMANCE_LOOP_NUM; counter++)
    {
	ixDmaAccCodeletTimeStore.startTime[counter] = 0;
	ixDmaAccCodeletTimeStore.stopTime[counter] = 0;
    }

    ixDmaAccCodeletInitialised = TRUE;
    return (IX_SUCCESS);
}

/*
 * Function definition: ixDmaAccCodeletDispatcherPoll()
 * See header file for documentation.
 */
PRIVATE void ixDmaAccCodeletDispatcherPoll (void* argUnused, void** ptrRetObjUnused)
{
    while (1)
    {
	    /* Dma NPE A: queues 19 & 20; NPE B: 24 & 26; NPE B: 25 & 27 */
	    (*ixDmaAccCodeletDispatcherFunc) (IX_QMGR_QUELOW_GROUP);
	    /* Yield Execution of Current Thread with 0 Delay */
            ixOsalYield();
    }
}

/*
 * Function definition : ixDmaAccCodeletDispatcherStart()
 * See header file for documentation.
 */
PRIVATE IX_STATUS ixDmaAccCodeletDispatcherStart(BOOL useInterrupt)
{
    IxOsalThreadAttr threadAttr;
    char *pThreadName = "Dispatcher";
    
    ixQMgrDispatcherLoopGet(&ixDmaAccCodeletDispatcherFunc);

    if(useInterrupt)	/* Interrupt mode */
    {
	/*
	 * Hook the QM QLOW dispatcher to the interrupt controller.
	 * IX_QMGR_QUELOW_GROUP refers to Queues 0-31
	 * Dma NPE A: queues 19 & 20; NPE B: 24 & 26; NPE B: 25 & 27
	 */
	if (ixOsalIrqBind(IX_OSAL_IXP400_QM1_IRQ_LVL,
			    (IxOsalVoidFnVoidPtr)ixDmaAccCodeletDispatcherFunc,
			    (void *)IX_QMGR_QUELOW_GROUP) != IX_SUCCESS)
	{
	    printf("\nFailed to bind to QM1 interrupt");
	    return (IX_FAIL);
	}
    }
    else	/* Polled mode */
    {
 	    threadAttr.name = pThreadName;
        threadAttr.stackSize = IX_DMA_CODELET_QMGR_STACK_SIZE;
        threadAttr.priority = IX_DMA_CODELET_QMR_PRIORITY;
 	    
 	    if (ixOsalThreadCreate(&ixDmaAccCodeletDispatchId,
 	    		&threadAttr,
 	    		(IxOsalVoidFnVoidPtr)ixDmaAccCodeletDispatcherPoll,
				NULL
				) != IX_SUCCESS)
	    {
	        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
	        		   IX_OSAL_LOG_DEV_STDERR,
	        		   "\nError spawning dispatch task",
	        		   0,0,0,0,0,0);
	        return (IX_FAIL);
	    }
	    	    
	   	if (ixOsalThreadStart (&ixDmaAccCodeletDispatchId) != IX_SUCCESS)
	    {
	    	ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
	        		   IX_OSAL_LOG_DEV_STDERR,
	        		   "\nError starting spawing dispatch task",
	        		   0,0,0,0,0,0);
	        		   
	    	return (IX_FAIL);
	    }
    }
    return (IX_SUCCESS);
}

/*
 * Function definition : ixDmaAccCodeletNpeInit()
 * See header file for documentation.
 */
PRIVATE IX_STATUS ixDmaAccCodeletNpeInit(IxNpeDlNpeId npeId)
{
    UINT32 imageId;        /* Storage for single image Id */

    /* Check for invalid npeId value */
    /* Set the image Id for DMA component */
    switch (npeId)
    {
        case IX_NPEDL_NPEID_NPEA :
        imageId = IX_DMA_CODELET_NPE_A_IMAGEID;
        break;
        case IX_NPEDL_NPEID_NPEB :
        imageId = IX_DMA_CODELET_NPE_B_IMAGEID;
        break;
        case IX_NPEDL_NPEID_NPEC :
        imageId = IX_DMA_CODELET_NPE_C_IMAGEID;
        break;
        default:
            /* Invalid NPE ID */
            ixOsalLog (IX_OSAL_LOG_LVL_ALL,
            		   IX_OSAL_LOG_DEV_STDOUT,	
                       "\nDMAAccDemoNpeInit : invalid Npe ID.",
                       0,0,0,0,0,0);
            return (IX_FAIL);
    } /* end of switch(npeId) */

    /* Download NPE code */
    if(ixNpeDlNpeInitAndStart(imageId) != IX_SUCCESS)
    {
        printf("\nNPE download failed");
        return(IX_FAIL);
    }
    return(IX_SUCCESS);
}

/*
 * Function definition : ixDmaAccCodeletShow()
 * See header file for documentation.
 */

void
ixDmaAccCodeletShow(void)
{
    UINT32 i;       /* Counter for memory dump loop */

    /* Invalidate cache for source block before a read to ensure consistency */
    IX_OSAL_CACHE_INVALIDATE(ixDmaAccCodeletSrcBlock,IX_DMA_CODELET_MEMDUMPSIZE);

    /* Show memory dump for source test array */
    for(i = 0; i < IX_DMA_CODELET_MEMDUMPSIZE; i++)
    {
        if ( i%16 == 0 )       /* Show newline and address for every 16 bytes */
        {
            printf ("\n Src Addr %02x : ", (UINT32) &ixDmaAccCodeletSrcBlock[i]);
        }
        printf ("%02x ", (UINT32) ixDmaAccCodeletSrcBlock[i]);  /* Show raw memory byte */
    }

    /* Invalidate cache for destination block before a read access to ensure consistency */
    IX_OSAL_CACHE_INVALIDATE(ixDmaAccCodeletDestBlock,IX_DMA_CODELET_MEMDUMPSIZE);

    /* Show memory dump for destination test array */
    for(i = 0; i < IX_DMA_CODELET_MEMDUMPSIZE; i++)
    {
        if ( i%16 == 0 )       /* Show newline and address for every 16 bytes */
        {
            printf ("\n Dst Addr %02x : ", (UINT32) &ixDmaAccCodeletDestBlock[i]);
        }
        printf ("%02x ", (UINT32) ixDmaAccCodeletDestBlock[i]);  /* Show raw memory byte */
    }
    printf ("\n");
    return;
}

/*
 * Function definition : ixDmaAccCodeletCallback()
 * See header file for documentation.
 */
void
ixDmaAccCodeletCallback(IX_STATUS status)
{
    /* Stop the time */
    ixDmaAccCodeletTimeStore.stopTime[ixDmaAccCodeletLoopCount] = ixOsalTimestampGet();

    /* Set the CallBack return flag to true */
    ixDmaAccCodeletCallBackReturnFlag = TRUE;

    /* Set the CallBackStatus */
    ixDmaAccCodeletCallBackStatus = status;
    
    return;
}

/*
 * Function definition : ixDmaAccCodeletTestPatternReset()
 * See header file for documentation.
 */
PRIVATE void ixDmaAccCodeletTestPatternReset(void)
{
    /* Counter for Test For-Loops */
    UINT32 i,j;

    /* Test pattern is loaded into a temporary array here */
    UINT8 testPattern[IX_DMA_CODELET_TESTPATTERN_LENGTH]
          = IX_DMA_CODELET_TESTPATTERN_LIST;

    /* Initialise the values in the Source Array */
    for ( i = 0;
          i < IX_DMA_CODELET_TEST_MAXLENGTH;
          i += IX_DMA_CODELET_TESTPATTERN_LENGTH)
    {
        for ( j = 0;
              j < IX_DMA_CODELET_TESTPATTERN_LENGTH;
              j += 1)
        {
            ixDmaAccCodeletSrcBlock[i+j] = testPattern[j];
        }
    }

    /* Flush the cache before performing a write to memory block */
    IX_OSAL_CACHE_FLUSH(ixDmaAccCodeletSrcBlock,IX_DMA_CODELET_TESTPATTERN_LENGTH);

    /* Initialise the values in the Destination Array */
    ixOsalMemSet( ixDmaAccCodeletDestBlock,
            0xFF,
            IX_DMA_CODELET_TEST_MAXLENGTH);

    /* Flush the cache before performing a write to memory block */
    IX_OSAL_CACHE_FLUSH(ixDmaAccCodeletDestBlock,IX_DMA_CODELET_TESTPATTERN_LENGTH);

    return;
}

/*
 * Function definition: ixDmaAccCodeletReportAverageTime() 
 * It is used to calculate and display the average time for 
 * PERFORMANCE_NUM_LOOP (i.e. 100 runs) 
 */ 
PRIVATE void ixDmaAccCodeletReportAverageTime(UINT16 tLength)
{
    UINT32 temp_count = 0; 
    UINT32 diffTime = 0; 
    UINT32 averageTick=0;
    UINT32 stopTime;
    UINT32 startTime;

    for(temp_count=0;temp_count < PERFORMANCE_LOOP_NUM; temp_count++)
    {
        stopTime = ixDmaAccCodeletTimeStore.stopTime[temp_count];
        startTime = ixDmaAccCodeletTimeStore.startTime[temp_count];

	/* Check if the timer wrap over */
	if (stopTime < startTime)
	{
	    diffTime = (0xffffffff -  stopTime + startTime);
	}
	else
	{
	    diffTime = stopTime-startTime;
	}

	averageTick =  averageTick + diffTime;
    }
        
    averageTick = (averageTick/PERFORMANCE_LOOP_NUM) / 
	           (IX_DMA_CODELET_XSCALE_TICK);

    printf ("\nAverage Rate (in Mbps) : ");
    ratioPrintf (IX_DMA_CODELET_DECIMAL_POINT, 
		 tLength*IX_DMA_CODELET_EIGHT_BITS,
		 averageTick);
    printf ("\n===============================================\n\n");

    return;
}

/*
 * Function definition: ratioPrintf ()
 * It is used to display the ratio between two parameters
 */ 
PRIVATE void ratioPrintf (int decimalPoint, UINT64 param1, UINT64 param2)
{
    UINT64 number = IX_OSAL_UDIV64_32(param1, param2);
    unsigned char tempStr[30];
    unsigned char *pStr = &tempStr[29];
    int count = 0;
    
    *pStr = 0;
    while (number != 0)
    {
	count++;
	*(--pStr) = IX_OSAL_UMOD64_32(number, 10) + '0';
	number = IX_OSAL_UDIV64_32(number, 10);
    }

    if (*pStr == 0)
    {
	count++;
	*(--pStr) = '0';
    }

    while (count < 10)
    {
	count++;
	*(--pStr) = ' ';
    }

    printf ("%s.", pStr); 
    param1 = IX_OSAL_UMOD64_32(param1, param2);
    while (decimalPoint-- != 0)
    {
	param1 *= 10;
	printf ("%d", (unsigned int) IX_OSAL_UDIV64_32(param1, param2));
	param1 = IX_OSAL_UMOD64_32(param1, param2);
    }
}

/*
 * Function definition : ixDmaAccCodeletTestPerform()
 * See header file for documentation.
 */
IX_STATUS ixDmaAccCodeletTestPerform( UINT16 transferLength,
				      IxDmaTransferMode transferMode,
				      IxDmaAddressingMode addressingMode,
				      IxDmaTransferWidth transferWidth)
{
    UINT32 transferCounter;
    UINT32 counterTimeOut = 0;
    IX_STATUS retval;

    /* Initialise Source Address to ixDmaAccCodeletSrcBlock */
    UINT32 sourceAddr = (UINT32) ixDmaAccCodeletSrcBlock;

    /* Initialise Destination Address to ixDmaAccCodeletDestBlock */
    UINT32 destinationAddr = (UINT32) ixDmaAccCodeletDestBlock;

    /* Initialise Test Patterns */
    ixDmaAccCodeletTestPatternReset();

    printf ("\nTransferred %d times with the following parameters", PERFORMANCE_LOOP_NUM);
    printf ("\nSource Address        : 0x%x", sourceAddr);
    printf ("\nDestination Address   : 0x%x", destinationAddr);
    printf ("\nTransfer Length       : %d", transferLength);
    printf ("\nTransfer Mode         : %s", ixDmaAccCodeletMsgTM[transferMode]);
    printf ("\nAddressing Mode       : %s", ixDmaAccCodeletMsgAM[addressingMode]);
    printf ("\nTransfer Width        : %s", ixDmaAccCodeletMsgTW[transferWidth]);
    printf ("\n\n%d byte memory dump before transfer :", IX_DMA_CODELET_MEMDUMPSIZE );

    /* Show memory dump before transfer */
    ixDmaAccCodeletShow();

   /* Do Dma transfer for PERFORMANCE_NUM_LOOP (i.e. 100 runs) for each 
      different type of configuration */
    for (transferCounter = 0; transferCounter < PERFORMANCE_LOOP_NUM; transferCounter++)
    {
        /* Initialise Test Patterns */
        ixDmaAccCodeletTestPatternReset();
    
        ixDmaAccCodeletLoopCount = transferCounter;
	ixDmaAccCodeletTimeStore.startTime[transferCounter] = ixOsalTimestampGet();

	/* Perform Dma Transfer */
	/* Callback function will update the status and CallBackReturn flag */
	retval = ixDmaAccDmaTransfer( (IxDmaAccDmaCompleteCallback) ixDmaAccCodeletCallback,
				      sourceAddr,
				      destinationAddr,
				      transferLength,
				      transferMode,
				      addressingMode,
				      transferWidth );

	/* Go into while loop if FIFO FULL happens */
	while (IX_DMA_REQUEST_FIFO_FULL == retval)
	{
	    printf ("\nFIFO is FULL. Going to sleep mode ");
	    /*Delay in 1 MSecond before Dma Request Q recovers */
	    ixOsalSleep(IX_CLIENT_SLEEP_IN_MS);

	    counterTimeOut++;
	    if (IX_DMA_CODELET_MAX_TIME_OUT == counterTimeOut)
	    {	       
		/* Stop the operation and return fail */
		printf("\nTime out on FIFO full");
		return IX_FAIL;
	    }

	    /* Retransmit after 1ms */
	    retval = ixDmaAccDmaTransfer( (IxDmaAccDmaCompleteCallback) ixDmaAccCodeletCallback,
					  sourceAddr,
					  destinationAddr,
					  transferLength,
					  transferMode,
					  addressingMode,
					  transferWidth );	
	}
	
	/* If retval is not successful returns failure */
        if (IX_DMA_SUCCESS != retval)
	{    
	    printf("\nDma Transfer fail");
	    return IX_FAIL;
	}

	/* set to 0 used for callback return count time out */
	counterTimeOut = 0;

	/* Wait for callback to return */
	while (!ixDmaAccCodeletCallBackReturnFlag)
	{
	    /*wait until current transfer complete */
	    ixOsalSleep(IX_CLIENT_SLEEP_IN_MS);
	    counterTimeOut++;
	    
	    if (IX_DMA_CODELET_MAX_TIME_OUT == counterTimeOut)
	    {	       
		/* Stop the operation and return fail */
		printf("\nTime out on callback");
		return IX_FAIL;
	    }
	}

	/* set counter to 0 so that it will be used by the next iteration */
	counterTimeOut = 0;

	/* When callback is returned, set flag to false */
	ixDmaAccCodeletCallBackReturnFlag = FALSE;

    }/* End of for-loop for transfer counter*/

    /* At the end of 100 runs and after callback returned successfully then
       do show here */
    if (IX_SUCCESS == ixDmaAccCodeletCallBackStatus)
    {
	printf ("\n\n%d byte memory dump after transfer :", IX_DMA_CODELET_MEMDUMPSIZE );
	/* Memory dump to show result of the current transfer */
	ixDmaAccCodeletShow();
	
	/* Print out the average time for that configuration */
	ixDmaAccCodeletReportAverageTime(transferLength);
    }

    return IX_SUCCESS;
}

/*
 * Function definition : ixDmaAccCodeletMain(void)
 * See header file for documentation.
 */
IX_STATUS ixDmaAccCodeletMain(void)
{
    UINT16 tLength[]={8,1024,16384,32768,65528};    
    IxDmaTransferMode tMode;
    IxDmaAddressingMode aMode;
    IxDmaTransferWidth tWidth;
    UINT16 localIndex;
    IX_STATUS retval;

    printf("Load DmaAcc Codelet\n");
    ixDmaAccCodeletInit(IX_NPEDL_NPEID_NPEA);

    /*Code for Data Transfer*/
    for (aMode = IX_DMA_INC_SRC_INC_DST; aMode <= IX_DMA_FIX_SRC_INC_DST; aMode++)
    {
	for(tMode = IX_DMA_COPY_CLEAR; tMode <= IX_DMA_COPY_REVERSE; tMode++)
	{
	    for(tWidth = IX_DMA_32_SRC_32_DST; tWidth <= IX_DMA_BURST_SRC_BURST_DST; tWidth++)
	    {
		for(localIndex = 0; localIndex < 5; localIndex++)
		{
		    /* Do test perform for each configuration */
		    retval = ixDmaAccCodeletTestPerform(tLength[localIndex], tMode, aMode, tWidth);

		    if (IX_SUCCESS != retval)
		    {
			return IX_FAIL;
		    }

		}/*End of lengthCode*/

		 /* Exclusion for invalid cases. In this address mode 
		    8_SRFC_BURST_DST, 16_SRC_BURST_DST, 32_SRC_BURST_DST
		    and BURST_SRC_BURST_DST are not valid */
		if(IX_DMA_INC_SRC_FIX_DST == aMode)
		{		    
		    if (IX_DMA_8_SRC_8_DST == tWidth)
		    { 
			tWidth =IX_DMA_32_SRC_BURST_DST; 
		    }
		    else  if (IX_DMA_BURST_SRC_32_DST == tWidth)
		    { 
			tWidth = IX_DMA_TRANSFER_WIDTH_INVALID;
		    }
		}
		
		/* Exclusion for invalid cases. In this address mode
		   BURST_SRC_8_DST, BURST_SRC_16_DST, BURST_SRC_32_DST
		   and BURST_SRC_BURST_DST are not valid */
		if(IX_DMA_FIX_SRC_INC_DST == aMode)
		{
		    if ( IX_DMA_32_SRC_BURST_DST == tWidth)
		    { 
			tWidth = IX_DMA_TRANSFER_WIDTH_INVALID;
		    }
		}
	    }/*End of tWidth*/
	} /*End of tMode*/
    } /*End of aMode*/

    return IX_SUCCESS;
}

