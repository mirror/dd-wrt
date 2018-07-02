/**
 * @file IxEthAccCodeletLinkSetup.c
 *
 * @date 22 April 2004
 *
 * @brief This file contains the implementation of the Ethernet Access 
 * Codelet that implements the link layer configuration and control
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
 * Put the component defined include files required.
 */
#include "IxFeatureCtrl.h"
#include "IxEthAcc.h"
#include "IxEthMii.h"
#include "IxEthAccCodelet.h"
#include "IxEthAccCodelet_p.h"

/*
 * Variable declarations global to this file only.
 */
PRIVATE IxOsalMutex ixEthAccCodeletPhyUpdateTaskRunning;
PRIVATE volatile BOOL ixEthAccCodeletPhyUpdateTaskStopTrigger;

/*
 * Function prototype
 */
PRIVATE void linkStatusMonitor (void *arg, void **ptrRetObj);

/* The Eth PHY Configuration (100/10 Mbps , Full/Half Duplex, 
 * with/without autonegotiation).
 */
typedef struct
{
    BOOL speed100;	/**< 100 Mbits */
    BOOL fullDuplex;	/**< Full Duplex */
    BOOL autonegotiate;	/**< Autonegotiation */
    BOOL syncPhyMac;    /**< Synchronize MAC duplex mode with PHY status */
} IxEthAccCodeletPhyConf;

/* Default PHY Configuration parameters */
PRIVATE IxEthAccCodeletPhyConf ixEthAccCodeletPhyConf = 
{
    TRUE,	/* 100 Mbits */
    TRUE,	/* Full duplex */
    TRUE,	/* Autonegotiate */
#ifdef __kixrp435
    FALSE	/* MAC duplex mode will not be updated according to current PHY status */
#else
    TRUE 	/* MAC duplex mode will be updated according to current PHY status */    
#endif
};

/*
 * Function definition: ixEthAccCodeletLinkUpCheck()
 *
 * Check the phys are ready
 */

IX_STATUS ixEthAccCodeletLinkUpCheck(IxEthAccPortId portId)
{
   IX_STATUS returnStatus = IX_SUCCESS;
   BOOL fullDuplex;
   BOOL linkUp;
   BOOL speed;
   BOOL autoneg;

   /* get the status */
   ixEthMiiLinkStatus(IxEthEthPortIdToPhyAddressTable[portId], 
		      &linkUp, 
		      &speed, 
		      &fullDuplex, 
		      &autoneg);

   if (!linkUp)
   {
          unsigned int retry = 20; /* 20 retries */
          printf("Wait for PHY %u to be ready ...\n", IxEthEthPortIdToPhyAddressTable[portId]);
          while ((!linkUp) && (retry-- > 0))
          {
            ixOsalSleep(100);  /* 100 milliseconds */

	    /* get the status again */
            ixEthMiiLinkStatus(IxEthEthPortIdToPhyAddressTable[portId], 
			       &linkUp, 
			       &speed, 
			       &fullDuplex, 
			       &autoneg);
          }
          if (!linkUp) 
          {
            returnStatus = IX_FAIL;
          }
   }

   /* return fail if one of the links is not up */
   return returnStatus;
}


/**
 * Function definition: ixEthAccCodeletLinkMonitor()
 *
 * Function checks the link status and sets the duplex mode if there
 * is a change in it
 */

PRIVATE void
ixEthAccCodeletLinkMonitor(IxEthAccPortId portId)
{
    BOOL speed, linkUp, fullDuplex, autoneg;
    /* get the link status */
    if(ixEthAccCodeletPhyConf.syncPhyMac)
    {
      ixEthMiiLinkStatus(IxEthEthPortIdToPhyAddressTable[portId], 
		         &linkUp,
		         &speed,
		         &fullDuplex,
		         &autoneg);
      /* Set the MAC duplex mode */
      if(fullDuplex)
      {
	  ixEthAccPortDuplexModeSet (portId, IX_ETH_ACC_FULL_DUPLEX);
      }
      else
      {
	  ixEthAccPortDuplexModeSet (portId, IX_ETH_ACC_HALF_DUPLEX);
      }
    }
}

void
linkStatusMonitor (void *arg, void **ptrRetObj)
{
    UINT32 portIndex;

    ixEthAccCodeletPhyUpdateTaskStopTrigger = FALSE;
    if (ixOsalMutexLock (&ixEthAccCodeletPhyUpdateTaskRunning,
			 IX_OSAL_WAIT_FOREVER) != IX_SUCCESS)
    {
        printf("CodeletMain: Error starting Phy update thread! Failed to lock mutex.\n");
        return;
    }

	/*
	 * Start the infinite loop
	 */
    while (1)
    {
        if(ixEthAccCodeletPhyUpdateTaskStopTrigger)
        {
		break;
	}
        for (portIndex = 0; portIndex < IX_ETHACC_NUMBER_OF_PORTS; portIndex++)
	{
            ixOsalSleep (5000/IX_ETHACC_NUMBER_OF_PORTS);
            ixEthAccCodeletLinkMonitor(IX_ETHNPE_INDEX_TO_PORT_ID(portIndex));
	}
    }

    ixOsalMutexUnlock (&ixEthAccCodeletPhyUpdateTaskRunning);  
}

/**
 * Function definition: ixEthAccCodeletPhyInit()
 *
 * This function scans for and then initialises any available PHYs on
 * the board. It uses the EthAcc MII library routines to do so.
 * 
 */

IX_STATUS ixEthAccCodeletPhyInit(void)
{
   BOOL phyPresent[IXP400_ETH_ACC_MII_MAX_ADDR];
   UINT32 phyNo;
   UINT32 phyNoAddr;
   UINT32 ixEthAccCodeletMaxPhyNo;
   unsigned int portId;
   IxOsalThread phyStatusThread;
   IxOsalThreadAttr threadAttr;

   threadAttr.name      = "Codelet Phy Status";
   threadAttr.stackSize = 32 * 1024; /* 32kbytes */
   threadAttr.priority  = 128;


   /* Scan for available Ethernet PHYs on the board */
   if(ixEthMiiPhyScan(phyPresent,IXP400_ETH_ACC_MII_MAX_ADDR) == IX_FAIL)
   {	
       return (IX_FAIL);
   }
   else
   {
       /* 
	* Mapping from portId to Phy Address is done here. 
	* This is board specific and depends on PHY address
	* setup (generally done by EEPROM or Jumpers)
	*
	* This codelet assumes the PHY with the lowest
	* address is connected to the NPE B and the PHY with the
	* highest address is connected to NPE C.
	*/
       ixEthAccCodeletMaxPhyNo = 0;
       for(phyNoAddr=0, phyNo=IX_ETHNPE_INDEX_TO_PORT_ID(0);
	   (phyNoAddr<IXP400_ETH_ACC_MII_MAX_ADDR) && (phyNo < IX_ETHNPE_MAX_NUMBER_OF_PORTS) ; 
	   phyNoAddr++)
       {
	   if(phyPresent[phyNoAddr])
	   {	   
	       IxEthEthPortIdToPhyAddressTable[phyNo++] = phyNoAddr;
               ixEthAccCodeletMaxPhyNo = phyNo;

	       if(ixEthAccCodeletMaxPhyNo == IX_ETHNPE_MAX_NUMBER_OF_PORTS)
	       {
		   break;
	       }
	   }
       }
   }
#ifdef __kixrp435
   /* For KIXRP43X, hardcode Phy Addr 1 for NPE-C and Phy Addr 5 for NPE-A */     
   IxEthEthPortIdToPhyAddressTable[IX_ETH_PORT_2] = 1;
   IxEthEthPortIdToPhyAddressTable[IX_ETH_PORT_3] = 5;
   ixEthAccCodeletMaxPhyNo = 3;
   ixEthAccPortDuplexModeSet(IX_ETH_PORT_2,IX_ETH_ACC_FULL_DUPLEX);
   ixEthAccPortDuplexModeSet(IX_ETH_PORT_3,IX_ETH_ACC_FULL_DUPLEX);
#endif    

   for(phyNo=IX_ETHNPE_INDEX_TO_PORT_ID(0); 
       phyNo<ixEthAccCodeletMaxPhyNo; 
       phyNo++)
   {
       portId = phyNo;
       /* Reset each phy */

       if (ixFeatureCtrlDeviceRead() == IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X)
       {
	   /*Only when Ethernet is available, then add dynamic entries */
           if (((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) == 
                 IX_FEATURE_CTRL_COMPONENT_ENABLED) && (0 == portId)) ||
               ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) == 
                 IX_FEATURE_CTRL_COMPONENT_ENABLED) && (1 == portId)))
	   {
               ixEthMiiPhyReset(IxEthEthPortIdToPhyAddressTable[portId]);

	       /* Set each phy properties */
	       ixEthMiiPhyConfig(IxEthEthPortIdToPhyAddressTable[portId],
	        		  ixEthAccCodeletPhyConf.speed100, 
				  ixEthAccCodeletPhyConf.fullDuplex, 
				  ixEthAccCodeletPhyConf.autonegotiate);
	       /* wait until the link is up before setting the MAC duplex
	        * mode, the PHY duplex mode may change after autonegotiation 
	        */
	       (void)ixEthAccCodeletLinkUpCheck(portId);
	       (void)ixEthAccCodeletLinkMonitor(portId);
	       
	       printf("\nPHY %d configuration:\n", IxEthEthPortIdToPhyAddressTable[portId]);
	       ixEthMiiPhyShow(IxEthEthPortIdToPhyAddressTable[portId]);
           }

       }
       else if ((ixFeatureCtrlDeviceRead() == IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X) ||
		 (ixFeatureCtrlDeviceRead() == IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X) )
       {
           ixEthMiiPhyReset(IxEthEthPortIdToPhyAddressTable[portId]);

           /* Set each phy properties */
           ixEthMiiPhyConfig(IxEthEthPortIdToPhyAddressTable[portId],
                             ixEthAccCodeletPhyConf.speed100,
                             ixEthAccCodeletPhyConf.fullDuplex,
                             ixEthAccCodeletPhyConf.autonegotiate);

	   /* wait until the link is up before setting the MAC duplex
	    * mode, the PHY duplex mode may change after autonegotiation 
	    */
	   (void)ixEthAccCodeletLinkUpCheck(portId);
	   (void)ixEthAccCodeletLinkMonitor(portId);
	   
	   printf("\nPHY %d configuration:\n",IxEthEthPortIdToPhyAddressTable[portId]);
	   ixEthMiiPhyShow(IxEthEthPortIdToPhyAddressTable[portId]);
       }
   }

   if(ixEthAccCodeletPhyConf.syncPhyMac)
   {
      ixOsalMutexInit (&ixEthAccCodeletPhyUpdateTaskRunning);
      if (ixOsalThreadCreate(&phyStatusThread,
		             &threadAttr,
		             (IxOsalVoidFnVoidPtr) linkStatusMonitor,
		            NULL) != IX_SUCCESS)
      {
	  printf("CodeletMain: Error spawning stats task\n");
	  return (IX_FAIL);
      }
      /* Start the thread */
      if (ixOsalThreadStart(&phyStatusThread) != IX_SUCCESS)
      {
	  printf("CodeletMain: Error failed to start the Phy status update thread\n");
          return IX_FAIL;
      }  
   }
   return (IX_SUCCESS);
}

IX_STATUS ixEthAccCodeletPhyUninit(void)
{
   if(ixEthAccCodeletPhyConf.syncPhyMac)
   {
      if (!ixEthAccCodeletPhyUpdateTaskStopTrigger)
      {
	  ixEthAccCodeletPhyUpdateTaskStopTrigger = TRUE;
	  if (ixOsalMutexLock (&ixEthAccCodeletPhyUpdateTaskRunning, 
			       IX_OSAL_WAIT_FOREVER)
	      != IX_SUCCESS)
	  {
	      printf("CodeletMain: Error stopping Phy Update thread!\n");
	      return (IX_FAIL);
	  }
	  ixOsalMutexUnlock (&ixEthAccCodeletPhyUpdateTaskRunning);
	  ixOsalMutexDestroy(&ixEthAccCodeletPhyUpdateTaskRunning);
      }
   }
   return (IX_SUCCESS);
}

/**
 * Function definition: ixEthAccCodeletLinkLoopbackEnable()
 *
 * This function sets the PHY in test loopback mode
 * 
 */

IX_STATUS ixEthAccCodeletLinkLoopbackEnable(IxEthAccPortId portId)
{
    /* force the PHY setup to 100 Mb full Duplex */
    ixEthMiiPhyConfig(IxEthEthPortIdToPhyAddressTable[portId],
		      TRUE, 
		      TRUE, 
		      FALSE);

    /* Enable PHY Loopback */
    ixEthMiiPhyLoopbackEnable(IxEthEthPortIdToPhyAddressTable[portId]);

    /* Get the link status. This is only used to display the current
     * state of the link on the console
     */
    (void)ixEthAccCodeletLinkUpCheck(portId);
    (void)ixEthAccCodeletLinkMonitor(portId);

    return (IX_SUCCESS);
}

/**
 * Function definition: ixEthAccCodeletLinkLoopbackDisable()
 *
 * This function turns off the PHY loopback mode
 * 
 */

IX_STATUS ixEthAccCodeletLinkLoopbackDisable(IxEthAccPortId portId)
{
    /* Disable PHY Loopback */
    ixEthMiiPhyLoopbackDisable(IxEthEthPortIdToPhyAddressTable[portId]);

    /* reset the PHY */
    ixEthMiiPhyReset(IxEthEthPortIdToPhyAddressTable[portId]);

    /* Set the phy properties */
    ixEthMiiPhyConfig(IxEthEthPortIdToPhyAddressTable[portId],
		      ixEthAccCodeletPhyConf.speed100, 
		      ixEthAccCodeletPhyConf.fullDuplex, 
		      ixEthAccCodeletPhyConf.autonegotiate);

    /* get the link status */
    (void)ixEthAccCodeletLinkUpCheck(portId);
    (void)ixEthAccCodeletLinkMonitor(portId);

    printf("\nPHY %d configuration:\n", IxEthEthPortIdToPhyAddressTable[portId]);
    ixEthMiiPhyShow(IxEthEthPortIdToPhyAddressTable[portId]);

    return (IX_SUCCESS);
}

/**
 * Function definition: ixEthAccCodeletPortSlowSpeedSet()
 *
 * This function sets the port to 10 Mbit with no negotiaton
 *
 */

IX_STATUS ixEthAccCodeletLinkSlowSpeedSet(IxEthAccPortId portId)
{
   BOOL speed, linkUp, fullDuplex, autoneg;
    /* get current duplex mode  */
   ixEthMiiLinkStatus(IxEthEthPortIdToPhyAddressTable[portId], 
		       &linkUp, 
		       &speed, 
		       &fullDuplex, 
		       &autoneg);
    /* set 10 Mbit, current duplex mode, no negotiation */
    ixEthMiiPhyConfig(IxEthEthPortIdToPhyAddressTable[portId],
		      FALSE, 
		      fullDuplex, 
		      FALSE);
    ixEthMiiPhyReset(IxEthEthPortIdToPhyAddressTable[portId]);

    /* get the link status */
    (void)ixEthAccCodeletLinkUpCheck(portId);
    (void)ixEthAccCodeletLinkMonitor(portId);

    printf("\nPHY %d configuration:\n", IxEthEthPortIdToPhyAddressTable[portId]);
    ixEthMiiPhyShow(IxEthEthPortIdToPhyAddressTable[portId]);

    return (IX_SUCCESS);
}

/**
 * Function definition: ixEthAccCodeletPortDefaultSpeedSet()
 *
 * This function restores the port default rate and negotiation mode.
 * 
 */

IX_STATUS ixEthAccCodeletLinkDefaultSpeedSet(IxEthAccPortId portId)
{
    /* set default values */
    ixEthMiiPhyConfig(IxEthEthPortIdToPhyAddressTable[portId],
		      ixEthAccCodeletPhyConf.speed100, 
		      ixEthAccCodeletPhyConf.fullDuplex, 
		      ixEthAccCodeletPhyConf.autonegotiate);

    ixEthMiiPhyReset(IxEthEthPortIdToPhyAddressTable[portId]);

    /* get the link status */
    (void)ixEthAccCodeletLinkUpCheck(portId);
    (void)ixEthAccCodeletLinkMonitor(portId);

    return (IX_SUCCESS);
}
