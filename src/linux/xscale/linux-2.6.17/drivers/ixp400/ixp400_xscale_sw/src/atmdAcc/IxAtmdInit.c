/**
* @file IxAtmdInit.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief IxAtmdAcc initialisation functions
*
* This file contains the initialisation function, the stats display
* and stats reset entry points
*
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
* Put the user defined include files required.
*/

#include "IxOsal.h"

#include "IxAtmdAccCtrl.h"

#include "IxAtmdDefines_p.h"
#include "IxAtmdUtil_p.h"
#include "IxAtmdRxCfgIf_p.h"
#include "IxAtmdTxCfgIf_p.h"
#include "IxAtmdPortMgmt_p.h"
#include "IxAtmdDescMgmt_p.h"
#include "IxAtmdAssert_p.h"
#include "IxFeatureCtrl.h"

static BOOL initDone = FALSE;

/*------------------------------------------------------------------------
* Initialize all modules
*/


PUBLIC IX_STATUS
ixAtmdAccInit (void)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* test if initialialisation already ran */
    if (initDone)
    {
        /* protect against multiple initialisations */
        returnStatus = IX_FAIL;
    }

    if (returnStatus == IX_SUCCESS)
    {
        /* utilities */
        returnStatus = ixAtmdAccUtilInit ();
    }
    if (returnStatus == IX_SUCCESS)
    {
        /* QMgr queues */
        returnStatus = ixAtmdAccUtilQueuesInit();
    }
    if (returnStatus == IX_SUCCESS)
    {
        /* descriptor management */
        returnStatus = ixAtmdAccDescMgmtInit ();
    }
    if (returnStatus == IX_SUCCESS)
    {
        /* port management */
        returnStatus = ixAtmdAccPortMgmtInit ();
    }
    if (returnStatus == IX_SUCCESS)
    {
        /* Rx services */
        returnStatus = ixAtmdAccRxCfgIfInit ();
    }
    if (returnStatus == IX_SUCCESS)
    {
        /* Tx services */
        returnStatus = ixAtmdAccTxCfgIfInit ();
    }

    if (returnStatus == IX_SUCCESS)
    {
        initDone = TRUE;
    }

    if (returnStatus == IX_SUCCESS)
    {
      /* Do not perform feature checkings only if
       * IXP42X - A0 silicon. 
       */
      if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 != 
        (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
        || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
      {
	if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_UTOPIA)== 
	    IX_FEATURE_CTRL_COMPONENT_DISABLED)
	{
	    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
		       "Warning: the UTOPIA component you specified does"
		       " not exist\n", 0,0,0,0,0,0);
	}
	
	if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_AAL)== 
	    IX_FEATURE_CTRL_COMPONENT_DISABLED)
        {
	    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
		     "Warning: the AAL component you specified does"
		     " not exist\n", 0,0,0,0,0,0);
        }
      }
    } 

    return returnStatus;
}



/*------------------------------------------------------------------------
* Uninitialize all modules
*/
PUBLIC IX_STATUS
ixAtmdAccUninit (void)
{
    IX_STATUS returnStatus = IX_SUCCESS;

    /* test if initialisation already ran */
    if (!initDone)
    {
        /* Only if un-initialisation already done */
        return IX_SUCCESS;
    }

    /* Tx services uninit*/
    returnStatus = ixAtmdAccTxCfgIfUninit ();
 
    if (IX_SUCCESS == returnStatus)
    {
        /* Rx services uninit*/
        returnStatus = ixAtmdAccRxCfgIfUninit ();
    }
    

    if (IX_SUCCESS == returnStatus)
    {
        /* port management uninit*/
        returnStatus = ixAtmdAccPortMgmtUninit ();
    }
    
 
    if (IX_SUCCESS == returnStatus)
    {
        /* descriptor management uninit*/
        returnStatus = ixAtmdAccDescMgmtUninit ();
    }
 
    
    if (IX_SUCCESS == returnStatus)
    {
        /* QMgr queues uninit*/
        returnStatus = ixAtmdAccUtilQueuesUninit();
    }

    
    if (IX_SUCCESS == returnStatus)
    {
        /* utilities uninit*/
        returnStatus = ixAtmdAccUtilUninit ();
    }


    if (IX_SUCCESS == returnStatus)
    {
        initDone = FALSE;
    }

   return returnStatus;
}




/*------------------------------------------------------------------------
* display friendly configuration
*/
PUBLIC void
ixAtmdAccShow (void)
{
    unsigned int port;

    if (initDone)
    {
        for (port = 0; port < IX_UTOPIA_MAX_PORTS; port++)
        {
            ixAtmdAccTxCfgIfPortShow (port);
            ixAtmdAccTxCfgIfChannelShow (port);
            ixAtmdAccRxCfgIfChannelShow (port);
        }
    }
    else
    {
        printf("IxAtmdAcc Not initialized\n");
    }
}

/*------------------------------------------------------------------------
* display stats from all modules
*/
PUBLIC void
ixAtmdAccStatsShow (void)
{
    if (initDone)
    {
        ixAtmdAccDescMgmtStatsShow ();
        ixAtmdAccPortMgmtStatsShow ();
        ixAtmdAccUtopiaControlStatsShow();
        ixAtmdAccTxCfgIfStatsShow ();
        ixAtmdAccRxCfgIfStatsShow ();
        ixAtmdAccUtilStatsShow ();
    }
    else
    {
        printf("IxAtmdAcc Not initialized\n");
    }
}

/*------------------------------------------------------------------------
* display stats from all modules
*/
PUBLIC void
ixAtmdAccStatsReset (void)
{
    if (initDone)
    {
        ixAtmdAccDescMgmtStatsReset ();
        ixAtmdAccPortMgmtStatsReset ();
        ixAtmdAccUtopiaControlStatsReset();
        ixAtmdAccTxCfgIfStatsReset ();
        ixAtmdAccRxCfgIfStatsReset ();
        ixAtmdAccUtilStatsReset ();
    }
    else
    {
        printf("IxAtmdAcc Not initialized\n");
    }
}


