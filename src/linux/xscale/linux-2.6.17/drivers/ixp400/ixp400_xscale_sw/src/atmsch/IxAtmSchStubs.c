/**
 * @file IxAtmSchStub.c
 *
 * @author Intel Corporation
 * @date  26 Feb 2002
 *
 * @brief Atm Scheduler Stubs
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

#include <IxAtmSch.h>


IX_STATUS
ixAtmSchInit(void)
{
    return IX_FAIL;
}

IX_STATUS
ixAtmSchPortModelInitialize( IxLogicalPort port, 
                             unsigned int portRate,
                             unsigned int minCellsToSchedule)
{
    return IX_FAIL;
}

IX_STATUS
ixAtmSchPortRateModify( IxLogicalPort port, unsigned int portRate)
{
    return IX_FAIL;
}

IX_STATUS
ixAtmSchVcModelSetup( IxLogicalPort port,
 		      IxAtmTrafficDescriptor *trafficDesc,
	 	      IxAtmSchedulerVcId *vcId)
{
    return IX_FAIL;
}

IX_STATUS
ixAtmSchVcConnIdSet( IxLogicalPort port,
                     IxAtmSchedulerVcId vcId,
                     IxAtmConnId connId)
{
    return IX_FAIL;
}

IX_STATUS
ixAtmSchVcModelRemove( IxLogicalPort port,
		       IxAtmSchedulerVcId vcId)
{
    return IX_FAIL;
}

IX_STATUS 
ixAtmSchVcQueueUpdate( IxLogicalPort port, 
                       IxAtmSchedulerVcId vcId, 
                       unsigned int numberOfCells)
{
    return IX_FAIL;
}

 
IX_STATUS 
ixAtmSchVcQueueClear(IxLogicalPort port, IxAtmSchedulerVcId vcId)
{
    return IX_FAIL;
}

IX_STATUS 
ixAtmSchTableUpdate( IxLogicalPort port,
                     unsigned int maxCells,
                     IxAtmScheduleTable **table)
{
    return IX_FAIL;
}

IX_STATUS 
ixAtmSchVcIdGet( IxLogicalPort port,
                 IxAtmConnId connId,
                 IxAtmSchedulerVcId *vcId)
{
    return IX_FAIL;
}

void
ixAtmSchShow(void)
{
}

void
ixAtmSchStatsClear(void)
{
}

