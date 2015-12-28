/**
 * @file IxEthAccCodeletMain.c
 *
 * @date 22 April 2004
 *
 * @brief This file contains the implementation of the Ethernet Access Codelet.
 *
 * Descriptions of the functions used in this codelet is contained in
 * IxEthAccCodelet_p.h
 *
 * IxEthAccCodelet API Functions:
 *       ixEthAccCodeletInit()
 *       ixEthAccCodeletUninit()
 *       ixEthAccCodeletRxSink()
 *       ixEthAccCodeletSwLoopback()
 *       ixEthAccCodeletTxGenRxSinkLoopback()
 *       ixEthAccCodeletPhyLoopback()
 *       ixEthAccCodeletBridge()
 *       ixEthAccCodeletBridgeQoS()
 *       ixEthAccCodeletBridgeFirewall()
 *       ixEthAccCodeletDBLearning()
 * 	 ixEthAccCodeletSwBridgeWiFi()
 *       ixEthAccCodeletShow()
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
 * Put the system defined include files required.
 */

#ifdef __vxworks
#include <end.h>    /* END drivers */
#include <endLib.h> /* END drivers */
#endif

/*
 * Put the user defined include files required.
 */

#include "IxOsal.h"
#include "IxQMgr.h"
#include "IxNpeDl.h"
#include "IxNpeMh.h"
#include "IxFeatureCtrl.h"
#include "IxEthDB.h"
#include "IxEthAccCodelet.h"

/* Includes all variables globally used in the codelet.
 *
 * EXTERN is defined before IxEthAccCodeletGlobal_p.h is included:
 * Global variables are defined for this file and set as extern
 * in the other files (default). As a result, only one instance of
 * the variable is created in this file and there is no declaration
 * duplication.
 */
#define EXTERN
#include "IxEthAccCodelet_p.h"


#ifdef __wince
#include "pkfuncs.h"
#include "oalintr.h"
#endif

/* correct rounding for the operation A * B / C 
 * without losing accuracy . This is based on the
 * approximation float(x)/float(y)+0.5 ~= (x + y/2)/y
 */
#ifdef _DIAB_TOOL
UINT64 __inline__ IX_ETHACC_CODELET_MULDIV(UINT64 A, UINT64 B, UINT64 C)
{
    return ((((A)*(B)) + (( (INT64) C)+1)/2) / (C));
}
#else
    #define IX_ETHACC_CODELET_MULDIV(A,B,C) 				\
    	({ 								\
	   UINT64 x = (((A)*(B)) + IX_OSAL_UDIV64_32((C)+1,2));		\
	   IX_OSAL_UDIV64_32(x, (C));					\
	})								
#endif

/* 
 * @def IX_ETHACC_CODELET_XCLOCK_FREQ 
 *
 * 2133 is the xclock frequency (timer block)
 *
 * @note: The conversion from APB bus cycles to microseconds 
 * is approximated by (cycles/66). The exact value
 * is (cycles*32/2133)
 */
#define IX_ETHACC_CODELET_XCLOCK_FREQ        2133
/* 
 * @def IX_ETHACC_CODELET_PCLOCK_DIVIDER  
 *
 * 32 is the pclock divider value (timer block)
 *
 * @note: The conversion from APB bus cycles to microseconds 
 * is approximated by (cycles/66). The exact value
 * is (cycles*32/2133)
 */
#define IX_ETHACC_CODELET_PCLOCK_DIVIDER       32

/*
 * @def IX_ETHACC_CODELET_USEC_PER_SEC
 *
 * Number of microsecond per second
 */
#define IX_ETHACC_CODELET_USEC_PER_SEC    1000000

/**
 * ETH_NPE_IMAGEIDS
 */

UINT32 ETH_NPEA_IMAGEID = IX_NPEDL_NPEIMAGE_NPEA_ETH_MACFILTERLEARN_HSSCHAN_COEXIST;
UINT32 ETH_NPEB_IMAGEID = IX_NPEDL_NPEIMAGE_NPEB_ETH_LEARN_FILTER_SPAN_MASK_FIREWALL_VLAN_QOS_EXTMIB;
UINT32 ETH_NPEC_IMAGEID = IX_NPEDL_NPEIMAGE_NPEC_ETH_LEARN_FILTER_SPAN_MASK_FIREWALL_VLAN_QOS_EXTMIB;

/*
 * Variable declarations global to this file only.
 */
PRIVATE IxOsalMutex ixEthAccCodeletStatsPollTaskRunning;
PRIVATE volatile BOOL ixEthAccCodeletTrafficPollEnabled;
PRIVATE volatile BOOL ixEthAccCodeletStatsPollTaskStop;
PRIVATE BOOL ixEthAccCodeletHardwareExists[IX_ETHACC_CODELET_MAX_PORT];
PRIVATE BOOL ixEthAccCodeletInitialised = FALSE;

/*
 * Static function prototypes.
 */
PRIVATE void ixEthAccCodeletStatsPollTask(void* arg, void** ptrRetObj);
PRIVATE IX_STATUS ixEthAccCodeletLoop(void);

/*
 * Function definition: ixEthAccCodeletMain()
 *
 * See header file for documentation.
 */
PUBLIC IX_STATUS
ixEthAccCodeletMain(IxEthAccCodeletOperation operationType, 
		    IxEthAccPortId inPort, 
		    IxEthAccPortId outPort, 
		    int disableStats)
{
    if( inPort >= IX_ETHACC_CODELET_MAX_PORT ||
	outPort >= IX_ETHACC_CODELET_MAX_PORT)
    {
        printf("ERROR: Invalid input. Please choose again...\n ");
	printf("in/out port IDs must be in the range 0 to %d\n",IX_ETHACC_CODELET_MAX_PORT - 1);
	return IX_FAIL ;
    }
 
    if (IX_SUCCESS != ixEthAccCodeletInit(operationType,inPort,outPort,disableStats))
    {
	printf("ixEthAccCodeletInit() fails ! Exit \n") ;
	return IX_FAIL ;
    }
    
    switch (operationType)
    {
    case IX_ETHACC_CODELET_RX_SINK :
         if (IX_SUCCESS != ixEthAccCodeletRxSink())
         {
	     printf("ixEthAccCodeletRxSink() fails ! Exit \n") ;
	     return IX_FAIL ;
         }
         /* Dump Stats */
         ixEthAccCodeletShow();
	 break ;

    case IX_ETHACC_CODELET_SW_LOOPBACK :
         if (IX_SUCCESS != ixEthAccCodeletSwLoopback())
         {
	     printf("ixEthAccCodeletSwLoopback() fails ! Exit \n") ;
	     return IX_FAIL ;
         }
         /* Dump Stats */
         ixEthAccCodeletShow();
	 break ;
  
    case IX_ETHACC_CODELET_TXGEN_RXSINK : 
         if (IX_SUCCESS != ixEthAccCodeletTxGenRxSinkLoopback(inPort,outPort))
         {
	     printf("ixEthAccCodeletTxGenRxSinkLoopback() fails ! Exit \n") ;
	     return IX_FAIL ;
         }
         /* Dump Stats */
         ixEthAccCodeletShow();
	 break ;

    case IX_ETHACC_CODELET_PHY_LOOPBACK :
         if(IX_SUCCESS != ixEthAccCodeletPhyLoopback())
         {
	     printf("ixEthAccCodeletPhyLoopback() fails ! Exit \n") ;
	     return IX_FAIL ;
	 }
         /* Dump Stats */
         ixEthAccCodeletShow();
	 break;

    case IX_ETHACC_CODELET_BRIDGE :
         if(IX_SUCCESS != ixEthAccCodeletSwBridge(inPort,outPort))
	 {
	     printf("ixEthAccCodeletBridge() fails ! Exit \n") ;
	     return IX_FAIL ;
	 }
         /* Dump Stats */
         ixEthAccCodeletShow();
	 break;

    case IX_ETHACC_CODELET_BRIDGE_QOS :
         if(IX_SUCCESS != ixEthAccCodeletSwBridgeQoS(inPort,outPort))
	 {
	     printf("ixEthAccCodeletBridgeQoS() fails ! Exit \n") ;
	     return IX_FAIL ;
	 }
         /* Dump Stats */
         ixEthAccCodeletShow();
	 break;

    case IX_ETHACC_CODELET_BRIDGE_FIREWALL :
         if(IX_SUCCESS != ixEthAccCodeletSwBridgeFirewall(inPort,outPort))
	 {
	     printf("ixEthAccCodeletBridgeFirewall() fails ! Exit \n") ;
	     return IX_FAIL ;
	 }
         /* Dump Stats */
         ixEthAccCodeletShow();
	 break;

    case IX_ETHACC_CODELET_ETH_LEARNING :
         if (IX_SUCCESS != ixEthAccCodeletDBLearning())
         {
	     printf("ixEthAccCodeletDBLearning() fails ! Exit \n") ;
	     return IX_FAIL ;
         }
         /* Dump Stats */
         ixEthAccCodeletShow();
	 break ;

    case IX_ETHACC_CODELET_BRIDGE_WIFI :
        if (IX_SUCCESS != ixEthAccCodeletSwBridgeWiFi(inPort,outPort))
        {
            printf("ixEthAccCodeletSwBridgeWiFi() fails ! Exit \n");
            return IX_FAIL;
        }
         /* Dump Stats */
         ixEthAccCodeletShow();
        break;

    default:
         printf("ERROR: Invalid input. Please choose again...\n ");
 	 printf(">ixEthAccCodeletMain(operationType,inPort,outPort)\n");
	 printf("Where operationType : %d = Rx Sink\n", IX_ETHACC_CODELET_RX_SINK);
	 printf("                      %d = Sw Loopback\n", IX_ETHACC_CODELET_SW_LOOPBACK);
	 printf("                      %d = Tx Gen/Rx Sink Loopback\n", IX_ETHACC_CODELET_TXGEN_RXSINK);
	 printf("                      %d = Perform PHY loopback on the same port\n",IX_ETHACC_CODELET_PHY_LOOPBACK);
	 printf("                      %d = Bridge\n",IX_ETHACC_CODELET_BRIDGE);
	 printf("                      %d = Bridge + QoS\n",IX_ETHACC_CODELET_BRIDGE_QOS);
	 printf("                      %d = Bridge + Firewall\n",IX_ETHACC_CODELET_BRIDGE_FIREWALL);
         printf("                      %d = Eth DB Learning\n", IX_ETHACC_CODELET_ETH_LEARNING);
         printf("                      %d = Bridge + WiFi header conversion\n", IX_ETHACC_CODELET_BRIDGE_WIFI);
         return IX_FAIL ;
    }

    return IX_SUCCESS ;
}

/*
 * Function definition: ixEthAccCodeletInit()
 *
 * See header file for documentation.
 */
IX_STATUS ixEthAccCodeletInit(IxEthAccCodeletOperation operationType,
                    IxEthAccPortId inPort,
                    IxEthAccPortId outPort, 
		    int disableStats)
{  
    IxEthAccPortId portId;
    UINT32 portIndex;
    IxOsalThread statsPollThread;
    IxOsalThreadAttr threadAttr;
    IxFeatureCtrlDeviceId deviceId;

    threadAttr.name      = "Codelet Stats";
    threadAttr.stackSize = 32 * 1024; /* 32kbytes */
    threadAttr.priority  = 128;

    /* Set the expansion bus fuse register to enable MUX for NPEA MII */
    if ((ixFeatureCtrlDeviceRead() == IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X) ||
        (ixFeatureCtrlDeviceRead() == IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X) )
    {
        UINT32 expbusCtrlReg;
        expbusCtrlReg = ixFeatureCtrlRead ();
        expbusCtrlReg |= ((unsigned long)1<<8);
        ixFeatureCtrlWrite (expbusCtrlReg);
    }

    /* check the component is already initialized */
    if(ixEthAccCodeletInitialised) 
    {
	printf("CodeletMain: Ethernet codelet already initialised\n");
	return(IX_SUCCESS);
    }

#ifdef __vxworks
    /* When the ixe drivers are running, the codelets
    * cannot run.
    */
    for (portId = 0; portId < IX_ETHACC_CODELET_MAX_PORT; portId++)
    {
        if (endFindByName ("ixe", portId) != NULL)
        {
            printf("CodeletMain: FAIL: Driver ixe%d detected\n",portId);
            return IX_FAIL;
        }
    }
#endif

    /* Initialize NPE IMAGE ID here again to prevent confusion in multiple 
     * ixEthAccCodeletMain() calls with different operationType.
     */   
    ETH_NPEA_IMAGEID = IX_NPEDL_NPEIMAGE_NPEA_ETH_MACFILTERLEARN_HSSCHAN_COEXIST;
    ETH_NPEB_IMAGEID = IX_NPEDL_NPEIMAGE_NPEB_ETH_LEARN_FILTER_SPAN_MASK_FIREWALL_VLAN_QOS_EXTMIB;
    ETH_NPEC_IMAGEID = IX_NPEDL_NPEIMAGE_NPEC_ETH_LEARN_FILTER_SPAN_MASK_FIREWALL_VLAN_QOS_EXTMIB;

    /* Create mutexes for thread control */
    ixEthAccCodeletStatsPollTaskStop = TRUE;
    ixOsalMutexInit (&ixEthAccCodeletStatsPollTaskRunning);

    /* Initialise MBUF pool */
    if(ixEthAccCodeletMemPoolInit() != IX_SUCCESS)
    {
	printf("CodeletMain: Error initialising mBuf pool\n");
	return (IX_FAIL);
    }

    /* Check Silicon stepping */
    printf("Checking Silicon stepping...\n");
    deviceId = ixFeatureCtrlDeviceRead();

    switch(deviceId)
    {
       
        case IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X:
        
        /*
         * We only enable port when its corresponding  
         * Eth Coprocessor is available.
         */
        if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) == 
           IX_FEATURE_CTRL_COMPONENT_ENABLED)
        {
            ixEthAccCodeletHardwareExists[IX_ETH_PORT_1] = TRUE;
        }

        if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) == 
            IX_FEATURE_CTRL_COMPONENT_ENABLED)
        {
            ixEthAccCodeletHardwareExists[IX_ETH_PORT_2] = TRUE;
        }
        break;

        case IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X:
             ixEthAccCodeletHardwareExists[IX_ETH_PORT_1] = TRUE;
             ixEthAccCodeletHardwareExists[IX_ETH_PORT_2] = TRUE;
             ixEthAccCodeletHardwareExists[IX_ETH_PORT_3] = TRUE;
             break;

        case IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X:
             ixEthAccCodeletHardwareExists[IX_ETH_PORT_2] = TRUE;
             ixEthAccCodeletHardwareExists[IX_ETH_PORT_3] = TRUE;
     	     break;

        default:
	    printf("CodeletMain: Error. Operation for silicon Id %d is undefined!.\n",deviceId);
            return (IX_FAIL);
	    break;
    }
    /***********************************************************************
     *
     * System initialisation done. Now initialise Access components. 
     *
     ***********************************************************************/

    /* Initialise Queue Manager */
    printf("Initialising Queue Manager...\n");
    if (ixQMgrInit() != IX_SUCCESS)
    {
	printf("CodeletMain: Error initialising queue manager!\n");
	return (IX_FAIL);
    }

    /* Start the Queue Manager dispatcher */   
    if(ixEthAccCodeletDispatcherStart(IX_ETH_CODELET_QMGR_DISPATCH_MODE) 
       != IX_SUCCESS)
    {
	printf("CodeletMain: Error starting queue manager dispatch loop!\n");
	return (IX_FAIL);
    }

    /* Initialise NPE Message handler */
    printf("\nStarting NPE message handler...\n");
    if(ixNpeMhInitialize(IX_NPEMH_NPEINTERRUPTS_YES) != IX_SUCCESS)
    {
	printf("CodeletMain: Error initialising NPE Message handler!\n");
	return (IX_FAIL);
    }

    /* Initialise NPEs firmware */
    printf ("Initialising NPEs...\n");
    if (ixEthAccCodeletHardwareExists[IX_ETH_PORT_1])
    {
        if ((operationType == IX_ETHACC_CODELET_BRIDGE_WIFI) && (inPort == IX_ETH_PORT_1))
        {
            printf("CodeletMain: the 802.3 <=> 802.11 header conversion image is loaded on NPE B\n");
            ETH_NPEB_IMAGEID = IX_NPEDL_NPEIMAGE_NPEB_ETH_SPAN_VLAN_QOS_HDR_CONV_EXTMIB;
        }

	if (IX_SUCCESS != ixNpeDlNpeInitAndStart(ETH_NPEB_IMAGEID))
        {
	    printf ("CodeletMain: Error initialising and starting NPE B!\n");
	    return (IX_FAIL);
	}
    }

    if (ixEthAccCodeletHardwareExists[IX_ETH_PORT_2])
    {
        if ((operationType == IX_ETHACC_CODELET_BRIDGE_WIFI) && (inPort == IX_ETH_PORT_2))
        {
            printf("CodeletMain: the 802.3 <=> 802.11 header conversion image is loaded on NPE C\n");
            ETH_NPEC_IMAGEID = IX_NPEDL_NPEIMAGE_NPEC_ETH_SPAN_VLAN_QOS_HDR_CONV_EXTMIB;
        }

	if (IX_SUCCESS != ixNpeDlNpeInitAndStart(ETH_NPEC_IMAGEID))
        {
	    printf ("CodeletMain: Error initialising and starting NPE C!\n");
	    return (IX_FAIL);
	}
    }
    if (ixEthAccCodeletHardwareExists[IX_ETH_PORT_3])
    {
        if ((operationType == IX_ETHACC_CODELET_BRIDGE_WIFI) && (inPort == IX_ETH_PORT_3))
        {
            printf("CodeletMain: the 802.3 <=> 802.11 header conversion image is loaded on NPE A\n");
            ETH_NPEA_IMAGEID = IX_NPEDL_NPEIMAGE_NPEA_ETH_SPAN_VLAN_QOS_HDR_CONV_EXTMIB;
        }

	if (IX_SUCCESS != ixNpeDlNpeInitAndStart(ETH_NPEA_IMAGEID))
        {
	    printf ("CodeletMain: Error initialising and starting NPE A!\n");
	    return (IX_FAIL);
	}
    }

    printf ("Initialising Access Layers\n");

    /* Enable QoS on ethDB. This has to be done before ethAcc initialisation */
    if (operationType == IX_ETHACC_CODELET_BRIDGE_QOS)
    {
	printf("Enabling QoS\n");
        if (IX_ETH_DB_SUCCESS != ixEthDBInit())
	{
	    printf ("CodeletMain: Error initialising EthDB\n");
	    return (IX_FAIL);
	}

	(void)ixEthDBPortInit(inPort);

        if (IX_ETH_DB_SUCCESS != ixEthDBFeatureEnable(inPort, 
						      IX_ETH_DB_VLAN_QOS, 
						      TRUE))
	{
            printf("CodeletMain: Error enabling QoS on port %d\n",inPort);
	    return (IX_FAIL);
        }
    }

    /* initialise ethAcc : QoS, if needed is already configured */
    if (ixEthAccInit() != IX_ETH_ACC_SUCCESS)
    {
	printf("CodeletMain: Error initialising Ethernet access driver!\n");
	return (IX_FAIL);
    }

    /***********************************************************************
     *
     * Access components initialisation done. Now initialize the ports
     *
     ***********************************************************************/

    /* Configure all available ports */
    for (portIndex = 0; portIndex < IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
    {   
	portId = IX_ETHNPE_INDEX_TO_PORT_ID(portIndex);
	if (ixEthAccCodeletHardwareExists[portId])
	{
	    if(ixEthAccCodeletPortInit(portId) != IX_ETH_ACC_SUCCESS)
            {
   	        printf("CodeletMain: Error setup port %u\n",
		       portId);
	        return (IX_FAIL);
            }
        }
    }

    /* Find and initialise all available PHYs */
    printf ("Discover and reset the PHYs...\n");
    if(ixEthAccCodeletPhyInit() != IX_SUCCESS)
    {
	printf("CodeletMain: Error initialising Ethernet phy(s)!\n");
	return (IX_FAIL);
    }

    /***********************************************************************
     *
     * PortInitialization done. Now start the codelet features
     *
     ***********************************************************************/

    /* starts ethDB maintenance running from a different task */
    if (ixEthAccCodeletDBMaintenanceStart()
	!= IX_SUCCESS)
    {
	printf("CodeletMain: Error spawning DB maintenance task\n");
	return (IX_FAIL);
    }

    /* Starts the traffic display (in a different task) this is initially
     * set to FALSE in order to allow the traffic stats to start only
     * once traffic is started to be received 
     */
    ixEthAccCodeletTrafficPollEnabled = FALSE;

    if(disableStats == 0)
    {
    	if (ixOsalThreadCreate(&statsPollThread,
			   &threadAttr,
			   (IxOsalVoidFnVoidPtr) ixEthAccCodeletStatsPollTask,
			   NULL)	
		!= IX_SUCCESS)
    	{
	    printf("CodeletMain: Error spawning stats task\n");
	    return (IX_FAIL);
    	}
    
     	/* Start the thread */
    	if (ixOsalThreadStart(&statsPollThread) != IX_SUCCESS)
    	{
	    printf("CodeletMain: Error failed to start the stats thread\n");
        	return IX_FAIL;
    	}
    }

    ixEthAccCodeletInitialised = TRUE;
    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletUninit()
 *
 * Stop all threads and interrupts
 */

IX_STATUS ixEthAccCodeletUninit(void)
{  
    IxEthAccPortId portIndex;
    IxEthAccStatus status;

    if(!ixEthAccCodeletInitialised) 
    {
	/* already uninitialized */
	return(IX_SUCCESS);
    }

    if (!ixEthAccCodeletStatsPollTaskStop)
    {
	ixEthAccCodeletStatsPollTaskStop = TRUE;
	if (ixOsalMutexLock (&ixEthAccCodeletStatsPollTaskRunning, 
			     IX_OSAL_WAIT_FOREVER)
	    != IX_SUCCESS)
	{
	    printf("CodeletMain: Error stopping Statistics Polling thread!\n");
	    return (IX_FAIL);
	}
	ixOsalMutexUnlock (&ixEthAccCodeletStatsPollTaskRunning);
    }

    for (portIndex = 0; portIndex < IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
    {
	status = ixEthAccPortDisable (IX_ETHNPE_INDEX_TO_PORT_ID(portIndex));
	if (IX_ETH_ACC_SUCCESS != status)
	{
	    printf("CodeletMain: Failed to disable port %d, error code %d\n", IX_ETHNPE_INDEX_TO_PORT_ID(portIndex), status); 
	    return (IX_FAIL);
	} 
    }

    if (ixEthAccCodeletDBMaintenanceStop() != IX_SUCCESS)
    {
	printf("CodeletMain: Error stopping DB Maintenance task!\n");
	return (IX_FAIL);
    }

    if (ixEthAccCodeletPhyUninit() != IX_SUCCESS)
    {
	printf("CodeletMain: Error stopping Phy Uninit task!\n");
	return (IX_FAIL);
    }

    if (ixEthAccUninit() != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to uninitialize Ethernet Access Layer!\n");
	return (IX_FAIL);
    }

    if( (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == ixFeatureCtrlDeviceRead ()) ||
	 IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X == ixFeatureCtrlDeviceRead () )
    {	
    	if (ixNpeDlNpeStopAndReset(IX_NPEDL_NPEID_NPEA) != IX_SUCCESS)
    	{
	    printf("CodeletMain: Failed to stop and reset NPE A!\n");
	    return (IX_FAIL);
	}
    }

    if( (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == ixFeatureCtrlDeviceRead ()) ||
	 IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X == ixFeatureCtrlDeviceRead () )
    {	
    	if (ixNpeDlNpeStopAndReset(IX_NPEDL_NPEID_NPEB) != IX_SUCCESS)
    	{
	    printf("CodeletMain: Failed to stop and reset NPE B!\n");
    	    return (IX_FAIL);
    	}
    }

    if (ixNpeDlNpeStopAndReset(IX_NPEDL_NPEID_NPEC) != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to stop and reset NPE C!\n");
	return (IX_FAIL);
    }

    if (ixNpeDlUnload() != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to unload NPE Downloader!\n");
	return (IX_FAIL);
    }

    if (ixNpeMhUnload() != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to unload NPE Message Handler!\n");
	return (IX_FAIL);
    }

    if (ixEthAccCodeletDispatcherStop(IX_ETH_CODELET_QMGR_DISPATCH_MODE)
	!= IX_SUCCESS)
    {
	printf("CodeletMain: Error stopping QMgr Dispatcher loop!\n");
	return (IX_FAIL);
    }

    if (ixQMgrUnload() != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to unload QMgr!\n");
	return (IX_FAIL);
    }

    if (ixEthAccCodeletMemPoolFree() != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to free memory pool!\n");
	return (IX_FAIL);
    }

    ixEthAccCodeletInitialised = FALSE;

    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletRxSink()
 *
 * See header file for documentation.
 */

IX_STATUS 
ixEthAccCodeletRxSink(void)
{
    UINT32 portIndex;
    UINT32 portId;

    IX_ETHACC_IS_CODELET_INITIALISED();

    /* Configure all available ports and start traffic */
    for (portIndex = 0; portIndex < IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
    {
	portId = IX_ETHNPE_INDEX_TO_PORT_ID(portIndex);
	if (ixEthAccCodeletHardwareExists[portId])
	{
	    if (ixEthAccCodeletRxSinkStart(portId) 
		!= IX_ETH_ACC_SUCCESS)
	    {
		printf("CodeletMain: Failed to configure the RX Sink Operation Port %u\n",
		       (UINT32)portId);
		return IX_FAIL;
	    }
	}
    }

    printf("\n");
    printf("Rx Sink Operation started.\n");
    printf("Begin sending packets in from external source\n");
    printf("\n");

    /* wait for user input */
    if(ixEthAccCodeletLoop() != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to get User Input (should never happen)");
	return (IX_FAIL);
    }
    
    /* Stop traffic and unconfigure all available ports */
    for (portIndex = 0; portIndex < IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
    {
	portId = IX_ETHNPE_INDEX_TO_PORT_ID(portIndex);
	if (ixEthAccCodeletHardwareExists[portId])
	{
	    if (ixEthAccCodeletRxSinkStop(portId) 
		!= IX_ETH_ACC_SUCCESS)
	    {
		printf("CodeletMain: Failed to unconfigure the RX Sink Operation Port %u\n",
		       (UINT32)portId);
		return IX_FAIL;
	    }
	}
    }

    /* wait for pending traffic to be completely received */
    if (ixEthAccCodeletRecoverBuffers() 
	!= IX_SUCCESS)
    {
	printf("CodeletMain: Warning! Not all buffers are accounted for\n");
    }

    return (IX_SUCCESS);
}


/*
 * Function definition: ixEthAccCodeletSwLoopback()
 *
 * See header file for documentation.
 */

IX_STATUS 
ixEthAccCodeletSwLoopback(void)
{
    UINT32 portIndex;
    UINT32 portId;

    IX_ETHACC_IS_CODELET_INITIALISED();

    /* Configure all available ports */
    for (portIndex = 0; portIndex < IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
    {  
	portId = IX_ETHNPE_INDEX_TO_PORT_ID(portIndex);
	if (ixEthAccCodeletHardwareExists[portId])
	{
	    if (ixEthAccCodeletSwLoopbackStart(portId) 
		!= IX_ETH_ACC_SUCCESS)
	    {
		printf("CodeletMain: Failed to configure the Sw Loopback Operation Port %u\n",
		       (UINT32)portId);
		return IX_FAIL;
	    }
	}
    }

    printf("\n");
    printf("Software loopback successfully started.\n");
    printf("Begin sending packets from external source\n");  
    printf("\n");

    /* wait for user input */
    if(ixEthAccCodeletLoop() != IX_SUCCESS)
    {
	return (IX_FAIL);
    }

    /* Stop traffic and unconfigure all available ports */
    for (portIndex = 0; portIndex < IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
    {
	portId = IX_ETHNPE_INDEX_TO_PORT_ID(portIndex);
	if (ixEthAccCodeletHardwareExists[portId])
	{
	    if (ixEthAccCodeletSwLoopbackStop(portId) 
		!= IX_ETH_ACC_SUCCESS)
	    {
		printf("CodeletMain: Failed to unconfigure the Sw Loopback Operation Port %u\n",
		       (UINT32)portId);
		return IX_FAIL;
	    }
	}
    }

    /* wait for pending traffic to be completely received */
    if (ixEthAccCodeletRecoverBuffers() 
	!= IX_SUCCESS)
    {
	printf("CodeletMain: Warning! Not all buffers are accounted for\n");
    }
    return (IX_SUCCESS);
}


/*
 * Function definition: ixEthAccCodeletTxGenRxSinkLoopback()
 *
 * See header file for documentation.
 */

IX_STATUS 
ixEthAccCodeletTxGenRxSinkLoopback(IxEthAccPortId inPort,
				   IxEthAccPortId outPort)
{
    IX_ETHACC_IS_CODELET_INITIALISED();
    
#if defined(__linux) && (IX_ETH_CODELET_QMGR_DISPATCH_MODE==TRUE)
    printf("\n*********************************************************************\n");
    printf("WARNING: This operation uses too much CPU time in interrupt mode to display\n");
    printf("         any counters. Please recompile the codelet for polled mode when\n");
    printf("         using this operation\n\n");
    printf("Polled mode: set IX_ETH_CODELET_QMGR_DISPATCH_MODE to FALSE in IxEthAccCodelet_p.h\n");
    printf("*********************************************************************\n\n");
#endif

    /* This configuration requires two NPE be available to use */
    if ( !((ixEthAccCodeletHardwareExists[inPort]) &&
	   (ixEthAccCodeletHardwareExists[outPort]) ))
    {
	printf("CodeletMain: Failed to start Tx Gen RX Sink Operation as two NPEs are needed\n");
	return (IX_FAIL);
    }

    if (ixEthAccCodeletTxGenRxSinkLoopbackStart(outPort, inPort)
        != IX_ETH_ACC_SUCCESS)
    {
	printf("CodeletMain: Failed to start the Tx Gen RX Sink Operation\n");
	return IX_FAIL;
    }

    printf("\n");
    printf("TxGen RxSink successfully started.\n");
    printf("Begin sending packets from external source\n");  
    printf("Or use a crossover cable between port %d and port %d\n",inPort,outPort);  
    printf("\n");

    /* wait for user input */
    if (ixEthAccCodeletLoop() != IX_SUCCESS)
    {
	return (IX_FAIL);
    }
    
    /* stop traffic, recover buffers */
    if (ixEthAccCodeletTxGenRxSinkLoopbackStop(outPort, inPort)
	    != IX_ETH_ACC_SUCCESS)
    {
	printf("CodeletMain: Failed to stop the Tx Gen RX Sink Operation\n");
	return (IX_FAIL);
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
 * Function definition: ixEthAccCodeletPhyLoopback()
 *
 * See header file for documentation.
 */
IX_STATUS
ixEthAccCodeletPhyLoopback(void)
{
    UINT32 portIndex;
    UINT32 portId;

    IX_ETHACC_IS_CODELET_INITIALISED();

#if defined(__linux) && (IX_ETH_CODELET_QMGR_DISPATCH_MODE==TRUE)
    printf("\n*********************************************************************\n");
    printf("WARNING: This operation uses too much CPU time in interrupt mode to display\n");
    printf("         any counters. Please recompile the codelet for polled mode when\n");
    printf("         using this operation.\n\n");
    printf("Polled mode: set IX_ETH_CODELET_QMGR_DISPATCH_MODE to FALSE in IxEthAccCodelet_p.h\n");
    printf("*********************************************************************\n\n");
#endif    /* Configure all available ports */

    for (portIndex = 0; portIndex < IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
    {
	portId = IX_ETHNPE_INDEX_TO_PORT_ID(portIndex);
	if (ixEthAccCodeletHardwareExists[portId])
	{
	    if (ixEthAccCodeletPhyLoopbackStart(portId) 
		!= IX_ETH_ACC_SUCCESS)
	    {
		printf("CodeletMain: Failed to configure the PHY Loopback Operation on Port %u\n",
		       (UINT32)portId);
		return IX_FAIL;
	    }
	}
    }

    printf("\n");
    printf("PHY Loopback operation successfully started\n");
    printf("\n");

    /* wait for user input */
    if(ixEthAccCodeletLoop() != IX_SUCCESS)
    {
	return (IX_FAIL);
    }

    /* Stop traffic and unconfigure all available ports */
    for (portIndex = 0; portIndex < IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
    {
	portId = IX_ETHNPE_INDEX_TO_PORT_ID(portIndex);
	if (ixEthAccCodeletHardwareExists[portId])
	{
	    if (ixEthAccCodeletPhyLoopbackStop(portId) 
		!= IX_ETH_ACC_SUCCESS)
	    {
		printf("CodeletMain: Failed to unconfigure the PHY Loopback Operation on Port %u\n",
		       (UINT32)portId);
		return IX_FAIL;
	    }
	}
    }

    /* wait for pending traffic to be completely received */
    if (ixEthAccCodeletRecoverBuffers() 
	!= IX_SUCCESS)
    {
	printf("Warning: Not all buffers are accounted for!\n");
    }

    return (IX_SUCCESS);
}

/*
 * Function definition: ixEthAccCodeletBridge()
 *
 * See header file for documentation.
 */

IX_STATUS 
ixEthAccCodeletSwBridge(IxEthAccPortId inPort,
			IxEthAccPortId outPort)
{
    IX_ETHACC_IS_CODELET_INITIALISED();

    if (!ixEthAccCodeletHardwareExists[inPort]
	|| !ixEthAccCodeletHardwareExists[outPort])
    {
	/* Bridge operation only works 
	 * when two Eth NPEs are available
	 *
	 */
	printf("\n*********************************************************************\n");
	printf ("Bridge operation needs two Eth coprocessors.\n");
	printf ("Underlying silicon has only one Eth coprocessor!\n");
	printf("\n*********************************************************************\n");
	return IX_FAIL ; 
    }

    if ( ixEthAccCodeletSwBridgeStart(inPort, 
				      outPort)
	 != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to start the Bridge Operation\n");
	return (IX_FAIL);
    }

    printf("\n");
    printf("Bridge Operation started. Begin sending packets from\n");
    printf("external source between port %u and port %u\n", 
	   (UINT32)inPort, 
	   (UINT32)outPort);
    printf("\n");

    if(ixEthAccCodeletLoop()
       != IX_SUCCESS)
    {
	return (IX_FAIL);
    }

    if ( ixEthAccCodeletSwBridgeStop(inPort, 
				     outPort) 
	 != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to stop the Bridge Operation\n");
	return (IX_FAIL);
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
 * Function definition: ixEthAccCodeletBridge()
 *
 * See header file for documentation.
 */

IX_STATUS 
ixEthAccCodeletSwBridgeQoS(IxEthAccPortId inPort,
			   IxEthAccPortId outPort)
{
    IX_ETHACC_IS_CODELET_INITIALISED();

    if (!ixEthAccCodeletHardwareExists[inPort]
	|| !ixEthAccCodeletHardwareExists[outPort])
    {
	/* Bridge operation only works 
	 * when both Eth NPEs are available
	 *
	 */
	printf("\n*********************************************************************\n");
	printf ("Bridge operation needs two Eth coprocessors.\n");
	printf ("Underlying silicon has only one Eth coprocessor!\n");
	printf("\n*********************************************************************\n");
	return IX_FAIL ; 
    }

    if ( ixEthAccCodeletSwBridgeQoSStart(inPort, 
					 outPort)
	 != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to start the QoS Bridge Operation\n");
	return (IX_FAIL);
    }

    printf("\n");
    printf("Bridge Operation started. Begin sending packets from\n");
    printf("external source between port %u and port %u\n", 
	   (UINT32)inPort, 
	   (UINT32)outPort);
    printf("\n");

    if(ixEthAccCodeletLoop()
       != IX_SUCCESS)
    {
	return (IX_FAIL);
    }

    if ( ixEthAccCodeletSwBridgeQoSStop(inPort, 
					outPort) 
	 != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to stop the QoS Bridge Operation\n");
	return (IX_FAIL);
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
 * Function definition: ixEthAccCodeletBridgeFirewall()
 *
 * See header file for documentation.
 */

IX_STATUS 
ixEthAccCodeletSwBridgeFirewall(IxEthAccPortId inPort,
				IxEthAccPortId outPort)
{
    IX_ETHACC_IS_CODELET_INITIALISED();

    if (!ixEthAccCodeletHardwareExists[inPort]
	|| !ixEthAccCodeletHardwareExists[outPort])
    {
	/* Bridge operation only works 
	 * when both Eth NPEs are available
	 *
	 */
	printf("\n*********************************************************************\n");
	printf ("Bridge operation needs two Eth coprocessors.\n");
	printf ("Underlying silicon has only one Eth coprocessor!\n");
	printf("\n*********************************************************************\n");
	return IX_FAIL ; 
    }

    if ( ixEthAccCodeletSwBridgeFirewallStart(inPort, 
					      outPort)
	 != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to start the Firewall Bridge Operation\n");
	return (IX_FAIL);
    }

    printf("\n");
    printf("Bridge Operation started. Begin sending packets from\n");
    printf("external source between port %u and port %u\n", 
	   (UINT32)inPort, 
	   (UINT32)outPort);
    printf("All frame with unknown source MAC addresses are filtered\n");
    printf("\n");

    if(ixEthAccCodeletLoop()
       != IX_SUCCESS)
    {
	return (IX_FAIL);
    }

    if ( ixEthAccCodeletSwBridgeFirewallStop(inPort, 
					     outPort) 
	 != IX_SUCCESS)
    {
	printf("CodeletMain: Failed to stop the Firewall Bridge Operation\n");
	return (IX_FAIL);
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
 * Function definition: ixEthAccCodeletSwBridgeWiFi()
 *
 * See header file for documentation.
 */

IX_STATUS 
ixEthAccCodeletSwBridgeWiFi(IxEthAccPortId inPort,
			    IxEthAccPortId outPort)
{
    IX_ETHACC_IS_CODELET_INITIALISED();

    if (!ixEthAccCodeletHardwareExists[inPort]
	|| !ixEthAccCodeletHardwareExists[outPort])
    {
	/* Bridge operation only works 
	 * when both Eth NPEs are available
	 *
	 */
	printf("\n*********************************************************************\n");
	printf ("Bridge operation needs two Eth coprocessors.\n");
	printf ("Underlying silicon has only one Eth coprocessor!\n");
	printf("\n*********************************************************************\n");
	return IX_FAIL ; 
    }

    /* to demonstate 802.11 to 802.3 header conversion, 
     * invalid source address filtering behavior has to be turned off.
     * This prevent WiFi packet from been filtered by NPE
     */
     if (IX_ETH_DB_SUCCESS != ixEthDBFeatureEnable(outPort, 
        					   IX_ETH_DB_FIREWALL, 
						   TRUE))
     {
         printf("CodeletMain: Error enabling firewall on port %d\n",outPort);
	 return (IX_FAIL);
     }

     ixEthDBFirewallInvalidAddressFilterEnable(outPort, FALSE);

     if (IX_ETH_DB_SUCCESS != ixEthDBFeatureEnable(outPort, 
        					   IX_ETH_DB_FIREWALL, 
						   FALSE))
     {
         printf("CodeletMain: Error disabling firewall on port %d\n",outPort);
	 return (IX_FAIL);
     }

    if ( ixEthAccCodeletSwBridgeWiFiStart(inPort, 
					      outPort)
	 != IX_SUCCESS)
    {
	printf("Failed to start the Bridge Operation\n");
	return (IX_FAIL);
    }

    if(ixEthAccCodeletLoop()
       != IX_SUCCESS)
    {
	return (IX_FAIL);
    }

    if ( ixEthAccCodeletSwBridgeWiFiStop(inPort, 
					     outPort) 
	 != IX_SUCCESS)
    {
	printf("Failed to stop the Bridge Operation\n");
	return (IX_FAIL);
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
 * Function definition: ixEthAccCodeletDBLearning()
 *
 * See header file for documentation.
 */

IX_STATUS 
ixEthAccCodeletDBLearning(void)
{
    IX_ETHACC_IS_CODELET_INITIALISED();

    /* Configure ethDB tests for all available ports */
    if (ixEthAccCodeletDBLearningRun(ixEthAccCodeletHardwareExists) 
	!= IX_ETH_ACC_SUCCESS)
    {
	printf("CodeletMain: Failed to run ethDB configuration\n");
	return IX_FAIL;
    }

    return (IX_SUCCESS);
}


/***********************************************************************
 *
 * Codelet utility functions
 *
 ***********************************************************************/

/*
 * Function definition: ixEthAccCodeletShow()
 *
 * See header file for documentation.
 */

void ixEthAccCodeletShow(void)
{
    static IxEthEthObjStats *portStats = NULL;
    UINT32 portIndex;
    UINT32 portId;

    if (!ixEthAccCodeletInitialised)
    {
	printf("CodeletMain: Codelet is not initialized!\n");
	return;
    }

    if(portStats == NULL)
    {
	/*
	 * The statistics are gathered by the NPE therefore we use DMA_MALLOC
	 * to make it cache safe for us to read.
	 */
	if((portStats = IX_OSAL_CACHE_DMA_MALLOC(sizeof(IxEthEthObjStats))) == NULL)
	{
	    printf("CodeletMain: Unable to create port stats!\n");
	    return;
	}
    }

    for(portIndex=0; portIndex< IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
    {
	portId = IX_ETHNPE_INDEX_TO_PORT_ID(portIndex);
	if (ixEthAccCodeletHardwareExists[portId])
	{
	    ixOsalMemSet(portStats, 0, sizeof(IxEthEthObjStats));
	    
	    printf("\nStatistics for port %d:\n", portId);
	    if(ixEthAccMibIIStatsGetClear(portId, portStats) != IX_ETH_ACC_SUCCESS)
	    {
		printf("Unable to retrieve statistics for port %d!\n", portId);
	    }
	    else
	    {
                printf (" dot3portStatsAlignmentErrors:           %u\n",
                    (unsigned) portStats->dot3StatsAlignmentErrors);
                printf (" dot3portStatsFCSErrors:                 %u\n",
                    (unsigned) portStats->dot3StatsFCSErrors);
                printf (" dot3portStatsInternalMacReceiveErrors:  %u\n",
                    (unsigned) portStats->dot3StatsInternalMacReceiveErrors);        
                printf (" RxOverrunDiscards:                      %u\n",
                    (unsigned) portStats->RxOverrunDiscards);
                printf (" RxLearnedEntryDiscards:                 %u\n",
                    (unsigned) portStats->RxLearnedEntryDiscards);
                printf (" RxLargeFramesDiscards:                  %u\n",
                    (unsigned) portStats->RxLargeFramesDiscards);
                printf (" RxSTPBlockedDiscards:                   %u\n",
                    (unsigned) portStats->RxSTPBlockedDiscards);    
                printf (" RxVLANTypeFilterDiscards:               %u\n",
                    (unsigned) portStats->RxVLANTypeFilterDiscards);
                printf (" RxVLANIdFilterDiscards:                 %u\n",
                    (unsigned) portStats->RxVLANIdFilterDiscards);
                printf (" RxInvalidSourceDiscards:                %u\n",
                    (unsigned) portStats->RxInvalidSourceDiscards);
                printf (" RxWhiteListDiscards:                    %u\n",
                    (unsigned) portStats->RxWhiteListDiscards);
                printf (" RxBlackListDiscards:                    %u\n",
                    (unsigned) portStats->RxBlackListDiscards);
                printf (" RxUnderflowEntryDiscards:               %u\n",
                    (unsigned) portStats->RxUnderflowEntryDiscards);
                printf (" dot3portStatsSingleCollisionFrames:     %u\n",
                    (unsigned) portStats->dot3StatsSingleCollisionFrames);
                printf (" dot3portStatsMultipleCollisionFrames:   %u\n",
                    (unsigned) portStats->dot3StatsMultipleCollisionFrames);
                printf (" dot3portStatsDeferredTransmissions:     %u\n",
                    (unsigned) portStats->dot3StatsDeferredTransmissions);
                printf (" dot3portStatsLateCollisions:            %u\n",
                    (unsigned) portStats->dot3StatsLateCollisions);
                printf (" dot3portStatsExcessiveCollsions:        %u\n",
                    (unsigned) portStats->dot3StatsExcessiveCollsions);
                printf (" dot3portStatsInternalMacTransmitErrors: %u\n",
                    (unsigned) portStats->dot3StatsInternalMacTransmitErrors);
                printf (" dot3portStatsCarrierSenseErrors:        %u\n",
                    (unsigned) portStats->dot3StatsCarrierSenseErrors);
                printf (" TxLargeFrameDiscards:                   %u\n",
                    (unsigned) portStats->TxLargeFrameDiscards);
                printf (" TxVLANIdFilterDiscards:                 %u\n",
                    (unsigned) portStats->TxVLANIdFilterDiscards);
                printf (" RxValidFramesTotalOctets:               %u\n",
                    (unsigned) portStats->RxValidFramesTotalOctets);
                printf (" RxUcastPkts:                            %u\n",
                    (unsigned) portStats->RxUcastPkts);
                printf (" RxBcastPkts:                            %u\n",
                    (unsigned) portStats->RxBcastPkts);
                printf (" RxMcastPkts:                            %u\n",
                    (unsigned) portStats->RxMcastPkts);
                printf (" RxPkts64Octets:                         %u\n",
                    (unsigned) portStats->RxPkts64Octets);
                printf (" RxPkts65to127Octets:                    %u\n",
                    (unsigned) portStats->RxPkts65to127Octets);
                printf (" RxPkts128to255Octets:                   %u\n",
                    (unsigned) portStats->RxPkts128to255Octets);
                printf (" RxPkts256to511Octets:                   %u\n",
                    (unsigned) portStats->RxPkts256to511Octets);
                printf (" RxPkts512to1023Octets:                  %u\n",
                    (unsigned) portStats->RxPkts512to1023Octets);
                printf (" RxPkts1024to1518Octets:                 %u\n",
                    (unsigned) portStats->RxPkts1024to1518Octets);
                printf (" RxInternalNPEReceiveErrors:             %u\n",
                    (unsigned) portStats->RxInternalNPEReceiveErrors);
                printf (" TxInternalNPETransmitErrors:            %u\n",
                    (unsigned) portStats->TxInternalNPETransmitErrors);
		printf("\n");
	    }
	}
    }
}

/**
 * @fn void ixEthAccCodeletStatsPollTask
 *
 * This task polls the Codelet Stats and displays the rate of Rx and 
 * Tx packets per second.
 * 
 */
PRIVATE void ixEthAccCodeletStatsPollTask(void* arg, void** ptrRetObj)
{
    UINT32 portIndex = 0;
    IxEthAccPortId portId;
    static char stillRunning[] = "|/-\\";
    static int  stillRunningIndex = 0;
    static char displayString[20 + (21 * IX_ETHACC_CODELET_MAX_PORT)];
    static char *stringPtr;
    static UINT32 busTimestampEnd = 0;
    static UINT32 busTimestampStart = 0;
    static UINT32 pTimeCycles = 0;
    static UINT64 rxCount = 0;
    static UINT64 txCount = 0;
    static UINT64 pTimeUsecs = 0;

    ixEthAccCodeletStatsPollTaskStop = FALSE;

    if (ixOsalMutexLock (&ixEthAccCodeletStatsPollTaskRunning,
			 IX_OSAL_WAIT_FOREVER) != IX_SUCCESS)
    {
        printf("CodeletMain: Error starting Stats thread! Failed to lock mutex.\n");
        return;
    }

    while (1)
    {
	while (ixEthAccCodeletTrafficPollEnabled == FALSE)
	{
	    /* Sleep 1 sec */
	    ixOsalSleep(1000);
	    if (ixEthAccCodeletStatsPollTaskStop)
	    {
		break;  /* Exit the thread */
	    }
	}

	if (ixEthAccCodeletStatsPollTaskStop)
	{
	    break;  /* Exit the thread */
	}
	printf("\n");
#ifdef __wince
	printf("\r");
#endif
	for(portIndex=0; portIndex< IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
	{
	    printf("Port%d Rates:        |",IX_ETHNPE_INDEX_TO_PORT_ID(portIndex));
	}
	printf("\n");
#ifdef __wince
	printf("\r");
#endif
	for(portIndex=0; portIndex< IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
	{
	    printf("=====================");
	}
	printf("=\n");
#ifdef __wince
	printf("\r");
#endif

	/* reset the stats */
	for(portIndex=0; portIndex< IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
	{
	    ixEthAccCodeletStats[IX_ETHNPE_INDEX_TO_PORT_ID(portIndex)].rxCount=0;
	    ixEthAccCodeletStats[IX_ETHNPE_INDEX_TO_PORT_ID(portIndex)].txCount=0;
	}

	while (ixEthAccCodeletTrafficPollEnabled)
	{
	    busTimestampStart = ixOsalTimestampGet();
	    /* Sleep approximatively 1 sec */
	    ixOsalSleep(1000);
       
	    /* check if the task should stop */ 
	    if (ixEthAccCodeletStatsPollTaskStop)
	    {
	        break;  /* Exit the thread */
	    }

	    if  (ixEthAccCodeletTrafficPollEnabled)
	    {
		/* \r : reset print curser to beginning of line */
		stringPtr = displayString;
		*stringPtr++ = '\r';

		for(portIndex=0; portIndex< IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
		{
		    portId = IX_ETHNPE_INDEX_TO_PORT_ID(portIndex);
		    /* get a snapshot */
		    busTimestampEnd = ixOsalTimestampGet();
		    rxCount = ixEthAccCodeletStats[portId].rxCount;
		    txCount = ixEthAccCodeletStats[portId].txCount;
		    /* Got stats, now clear counters */
		    ixEthAccCodeletStats[portId].rxCount=0;
		    ixEthAccCodeletStats[portId].txCount=0;

		    /* get the measurement interval using a unsigned
		     * subtraction in order to handle wrap-around. The time unit 
		     * is in APB bus cycles.
		     */
		    pTimeCycles = busTimestampEnd - busTimestampStart;
		    /* convert the time in APB bus cycles to microseconds
		     *
		     * multiplications are done before divisions and
		     * will not overflow the UINT64.
		     */
		    pTimeUsecs = (UINT64)pTimeCycles;

		
		    pTimeUsecs = IX_ETHACC_CODELET_MULDIV(pTimeUsecs,
			     IX_ETHACC_CODELET_PCLOCK_DIVIDER, 
			     IX_ETHACC_CODELET_XCLOCK_FREQ);
		    if (!pTimeUsecs) 
		    {
			/* time may be 0 as the result of the previous operation
			 * In this case (very unlikely to occur), 
			 * just skip the remaining of this loop.
			 * The display will not be updated before the next
			 * loop.
			 */
			continue;
		    }

		    /* convert from the packet count to a rate in pkts per second 
		     *
		     * rate = pkts * usecsPerSec / timeUsec
		     *
		     * multiplications are done before divisions and
		     * will not overflow the UINT64.
		     */
		    rxCount = IX_ETHACC_CODELET_MULDIV(rxCount, 
			        IX_ETHACC_CODELET_USEC_PER_SEC, pTimeUsecs);
		    txCount = IX_ETHACC_CODELET_MULDIV(txCount, 
			        IX_ETHACC_CODELET_USEC_PER_SEC, pTimeUsecs);

		    /* print stats */
		    stringPtr += sprintf(stringPtr, "Rx:%6u Tx:%6u |", 
					 (unsigned)rxCount, 
					 (unsigned)txCount);
		}
		*stringPtr++ = stillRunning[stillRunningIndex++ & 3];
		*stringPtr = 0;
		printf("%s", displayString);
	    }
	}
	printf("\n");
#ifdef __wince
	printf("\r");
#endif
    }

    ixOsalMutexUnlock (&ixEthAccCodeletStatsPollTaskRunning);
}


/**
 * @fn void ixEthAccCodeletLoop()
 *
 * This function is called once a operation has started. It spawns the
 * stats poll task and waits for use interrupt to end the codelet.
 * 
 * @return IX_SUCCESS - operation successfully run
 * @return IX_FAIl - error spawning stats poll task
 */

PRIVATE IX_STATUS ixEthAccCodeletLoop(void)
{
#ifdef __wince
    char str[16];
#elif defined (__vxworks)
    int ch = 0;
#elif defined (__linux)
#endif

    printf(IX_ETHACC_CODELET_USER_INSTRUCTION);

    ixEthAccCodeletTrafficPollEnabled = TRUE;

#ifdef __wince
    /* wait for the user to enter the sequence q<CR> */
    do
    {
        gets(str);
    }
    while(str[0] != 'q');
#elif defined (__linux)
    /* wait for the user to stop the codelet */
    ixEthAccCodelet_wait();

#elif defined (__vxworks)
    /* wait for the user to enter the sequence <ESC><CR> */
    do
    {
	ch = getchar();
    }
    while(ch != 27);
#else
#error "Unsupported platform"
#endif
    
    ixEthAccCodeletTrafficPollEnabled = FALSE;

    return (IX_SUCCESS);
}


#ifdef __wince

/**
 * @fn int wmain(int argc, WCHAR **argv)
 *
 * This function is the entryPoint for winCE menu
 * 
 * @return 0 (always)
 */
int wmain(int argc, WCHAR **argv)
{
    int ethAccCodeletTestNr;
    BOOL ethAccCodeletRun = TRUE;
    char line[256];
    IxEthAccPortId inPort;
    IxEthAccPortId outPort;

    while (ethAccCodeletRun)
    {
        printf("\n");
        printf("************* ethAcc Codelet *********************************\n");
        printf("  %d. RxOnly: All frames received (from external device)\n",
	       IX_ETHACC_CODELET_RX_SINK);
        printf("     will be sinked, for all available ports.\n");

        printf("  %d. Loopback: All frames received are software loopbacked\n",
	       IX_ETHACC_CODELET_SW_LOOPBACK);
        printf("     to the same port, for all available ports.\n");

        printf("  %d. TxGenRxSink: Frames generated and transmitted from outPort,\n",
	        IX_ETHACC_CODELET_TXGEN_RXSINK);
        printf("     and received on inPort.\n");

        printf("  %d. PhyLoopback: Frames generated and PHY loopbacked on the\n",
	       IX_ETHACC_CODELET_PHY_LOOPBACK);
        printf("     same port, for all available ports.\n");

        printf("  %d. Bridge: Frames received on one port will be transmitted\n",
	       IX_ETHACC_CODELET_BRIDGE);

        printf("  %d. QoS Bridge: VLAN-tagged Frames received on inPort will be\n",
	       IX_ETHACC_CODELET_BRIDGE_QOS);
        printf("     prioritized and untagged before being transmitted through\n");
	printf("     the outPort and vice-versa.\n");

        printf("  %d. Firewall bridge: Only the frames with one of the\n",
	       IX_ETHACC_CODELET_BRIDGE_FIREWALL );
        printf("     authorized source MAC address will be transmitted\n");
	printf("     through the other port.\n");

        printf("  %d. EthDB: Ethernet Learning Facility where it adds some static\n",
	       IX_ETHACC_CODELET_ETH_LEARNING);
        printf("     and dynamic entries. Dynamic entries are then aged and\n");
        printf("     verified that they no longer appear in the database.\n");

        printf("  %d. WiFi bridge: Frames received on one port get their WIFI\n",
	       IX_ETHACC_CODELET_BRIDGE_WIFI );
        printf("     header stripped. Trafic is forwarded to the other port.\n");

        printf("100. Exit ethAcc Codelet\n");
        printf("\n");
        printf("Enter test number: ");

	/* read the menu option */
	gets(line);
        ethAccCodeletTestNr = atoi(line);

	/* read the port numbers */
        printf("\n");
        printf("Enter inPort number: ");
	gets(line);
        inPort = atoi(line);
        printf("\n");
        printf("Enter outPort number: ");
	gets(line);
        outPort = atoi(line);

        if (ethAccCodeletTestNr == 100)
	{
	    /* exit the codelet */
	    ethAccCodeletRun = FALSE;
	}
	else if ((ethAccCodeletTestNr >= IX_ETHACC_CODELET_RX_SINK) 
		 && (ethAccCodeletTestNr <= IX_ETHACC_CODELET_BRIDGE_WIFI))
	{
	    /* run the codelet's option */
	    ixEthAccCodeletMain(ethAccCodeletTestNr,inPort,outPort);
	}
    }

    return 0;
}
#endif

