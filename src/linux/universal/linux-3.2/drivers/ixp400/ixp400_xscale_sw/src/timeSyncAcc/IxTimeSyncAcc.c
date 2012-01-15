/**
 * @file IxTimeSyncAcc.c
 *
 * @author Intel Corporation
 * @date 07 July 2004
 *
 * @brief  Source file for IXP400 Access Layer to IEEE 1588(TM) Precision
 * Clock Synchronisation Protocol Hardware Assist.
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
 * System defined include files
 */
#if defined (__ixp46X)


#include "IxOsal.h"

/*
 * User defined include files
 */
#include "IxTimeSyncAcc.h"
#include "IxTimeSyncAcc_p.h"
#include "IxFeatureCtrl.h"

/*
 * Support functions definitions
 */

/* Function for setting the base address registers */
PRIVATE IX_STATUS
ixTimeSyncAccBlPlBaseAddressesSet (void)
{
    /* Local variables */
    UINT32 blRegsVirtualBaseAddr = (UINT32) NULL;
    UINT32 plRegsVirtualBaseAddr = (UINT32) NULL;
    UINT32 ptpPortNum = 0;

    /* Memory mapping of the Block Level registers starting address */
    blRegsVirtualBaseAddr = (UINT32)IX_OSAL_MEM_MAP (
                                IXP400_TIMESYNCACC_BLREGS_BASEADDR,
                                IXP400_TIMESYNCACC_BLREGS_MEMMAP_SIZE); 
    if ((UINT32)NULL == blRegsVirtualBaseAddr)
    {
        /* Virtual memory mapping failed */
        return IX_FAIL;
    } /* end of if ((UINT32)NULL == blRegsVirtualBaseAddr) */

    /* Memory mapping of the Port Level registers starting address */
    plRegsVirtualBaseAddr = (UINT32)IX_OSAL_MEM_MAP (
                                IXP400_TIMESYNCACC_PLREGS_BASEADDR, 
                                IXP400_TIMESYNCACC_PLREGS_MEMMAP_SIZE); 
    if ((UINT32)NULL == plRegsVirtualBaseAddr)
    {
        /* Virtual memory mapping failed */

        /* Unmap the virtual mapping of Block Level registers */
        IX_OSAL_MEM_UNMAP(blRegsVirtualBaseAddr); 
        return IX_FAIL;
    } /* end of if ((UINT32)NULL == plRegsVirtualBaseAddr) */

    /* Virtual Addresses assignment for Block Level Registers */
    ixTsRegisters.blRegisters.tsControl   = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_TSC_OFFSET;
    ixTsRegisters.blRegisters.tsEvent     = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_TSE_OFFSET;
    ixTsRegisters.blRegisters.tsAddend    = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_ADD_OFFSET;
    ixTsRegisters.blRegisters.tsAccum     = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_ACC_OFFSET;
    ixTsRegisters.blRegisters.tsSysTimeLo = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_STL_OFFSET;
    ixTsRegisters.blRegisters.tsSysTimeHi = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_STH_OFFSET;
    ixTsRegisters.blRegisters.tsTrgtLo    = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_TTL_OFFSET;
    ixTsRegisters.blRegisters.tsTrgtHi    =
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_TTH_OFFSET;
    ixTsRegisters.blRegisters.tsASMSLo    = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_ASSL_OFFSET;
    ixTsRegisters.blRegisters.tsASMSHi    = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_ASSH_OFFSET;
    ixTsRegisters.blRegisters.tsAMMSLo    = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_AMSL_OFFSET;
    ixTsRegisters.blRegisters.tsAMMSHi    = 
        blRegsVirtualBaseAddr + IXP400_TIMESYNCACC_AMSH_OFFSET;

    /* Virtual Addresses assignment for Port Level Registers */
    for (ptpPortNum = 0; 
         ptpPortNum < IXP400_TIMESYNCACC_MAX_1588PTP_PORT;
         ptpPortNum++)
    {
        ixTsRegisters.plRegisters[ptpPortNum].tsChControl  = 
            plRegsVirtualBaseAddr + IXP400_TIMESYNCACC_CC_OFFSET(ptpPortNum);
        ixTsRegisters.plRegisters[ptpPortNum].tsChEvent    = 
            plRegsVirtualBaseAddr + IXP400_TIMESYNCACC_CE_OFFSET(ptpPortNum);
        ixTsRegisters.plRegisters[ptpPortNum].tsTxSnapLo   = 
            plRegsVirtualBaseAddr + IXP400_TIMESYNCACC_XSL_OFFSET(ptpPortNum);
        ixTsRegisters.plRegisters[ptpPortNum].tsTxSnapHi   = 
            plRegsVirtualBaseAddr + IXP400_TIMESYNCACC_XSH_OFFSET(ptpPortNum);
        ixTsRegisters.plRegisters[ptpPortNum].tsRxSnapLo   =
            plRegsVirtualBaseAddr + IXP400_TIMESYNCACC_RSL_OFFSET(ptpPortNum);
        ixTsRegisters.plRegisters[ptpPortNum].tsRxSnapHi   =
            plRegsVirtualBaseAddr + IXP400_TIMESYNCACC_RSH_OFFSET(ptpPortNum);
        ixTsRegisters.plRegisters[ptpPortNum].tsSrcUUIDLo  =
            plRegsVirtualBaseAddr + IXP400_TIMESYNCACC_UID_OFFSET(ptpPortNum);
        ixTsRegisters.plRegisters[ptpPortNum].tsSrcUUIDHi  =
            plRegsVirtualBaseAddr + IXP400_TIMESYNCACC_SID_OFFSET(ptpPortNum);
    } /* end of for (ptpPortNum = 0; 
                     ptpPortNum < IXP400_TIMESYNCACC_MAX_1588PTP_PORT;
                     ptpPortNum++ ) */

    return IX_SUCCESS;
}  /* end of ixTimeSyncAccBlPlBaseAddressesSet() function */

/* Function to perform the following for Initialsation Check 
 * 
 * - Check for the correct hardware platform
 * - NPE Port fused-out status
 * - Virtual Memory Map assignment
 */
PRIVATE IxTimeSyncAccInitStatus
ixTimeSyncAccInitCheck (void)
{
    /* Check for IXP46X device */
    if(IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X != ixFeatureCtrlDeviceRead())
    {
#ifndef NDEBUG
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixTimeSyncAccInitCheck(): "
                "TimeSync not supported on this device\n",
                0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
        return IXP400_TIMESYNCACC_INIT_TS_SUPPORT_FAIL;
    } /* end of if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X != 
                    ixFeatureCtrlDeviceRead()) */

    /*
     * Check TimeSync/ECC Fuse-Out Status
     * 
     * NOTE: Feature Control uses #define - IX_FEATURECTRL_ECC_TIMESYNC for
     * both the TimeSync and ECC feature of SDRAM Controller
     */
    if (IX_FEATURE_CTRL_COMPONENT_ENABLED != 
        ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ECC_TIMESYNC))
    {
        return IXP400_TIMESYNCACC_INIT_TS_DISABLED_FAIL;
    } /* end of if (IX_FEATURE_CTRL_COMPONENT_ENABLED != 
                    ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ECC_TIMESYNC)) */

    /*
     * Secure mutex to protect the initialisation sequence so as to 
     * avoid the possibility of two public APIs invoking this function
     * which can result in corruption of the memory mapping of virtual
     * addresses in the registers data structures.
     */
    if (IX_SUCCESS != ixOsalMutexInit(&ixTsInitChkMutex))
    {
        return IXP400_TIMESYNCACC_INIT_MUTEX_FAIL;
    } /* end of if (IX_SUCCESS != ixOsalMutexInit(&ixTsInitChkMutex)) */

    if (IX_SUCCESS != ixOsalMutexLock(&ixTsInitChkMutex, IX_OSAL_WAIT_FOREVER))
    {
        /*
         * Return failure without destroying the init mutex here
         * since, there is a possibility that task/thread switch
         * over after initialisation of the mutex before locking
         * the mutex. This allows the other task/thread to have
         * the initialisation sequence complete successfully and
         * will eventually release the mutex at the end of init.
         */
        return IXP400_TIMESYNCACC_INIT_MUTEX_FAIL;
    } /* end of if (IX_SUCCESS != ixOsalMutexLock(&ixTsInitChkMutex, IX_OSAL_WAIT_FOREVER)) */

    /*
     * Initialised before?
     * 
     * NOTE: This would be possible, if multiple public APIs are invoked
     * where the initialisation condition is verified for FALSE but has
     * been subjected to context switch. Now other APIs completed the
     * initialisation successfully and updates the status to TRUE, but
     * the API which will resume later has the knowledge of FALSE status
     * only and enters this function and will still be able to make out
     * that the status now is TRUE as if it succeeded in initialisation
     * checks.
     */
    if (TRUE == ixTs1588HardwareAssistEnabled)
    {
        /* Release the init mutex  */
        IXP400_TIMESYNCACC_MUTEX_RELEASE(ixTsInitChkMutex);

        return IXP400_TIMESYNCACC_INIT_SUCCESS;
    } /* end of (TRUE == ixTs1588HardwareAssistEnabled) */

    /*
     * Initialise other mutexes
     */
    if (IX_SUCCESS != ixOsalMutexInit(&ixTsSysTimeMutex))
    {
        /* Release the init mutex */
        IXP400_TIMESYNCACC_MUTEX_RELEASE(ixTsInitChkMutex);

        return IXP400_TIMESYNCACC_INIT_MUTEX_FAIL;
    } /* end of if (IX_SUCCESS != ixOsalMutexInit(&ixTsSysTimeMutex)) */

    /* Update NPEs Fused-Out status */

    if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
        ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA))
    {
        ixTsNpeEnabled[IXP400_TIMESYNCACC_NPEA] = TRUE;
    } /* end of if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
                    ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA)) */

    if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
        ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB))
    {
        ixTsNpeEnabled[IXP400_TIMESYNCACC_NPEB] = TRUE;
    } /* end of if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
                    ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB)) */

    if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
        ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC))
    {
        ixTsNpeEnabled[IXP400_TIMESYNCACC_NPEC] = TRUE;
    } /* end of if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
                    ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC)) */

    /* Register the TimeSync Interrupt Service Routine */
    if (IX_SUCCESS != ixOsalIrqBind((UINT32) IRQ_IXP400_INTC_TSYNC, 
                          (IxOsalVoidFnVoidPtr) ixTimeSyncAccIsr,
                          (void *) NULL))
    {
        /* Destroy the system time mutex */
        ixOsalMutexDestroy(&ixTsSysTimeMutex);
        /* Release the init mutex */
        IXP400_TIMESYNCACC_MUTEX_RELEASE(ixTsInitChkMutex);

        return IXP400_TIMESYNCACC_INIT_ISR_BIND_FAIL;
    } /* end of if (IX_SUCCESS != ixOsalIrqBind((UINT32) IRQ_IXP400_INTC_TSYNC, 
                                      (IxOsalVoidFnVoidPtr) ixTimeSyncAccIsr,
                                      (void *) NULL)) */

    /* Assign memory mapped virtual addresses */
    if (IX_SUCCESS != ixTimeSyncAccBlPlBaseAddressesSet())
    {
        /* Destroy the system time mutex */
        ixOsalMutexDestroy(&ixTsSysTimeMutex);
        /* Release the init mutex */
        IXP400_TIMESYNCACC_MUTEX_RELEASE(ixTsInitChkMutex);

        /* Deregister TimeSync Interrupt Service Routine */
        ixOsalIrqUnbind((UINT32) IRQ_IXP400_INTC_TSYNC);

        return IXP400_TIMESYNCACC_INIT_MEM_ASGN_FAIL;
    } /* end of if (IX_SUCCESS != ixTimeSyncAccBlPlBaseAddressesSet()) */

    /* Clear the snapshot availability condition for both master aux and slave
        aux */
    ixTimeSyncAccEventAmmsFlagClear();
    ixTimeSyncAccEventAsmsFlagClear();
    
    /* Set Initialisation Check Status */
    ixTs1588HardwareAssistEnabled = TRUE;

    /* Release the init mutex */
    IXP400_TIMESYNCACC_MUTEX_RELEASE(ixTsInitChkMutex);

    return IXP400_TIMESYNCACC_INIT_SUCCESS;
} /* end of ixTimeSyncAccInitCheck() function */

/* Function to determine the port mode */
PRIVATE IxTimeSyncAcc1588PTPPortMode
ixTimeSyncAccPTPPortModeGet(IxTimeSyncAcc1588PTPPort ptpPort)
{
    /* Local variables */
    BOOL masterMode = FALSE;
    BOOL allMsgMode = FALSE;
    IxTimeSyncAcc1588PTPPortMode ptpPortMode = 
                                 IX_TIMESYNCACC_1588PTP_PORT_SLAVE;

    /* Get the Mode of the PTP Port */
    masterMode = ixTimeSyncAccControlPTPPortMasterModeGet(ptpPort);
    allMsgMode = ixTimeSyncAccControlPTPPortPTPMsgTimestampGet(ptpPort);

    /* Is ANY mode (all message timestamp mode) on? */
    if (FALSE == allMsgMode)
    {
        /* Is Master mode on? */
        if (TRUE == masterMode)
        {
            ptpPortMode = IX_TIMESYNCACC_1588PTP_PORT_MASTER;
        } /* if (TRUE == masterMode)  */
        else
        {
            ptpPortMode = IX_TIMESYNCACC_1588PTP_PORT_SLAVE;
        } /* end of if (TRUE == masterMode)  */
    } /* if (FALSE == allMsgMode) */
    else
    {
        /* 
         * When Any mode is on (the ta bit is set) we do not care
         * for Master/Slave mode (the mm bit status) since all the
         * packets gets time stamped anyways.
         */
        ptpPortMode = IX_TIMESYNCACC_1588PTP_PORT_ANYMODE;
    } /* end of if (FALSE == allMsgMode) */

    return ptpPortMode;
} /* end of ixTimeSyncAccPTPPortModeGet() function */

/*
 * TimeSync Interrupt Service Routine definition
 */
void
ixTimeSyncAccIsr(void)
{
    /* Local variables */
    IxTimeSyncAccTimeValue targetTime = {0, 0};
    IxTimeSyncAccTimeValue auxTime = {0, 0};

    /* 
     * Handle the Interrupts in the following order
     * 
     * 1 - Target Time Reached/Hit Condition
     * 2 - Auxiliary Master Timestamp
     * 3 - Auxiliary Slave Timestamp
     *
     * Also verify that valid callbacks are available for each of the following
     * since the client application may choose to use one or more of the following
     * in interrupt mode while others in non-interrupt mode i.e., makes use of poll
     * or get methods in which case there is no valid callback registered.
     */

    /* Handle Target Time Reached or Exceeded Interrupt */
    if ((NULL != ixTsTargetTimeCallback) && 
        (TRUE == ixTimeSyncAccEventTtmFlagGet()))
    {
        /* Target Time registers contents  */
        ixTimeSyncAccTargetTimeSnapshotGet(&targetTime.timeValueLowWord,
                                           &targetTime.timeValueHighWord);

        /* Invoke client callback */
        (*ixTsTargetTimeCallback)(targetTime);

        /* Clear the target time reached condition (ttipend bit) */
        ixTimeSyncAccEventTtmFlagClear();

        return;
    } /* end of if ((NULL != ixTsTargetTimeCallback) && 
                (TRUE == ixTimeSyncAccEventTtmFlagGet())) */

    /* Handle Auxiliary Master Mode Snapshot Interrupt */
    if ((NULL != ixTsAuxMasterTimeCallback) &&
        (TRUE == ixTimeSyncAccEventAmmsFlagGet()))
    {
        /* Fetch Auxiliary Master Mode Snapshot */
        ixTimeSyncAccAuxMasterModeSnapshotGet(&auxTime.timeValueLowWord,
                                              &auxTime.timeValueHighWord);

        /* Return Auxiliary Master Mode Snapshot */
        (*ixTsAuxMasterTimeCallback)(IX_TIMESYNCACC_AUXMODE_MASTER,auxTime);

        /* Clear the snapshot availability condition */
        ixTimeSyncAccEventAmmsFlagClear();

        return;
    } /* end of if ((NULL != ixTsAuxMasterTimeCallback) &&
                    (TRUE == ixTimeSyncAccEventAmmsFlagGet())) */

    /* Handle Auxiliary Slave Mode Snapshot Interrupt */
    if ((NULL != ixTsAuxSlaveTimeCallback) &&
        (TRUE == ixTimeSyncAccEventAsmsFlagGet()))
    {
        /* Fetch Auxiliary Slave Mode Snapshot */
        ixTimeSyncAccAuxSlaveModeSnapshotGet(&auxTime.timeValueLowWord,
                                             &auxTime.timeValueHighWord);

        /* Return Auxiliary Slave Mode Snapshot */
        (*ixTsAuxSlaveTimeCallback)(IX_TIMESYNCACC_AUXMODE_SLAVE,auxTime);

        /* Clear the snapshot availability condition */
        ixTimeSyncAccEventAsmsFlagClear();

        return;
    } /* end of if ((NULL != ixTsAuxSlaveTimeCallback) &&
                    (TRUE == ixTimeSyncAccEventAsmsFlagGet())) */

    /* This part of code should never be reached */
#ifndef NDEBUG
    ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
        "ixTimeSyncAccIsr(): Invalid Interrupt!!!\n",
        0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
    return;
} /* end of ixTimeSyncAccIsr() function */

/*
 * ------------------------------------------------------------------ *
 *                        Public API definitions
 * ------------------------------------------------------------------ *
 */

/* Configure IEEE 1588 message detection on a particular PTP port */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccPTPPortConfigSet(
    IxTimeSyncAcc1588PTPPort ptpPort,
    IxTimeSyncAcc1588PTPPortMode ptpPortMode)
{
    /* Verify the parameters for proper values */
    if ((IX_TIMESYNCACC_NPE_1588PORT_INVALID <= ptpPort) || 
        (IX_TIMESYNCACC_1588PTP_PORT_MODE_INVALID <= ptpPortMode))
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of if ((IX_TIMESYNCACC_NPE_1588PORT_INVALID <= ptpPort) || 
                    (IX_TIMESYNCACC_1588PTP_PORT_MODE_INVALID <= ptpPortMode)) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Check NPE fused-out status */
    IXP400_TIMESYNCACC_NPE_FUSED_STATUS_CHECK(ptpPort);

    /* Set the Mode of the PTP Port */
    switch (ptpPortMode)
    {
        case IX_TIMESYNCACC_1588PTP_PORT_MASTER:
        {
             ixTimeSyncAccControlPTPPortMasterModeSet(ptpPort, TRUE);
             ixTimeSyncAccControlPTPPortPTPMsgTimestampSet(ptpPort, FALSE);
             break;
        } /* end of case IX_TIMESYNCACC_1588PTP_PORT_MASTER */
        case IX_TIMESYNCACC_1588PTP_PORT_SLAVE:
        {
             ixTimeSyncAccControlPTPPortMasterModeSet(ptpPort, FALSE);
             ixTimeSyncAccControlPTPPortPTPMsgTimestampSet(ptpPort, FALSE);
             break;
        } /* end of case IX_TIMESYNCACC_1588PTP_PORT_SLAVE */
        case IX_TIMESYNCACC_1588PTP_PORT_ANYMODE:
        {
             ixTimeSyncAccControlPTPPortMasterModeSet(ptpPort, FALSE);
             ixTimeSyncAccControlPTPPortPTPMsgTimestampSet(ptpPort, TRUE);
             break;
        } /* end of case IX_TIMESYNCACC_1588PTP_PORT_ANYMODE */
        default:
        {
            /* This part of the code should not be reached */
#ifndef NDEBUG
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixTimeSyncAccPTPPortConfigSet(): "
                "Invalid Port Mode\n",
                0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
            return IX_TIMESYNCACC_FAILED;
        } /* end of case default */
    } /* end of switch (ptpPortMode) */

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccPTPPortConfigSet() function */

/* Retrieve IEEE 1588 PTP operation mode on particular PTP port */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccPTPPortConfigGet(
    IxTimeSyncAcc1588PTPPort ptpPort,
    IxTimeSyncAcc1588PTPPortMode *ptpPortMode)
{
    /* Verify the parameters for proper values */
    if ((IX_TIMESYNCACC_NPE_1588PORT_INVALID <= ptpPort) ||
        ((IxTimeSyncAcc1588PTPPortMode *)NULL == ptpPortMode))
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of if ((IX_TIMESYNCACC_NPE_1588PORT_INVALID <= ptpPort) ||
                    ((IxTimeSyncAcc1588PTPPortMode *)NULL == ptpPortMode)) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Check NPE fused-out status */
    IXP400_TIMESYNCACC_NPE_FUSED_STATUS_CHECK(ptpPort);

    /* Get the Mode of the PTP Port */
    *ptpPortMode = ixTimeSyncAccPTPPortModeGet(ptpPort);

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccPTPPortConfigGet() function */

/* 
 * Poll the IEEE 1588 message/time stamp detect status on a particular 
 * PTP Port on the Receive side
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccPTPRxPoll(
    IxTimeSyncAcc1588PTPPort ptpPort,
    IxTimeSyncAccPtpMsgData  *ptpMsgData)
{
    /* Local variables */
    BOOL rxsFlag = FALSE;
    IxTimeSyncAcc1588PTPPortMode ptpPortMode = 
                                 IX_TIMESYNCACC_1588PTP_PORT_SLAVE;

    /* Verify the parameters for proper values */
    if ((IX_TIMESYNCACC_NPE_1588PORT_INVALID <= ptpPort) ||
        ((IxTimeSyncAccPtpMsgData *)NULL == ptpMsgData))
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of if ((IX_TIMESYNCACC_NPE_1588PORT_INVALID <= ptpPort) ||
                    ((IxTimeSyncAccPtpMsgData *)NULL == ptpMsgData)) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Check NPE fused-out status */
    IXP400_TIMESYNCACC_NPE_FUSED_STATUS_CHECK(ptpPort);

    /* Get the Mode of the PTP Port */
    ptpPortMode = ixTimeSyncAccPTPPortModeGet(ptpPort);
    
    /* Is the Port Mode is ANY mode OR the receive timestamp available? */
    rxsFlag = ixTimeSyncAccControlPTPPortRxsFlagGet(ptpPort);

    /* Neither the port is configured for 'Any Mode' nor there is a timestamp */
    if ((IX_TIMESYNCACC_1588PTP_PORT_ANYMODE != ptpPortMode) &&
        (TRUE != rxsFlag))
    {
        return IX_TIMESYNCACC_NOTIMESTAMP;
    } /* end of if ((IX_TIMESYNCACC_1588PTP_PORT_ANYMODE != ptpPortMode) &&
                    (TRUE != rxsFlag)) */

    /* Fetch the receive timestamp */
    ixTimeSyncAccPTPPortReceiveSnapshotGet(ptpPort, 
        &ptpMsgData->ptpTimeStamp.timeValueLowWord,
        &ptpMsgData->ptpTimeStamp.timeValueHighWord);

    /* Fetch the UUID & Seq# of PTP messages in 'Master/Slave Mode' only */
    if (TRUE == rxsFlag)
    {
        ixTimeSyncAccPTPMsgUuidSeqIdGet(ptpPort,
            &ptpMsgData->ptpUuid.uuidValueLowWord,
            &ptpMsgData->ptpUuid.uuidValueHighHalfword,
            &ptpMsgData->ptpSequenceNumber);
    }
    /* Clear-off the UUID & Seq# of all the messages in 'Any Mode' */
    else
    {
        ptpMsgData->ptpUuid.uuidValueLowWord = 0;
        ptpMsgData->ptpUuid.uuidValueHighHalfword = 0;
        ptpMsgData->ptpSequenceNumber = 0;
    } /* end of if (TRUE == rxsFlag) */

    /* Fill-in the PTP message type */
    switch (ptpPortMode)
    {
        case IX_TIMESYNCACC_1588PTP_PORT_MASTER:
        {
            ptpMsgData->ptpMsgType = IX_TIMESYNCACC_1588PTP_MSGTYPE_DELAYREQ;
            break;
        } /* end of case IX_TIMESYNCACC_1588PTP_PORT_MASTER */
        case IX_TIMESYNCACC_1588PTP_PORT_SLAVE:
        {
            ptpMsgData->ptpMsgType = IX_TIMESYNCACC_1588PTP_MSGTYPE_SYNC;
            break;
        } /* end of case IX_TIMESYNCACC_1588PTP_PORT_SLAVE */
        case IX_TIMESYNCACC_1588PTP_PORT_ANYMODE:
        {
            ptpMsgData->ptpMsgType = IX_TIMESYNCACC_1588PTP_MSGTYPE_UNKNOWN;
            break;
        } /* end of case IX_TIMESYNCACC_1588PTP_PORT_ANYMODE */
        default:
        {
            /* This part of the code should never be reached */
#ifndef NDEBUG
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixTimeSyncAccPTPRxPoll(): Invalid Port Mode\n",
                0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
            return IX_TIMESYNCACC_FAILED;
        } /* end of case default */
    } /* end of switch (ptpPortMode) */

    /* Increment receive timestamp counter */
    ixTsStats.rxMsgs++;

    /* Allow next timestamp to be captured */
    ixTimeSyncAccControlPTPPortRxsFlagClear(ptpPort);

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccPTPRxPoll() function */

/*
 * Poll for the IEEE 1588 message/time stamp detect status on a particular 
 * PTP Port on the Transmit side.
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccPTPTxPoll(
    IxTimeSyncAcc1588PTPPort ptpPort,
    IxTimeSyncAccPtpMsgData  *ptpMsgData)
{
    /* Local variables */
    BOOL txsFlag = FALSE;
    IxTimeSyncAcc1588PTPPortMode ptpPortMode = 
                                 IX_TIMESYNCACC_1588PTP_PORT_SLAVE;

    /* Verify the parameters for proper values */
    if ((IX_TIMESYNCACC_NPE_1588PORT_INVALID <= ptpPort) ||
        ((IxTimeSyncAccPtpMsgData *)NULL == ptpMsgData))
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of if ((IX_TIMESYNCACC_NPE_1588PORT_INVALID <= ptpPort) ||
                    ((IxTimeSyncAccPtpMsgData *)NULL == ptpMsgData)) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Check NPE fused-out status */
    IXP400_TIMESYNCACC_NPE_FUSED_STATUS_CHECK(ptpPort);

    /* Get the Mode of the PTP Port */
    ptpPortMode = ixTimeSyncAccPTPPortModeGet(ptpPort);

    /* Is the Port Mode is ANY mode OR the transmit timestamp available? */
    txsFlag = ixTimeSyncAccControlPTPPortTxsFlagGet(ptpPort);

    if ((IX_TIMESYNCACC_1588PTP_PORT_ANYMODE == ptpPortMode) ||
       (TRUE == txsFlag))
    {
        /* Fetch the transmit timestamp */
        ixTimeSyncAccPTPPortTransmitSnapshotGet(ptpPort, 
            &ptpMsgData->ptpTimeStamp.timeValueLowWord,
            &ptpMsgData->ptpTimeStamp.timeValueHighWord);

        /*
         * Fill the UUID and Seq# with invalid values (zeros) 
         * since they are not relevant for transmit timestamp 
         */
        ptpMsgData->ptpUuid.uuidValueLowWord = 0;
        ptpMsgData->ptpUuid.uuidValueHighHalfword = 0;
        ptpMsgData->ptpSequenceNumber = 0;
    }
    /* else of if ((IX_TIMESYNCACC_1588PTP_PORT_ANYMODE == ptpPortMode) ||
     *             (TRUE == txsFlag)) 
     */
    else
    {
        return IX_TIMESYNCACC_NOTIMESTAMP;
    } /* end of if ((IX_TIMESYNCACC_1588PTP_PORT_ANYMODE == ptpPortMode) ||
                    (TRUE == txsFlag)) */

    /* Fill-in the PTP message type */
    switch (ptpPortMode)
    {
        case IX_TIMESYNCACC_1588PTP_PORT_MASTER:
        {
            ptpMsgData->ptpMsgType = IX_TIMESYNCACC_1588PTP_MSGTYPE_SYNC;
            break;
        } /* end of case IX_TIMESYNCACC_1588PTP_PORT_MASTER */
        case IX_TIMESYNCACC_1588PTP_PORT_SLAVE:
        {
            ptpMsgData->ptpMsgType = IX_TIMESYNCACC_1588PTP_MSGTYPE_DELAYREQ;
            break;
        } /* end of case IX_TIMESYNCACC_1588PTP_PORT_SLAVE */
        case IX_TIMESYNCACC_1588PTP_PORT_ANYMODE:
        {
            ptpMsgData->ptpMsgType = IX_TIMESYNCACC_1588PTP_MSGTYPE_UNKNOWN;
            break;
        } /* end of case IX_TIMESYNCACC_1588PTP_PORT_ANYMODE */
        default:
        {
            /* This part of the code should never be reached */
#ifndef NDEBUG
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
                "ixTimeSyncAccPTPTxPoll(): Invalid Port Mode\n",
                0,0,0,0,0,0);
#endif /* end of #ifndef NDEBUG */
            return IX_TIMESYNCACC_FAILED;
        } /* end of case default */
    } /* end of switch (ptpPortMode) */

    /* Increment transmit timestamp counter */
    ixTsStats.txMsgs++;

    /* Allow next timestamp to be captured */
    ixTimeSyncAccControlPTPPortTxsFlagClear(ptpPort);

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccPTPTxPoll() function */

/* Set the System Time in the IEEE 1588 hardware assist block */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccSystemTimeSet(IxTimeSyncAccTimeValue systemTime)
{
    /* Local variables */
    UINT32 oldFsv = 0;

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /*
     * Secure Mutex so that Change in Frequency Scaling Value
     * else where affect the System Time
     */
    IXP400_TIMESYNCACC_MUTEX_LOCK(ixTsSysTimeMutex);

    /* Retrieve old Frequency Scaling Value */
    ixTimeSyncAccAddendFsvGet(&oldFsv);

    /*
     * Set the Frequency Scaling Value to zero (0) so that
     * System Time doesn't get increment while it is being
     * written into low and high registers 
     */
    ixTimeSyncAccAddendFsvSet(0);

    /* Update System Time with user specified values */
    ixTimeSyncAccSystemTimeSnapshotSet(systemTime.timeValueLowWord,
                                       systemTime.timeValueHighWord);

    /*
     * Let the hardware assist to re-evaluate the target time reached 
     * condition based on the new system time
     */
    ixTimeSyncAccEventTtmFlagClear();

    /*
     * Restore old Frequency Scaling Value so that System Time
     * can be incremented 
     */
    ixTimeSyncAccAddendFsvSet(oldFsv);

    /* Unlock the mutex obtained at the beginning */
    IXP400_TIMESYNCACC_MUTEX_UNLOCK(ixTsSysTimeMutex);

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccSystemTimeSet() function */

/* Get the System Time from the IEEE 1588 hardware assist block */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccSystemTimeGet(IxTimeSyncAccTimeValue *systemTime)
{
    /* Verify the parameter */
    if ((IxTimeSyncAccTimeValue *)NULL == systemTime)
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of if ((IxTimeSyncAccTimeValue *)NULL == systemTime) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Fetch System Time */
    ixTimeSyncAccSystemTimeSnapshotGet(&systemTime->timeValueLowWord,
                                       &systemTime->timeValueHighWord);

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccSystemTimeGet() function */

/*
 * Set the Tick Rate (Frequency Scaling Value) in the IEEE 1588
 * hardware assist block
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTickRateSet(UINT32 tickRate)
{
    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /*
     * Secure Mutex to avoid update of Frequency Scaling Value
     * and the System Time, happening at the same time 
     */
    IXP400_TIMESYNCACC_MUTEX_LOCK(ixTsSysTimeMutex);

    /* Update the Frequency Scaling Value */
    ixTimeSyncAccAddendFsvSet(tickRate);

    /* Release the mutex  obtained at the beginning */
    IXP400_TIMESYNCACC_MUTEX_UNLOCK(ixTsSysTimeMutex);

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccTickRateSet() function */

/*
 * Get the Tick Rate (Frequency Scaling Value) from the IEEE 1588
 * hardware assist block
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTickRateGet(UINT32 *tickRate)
{
    /* Verify the parameter */
    if ((UINT32 *)NULL == tickRate)
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of if ((UINT32 *)NULL == tickRate) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Retrieve Current Frequency Scaling Value */
    ixTimeSyncAccAddendFsvGet(tickRate);

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccTickRateGet() function */

/*
 * Enable the interrupt to verify the condition where the System Time 
 * greater or equal to the Target Time in the IEEE 1588 hardware assist 
 * block.
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTargetTimeInterruptEnable(
    IxTimeSyncAccTargetTimeCallback targetTimeCallback)
{
    /* Verify the parameter */
    if ((IxTimeSyncAccTargetTimeCallback)NULL == targetTimeCallback)
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of if ((IxTimeSyncAccTargetTimeCallback)NULL == 
                     targetTimeCallback) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Register the Callback */
    ixTsTargetTimeCallback = targetTimeCallback;

    /* Set target time interrupt mask */
    ixTimeSyncAccControlTtmInterruptMaskSet();

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccTargetTimeInterruptEnable() function */

/*
 * Disable the interrupt to verify the condition that the System Time 
 * greater or equal to the Target Time in the IEEE 1588 hardware assist 
 * block.
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTargetTimeInterruptDisable(void)
{
    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Clear target time interrupt mask */
    ixTimeSyncAccControlTtmInterruptMaskClear();

    /* Unregister the Callback */
    ixTsTargetTimeCallback = (IxTimeSyncAccTargetTimeCallback) NULL;

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccTargetTimeInterruptDisable() function */

/*
 * Poll to verify the condition where the System Time greater or equal 
 * to the Target Time in the IEEE 1588 hardware assist block.
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTargetTimePoll(
    BOOL *ttmPollFlag,
    IxTimeSyncAccTimeValue *targetTime)
{
    /* Verify the parameters */
    if (((BOOL *)NULL == ttmPollFlag) ||
        ((IxTimeSyncAccTimeValue *)NULL == targetTime))
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of if (((BOOL *)NULL == ttmPollFlag) ||
                    ((IxTimeSyncAccTimeValue)NULL == targetTime)) */

    /* Is interrupt mode of processing is enabled? */
    if ((IxTimeSyncAccTargetTimeCallback) NULL != ixTsTargetTimeCallback)
    {
        return IX_TIMESYNCACC_INTERRUPTMODEINUSE;
    } /* if ((IxTimeSyncAccTargetTimeCallback) NULL != ixTsTargetTimeCallback) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Has the System Time reached or exceeded Target Time? */
    *ttmPollFlag = ixTimeSyncAccEventTtmFlagGet();
    if (FALSE == *ttmPollFlag)
    {
        /* Target Time not to be returned yet */
        targetTime->timeValueLowWord = 0;
        targetTime->timeValueHighWord = 0;

        return IX_TIMESYNCACC_SUCCESS;
    } /* if (FALSE == *ttmPollFlag) */

    /* Fetch Target Time */
    ixTimeSyncAccTargetTimeSnapshotGet(&targetTime->timeValueLowWord,
                                  &targetTime->timeValueHighWord);

    /* Clear the target time reached condition (ttipend bit) */
    ixTimeSyncAccEventTtmFlagClear();

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccTargetTimePoll() function */

/* Set the Target Time in the IEEE 1588 hardware assist block */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTargetTimeSet(IxTimeSyncAccTimeValue targetTime)
{
    /* Local variables */
    BOOL oldTtmMask = FALSE;

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Retrieve existing target time interrupt mask value */
    oldTtmMask = ixTimeSyncAccControlTtmInterruptMaskGet();

    /* Clear the target time interrupt mask value to prevent false 
     * interrupts from being asserted due to the increments of the 
     * values in the System Time low and high registers (i.e., the
     * target time reached or exceeded interrupt does not get
     * generated
     */
    ixTimeSyncAccControlTtmInterruptMaskClear();

    /* Update Target Time with user specified values */
    ixTimeSyncAccTargetTimeSnapshotSet(targetTime.timeValueLowWord,
                                       targetTime.timeValueHighWord);

    /*
     * Let the hardware assist to re-evaluate the target time reached 
     * condition based on the new target time
     */
    ixTimeSyncAccEventTtmFlagClear();

    /* Restore the preserved target time interrupt mask value */
    if (TRUE == oldTtmMask)
    {
        ixTimeSyncAccControlTtmInterruptMaskSet();
    } /* end of if (TRUE == oldTtmMask) */

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccTargetTimeSet() function */

/* Get the Target Time in the IEEE 1588 hardware assist block */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccTargetTimeGet(IxTimeSyncAccTimeValue *targetTime)
{
    /* Verify the parameter */
    if ((IxTimeSyncAccTimeValue *)NULL == targetTime)
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of if ((IxTimeSyncAccTimeValue *)NULL == systemTime) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Fetch Target Time */
    ixTimeSyncAccTargetTimeSnapshotGet(&targetTime->timeValueLowWord,
                               &targetTime->timeValueHighWord);

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccTargetTimeGet() function */

/*
 * Enable the interrupt notification for the given mode of Auxiliary Time 
 * Stamp in the IEEE 1588 hardware assist block
 */ 
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccAuxTimeInterruptEnable(
    IxTimeSyncAccAuxMode auxMode,
    IxTimeSyncAccAuxTimeCallback auxTimeCallback)
{
    /* Verify the parameters */
    if ((IX_TIMESYNCACC_AUXMODE_INVALID <= auxMode) ||
        ((IxTimeSyncAccAuxTimeCallback)NULL == auxTimeCallback))
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* if ((IX_TIMESYNCACC_AUXMODE_INVALID <= auxMode) ||
             ((IxTimeSyncAccAuxTimeCallback)NULL == auxTimeCallback)) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Register the Callback and SET the amm/asm bits on */
    if (IX_TIMESYNCACC_AUXMODE_MASTER == auxMode)
    {
        ixTsAuxMasterTimeCallback = auxTimeCallback;
        ixTimeSyncAccControlAmmsInterruptMaskSet();

    } /* else of if (IX_TIMESYNCACC_AUXMODE_MASTER == auxMode) */
    else
    {
        ixTsAuxSlaveTimeCallback = auxTimeCallback;
        ixTimeSyncAccControlAsmsInterruptMaskSet();
    } /* end of if (IX_TIMESYNCACC_AUXMODE_MASTER == auxMode) */

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccAuxTimeInterruptEnable() function */

/*
 * Disable the interrupt for the indicated mode of Auxiliary Time Stamp
 * in the IEEE 1588 hardware assist block
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccAuxTimeInterruptDisable(IxTimeSyncAccAuxMode auxMode)
{
    /* Verify the parameters */
    if (IX_TIMESYNCACC_AUXMODE_INVALID <= auxMode)
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* if (IX_TIMESYNCACC_AUXMODE_INVALID <= auxMode) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Unregister the Callback and CLEAR the amm/asm bits on */
    if (IX_TIMESYNCACC_AUXMODE_MASTER == auxMode)
    {
        ixTimeSyncAccControlAmmsInterruptMaskClear();
        ixTsAuxMasterTimeCallback = (IxTimeSyncAccAuxTimeCallback) NULL;
    }
    /* else of if (IX_TIMESYNCACC_AUXMODE_MASTER == auxMode) */
    else
    {
        ixTimeSyncAccControlAsmsInterruptMaskClear();
        ixTsAuxSlaveTimeCallback = (IxTimeSyncAccAuxTimeCallback) NULL;
    } /* end of if (IX_TIMESYNCACC_AUXMODE_MASTER == auxMode) */

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccAuxTimeInterruptDisable() function */

/*
 * Poll for the Auxiliary Time Stamp captured for the mode indicated 
 * (Master or Slave)
 */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccAuxTimePoll(IxTimeSyncAccAuxMode auxMode,
    BOOL *auxPollFlag,
    IxTimeSyncAccTimeValue *auxTime)
{
    /* Local variables */
    BOOL ammsFlag = FALSE;
    BOOL asmsFlag = FALSE;

    /* Verify the parameters */
    if (((BOOL *)NULL == auxPollFlag) ||
        (IX_TIMESYNCACC_AUXMODE_INVALID <= auxMode) ||
        ((IxTimeSyncAccTimeValue *)NULL == auxTime))
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of if (((BOOL *)NULL == auxPollFlag) ||
                    (IX_TIMESYNCACC_AUXMODE_INVALID <= auxMode) ||
                    ((IxTimeSyncAccTimeValue *)NULL == auxTime)) */

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Fetch Auxiliary Master/Slave Mode Snapshot */
    if (IX_TIMESYNCACC_AUXMODE_MASTER == auxMode)
    {
        /* Is interrupt mode of processing is enabled? */
        if ((IxTimeSyncAccAuxTimeCallback) NULL != ixTsAuxMasterTimeCallback)
        {
            return IX_TIMESYNCACC_INTERRUPTMODEINUSE;
        } /* end of if ((IxTimeSyncAccAuxTimeCallback) NULL != 
                        ixTsAuxMasterTimeCallback) */

        /* Is the Auxiliary Master Mode Snapshot available? */
        ammsFlag = ixTimeSyncAccEventAmmsFlagGet();
        if (FALSE == ammsFlag)
        {
            *auxPollFlag = FALSE;
            auxTime->timeValueLowWord =0;
            auxTime->timeValueHighWord = 0;
            return IX_TIMESYNCACC_SUCCESS;
        } /* end of if (FALSE == ammsFlag) */

        /* Fetch Auxiliary Master Snapshot */
        ixTimeSyncAccAuxMasterModeSnapshotGet(&auxTime->timeValueLowWord,
                                              &auxTime->timeValueHighWord);
        *auxPollFlag = TRUE;

        /* Clear the snapshot availability condition */
        ixTimeSyncAccEventAmmsFlagClear();
    }
    /* else of if (IX_TIMESYNCACC_AUXMODE_MASTER == auxMode) */
    else
    {
        /* Is interrupt mode of processing is enabled? */
        if ((IxTimeSyncAccAuxTimeCallback) NULL != ixTsAuxSlaveTimeCallback)
        {
            return IX_TIMESYNCACC_INTERRUPTMODEINUSE;
        } /* end of if ((IxTimeSyncAccAuxTimeCallback) NULL != 
                        ixTsAuxSlaveTimeCallback) */

        /* Is the Auxiliary Slave Mode Snapshot available? */
        asmsFlag = ixTimeSyncAccEventAsmsFlagGet();
        if (FALSE == asmsFlag)
        {
            *auxPollFlag = FALSE;
            auxTime->timeValueLowWord =0;
            auxTime->timeValueHighWord = 0;
            return IX_TIMESYNCACC_SUCCESS;
        } /* end of if (FALSE == asmsFlag) */

        /* Fetch Auxiliary Slave Snapshot */
        ixTimeSyncAccAuxSlaveModeSnapshotGet(&auxTime->timeValueLowWord,
                                             &auxTime->timeValueHighWord);
        *auxPollFlag = TRUE;

        /* Clear the snapshot availability condition */
        ixTimeSyncAccEventAsmsFlagClear();
    } /* end of if (IX_TIMESYNCACC_AUXMODE_MASTER == auxMode) */

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccAuxTimePoll() function */

/* Reset the IEEE 1588 hardware assist block */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccReset(void)
{
    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Reset Hardware Assist */
    ixTimeSyncAccControlReset();

    /* Clear Stats */
    ixTsStats.rxMsgs = ixTsStats.txMsgs = 0;

    /* Unregister any Callback Routines */
    ixTsTargetTimeCallback    = (IxTimeSyncAccTargetTimeCallback) NULL;
    ixTsAuxMasterTimeCallback = (IxTimeSyncAccAuxTimeCallback) NULL;
    ixTsAuxSlaveTimeCallback  = (IxTimeSyncAccAuxTimeCallback) NULL;

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccReset() function */

/* Return the IxTimeSyncAcc Statistics */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccStatsGet(IxTimeSyncAccStats *timeSyncStats)
{
    /* Verify the parameter */
    if ((IxTimeSyncAccStats *) NULL == timeSyncStats)
    {
        return IX_TIMESYNCACC_INVALIDPARAM;
    } /* end of ((IxTimeSyncAccStats *) NULL == timeSyncStats) */

    /* Return Stats */
    timeSyncStats->rxMsgs = ixTsStats.rxMsgs; 
    timeSyncStats->txMsgs = ixTsStats.txMsgs; 

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccStatsGet() function */

/* Reset Time Sync statistics */
PUBLIC void
ixTimeSyncAccStatsReset(void)
{
    /* Clear Stats */
    ixTsStats.rxMsgs = ixTsStats.txMsgs = 0;

    return;
} /* end of ixTimeSyncAccStatsReset() function */

/* Display the Time Sync current status */
PUBLIC IxTimeSyncAccStatus
ixTimeSyncAccShow(void)
{
    /* Local Varaiables */
    UINT32 regValue = 0;
    UINT32 regLowValue = 0;
    UINT32 regHighValue = 0;
    UINT16 seqId = 0;
    UINT32 uuIdLow = 0;
    UINT16 uuIdHigh = 0;
    BOOL   bitSet   = FALSE;
    UINT32 ptpPortNum = 0;

    /* Initialised before? */
    IXP400_TIMESYNCACC_INIT_CHECK();

    /* Dump Block Level Status */

    /* System Time registers contents */
    regLowValue = 0;
    regHighValue = 0;
    ixTimeSyncAccSystemTimeSnapshotGet(&regLowValue, &regHighValue);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "System Test (Low:High): 0x%08x : 0x%08x\n",
        regLowValue, regHighValue,0,0,0,0);

    /* Frequency Scaling Value */
    regValue = 0;
    ixTimeSyncAccAddendFsvGet(&regValue);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Frequency Scaling Value: 0x%08x\n",
        regValue,0,0,0,0,0);

    /* Target time reached/exceeded interrupt mask value */
    bitSet = ixTimeSyncAccControlTtmInterruptMaskGet();
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Target Time Reached/Exceeded Interrupt Mask: %s\n",
        (INT32)((TRUE == bitSet) ? "ENABLED":"DISABLED"),0,0,0,0,0);

    /* Target time reached/exceeded event flag value */
    bitSet = ixTimeSyncAccEventTtmFlagGet();
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Target Time Reached/Exceeded Event Flag: %s\n",
        (INT32)((TRUE == bitSet) ? "SET":"CLEAR"),0,0,0,0,0);

    /* Target Time registers contents  */
    regLowValue = 0;
    regHighValue = 0;
    ixTimeSyncAccTargetTimeSnapshotGet(&regLowValue, &regHighValue);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Target Time (Low:High): 0x%08x : 0x%08x\n",
        regLowValue, regHighValue,0,0,0,0);

    /* Auxiliary Master Mode Snapshot interrupt mask value */
    bitSet = ixTimeSyncAccControlAmmsInterruptMaskGet();
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Auxiliary Master Mode Snapshot Interrupt Mask: %s\n",
        (INT32)((TRUE == bitSet) ? "ENABLED":"DISABLED"),0,0,0,0,0);

    /* Auxiliary Master Mode Snapshot event flag value */
    bitSet = ixTimeSyncAccEventAmmsFlagGet();
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Auxiliary Master Mode Snapshot Event Flag: %s\n",
        (INT32)((TRUE == bitSet) ? "SET":"CLEAR"),0,0,0,0,0);

    /* Auxiliary Master Snapshot registers */
    regLowValue = 0;
    regHighValue = 0;
    ixTimeSyncAccAuxMasterModeSnapshotGet(&regLowValue, &regHighValue);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
       "Auxiliary Master Mode Snapshot (Low:High): 0x%08x : 0x%08x\n",
        regLowValue, regHighValue,0,0,0,0);

    /* Auxiliary Slave Mode Snapshot interrupt mask value */
    bitSet = ixTimeSyncAccControlAsmsInterruptMaskGet();
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Auxiliary Slave Mode Snapshot Interrupt Mask: %s\n",
        (INT32)((TRUE == bitSet) ? "ENABLED":"DISABLED"),0,0,0,0,0);

    /* Auxiliary Slave Mode Snapshot event flag value */
    bitSet = ixTimeSyncAccEventAsmsFlagGet();
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Auxiliary Slave Mode Snapshot Event Flag: %s\n",
        (INT32)((TRUE == bitSet) ? "SET":"CLEAR"),0,0,0,0,0);
    
    /* Auxiliary Slave Snapshot registers */
    regLowValue = 0;
    regHighValue = 0;
    ixTimeSyncAccAuxSlaveModeSnapshotGet(&regLowValue, &regHighValue);
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Auxiliary Slave Mode Snapshot (Low:High) : 0x%08x : 0x%08x\n",
        regLowValue, regHighValue,0,0,0,0);

    /* Dump Port Level Status */
    for (ptpPortNum = 0;
         ptpPortNum < IXP400_TIMESYNCACC_MAX_1588PTP_PORT;
         ptpPortNum++)
    {
        /* Display the port number */
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "PTP Port #: %u %s\n",
            ptpPortNum,
            (INT32)((TRUE == ixTsNpeEnabled[ptpPortNum]) ? "***FUSED-OUT***":""),
            0,0,0,0);

        /* Get the Master Mode and Timestamp All PTP messages status */
        bitSet = ixTimeSyncAccControlPTPPortMasterModeGet(ptpPortNum);
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "\tMaster Mode: %s\n",
            (INT32)((TRUE == bitSet) ? "SET":"CLEAR"),0,0,0,0,0);

        bitSet = ixTimeSyncAccControlPTPPortPTPMsgTimestampGet(ptpPortNum);
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "\tTimestamp All Mode: %s\n",
            (INT32)((TRUE == bitSet) ? "SET":"CLEAR"),0,0,0,0,0);

        /* Receive Timestamp Event Flag */
        bitSet = ixTimeSyncAccControlPTPPortRxsFlagGet(ptpPortNum);
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "\tReceive Timestamp Event Flag: %s\n",
            (INT32)((TRUE == bitSet) ? "SET":"CLEAR"),0,0,0,0,0);

        /* Receive timestamp registers */
        regLowValue = 0;
        regHighValue = 0;
        ixTimeSyncAccPTPPortReceiveSnapshotGet(ptpPortNum,
            &regLowValue, &regHighValue);
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "\tReceive Timestamp (Low:High): 0x%08x : 0x%08x\n",
            regLowValue,regHighValue,0,0,0,0);

        /* UUID and Seq# */
        ixTimeSyncAccPTPMsgUuidSeqIdGet(ptpPortNum, &uuIdLow, &uuIdHigh, &seqId);
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "\tUUID (Low: High (16-Bits)): 0x%08x : 0x%08x \n"
            "Seq# (16Bits Only): 0x%08x\n",
            uuIdLow, uuIdHigh, seqId,0,0,0);

        /* Transmit Timestamp Event Flag */
        bitSet = ixTimeSyncAccControlPTPPortTxsFlagGet(ptpPortNum);
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "\tTransmit Timestamp Event Flag: %s\n",
            (INT32)((TRUE == bitSet) ? "SET":"CLEAR"),0,0,0,0,0);

        /* Transmit timestamp registers */
        regLowValue = 0;
        regHighValue = 0;
        ixTimeSyncAccPTPPortTransmitSnapshotGet(ptpPortNum,
            &regLowValue,&regHighValue);
        ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
            "\tTransmit Timestamp (Low:High): 0x%08x : 0x%08x\n",
            regLowValue,regHighValue,0,0,0,0);
    } /* end of for (ptpPortNum = 0;
                     ptpPortNum < IXP400_TIMESYNCACC_MAX_1588PTP_PORT;
                     ptpPortNum++) */

    /* Stats */
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Receive Timestamps Count: %u\n"
        "Transmit Timestamp Count: %u\n",
        ixTsStats.rxMsgs, ixTsStats.txMsgs,0,0,0,0);

    /* Callback Routine Addresses */
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Target Time Callback: %p\n"
        "Auxiliary Master Mode Snapshot Callback: %p\n"
        "Auxiliary Slave Mode Snapshot Callback: %p\n",
        (UINT32)ixTsTargetTimeCallback,
        (UINT32)ixTsAuxMasterTimeCallback,
        (UINT32)ixTsAuxSlaveTimeCallback,
        0,0,0);

    return IX_TIMESYNCACC_SUCCESS;
} /* end of ixTimeSyncAccShow() function */

/* This function can be used to uninitilaze the timesyncAcc component 
 * from the client application. It performs the following tasks as part
 * of unintialization:
 *  => Unmaps the memory of Block and Register level registers 
 *     starting base address.
 *  => Unbinds the timesyncAcc IRQ
 *  => Resets the fused out state of all NPEs to false.
 *  => Resets the 1588 Hardware Assist block enabled flag to false and 
 *     also destroys the system time mutex.
 */
PUBLIC IxTimeSyncAccStatus 
ixTimeSyncAccUnInit(void) 
{    
    UINT32 lockKey;
    UINT32 blRegsVirtualBaseAddr = (UINT32) NULL;
    UINT32 plRegsVirtualBaseAddr = (UINT32) NULL;
    IxTimeSyncAccStatus status = IX_TIMESYNCACC_SUCCESS;

    if (TRUE == ixTs1588HardwareAssistEnabled)
    {
        /* Unbind the TimeSyncAcc IRQ */
        lockKey = ixOsalIrqLock();
        if (IX_SUCCESS !=
            ixOsalIrqUnbind ((UINT32) IRQ_IXP400_INTC_TSYNC))
        {
            ixOsalLog(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
            "ixTimeSyncAccUnInit(): "\
            "Can't unbind the TimeSyncAcc ISR to IRQ_IXP400_INTC_TSYNC!!!\n",
            0,0,0,0,0,0);
            status = IX_TIMESYNCACC_FAILED;
        }
        ixOsalIrqUnlock(lockKey);

        /* Get the Virtual address of Block and Register level registers */
	blRegsVirtualBaseAddr = 
		IX_OSAL_MMAP_PHYS_TO_VIRT(IXP400_TIMESYNCACC_BLREGS_BASEADDR);
        if ((UINT32)NULL == blRegsVirtualBaseAddr)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
            "ixTimeSyncAccUnInit(): "\
            "Physical to Virtual address conversion for Block level register base failed!!!\n",
            0,0,0,0,0,0);
            status = IX_TIMESYNCACC_FAILED;
        }
	plRegsVirtualBaseAddr = 
		IX_OSAL_MMAP_PHYS_TO_VIRT(IXP400_TIMESYNCACC_PLREGS_BASEADDR);
        if ((UINT32)NULL == plRegsVirtualBaseAddr)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDERR,
            "ixTimeSyncAccUnInit(): "\
            "Physical to Virtual address conversion for Register level register base failed!!!\n",
            0,0,0,0,0,0);
            status = IX_TIMESYNCACC_FAILED;
        }
	
	/* Unmap the memory for Block and Register level registers */
        IX_OSAL_MEM_UNMAP(blRegsVirtualBaseAddr);
        IX_OSAL_MEM_UNMAP(plRegsVirtualBaseAddr);

        /* Reset the Fused-Out States of NPEs to False */
        if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
            ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA))
        {
            ixTsNpeEnabled[IXP400_TIMESYNCACC_NPEA] = FALSE;
        }
        if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
            ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB))
        {
            ixTsNpeEnabled[IXP400_TIMESYNCACC_NPEB] = FALSE;
        }
        if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
            ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC))
        {
            ixTsNpeEnabled[IXP400_TIMESYNCACC_NPEC] = FALSE;
        }

        /* Destroy the system time mutex */
        ixOsalMutexDestroy(&ixTsSysTimeMutex);
	
	/* Reset the 1588 Hardware Assist block enabled flag */
        ixTs1588HardwareAssistEnabled = FALSE;
    }
    else
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixTimeSyncAcc Component Not Intialized!!!\n", 0,0,0,0,0,0);

        return IX_TIMESYNCACC_FAILED;
    }
    
    return status;
} /* end of ixTimeSyncAccUnInit() function */

#endif /* __ixp46X */

