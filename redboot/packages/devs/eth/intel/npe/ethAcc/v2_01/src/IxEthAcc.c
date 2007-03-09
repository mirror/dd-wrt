/**
 * @file IxEthAcc.c
 *
 * @author Intel Corporation
 * @date 20-Feb-2001
 *
 * @brief This file contains the implementation of the IXP425 Ethernet Access Component
 *
 * Design Notes:
 *
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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


   if ( ixEthAccQMgrQueuesConfig() != IX_ETH_ACC_SUCCESS )
   {
      IX_ETH_ACC_WARNING_LOG("ixEthAccInit: queue config failed\n", 0, 0, 0, 0, 0, 0);
      
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
   
   /* initialiasation is complete */
   ixEthAccServiceInit = TRUE;
   
   return IX_ETH_ACC_SUCCESS;

}

PUBLIC void ixEthAccUnload(void)
{
    IxEthAccPortId portId;

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
		   return;
	       }
	   }
       }

       /* unmap the memory areas */
       ixEthAccMiiUnload();
       ixEthAccMacUnload();

       /* set all ports as uninitialized */
       for (portId = 0; portId < IX_ETH_ACC_NUMBER_OF_PORTS; portId++)
       {
	       ixEthAccPortData[portId].portInitialized = FALSE;
       }
	
       /* uninitialize the service */
       ixEthAccServiceInit = FALSE;
   }
}

PUBLIC IxEthAccStatus ixEthAccPortInit( IxEthAccPortId portId)
{

  IxEthAccStatus ret=IX_ETH_ACC_SUCCESS;

   if ( ! IX_ETH_ACC_IS_SERVICE_INITIALIZED() ) 
   {
   	return(IX_ETH_ACC_FAIL);
   }

   /*
    * Check for valid port
    */
    
   if ( ! IX_ETH_ACC_IS_PORT_VALID(portId) )
   {
       return (IX_ETH_ACC_INVALID_PORT);
   }   

   if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
   {
       IX_ETH_ACC_WARNING_LOG("EthAcc: Unavailable Eth %d: Cannot initialize Eth port.\n",(INT32) portId,0,0,0,0,0);
       return IX_ETH_ACC_SUCCESS ;
   }   

   if ( IX_ETH_IS_PORT_INITIALIZED(portId) )
   {
   	/* Already initialized */
   	return(IX_ETH_ACC_FAIL);
   }

   if(ixEthAccMacInit(portId)!=IX_ETH_ACC_SUCCESS)
   {
       return IX_ETH_ACC_FAIL;
   }

   /*
     * Set the port init flag.
     */

    ixEthAccPortData[portId].portInitialized = TRUE;
    
    /* init learning/filtering database structures for this port */
    ixEthDBPortInit(portId);

    return(ret);    
}


