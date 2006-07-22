/* 
 * FileName:    IxAtmm.c
 * Author: Intel Corporation
 * Created:     14-AUG-2000
 * Description: 
 *    This is the implementiation file for the Atm manager.
 * 
 * Design Notes:
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
 * Put the system defined include files required
 */

#include <IxOsalTypes.h>
#include <IxAtmm.h>
#include "IxTrace.h"
#include "IxRscm.h"

/*
 * Put the user defined include files required
 */
#include "IxAtmmStubs.h"

/*
 * #defines and macros used in this file.
 */
#define RX_VC_ID 0x0010
#define TX_VC_ID 0x0020

/*
 * Typedefs whose scope is limited to this file.
 */

/*
 * Extern function prototypes
 */

/*
 * Static function prototypes
 */
PRIVATE int 
ixAtmmDefaultUserDeregister (LogicalPort port, IxAtmVcId vcId, int userType);

/*
 * Variable declarations global to this file only. Externs are followed by
 * static variables.
 */

/*
 * These global variables enable/disable IxAtmm functions for unit test
 * case (not production code)
 */
BOOL ixAtmmRxUserRegisterEnabled = TRUE;
BOOL ixAtmmTxUserRegisterEnabled = TRUE;

BOOL ixAtmmRxVcQueryEnabled = TRUE;
BOOL ixAtmmTxVcQueryEnabled = TRUE;

BOOL ixAtmmRxUserDeregisterEnabled = TRUE;
BOOL ixAtmmTxUserDeregisterEnabled = TRUE;

ixAtmmUserDeregisterFunc userDeregister = &ixAtmmDefaultUserDeregister;

IxAtmmCallbackVcChange atmmCallbackVcChange;

/*
 * Function definition
 */

int ixAtmmInitializePort (LogicalPort port, unsigned upstreamPortRate, 
			  unsigned downstreamPortRate)
{
    return IX_FAIL;
}

int 
ixAtmmVcRegister (LogicalPort port, IxAtmmVc *vcToAdd, IxAtmVcId *vcId)
{
    return IX_FAIL;
}

int 
ixAtmmVcDeregister (LogicalPort port, IxAtmVcId vcId)
{
    return IX_FAIL;
}

int 
ixAtmmVpRegister (LogicalPort port, AtmVpi vpi, 
		  IxAtmTrafficDescriptor *descriptor, int *vpId)
{
    return IX_FAIL;
}

int 
ixAtmmVpDeregister (LogicalPort port, int vpId)
{
    return IX_FAIL;
}

int 
ixAtmmPortQuery (LogicalPort *port, unsigned *upstreamPortRate, 
		 unsigned *downstreamPortRate)
{
    TRACE0(IxAtmmId, IXTR_FNENTRYEXIT, "ixAtmmPortQuery");
    *port = IX_UTOPIA_PORT_1;
    return IX_SUCCESS;
}

int 
ixAtmmVcQuery (LogicalPort port, AtmVpi vpi, AtmVci vci, int direction, 
	       IxAtmVcId *vcId, IxAtmTrafficDescriptor *descriptor)
{
    TRACE0(IxAtmmId, IXTR_FNENTRYEXIT, "ixAtmmVcQuery");

    IX_RSCM_ASSERT((port == UTOPIA_PORT_1) || (port == HSS_PORT_1));
    
    switch (direction)
    {
	case IX_ATMM_VC_DIRECTION_RX:
	    if (!ixAtmmRxVcQueryEnabled)
	    {
		return IX_FAIL;
	    }
	    *vcId = RX_VC_ID;
	    break;
	case IX_ATMM_VC_DIRECTION_TX:
	default:
	    if (!ixAtmmTxVcQueryEnabled)
	    {
		return IX_FAIL;
	    }
	    *vcId = TX_VC_ID;
	    break;
    }
    
    return IX_SUCCESS;
}

int 
ixAtmmVpQuery (LogicalPort port, AtmVpi vpi, int *vpId, 
	       IxAtmTrafficDescriptor *descriptor)
{
    return IX_FAIL;
}

int 
ixAtmmVcIdQuery (LogicalPort port, IxAtmVcId vcId, IxAtmmVc *vc)
{
    return IX_FAIL;
}

int 
ixAtmmVpIdQuery (LogicalPort port, int vpId, AtmVpi *vpi, 
		 IxAtmTrafficDescriptor *descriptor)
{
    return IX_FAIL;
}

int 
ixAtmmVcHookChange (IxAtmmCallbackVcChange callback)
{
    atmmCallbackVcChange = callback;
    return IX_SUCCESS;
}

int
ixAtmmVcUnhookChange (IxAtmmCallbackVcChange callback)
{
    return IX_FAIL;
}

int 
ixAtmmUserRegister (LogicalPort port, IxAtmVcId vcId, int userType, IxCompId compId)
{
    TRACE0(IxAtmmId, IXTR_FNENTRYEXIT, "ixAtmmUserRegister");


    IX_RSCM_ASSERT((port == UTOPIA_PORT_1) || (port == HSS_PORT_1));
    IX_RSCM_ASSERT((vcId == RX_VC_ID) || (vcId == TX_VC_ID));
    IX_RSCM_ASSERT(
	(userType == IX_ATMM_USER_TYPE_DATA_USER)
	|| (userType == IX_ATMM_USER_TYPE_OAM_USER));
    IX_RSCM_ASSERT(
	(compId == IxAal2Id)
	|| (compId == IxAal5Id)
	|| (compId == IxAal0Id));

    if (vcId == RX_VC_ID)
    {
	if (!ixAtmmRxUserRegisterEnabled)
	{
	    return IX_FAIL;
	}
    }
    else
    {
	if (!ixAtmmTxUserRegisterEnabled)
	{
	    return IX_FAIL;
	}
    }

    return IX_SUCCESS;
}

int 
ixAtmmUserDeregister (LogicalPort port, IxAtmVcId vcId, int userType)
{
    return (*userDeregister)(
        port, 
	vcId, 
	userType);
}

PRIVATE int 
ixAtmmDefaultUserDeregister (LogicalPort port, IxAtmVcId vcId, int userType)
{
    if (vcId == RX_VC_ID)
    {
	if (!ixAtmmRxUserDeregisterEnabled)
	{
	    return IX_FAIL;
	}
    }
    else
    {
	if (!ixAtmmTxUserDeregisterEnabled)
	{
	    return IX_FAIL;
	}
    }

    return IX_SUCCESS;
}

int 
ixAtmmManagementInfoGet (const IxAtmmManagerInfo entity, 
			 const UINT32 index, 
                         ManagerInfoData *dataBufferToWriteInto)
{
    return IX_FAIL;
}

int 
ixAtmmManagementInfoGetNext (const IxAtmmManagerInfo entity, 
			     UINT32 *index, 
                             ManagerInfoData *dataBufferToWriteInto)
{
    return IX_FAIL;
}

int 
ixAtmmManagementInfoTest (const IxAtmmManagerInfo entity, 
			  const UINT32 index, 
			  const ManagerInfoData *dataBufferToRead)
{
    return IX_FAIL;
}

int 
ixAtmmManagementInfoSet (const IxAtmmManagerInfo entity, 

			 const UINT32 index, 
                         const ManagerInfoData *dataBufferToRead)
{
    return IX_FAIL;
}

int 
ixAtmmTrapSubscriberBind (IxAtmmTrapIf snmpTrapIf)
{
    return IX_FAIL;
}

void ixAtmmStubsUserDeregisterCallbackSet (ixAtmmUserDeregisterFunc funcPtr)
{
    userDeregister = funcPtr;
}
void ixAtmmStubsUserDeregisterCallbackClear (void)
{
    userDeregister = &ixAtmmDefaultUserDeregister;
}

