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
        /* Check for presence of components */
        if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_UTOPIA)== 
	    IX_FEATURE_CTRL_COMPONENT_DISABLED)
	{
	    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
		       "Warning: the UTOPIA component you specified does"
		       " not exist\n", 0,0,0,0,0,0);
            return IX_FAIL; 
	}
	
	if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_AAL)== 
	    IX_FEATURE_CTRL_COMPONENT_DISABLED)
        {
	    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
		     "Warning: the AAL component you specified does"
		     " not exist\n", 0,0,0,0,0,0);
            return IX_FAIL;
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


