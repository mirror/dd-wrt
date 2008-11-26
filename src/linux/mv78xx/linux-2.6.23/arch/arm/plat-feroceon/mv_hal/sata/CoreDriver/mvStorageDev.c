/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
/*******************************************************************************
* mvStorageDev.c - C File for implementation of the core driver.
*
* DESCRIPTION:
*       None.
*
* DEPENDENCIES:
*   mvOs.h
*   mvSata.h.
*   mvStorageDev.h
*   mvRegs.h
*
*******************************************************************************/
#include "mvOsS.h"
#include "mvSata.h"
#include "mvStorageDev.h"
#include "mvRegs.h"

/* Defines */

/* functions for internal driver use */
MV_BOOLEAN waitWhileStorageDevIsBusy(MV_SATA_ADAPTER* pAdapter,
                                     MV_BUS_ADDR_T ioBaseAddr,
                                     MV_U32 eDmaRegsOffset, MV_U32 loops,
                                     MV_U32 delayParam);
MV_BOOLEAN waitForDRQToClear(MV_SATA_ADAPTER* pAdapter,
                             MV_BUS_ADDR_T ioBaseAddr,
                             MV_U32 eDmaRegsOffset, MV_U32 loops,
                             MV_U32 delayParam);

void enableStorageDevInterrupt (MV_SATA_CHANNEL *pSataChannel);
void disableStorageDevInterrupt(MV_SATA_CHANNEL *pSataChannel);
static MV_BOOLEAN isStorageDevReadyForPIO(MV_SATA_CHANNEL *pSataChannel);
void dumpAtaDeviceRegisters(MV_SATA_ADAPTER *pAdapter,
                            MV_U8 channelIndex, MV_BOOLEAN isEXT,
                            MV_STORAGE_DEVICE_REGISTERS *pRegisters);
MV_BOOLEAN _doSoftReset(MV_SATA_CHANNEL *pSataChannel);
extern void _setActivePMPort(MV_SATA_CHANNEL *pSataChannel, MV_U8 PMPort);
extern void disableSaDevInterrupts(MV_SATA_ADAPTER *pAdapter, MV_U8 channelIndex);

MV_BOOLEAN  _PMAccessReg(MV_SATA_ADAPTER *pAdapter, MV_U8 channelIndex,
                         MV_U8 PMPort, MV_U8 PMReg, MV_U32 *pValue,
                         MV_STORAGE_DEVICE_REGISTERS *registerStruct,
                         MV_BOOLEAN isRead);

MV_BOOLEAN executeNonUDMACommand(MV_SATA_ADAPTER *pAdapter,
                                 MV_U8 channelIndex,
                                 MV_U8  PMPort,
                                 MV_NON_UDMA_PROTOCOL protocolType,
                                 MV_BOOLEAN  isEXT,
                                 MV_U16_PTR bufPtr, MV_U32 count,
                                 MV_U16 features,
                                 MV_U16 sectorCount,
                                 MV_U16 lbaLow, MV_U16 lbaMid,
                                 MV_U16 lbaHigh, MV_U8 device,
                                 MV_U8 command);

MV_BOOLEAN waitWhileStorageDevIsBusy_88SX60X1(MV_SATA_ADAPTER* pAdapter,
					      MV_BUS_ADDR_T ioBaseAddr,
					      MV_U32 eDmaRegsOffset, MV_U8 channelIndex,
					      MV_U32 loops,
					      MV_U32 delayParam);

MV_BOOLEAN waitForDRQ(MV_SATA_ADAPTER* pAdapter,
                      MV_BUS_ADDR_T ioBaseAddr,
                      MV_U32 eDmaRegsOffset, MV_U32 loops,
                      MV_U32 delayParam);

void _startSoftResetDevice(MV_SATA_CHANNEL *pSataChannel);
MV_BOOLEAN _isDeviceBsyBitOff(MV_SATA_CHANNEL *pSataChannel);
/*******************************************************************************
* waitWhileStorageDevIsBusy - Wait for the storage device to be released from
*                             busy state.
*
* DESCRIPTION:
*   The busy bit is set to one to indicate that the storage device is busy. The
*   busy bit shall be set to one by the device only when one of the following
*   events occurs:
*
*   1) after either the negation of RESET- or the setting of the SRST bit to one
*      in the Device Control register.
*   2) after writing the Command register if the DRQ bit is not set to one.
*   3) between blocks of a data transfer during PIO data-in commands before the
*      DRQ bit is cleared to zero.
*   4) after the transfer of a data block during PIO data-out commands before
*      the DRQ bit is cleared to zero.
*   5) during the data transfer of DMA commands either the BSY bit, the DRQ bit,
*      or both shall be set to one.
*   6) after the command packet is received during the execution of a PACKET
*      command.
*
* INPUT:
*     ioBaseAddr     - The PCI I/O window base address of the adapter.
*     eDmaRegsOffset - The EDMA register's offset of the relevant SATA channel.
*     loops          - max number of times to pool the status register.
*     delay          - number of u seconds to wait each loop.
*
* RETURN:
*     MV_TRUE if the device is released from busy state, MV_FALSE otherwise.
* COMMENTS:
*     None.
*
*******************************************************************************/
MV_BOOLEAN waitWhileStorageDevIsBusy(MV_SATA_ADAPTER* pAdapter,
                                     MV_BUS_ADDR_T ioBaseAddr,
                                     MV_U32 eDmaRegsOffset, MV_U32 loops,
                                     MV_U32 delayParam)
{
    MV_U8   ATAstatus = 0;
    MV_U32  i;

    for (i = 0;i < loops; i++)
    {
        ATAstatus = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                     MV_ATA_DEVICE_STATUS_REG_OFFSET);
        if ((ATAstatus & MV_ATA_BUSY_STATUS) == 0)
        {

            if ((ATAstatus & MV_ATA_ERROR_STATUS) == 0)
            {
                mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, "waitWhileStorageDevIsBusy: %d loops *"
                         "%d usecs\n", i, delayParam);
                return MV_TRUE;
            }
            else
            {
                mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, "waitWhileStorageDevIsBusy<FAILED>: Device ERROR"
                         " Status: 0x%02x\n", ATAstatus);
                return MV_FALSE;
            }
        }
        mvMicroSecondsDelay(pAdapter, delayParam);
    }
    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "waitWhileStorageDevIsBusy<FAILED>: Time out - Device ERROR"
             " Status: 0x%02x. loops %d, delay %d\n", ATAstatus, loops, delayParam);

    return MV_FALSE;
}


MV_BOOLEAN waitWhileStorageDevIsBusy_88SX60X1(MV_SATA_ADAPTER* pAdapter,
                                                                    MV_BUS_ADDR_T ioBaseAddr,
                                                                    MV_U32 eDmaRegsOffset, MV_U8 channelIndex,
                                                                    MV_U32 loops,
                                                                    MV_U32 delayParam)
{
    MV_U8   ATAstatus = 0;
    MV_U32  i,intReg;
    MV_U8   sataUnit = channelIndex >> 2, portNum = (channelIndex & 0x3);


    for (i = 0;i < loops; i++)
    {
        intReg = MV_REG_READ_DWORD (ioBaseAddr, MV_SATAHC_REGS_BASE_OFFSET(sataUnit) +
                                    MV_SATAHC_INTERRUPT_CAUSE_REG_OFFSET);

        if (intReg & (1 << (8 + portNum)))
        {
            ATAstatus = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                         MV_ATA_DEVICE_STATUS_REG_OFFSET);
            MV_REG_WRITE_DWORD (ioBaseAddr, MV_SATAHC_REGS_BASE_OFFSET(sataUnit) +
                                MV_SATAHC_INTERRUPT_CAUSE_REG_OFFSET,
                                ~(1 << (8 + portNum)));
            if ((ATAstatus & MV_ATA_ERROR_STATUS) == 0)
            {
                mvLogMsg(MV_CORE_DRIVER_LOG_ID,MV_DEBUG, "waitWhileStorageDevIsBusy: %d loops *"
                         "%d usecs\n", i, delayParam);
                return MV_TRUE;
            }
            else
            {
                mvLogMsg(MV_CORE_DRIVER_LOG_ID,MV_DEBUG_ERROR, "waitWhileStorageDevIsBusy<FAILED>: Device ERROR"
                         " Status: 0x%02x\n", ATAstatus);
                return MV_FALSE;
            }
        }
        mvMicroSecondsDelay(pAdapter, delayParam);
    }
    ATAstatus = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                 MV_ATA_DEVICE_STATUS_REG_OFFSET);
 
    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, "waitWhileStorageDevIsBusy<FAILED>: Time out - Device ERROR"
             " Status: 0x%02x. loops %d, delay %d\n", ATAstatus, loops, delayParam);
    return MV_FALSE;
}

MV_BOOLEAN waitForDRQ(MV_SATA_ADAPTER* pAdapter,
                      MV_BUS_ADDR_T ioBaseAddr,
                      MV_U32 eDmaRegsOffset, MV_U32 loops,
                      MV_U32 delayParam)
{
    MV_U8   ATAstatus = 0;
    MV_U32  i;

    for (i = 0;i < loops; i++)
    {
        ATAstatus = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                     MV_ATA_DEVICE_STATUS_REG_OFFSET);
        if ((ATAstatus & MV_ATA_BUSY_STATUS) == 0)
        {
            if (ATAstatus & MV_ATA_DATA_REQUEST_STATUS)
            {
                mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, "waitWhileStorageDevIsBusy: %d loops *"
                         "%d usecs\n", i, delayParam);
                return MV_TRUE;
            }
        }
        mvMicroSecondsDelay(pAdapter, delayParam);
    }
    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "waitWhileStorageDevIsBusy<FAILED>: Time out - Device ERROR"
             " Status: 0x%02x. loops %d, delay %d\n", ATAstatus, loops, delayParam);

    return MV_FALSE;
}
/*******************************************************************************
* enableStorageDevInterrupt - Enable the storage device to be able to issue
*                       interrupt.
*
* DESCRIPTION:
*   Enable the connected storage device to the given channel to assert INTRQ by
*   clearing nIEN bit in the Device Control register.
*
* INPUT:
*   pSataChannel - Pointer to the Sata channel data structure.
*
* RETURN:
*   None.
*
* COMMENTS:
*   this function also clears the SRST bit in the Device Control register.
*
*******************************************************************************/
void enableStorageDevInterrupt(MV_SATA_CHANNEL *pSataChannel)
{

    MV_REG_WRITE_BYTE(pSataChannel->mvSataAdapter->adapterIoBaseAddress,
                      pSataChannel->eDmaRegsOffset +
                      MV_ATA_DEVICE_CONTROL_REG_OFFSET,0);
    MV_REG_READ_BYTE(pSataChannel->mvSataAdapter->adapterIoBaseAddress,
                     pSataChannel->eDmaRegsOffset +
                     MV_ATA_DEVICE_CONTROL_REG_OFFSET);
}

/*******************************************************************************
* disableStorageDevInterrupt - Disable the storage device to be able to issue
*                              interrupt.
*
* DESCRIPTION:
*   This function disable the connected storage device to the given channel to
*   assert INTRQ by setting nIEN bit in the Device Control register.
*
* INPUT:
*   pSataChannel - Pointer to the Sata channel data structure.
*
* RETURN:
*   None.
*
* COMMENTS:
*   this function also clears the SRST bit
*
*******************************************************************************/
void disableStorageDevInterrupt(MV_SATA_CHANNEL *pSataChannel)
{


    MV_REG_WRITE_BYTE(pSataChannel->mvSataAdapter->adapterIoBaseAddress,
                      pSataChannel->eDmaRegsOffset +
                      MV_ATA_DEVICE_CONTROL_REG_OFFSET, MV_BIT1);
    MV_REG_READ_BYTE(pSataChannel->mvSataAdapter->adapterIoBaseAddress,
                     pSataChannel->eDmaRegsOffset +
                     MV_ATA_DEVICE_STATUS_REG_OFFSET);
}

/*******************************************************************************
* isStorageDevReadyForPIO - Check that the storage device connected to the given
*                           channel and the channel itself are ready to perform
*                           PIO commands.
*
* DESCRIPTION:
*   Check if the device connected to the given channel and the channel itself
*   are ready for PIO commands.
*
* INPUT:
*       pSataChannel - Pointer to the SATA channel data structure.
*
* RETURN:
*       MV_TRUE if the channel and the connected device ready to do PIO,
*       MV_FALSE otherwise.
*
* COMMENTS:
*     If the adapter's eEnEDMA bit in the EDMA Command Register is set, PIO
*     commands cannot be issued. The eEnEDMA bit cannot be reset by the CPU.
*     Only the EDMA resets it, when bit eDsEDMA is set or when an error
*     condition occurs.
*
*******************************************************************************/
static MV_BOOLEAN isStorageDevReadyForPIO(MV_SATA_CHANNEL *pSataChannel)
{
    MV_BUS_ADDR_T ioBaseAddr =pSataChannel->mvSataAdapter->adapterIoBaseAddress;
    MV_U32  eDmaRegsOffset = pSataChannel->eDmaRegsOffset;
    MV_U8   ATAcontrolRegValue;

    /* If the adapter's eEnEDMA bit in the EDMA Command Register is set  */
    /* PIO commands cannot be issued.                                       */
    if (pSataChannel->queueCommandsEnabled == MV_TRUE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d: PIO command failed:"
                 "EDMA is active\n", pSataChannel->mvSataAdapter->adapterId,
                 pSataChannel->channelNumber);
        return MV_FALSE;
    }
    /* Check if BUSY bit is '0' */

    /* Reading the Control register actually gives us the Alternate Status  */
    /* register content (ATA protocol). If the busy bit is set in the       */
    /* Alternate Status register, we wait for 50 mili-sec and try again, if */
    /* the busy bit is still set, we return false indicating that the       */
    /* device is not ready for PIO commands.                                */
    ATAcontrolRegValue = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                          MV_ATA_DEVICE_CONTROL_REG_OFFSET);
    if ((ATAcontrolRegValue & MV_ATA_BUSY_STATUS)!= 0)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  control regiser is "
                 "0x%02x\n",pSataChannel->mvSataAdapter->adapterId,
                 pSataChannel->channelNumber,ATAcontrolRegValue);
        return MV_FALSE;
    }
    if ( (pSataChannel->deviceType != MV_SATA_DEVICE_TYPE_ATAPI_DEVICE) && 
        ((ATAcontrolRegValue & MV_ATA_READY_STATUS) != MV_ATA_READY_STATUS))
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  storage drive is not"
                 " ready, ATA STATUS=0x%02x\n",
                 pSataChannel->mvSataAdapter->adapterId,
                 pSataChannel->channelNumber, ATAcontrolRegValue);
        return MV_FALSE;
    }
    /* The device is ready for PIO commands */
    return MV_TRUE;
}

MV_BOOLEAN mvStorageDevATAIdleImmediate(MV_SATA_ADAPTER *pAdapter, MV_U8 channelIndex)
{
    MV_SATA_CHANNEL *pSataChannel;
    MV_BUS_ADDR_T ioBaseAddr;
    MV_U32  eDmaRegsOffset;

    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  mvStorageDevATAIdentif"
                 "yDevice failed, Bad adapter data structure pointer\n");
        return MV_FALSE;
    }
    pSataChannel = pAdapter->sataChannel[channelIndex];
    if (pSataChannel == NULL)
    {     /* If the pointer do not exists, retrun false */
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  channel data "
                 "structure is not allocated\n", pAdapter->adapterId,
                 channelIndex);
        return MV_FALSE;
    }

    ioBaseAddr =pSataChannel->mvSataAdapter->adapterIoBaseAddress;

    mvOsSemTake(&pSataChannel->semaphore);
    eDmaRegsOffset = pSataChannel->eDmaRegsOffset;

    if (pSataChannel->queueCommandsEnabled == MV_TRUE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  mvStorageDevATAIdle"
                 "Immediate command failed: EDMA is active\n",
                 pSataChannel->mvSataAdapter->adapterId,
                 pSataChannel->channelNumber);
        mvOsSemRelease( &pSataChannel->semaphore);
        return MV_FALSE;
    }

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, "Issue IDLE IMMEDIATE COMMAND\n");
    disableStorageDevInterrupt(pSataChannel);
    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset + MV_ATA_DEVICE_COMMAND_REG_OFFSET,
                      MV_ATA_COMMAND_IDLE_IMMEDIATE);

    if (waitWhileStorageDevIsBusy(pAdapter,
                                  ioBaseAddr, eDmaRegsOffset, 10000, 100) == MV_FALSE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  Idle Immediate failed\n",
                 pSataChannel->mvSataAdapter->adapterId, pSataChannel->channelNumber);

        enableStorageDevInterrupt(pSataChannel);
        mvOsSemRelease( &pSataChannel->semaphore);
        return MV_FALSE;
    }

    enableStorageDevInterrupt(pSataChannel);
    mvOsSemRelease( &pSataChannel->semaphore);
    return MV_TRUE;
}

/*******************************************************************************
* mvStorageDevATAIdentifyDevice - Perform an ATA IDENTIFY device command.
*
* DESCRIPTION:
*       This function issues an IDENTIFY command to the connected device, and
*       stores all the information in the identifyDevice buffer of the channel.
*
* INPUT:
*       pAdapter     - Pointer to the device data structure.
*       channelIndex - The index of the channel where the storage device
*                      connected to.
*       PMPort       - index of the required destination port multipliers
*                      device Port (0 if no PM available).
*       identifyDeviceResult - a buffer that is allocated by IAL that will hold
*                              the IDENTIFY DEIVICE command result.
*
* RETURN:
*       MV_TRUE on success, MV_FALSE on failure.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
MV_BOOLEAN mvStorageDevATAIdentifyDevice(MV_SATA_ADAPTER *pAdapter,
                                         MV_U8 channelIndex,
                                         MV_U8 PMPort,
                                         MV_U16_PTR  identifyDeviceResult
                                        )
{
    MV_BOOLEAN result;
    /* Get the pointer to the relevant channel. */
    MV_SATA_CHANNEL *pSataChannel;
    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  mvStorageDevATAIdentif"
                 "yDevice failed, Bad adapter data structure pointer\n");
        return MV_FALSE;
    }
    pSataChannel = pAdapter->sataChannel[channelIndex];
    if (pSataChannel == NULL)
    {     /* If the pointer do not exists, retrun false */
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  channel data "
                 "structure is not allocated\n", pAdapter->adapterId,
                 channelIndex);
        return MV_FALSE;
    }
    if (identifyDeviceResult == NULL)
    {     /* If the pointer do not exists, retrun false */
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  identify data buffer"
                 " is not allocated\n", pAdapter->adapterId, channelIndex);
        return MV_FALSE;
    }
    result = mvStorageDevATAExecuteNonUDMACommand(pAdapter, channelIndex,
                                                  PMPort,
                                                  MV_NON_UDMA_PROTOCOL_PIO_DATA_IN,
                                                  MV_FALSE,
                                                  /* pBuffer */
                                                  identifyDeviceResult,
                                                  256,     /* count       */
                                                  0,       /* features    */
                                                  0,       /* sectorCount */
                                                  0,       /* lbaLow      */
                                                  0,       /* lbaMid      */
                                                  0,       /* lbaHigh     */
                                                  0,       /* device      */
                                                  /* The command */
                                                  MV_ATA_COMMAND_IDENTIFY);
    if (result == MV_FALSE)
    {
        return MV_FALSE;
    }
    if (identifyDeviceResult[IDEN_ATA_VERSION] & (MV_BIT7 | MV_BIT6 | MV_BIT5))
    {
        /* if ATA 5/6/7 then check CRC of Identify command result */
        MV_U8 crc = 0;
        MV_U16 count;
        MV_U8_PTR pointer = (MV_U8_PTR)identifyDeviceResult;
        /* If no 0xa5 signature valid, then don't check CRC */
        if (pointer[510] != 0xa5)
        {
            return MV_TRUE;
        }
        for (count = 0 ; count < ATA_SECTOR_SIZE ; count ++)
        {
            crc += pointer[count];
        }
        if (crc != 0)
        {
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  IDENTIFY DEVICE "
                     "ATA Command failed due to wrong CRC checksum (%02x)\n",
                     pAdapter->adapterId, channelIndex,crc);
            return MV_FALSE;
        }

    }
    return MV_TRUE;
}

/*******************************************************************************
* mvStorageDevATASoftResetDevice - Issue SATA SOFTWARE reset to device.
*
* DESCRIPTION:
*       Perform SOFTWARE RESET to the connected storage device by setting the
*       SRST bit of the ATA device COMMAND
*
* INPUT:
*       pAdapter     - Pointer to the device data structure.
*       channelIndex - Index of the required channel.
*       PMPort       - index of the required destination port multipliers
*                      device port (0 if no PM available).
*       registerStruct - Pointer to ATA registers data structure
*
* RETURN:
*       MV_TRUE on success, MV_FALSE otherwise.
*
* COMMENTS:
*       NONE
*
*******************************************************************************/
MV_BOOLEAN mvStorageDevATASoftResetDevice(MV_SATA_ADAPTER *pAdapter,
                                          MV_U8 channelIndex,
                                          MV_U8 PMPort,
                                          MV_STORAGE_DEVICE_REGISTERS *registerStruct
                                         )
{
    MV_SATA_CHANNEL *pSataChannel;
    MV_BUS_ADDR_T   ioBaseAddr;
    MV_U32          eDmaRegsOffset;
    MV_BOOLEAN      result;
    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  mvStorageDevATASoftRes"
                 "etDevice Failed, Bad adapter data structure pointer\n");
        return MV_FALSE;
    }
    pSataChannel = pAdapter->sataChannel[channelIndex];
    ioBaseAddr = pAdapter->adapterIoBaseAddress;
    if (pSataChannel == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  channel data structu"
                 "re is not allocated\n", pAdapter->adapterId, channelIndex);
        return MV_FALSE;
    }

    mvOsSemTake(&pSataChannel->semaphore);
    eDmaRegsOffset = pSataChannel->eDmaRegsOffset;

    if (pSataChannel->queueCommandsEnabled == MV_TRUE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  mvStorageDevATASoft"
                 "ResetDevice command failed: EDMA is active\n",
                 pSataChannel->mvSataAdapter->adapterId,
                 pSataChannel->channelNumber);
        mvOsSemRelease( &pSataChannel->semaphore);
        return MV_FALSE;
    }
    _setActivePMPort(pSataChannel, PMPort);
    result = _doSoftReset(pSataChannel);
    if (registerStruct)
    {
        dumpAtaDeviceRegisters(pAdapter, channelIndex, MV_FALSE,
                               registerStruct);
    }
    mvOsSemRelease( &pSataChannel->semaphore);
    return result;
}



void _startSoftResetDevice(MV_SATA_CHANNEL *pSataChannel)
{
    MV_BUS_ADDR_T   ioBaseAddr =
    pSataChannel->mvSataAdapter->adapterIoBaseAddress;

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_NON_UDMA_COMMAND | MV_DEBUG, "Issue SRST COMMAND\n");

/* Write to the Device Control register, bits 1,2:                      */
/* - bit 1 (nIEN): is the enable bit for the device assertion of INTRQ  */
/*   to the host. When the nIEN bit is set to one, or the device is not */
/*   selected, the device shall release the INTRQ signal.               */
/* - bit 2 (SRST): is the host software reset bit.                      */
    MV_REG_WRITE_BYTE(ioBaseAddr, pSataChannel->eDmaRegsOffset +
                      MV_ATA_DEVICE_CONTROL_REG_OFFSET, MV_BIT2|MV_BIT1);
    MV_REG_READ_BYTE(ioBaseAddr, pSataChannel->eDmaRegsOffset +
                     MV_ATA_DEVICE_CONTROL_REG_OFFSET);
    mvMicroSecondsDelay(pSataChannel->mvSataAdapter, 10);
    /* enableStorageDevInterrupt will clear the SRST bit*/
    enableStorageDevInterrupt(pSataChannel);
}

MV_BOOLEAN _isDeviceBsyBitOff(MV_SATA_CHANNEL *pSataChannel)
{
    MV_BUS_ADDR_T   ioBaseAddr =
    pSataChannel->mvSataAdapter->adapterIoBaseAddress;
    MV_U8           ATAstatus;
    MV_U32          eDmaRegsOffset = pSataChannel->eDmaRegsOffset;

    ATAstatus = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                 MV_ATA_DEVICE_STATUS_REG_OFFSET);
    if ((ATAstatus & MV_ATA_BUSY_STATUS) == 0)
    {
        return MV_TRUE;
    }
    else
    {
#ifdef MV_LOGGER
        if (pSataChannel->mvSataAdapter->sataAdapterGeneration >=
            MV_SATA_GEN_II)
        {
            MV_U32 ifStatus = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                               MV_SATA_II_IF_STATUS_REG_OFFSET);
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG,
                     "[%d %d] SATA interface status register = 0x%X\n",
                     pSataChannel->mvSataAdapter->adapterId,
                     pSataChannel->channelNumber,
                     ifStatus);
        }
#endif
        return MV_FALSE;
    }
}


/*******************************************************************************
* mvStorageDevATAStartSoftResetDevice -
*                   begins device software reset
*
* DESCRIPTION:
*
*   Submits SRST for channel connected device and exit. The IAL must call the
*   mvStorageIsDeviceBsyBitOff later on to check whether the device is
*   ready
*
* INPUT:
*       pAdapter    - pointer to the adapter data structure.
*       channelIndex - channel number
*       PMPort      - port multiplier port
*
* OUTPUT:
*       None
* RETURN:
*       MV_TRUE on success,
*       MV_FALSE otherwise.
* COMMENTS:
*
*******************************************************************************/
MV_BOOLEAN mvStorageDevATAStartSoftResetDevice(MV_SATA_ADAPTER *pAdapter,
                                               MV_U8 channelIndex,
                                               MV_U8 PMPort
                                              )
{
    MV_SATA_CHANNEL *pSataChannel;
    MV_BUS_ADDR_T   ioBaseAddr;
    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  mvStorageDevATASoftRes"
                 "etDevice Failed, Bad adapter data structure pointer\n");
        return MV_FALSE;
    }
    pSataChannel = pAdapter->sataChannel[channelIndex];
    ioBaseAddr = pAdapter->adapterIoBaseAddress;
    if (pSataChannel == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  channel data structu"
                 "re is not allocated\n", pAdapter->adapterId, channelIndex);
        return MV_FALSE;
    }

    mvOsSemTake(&pSataChannel->semaphore);
    if (pSataChannel->queueCommandsEnabled == MV_TRUE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  mvStorageDevATASoft"
                 "ResetDevice command failed: EDMA is active\n",
                 pSataChannel->mvSataAdapter->adapterId,
                 pSataChannel->channelNumber);
        mvOsSemRelease( &pSataChannel->semaphore);
        return MV_FALSE;
    }
    _setActivePMPort(pSataChannel, PMPort);
    _startSoftResetDevice(pSataChannel);
    mvOsSemRelease( &pSataChannel->semaphore);
    return MV_TRUE;
}

/*******************************************************************************
* mvStorageIsDeviceBsyBitOff -
*                    check if device is BUSY bit cleared after SRST
*
* DESCRIPTION:
*
*   Checks the if BSY bit in ATA status is on/off
*
* INPUT:
*       pAdapter    - pointer to the adapter data structure.
*       channelIndex - channel number
*       registerStruct - If non-zero then this function dumps ATA registers
*                        to this data structure before exit.
*
* OUTPUT:
*       None
* RETURN:
*       MV_TRUE if BSY bit is off
*       MV_FALSE if BSY bit is on (or on failure)
* COMMENTS:
*
*******************************************************************************/
MV_BOOLEAN mvStorageIsDeviceBsyBitOff(MV_SATA_ADAPTER *pAdapter,
                                      MV_U8 channelIndex,
                                      MV_STORAGE_DEVICE_REGISTERS *registerStruct
                                     )
{
    MV_SATA_CHANNEL *pSataChannel;
    MV_BUS_ADDR_T   ioBaseAddr;
    MV_BOOLEAN      result;
    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  mvStorageDevATASoftRes"
                 "etDevice Failed, Bad adapter data structure pointer\n");
        return MV_FALSE;
    }
    pSataChannel = pAdapter->sataChannel[channelIndex];
    ioBaseAddr = pAdapter->adapterIoBaseAddress;
    if (pSataChannel == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  channel data structu"
                 "re is not allocated\n", pAdapter->adapterId, channelIndex);
        return MV_FALSE;
    }
    mvOsSemTake(&pSataChannel->semaphore);
    result = _isDeviceBsyBitOff(pSataChannel);
    if (registerStruct)
    {
        dumpAtaDeviceRegisters(pAdapter, channelIndex, MV_FALSE,
                               registerStruct);
    }
    mvOsSemRelease( &pSataChannel->semaphore);
    return result;
}


/*******************************************************************************
* mvStorageDevATASetFeatures - Perform ATA SET FEATURES command.
*
* DESCRIPTION:
*       Perform ATA SET FEATURES command to the ATA device connected to the
*       given channel. This command is used by the host to establish parameters
*       that affect the execution of certain device features (Table 44 in the
*       ATA protocol document defines these features).
*
* INPUT:
*       pAdapter            - Pointer to the device data structure.
*       channelIndex        - Index of the required channel
*       PMPort              - index of the required destination port multipliers
*                             device Port (0 if no PM available).
*       subCommand          - Sub command for the SET FEATURES ATA command
*       subCommandSpecific1 - First parameter to the sub command.
*       subCommandSpecific2 - Second parameter to the sub command.
*       subCommandSpecific3 - Third parameter to the sub command.
*       subCommandSpecific4 - Fourth parameter to the sub command.
*
* RETURN:
*       MV_TRUE on success, MV_FALSE otherwise.
* COMMENTS:
*       NONE
*
*******************************************************************************/
MV_BOOLEAN mvStorageDevATASetFeatures(MV_SATA_ADAPTER *pAdapter,
                                      MV_U8 channelIndex,
                                      MV_U8   PMPort,
                                      MV_U8 subCommand,
                                      MV_U8 subCommandSpecific1,
                                      MV_U8 subCommandSpecific2,
                                      MV_U8 subCommandSpecific3,
                                      MV_U8 subCommandSpecific4)
{
    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG|MV_DEBUG_NON_UDMA_COMMAND,
             "ATA Set Features: %x , %x , %x , %x , %x\n", subCommand,
             subCommandSpecific1, subCommandSpecific2, subCommandSpecific3,
             subCommandSpecific4);
    return mvStorageDevATAExecuteNonUDMACommand(pAdapter, channelIndex,
                                                PMPort,
                                                MV_NON_UDMA_PROTOCOL_NON_DATA,
                                                MV_FALSE,
                                                NULL,    /* pBuffer*/
                                                0,       /* count  */
                                                subCommand,     /*features*/
                                                /* sectorCount */
                                                subCommandSpecific1,
                                                subCommandSpecific2,    /* lbaLow */
                                                subCommandSpecific3,    /* lbaMid */
                                                /* lbaHigh */
                                                subCommandSpecific4,
                                                0,      /* device */
                                                /* command */
                                                MV_ATA_COMMAND_SET_FEATURES);
}


/*******************************************************************************
* mvStorageDevATAExecuteNonUdmaCommand - perform ATA non udma command.
*
* DESCRIPTION:
*       perform ATA non UDMA command to the ATA device connected to the given
*       channel
*
* INPUT:
*   pAdapter    - pointer to the device data structure.
*   channelIndex    - index of the required channel
*   PMPort          - index of the required destination port multipliers
*                     device Port (0 if no PM available).
*   protocolType    - protocol type of the command
*   isEXT   - true when the given command is the EXTENDED
*   bufPtr  - pointer to the buffer to write/read to/from
*   count   - number of words to transfer
*   features    - the value to be written to the FEATURES register
*   sectorCount - the value to be written to the SECTOR COUNT register
*   lbaLow  - the value to be written to the LBA LOW register
*   lbaMid  - the value to be written to the LBA MID register
*   lbaHigh - the value to be written to the LBA HIGH register
*   device  - the value to be written to the DEVICE register
*   command - the value to be written to the COMMAND register
*
* RETURN:
*   MV_TRUE on success, MV_FALSE otherwise.
* COMMENTS:
*       when the command is EXTENDED, then the high 8 bits of the 16 bits values
*   will be written first, so they should hold the previous value as defined in
*   the ATA 6 standard
*
*******************************************************************************/
MV_BOOLEAN mvStorageDevATAExecuteNonUDMACommand(MV_SATA_ADAPTER *pAdapter,
                                                MV_U8 channelIndex,
                                                MV_U8 PMPort,
                                                MV_NON_UDMA_PROTOCOL protocolType,
                                                MV_BOOLEAN  isEXT,
                                                MV_U16_PTR bufPtr, MV_U32 count,
                                                MV_U16 features,
                                                MV_U16 sectorCount,
                                                MV_U16 lbaLow, MV_U16 lbaMid,
                                                MV_U16 lbaHigh, MV_U8 device,
                                                MV_U8 command)
{
    MV_BOOLEAN result;
    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  mvStorageDevATAExecute"
                 "NonUDMACommand Failed, Bad adapter data structure pointer\n");
        return MV_FALSE;
    }
    if (pAdapter->sataChannel[channelIndex] == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  mvStorageDevATAExecu"
                 "teNonUDMACommand Failed, channel data structure not allocated"
                 "\n",
                 pAdapter->adapterId, channelIndex);
        return MV_FALSE;
    }
    mvOsSemTake(&pAdapter->sataChannel[channelIndex]->semaphore);
    result = executeNonUDMACommand(pAdapter, channelIndex, PMPort, protocolType,
                                   isEXT, bufPtr, count, features, sectorCount,
                                   lbaLow, lbaMid, lbaHigh, device, command);
    mvOsSemRelease(&pAdapter->sataChannel[channelIndex]->semaphore);
    return result;
}
#if 0
MV_BOOLEAN executePacketCommand(MV_SATA_ADAPTER *pAdapter,
                                 MV_U8 channelIndex,
                                 MV_NON_UDMA_PROTOCOL   protocolType, 
                                 MV_U8  PMPort,
                                 MV_U16_PTR cdb,
                                MV_U8   cdb_len,
                                 MV_U16_PTR dataBufPtr
                                 )
{
    MV_SATA_CHANNEL *pSataChannel = pAdapter->sataChannel[channelIndex];
    MV_BUS_ADDR_T   ioBaseAddr = pAdapter->adapterIoBaseAddress;
    MV_U32          eDmaRegsOffset;
    MV_U32          i;
    MV_U32          count;
    MV_U8           ATAstatus;

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG|MV_DEBUG_NON_UDMA_COMMAND, " %d %d send PACKET "
             " command: protocol(%d) cdb %p cdb len %p buffer %p \n", pAdapter->adapterId,
             channelIndex, protocolType, cdb, cdb_len, dataBufPtr);

    eDmaRegsOffset = pSataChannel->eDmaRegsOffset;
    if ((PMPort) && ((pSataChannel->PMSupported == MV_FALSE) ||
                     (pSataChannel->deviceType != MV_SATA_DEVICE_TYPE_PM)))
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  executePacketCommand"
                 " failed PM not supported for this channel\n",
                 pSataChannel->mvSataAdapter->adapterId,
                 pSataChannel->channelNumber);
        mvOsSemRelease( &pSataChannel->semaphore);
        return MV_FALSE;
    }
    {
        if (isStorageDevReadyForPIO(pSataChannel) == MV_FALSE)
        {
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR,
                     " %d %d : Error in Issue NON UDMA command:"
                     " isStorageDevReadyForPIO failed\n",
                     pAdapter->adapterId, channelIndex);

            return MV_FALSE;
        }
    }
    _setActivePMPort(pSataChannel, PMPort);
    if (pSataChannel->queueCommandsEnabled == MV_TRUE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  PIO command failed:"
                 "EDMA is active\n", pSataChannel->mvSataAdapter->adapterId,
                 pSataChannel->channelNumber);
        return MV_FALSE;
    }

    if (pAdapter->sataAdapterGeneration == MV_SATA_GEN_I)
    {
        disableStorageDevInterrupt(pSataChannel);
    }


    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_FEATURES_REG_OFFSET, 0);

    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_SECTOR_COUNT_REG_OFFSET, 0);


    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_LBA_LOW_REG_OFFSET, 0);


    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_LBA_MID_REG_OFFSET,  0);

    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_LBA_HIGH_REG_OFFSET, 0x20);

    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_HEAD_REG_OFFSET, 0);
    MV_CPU_WRITE_BUFFER_FLUSH();

    /* 88SX60X1 FEr SATA #16 */
    if (pAdapter->sataAdapterGeneration >= MV_SATA_GEN_II)
    {
        enableStorageDevInterrupt(pSataChannel);
    }

    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_COMMAND_REG_OFFSET,MV_ATA_COMMAND_PACKET );

    /* Wait for PIO Setup or completion*/
    if (waitWhileStorageDevIsBusy(pAdapter, ioBaseAddr, eDmaRegsOffset, 3100, 10000) ==
        MV_FALSE)
    {
        enableStorageDevInterrupt(pSataChannel);
        return MV_FALSE;
    }
    if (protocolType == MV_NON_UDMA_PROTOCOL_PACKET_PIO_NON_DATA)
    {
        enableStorageDevInterrupt(pSataChannel);
        pSataChannel->recoveredErrorsCounter = 0;
        return MV_TRUE;
    }
    

    /* Check the status register on DATA request commands */
    ATAstatus = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                 MV_ATA_DEVICE_STATUS_REG_OFFSET);
    if (pAdapter->sataAdapterGeneration == MV_SATA_GEN_I)
    {
        if (!(ATAstatus & MV_ATA_DATA_REQUEST_STATUS))
        {
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d: DRQ bit in ATA STATUS"
                     " register is not set\n", pAdapter->adapterId, channelIndex);
            enableStorageDevInterrupt(pSataChannel);
            return MV_FALSE;
        }
    }
    if (pAdapter->sataAdapterGeneration >= MV_SATA_GEN_II)
    {

        if (waitForDRQ(pAdapter, ioBaseAddr, eDmaRegsOffset, 500, 10000)
            == MV_FALSE)
        {
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d: DRQ bit in ATA STATUS"
                     " register is not set\n", pAdapter->adapterId, channelIndex);
            enableStorageDevInterrupt(pSataChannel);
            return MV_FALSE;
        }
    }
    for (i = 0; i < cdb_len; i++)
    {
            MV_REG_WRITE_WORD(ioBaseAddr, eDmaRegsOffset +
                              MV_ATA_DEVICE_PIO_DATA_REG_OFFSET, cdb[i]);
            MV_CPU_WRITE_BUFFER_FLUSH();
    }

    if (waitForDRQ(pAdapter, ioBaseAddr, eDmaRegsOffset, 500, 10000)
          == MV_FALSE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d: DRQ bit in ATA STATUS"
                     " register is not set\n", pAdapter->adapterId, channelIndex);
        enableStorageDevInterrupt(pSataChannel);
        return MV_FALSE;
    }

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, " Status: %x\n", 
        MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset + MV_ATA_DEVICE_STATUS_REG_OFFSET));

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, " Sector Count: %x\n", 
        MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset + MV_ATA_DEVICE_SECTOR_COUNT_REG_OFFSET));

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, " LBA Mid: %x\n", 
        MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset + MV_ATA_DEVICE_LBA_MID_REG_OFFSET));

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, " LBA High: %x\n", 
        MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset + MV_ATA_DEVICE_LBA_HIGH_REG_OFFSET));
    
    count =  MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset + MV_ATA_DEVICE_LBA_MID_REG_OFFSET) + 
            (MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset + MV_ATA_DEVICE_LBA_HIGH_REG_OFFSET) << 8);
    count >>= 1;
    for ( i = 0 ; i < count; i++)
    {
        if(protocolType == MV_NON_UDMA_PROTOCOL_PACKET_PIO_DATA_IN)
        {
            dataBufPtr[i] = MV_REG_READ_WORD(ioBaseAddr, eDmaRegsOffset +
                              MV_ATA_DEVICE_PIO_DATA_REG_OFFSET);
        }
        else
        {
           MV_REG_WRITE_WORD(ioBaseAddr, eDmaRegsOffset +
                              MV_ATA_DEVICE_PIO_DATA_REG_OFFSET, dataBufPtr[i]);
 
        }
    }
    
    /* Wait for the storage device to be available */
    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, " %d %d: on non-UDMA sequence - checking if"
             " device is has finished the command\n",
             pAdapter->adapterId, channelIndex);

    if (waitWhileStorageDevIsBusy(pAdapter,
                                  ioBaseAddr, eDmaRegsOffset, 50000, 100) ==
        MV_FALSE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, " Status: %x\n", 
            MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset + MV_ATA_DEVICE_STATUS_REG_OFFSET));
        
        enableStorageDevInterrupt(pSataChannel);
        return MV_FALSE;
    }

    if (pAdapter->sataAdapterGeneration >= MV_SATA_GEN_II)
    {

        if (waitForDRQToClear(pAdapter, ioBaseAddr, eDmaRegsOffset, 50000, 100)
            == MV_FALSE)
        {
            enableStorageDevInterrupt(pSataChannel);
            return MV_FALSE;
        }
    }

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, " %d %d: Finish NonUdma Command. Status=0x%02x"
             "\n", pAdapter->adapterId, channelIndex,
             MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                              MV_ATA_DEVICE_STATUS_REG_OFFSET));
    enableStorageDevInterrupt(pSataChannel);
    pSataChannel->recoveredErrorsCounter = 0;
    return MV_TRUE;
}
#endif /* 0 */
MV_BOOLEAN executeNonUDMACommand(MV_SATA_ADAPTER *pAdapter,
                                 MV_U8 channelIndex,
                                 MV_U8  PMPort,
                                 MV_NON_UDMA_PROTOCOL protocolType,
                                 MV_BOOLEAN  isEXT,
                                 MV_U16_PTR bufPtr, MV_U32 count,
                                 MV_U16 features,
                                 MV_U16 sectorCount,
                                 MV_U16 lbaLow, MV_U16 lbaMid,
                                 MV_U16 lbaHigh, MV_U8 device,
                                 MV_U8 command)
{
    MV_SATA_CHANNEL *pSataChannel = pAdapter->sataChannel[channelIndex];
    MV_BUS_ADDR_T   ioBaseAddr = pAdapter->adapterIoBaseAddress;
    MV_U32          eDmaRegsOffset;
    MV_U32          i;
    MV_U8           ATAstatus;

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG|MV_DEBUG_NON_UDMA_COMMAND, " %d %d Issue NON "
             "UDMA command: protocol(%d) %p , %x , %x , %x , %x.%x.%x %x "
             "command=%x\n", pAdapter->adapterId, channelIndex, protocolType,
             bufPtr, count, features, sectorCount, lbaLow, lbaMid,
             lbaHigh, device, command);

    eDmaRegsOffset = pSataChannel->eDmaRegsOffset;
    if ((PMPort) && ((pSataChannel->PMSupported == MV_FALSE) ||
                     (pSataChannel->deviceType != MV_SATA_DEVICE_TYPE_PM)))
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  executeNonUDMACommand"
                 " failed PM not supported for this channel\n",
                 pSataChannel->mvSataAdapter->adapterId,
                 pSataChannel->channelNumber);
        mvOsSemRelease( &pSataChannel->semaphore);
        return MV_FALSE;
    }
    if (command != MV_ATA_COMMAND_PM_READ_REG &&
        command != MV_ATA_COMMAND_PM_WRITE_REG)
    {
        if (isStorageDevReadyForPIO(pSataChannel) == MV_FALSE)
        {
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR,
                     " %d %d : Error in Issue NON UDMA command:"
                     " isStorageDevReadyForPIO failed\n",
                     pAdapter->adapterId, channelIndex);

            return MV_FALSE;
        }
    }
    _setActivePMPort(pSataChannel, PMPort);
    if (pSataChannel->queueCommandsEnabled == MV_TRUE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  PIO command failed:"
                 "EDMA is active\n", pSataChannel->mvSataAdapter->adapterId,
                 pSataChannel->channelNumber);
        return MV_FALSE;
    }

    if (pAdapter->sataAdapterGeneration == MV_SATA_GEN_I)
    {
        disableStorageDevInterrupt(pSataChannel);
    }

    if (isEXT == MV_TRUE)
    {
        MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                          MV_ATA_DEVICE_FEATURES_REG_OFFSET,
                          (features & 0xff00) >> 8);
        MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                          MV_ATA_DEVICE_SECTOR_COUNT_REG_OFFSET,
                          (sectorCount & 0xff00) >> 8);
        MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                          MV_ATA_DEVICE_LBA_LOW_REG_OFFSET,
                          (lbaLow & 0xff00) >> 8);
        MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                          MV_ATA_DEVICE_LBA_MID_REG_OFFSET,
                          (lbaMid & 0xff00) >> 8);
        MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                          MV_ATA_DEVICE_LBA_HIGH_REG_OFFSET,
                          (lbaHigh & 0xff00) >> 8);
    }
    else
    {
        if ((features & 0xff00) || (sectorCount & 0xff00) || (lbaLow & 0xff00) ||
            (lbaMid & 0xff00) || (lbaHigh & 0xff00))
        {
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR,
                     " %d %d : Error in Issue NON UDMA command:"
                     " bits[15:8] of register values should be reserved"
                     " Features 0x%02x, SectorCount 0x%02x, LBA Low 0x%02x,"
                     " LBA Mid 0x%02x, LBA High 0x%02x\n",
                     pAdapter->adapterId, channelIndex, features,
                     sectorCount, lbaLow, lbaMid, lbaHigh);
            enableStorageDevInterrupt(pSataChannel);
            return MV_FALSE;
        }
    }

    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_FEATURES_REG_OFFSET, features & 0xff);

    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_SECTOR_COUNT_REG_OFFSET, sectorCount & 0xff);


    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_LBA_LOW_REG_OFFSET, lbaLow & 0xff);


    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_LBA_MID_REG_OFFSET, lbaMid & 0xff);

    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_LBA_HIGH_REG_OFFSET, lbaHigh & 0xff);

    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_HEAD_REG_OFFSET, device);
    MV_CPU_WRITE_BUFFER_FLUSH();

    /* 88SX60X1 FEr SATA #16 */
    /* 88SX6042/88SX7042 FEr SATA #16 */
    /* 88F5182 FEr #SATA-S11 */
    if (pAdapter->sataAdapterGeneration >= MV_SATA_GEN_II)
    {
        enableStorageDevInterrupt(pSataChannel);
    }

    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_COMMAND_REG_OFFSET, command);

    if (protocolType == MV_NON_UDMA_PROTOCOL_NON_DATA)
    {
        /* Wait for the command to complete */
        if (waitWhileStorageDevIsBusy(pAdapter, ioBaseAddr, eDmaRegsOffset, 3100, 10000) ==
            MV_FALSE)
        {
            enableStorageDevInterrupt(pSataChannel);
            return MV_FALSE;
        }
        enableStorageDevInterrupt(pSataChannel);
        pSataChannel->recoveredErrorsCounter = 0;
        return MV_TRUE;
    }
    /* Wait for the command to complete */
    if (waitWhileStorageDevIsBusy(pAdapter, ioBaseAddr, eDmaRegsOffset, 3100, 10000) ==
        MV_FALSE)
    {
        enableStorageDevInterrupt(pSataChannel);
        return MV_FALSE;
    }
    /* Check the status register on DATA request commands */
    ATAstatus = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                 MV_ATA_DEVICE_STATUS_REG_OFFSET);
    if (pAdapter->sataAdapterGeneration == MV_SATA_GEN_I)
    {
        if (!(ATAstatus & MV_ATA_DATA_REQUEST_STATUS))
        {
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d: DRQ bit in ATA STATUS"
                     " register is not set\n", pAdapter->adapterId, channelIndex);
            enableStorageDevInterrupt(pSataChannel);
            return MV_FALSE;
        }
    }
    if (pAdapter->sataAdapterGeneration >= MV_SATA_GEN_II)
    {

        if (waitForDRQ(pAdapter, ioBaseAddr, eDmaRegsOffset, 500, 10000)
            == MV_FALSE)
        {
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d: DRQ bit in ATA STATUS"
                     " register is not set\n", pAdapter->adapterId, channelIndex);
            enableStorageDevInterrupt(pSataChannel);
            return MV_FALSE;
        }
    }
    for (i = 0; i < count; i++)
    {
        /* Every DRQ data block we have to check the BUSY bit to verify that
           the Disk is ready for next block transfer  */
        if ((i & (((MV_U32)pSataChannel->DRQDataBlockSize * ATA_SECTOR_SIZE_IN_WORDS) - 1)) == 0)
        {
            if (pAdapter->sataAdapterGeneration >= MV_SATA_GEN_II)
            {
                /* Perform a dummy read */
                MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                 MV_ATA_DEVICE_ALTERNATE_REG_OFFSET);
                /* 88SX60X1 FEr SATA #16 */
                /* 88SX6042/88SX7042 FEr SATA #16 */
                /* 88F5182 FEr #SATA-S11 */
                if (i != 0)
                {
                    if (waitWhileStorageDevIsBusy_88SX60X1(pAdapter,
                                                                                  ioBaseAddr, eDmaRegsOffset, channelIndex,
                                                                                  50000, 100) == MV_FALSE)
                    {
                        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR,
                                 "Sata device interrupt timeout...i = %d\n",i);
                        enableStorageDevInterrupt(pSataChannel);
                        return MV_FALSE;
                    }
                }
                else
                {
                    MV_U8 sataUnit = channelIndex >> 2,portNum = channelIndex & 3;

                    if (waitWhileStorageDevIsBusy(pAdapter,ioBaseAddr,
                                                  eDmaRegsOffset, 50000, 100) == MV_FALSE)
                    {
                        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR,
                                 "Busy bit timeout...i = %d\n",i);
                        enableStorageDevInterrupt(pSataChannel);
                        return MV_FALSE;
                    }
                    MV_REG_WRITE_DWORD (ioBaseAddr, MV_SATAHC_REGS_BASE_OFFSET(sataUnit) +
                                        MV_SATAHC_INTERRUPT_CAUSE_REG_OFFSET,
                                        ~(1 << (8 + portNum)));
                }
                if (waitForDRQ(pAdapter, ioBaseAddr, eDmaRegsOffset, 50000, 100)
                    == MV_FALSE)
                {
                    enableStorageDevInterrupt(pSataChannel);
                    return MV_FALSE;
                }
            }
            else if (pAdapter->sataAdapterGeneration == MV_SATA_GEN_I)
            {
                if (waitWhileStorageDevIsBusy(pAdapter,
                                              ioBaseAddr, eDmaRegsOffset,
                                              50000, 100) == MV_FALSE)
                {
                    enableStorageDevInterrupt(pSataChannel);
                    return MV_FALSE;
                }
            }
        }
        if (protocolType == MV_NON_UDMA_PROTOCOL_PIO_DATA_IN)
        {
            bufPtr[i] = MV_REG_READ_WORD(ioBaseAddr, eDmaRegsOffset +
                                         MV_ATA_DEVICE_PIO_DATA_REG_OFFSET);
        }
        else
        {
            MV_REG_WRITE_WORD(ioBaseAddr, eDmaRegsOffset +
                              MV_ATA_DEVICE_PIO_DATA_REG_OFFSET, bufPtr[i]);
            MV_CPU_WRITE_BUFFER_FLUSH();
        }
    }


    /* Wait for the storage device to be available */
    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, " %d %d: on non-UDMA sequence - checking if"
             " device is has finished the command\n",
             pAdapter->adapterId, channelIndex);

    if (waitWhileStorageDevIsBusy(pAdapter,
                                  ioBaseAddr, eDmaRegsOffset, 50000, 100) ==
        MV_FALSE)
    {
        enableStorageDevInterrupt(pSataChannel);
        return MV_FALSE;
    }

    if (pAdapter->sataAdapterGeneration >= MV_SATA_GEN_II)
    {

        if (waitForDRQToClear(pAdapter, ioBaseAddr, eDmaRegsOffset, 50000, 100)
            == MV_FALSE)
        {
            enableStorageDevInterrupt(pSataChannel);
            return MV_FALSE;
        }
    }

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, " %d %d: Finish NonUdma Command. Status=0x%02x"
             "\n", pAdapter->adapterId, channelIndex,
             MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                              MV_ATA_DEVICE_STATUS_REG_OFFSET));
    enableStorageDevInterrupt(pSataChannel);
    pSataChannel->recoveredErrorsCounter = 0;
    return MV_TRUE;
}
MV_BOOLEAN  _PMAccessReg(MV_SATA_ADAPTER *pAdapter, MV_U8 channelIndex,
                         MV_U8 PMPort, MV_U8 PMReg, MV_U32 *pValue,
                         MV_STORAGE_DEVICE_REGISTERS *registerStruct,
                         MV_BOOLEAN isRead)
{
    MV_BOOLEAN result;

    if (isRead == MV_TRUE)
    {
        result = executeNonUDMACommand(pAdapter, channelIndex,
                                       MV_SATA_PM_CONTROL_PORT,
                                       MV_NON_UDMA_PROTOCOL_NON_DATA,
                                       MV_TRUE/*isEXT*/,
                                       NULL/*bufPtr*/,
                                       0/*count*/,
                                       PMReg /*features*/, 0/*sectorCount*/,
                                       0 /*lbaLow*/, 0 /*lbaMid*/, 0 /*lbaHigh*/,
                                       PMPort/*device*/,
                                       MV_ATA_COMMAND_PM_READ_REG/*command*/);
        if (result == MV_TRUE)
        {
            MV_BUS_ADDR_T   ioBaseAddr = pAdapter->adapterIoBaseAddress;
            MV_U32 eDmaRegsOffset = pAdapter->sataChannel[channelIndex]->eDmaRegsOffset;

            *pValue = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                       MV_ATA_DEVICE_SECTOR_COUNT_REG_OFFSET);
            *pValue |= MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                        MV_ATA_DEVICE_LBA_LOW_REG_OFFSET) << 8;
            *pValue |= MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                        MV_ATA_DEVICE_LBA_MID_REG_OFFSET) << 16;
            *pValue |= MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                        MV_ATA_DEVICE_LBA_HIGH_REG_OFFSET) << 24;
        }
    }
    else
    {
        result = executeNonUDMACommand(pAdapter, channelIndex,
                                       MV_SATA_PM_CONTROL_PORT,
                                       MV_NON_UDMA_PROTOCOL_NON_DATA,
                                       MV_TRUE/*isEXT*/,
                                       NULL/*bufPtr*/,
                                       0/*count*/,
                                       PMReg /*features*/,
                                       (MV_U16)((*pValue) & 0xff)/*sectorCount*/,
                                       (MV_U16)(((*pValue) & 0xff00) >> 8) /*lbaLow*/,
                                       (MV_U16)(((*pValue) & 0xff0000) >> 16)   /*lbaMid*/,
                                       (MV_U16)(((*pValue) & 0xff000000) >> 24) /*lbaHigh*/,
                                       PMPort/*device*/,
                                       MV_ATA_COMMAND_PM_WRITE_REG/*command*/);
    }
    if (registerStruct)
    {
        dumpAtaDeviceRegisters(pAdapter, channelIndex, MV_FALSE,
                               registerStruct);
    }
    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG|MV_DEBUG_PM, " %d %d: %s PM Reg %s: PM Port %x"
             ", PM Reg %d, value %x\n", pAdapter->adapterId, channelIndex,
             (isRead == MV_TRUE) ? "Read" : "Write",
             (result == MV_TRUE) ? "Succeeded" : "Failed",
             PMPort, PMReg, *pValue);

    return result;
}

MV_BOOLEAN waitForDRQToClear(MV_SATA_ADAPTER* pAdapter,
                             MV_BUS_ADDR_T ioBaseAddr,
                             MV_U32 eDmaRegsOffset, MV_U32 loops,
                             MV_U32 delayParam)
{
    MV_U8   ATAstatus = 0;
    MV_U32  i;

    for (i = 0;i < loops; i++)
    {
        ATAstatus = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                     MV_ATA_DEVICE_STATUS_REG_OFFSET);
        if ((ATAstatus & MV_ATA_BUSY_STATUS) == 0)
        {
            if (!(ATAstatus & MV_ATA_DATA_REQUEST_STATUS))
            {
                mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG, "waitWhileStorageDevIsBusy: %d loops *"
                         "%d usecs\n", i, delayParam);
                return MV_TRUE;
            }
        }
        mvMicroSecondsDelay(pAdapter, delayParam);
    }
    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, "waitWhileStorageDevIsBusy<FAILED>: Time out - Device ERROR"
             " Status: 0x%02x. loops %d, delay %d\n", ATAstatus, loops, delayParam);

    return MV_FALSE;
}

void dumpAtaDeviceRegisters(MV_SATA_ADAPTER *pAdapter,
                            MV_U8 channelIndex, MV_BOOLEAN isEXT,
                            MV_STORAGE_DEVICE_REGISTERS *pRegisters)
{
    MV_BUS_ADDR_T   ioBaseAddr = pAdapter->adapterIoBaseAddress;
    MV_U32 eDmaRegsOffset = pAdapter->sataChannel[channelIndex]->eDmaRegsOffset;

	if (pAdapter->sataAdapterGeneration < MV_SATA_GEN_IIE)
	{
	    if (MV_REG_READ_DWORD(ioBaseAddr, eDmaRegsOffset +
			MV_EDMA_COMMAND_REG_OFFSET) & MV_BIT0)
		{
			mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR,
				" %d %d: dumpAtaDeviceRegisters: Edma is active!!!\n",
				pAdapter->adapterId, channelIndex);
			return;
		}
	}
    MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                      MV_ATA_DEVICE_CONTROL_REG_OFFSET, 0);

    pRegisters->errorRegister =
    MV_REG_READ_BYTE(ioBaseAddr,
                     eDmaRegsOffset + MV_ATA_DEVICE_ERROR_REG_OFFSET);

    pRegisters->sectorCountRegister =
    MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                     MV_ATA_DEVICE_SECTOR_COUNT_REG_OFFSET) & 0x00ff;
    pRegisters->lbaLowRegister =
    MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                     MV_ATA_DEVICE_LBA_LOW_REG_OFFSET) & 0x00ff;

    pRegisters->lbaMidRegister =
    MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                     MV_ATA_DEVICE_LBA_MID_REG_OFFSET) & 0x00ff;

    pRegisters->lbaHighRegister =
    MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                     MV_ATA_DEVICE_LBA_HIGH_REG_OFFSET) & 0x00ff;

    if (isEXT == MV_TRUE)
    {
        /*set the HOB bit of DEVICE CONTROL REGISTER */

        MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                          MV_ATA_DEVICE_CONTROL_REG_OFFSET, MV_BIT7);

        pRegisters->sectorCountRegister |= (MV_REG_READ_BYTE(ioBaseAddr,
                                                             eDmaRegsOffset +
                                                             MV_ATA_DEVICE_SECTOR_COUNT_REG_OFFSET) << 8) & 0xff00;

        pRegisters->lbaLowRegister |= (MV_REG_READ_BYTE(ioBaseAddr,
                                                        eDmaRegsOffset + MV_ATA_DEVICE_LBA_LOW_REG_OFFSET) << 8)
                                      & 0xff00;

        pRegisters->lbaMidRegister |= (MV_REG_READ_BYTE(ioBaseAddr,
                                                        eDmaRegsOffset + MV_ATA_DEVICE_LBA_MID_REG_OFFSET) << 8)
                                      & 0xff00;

        pRegisters->lbaHighRegister |= (MV_REG_READ_BYTE(ioBaseAddr,
                                                         eDmaRegsOffset + MV_ATA_DEVICE_LBA_HIGH_REG_OFFSET) << 8)
                                       & 0xff00;
        MV_REG_WRITE_BYTE(ioBaseAddr, eDmaRegsOffset +
                          MV_ATA_DEVICE_CONTROL_REG_OFFSET, 0);

    }

    pRegisters->deviceRegister = MV_REG_READ_BYTE(ioBaseAddr,
                                                  eDmaRegsOffset + MV_ATA_DEVICE_HEAD_REG_OFFSET);

    pRegisters->statusRegister = MV_REG_READ_BYTE(ioBaseAddr,
                                                  eDmaRegsOffset + MV_ATA_DEVICE_STATUS_REG_OFFSET);


}


MV_BOOLEAN _doSoftReset(MV_SATA_CHANNEL *pSataChannel)
{
    MV_BUS_ADDR_T   ioBaseAddr = pSataChannel->mvSataAdapter->adapterIoBaseAddress;
    MV_U32          i;
    MV_U8           ATAstatus;
    MV_U32          eDmaRegsOffset = pSataChannel->eDmaRegsOffset;

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_NON_UDMA_COMMAND | MV_DEBUG, "Issue SRST COMMAND\n");

/* Write to the Device Control register, bits 1,2:                      */
/* - bit 1 (nIEN): is the enable bit for the device assertion of INTRQ  */
/*   to the host. When the nIEN bit is set to one, or the device is not */
/*   selected, the device shall release the INTRQ signal.               */
/* - bit 2 (SRST): is the host software reset bit.                      */
    MV_REG_WRITE_BYTE(ioBaseAddr, pSataChannel->eDmaRegsOffset +
                      MV_ATA_DEVICE_CONTROL_REG_OFFSET, MV_BIT2|MV_BIT1);
    MV_REG_READ_BYTE(ioBaseAddr, pSataChannel->eDmaRegsOffset +
                     MV_ATA_DEVICE_CONTROL_REG_OFFSET);
    mvMicroSecondsDelay(pSataChannel->mvSataAdapter, 10);

/* enableStorageDevInterrupt will clear the SRST bit*/
    enableStorageDevInterrupt(pSataChannel);

    mvMicroSecondsDelay(pSataChannel->mvSataAdapter, 500);
    mvMicroSecondsDelay(pSataChannel->mvSataAdapter, 500);
    mvMicroSecondsDelay(pSataChannel->mvSataAdapter, 500);
    mvMicroSecondsDelay(pSataChannel->mvSataAdapter, 500);

    for (i = 0;i < 31000; i++)
    {
        ATAstatus = MV_REG_READ_BYTE(ioBaseAddr, eDmaRegsOffset +
                                     MV_ATA_DEVICE_STATUS_REG_OFFSET);
        if ((ATAstatus & MV_ATA_BUSY_STATUS) == 0)
        {
            return MV_TRUE;
        }
        mvMicroSecondsDelay(pSataChannel->mvSataAdapter, 1000);
    }
    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d: Software reset failed "
             "Status=0x%02x\n", pSataChannel->mvSataAdapter->adapterId,
             pSataChannel->channelNumber, ATAstatus);

    return MV_FALSE;
}

/*******************************************************************************
* mvPMDevReadReg - Reads port multiplier's internal register
*
*
* DESCRIPTION:
*       Performs PIO non-data command for reading port multiplier's internal
*       register.
*
* INPUT:
*       pAdapter            - Pointer to the device data structure.
*       channelIndex        - Index of the required channel
*       PMPort              - This should be port 0xf
*       PMReg               - The required register to be read
*       pValue              - A pointer to 32bit data container that holds
*                             the result.
*       registerStruct      - A pointer to ATA register data structure. This
*                             holds the ATA registers dump after command
*                             is executed.
*
* RETURN:
*       MV_TRUE on success, MV_FALSE otherwise.
* COMMENTS:
*       NONE
*
*******************************************************************************/
MV_BOOLEAN  mvPMDevReadReg(MV_SATA_ADAPTER *pAdapter, MV_U8 channelIndex,
                           MV_U8 PMPort, MV_U8 PMReg, MV_U32 *pValue,
                           MV_STORAGE_DEVICE_REGISTERS *registerStruct)
{
    MV_SATA_CHANNEL *pSataChannel;
    MV_BOOLEAN      result;

    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  mvStorageDevPMReadReg"
                 " Failed, Bad adapter data structure pointer\n");
        return MV_FALSE;
    }
    pSataChannel = pAdapter->sataChannel[channelIndex];
    if (pSataChannel == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  channel data structu"
                 "re is not allocated\n", pAdapter->adapterId, channelIndex);
        return MV_FALSE;
    }

    mvOsSemTake(&pSataChannel->semaphore);
    if (pSataChannel->PMSupported == MV_FALSE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  mvStorageDevPMReadReg"
                 " failed PM not supported for this channel\n",
                 pAdapter->adapterId, channelIndex);
        mvOsSemRelease( &pSataChannel->semaphore);
        return MV_FALSE;
    }
    if (pSataChannel->queueCommandsEnabled == MV_TRUE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  mvStorageDevPMReadReg"
                 " command failed: EDMA is active\n",
                 pAdapter->adapterId, channelIndex);
        mvOsSemRelease( &pSataChannel->semaphore);
        return MV_FALSE;
    }

    result = _PMAccessReg(pAdapter, channelIndex, PMPort, PMReg, pValue,
                          registerStruct, MV_TRUE);

    mvOsSemRelease( &pSataChannel->semaphore);
    return result;
}



/*******************************************************************************
* mvPMDevWriteReg - Writes to port multiplier's internal register
*
*
* DESCRIPTION:
*       Performs PIO non-data command for writing to port multiplier's internal
*       register.
*
* INPUT:
*       pAdapter            - Pointer to the device data structure.
*       channelIndex        - Index of the required channel
*       PMPort              - This should be port 0xf
*       PMReg               - The required register to be read
*       value               - Holds 32bit of the value to be written
*       registerStruct      - A pointer to ATA register data structure. This
*                             holds the ATA registers dump after command
*                             is executed.
*
* RETURN:
*       MV_TRUE on success, MV_FALSE otherwise.
* COMMENTS:
*       NONE
*
*******************************************************************************/
MV_BOOLEAN  mvPMDevWriteReg(MV_SATA_ADAPTER *pAdapter, MV_U8 channelIndex,
                            MV_U8 PMPort, MV_U8 PMReg, MV_U32 value,
                            MV_STORAGE_DEVICE_REGISTERS *registerStruct)
{
    MV_SATA_CHANNEL *pSataChannel;
    MV_BOOLEAN      result;

    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  mvStorageDevPMWriteReg"
                 " Failed, Bad adapter data structure pointer\n");
        return MV_FALSE;
    }
    pSataChannel = pAdapter->sataChannel[channelIndex];
    if (pSataChannel == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  channel data structu"
                 "re is not allocated\n", pAdapter->adapterId, channelIndex);
        return MV_FALSE;
    }

    mvOsSemTake(&pSataChannel->semaphore);
    if (pSataChannel->PMSupported == MV_FALSE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  mvStorageDevPMWriteReg"
                 " failed PM not supported for this channel\n",
                 pAdapter->adapterId, channelIndex);
        mvOsSemRelease( &pSataChannel->semaphore);
        return MV_FALSE;
    }
    if (pSataChannel->queueCommandsEnabled == MV_TRUE)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR, " %d %d:  mvStorageDevPMWriteReg"
                 " command failed: EDMA is active\n",
                 pAdapter->adapterId, channelIndex);
        mvOsSemRelease( &pSataChannel->semaphore);
        return MV_FALSE;
    }

    result = _PMAccessReg(pAdapter, channelIndex, PMPort, PMReg, &value,
                          registerStruct, MV_FALSE);

    mvOsSemRelease( &pSataChannel->semaphore);
    return result;
}

static MV_BOOLEAN _checkPMPortSStatus(MV_SATA_ADAPTER* pAdapter,
                                      MV_U8 channelIndex,
                                      MV_U8 PMPort,
                                      MV_BOOLEAN *error)
{
    MV_BOOLEAN result;
    MV_U32 SStatus;

    result = mvPMDevReadReg(pAdapter, channelIndex, PMPort,
                            MV_SATA_PSCR_SSTATUS_REG_NUM, &SStatus, NULL);

    if (result == MV_FALSE)
    {
        *error = MV_TRUE;
        return result;
    }
    *error = MV_FALSE;
    SStatus &= (MV_BIT0 | MV_BIT1 | MV_BIT2);
    if ((SStatus == (MV_BIT0 | MV_BIT1)) || (SStatus == 0))
    {
        return MV_TRUE;
    }
    return MV_FALSE;
}

MV_BOOLEAN  mvPMLinkUp(MV_SATA_ADAPTER *pAdapter, MV_U8 channelIndex, MV_U8 PMPort,
		       MV_BOOLEAN force_gen1)
{
    MV_BOOLEAN  result;
    MV_U32	speed_force = 0;
    
    if(force_gen1 == MV_TRUE)
	 speed_force = 0x10;

    result = mvPMDevWriteReg(pAdapter, channelIndex, PMPort,
                             MV_SATA_PSCR_SCONTROL_REG_NUM, 0x301 | speed_force, NULL);
    if (result == MV_FALSE)
    {
        return result;
    }
    mvMicroSecondsDelay(pAdapter, MV_SATA_COMM_INIT_DELAY);
    result = mvPMDevWriteReg(pAdapter, channelIndex, PMPort,
                             MV_SATA_PSCR_SCONTROL_REG_NUM, 0x300 | speed_force, NULL);
    return result;
}

/*******************************************************************************
* mvPMDevEnableStaggeredSpinUp -
*
*
* DESCRIPTION:
*       Enables commnucation on a port multiplier's device SATA channel
*
* INPUT:
*       pAdapter            - Pointer to the device data structure.
*       channelIndex        - Index of the required channel
*       PMPort              - Required device SATA channel on port multiplier
*
* RETURN:
*       MV_TRUE on success, MV_FALSE otherwise.
* COMMENTS:
*       NONE
*
*******************************************************************************/
MV_BOOLEAN  mvPMDevEnableStaggeredSpinUp(MV_SATA_ADAPTER *pAdapter,
                                         MV_U8 channelIndex, MV_U8 PMPort)
{
    return mvPMLinkUp(pAdapter, channelIndex, PMPort, MV_FALSE);
}


/*******************************************************************************
* mvPMDevEnableStaggeredSpinUpAll -
*
*
* DESCRIPTION:
*       Enables commnucation on all port multiplier's device SATA channels
*
* INPUT:
*       pAdapter            - Pointer to the device data structure.
*       channelIndex        - Index of the required channel
*       PMNumOfPorts        - Number of device SATA channel the port multiplier
*                             has.
*       bitmask             - A pointer to 16bit data container that holds
*                             a bitmask of '1' when the relevant port multiplier's
*                             device port staggered spinup operation is success.
*
* RETURN:
*       MV_TRUE on success, MV_FALSE otherwise.
* COMMENTS:
*       NONE
*
*******************************************************************************/

MV_BOOLEAN mvPMDevEnableStaggeredSpinUpAll(MV_SATA_ADAPTER *pSataAdapter,
                                           MV_U8 channelIndex,
                                           MV_U8 PMNumOfPorts,
                                           MV_U16 *bitmask)
{
    MV_U8 PMPort;
    MV_U8 retryCount;
    MV_U8 tmpBitmask = 1;
    if (bitmask == NULL)
    {
        return MV_FALSE;
    }
    /*Do not issue staggered spinup for port 0 - already done because of
    legacy port mode*/
    *bitmask = 1;
    for (PMPort = 0; PMPort < PMNumOfPorts; PMPort++)
    {
        MV_BOOLEAN error;
        /* if sata communication already done(No staggered spin-up)*/
        if (_checkPMPortSStatus(pSataAdapter, channelIndex, PMPort, &error) ==
             MV_TRUE)
        {
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG,
                     "[%d %d %d]: sata communication already established.\n",
                     pSataAdapter->adapterId, channelIndex, PMPort);
            tmpBitmask |= (1 << PMPort);
            continue;
        }
        if (mvPMDevEnableStaggeredSpinUp(pSataAdapter,
                                         channelIndex,
                                         PMPort) == MV_TRUE)
        {
            tmpBitmask |= (1 << PMPort);
        }
        else
        {
            mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR,
                     "Error [%d %d %d]: "
                     "PM enable staggered spin-up failed.\n",
                     pSataAdapter->adapterId, channelIndex, PMPort);
            return MV_FALSE;
        }
    }
    mvMicroSecondsDelay(pSataAdapter, MV_SATA_COMM_INIT_WAIT_DELAY);
    for (retryCount = 0; retryCount < 200; retryCount++)
    {
        for (PMPort = 0; PMPort < PMNumOfPorts; PMPort++)
        {
            MV_BOOLEAN error;
            if ((*bitmask) & (1 << PMPort))
            {
                continue;
            }
            if (_checkPMPortSStatus(pSataAdapter,
                                    channelIndex, PMPort, &error) == MV_FALSE)
            {
                if (error == MV_TRUE)
                {
                    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR,
                             "[%d %d %d]: "
                             "Fatal error - cannot read PM port SStatus.\n",
                             pSataAdapter->adapterId, channelIndex, PMPort);
                    break;
                }
                mvMicroSecondsDelay(pSataAdapter, 1000);
            }
            else
            {
                if (bitmask != NULL)
                {
                    *bitmask |= (1 << PMPort);
                }
                mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG,
                         "[%d %d %d] PM SATA PHY ready after %d msec\n",
                         pSataAdapter->adapterId, channelIndex,
                         PMPort, retryCount);
            }
        }
        if (tmpBitmask == *bitmask)
        {
            break;
        }
    }
    if (tmpBitmask != *bitmask)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR,
                 "[%d %d %d]: "
                 "Some of PM ports PHY are not initialized.\n",
                 pSataAdapter->adapterId, channelIndex, PMPort);

    }
    return MV_TRUE;
}

/*******************************************************************************
* mvPMDevEnableStaggeredSpinUpPort -
*
*
* DESCRIPTION:
*       Enables commnucation on port multiplier's device SATA port
*
* INPUT:
*       pAdapter            - Pointer to the device data structure.
*       channelIndex        - Index of the required channel
*       PMPort              - the port number 
*
* RETURN:
*       MV_TRUE on success, MV_FALSE otherwise.
* COMMENTS:
*       NONE
*
*******************************************************************************/
MV_BOOLEAN mvPMDevEnableStaggeredSpinUpPort(MV_SATA_ADAPTER *pSataAdapter,
					    MV_U8 channelIndex,
 					    MV_U8 PMPort,
 					    MV_BOOLEAN force_speed_gen1)

{
    MV_U8 retryCount;

    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG,
	     "[%d %d %d]: init sata communication.\n",
	     pSataAdapter->adapterId, channelIndex, PMPort);
 
    if (mvPMLinkUp(pSataAdapter, channelIndex, PMPort,
 		   force_speed_gen1) != MV_TRUE)
    {
	    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR,
                     "Error [%d %d %d]: "
                     "PM enable staggered spin-up failed.\n",
                     pSataAdapter->adapterId, channelIndex, PMPort);
            return MV_FALSE;
    }
    mvMicroSecondsDelay(pSataAdapter, MV_SATA_COMM_INIT_WAIT_DELAY);
    for (retryCount = 0; retryCount < 200; retryCount++)
    {
            MV_BOOLEAN error;
            
            if (_checkPMPortSStatus(pSataAdapter,
                                    channelIndex, PMPort, &error) == MV_FALSE)
            {
		    if (error == MV_TRUE)
		    {
			    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_ERROR,
				     "[%d %d %d]: "
				     "Fatal error - cannot read PM port SStatus.\n",
				     pSataAdapter->adapterId, channelIndex, PMPort);
			    return MV_FALSE;
		    }
		    mvMicroSecondsDelay(pSataAdapter, 1000);
            }
            else
            {
		    mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG,
			     "[%d %d %d] PM SATA PHY ready after %d msec\n",
			     pSataAdapter->adapterId, channelIndex,
			     PMPort, retryCount);
		    break;
            }
        
    }

    return MV_TRUE;
}

/*******************************************************************************
* mvStorageDevExecutePIO -
*
*
* DESCRIPTION:
*       Sends custom PIO command (polling driven)
*
* INPUT:
*   pAdapter    - pointer to the device data structure.
*   channelIndex    - index of the required channel
*   PMPort          - index of the required destination port multipliers
*                     device Port (0 if no PM available).
*   protocolType    - protocol type of the command
*   isEXT   - true when the given command is the EXTENDED
*   bufPtr  - pointer to the buffer to write/read to/from
*   pInATARegs  - Holds ATA registers for the command
*   pOutATARegs - Holds ATA registers after the command completed
*
* RETURN:
*   MV_TRUE on success, MV_FALSE otherwise.
* COMMENTS:
*       when the command is EXTENDED, then the high 8 bits of the 16 bits values
*   will be written first, so they should hold the previous value as defined in
*   the ATA 6 standard
*******************************************************************************/
MV_BOOLEAN mvStorageDevExecutePIO(MV_SATA_ADAPTER *pAdapter,
                                  MV_U8 channelIndex,
                                  MV_U8 PMPort,
                                  MV_NON_UDMA_PROTOCOL protocolType,
                                  MV_BOOLEAN  isEXT, MV_U16_PTR bufPtr,
                                  MV_U32 count,
                                  MV_STORAGE_DEVICE_REGISTERS *pInATARegs,
                                  MV_STORAGE_DEVICE_REGISTERS *pOutATARegs)
{
    MV_BOOLEAN result;
    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  mvPMDevExecutePIO"
                 "Command Failed, Bad adapter data structure pointer\n");
        return MV_FALSE;
    }
    if (pAdapter->sataChannel[channelIndex] == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  mvPMDevExecutePIO"
                 "Command Failed, channel data structure not allocated\n",
                 pAdapter->adapterId, channelIndex);
        return MV_FALSE;
    }
    mvOsSemTake(&pAdapter->sataChannel[channelIndex]->semaphore);
    result = executeNonUDMACommand(pAdapter, channelIndex, PMPort,
                                   protocolType, isEXT,
                                   bufPtr, count, pInATARegs->featuresRegister,
                                   pInATARegs->sectorCountRegister,
                                   pInATARegs->lbaLowRegister,
                                   pInATARegs->lbaMidRegister,
                                   pInATARegs->lbaHighRegister,
                                   pInATARegs->deviceRegister,
                                   pInATARegs->commandRegister);
    if (pOutATARegs)
    {
        dumpAtaDeviceRegisters(pAdapter, channelIndex, isEXT, pOutATARegs);
    }
    mvOsSemRelease(&pAdapter->sataChannel[channelIndex]->semaphore);
    return result;
}

/*******************************************************************************
* mvStorageDevSetDeviceType -
*
*
* DESCRIPTION:
*       Sets the device type connected directly to the specific adapter's
*       SATA channel.
*
* INPUT:
*       pAdapter            - Pointer to the device data structure.
*       channelIndex        - Index of the required channel
*       deviceType          - Type of device
*
* RETURN:
*       MV_TRUE on success, MV_FALSE otherwise.
* COMMENTS:
*       NONE
*
*******************************************************************************/
MV_BOOLEAN  mvStorageDevSetDeviceType(MV_SATA_ADAPTER *pAdapter, MV_U8 channelIndex,
                                      MV_SATA_DEVICE_TYPE deviceType)
{
    MV_SATA_CHANNEL *pSataChannel;

    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  "
                 " mvStorageDevSetDeviceType Failed, Bad adapter data structure "
                 "pointer\n");
        return MV_SATA_DEVICE_TYPE_UNKNOWN;
    }
    pSataChannel = pAdapter->sataChannel[channelIndex];
    if (pSataChannel == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  channel data structu"
                 "re is not allocated\n", pAdapter->adapterId, channelIndex);
        return MV_FALSE;
    }

    pSataChannel->deviceType = deviceType;

    return MV_TRUE;
}

/*******************************************************************************
* mvStorageDevGetDeviceType -
*
*
* DESCRIPTION:
*       Gets the device type connected directly to the specific adapter's
*       SATA channel.
*
* INPUT:
*       pAdapter            - Pointer to the device data structure.
*       channelIndex        - Index of the required channel
*
* RETURN:
*       MV_TRUE on success, MV_FALSE otherwise.
* COMMENTS:
*       NONE
*
*******************************************************************************/
MV_SATA_DEVICE_TYPE mvStorageDevGetDeviceType(MV_SATA_ADAPTER *pAdapter,
                                              MV_U8 channelIndex)
{
    MV_SATA_CHANNEL *pSataChannel;

    if (pAdapter == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, "    :  "
                 " mvStorageDevGetDeviceType Failed, Bad adapter data structure "
                 "pointer\n");
        return MV_SATA_DEVICE_TYPE_UNKNOWN;
    }
    pSataChannel = pAdapter->sataChannel[channelIndex];
    if (pSataChannel == NULL)
    {
        mvLogMsg(MV_CORE_DRIVER_LOG_ID, MV_DEBUG_FATAL_ERROR, " %d %d:  "
                 "channel data structure is not allocated\n",
                 pAdapter->adapterId, channelIndex);
        return MV_SATA_DEVICE_TYPE_UNKNOWN;
    }

    return pSataChannel->deviceType;
}

