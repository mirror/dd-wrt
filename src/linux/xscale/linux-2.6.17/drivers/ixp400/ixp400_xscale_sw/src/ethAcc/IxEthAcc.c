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



#include "IxEthAcc.h"
#include "IxEthDB.h"
#include "IxFeatureCtrl.h"

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

IxEthAccPortDataInfo ixEthAccPortData[IX_ETH_ACC_NUMBER_OF_PORTS];

PUBLIC IxEthAccStatus ixEthAccInit()
{

  if (ixEthAccServiceInit == TRUE)
  {
      IX_ETH_ACC_WARNING_LOG("ixEthAccInit: EthAcc was already initialized\n", 0, 0, 0, 0, 0, 0);
      
      return IX_ETH_ACC_SUCCESS;
  }

  /*
   * Initialize Control plane
   */
  if (ixEthDBInit() != IX_ETH_ACC_SUCCESS)
  {
      IX_ETH_ACC_WARNING_LOG("ixEthAccInit: EthDB init failed\n", 0, 0, 0, 0, 0, 0);
      
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
      IX_ETH_ACC_WARNING_LOG("ixEthAccInit: data plane init failed\n", 0, 0, 0, 0, 0, 0);
      
       return IX_ETH_ACC_FAIL;
   }

   /*
    * Initialize MII 
    */
   if ( ixEthAccMiiInit() != IX_ETH_ACC_SUCCESS )
   {
      IX_ETH_ACC_WARNING_LOG("ixEthAccInit: Mii init failed\n", 0, 0, 0, 0, 0, 0);
      
       return IX_ETH_ACC_FAIL;
   }
   
   /*
    * Initialize MAC I/O memory
    */
   if (ixEthAccMacMemInit() != IX_ETH_ACC_SUCCESS)
   {
      IX_ETH_ACC_WARNING_LOG("ixEthAccInit: Mac init failed\n", 0, 0, 0, 0, 0, 0);
      
     return IX_ETH_ACC_FAIL;
   }

   /* 
    * Initialize control plane interface lock 
    */
   if (ixOsalMutexInit(&ixEthAccControlInterfaceMutex) != IX_SUCCESS)
   {
       IX_ETH_ACC_WARNING_LOG("ixEthAccInit: Control plane interface lock initialization failed\n", 0, 0, 0, 0, 0, 0);

       return IX_ETH_ACC_FAIL;
   }

   /* map the qmgr interrupt registers used for rx queue interrupt disable/enable */
   ixEthAccQMIntEnableBaseAddress = 
       (UINT32)IX_OSAL_MEM_MAP(IX_OSAL_IXP400_QMGR_PHYS_BASE + IX_ETH_ACC_QMGR_LOWQ_INT_ENABLE_REG_OFFSET,sizeof(UINT32));
   ixEthAccQMIntStatusBaseAddress = 
       (UINT32)IX_OSAL_MEM_MAP(IX_OSAL_IXP400_QMGR_PHYS_BASE + IX_ETH_ACC_QMGR_LOWQ_INT_STATUS_REG_OFFSET,sizeof(UINT32));

   /* initialiasation is complete */
   ixEthAccServiceInit = TRUE;
   
   return IX_ETH_ACC_SUCCESS;

}



/*Uninitialize the ethAcc Service */
PUBLIC IxEthAccStatus
ixEthAccUninit (void)
{
    IxEthAccPortId portId;
    IxEthAccStatus Status = IX_ETH_ACC_SUCCESS;
    if ( IX_ETH_ACC_IS_SERVICE_INITIALIZED() ) 
    {
       /* check none of the port is still active */
       for (portId = 0; portId < IX_ETH_ACC_NUMBER_OF_PORTS; portId++)
       {
           if ( IX_ETH_IS_PORT_INITIALIZED(portId) )
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

       /* unmap the memory areas */
       ixEthAccMiiUnload();
       ixEthAccMacUnload();

       IX_OSAL_MEM_UNMAP(ixEthAccQMIntEnableBaseAddress);
       IX_OSAL_MEM_UNMAP(ixEthAccQMIntStatusBaseAddress);

       /* set all ports as uninitialized */
       for (portId = 0; portId < IX_ETH_ACC_NUMBER_OF_PORTS; portId++)
       {
          ixEthAccPortData[portId].portInitialized = FALSE;
       }
	
       /*Unload the database*/
       if (IX_ETH_DB_SUCCESS != ixEthDBUnload ())
       {
           IX_ETH_ACC_FATAL_LOG ("Port busy Can't Unload", 0, 0, 0, 0, 0, 0);
           return IX_ETH_ACC_FAIL;
       }
   
       /* uninitialize the service */
       ixEthAccServiceInit = FALSE;
    }
    return Status;
}



PUBLIC IxEthAccStatus ixEthAccPortInit( IxEthAccPortId portId)
{
   IxEthNpePortId npePort = 0;
   
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
   for (npePort = 0 ; npePort < IX_ETHNPE_NUM_PORTS_PER_NPE ; npePort++)
   {
       if (IX_ETH_IS_PORT_INITIALIZED(
              IX_ETHNPE_NODE_AND_PORT_TO_PHYSICAL_ID(IX_ETHNPE_PHYSICAL_ID_TO_NODE(portId), npePort))
          )
           break;
   }
   if(npePort == IX_ETHNPE_NUM_PORTS_PER_NPE)
   {
       ixEthAccDataInfo.npeCount++;
   }

   if ( ixEthAccQMgrQueuesConfig(portId) != IX_ETH_ACC_SUCCESS )
   {
      IX_ETH_ACC_WARNING_LOG("ixEthAcc: queue config failed for port %d\n", portId, 0, 0, 0, 0, 0);
	  
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


