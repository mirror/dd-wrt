/**
 * @file IxEthAcc.c
 *
 * @author Intel Corporation
 * @date 20-Feb-2001
 *
 * @brief This file contains the implementation of the IXP400 Ethernet Access Component
 *
 * Design Notes:
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



#include "IxEthAcc.h"
#include "IxEthDB.h"
#include "IxFeatureCtrl.h"
#include "IxAccCommon.h"

#include "IxEthAcc_p.h"
#include "IxEthAccMac_p.h"
#include "IxEthAccMii_p.h"

/**
 * @addtogroup IxEthAcc
 *@{
 */


/**
 * @brief System-wide information data strucure.
 *
 * @ingroup IxEthAccPri
 *
 */

IxEthAccInfo ixEthAccDataInfo;
extern PUBLIC IxEthAccMacState ixEthAccMacState[];
extern PUBLIC IxOsalMutex ixEthAccControlInterfaceMutex;
extern UINT32 ixEthAccQMIntEnableBaseAddress;
extern UINT32 ixEthAccQMIntStatusBaseAddress;


/**
 * @brief System-wide information
 *
 * @ingroup IxEthAccPri
 *
 */
BOOL ixEthAccServiceInit = FALSE;

/* global filtering bit mask */
PUBLIC UINT32 ixEthAccNewSrcMask;

/**
 * @brief Per port information data strucure.
 *
 * @ingroup IxEthAccPri
 *
 */

IxEthAccPortDataInfo ixEthAccPortData[IX_ETHNPE_MAX_NUMBER_OF_PORTS];

PUBLIC IxEthAccStatus ixEthAccInit()
{

  if (ixEthAccServiceInit == TRUE)
  {
      IX_ETH_ACC_WARNING_LOG("ixEthAccInit: EthAcc was already initialized\n", 0, 0, 0, 0, 0, 0);
      
      return IX_ETH_ACC_SUCCESS;
  }

  /*
   * Init co-existence services
   */
  if (ixEthHssAccCoExistInit() != IX_ETH_ACC_SUCCESS)
  {
      IX_ETH_ACC_FATAL_LOG("ixEthAccInit: Co-exist features init failed\n", 0, 0, 0, 0, 0, 0);
      
      return IX_ETH_ACC_FAIL;
  }

   /* 
    * Initialize port mapping table
    */
   if (ixEthNpePortMapCreate() != IX_ETH_NPE_SUCCESS)
   {
       IX_ETH_ACC_FATAL_LOG("ixEthAccInit: failed to build port mapping lookup table\n", 0, 0, 0, 0, 0, 0);
      
       return IX_ETH_ACC_FAIL;
   }

  /*
   * Initialize Control plane
   */
  if (ixEthDBInit() != IX_ETH_ACC_SUCCESS)
  {
      IX_ETH_ACC_FATAL_LOG("ixEthAccInit: EthDB init failed\n", 0, 0, 0, 0, 0, 0);
      
      return IX_ETH_ACC_FAIL;
  }

  if (IX_FEATURE_CTRL_SWCONFIG_ENABLED == ixFeatureCtrlSwConfigurationCheck (IX_FEATURECTRL_ETH_LEARNING))
  {
      ixEthAccNewSrcMask = (~0); /* want all the bits */
  }
  else
  {
      ixEthAccNewSrcMask = (~IX_ETHACC_NE_NEWSRCMASK); /* want all but the NewSrc bit */
  }

  /*
   * Initialize Data plane
   */
   if ( ixEthAccInitDataPlane()  != IX_ETH_ACC_SUCCESS )
   {
      IX_ETH_ACC_FATAL_LOG("ixEthAccInit: data plane init failed\n", 0, 0, 0, 0, 0, 0);
      
       return IX_ETH_ACC_FAIL;
   }

   /*
    * Initialize MII 
    */
   if ( ixEthAccMiiInit() != IX_ETH_ACC_SUCCESS )
   {
      IX_ETH_ACC_FATAL_LOG("ixEthAccInit: Mii init failed\n", 0, 0, 0, 0, 0, 0);
      
       return IX_ETH_ACC_FAIL;
   }
   
   /*
    * Initialize MAC I/O memory
    */
   if (ixEthAccMacMemInit() != IX_ETH_ACC_SUCCESS)
   {
      IX_ETH_ACC_FATAL_LOG("ixEthAccInit: Mac init failed\n", 0, 0, 0, 0, 0, 0);
      
     return IX_ETH_ACC_FAIL;
   }

   /* 
    * Initialize control plane interface lock 
    */
   if (ixOsalMutexInit(&ixEthAccControlInterfaceMutex) != IX_SUCCESS)
   {
       IX_ETH_ACC_FATAL_LOG("ixEthAccInit: Control plane interface lock initialization failed\n", 0, 0, 0, 0, 0, 0);

       return IX_ETH_ACC_FAIL;
   }

   /*
    * Spawning mac recovery thread
    */
   if (ixEthAccMacRecoveryLoopStart() !=  IX_ETH_ACC_SUCCESS)
   {
       IX_ETH_ACC_FATAL_LOG("ixEthAccInit: Mac recovery thread failed to be spawned\n", 0, 0, 0, 0, 0, 0);

       return IX_ETH_ACC_FAIL;
   }

   /* map the qmgr interrupt registers used for rx queue interrupt disable/enable */
   ixEthAccQMIntEnableBaseAddress = 
       (UINT32)IX_OSAL_MEM_MAP(IX_ETH_ACC_QMGR_PHY_BASE_ADDRESS + IX_ETH_ACC_QMGR_LOWQ_INT_ENABLE_REG_OFFSET,sizeof(UINT32));
   ixEthAccQMIntStatusBaseAddress = 
       (UINT32)IX_OSAL_MEM_MAP(IX_ETH_ACC_QMGR_PHY_BASE_ADDRESS + IX_ETH_ACC_QMGR_LOWQ_INT_STATUS_REG_OFFSET,sizeof(UINT32));

   /* initialiasation is complete */
   ixEthAccServiceInit = TRUE;
   
   return IX_ETH_ACC_SUCCESS;

}



/*Uninitialize the ethAcc Service */
PUBLIC IxEthAccStatus
ixEthAccUninit (void)
{
    UINT32 portIndex;
    IxEthAccPortId portId;
    IxEthAccStatus Status = IX_ETH_ACC_SUCCESS;

    if ( IX_ETH_ACC_IS_SERVICE_INITIALIZED() ) 
    {
       /* check none of the port is still active */
       for (portIndex = 0; portIndex < IxEthAccPortInfo->IxEthAccNumberOfPorts; portIndex++)
       {
	   portId = IX_ETHNPE_INDEX_TO_PORT_ID(portIndex);
           if ( IX_ETH_IS_PORT_INITIALIZED(portId))
           {
	       if (ixEthAccMacState[portId].portDisableState == ACTIVE)
	       {
			IX_ETH_ACC_WARNING_LOG("ixEthAccUnload: port %u still active, bail out\n", portId, 0, 0, 0, 0, 0);
			return IX_ETH_ACC_FAIL;
	       }
               /* Unconfigure the queues configured by swConfigSet in QswConfig */
               if (IX_ETH_ACC_SUCCESS != ixEthAccQueuesUnconfig ())
               {
                   IX_ETH_ACC_FATAL_LOG ("Failed to unconfig queues", 0, 0, 0, 0, 0, 0);
                   return IX_ETH_ACC_FAIL;
               }
               /*Unload the Eth access mac and mii components, previously ixEthAccUnload ()*/
               if (IX_ETH_ACC_SUCCESS != ixEthAccMacUninit(portId))
               {
                   IX_ETH_ACC_WARNING_LOG("ixEthAccUnload: EthAccMacUninit Failed ", portId, 0, 0, 0, 0, 0);
               }

           }
       }

       ixOsalMutexDestroy(&ixEthAccControlInterfaceMutex);

       /*
	* Stopping mac recovery thread
	*/
       if (ixEthAccMacRecoveryLoopStop() !=  IX_ETH_ACC_SUCCESS)
       {
	   IX_ETH_ACC_WARNING_LOG("ixEthAccUnload: Mac recovery thread failed to be stoped\n", 0, 0, 0, 0, 0, 0);
	   return IX_ETH_ACC_FAIL;
       }

       /* unmap the memory areas */
       ixEthAccMiiUnload();
       ixEthAccMacUnload();

       IX_OSAL_MEM_UNMAP(ixEthAccQMIntEnableBaseAddress);
       IX_OSAL_MEM_UNMAP(ixEthAccQMIntStatusBaseAddress);

       /* set all ports as uninitialized */
       for (portIndex = 0; portIndex < IxEthAccPortInfo->IxEthAccNumberOfPorts; portIndex++)
       {
          ixEthAccPortData[IX_ETHNPE_INDEX_TO_PORT_ID(portIndex)].portInitialized = FALSE;
       }
	
       /*Unload the database*/
       if (IX_ETH_DB_SUCCESS != ixEthDBUnload ())
       {
           IX_ETH_ACC_FATAL_LOG ("Port busy Can't Unload", 0, 0, 0, 0, 0, 0);
           return IX_ETH_ACC_FAIL;
       }
   
       /* Check co-existence services can be uninitialized */
       if (IX_ETH_ACC_SUCCESS != ixEthHssAccCoExistUninit())
       {
           IX_ETH_ACC_FATAL_LOG ("Can't Unload co-exists features", 0, 0, 0, 0, 0, 0);
           return IX_ETH_ACC_FAIL;
       }

       /* uninitialize the service */
       ixEthAccServiceInit = FALSE;
    }

    return Status;
}



PUBLIC IxEthAccStatus ixEthAccPortInit(IxEthAccPortId portId)
{
   UINT32 portIndex = 0;
   UINT32 npePort;

   if ( ! IX_ETH_ACC_IS_SERVICE_INITIALIZED() ) 
   {
       IX_ETH_ACC_WARNING_LOG("EthAcc: Service Not Initialized. Cannot initialize Eth port %d.\n",(INT32) portId,0,0,0,0,0);
   		
	   return(IX_ETH_ACC_FAIL);
   }

   /*
    * Check for valid port
    */
    
   if ( ! IX_ETH_ACC_IS_PORT_VALID(portId) )
   {
       IX_ETH_ACC_WARNING_LOG("EthAcc: Eth port %d invalid.\n",(INT32) portId,0,0,0,0,0);

	   return (IX_ETH_ACC_INVALID_PORT);
   }   

   if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
   {
       /* no error message printed because some configurations expect this to fail */
       return IX_ETH_ACC_INVALID_PORT;
   }   

   if ( IX_ETH_IS_PORT_INITIALIZED(portId) )
   {
   	/* Already initialized */
       IX_ETH_ACC_WARNING_LOG("EthAcc: Eth port %d already initialized.\n",(INT32) portId,0,0,0,0,0);

   	return(IX_ETH_ACC_FAIL);
   }

   /* Update the npe count if this port is from an NPE that hasn't already been initialized */
   /* Note: npeCount must be updated before configuring this port's queues */
   for (npePort = 0; npePort < IxEthAccPortInfo->port[portIndex].IxEthNpeNumberOfPortPerNpe; npePort++)
   {
       if (IX_ETH_IS_PORT_INITIALIZED(
              IX_ETHNPE_NODE_AND_PORT_TO_PHYSICAL_ID(IX_ETHNPE_PHYSICAL_ID_TO_NODE(portId), npePort))
          )
           break;
   }
   if(npePort == IxEthAccPortInfo->port[portIndex].IxEthNpeNumberOfPortPerNpe)
   {
       ixEthAccDataInfo.npeCount++;
   }

   if ( ixEthAccQMgrQueuesConfig(portId) != IX_ETH_ACC_SUCCESS )
   {
      IX_ETH_ACC_FATAL_LOG("ixEthAcc: queue config failed for port %d\n", portId, 0, 0, 0, 0, 0);
	  
      return IX_ETH_ACC_FAIL;
   }

   if(ixEthAccMacInit(portId) != IX_ETH_ACC_SUCCESS)
   {
      IX_ETH_ACC_WARNING_LOG("ixEthAcc: MAC init failed for port %d\n", portId, 0, 0, 0, 0, 0);

	  return IX_ETH_ACC_FAIL;
   }

   /*
     * Set the port init flag.
     */

    ixEthAccPortData[portId].portInitialized = TRUE;
    
    /* init learning/filtering database structures for this port */
    ixEthDBPortInit(portId);

    return(IX_ETH_ACC_SUCCESS);    
}


