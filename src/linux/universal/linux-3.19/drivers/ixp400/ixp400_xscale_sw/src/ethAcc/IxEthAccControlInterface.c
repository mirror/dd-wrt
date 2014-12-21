/**
 * @file IxEthAccControlInterface.c
 *
 * @author Intel Corporation
 * @date 
 *
 * @brief IX_ETH_ACC_PUBLIC wrappers for control plane functions
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

#include "IxOsal.h"
#include "IxQMgr.h"
#include "IxEthAcc.h"
#include "IxEthAcc_p.h"

PUBLIC IxOsalMutex ixEthAccControlInterfaceMutex;
PUBLIC UINT32 ixEthAccTrafficStopRequestCount = IX_ETH_ACC_NO_STOP_REQUEST;

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
        IX_ETH_ACC_WARNING_LOG("ixEthAccPortEnable: In port %d, service not initialized\n",
			       (INT32) portId,0,0,0,0,0);
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    /* check the context is iinitialized */
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
        IX_ETH_ACC_WARNING_LOG("ixEthAccPortDisable: In port %d, service not initialized\n",
			       (INT32) portId,0,0,0,0,0);
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortEnabledQuery(IxEthAccPortId portId, BOOL *enabled)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortEnabledQueryPriv(portId, enabled);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortPromiscuousModeClear(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    if (!in_atomic())
    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortPromiscuousModeClearPriv(portId);
    if (!in_atomic())
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortPromiscuousModeSet(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    if (!in_atomic())
    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortPromiscuousModeSetPriv(portId);
    if (!in_atomic())
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortUnicastMacAddressSet(IxEthAccPortId portId, IxEthAccMacAddr *macAddr)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    if (!in_atomic())
    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortUnicastMacAddressSetPriv(portId, macAddr);
    if (!in_atomic())
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortUnicastMacAddressGet(IxEthAccPortId portId, IxEthAccMacAddr *macAddr)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    if (!in_atomic())
    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortUnicastMacAddressGetPriv(portId, macAddr);
    if (!in_atomic())
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressJoin(IxEthAccPortId portId, IxEthAccMacAddr *macAddr)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    if (!in_atomic())
    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortMulticastAddressJoinPriv(portId, macAddr);
    if (!in_atomic())
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressJoinAll(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    if (!in_atomic())
    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortMulticastAddressJoinAllPriv(portId);
    if (!in_atomic())
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressLeave(IxEthAccPortId portId, IxEthAccMacAddr *macAddr)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    if (!in_atomic())
    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortMulticastAddressLeavePriv(portId, macAddr);
    if (!in_atomic())
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortMulticastAddressLeaveAll(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!in_atomic())
    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortMulticastAddressLeaveAllPriv(portId);
    if (!in_atomic())
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortUnicastAddressShow(IxEthAccPortId portId)
{
    IxEthAccStatus result;
 
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortUnicastAddressShowPriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC void 
ixEthAccPortMulticastAddressShow(IxEthAccPortId portId)
{
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return;
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    ixEthAccPortMulticastAddressShowPriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortDuplexModeSet(IxEthAccPortId portId, IxEthAccDuplexMode mode)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortDuplexModeSetPriv(portId, mode);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortDuplexModeGet(IxEthAccPortId portId, IxEthAccDuplexMode *mode)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortDuplexModeGetPriv(portId, mode);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendPaddingEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;
     
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxFrameAppendPaddingEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendPaddingDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxFrameAppendPaddingDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendFCSEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxFrameAppendFCSEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortTxFrameAppendFCSDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxFrameAppendFCSDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortRxFrameAppendFCSEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortRxFrameAppendFCSEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccPortRxFrameAppendFCSDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortRxFrameAppendFCSDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccTxSchedulingDisciplineSet(IxEthAccPortId portId, IxEthAccSchedulerDiscipline sched)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccTxSchedulingDisciplineSetPriv(portId, sched);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus
ixEthAccRxSchedulingDisciplineSet(IxEthAccSchedulerDiscipline sched)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccRxSchedulingDisciplineSetPriv(sched);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortNpeLoopbackEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccNpeLoopbackEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortTxEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortRxEnable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortRxEnablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortNpeLoopbackDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccNpeLoopbackDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortTxDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortTxDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortRxDisable(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortRxDisablePriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccPortMacReset(IxEthAccPortId portId)
{
    IxEthAccStatus result;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&ixEthAccControlInterfaceMutex, IX_OSAL_WAIT_FOREVER);
    result = ixEthAccPortMacResetPriv(portId);
    ixOsalMutexUnlock(&ixEthAccControlInterfaceMutex);
    return result;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccStopRequest(void)
{
   UINT32 intLockKey;   

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

   /* lock interrupts */
   intLockKey = ixOsalIrqLock();   

   /* Increment internal counter */
   ixEthAccTrafficStopRequestCount++;
   
   if (ixEthAccTrafficStopRequestCount == IX_ETH_ACC_FIRST_STOP_REQUEST)
   {
      /* Notify QDispatcher to disable its service only on the first request.
       * Subsequent request might be called by due to soft-error at other NPE.
       * ixEthAccTrafficStopRequestCount is used to keep synchronize multiple 
       * service stop requests and only re-enable QDispatcher when the counter
       * becomes zero. 
       */
      ixQMgrDispatcherLoopDisable();
   }

   /* Unlock interrupts */
   ixOsalIrqUnlock(intLockKey);

   return IX_ETH_ACC_SUCCESS;
}

IX_ETH_ACC_PUBLIC IxEthAccStatus 
ixEthAccStartRequest(void)
{
    UINT32 intLockKey; 

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

   /* lock interrupts */
   intLockKey = ixOsalIrqLock();   

   /* The check is to protect the counter when going negative
    * if this function is called multiple time 
    */
   if (ixEthAccTrafficStopRequestCount > IX_ETH_ACC_NO_STOP_REQUEST)
   {
      /* Decrement internal counter */
      ixEthAccTrafficStopRequestCount--;
   }
   else
   {
     return (IX_ETH_ACC_FAIL);
   }

   /* After this point, return status is IX_ETH_ACC_SUCCESS
    * to notify that the start request has been accepted.
    */

   /* Eth services are only started if there is no more stop request */
   if (ixEthAccTrafficStopRequestCount == IX_ETH_ACC_NO_STOP_REQUEST)
   {
      /* Notify QDispatcher to enable its service.
       * This starts data-plane related traffic services.
       */
      ixQMgrDispatcherLoopEnable();
   } 

   /* Unlock interrupts */
   ixOsalIrqUnlock(intLockKey);

   return (IX_ETH_ACC_SUCCESS);
}

IX_ETH_ACC_PUBLIC BOOL 
ixEthAccStopDoneCheck(void)
{
  UINT32 intLockKey; 
  BOOL ethTrafficIsStopped;

   /* lock interrupts */
   intLockKey = ixOsalIrqLock();

   if ((ixEthAccTxSubmitBusyCount == IX_ETH_ACC_TX_FRAME_SUBMIT_NOT_BUSY) && 
      (ixQMgrDispatcherLoopStatusGet() == IX_QMGR_DISPATCHER_LOOP_FREE))
   {
       ethTrafficIsStopped = TRUE;
   } 
   else
   {
      ethTrafficIsStopped = FALSE;
   } 

   /* Unlock interrupts */
   ixOsalIrqUnlock(intLockKey);

   return ethTrafficIsStopped;
}

