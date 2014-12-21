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

