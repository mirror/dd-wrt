/**
 * @file IxParityENAccCodelet.c
 *
 * @author Intel Corporation
 *
 * @date 01 June 2005
 *
 * @brief  Codelet for IXP46X Parity Error Notifier access component.
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

#if defined(__ixp46X) || defined(__ixp43X)

/********************************************************************* 
 *	user include file 
 *********************************************************************/

#include "IxParityENAccCodelet.h"

/*********************************************************************
 *	private function prototype
 *********************************************************************/

PRIVATE IX_STATUS
ixParityENAccCodeletECCErrorInject (
	BOOL multiBit,	
	BOOL injectLater);

PRIVATE IX_STATUS
ixParityENAccCodeletParityConfigure (IxParityENAccHWParityConfig *pENConfig);

PRIVATE void
ixParityENAccCodeletSDRAMScanTask (void);

PRIVATE IX_STATUS 
ixParityENAccCodeletNewThreadCreate (
	IxOsalVoidFnPtr func, 
	UINT32 priority,
	char *label);

PRIVATE IX_STATUS
ixParityENAccCodeletSDRAMScrub (
	IxParityENAccParityErrorContextMessage *pENContext);

PRIVATE void
ixParityENAccCodeletParityErrActionPerformed (
	IxParityENAccParityErrorContextMessage *pENContext);

PRIVATE void
ixParityENAccCodeletParityErrHandler (void);

#ifdef __vxworks
	PRIVATE void
	ixParityENAccCodeletDataAbortHandler (void);
#endif

PRIVATE void
ixParityENAccCodeletReboot (void);

PRIVATE void 
ixParityENAccCodeletShutDownTask (void);


/*********************************************************************
 *	PRIVATE variable
 *********************************************************************/

PRIVATE BOOL ixParityENAccCodeletTerminate;
PRIVATE UINT32 ixParityENAccCodeletMcuRegBaseAddr = 0;
PRIVATE UINT32 *memAddr4ECCErrorInjection = NULL;
PRIVATE IxOsalSemaphore ixParityENAccCodeletRebootSemId = NULL;
PRIVATE char *ixParityENAccCodeletSourceLabel[] = 
{
    "NPE A IMEM",
    "NPE A DMEM",
    "NPE A External",
    "NPE B IMEM",
    "NPE B DMEM",
    "NPE B External",
    "NPE C IMEM",
    "NPE C DMEM",
    "NPE C External",
    "SWCP",
    "AQM",
    "DDR MCU Single bit",
    "DDR MCU Multi bit",
    "DDR MCU Overflow",
    "PCI Initiator",
    "PCI Target",
    "EBC Chip Select",
    "EBC External Master"
};

PRIVATE char *ixParityENAccCodeletAccessTypeLabel[] =
{
    "READ",
    "WRITE"
};

PRIVATE char *ixParityENAccCodeletRequesterLabel[] =
{
    "Memory Port Interface",
    "North/South AHB Bus"
};

/* 
 * parityENAcc configuration buffer. All components' parity error detection  
 * are set to disable in this buffer.
 */ 
PRIVATE IxParityENAccHWParityConfig ixParityENAccCodeletConfig =
{
    {IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY}, /* NPE-A */ 
    {IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY}, /* NPE-B */
    {IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY}, /* NPE-C */
    {IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,      /* MCU */
     IX_PARITYENACC_DISABLE},
     IX_PARITYENACC_DISABLE,                              /* SWCP */
     IX_PARITYENACC_DISABLE,                              /* AQM */
    {IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE},     /* PBC */    
    {IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,      /* EBC */
     IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
     IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
     IX_PARITYENACC_DISABLE, IX_PARITYENACC_DISABLE,
     IX_PARITYENACC_DISABLE, IX_PARITYENACC_EVEN_PARITY}
};

/*********************************************************************
 *	PUBLIC functions
 *********************************************************************/

PUBLIC IX_STATUS
ixParityENAccCodeletMain (
	BOOL multiBit,		/* 0 - single bit ECC error, 1 - multi bit ECC error */
	BOOL injectNow)		/* 0 - inject ECC error later, 1 - inject ECC error now */
{
	IxParityENAccStatus pENStatus;
	IxParityENAccHWParityConfig pENConfig;

	/* set termination flag to false */
	ixParityENAccCodeletTerminate = FALSE;

#ifdef __vxworks
	/* 
	 * disable data caching. 
	 * If the data is cached, bad ECC generation on SDRAM memory may fail.
	 */
	cacheDisable (DATA_CACHE);
#endif

	/* initialize local parityENAcc configuration buffer */
	ixOsalMemCopy (&pENConfig, &ixParityENAccCodeletConfig, sizeof (IxParityENAccHWParityConfig));

	/* initialize Parity Error Notifier Access Component */
	pENStatus = ixParityENAccInit ();
	if ((IX_PARITYENACC_SUCCESS != pENStatus) && (IX_PARITYENACC_ALREADY_INITIALISED != pENStatus))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletMain: failed to initialize ParityENAcc, error code %d\n",
					pENStatus, 0, 0, 0, 0, 0);
		return IX_FAIL;
	} 

	/* setup callback to handle parity error */
	pENStatus = ixParityENAccCallbackRegister (ixParityENAccCodeletParityErrHandler);
	if (IX_PARITYENACC_SUCCESS != pENStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletMain: Failed to register parity error callback, error code %d\n",
					pENStatus, 0, 0, 0, 0, 0);
		return IX_FAIL;
	} 

	/* enable MCU parity error detection */
	pENConfig.mcuConfig.singlebitDetectEnabled = IX_PARITYENACC_ENABLE;
	pENConfig.mcuConfig.singlebitCorrectionEnabled = IX_PARITYENACC_ENABLE;
	pENConfig.mcuConfig.multibitDetectionEnabled = IX_PARITYENACC_ENABLE;
	if (IX_SUCCESS != ixParityENAccCodeletParityConfigure (&pENConfig))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletMain: Failed to enable MCU parity error detection\n",
					0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}    

	if (IX_SUCCESS != ixOsalSemaphoreInit (&ixParityENAccCodeletRebootSemId, 0))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletMain: failed to create semaphore\n",
			0, 0, 0, 0, 0, 0);
		
		return IX_FAIL;
	}

	/* spawn "Shut Down" task */
	if (IX_SUCCESS != 
	    ixParityENAccCodeletNewThreadCreate (ixParityENAccCodeletShutDownTask, 
						 IX_OSAL_MAX_THREAD_PRIORITY, 
						 "Shut Down"))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletMain: failed to start Shut Down thread\n",
					0, 0, 0, 0, 0, 0);

		/* terminate parityENAcc codelet execution */
		ixParityENAccCodeletQuit ();

		return IX_FAIL;
	}

	/* 
	 * connect ixParityENAccCodeletDataAbortHandler() to DATA ABORT exception 
	 * vector, so that ixParityENAccCodeletDataAbortHandler() will be called 
	 * whenever DATA ABORT occurs.
	 */
	#ifdef __vxworks
		excVecSet ((FUNCPTR *)EXC_OFF_DATA, (FUNCPTR)ixParityENAccCodeletDataAbortHandler);
	#endif
 
	ixOsalLog (IX_OSAL_LOG_LVL_DEBUG1, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletMain: ParityENAcc is initialized\n", 
				0, 0, 0, 0, 0, 0);

	/* inject ECC error */
	if (IX_SUCCESS != ixParityENAccCodeletECCErrorInject (multiBit, injectNow))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletMain: Failed to inject ECC error\n",
					0, 0, 0, 0, 0, 0);
		/* terminate parityENAcc codelet execution */
		ixParityENAccCodeletQuit ();

		return IX_FAIL;
	}    

	/* start SDRAM memory scan at background */
	if (IX_SUCCESS != 
	    ixParityENAccCodeletNewThreadCreate (ixParityENAccCodeletSDRAMScanTask, 
						 IX_OSAL_DEFAULT_THREAD_PRIORITY,
						 "SDRAM Scan"))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletMain: Failed to start SDRAM memory scan\n",
			0, 0, 0, 0, 0, 0);

		/* terminate parityENAcc codelet execution */
		ixParityENAccCodeletQuit ();

		return IX_FAIL;
	}

	return IX_SUCCESS;

} /* end of ixParityENAccCodeletMain function */


PUBLIC void 
ixParityENAccCodeletQuit ()
{
	IxParityENAccHWParityConfig pENConfig;

	/* set termination flag  */
	ixParityENAccCodeletTerminate = TRUE;

	/* initialize local parityENAcc configuration buffer */
	ixOsalMemCopy (&pENConfig, &ixParityENAccCodeletConfig, sizeof (IxParityENAccHWParityConfig));

	/* disable MCU parity error detection */
	if (IX_SUCCESS != ixParityENAccCodeletParityConfigure (&pENConfig))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletQuit: Failed to disable MCU parity error detection\n",
			0, 0, 0, 0, 0, 0);
	}    

	/* 
	 * sleep to initiate task switch, so that "SDRAM Scan" thread will
	 * run and quit scanning.
	 */  
	ixOsalSleep (10);

	/* free allocated memory */
	if (NULL != memAddr4ECCErrorInjection)
	{
		IX_OSAL_CACHE_DMA_FREE (memAddr4ECCErrorInjection);

		memAddr4ECCErrorInjection = NULL;
	}

	/* unmap MCU register base address */
	if (0 != ixParityENAccCodeletMcuRegBaseAddr)
	{
		IX_OSAL_MEM_UNMAP (ixParityENAccCodeletMcuRegBaseAddr);

		ixParityENAccCodeletMcuRegBaseAddr = 0;
	}

	/* destroy semaphore */
	if (NULL != ixParityENAccCodeletRebootSemId)
	{
		/* give semaphore to get "Shut Down" thread to quit */
		if (IX_SUCCESS != ixOsalSemaphorePost (&ixParityENAccCodeletRebootSemId))
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletQuit: failed to give semaphore\n",
				0, 0, 0, 0, 0, 0);
		}

		/* 
	 	 * sleep to initiate task switch, so that "Shut Down" thread will
	 	 * run and exit. 
	 	 */  
		ixOsalSleep (10);

		if (IX_SUCCESS != ixOsalSemaphoreDestroy (&ixParityENAccCodeletRebootSemId))
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletQuit: failed to destroy semaphore\n",
				0, 0, 0, 0, 0, 0);
		}
		ixParityENAccCodeletRebootSemId = NULL;
	}

#ifdef __vxworks
	/* enable data caching */
	cacheEnable (DATA_CACHE);
#endif

} /* end of ixParityENAccCodeletQuit function */

 
/*********************************************************************
 *	PRIVATE functions
 *********************************************************************/

/**
 * @ingroup IxParityENAccCodelet
 *
 * @fn ixParityENAccCodeletECCErrorInject (
		BOOL multiBit,
		BOOL injectNow)
 *
 * @brief  This function injects ECC error on allocated memory.
 *
 * Memory Controller provides a ECC Test Register (Hex Offset
 * Address = 0xCC00E530) that allows bad ECC generation for ECC
 * testing. To generate bad ECC, first write a 8-bit non-zero
 * value to this test register. This 8-bit non-zero value will
 * be XORed with the ECC generated by subsequent writes, and the
 * computed ECC (bad) will be written to memory. Thus, any 
 * subsequent reads from this memory will result in ECC error.
 * The user needs to provide two parameters to specify how
 * the ECC error should be generated. First, the user has to 
 * specify the type of ECC error injection - single bit or  
 * multi bit. Second, the user has to tell this function when to 
 * cause ECC error after bad ECC generation.
 *	  
 * @param multiBit BOOL [in] - type of ECC error injection.
 *	<LI> FALSE : Single bit ECC error.
 *	<LI> TRUE  : Multi-bit ECC error.
 * 
 * @param injectNow BOOL [in] - preference for when to generate 
 *		ECC error.
 *	<LI> FALSE : This function will only generate bad ECC 
 * 		     on allocated memory. ECC error will only occur
 *		     when the memory is read later.
 *	<LI> TRUE : After generating bad ECC, this function will  
 *		    immediately read the memory to cause ECC error.
 *
 * @return  IX_STATUS
 *          @li IX_SUCCESS - inject bad ECC successfully
 *          @li IX_FAIL    - fails
 */

PRIVATE IX_STATUS
ixParityENAccCodeletECCErrorInject (
	BOOL multiBit,
	BOOL injectNow)
{
	UINT32 *tstECCRegAddr; 

	/* allocate SDRAM memory for ECC error injection */	
	memAddr4ECCErrorInjection = (UINT32 *) IX_OSAL_CACHE_DMA_MALLOC (sizeof (UINT32));

	if (NULL == memAddr4ECCErrorInjection)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletECCErrorInject: Failed to allocate memory for ECC error injection\n",
					0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}
 
	ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletECCErrorInject: inject ECC error at %08x, multiBit %d injectNow %d\n",
				(UINT32)memAddr4ECCErrorInjection, multiBit, injectNow, 0, 0, 0); 

	/* initialize the value */
	IX_OSAL_WRITE_LONG (memAddr4ECCErrorInjection, 0); 

	ixParityENAccCodeletMcuRegBaseAddr = (UINT32) IX_OSAL_MEM_MAP (IX_OSAL_IXP400_PARITYEN_PHYS_BASE,
				IX_OSAL_IXP400_PARITYEN_MAP_SIZE);

	if (0 == ixParityENAccCodeletMcuRegBaseAddr)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletECCErrorInject: Failed to get ECC Register Base Address\n",
					0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	tstECCRegAddr = (UINT32 *) (ixParityENAccCodeletMcuRegBaseAddr + IX_PARITYENACC_CODELET_ECC_TEST_REG_OFFSET);

	/* configure ECC Test Register to generate single bit bad ECC */
	if (FALSE == multiBit)		
	{
		IX_OSAL_WRITE_LONG (tstECCRegAddr,
					IX_PARITYENACC_CODELET_SINGLE_BIT_ERROR_BIT_0_SYNDROME);
	}
	/* configure ECC Test Register to generate multi bit bad ECC */
	else
	{
		IX_OSAL_WRITE_LONG (tstECCRegAddr,
					IX_PARITYENACC_CODELET_MULTI_BIT_ERROR_SYNDROME);
	}
	/* perform write to generate bad ECC */
	IX_OSAL_WRITE_LONG (memAddr4ECCErrorInjection, 0);
   
	/* configure ECC Test Register to stop generating bad ECC */ 
	IX_OSAL_WRITE_LONG (tstECCRegAddr, 0); 

	/* perform read to cause ECC error */
	if (TRUE == injectNow)
	{
		IX_OSAL_READ_LONG (memAddr4ECCErrorInjection);
	}

	return IX_SUCCESS;

} /* end of ixParityENAccCodeletECCErrorInject function */


/**
 * @ingroup IxParityENAccCodelet
 *
 * @fn ixParityENAccCodeletParityConfigure (
 		IxParityENAccHWParityConfig *pENConfig)
 *
 * @brief  Modify parity configuration. After modification, check the   
 *	   configuration to make sure it is configured as desired. 
 *
 * @param pENConfig IxParityENAccHWParityConfig * [in] - pointer to 
 *		IxParityENAccHWParityConfig buffer.
 *
 * @return  IX_STATUS
 *          @li IX_SUCCESS - configure parityENAcc successfully
 *          @li IX_FAIL    - fail
 */
PRIVATE IX_STATUS
ixParityENAccCodeletParityConfigure (IxParityENAccHWParityConfig *pENConfig)
{
	IxParityENAccStatus		pENStatus;
	IxParityENAccHWParityConfig	pENConfigTmp;

	if (NULL == pENConfig)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletParityConfigure: pass in pointer is null\n",
					0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	/* configure Parity Error Notifier with new setting */
	pENStatus = ixParityENAccParityDetectionConfigure (pENConfig);
	if (IX_PARITYENACC_SUCCESS != pENStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletParityConfigure: Failed to configure ParityENAcc, error code %d\n",
					pENStatus, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}    

	/* query to retrieve recently configured parity configuration */
	pENStatus = ixParityENAccParityDetectionQuery (&pENConfigTmp);
	if (IX_PARITYENACC_SUCCESS != pENStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletParityConfigure: Failed to query ParityENAcc config, error code %d\n", 
					pENStatus, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}  

	/* make sure parity configuration is configured as desired */ 
	if (0 != memcmp ((void *)pENConfig, (void *)&pENConfigTmp, 
			sizeof (IxParityENAccHWParityConfig)))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletParityConfigure: ParityENAcc was not configured as desired\n",
					0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	return IX_SUCCESS;

} /* end of ixParityENAccCodeletParityConfigure function */


/**
 * @ingroup IxParityENAccCodelet
 *
 * @fn ixParityENAccCodeletSDRAMScanTask ()
 *
 * @brief  Perform SDRAM memory scan at background. The memory range for scanning will
 *	   be determined here at run time.
 *	
 * @return void
 *
 */
PRIVATE void
ixParityENAccCodeletSDRAMScanTask ()
{
	UINT32 addr, start, end;

#ifdef __linux
	struct sysinfo sysInfo;
	UINT32 size;

	/* initialize sysinfo buffer */
	ixOsalMemSet (&sysInfo, 0, sizeof (struct sysinfo));

	/* get total memory size */
	si_meminfo (&sysInfo);

	/* the totalram returned by si_meminfo is total memory less kernel reserved memory */
	size = ((UINT32) sysInfo.totalram) << PAGE_SHIFT;

	if (0 == size)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletSDRAMScanTask: failed to get memory size\n",
			0, 0, 0, 0, 0, 0);
	
		/* terminate parityENAcc codelet execution */
		ixParityENAccCodeletQuit ();

		return;
	}

	/* get memory scan start address */	
	start = (UINT32) IX_OSAL_MMU_PHYS_TO_VIRT (IX_PARITYENACC_CODELET_SDRAM_BASE_ADDR);

	/* compute memory scan end address, end address (0 relative) is size - 1 */
	end = (start - IX_PARITYENACC_CODELET_SDRAM_SCAN_START_OFFSET) + (size - 1);

#elif defined(__vxworks)

	/* get memory scan start address */	
	start = (UINT32) IX_OSAL_MMU_PHYS_TO_VIRT (IX_PARITYENACC_CODELET_SDRAM_BASE_ADDR);

	/* get memory scan end address */
	end = (UINT32) IX_OSAL_MMU_PHYS_TO_VIRT (sysMemTop ());

	if (0 == end)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletSDRAMScanTask: failed to get memory end address\n",
			0, 0, 0, 0, 0, 0);

		/* terminate parityENAcc codelet execution */
		ixParityENAccCodeletQuit ();

		return;
	}
#else

	#error unsupported OS
#endif
 
	ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletSDRAMScanTask: start %08x end %08x\n", 
				start, end, 0, 0, 0, 0);

	while (TRUE)
	{
		/* continuously read 4 bytes a time throughout the entire SDRAM */
		for (addr = start; addr < end - 4; addr+=4)
		{
			/* user requests to terminate SDRAM memory scan */
			if (TRUE == ixParityENAccCodeletTerminate)
			{
				ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletSDRAMScanTask: user terminated SDRAM memory scan.\n",
			0, 0, 0, 0, 0, 0);

				return;
			}

			/* 
			 * preempt this task after scanning one segment of memory.
			 * One segment is 256 Kb. 
			 */
			if (0 == (addr % IX_PARITYENACC_CODELET_SCAN_SEGMENT_SIZE))
			{
				/* sleep for 1 ms */	
				ixOsalSleep (1);
			}
			
			if (0 == (addr % IX_PARITYENACC_CODELET_DISPLAY_INTERVAL))
			{
				ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletSDRAMScanTask: scan SDRAM @ %08x\n", 
						addr, 0, 0, 0, 0, 0);
			}
 		
			IX_OSAL_READ_LONG (addr);
		}
	}
} /* end of ixParityENAccCodeletSDRAMScanTask function */ 


/**
 * @ingroup IxParityENAccCodelet
 *
 * @fn ixParityENAccCodeletNewThreadCreate (
 	IxOsalVoidFnPtr func, 
	UINT32 priority,
	char *label)
 *
 * @brief Spawn a thread to execute given function
 *
 * @param 
 * func IxOsalVoidFnPtr [in] - pointer to the function that will be
 *			       executed after the thread is spawned. 
 *
 * @param 
 * priority UINT32 [in] - thread's priority
 *
 * @param 
 * label char* [in] - pointer to the Thread name's buffer  
 *
 * @return IX_STATUS 
 *          @li IX_SUCCESS - create Thread successfully 
 *          @li IX_FAIL    - fai to create Thread
 */
PRIVATE IX_STATUS 
ixParityENAccCodeletNewThreadCreate (
	IxOsalVoidFnPtr func, 
	UINT32 priority,
	char *label)
{
	IxOsalThread memScanThread;
	IxOsalThreadAttr memScanThreadAttr;

	/* check the validity of function pointer */
	if (NULL == func)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletNewThreadCreate: NULL function pointer\n", 
			0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	/* check the validity of Thread name's buffer pointer */
	if (NULL == label) 
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletNewThreadCreate: NULL Thread name's pointer\n", 
			0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	/* zero out the thread attribute buffer */
	ixOsalMemSet ((void *)&memScanThreadAttr, 0, sizeof (IxOsalThreadAttr));

	/* setup thread attribute */
	memScanThreadAttr.name = label;
	memScanThreadAttr.priority = priority;
	
	if (IX_SUCCESS != ixOsalThreadCreate (&memScanThread, 
				&memScanThreadAttr,
				(IxOsalVoidFnVoidPtr)func,
				NULL))
	{ 
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletNewThreadCreate: Failed to create %s thread.\n", 
					(UINT32) label, 0, 0, 0, 0, 0);
				
		return IX_FAIL;
	}

	if (IX_SUCCESS != ixOsalThreadStart (&memScanThread))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletNewThreadCreate: Failed to start %s thread\n", 
					(UINT32) label, 0, 0, 0, 0, 0);
				
		return IX_FAIL;
	} 
	
	ixOsalLog (IX_OSAL_LOG_LVL_DEBUG1, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletNewThreadCreate: %s thread is spawned\n",
					(UINT32) label, 0, 0, 0, 0, 0);

	return IX_SUCCESS;

} /* end of ixParityENAccCodeletNewThreadCreate function */


/**
 * @ingroup IxParityENAccCodelet
 *
 * @fn ixParityENAccCodeletSDRAMScrub (
		IxParityENAccParityErrorContextMessage *pENContext)
 *
 * @brief  Scrub SDRAM memory to correct single bit ECC error. 
 *	   As example, this routine will scrub the entire row of 
 * 	   the data (Q-WORD aligned, 16 bytes) that failed.
 *
 * @param pENContext IxParityENAccParityErrorContextMessage * [in] - pointer to 
 *		IxParityENAccParityErrorContextMessage buffer.
 *
 * @return  IX_STATUS
 *	    @li IX_SUCCESS - scrub memory successfully
 *	    @li IX_FAIL    - fail
 */

PRIVATE IX_STATUS
ixParityENAccCodeletSDRAMScrub (IxParityENAccParityErrorContextMessage *pENContext)
{
	IX_STATUS			pENStatus;
	UINT32				count; 
	UINT32				data[4], *rowAddr = NULL, *scrubAddr = NULL;
	IxParityENAccHWParityConfig	pENConfig;

	if (NULL == pENContext)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletSDRAMScrub: pass in pointer is null\n",
					0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	/* 
	 * memory scrub is not required on write transaction as MCU 
	 * will scrub the memory automatically on write transaction.
	 */
	if (IX_PARITYENACC_WRITE == pENContext->pecAccessType)
	{
		return IX_SUCCESS;
	}
 
	/* get the current parity configuration */
	pENStatus = ixParityENAccParityDetectionQuery (&pENConfig);
	if (IX_PARITYENACC_SUCCESS != pENStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletSDRAMScrub: failed to query ParityENAcc config, error code %d\n",
			pENStatus, 0, 0, 0, 0, 0);	
		return IX_FAIL;
	} 

	/*
	 * disable single bit ECC error reporting during scrubbing.  
	 * The scrubbing routine will read the failed location to 
	 * correct single bit ECC error. This read will cause  
	 * second ECC error to be reported if single bit ECC error 
	 * reporting is not disabled during scrubbing. 
	 */
	pENConfig.mcuConfig.singlebitDetectEnabled = IX_PARITYENACC_DISABLE;
	pENStatus = ixParityENAccCodeletParityConfigure (&pENConfig);
	if (IX_SUCCESS != pENStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletSDRAMScrub: Failed to disable single bit ECC error reporting, error code %d\n", 
			pENStatus, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}   

	/* 
	 * get the Q-word aligned address of the row where the single bit 
	 * ECC error occurred.
	 */
	rowAddr = (UINT32 *) IX_OSAL_MMU_PHYS_TO_VIRT (pENContext->pecAddress & 
				IX_PARITYENACC_CODELET_QWORD_ALIGNED_MASK); 
	/* 
	 * reading the location that failed will trigger ECC hardware 
	 * to fix the error, and writing the data back will correct  
	 * the data on memory. 
	 */

	/* read entire row of data (16 bytes) */
	for (scrubAddr = rowAddr, count = 0; count < 4; count++)
	{
		data[count] = IX_OSAL_READ_LONG (scrubAddr);
		scrubAddr++;
	}

	/* write entire row of data back */ 
	for (scrubAddr = rowAddr, count = 0; count < 4; count++)
	{
		IX_OSAL_WRITE_LONG (scrubAddr, data[count]);
		scrubAddr++;
	}

	/* clear the single bit error */
	pENStatus = ixParityENAccParityErrorInterruptClear (pENContext);
	if (IX_PARITYENACC_SUCCESS != pENStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletSDRAMScrub: failed to clear interrupt, error code %d\n",
			pENStatus, 0, 0, 0, 0, 0);
	} 

	/* re-enable single bit ECC error reporting */
	pENConfig.mcuConfig.singlebitDetectEnabled = IX_PARITYENACC_ENABLE;
	pENStatus = ixParityENAccCodeletParityConfigure (&pENConfig);
	if (IX_SUCCESS != pENStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletSDRAMScrub: Failed to re-enable single bit ECC error reporting, error code %d\n",
					pENStatus, 0, 0, 0, 0, 0);
	}   
  
	ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletSDRAMScrub: virtual address %08x [physical address %08x] is scrubbed\n", 
				(UINT32) IX_OSAL_MMU_PHYS_TO_VIRT (pENContext->pecAddress), 
				pENContext->pecAddress, 0, 0, 0, 0); 
 
	return IX_SUCCESS;

} /* end of ixParityENAccCodeletSDRAMScrub function */


/**
 * @ingroup IxParityENAccCodelet
 *
 * @fn ixParityENAccCodeletParityErrActionPerformed (
		IxParityENAccParityErrorContextMessage *pENContext)
 *
 * @brief  Determine actions required to handle the parity error 
 *
 * The actions can be one or more of the followings : 
 *	(1) reboot the board 
 *	(2) scrub the memory
 *	(3) clear the interrupt  
 *	(4) log the error
 *
 * The clients need to define the most appropriate actions for
 * each parity error scenario in their application. 
 *
 * @param pENContext IxParityENAccParityErrorContextMessage * [in] - 
 *		pointer to IxParityENAccParityErrorContextMessage buffer.
 *
 * @return void
 */

PRIVATE void
ixParityENAccCodeletParityErrActionPerformed (
	IxParityENAccParityErrorContextMessage *pENContext)
{
	IxParityENAccStatus		pENStatus;
	IxParityENAccHWParityConfig	pENConfig;

	if (NULL == pENContext)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletParityErrActionPerformed: pass in pointer is null\n",
					0, 0, 0, 0, 0, 0);
		return;
	}

	switch (pENContext->pecParitySource)
	{
		/* single bit ECC error */
		case IX_PARITYENACC_MCU_SBIT:
		{

			ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletParityErrActionPerformed: single bit ECC error's detected @ address: %08x, Syndrome: %02x, Requester: %s, Access Type: %s\n",
				(UINT32) IX_OSAL_MMU_PHYS_TO_VIRT (pENContext->pecAddress),
				pENContext->pecData, 
				(UINT32) ((IX_PARITYENACC_WRITE >= pENContext->pecRequester) ? ixParityENAccCodeletRequesterLabel[pENContext->pecRequester] : "N/A"),
				(UINT32) ((IX_PARITYENACC_AHB_BUS >= pENContext->pecAccessType) ? ixParityENAccCodeletAccessTypeLabel[pENContext->pecAccessType] : "N/A"), 
				0, 0);

			/* get the parity configuration */
			pENStatus = ixParityENAccParityDetectionQuery (&pENConfig);
			if (IX_PARITYENACC_SUCCESS != pENStatus)
			{
				ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletParityErrActionPerformed: failed to query ParityENAcc config, error code %d\n",
							pENStatus, 0, 0, 0, 0, 0);	
				return;
			} 

			/* 
			 * scrub the memory if single bit ECC correction 
			 * is enabled.
			 */ 
			if (IX_PARITYENACC_ENABLE == pENConfig.mcuConfig.singlebitCorrectionEnabled)
			{
				if (IX_SUCCESS != ixParityENAccCodeletSDRAMScrub (pENContext))
				{
					ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletParityErrActionPerformed: failed to scrub the memory\n",
								0, 0, 0, 0, 0, 0);
				}
			}
			break;
		} /* end of case IX_PARITYENACC_MCU_SBIT */

		case IX_PARITYENACC_PBC_TARGET:
		case IX_PARITYENACC_EBC_EXTMST:
		{
			/* clear the interrupt */
			pENStatus = ixParityENAccParityErrorInterruptClear (pENContext);
			if (IX_PARITYENACC_SUCCESS != pENStatus)
			{
				ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletParityErrActionPerformed: Failed to clear interrupt, error code %d\n",
							pENStatus, 0, 0, 0, 0, 0);
			}
 
			ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletParityErrActionPerformed: %s parity error is detected @ address: %08x, Access Type: %s, Requester: %s, Data: %08x\n",
				(UINT32) ((IX_PARITYENACC_EBC_EXTMST >= pENContext->pecParitySource) ? ixParityENAccCodeletSourceLabel[pENContext->pecParitySource] : "N/A"),
				(UINT32) IX_OSAL_MMU_PHYS_TO_VIRT (pENContext->pecAddress),
				(UINT32) ((IX_PARITYENACC_WRITE >= pENContext->pecAccessType) ? ixParityENAccCodeletAccessTypeLabel[pENContext->pecAccessType] : "N/A"),
				(UINT32) ((IX_PARITYENACC_AHB_BUS >= pENContext->pecRequester) ? ixParityENAccCodeletRequesterLabel[pENContext->pecRequester] : "N/A"),
				pENContext->pecData, 0);


			break;
		} /* end of case IX_PARITYENACC_PBC_TARGET and IX_PARITYENACC_EBC_EXTMST */

		case IX_PARITYENACC_MCU_MBIT:	
		case IX_PARITYENACC_AQM:	
		case IX_PARITYENACC_EBC_CS:
		case IX_PARITYENACC_NPE_A_IMEM:
		case IX_PARITYENACC_NPE_A_DMEM:
		case IX_PARITYENACC_NPE_A_EXT:
		case IX_PARITYENACC_NPE_B_IMEM:
		case IX_PARITYENACC_NPE_B_DMEM:
		case IX_PARITYENACC_NPE_B_EXT:
		case IX_PARITYENACC_NPE_C_IMEM:
		case IX_PARITYENACC_NPE_C_DMEM:
		case IX_PARITYENACC_NPE_C_EXT:
		case IX_PARITYENACC_SWCP:
		case IX_PARITYENACC_MCU_OVERFLOW:
		case IX_PARITYENACC_PBC_INITIATOR:
		{
			/* clear the interrupt */
			pENStatus = ixParityENAccParityErrorInterruptClear (pENContext);
			if (IX_PARITYENACC_SUCCESS != pENStatus)
			{
				ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletParityErrActionPerformed: failed to clear interrupt, error code %d\n",
					pENStatus, 0, 0, 0, 0, 0);
			} 
	
			ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletParityErrActionPerformed: %s parity error is detected @ address: %08x, Access Type: %s, Requester: %s, Data: %08x\n",
				(UINT32) ((IX_PARITYENACC_EBC_EXTMST >= pENContext->pecParitySource) ? ixParityENAccCodeletSourceLabel[pENContext->pecParitySource] : "N/A"),
				(UINT32) IX_OSAL_MMU_PHYS_TO_VIRT (pENContext->pecAddress),
				(UINT32) ((IX_PARITYENACC_WRITE >= pENContext->pecAccessType) ? ixParityENAccCodeletAccessTypeLabel[pENContext->pecAccessType] : "N/A"),
				(UINT32) ((IX_PARITYENACC_AHB_BUS >= pENContext->pecRequester) ? ixParityENAccCodeletRequesterLabel[pENContext->pecRequester] : "N/A"),
				pENContext->pecData, 0);

			/* reboot the board */
			ixParityENAccCodeletReboot ();

			break;
		}
		default:
		{
			/* reboot the board */
			ixParityENAccCodeletReboot ();

		}
	} /* end of switch (pENContext->pecParitySource) */

} /* end of ixParityENAccCodeletParityErrActionPerformed function */


/**
 * @ingroup IxParityENAccCodelet
 *
 * @fn ixParityENAccCodeletParityErrHandler ()
 *
 * @brief  Handle parity error informed by Parity Error Notifier.
 *
 * This is a callback routine called by Parity Error Notifier when
 * parity error is detected. Interrupts can be stacked. It is possible
 * that multiple parity errors occurred at the same time. So, 
 * making sure all parity errors are handled is required here. 
 *
 * @return void
 */
PRIVATE void
ixParityENAccCodeletParityErrHandler ()
{
	IxParityENAccParityErrorContextMessage	pENContext;
	IxParityENAccStatus			pENStatus;
	UINT32					count = 0;

	do 
	{
		/* initialize IxParityENAccParityErrorContextMessage buffer */
		ixOsalMemSet (&pENContext, 0xFF, sizeof (IxParityENAccParityErrorContextMessage));

		/* get parity error context */
		pENStatus = ixParityENAccParityErrorContextGet (&pENContext);
		if (IX_PARITYENACC_SUCCESS == pENStatus)    
		{
			count++;
	
			/* handle legitimate parity error */
			ixParityENAccCodeletParityErrActionPerformed (&pENContext);
		}
		else if (IX_PARITYENACC_NO_PARITY != pENStatus)
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletParityErrHandler: Failed to get context, error code %d\n",
						pENStatus, 0, 0, 0, 0, 0); 
			return;
		}
	} while (IX_PARITYENACC_NO_PARITY != pENStatus);

	if (0 == count)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletParityErrHandler: no parity error is detected\n",
					0, 0, 0, 0, 0, 0);
	}

} /* end of ixParityENAccCodeletParityErrHandler function */


#ifdef __vxworks

#ifdef _DIAB_TOOL
__asm volatile UINT32 _ixParityENAccCodeletStatusRegGet(void)
{
! "r0"
    mrc\tp15, 0, r0, c5, c0, 0;
    /* return value is returned through register R0 */
}

__asm volatile UINT32 _ixParityENAccCodeletAddrRegGet(void)
{
! "r0"
    mrc\tp15, 0, r0, c6, c0, 0;
    /* return value is returned through register R0 */
}
#endif /* #ifdef _DIAB_TOOL */
/**
 * @ingroup IxParityENAccCodelet
 *
 * @fn ixParityENAccCodeletDataAbortHandler ()
 *
 * @brief  Handle multi-bit ECC error that caused data abort to occur
 *
 * This routine demonstrates how to determine whether the imprecise/precise
 * data abort is caused by multi-bit ECC error (accessed by XScale). If so, 
 * the multi-bit ECC error will be reported before rebooting the board. If
 * data abort is related to other type of parity error, then parity error 
 * will be handled. This routine does not intend to demonstrate how data 
 * abort handler should be implemented, so it does not attempt to recover
 * the data abort. In this data abort handler, the board will be rebooted 
 * at the end. 
 *
 * @return void
 */
PRIVATE void
ixParityENAccCodeletDataAbortHandler ()
{
	IxParityENAccParityErrorContextMessage	pENContext;
	IxParityENAccStatus			pENStatus;
	UINT32					statusReg, addrReg;
	BOOL					impreciseDataAbort;

	/* 
	 * read co-processor 15's register 5 (Fault Status Register) and 
	 * register 6 (Fault Address Register), and write their values
	 * to statusReg and addrReg, respectively.
	 */ 
#ifdef _DIAB_TOOL
        statusReg = _ixParityENAccCodeletStatusRegGet();
        addrReg = _ixParityENAccCodeletAddrRegGet();
#else
	__asm__ volatile ("mrc p15, 0, %0, cr5, cr0, 0;"
			"mrc p15, 0, %1, cr6, cr0, 0;"
			:"=&r"(statusReg), "=&r"(addrReg):);
#endif /* #ifdef _DIAB_TOOL */

	/* determine if the data abort is imprecise or precise */
	impreciseDataAbort = 
		IX_PARITYENACC_CODELET_BIT_MASK_CHECK (statusReg, IX_PARITYENACC_CODELET_EXTERNAL_DATA_ABORT) || 
		IX_PARITYENACC_CODELET_BIT_MASK_CHECK (statusReg, IX_PARITYENACC_CODELET_DATA_CACHE_PARITY_ERR);

	/* initialize IxParityENAccParityErrorContextMessage buffer */
	ixOsalMemSet (&pENContext, 0xFF, sizeof (IxParityENAccParityErrorContextMessage));

	/* get parity error context */
	pENStatus = ixParityENAccParityErrorContextGet (&pENContext);

	/* data abort is not related to parity error */
	if (IX_PARITYENACC_NO_PARITY == pENStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletDataAbortHandler: Data abort is not related to parity error\n",
					0, 0, 0, 0, 0, 0);
	}
	/* parity error did occur at the same time as data abort */
	else if (IX_PARITYENACC_SUCCESS == pENStatus)
	{
		/* imprecise data abort occurred */
		if (TRUE == impreciseDataAbort)
		{
			/* 
			 * XScale detected multi-bit ECC error and the 
			 * error caused imprecise data abort 
			 */
			if ((IX_PARITYENACC_MCU_MBIT == pENContext.pecParitySource) &&
			    (IX_PARITYENACC_MPI == pENContext.pecRequester))
			{
				ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletDataAbortHandler: XScale detected multi-bit ECC error @ address %08x and the error caused imprecise data abort\n",
					pENContext.pecAddress, 0, 0, 0, 0, 0); 
			}
			else
			{
				ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletDataAbortHandler: Imprecise data abort is not related to parity error, source: %s requester: %s\n",
					(UINT32) ((IX_PARITYENACC_EBC_EXTMST >= pENContext.pecParitySource) ? ixParityENAccCodeletSourceLabel[pENContext.pecParitySource] : "N/A"),
					(UINT32) ((IX_PARITYENACC_AHB_BUS >= pENContext.pecRequester) ? ixParityENAccCodeletRequesterLabel[pENContext.pecRequester] : "N/A"),
					0, 0, 0, 0);
			}
		} /* end of if (TRUE == impreciseDataAbort) */

		/* precise data abort occurred */
		else if (addrReg == pENContext.pecAddress)
		{
			ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletDataAbortHandler: XScale detected multi-bit ECC error @ address %08x and the error caused precise data abort\n",
				pENContext.pecAddress, 0, 0, 0, 0, 0); 
		}
		/* precise data abort is not related to parity error */
		else
		{
			ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixParityENAccCodeletDataAbortHandler: Precise data abort is not related to parity error, source: %s requester: %s address: %08x\n",
				(UINT32) ((IX_PARITYENACC_EBC_EXTMST >= pENContext.pecParitySource) ? ixParityENAccCodeletSourceLabel[pENContext.pecParitySource] : "N/A"),
				(UINT32) ((IX_PARITYENACC_AHB_BUS >= pENContext.pecRequester) ? ixParityENAccCodeletRequesterLabel[pENContext.pecRequester] : "N/A"),
				(UINT32) IX_OSAL_MMU_PHYS_TO_VIRT (pENContext.pecAddress),
				0, 0, 0);
		}

		/* handle parity error */
		ixParityENAccCodeletParityErrActionPerformed (&pENContext);    

	} /* end of else if (IX_PARITYENACC_SUCCESS == pENStatus) */
	else
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletDataAbortHandler: Data abort occurred but failed to get parity error context\n",
					0, 0, 0, 0, 0, 0); 
	}

	/* reboot the board here if the board is not yet rebooted */ 
	ixParityENAccCodeletReboot ();

} /* end of ixParityENAccCodeletDataAbortHandler function */

#endif	/* end of #ifdef __vxworks */


/**
 * @ingroup IxParityENAccCodelet
 *
 * @fn ixParityENAccCodeletReboot ()
 *
 * @brief   Wake up Shut Down Thread to reboot the board
 *	
 * @return void
 *
 */
PRIVATE void
ixParityENAccCodeletReboot ()
{
	if (IX_SUCCESS != ixOsalSemaphorePost (&ixParityENAccCodeletRebootSemId))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixParityENAccCodeletReboot: failed to give semaphore\n",
			0, 0, 0, 0, 0, 0);

		/* proceed with reboot if failing to wake up "Shut Down" task */
		IX_PARITYENACC_CODELET_REBOOT();
	}
} /* end of ixParityENAccCodeletReboot function */


/**
 * @ingroup IxParityENAccCodelet
 *
 * @fn ixParityENAccCodeletShutDownTask ()
 *
 * @brief   Sleep most of the time. If it's invoked, terminate parityENAcc codelet execution 
 *	    and then reboot the board
 *
 * In this codelet, the board is rebooted in a thread instead of ISR. The purpose is to give 
 * ixOsalLog opportunity to display data abort and multi-bit ECC handling information. 
 * These informational messages are the only indication that data abort and multi-bit ECC error 
 * do occur and are handled as designed. If the reboot is executed during ISR, the board 
 * will be rebooted before the printout of data abort and multi-bit ECC error handling messages 
 * (observed in vxWorks). Thus, the user may not know why the board is rebooting. 
 *
 * In summary, spawning a thread to reboot the board is only for demonsration purpose, it may
 * not be suitable for user application.
 *	
 * @return void
 *
 */
PRIVATE void 
ixParityENAccCodeletShutDownTask ()
{
	ixOsalSemaphoreWait (&ixParityENAccCodeletRebootSemId, IX_OSAL_WAIT_FOREVER);

	if (TRUE == ixParityENAccCodeletTerminate)
		return;

	ixParityENAccCodeletQuit ();

	/* 
	 * sleep for a while to give ixOsalLog opportunity to display data abort 
	 * and multi-bit ECC handling information 
	 */
	ixOsalSleep (500);

	IX_PARITYENACC_CODELET_REBOOT();

} /* end of ixParityENAccCodeletShutDownTask function */ 

#endif /* __ixp46X || __ixp43X */

