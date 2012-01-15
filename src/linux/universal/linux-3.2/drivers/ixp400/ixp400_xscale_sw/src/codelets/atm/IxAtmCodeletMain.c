/**
 * @file    IxAtmCodeletMain.c
 *
 * @date    27-May-2000
 *
 * @brief   IxAtmCodelet scenarios.
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
 * User defined include files.
 */

#if defined(__wince) && defined(IX_USE_SERCONSOLE)
	#include "IxSerConsole.h"
	#define printf ixSerPrintf
	#define gets ixSerGets
#endif

#include "IxOsal.h"
#include "IxAtmCodelet_p.h"
#include "IxAtmCodelet.h"

/*
 * Defines and macros
 */
#define MAX_TIME_LIMIT_IN_SEC    (15)    /* 15 sec (unit seconds)*/

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */
#define AAL5_CPCS_SDU_SEND_LENGTH  1500
#define AAL0_CELLS_SEND_LENGTH     32
#define AAL5_PACKET_SEND_COUNT     32
#define AAL0_PACKET_SEND_COUNT     32

/* 
 * Time in MS the packet sending thread should sleep when MBUF is running low.
 * This is only applicable to UTOPIA loopback and remote loopback mode. This
 * value is obtained from tuning experience on IXDP465 with 533Mhz silicon 
 */
#define PACKET_SEND_MS_SLEEP  80

/*
 * This is the divider for PACKET_SEND_MS_SLEEP when codelet mode is only
 * sending packet but not receiving (remote loopback). These mode consumes less
 * CPU resources and hence, require lesser sleeping time.
 */
#define PACKET_SEND_MS_SLEEP_DEVIDER 2


/*
 * Typedef 
 */
/* Function pointer to send either AAL5 or AAL0 packets */
typedef void (*IxAtmCodeletSduSendTaskFnPtr)(void);

/* 
 * Global variables
 */
/* Function pointer variable */
IxAtmCodeletSduSendTaskFnPtr ixAtmCodeletSduSendTask;

/* Initial value of AAL Type is invalid */
IxAtmCodeletAalType ixAtmCodeletAalType = ixAtmCodeletAalTypeInvalid;

/* Flag to enable OAM Ping */
BOOL ixAtmCodeletOamPingF4F5Enabled = FALSE;

/*
 * Static variable
 */
/* Default to AAL5 Sleep time */
static UINT32 loopSleepTime = PACKET_SEND_MS_SLEEP;

/*
 * Function definitions
 */
/* Function for OAM Ping F4 & F5 cells */
PRIVATE void ixAtmCodeletOamPingF4F5CellsRun(void); 

/* Function to send AAL5 SDU */
PRIVATE void ixAtmCodeletAal5SduSendTask (void);  

/* Function to send AAL0 SDU */
PRIVATE void ixAtmCodeletAal0SduSendTask (void);

/* Function that displays the usage */
PRIVATE void ixAtmCodeletMainUsage (void); 

/* Function that performs utopia loopback */
PRIVATE IX_STATUS ixAtmCodeletMainUtopiaLoopbackRun (void);

/* Function that performs software loopback */
PRIVATE IX_STATUS ixAtmCodeletMainSoftwareLoopbackRun (void);

/* Function that performs remote loopback */
PRIVATE IX_STATUS ixAtmCodeletMainRemoteLoopbackRun (void);

/* Function to create thread for SwLoopbackStatsShowTask */
PRIVATE IX_STATUS ixAtmCodeletSoftwareLoopbackThreadCreate(void);

/* Function that display the ATM Codelet Stats and/or Perform OAM 
   ping for Sw loopback*/
PRIVATE void ixAtmCodeletSoftwareLoopbackStatsShowTask(void);

PRIVATE IX_STATUS
ixAtmCodeletMainUtopiaLoopbackRun (void)
{
    IX_STATUS retval;
    UINT32 numPorts = 1;
    UINT32 rxToTxRatio = 1;
    IxOsalThread tid;
    IxOsalThreadAttr threadAttr;
    char *pThreadName = "PDU Send";
    IxAtmCodeletMode mode = IX_ATMCODELET_UTOPIA_LOOPBACK;
    
    IX_ATMCODELET_LOG ("*************************************\n");
    IX_ATMCODELET_LOG ("********** UTOPIA LOOPBACK **********\n");
    IX_ATMCODELET_LOG ("*************************************\n");

    /* Initialise system for the codelet */
    retval = ixAtmCodeletSystemInit (numPorts, mode);

    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to initialise IxAtmCodelet\n");
	return IX_FAIL;
    }

    retval = ixAtmCodeletInit (numPorts, rxToTxRatio);
    
    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to initialise IxAtmCodelet\n");
	return IX_FAIL;
    }

    /* AAL Type 5*/
    if( ixAtmCodeletAalType == ixAtmCodeletAalType5)
    { 
	ixAtmCodeletSduSendTask = ixAtmCodeletAal5SduSendTask;

	/* Provision some Aal5 channels */
	retval = ixAtmCodeletUbrChannelsProvision (numPorts,
						   IX_ATMCODELET_NUM_AAL5_CHANNELS,
						   IX_ATMDACC_AAL5);
	loopSleepTime = PACKET_SEND_MS_SLEEP;
    }

    /* AAL Type 0 with 48 bytes*/
    if( ixAtmCodeletAalType == ixAtmCodeletAalType0_48)
    { 
	ixAtmCodeletSduSendTask = ixAtmCodeletAal0SduSendTask;

	/* Provision some Aal0_48 channels */
	retval = ixAtmCodeletUbrChannelsProvision (numPorts,
						   IX_ATMCODELET_NUM_AAL0_48_CHANNELS,
						   IX_ATMDACC_AAL0_48);
	loopSleepTime = PACKET_SEND_MS_SLEEP;
    }

    /* AAL Type 0 with 52 bytes */
    if( ixAtmCodeletAalType == ixAtmCodeletAalType0_52)
    {
	ixAtmCodeletSduSendTask = ixAtmCodeletAal0SduSendTask;

	/* Provision some Aal0_52 channels */
	retval = ixAtmCodeletUbrChannelsProvision (numPorts,
						   IX_ATMCODELET_NUM_AAL0_52_CHANNELS,
						   IX_ATMDACC_AAL0_52);
	loopSleepTime = PACKET_SEND_MS_SLEEP;
    }

    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to provision channels\n");
	return IX_FAIL;
    }
    
    /*Set attributes of thread*/
    threadAttr.name = pThreadName;
    threadAttr.stackSize = IX_ATMCODELET_QMGR_DISPATCHER_THREAD_STACK_SIZE;
#ifdef __linux
    threadAttr.priority = IX_ATMCODELET_QMGR_DISPATCHER_PRIORITY;
#else
    threadAttr.priority = IX_ATMCODELET_UTOPIA_LB_THREAD_PRIORITY;
#endif

    if (ixOsalThreadCreate(&tid,
			   &threadAttr,
 			   (IxOsalVoidFnVoidPtr)ixAtmCodeletSduSendTask, 
 			   NULL) != IX_SUCCESS) 
      { 
	IX_ATMCODELET_LOG ("Error spawning SduSend task\n"); 
 	return IX_FAIL; 
      }  

    if(IX_SUCCESS != ixOsalThreadStart(&tid))
      {
        IX_ATMCODELET_LOG ("Error starting thread\n"); 
        return IX_FAIL;
      }	 
    return IX_SUCCESS;
}

/* --------------------------------------------------------------
   This scenario demonstrates that 32Mbps Rx/6 Mbps Tx can be
   achieved.
   -------------------------------------------------------------- */
PRIVATE IX_STATUS
ixAtmCodeletMainSoftwareLoopbackRun (void)
{
    IX_STATUS retval;
    UINT32 numPorts = 1;
    UINT32 rxToTxRatio = 5;/* Transmit 1 PDU for every 5 PDUs recieved. 
			    * This will show 32Mbps Rx/6 Mbps Tx */
    IxAtmCodeletMode mode = IX_ATMCODELET_SOFTWARE_LOOPBACK;

    IX_ATMCODELET_LOG ("*************************************\n");
    IX_ATMCODELET_LOG ("********* SOFTWARE LOOPBACK *********\n");
    IX_ATMCODELET_LOG ("*************************************\n");

    /* Initialise system for the codelet */
    retval = ixAtmCodeletSystemInit (numPorts, mode);

    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to initialise IxAtmCodelet\n");
	return IX_FAIL;
    }

    /* Initialise the codelet */
    retval = ixAtmCodeletInit (numPorts, rxToTxRatio);
    
    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to initialise IxAtmCodelet\n");
	return IX_FAIL;
    }

    /* AAL Type 5*/
    if (ixAtmCodeletAalType == ixAtmCodeletAalType5)
    {	
	/* Provision some Aal5 channels */
	retval = ixAtmCodeletUbrChannelsProvision (numPorts,
						   IX_ATMCODELET_NUM_AAL5_CHANNELS,
						   IX_ATMDACC_AAL5);
    }

    /* AAL Type 0 with 48 bytes */
    if (ixAtmCodeletAalType == ixAtmCodeletAalType0_48)
    {
	/* Provision some Aal0_48 channels */
	retval = ixAtmCodeletUbrChannelsProvision (numPorts,
						   IX_ATMCODELET_NUM_AAL0_48_CHANNELS,
						   IX_ATMDACC_AAL0_48);
    }

    /* AAL Type 0 with 52 bytes */
    if (ixAtmCodeletAalType == ixAtmCodeletAalType0_52)
    {
	/* Provision some Aal0_52 channels */
	retval = ixAtmCodeletUbrChannelsProvision (numPorts,
						   IX_ATMCODELET_NUM_AAL0_52_CHANNELS,
						   IX_ATMDACC_AAL0_52);
    }

    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to provision channels\n");
	return IX_FAIL;
    }

    return IX_SUCCESS;
}

PRIVATE IX_STATUS
ixAtmCodeletMainRemoteLoopbackRun (void)
{
    IX_STATUS retval;
    UINT32 numPorts = 1;
    UINT32 rxToTxRatio = 1;
    IxOsalThread tid;
    IxOsalThreadAttr threadAttr;
    char *pThreadName = "PDU Send";
    IxAtmCodeletMode mode = IX_ATMCODELET_REMOTE_LOOPBACK;
    IX_ATMCODELET_LOG ("*************************************\n");
    IX_ATMCODELET_LOG ("*********** REMOTE LOOPBACK *********\n");
    IX_ATMCODELET_LOG ("*************************************\n");

    /* Initialise system for the codelet */
    retval = ixAtmCodeletSystemInit (numPorts, mode);

    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to initialise IxAtmCodelet\n");
	return IX_FAIL;
    }
    
    /* Initialise the codelet */
    retval = ixAtmCodeletInit (numPorts, rxToTxRatio); 
   
    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to initialise IxAtmCodelet\n");
	return IX_FAIL;
    }

    /* AAL Type 5*/
    if (ixAtmCodeletAalType == ixAtmCodeletAalType5)
    {
	ixAtmCodeletSduSendTask = ixAtmCodeletAal5SduSendTask;

	/* Provision some Aal5 channels */
	retval = ixAtmCodeletUbrChannelsProvision (numPorts,
						   IX_ATMCODELET_NUM_AAL5_CHANNELS,
						   IX_ATMDACC_AAL5);
	/* Unlike UTOPIA loopback, remote loopback mode only send packets but
	 * never process it. Hence, the sending thread sleeping time when
	 * MBUF is full is theoritically half of the UTOPIA loopback mode. */
	loopSleepTime = PACKET_SEND_MS_SLEEP / PACKET_SEND_MS_SLEEP_DEVIDER;
    }

    /* AAL Type 0 with 48 bytes */
    if(ixAtmCodeletAalType == ixAtmCodeletAalType0_48)
    {
	ixAtmCodeletSduSendTask = ixAtmCodeletAal0SduSendTask;

	/* Provision some Aal0_48 channels */
	retval = ixAtmCodeletUbrChannelsProvision (numPorts,
						   IX_ATMCODELET_NUM_AAL0_48_CHANNELS,
						   IX_ATMDACC_AAL0_48);
	/* Unlike UTOPIA loopback, remote loopback mode only send packets but
	 * never process it. Hence, the sending thread sleeping time when
	 * MBUF is full is theoritically half of the UTOPIA loopback mode. */
	loopSleepTime = PACKET_SEND_MS_SLEEP / PACKET_SEND_MS_SLEEP_DEVIDER;
    }

    /* AAL Type 0 with 52 bytes */
    if (ixAtmCodeletAalType == ixAtmCodeletAalType0_52)
    {
	ixAtmCodeletSduSendTask = ixAtmCodeletAal0SduSendTask;

	/* Provision some Aal0_52 channels */
	retval = ixAtmCodeletUbrChannelsProvision (numPorts,
						   IX_ATMCODELET_NUM_AAL0_52_CHANNELS,
						   IX_ATMDACC_AAL0_52);
	/* Unlike UTOPIA loopback, remote loopback mode only send packets but
	 * never process it. Hence, the sending thread sleeping time when
	 * MBUF is full is theoritically half of the UTOPIA loopback mode. */
	loopSleepTime = PACKET_SEND_MS_SLEEP / PACKET_SEND_MS_SLEEP_DEVIDER;
    }

    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to provision channels\n");
	return IX_FAIL;
    }

    /*Set thread attribute*/
    threadAttr.name = pThreadName;
    threadAttr.stackSize = IX_ATMCODELET_QMGR_DISPATCHER_THREAD_STACK_SIZE;
    threadAttr.priority = IX_ATMCODELET_QMGR_DISPATCHER_PRIORITY;
    if (ixOsalThreadCreate(&tid,
			   &threadAttr,
 			   (IxOsalVoidFnVoidPtr)ixAtmCodeletSduSendTask, 
 			   NULL) != IX_SUCCESS) 
      { 
 	IX_ATMCODELET_LOG ("Error spawning SduSend task\n"); 
 	return IX_FAIL; 
      }  

    if(IX_SUCCESS != ixOsalThreadStart(&tid))
      {
        IX_ATMCODELET_LOG ("Error starting thread task\n"); 
        return IX_FAIL;
      }	 
    return IX_SUCCESS;
}

/* 
 * This is the ATM Codelet main function that will be used by the user as a single
 * point of execution
 */
PUBLIC IX_STATUS
ixAtmCodeletMain(IxAtmCodeletMode modeType, IxAtmCodeletAalType aalType)
{
    /* Validate the type of AAL selected */
    if (aalType <= ixAtmCodeletAalTypeInvalid || aalType >= ixAtmCodeletAalTypeMax)
    {
	IX_ATMCODELET_LOG ("Invalid AAL Type\n");
	ixAtmCodeletMainUsage();
	return IX_FAIL;
    }

    /* Setup aalType */ 
    ixAtmCodeletAalType = aalType;

    /* The type of mode - Utopia, Software, Remote or OAM Ping mode */
    switch (modeType)
    {
        case IX_ATMCODELET_UTOPIA_LOOPBACK_8VBR_8CBR_16UBR:
		/* Setup to have 8VBR, 8CBR, 16 UBR VCs */
		ixAtmUtilsAtmRtVcsSet();
        case IX_ATMCODELET_UTOPIA_LOOPBACK:
	    if (IX_SUCCESS != ixAtmCodeletMainUtopiaLoopbackRun())
	    {
		IX_ATMCODELET_LOG ("Error Setting Utopia Loopback\n");
		ixAtmUtilsAtmRtVcsUnset();
		return IX_FAIL;
	    }
	    break;

	case IX_ATMCODELET_SOFTWARE_LOOPBACK_8VBR_8CBR_16UBR:
		/* Setup to have 8VBR, 8CBR, 16 UBR VCs */
		ixAtmUtilsAtmRtVcsSet();
	case IX_ATMCODELET_SOFTWARE_LOOPBACK:
	    if (IX_SUCCESS != ixAtmCodeletMainSoftwareLoopbackRun())
	    {
		IX_ATMCODELET_LOG ("Error Setting Software Loopback\n");
		ixAtmUtilsAtmRtVcsUnset();
		return IX_FAIL;
	    }

	    /* Port rate for Software loopback */
	    ixAtmCodeletPortRateModify(0,IX_ATM_CODELET_SWLOOPBACK_PORT_RATE);

	    if (ixAtmCodeletSoftwareLoopbackThreadCreate() != IX_SUCCESS)
	    {
		IX_ATMCODELET_LOG("Unable to create thread for software loopback\n");
	    }

	    break;

	case IX_ATMCODELET_REMOTE_LOOPBACK_8VBR_8CBR_16UBR:
	    /* Setup to have 8VBR. 8CBR, 16 UBR VCs */
	    ixAtmUtilsAtmRtVcsSet();
	case IX_ATMCODELET_REMOTE_LOOPBACK:

	    if (IX_SUCCESS != ixAtmCodeletMainRemoteLoopbackRun())
	    {
		IX_ATMCODELET_LOG ("Error Setting Remote Loopback\n");
		ixAtmUtilsAtmRtVcsUnset();
		return IX_FAIL;
	    }

	    /* Port rate for Remote loopback */
	    ixAtmCodeletPortRateModify(0,IX_ATM_CODELET_REMOTELOOPBACK_PORT_RATE);

	    break;

	case IX_ATMCODELET_UTOPIA_LOOPBACK_PERFORM_OAM_PING_F4F5:

	    /* OAM Performs F4 and F5 in Utopia Loopback mode */
	    ixAtmCodeletOamPingF4F5Enabled = TRUE;

	    if (IX_SUCCESS != ixAtmCodeletMainUtopiaLoopbackRun())
	    {
		IX_ATMCODELET_LOG ("Error Setting Utopia Loopback\n");
		return IX_FAIL;
	    }	
   
	    IX_ATMCODELET_LOG ("\nOAM Ping for F4 & F5 cells in Utopia Loopback\n");

	    break;

	case IX_ATMCODELET_SOFTWARE_LOOPBACK_PERFORM_OAM_PING_F4F5:

	    /* OAM Performs F4 and F5 in Software loopback mode */
	    if (IX_SUCCESS != ixAtmCodeletMainSoftwareLoopbackRun())
	    {
		IX_ATMCODELET_LOG ("Error Setting Utopia Loopback\n");
		return IX_FAIL;
	    }	    	   

	    IX_ATMCODELET_LOG ("\nOAM Ping for F4 & F5 cells in Software Loopback\n");

	    ixAtmCodeletOamPingF4F5Enabled = TRUE;

	    if (ixAtmCodeletSoftwareLoopbackThreadCreate() != IX_SUCCESS)
	    {
		IX_ATMCODELET_LOG("Unable to create thread for software loopback\n");
	    }

	    break;
	 
	default:
	    IX_ATMCODELET_LOG ("Invalid modeType\n"); 
	    ixAtmCodeletMainUsage();
	    return IX_FAIL;
    }

    return IX_SUCCESS;
}

/*------------------------------------------------------------
 *          PRIVATE functions 
 *------------------------------------------------------------*/

PRIVATE void
ixAtmCodeletOamPingF4F5CellsRun(void)
{
    IX_ATMCODELET_LOG ("\nInitiate OAM F4 End-to-End loopback ping");
    IX_ATMCODELET_LOG ("\nPort = 0, VPI = 1, numCells = 10\n");
    ixOamCodeletOamF4EtePing(0,1,10); 
    ixOamCodeletShow();

    IX_ATMCODELET_LOG ("\nInitiate OAM F4 Segment loopback ping");
    IX_ATMCODELET_LOG ("\nPort = 0, VPI = 1, numCells = 10\n");
    ixOamCodeletOamF4SegPing(0,1,10); 
    ixOamCodeletShow();
    
    IX_ATMCODELET_LOG ("\nInitiate OAM F5 End-to-End loopback ping");
    IX_ATMCODELET_LOG ("\nPort = 0, VPI = 1, VCI = 34, numCells = 10\n");
    ixOamCodeletOamF5EtePing(0,1,34,10); 
    ixOamCodeletShow();
   
    IX_ATMCODELET_LOG ("\nInitiate OAM F5 Segment loopback ping");
    IX_ATMCODELET_LOG ("\nPort = 0, VPI = 1, VCI = 34, numCells = 10\n");
    ixOamCodeletOamF5SegPing(0,1,34,10); 
    ixOamCodeletShow();
}

PRIVATE IX_STATUS
ixAtmCodeletSoftwareLoopbackThreadCreate(void)
{
    IxOsalThread tid;
    IxOsalThreadAttr threadAttr;
    char *pThreadName = "SW Loopback Statistics";

    threadAttr.name = pThreadName;
    threadAttr.stackSize = IX_ATMCODELET_QMGR_DISPATCHER_THREAD_STACK_SIZE;
    threadAttr.priority = IX_ATMCODELET_QMGR_DISPATCHER_PRIORITY;

    /* Create thread for displaying stats & perform OAM ping in software loopback */
   
    if (ixOsalThreadCreate(&tid,
			   &threadAttr,
			    (IxOsalVoidFnVoidPtr) ixAtmCodeletSoftwareLoopbackStatsShowTask, 
			   NULL) != IX_SUCCESS) 
    { 
	IX_ATMCODELET_LOG ("Error spawning ixAtmCodeletSwLoopbackStatsShowTask task\n"); 
	return IX_FAIL; 
    }  	 

    if(IX_SUCCESS != ixOsalThreadStart(&tid))
    {
        IX_ATMCODELET_LOG ("Error starting thread\n"); 
        return IX_FAIL;
    }
    return IX_SUCCESS;
}

/* To display stats and/or perform OAM ping F4 and F5 for Software Loopback */
PRIVATE void
ixAtmCodeletSoftwareLoopbackStatsShowTask(void)
{
    UINT32 t0, t1, tlapse;

    t0 = ixOsalTimestampGet(); /* get the initial time stamp value */
	
 
    while (1)
    {
	t1 = ixOsalTimestampGet(); /* get the next time stamp */

	tlapse = (t1 > t0 )? (t1 - t0): (0xFFFFFFFF) - t1 + t0;
		
	if( tlapse > (MAX_TIME_LIMIT_IN_SEC*ixOsalTimestampResolutionGet()))
	{
	  
	    ixAtmCodeletShow(); /* Display the statistics*/

	    /* If OAM Ping F4 and F5 is used then enabled is TRUE */
	    if (ixAtmCodeletOamPingF4F5Enabled)
	    {
		ixAtmCodeletOamPingF4F5CellsRun();
	    }

	    t0 = ixOsalTimestampGet(); /* get t0 time stamp value */
	}
  
    ixOsalSleep (0);
    }
}

/*   SduSend task for AAL5. */
PRIVATE void
ixAtmCodeletAal5SduSendTask (void)
{

   UINT32 t0, t1, tlapse;
    static char stillRunning[] = "|/-\\";
    static int  stillRunningIndex = 0;

    t0 = ixOsalTimestampGet(); /* get the initial time stamp value */

    while (TRUE)
    {
	IX_ATMCODELET_LOG ("\rSending Multiple Aal5 Packets...%c",
			   stillRunning[stillRunningIndex++ & 3]);

	/* Send some Aal5 Cpcs SDUs */
	if (IX_SUCCESS != ixAtmCodeletAal5CpcsSdusSend
	    (AAL5_CPCS_SDU_SEND_LENGTH))
	{
	    IX_ATMCODELET_LOG ("Failed to send SDUs\n");
	    return;
	}
 
	t1 = ixOsalTimestampGet(); /* get the next time stamp */
	
   	tlapse = (t1 > t0 )? (t1 - t0): (0xFFFFFFFF) - t1 + t0;
		
	if( tlapse > (MAX_TIME_LIMIT_IN_SEC*ixOsalTimestampResolutionGet()))
	{	   
	    ixAtmCodeletShow(); /* Display the statistics*/

	    /* If OAM Ping F4 and F5 is used then enabled is TRUE */
	    if (ixAtmCodeletOamPingF4F5Enabled)
	    {
		ixAtmCodeletOamPingF4F5CellsRun();
	    }

	    t0 = ixOsalTimestampGet(); /* get t0 time stamp value */
	}
       
	ixOsalSleep (loopSleepTime);
    }
}

/*   SduSend task for AAL0_48 and AAL_52. */
PRIVATE void
ixAtmCodeletAal0SduSendTask (void)
{

    UINT32 t0, t1, tlapse;
    static char stillRunning[] = "|/-\\";
    static int  stillRunningIndex = 0;

    /* get the initial time stamp value */
    t0 = ixOsalTimestampGet();

    while (TRUE)
    {
	IX_ATMCODELET_LOG ("\rSending Multiple Aal0 Packets...%c",
			   stillRunning[stillRunningIndex++ & 3]);

	/* Send some Aal0 Packets */
	if (IX_SUCCESS != ixAtmCodeletAal0PacketsSend (AAL0_CELLS_SEND_LENGTH))
	{
	    IX_ATMCODELET_LOG ("Failed to send SDUs\n");
	    return;
	}

	t1 = ixOsalTimestampGet(); /* get the next time stamp */

	tlapse = (t1 > t0 )? (t1 - t0): (0xFFFFFFFF) - t1 + t0;
		
	if( tlapse > (MAX_TIME_LIMIT_IN_SEC*ixOsalTimestampResolutionGet()))
	{

	    ixAtmCodeletShow(); /* Display the statistics*/
	    /* If OAM Ping F4 and F5 is used then enabled is TRUE */
	    if (ixAtmCodeletOamPingF4F5Enabled)
	    {
		ixAtmCodeletOamPingF4F5CellsRun();
	    }

	    t0 = ixOsalTimestampGet(); /* get t0 time stamp value */
	}
#ifdef __linux
        ixOsalSleep (loopSleepTime);
#else
        ixOsalYield();
#endif
    }
}

PRIVATE void
ixAtmCodeletMainUsage (void)
{
    printf("\nUsage :");
    printf("\n ixAtmCodeletMain (modeType, aalType)");
    printf("\n");	    
    printf("\n  modeType : 0 = Utopia Loopback Mode 32 UBR");
    printf("\n             1 = Utopia Loopback Mode 8VBR, 8CBR, 16UBR");
    printf("\n             2 = Software Loopback Mode 32 UBR");
    printf("\n             3 = Software Loopback Mode 8VBR, 8CBR, 16UBR");
    printf("\n             4 = Remote Loopback Mode 32 UBR");
    printf("\n             5 = Remote Loopback 8VBR, 8CBR, 16UBR");
    printf("\n             6 = F4 & F5 cells OAM Ping in UTOPIA Loopback mode");
    printf("\n             7 = F4 & F5 cells OAM Ping in Software Loopback mode");
    printf("\n");
    printf("\n  aalType  : 1 = AAL5                                   ");
    printf("\n             2 = AAL0_48                                "); 
    printf("\n             3 = AAL0_52                                ");
    printf("\n");
}

#ifdef __wince
int readNumber(void)
{
    char line[256];
    gets(line);
    return atoi(line);
}

int wmain(int argc, WCHAR **argv)
{
	IxAtmCodeletMode atmCodeletModeTypeTest;
	IxAtmCodeletAalType atmCodeletAalTypeTest;
    BOOL atmCodeletRun = TRUE;
	BOOL testStarted = FALSE;
	IX_STATUS retVal = IX_FAIL;

    while(atmCodeletRun)
    {
		if (!testStarted)
		{
		    printf("\n");
		    printf("************* ATM Codelet *********************************\n");
		    printf("\n  Mode Type: 0 = Utopia Loopback Mode 32 UBR");
		    printf("\n             1 = Utopia Loopback Mode 8VBR, 8CBR, 16UBR");
		    printf("\n             2 = Software Loopback Mode 32 UBR");
		    printf("\n             3 = Software Loopback Mode 8VBR, 8CBR, 16UBR");
		    printf("\n             4 = Remote Loopback Mode 32 UBR");
		    printf("\n             5 = Remote Loopback 8VBR, 8CBR, 16UBR");
		    printf("\n             6 = F4 & F5 cells OAM Ping in UTOPIA Loopback mode");
		    printf("\n             7 = F4 & F5 cells OAM Ping in Software Loopback mode");
		    printf("\n\n");
		    printf("\n  AAL Type:  1 = AAL5\n");
		    printf("\n             2 = AAL0_48\n"); 
		    printf("\n             3 = AAL0_52\n");
		    printf("\n");
		    printf("\n");
		    printf("100. Exit ATM Codelet\n");
		    printf("\n");
		    printf("\nEnter Mode Type: ");
		}

		do
		{
			atmCodeletModeTypeTest = (IxAtmCodeletMode)readNumber();
			if (atmCodeletModeTypeTest == 100)
			{
				printf("\n\nExiting Codelet \n\n");
				return 0;
			}
			else if (atmCodeletModeTypeTest >= IX_ATMCODELET_INVALID_MODE)
			{
				printf("\nInvalid Mode Type Entered!. Please enter a correct mode type: \n");
			}
			else
			{
				break;
			}
		}while(1);

		printf("\nEnter AAL Type: ");
        do
		{
			atmCodeletAalTypeTest = (IxAtmCodeletAalType)readNumber();
			if ((atmCodeletAalTypeTest <= ixAtmCodeletAalTypeInvalid) || 
				(atmCodeletAalTypeTest >= ixAtmCodeletAalTypeMax))
			{
				printf("\nInvalid AAL Type Entered!. Please enter a correct AAL type: \n");
			}
			else
			{
				break;
			}
		}while(1);

        retVal = ixAtmCodeletMain(atmCodeletModeTypeTest, atmCodeletAalTypeTest);

		if (retVal == IX_SUCCESS)
		{
			testStarted = TRUE;
		}
		else
		{
			testStarted = FALSE;
		}
    }

    return 0;
}
#endif              
