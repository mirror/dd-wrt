/*
 * @file    IxAtmCodelet.c
 *
 * @date    17-May-2002
 *
 * @brief   API of the IXP400 Atm Codelet (IxAtmCodelet)
 *
 * A detailed description of the design of this codelet is contained in
 * IxAtmCodelet_p.h
 *
 * IxAtmCodelet API Functions:
 *       ixAtmCodeletInit
 *       ixAtmCodeletUbrChannelsProvision
 *       ixAtmCodeletUbrChannelsRemove
 *       ixAtmCodeletPortRateModify
 *       ixAtmCodeletPortQuery 
 *       ixAtmCodeletShow
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
 * System defined include files
 */

#if defined(__wince) && defined(IX_USE_SERCONSOLE)
	#include "IxSerConsole.h"
	#define printf ixSerPrintf
	#define gets ixSerGets
#endif

#ifdef __vxworks
#include <end.h>    /* END drivers */
#include <endLib.h> /* END drivers */
#endif /* def __vxworks */

#include "IxOsal.h"
#include "IxQMgr.h"
#include "IxNpeMh.h"
#include "IxNpeDl.h"
#include "IxAtmdAcc.h"
#include "IxAtmm.h"
#include "IxAtmCodelet.h"
#include "IxAtmCodelet_p.h"

/*
 * Defines and macros
 */
#define IX_ATMCODELET_UTOPIA_PHY_ADDR_BASE  (0)
#define IX_ATMCODELET_UTOPIA_TX_PORT_RATE   (22257 * 53 * 8) /* TX_PORT_RATE gives 
							      * approx 9Mbps.*/
#define IX_ATMCODELET_UTOPIA_RX_PORT_RATE   (52 * 1024 * 1024)  /* The actual 52 Mbps
								 * is 52 x 1024 x 1024 */
#define IX_ATMCODELET_MAX_PACKET_LENGTH     (1500)
#define IX_ATMCODELET_MAX_CELLS_TO_SEND     32


/* Define if queue manager dispatcher should be interrupt or polled
 */
#ifdef  __vxworks
#define IX_ATMCODELET_USE_QMGR_INT FALSE
#else
#define IX_ATMCODELET_USE_QMGR_INT TRUE
#endif


/*
 * This function macro is used to reduce the ammount of code needed to check
 * the return value of component initialisation functions. The
 * parameter FUNCTION_CALL is for example:
 * IX_ATMCODELET_COMP_INIT(ixAtmmUtopiaInit(numPorts, phyMode, portCfgs, loopbackMode));
 */
#define IX_ATMCODELET_COMP_INIT(FUNCTION_CALL) do{\
  if (IX_SUCCESS != FUNCTION_CALL) \
  { \
     IX_ATMCODELET_LOG ("IxAtmCodelet: %s Failed\n", #FUNCTION_CALL); \
  } \
  else \
  { \
     IX_ATMCODELET_LOG ("IxAtmCodelet: %s Succeeded\n", #FUNCTION_CALL); \
  } \
}while(0)

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */
static BOOL ixAtmCodeletInitialized                = FALSE;
static BOOL ixAtmCodeletUtopiaLoopbackEnabled      = FALSE;
static BOOL ixAtmCodeletSoftwareLoopbackEnabled    = FALSE;
static UINT32 ixAtmCodeletNumPortsConfigured = 0;
static IxAtmCodeletStats ixAtmCodeletStats;

/* Number of VCs configured */
static UINT32 ixAtmCodeletNumChannelsConfigured = 0;

static IxAtmCodeletMode ixAtmCodeletMode;

static IxQMgrDispatcherFuncPtr dispatcherFunc ;

/*
 * Function prototypes
 */

/* --------------------------------------------------------------
   This is the entry point for the IxQMgrDispatcher task used for
   IX_ATMCODELET_SOFTWARE_LOOPBACK mode of operation.
   -------------------------------------------------------------- */
PRIVATE int
ixAtmCodeletDispatchTask (void *arg, void **ptrRetObj);

/*
 * Function definitions
 */



/* --------------------------------------------------------------
   Initialise system components used by the Atm Codelet.
   -------------------------------------------------------------- */
PUBLIC IX_STATUS
ixAtmCodeletSystemInit (UINT32 numPorts,
			IxAtmCodeletMode mode)
{
    IX_STATUS retval = IX_SUCCESS;
#ifdef IX_UTOPIAMODE 
    IxAtmmPhyMode phyMode = IX_ATMM_SPHY_MODE;
#else
    IxAtmmPhyMode phyMode = IX_ATMM_MPHY_MODE;
#endif
    IxOsalThread dispatchtid;
    IxOsalThreadAttr threadAttr;
    char *pThreadName = "QMgr Dispatcher";

#ifdef __vxworks
    if (endFindByName ("ixe", 0) != NULL)
    {
	IX_ATMCODELET_LOG ("FAIL : Driver ixe0 detected\n");
	printf("FAIL : Driver ixe0 detected\n");
	return IX_FAIL;
    }
    if (endFindByName ("ixe", 1) != NULL)
    {
	IX_ATMCODELET_LOG ("FAIL : Driver ixe1 detected\\n");
	printf("FAIL : Driver ixe1 detected\n");
	return IX_FAIL;
    }
#endif


    /**************** System initialisation ****************/
    /*
     * The IxQMgr component provides interfaces for configuring and accessing the IXP4XX
     * AQM hardware queues used to facilitate communication of data between the NPEs and
     * the xscale. IxAtmdAcc configures these queues. The IxQMgr component provides a
     * dispatcher that will call registered callback functions will specified queue events
     * occur.
     */
    IX_ATMCODELET_COMP_INIT(ixQMgrInit());

    ixQMgrDispatcherLoopGet(&dispatcherFunc);

    /* This next section sets up how the IxQMgrDispatcher is called. 
     * For the purposes of demonstration under vxWorks the IxQMgrDispatcher 
     * is polled, and under Linux intterrupts are used.
     * This offers the best performance respectively.
     */
    if (IX_ATMCODELET_USE_QMGR_INT)
    {
	/* Running IxQMgrDispatcher from interrupt level */

	/* 
	 * Bind the IxQMgr dispatcher to interrupt. The IX_QMGR_QUELOW_GROUP group
	 * of queues concern ATM Transmit , Receive, Transmit Done queues
	 */

	retval = ixOsalIrqBind(IX_OSAL_IXP400_QM1_IRQ_LVL,
			       (IxOsalVoidFnVoidPtr)(dispatcherFunc),
			       (void *)IX_QMGR_QUELOW_GROUP);

	if (IX_SUCCESS != retval)
	{
	    IX_ATMCODELET_LOG ("Failed to bind to QM1 interrupt\n");
	    return IX_FAIL;
	}
	
	/*
	 * Bind the IxQMgr dispatcher to interrupt. The IX_QMGR_QUELOW_GROUP group
	 * of queues concern ATM Receive Free queues.
	 */
	retval = ixOsalIrqBind(IX_OSAL_IXP400_QM2_IRQ_LVL,
			       (IxOsalVoidFnVoidPtr)(dispatcherFunc),
			       (void *)IX_QMGR_QUEUPP_GROUP);
	if (IX_SUCCESS != retval)
	{
	    IX_ATMCODELET_LOG ("Failed to bind to QM2 interrupt\n");
	    return IX_FAIL;
	}
    }
    else /* Running IxQMgrDispatcher from task level */
    {
      threadAttr.name = pThreadName;
      threadAttr.stackSize = IX_ATMCODELET_QMGR_DISPATCHER_THREAD_STACK_SIZE;
      threadAttr.priority = IX_ATMCODELET_QMGR_DISPATCHER_PRIORITY;

      if (ixOsalThreadCreate(&dispatchtid,
			     &threadAttr,
			     (IxOsalVoidFnVoidPtr)ixAtmCodeletDispatchTask,
			      NULL) != IX_SUCCESS)
	{
	    IX_ATMCODELET_LOG ("Error spawning dispatch task\n");
	    return IX_FAIL;
	}  

      if(IX_SUCCESS != ixOsalThreadStart(&dispatchtid))
	{
	  IX_ATMCODELET_LOG ("Error starting dispatch task\n");
	  return IX_FAIL;
	}
    }

    /* Initialise IxNpeMh */
    IX_ATMCODELET_COMP_INIT(ixNpeMhInitialize (IX_NPEMH_NPEINTERRUPTS_YES));

    /* Download NPE image */
    retval = ixAtmUtilsAtmImageDownload (numPorts, &phyMode);

    if (retval != IX_SUCCESS)
    {
	IX_ATMCODELET_LOG ("NPE download failed\n");
	return IX_FAIL;
    }
    
    ixAtmCodeletMode = mode;

    return IX_SUCCESS;
}

/* --------------------------------------------------------------
   Initialise the Atm Codelet.
   In software loopback only 1 PDU is sent for every rxToTxRatio 
   PDUs received. NOTE: that the rxToTxRatio only applies to 
   IX_ATMCODELET_SOFTWARE_LOOPBACK mode.
   -------------------------------------------------------------- */
PUBLIC IX_STATUS
ixAtmCodeletInit (UINT32 numPorts, 
		  UINT32 rxToTxRatio)
{
    IX_STATUS retval;
    IxAtmmPortCfg portCfgs[IX_UTOPIA_MAX_PORTS];
    IxAtmmUtopiaLoopbackMode loopbackMode;
#ifdef IX_UTOPIAMODE 
    IxAtmmPhyMode phyMode = IX_ATMM_SPHY_MODE;
#else
    IxAtmmPhyMode phyMode = IX_ATMM_MPHY_MODE;
#endif
    IxAtmLogicalPort port;

    /* Check parameters */
    if (numPorts < 1 || numPorts > IX_UTOPIA_MAX_PORTS)
    {
	IX_ATMCODELET_LOG("ixAtmCodeletInit(): numPorts (%u) invalid\n", numPorts);
	return IX_FAIL;
    }

    if (IX_ATMCODELET_UTOPIA_LOOPBACK == ixAtmCodeletMode)
    {
	ixAtmCodeletUtopiaLoopbackEnabled = TRUE;
    }
    else if (IX_ATMCODELET_SOFTWARE_LOOPBACK == ixAtmCodeletMode)
    {
	ixAtmCodeletSoftwareLoopbackEnabled = TRUE;
    }
    else if (IX_ATMCODELET_REMOTE_LOOPBACK != ixAtmCodeletMode)
    {
	IX_ATMCODELET_LOG ("Invalid IxAtmCodelet mode\n");
	return IX_FAIL;
    }

    if (ixAtmCodeletSoftwareLoopbackEnabled)
    {
	if (rxToTxRatio == 0)
	{
	    IX_ATMCODELET_LOG("rxToTxRatio of 0 not allowed\n");
	    return IX_FAIL;
	}
    }
    else
    {
	if (rxToTxRatio > 0)
	{
	    /* Display a message to inform the user that rxToTxRatio only applies to
	     * IX_ATMCODELET_SOFTWARE_LOOPBACK mode.
	     */
	    IX_ATMCODELET_LOG ("rxToTxRatio ignored when codeletSoftwareLoopbackEnabled := FALSE\n");
	}
    }


    /**************** ATM initialisation ****************/
    /* Setup Utopia loopback mode */
    if (IX_ATMCODELET_UTOPIA_LOOPBACK == ixAtmCodeletMode)
    {
	if (numPorts == 1)
	{
	    loopbackMode = IX_ATMM_UTOPIA_LOOPBACK_ENABLED;
	}
	else
	{
	    IX_ATMCODELET_LOG("codeletInit():utopiaLoopbackEnabled(TRUE) not allowed for > 1 port\n");
	    return IX_FAIL;
	}
    }
    else
    {
#ifdef VALIDATION_PLATFORM_USED
	/* Request test and stimulus FPGA (validation platform only) to 
	 * configure Utopia as a master
	 */
	ixAtmUtilsUtopiaFpgaStimulusAsMasterSet();
#endif
	loopbackMode = IX_ATMM_UTOPIA_LOOPBACK_DISABLED;
    }

    /*
     * Set the UTOPIA Phy port addresss for each port. These addresses need to be defined
     * as per the Physical interface used.
     */
    for (port=0; port < (IxAtmLogicalPort)numPorts; port++)
    {
	portCfgs[port].UtopiaTxPhyAddr = IX_ATMCODELET_UTOPIA_PHY_ADDR_BASE + port;
	portCfgs[port].UtopiaRxPhyAddr = IX_ATMCODELET_UTOPIA_PHY_ADDR_BASE + port;
    }
    
    /*
      The following is the order in which each component should be initialised:
      Lower Level Components->IxAtmSch->IxAtmdAcc->IxAtmm->IxAtmCodelet.
      At this stage all lower level components(IxQMgr, IxNpeMh) have been initialised
      and the NPE image have been downloaded.
    */
    IX_ATMCODELET_COMP_INIT(ixAtmSchInit());
    IX_ATMCODELET_COMP_INIT(ixAtmdAccInit());
    IX_ATMCODELET_COMP_INIT(ixAtmmInit());
    IX_ATMCODELET_COMP_INIT(ixAtmmUtopiaInit(numPorts, phyMode, portCfgs, loopbackMode));
    IX_ATMCODELET_COMP_INIT(ixAtmUtilsMbufPoolInit());

    /* Initialize Ports */
    for (port=0; port < (IxAtmLogicalPort)numPorts; port++)
    {
	
	/* Initialise the port with IxAtmm */
	retval = ixAtmmPortInitialize (port,
				       IX_ATMCODELET_UTOPIA_TX_PORT_RATE,
				       IX_ATMCODELET_UTOPIA_RX_PORT_RATE);

	if (retval != IX_SUCCESS)
	{
	    IX_ATMCODELET_LOG ("Port Initialization failed for port %u\n", port);
	    return IX_FAIL;
	}
	
	/* Enable the port in AtmdAcc */
	retval = ixAtmmPortEnable(port);

	if (retval != IX_SUCCESS)
	{
	    IX_ATMCODELET_LOG ("Port Enable failed for port %u\n", port);
	    return IX_FAIL;
	}
    }

    /**************** Codelet initialisation ****************/
    /* Utopia Loopback/ Remote Loopback */
    if ((IX_ATMCODELET_UTOPIA_LOOPBACK == ixAtmCodeletMode) ||
	(IX_ATMCODELET_REMOTE_LOOPBACK == ixAtmCodeletMode))
    {
	/* Initialize the RxTx subcomponent */
	IX_ATMCODELET_COMP_INIT(ixAtmRxTxInit (&ixAtmCodeletStats));
    }
    else /* Software Loopback */
    {
	/* Initialize the SwLoopback subcomponent */
	IX_ATMCODELET_COMP_INIT(ixAtmSwLbInit (&ixAtmCodeletStats, rxToTxRatio));
    }

    /* Set the number of ports configured */
    ixAtmCodeletNumPortsConfigured = numPorts;

    /* Now provision the OAM Tx Port VCs and Rx VC channels */
    retval = ixOamCodeletInit (numPorts);  
    if (retval != IX_SUCCESS)  
    { 
 	IX_ATMCODELET_LOG ("ixOamCodeletInit failed\n"); 
 	return IX_FAIL; 
    }  

    /* set initialised flag */
    ixAtmCodeletInitialized = TRUE;
    
    IX_ATMCODELET_LOG ("Initialization Phase Complete\n");

    return IX_SUCCESS;
}

/* --------------------------------------------------------------
   Provision a specified number of ports and channels. The number
   of ports provisioned must be <= the number of ports specified
   to ixAtmCodeletInit(). 
   -------------------------------------------------------------- */
PUBLIC IX_STATUS
ixAtmCodeletUbrChannelsProvision (UINT32 numPorts,
				  UINT32 numChannels,
				  IxAtmdAccAalType aalType)
{
    IX_STATUS retval;

    /* --------------------------------------------------------------
       A call to ixAtmCodeletInit() must be made before this
       function can be called. The initialisation function sets the
       following values used by this function:
       - ixAtmCodeletInitialized
       - ixAtmCodeletSoftwareLoopbackEnabled
       - ixAtmCodeletUtopiaLoopbackEnabled
       -------------------------------------------------------------- */
    if (!ixAtmCodeletInitialized)
    {
	IX_ATMCODELET_LOG("System not initialized\n");
	return IX_FAIL;
    }

    /*
     * Check that the number of ports specified is not greater than the number
     * specified at initialisation
     */
    if (numPorts > ixAtmCodeletNumPortsConfigured)
    {
    	IX_ATMCODELET_LOG("numPorts is > ports configured at initialization %u\n", 
			  ixAtmCodeletNumPortsConfigured);
	return IX_FAIL;
    }

    if (ixAtmCodeletSoftwareLoopbackEnabled)
    {
	retval = ixAtmSwLbChannelsProvision (numPorts,
					     numChannels,
					     aalType);
    }
    else
    {
	retval = ixAtmRxTxChannelsProvision (numPorts,
						 numChannels,
						 aalType);
    }

    if (retval != IX_SUCCESS)
    {
	IX_ATMCODELET_LOG("Failed to provision channels\n");
    }
    else
    {
	/* Add to the total channels configured */
	ixAtmCodeletNumChannelsConfigured += numChannels;
    }

    return retval;
}

/* --------------------------------------------------------------
   Remove all VCs registered in a previous call to
   ixAtmCodeletTransportChannelsProvision
   -------------------------------------------------------------- */
PUBLIC IX_STATUS
ixAtmCodeletUbrChannelsRemove (void)
{
    IX_STATUS retval;

    if (ixAtmCodeletSoftwareLoopbackEnabled)
    {
        retval = ixAtmSwLbChannelsRemove();
    }
    else
    {
        retval = ixAtmRxTxChannelsRemove();
    }
    return retval;
}


/* --------------------------------------------------------------
   Transmit a number of Aal0 packets each containing cellsPerPacket
   cells.
   Packets are transmitted using the following algorithm:
   1st packet is transmitted on the first Aal0 channel configured
   2nd packet is transmitted on the next Aal0 channel configured
   3rd ...
   If the number of packets to send is greater than the number of
   channels configured then the (numChannlesConfigured + 1)th packet is
   sent on the 1st channel configured and so on.
   -------------------------------------------------------------- */
IX_STATUS
ixAtmCodeletAal0PacketsSend (UINT32 cellsPerPacket)
{
    IX_STATUS retval;

    /* Check it is less than max cells to transmit
     */
    if (cellsPerPacket > IX_ATMCODELET_MAX_CELLS_TO_SEND)
    {
	IX_ATMCODELET_LOG ("cellsPerPacket must be <= %u\n", IX_ATMCODELET_MAX_CELLS_TO_SEND);
	return IX_FAIL;
    }

    retval = ixAtmRxTxAal0PacketsSend (cellsPerPacket);

    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to send AAL0 Packets\n");
    }

    return retval;
}

/* --------------------------------------------------------------
   Transmit a number of Aal5 CPCS SDUs of length sduLength.
   Sdus are transmitted using the following algorithm:
   1st sdu is transmitted on the first Aal5 channel configured
   2nd sdu is transmitted on the next Aal5 channel configured
   3rd ...
   If the number of sdus to send is greater than the number of
   channels configured then (numChannlesConfigured + 1)th sdu is
   sent on the 1st channel configured and so on.
   -------------------------------------------------------------- */
PUBLIC IX_STATUS
ixAtmCodeletAal5CpcsSdusSend (UINT32 sduLength)
{
    IX_STATUS retval;

    if (sduLength > IX_ATMCODELET_MAX_PACKET_LENGTH)
    {
	IX_ATMCODELET_LOG ("SDU Length must be <= %u\n", IX_ATMCODELET_MAX_PACKET_LENGTH);
	return IX_FAIL;
    }

    retval = ixAtmRxTxAal5CpcsSdusSend (sduLength);

    if (IX_SUCCESS != retval)
    {
	IX_ATMCODELET_LOG ("Failed to send Aal5 SDUs\n");
    }

    return retval;
}

/* --------------------------------------------------------------
   Modify the transmit port rate. The transmit port rate the is
   by IxAtmSch to perform its shaping functions.
   -------------------------------------------------------------- */
PUBLIC IX_STATUS
ixAtmCodeletPortRateModify (IxAtmLogicalPort port,
			    UINT32 portRate)
{
    IX_STATUS retval;

    retval = ixAtmSchPortRateModify (port,
				     portRate);

    if (retval != IX_SUCCESS)
    {
	IX_ATMCODELET_LOG("ixAtmCodeletPortRateModify(): Failed to modifiy the port rate\n");
    }
    return retval;
}

/* --------------------------------------------------------------
   Display some statistics about each port configured.
   -------------------------------------------------------------- */
PUBLIC void
ixAtmCodeletPortQuery (void)
{
    ixAtmdAccShow ();
}

/* --------------------------------------------------------------
   Dispaly some statistics about the transmit and receive streams.
   -------------------------------------------------------------- */
PUBLIC void
ixAtmCodeletShow (void)
{
    IX_ATMCODELET_LOG("\nTx     =============================\n");
    IX_ATMCODELET_LOG("       Pdus transmitted............. %10.10u\n",
		      ixAtmCodeletStats.txPdus);
    IX_ATMCODELET_LOG("           Bytes transmitted........ %10.10u\n",
		      ixAtmCodeletStats.txBytes);
    IX_ATMCODELET_LOG("           Pdus submit busy......... %10.10u\n",
		      ixAtmCodeletStats.txPdusSubmitBusy);
    IX_ATMCODELET_LOG("           Pdus submit fail......... %10.10u\n",
		      ixAtmCodeletStats.txPdusSubmitFail);
    IX_ATMCODELET_LOG("TxDone ==========================\n");
    IX_ATMCODELET_LOG("       Pdus transmit done........... %10.10u\n",
		      ixAtmCodeletStats.txDonePdus);
    IX_ATMCODELET_LOG("Rx     =============================\n");
    IX_ATMCODELET_LOG("       Pdus received................ %10.10u\n",
		      ixAtmCodeletStats.rxPdus);
    IX_ATMCODELET_LOG("           Bytes received........... %10.10u\n",
		      ixAtmCodeletStats.rxBytes);
    IX_ATMCODELET_LOG("           Pdus Invalid............. %10.10u\n",
		      ixAtmCodeletStats.rxPdusInvalid);
    IX_ATMCODELET_LOG("RxFree =============================\n");
    IX_ATMCODELET_LOG("       Buffers receive replenish.... %10.10u\n\n\n\n",
		      ixAtmCodeletStats.rxFreeBuffers);
}

/* --------------------------------------------------------------
   IxQMgrDispatcher task entry point.
   -------------------------------------------------------------- */
PRIVATE int
ixAtmCodeletDispatchTask (void *arg, void **ptrRetObj)
{
    /* This is an example of a task based dispatcher */
    while (TRUE)
    {
	/* ATM Tx, Rx, TxDone queues */
	(*dispatcherFunc) (IX_QMGR_QUELOW_GROUP);

	/* ATM RxFree queues         */
	(*dispatcherFunc) (IX_QMGR_QUEUPP_GROUP);
       
	ixOsalYield();
    }
}                        
