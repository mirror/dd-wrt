/**
 * @file IxEthAccCodeletDbLearning.c
 *
 * @date 22 April 2004
 *
 * @brief This file contains the implementation of the Ethernet Access 
 * Codelet that implements a test of MAC Address Learning and Aging 
 * configuration
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

/*
 * Put the system defined include files required.
 */
#include "IxOsal.h"

/*
 * Put the user defined include files required.
 */
#include "IxEthDB.h"
#include "IxEthAccCodelet.h"
#include "IxEthAccCodelet_p.h"

/*
 * Static local variables.
 */
PRIVATE IxOsalMutex ixEthAccCodeletDBMaintenanceTaskRunning;
PRIVATE volatile BOOL ixEthAccCodeletDBMaintenanceTaskStopTrigger;
PRIVATE void ixEthAccCodeletDBMaintenanceTask(void* arg, void** ptrRetObj);
PRIVATE BOOL ixEthAccCodeletDBMaintenanceInitialized = FALSE;


/**
 * @fn void ixEthAccCodeletDBMaintenanceTask
 *
 * This function calls the ixEthAccDatabaseMaintenance every 
 * IX_ETH_DB_MAINTENANCE_TIME seconds.
 * 
 * @return void
 */

PRIVATE void ixEthAccCodeletDBMaintenanceTask (void* arg, void** ptrRetObj)
{
    UINT32 count;

    ixEthAccCodeletDBMaintenanceTaskStopTrigger = FALSE;

    if (ixOsalMutexLock (&ixEthAccCodeletDBMaintenanceTaskRunning, IX_OSAL_WAIT_FOREVER) 
	!= IX_SUCCESS)
    {
        printf("DbLearning: Error starting Database Maintenance thread! Failed to lock mutex.\n");
        return;
    }

    while (1)
    {
	/* 
	 * The Database maintenance function must be called at a period 
	 * of approximately IX_ETH_DB_MAINTENANCE_TIME seconds 
	 * regardless of whether learning is enabled or not.
 	 */
        for (count = 0; count < IX_ETH_DB_MAINTENANCE_TIME; count++)
        { 
	    ixOsalSleep(1000); /* 1000 milliseconds */
	    if (ixEthAccCodeletDBMaintenanceTaskStopTrigger)
	    {
		break;  /* Exit the delay loop */
	    }
        }
	if (ixEthAccCodeletDBMaintenanceTaskStopTrigger)
	{
	    break;  /* Exit the thread loop */
	}

	ixEthDBDatabaseMaintenance();
    }

    ixOsalMutexUnlock (&ixEthAccCodeletDBMaintenanceTaskRunning);
}


/*
 * Function definition: ixEthAccCodeletDBLearningRun()
 *
 * Run ethDB demo for all ports passed as parameters
 */
IX_STATUS 
ixEthAccCodeletDBLearningRun(BOOL validPorts[])
{
    IxEthAccPortId portId;
    IxEthDBPortId portPtr;

    IxEthDBMacAddr staticMacAddress[IX_ETHACC_CODELET_MAX_PORT] = 
	{{{0,1,2,3,4,5}}, {{6,7,8,9,0,1}}};

    IxEthDBMacAddr dynamMacAddress[IX_ETHACC_CODELET_MAX_PORT] = 
	{{{0xa,0xb,0xc,0xd,0xe,0xf}}, {{0x0,0xb,0x3,0xc,0x4,0xd}}};

    if (ixEthAccCodeletDBMaintenanceStart()
	!= IX_SUCCESS)
    {
	printf("DbLearning: Error spawning DB maintenance task\n");
	return (IX_FAIL);
    }

    /* configure all existing ports */
    for (portId = 0; portId < IX_ETHACC_CODELET_MAX_PORT; portId++)
    {
	if (validPorts[portId])
	{
	    /* Configure the port */
	    printf("Configuring the port...\n");

	    if (ixEthAccCodeletPortConfigure(portId,
					     ixEthAccCodeletMemPoolFreeRxCB,
					     (IxEthAccPortMultiBufferRxCallback) NULL,
					     ixEthAccCodeletMemPoolFreeTxCB,
					     portId) 
		!= IX_ETH_ACC_SUCCESS)
	    {
		printf("DbLearning: Failed to configure the port %u\n",
		       (UINT32)portId);
		return IX_FAIL;
	    }

	    /* Enable the port */
	    printf("Enabling the port...\n");

	    if(ixEthAccPortEnable(portId) != IX_ETH_ACC_SUCCESS)
	    {
		printf("DbLearning: Error enabling port %u\n", 
		       (UINT32)portId);
		return (IX_FAIL);
	    }
	    
	    /* Add some static entries */
	    printf("Adding static entries...\n");
	    
	    if (ixEthDBFilteringStaticEntryProvision(portId,
						     &staticMacAddress[portId])
		!= IX_ETH_DB_SUCCESS)
	    {
		printf("DbLearning: Failed to add static MAC to port %u\n", 
		       (UINT32)portId);
		return (IX_FAIL);
	    }

            ixOsalSleep(100);
	    
	    /* Verify that the address has successfully been added */
	    if (ixEthDBFilteringDatabaseSearch(&portPtr, 
					       &staticMacAddress[portId])
		== IX_ETH_DB_NO_SUCH_ADDR)
	    {
		printf("DbLearning: Static MAC address not found in Database\n");
		return (IX_FAIL);
	    }
	    
	    /* Add some dynamic entries */
	    printf("Adding dynamic entries...\n");
	    
	    if (ixEthDBFilteringDynamicEntryProvision(portId,
						      &dynamMacAddress[portId])
		!= IX_ETH_DB_SUCCESS)
	    {
		printf("DbLearning: Failed to add dynamic MAC to port %u\n", 
		       (UINT32)portId);
	    }
	    
	    printf("Enabling aging...\n");
	    
	    if (ixEthDBPortAgingEnable(portId) != IX_ETH_DB_SUCCESS)
	    {
		printf("DbLearning: Failed to enable aging for port %u\n", 
		       (UINT32)portId);
		return (IX_FAIL);
	    }
	}
    }
 
    ixEthDBFilteringDatabaseShowAll();

    /* Wait 4 minutes over aging time (for safety) and verify that the 
     * dynamic entries have been removed 
     */
    printf("Aging entries.\nWaiting for %d minutes...\n", 
	   IX_ETH_DB_LEARNING_ENTRY_AGE_TIME / 60 + 3);
    
    ixOsalSleep((IX_ETH_DB_LEARNING_ENTRY_AGE_TIME + 180) * 1000);
     
    ixEthDBFilteringDatabaseShowAll();
    
    for (portId = 0; portId < IX_ETHACC_CODELET_MAX_PORT; portId++)
    {
	if (validPorts[portId])
	{
	    if(ixEthDBFilteringEntryDelete(&staticMacAddress[portId])
	       != IX_ETH_DB_SUCCESS)
	    {
		printf("DbLearning: Failed to remove static MAC on port %u\n", 
		       (UINT32)portId);
	    }
	    
	    /* Disable ports */
	    if(ixEthAccPortDisable(portId) != IX_ETH_ACC_SUCCESS)
	    {
		printf("DbLearning: Error disabling port %u\n",
		       (UINT32)portId);
		return (IX_FAIL);
	    }
	}
    }

    /* wait for pending traffic to be completely received */
    if (ixEthAccCodeletRecoverBuffers() 
	!= IX_SUCCESS)
    {
	printf("Warning : Not all buffers are accounted for!\n");
    }    

    return (IX_SUCCESS);
}


/*
 * Function definition: ixEthAccCodeletDBMaintenanceStart()
 *
 * Start the EDB Maintenance task
 */
IX_STATUS 
ixEthAccCodeletDBMaintenanceStart(void)
{
    IxOsalThread maintenanceThread;
    IxOsalThreadAttr threadAttr;

    threadAttr.name      = "Codelet EDB Maintenance";
    threadAttr.stackSize = 32 * 1024; /* 32kbytes */
    threadAttr.priority  = IX_ETHACC_CODELET_DB_PRIORITY;

    if (!ixEthAccCodeletDBMaintenanceInitialized)
    {
	/* this should be initialized once */
	ixOsalMutexInit (&ixEthAccCodeletDBMaintenanceTaskRunning);
	ixEthAccCodeletDBMaintenanceInitialized = TRUE;
	ixEthAccCodeletDBMaintenanceTaskStopTrigger = TRUE;
    }

    if (ixEthAccCodeletDBMaintenanceTaskStopTrigger)
    {
	/* Polled mode based on a task running a loop */
	if (ixOsalThreadCreate(&maintenanceThread,
			       &threadAttr,
			       (IxOsalVoidFnVoidPtr) ixEthAccCodeletDBMaintenanceTask,
			       NULL)	
	    != IX_SUCCESS)
	{
	    ixOsalMutexDestroy(&ixEthAccCodeletDBMaintenanceTaskRunning);	
	    printf("DBLearning: Error spawning DB maintenance task\n");
	    return (IX_FAIL);
	}

	/* Start the thread */
	if (ixOsalThreadStart(&maintenanceThread) != IX_SUCCESS)
	{
	    ixOsalMutexDestroy(&ixEthAccCodeletDBMaintenanceTaskRunning);	
	    printf("DBLearning: Error failed to start the maintenance thread\n");
	    return IX_FAIL;
	}
    }

    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletDBMaintenanceStop()
 *
 * Stop the EDB Maintenance task
 */
IX_STATUS 
ixEthAccCodeletDBMaintenanceStop(void)
{
    if (!(ixEthAccCodeletDBMaintenanceTaskStopTrigger))
    {
	ixEthAccCodeletDBMaintenanceTaskStopTrigger = TRUE;
	if (ixOsalMutexLock (&ixEthAccCodeletDBMaintenanceTaskRunning, IX_OSAL_WAIT_FOREVER)
	    != IX_SUCCESS)
	{
	    printf("DBLearning: Error stopping Database Maintenance thread!\n");
	    return (IX_FAIL);
	}
	ixOsalMutexUnlock (&ixEthAccCodeletDBMaintenanceTaskRunning);
	
	ixOsalMutexDestroy(&ixEthAccCodeletDBMaintenanceTaskRunning);
    }

    ixEthAccCodeletDBMaintenanceInitialized = FALSE; 

    return (IX_SUCCESS);
}
