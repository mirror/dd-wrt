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
*******************************************************************************/
/*******************************************************************************
* file_name - mvLinuxIALSmart.c
*
* DESCRIPTION: C file for S.M.A.R.T. features - smartmontools app
*
* DEPENDENCIES:
*   None.
*
*
*******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/kdev_t.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <asm/dma.h>
#include <asm/system.h>
#include <asm/io.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_tcq.h>
#else
#include <linux/blk.h>
#include "scsi.h"
#include "hosts.h"
#endif
#include "mvScsiAtaLayer.h"
#include "mvLinuxIalHt.h"
#include "mvRegs.h"
#include "mvIALCommon.h"
#include "mvLinuxIalSmart.h"


extern MV_VOID handleNoneUdmaError(MV_SATA_SCSI_CMD_BLOCK  *pScb,
                                   MV_STORAGE_DEVICE_REGISTERS *registerStruct);

extern MV_VOID handleUdmaError(MV_SATA_SCSI_CMD_BLOCK  *pScb,
                               MV_U32 responseFlags,
                               MV_STORAGE_DEVICE_REGISTERS *registerStruct);

extern MV_VOID  checkQueueCommandResult(MV_SATA_SCSI_CMD_BLOCK *pScb,
                                        MV_QUEUE_COMMAND_RESULT result);

extern MV_VOID setSenseData(IN MV_SATA_SCSI_CMD_BLOCK *pScb, IN MV_U8 SenseKey,
                            IN MV_U8 AdditionalSenseCode);

static MV_BOOLEAN
SmartCommandCompletionCB(MV_SATA_ADAPTER *pSataAdapter,
                         MV_U8 channelNum,
                         MV_COMPLETION_TYPE comp_type,
                         MV_VOID_PTR commandId,
                         MV_U16 responseFlags,
                         MV_U32 timeStamp,
                         MV_STORAGE_DEVICE_REGISTERS *registerStruct);


MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaSendSmartCommand(IN  MV_SATA_ADAPTER* pSataAdapter,
                                                       IN  MV_SATA_SCSI_CMD_BLOCK *pScb)
{
    MV_U8 *buff = (MV_U8*)pScb->pDataBuffer;
    MV_NON_UDMA_PROTOCOL protocolType = MV_NON_UDMA_PROTOCOL_NON_DATA;
    MV_QUEUE_COMMAND_RESULT result = MV_QUEUE_COMMAND_RESULT_OK;
    MV_QUEUE_COMMAND_INFO   qCommandInfo;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];

    mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG, "SMART command: command received, bufflen = %d.\n",
             pScb->dataBufferLength);
    pScb->dataTransfered = 0;
    pScb->senseDataLength = 0;
    if (pScb->bus >= pSataAdapter->numberOfChannels)
    {
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_INVALID_BUS;
        pScb->dataTransfered = 0;
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    if ((pScb->target >= MV_SATA_PM_MAX_PORTS) ||
        (pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target].driveReady == MV_FALSE))
    {
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_NO_DEVICE;
        pScb->dataTransfered = 0;
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    if (pScb->dataBufferLength <= 6 || /*six byte opcode*/
        (buff[SMART_BUF_COMMAND_OFFSET] != WIN_IDENTIFY &&
        buff[SMART_BUF_COMMAND_OFFSET] != MV_ATA_COMMAND_IDLE &&
        buff[SMART_BUF_COMMAND_OFFSET] != MV_ATA_COMMAND_IDLE_IMMEDIATE &&
        buff[SMART_BUF_COMMAND_OFFSET] != MV_ATA_COMMAND_STANDBY_IMMEDIATE &&
         buff[SMART_BUF_COMMAND_OFFSET] != WIN_SMART))
    {
        mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG_ERROR, "invalid SMART command received");
        setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                     SCSI_ADSENSE_NO_SENSE);
        pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    if (buff[SMART_BUF_COMMAND_OFFSET] == WIN_IDENTIFY)
    {
        if (pScb->dataBufferLength < MV_ATA_IDENTIFY_DEV_DATA_LENGTH*2 + 6)
        {
            mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG_ERROR, "WIN_IDENTIFY: "
                     "invalid buffer length.%d\n", pScb->dataBufferLength);
            setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                         SCSI_ADSENSE_NO_SENSE);
            pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
            pScb->completionCallBack(pSataAdapter, pScb);
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }
        protocolType = MV_NON_UDMA_PROTOCOL_PIO_DATA_IN;
        buff[SMART_BUF_COMMAND_OFFSET] = MV_ATA_COMMAND_IDENTIFY;
        buff[SMART_BUF_SECTORCOUNT_OFFSET] = 0;
        buff[SMART_BUF_LBALOW_OFFSET] = 0;
        buff[SMART_BUF_LBAMID_OFFSET] = 0;
        buff[SMART_BUF_LBAHIGH_OFFSET] = 0;
        buff[SMART_BUF_FEATURES_OFFSET] = 0;
    }
    else  if (buff[SMART_BUF_COMMAND_OFFSET] == MV_ATA_COMMAND_IDLE)
    {
         mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG, "SMART command: IDLE command received, sector count = %d.\n",
              buff[SMART_BUF_SECTORCOUNT_OFFSET]);

        protocolType = MV_NON_UDMA_PROTOCOL_NON_DATA;
        buff[SMART_BUF_COMMAND_OFFSET] = MV_ATA_COMMAND_IDLE;
        buff[SMART_BUF_LBALOW_OFFSET] = 0;
        buff[SMART_BUF_LBAMID_OFFSET] = 0;
        buff[SMART_BUF_LBAHIGH_OFFSET] = 0;
        buff[SMART_BUF_FEATURES_OFFSET] = 0;
    }
    else  if (buff[SMART_BUF_COMMAND_OFFSET] == MV_ATA_COMMAND_IDLE_IMMEDIATE)
    {
         mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG, "SMART command: IDLE IMMEDIATE command received\n");
              
        protocolType = MV_NON_UDMA_PROTOCOL_NON_DATA;
        buff[SMART_BUF_COMMAND_OFFSET] = MV_ATA_COMMAND_IDLE_IMMEDIATE;
        buff[SMART_BUF_LBALOW_OFFSET] = 0;
        buff[SMART_BUF_LBAMID_OFFSET] = 0;
        buff[SMART_BUF_LBAHIGH_OFFSET] = 0;
        buff[SMART_BUF_FEATURES_OFFSET] = 0;
        buff[SMART_BUF_SECTORCOUNT_OFFSET] = 0;

    }
    else  if (buff[SMART_BUF_COMMAND_OFFSET] == MV_ATA_COMMAND_STANDBY_IMMEDIATE)
    {
         mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG, "SMART command: STANDBY IMMEDIATE command received\n");
              
        protocolType = MV_NON_UDMA_PROTOCOL_NON_DATA;
        buff[SMART_BUF_COMMAND_OFFSET] = MV_ATA_COMMAND_STANDBY_IMMEDIATE;
        buff[SMART_BUF_LBALOW_OFFSET] = 0;
        buff[SMART_BUF_LBAMID_OFFSET] = 0;
        buff[SMART_BUF_LBAHIGH_OFFSET] = 0;
        buff[SMART_BUF_FEATURES_OFFSET] = 0;
        buff[SMART_BUF_SECTORCOUNT_OFFSET] = 0;

    }
 
    else
    {
        buff[SMART_BUF_LBAMID_OFFSET] = 0x4F;
        buff[SMART_BUF_LBAHIGH_OFFSET] = 0xC2;
        switch (buff[SMART_BUF_FEATURES_OFFSET])
        {
        case SMART_READ_VALUES:
        case SMART_READ_THRESHOLDS:
        case SMART_READ_LOG_SECTOR:
            if (pScb->dataBufferLength < ATA_SECTOR_SIZE + 6)
            {
                setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                             SCSI_ADSENSE_NO_SENSE);
                pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
                pScb->ScsiCommandCompletion =
                MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
                pScb->completionCallBack(pSataAdapter, pScb);
                return MV_SCSI_COMMAND_STATUS_COMPLETED;
            }
            protocolType = MV_NON_UDMA_PROTOCOL_PIO_DATA_IN;
            break;
        case SMART_ENABLE:
        case SMART_DISABLE:
        case SMART_AUTO_OFFLINE:
        case SMART_AUTOSAVE:
        case SMART_IMMEDIATE_OFFLINE:
        case SMART_STATUS:
            protocolType = MV_NON_UDMA_PROTOCOL_NON_DATA;
            break;
        default:
            setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                         SCSI_ADSENSE_NO_SENSE);
            pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
            pScb->completionCallBack(pSataAdapter, pScb);
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }
    }
    qCommandInfo.type = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
    qCommandInfo.commandParams.NoneUdmaCommand.protocolType = protocolType;
    qCommandInfo.commandParams.NoneUdmaCommand.isEXT = MV_FALSE;
    qCommandInfo.PMPort = pScb->target;
    if (protocolType == MV_NON_UDMA_PROTOCOL_NON_DATA)
    {
        qCommandInfo.commandParams.NoneUdmaCommand.bufPtr = NULL;
        qCommandInfo.commandParams.NoneUdmaCommand.count = 0;
    }
    else
    {
        qCommandInfo.commandParams.NoneUdmaCommand.bufPtr =
        (MV_U16_PTR)&buff[6];
        qCommandInfo.commandParams.NoneUdmaCommand.count =
        (MV_U32)(ATA_SECTOR_SIZE/2);
        /*in words*/
    }
    qCommandInfo.commandParams.NoneUdmaCommand.features =
    (MV_U16)buff[SMART_BUF_FEATURES_OFFSET];
    qCommandInfo.commandParams.NoneUdmaCommand.sectorCount =
    (MV_U16)buff[SMART_BUF_SECTORCOUNT_OFFSET];
    qCommandInfo.commandParams.NoneUdmaCommand.lbaLow =
    (MV_U16)buff[SMART_BUF_LBALOW_OFFSET];
    qCommandInfo.commandParams.NoneUdmaCommand.lbaMid =
    (MV_U16)buff[SMART_BUF_LBAMID_OFFSET];
    qCommandInfo.commandParams.NoneUdmaCommand.lbaHigh =
    (MV_U16)buff[SMART_BUF_LBAHIGH_OFFSET];
    qCommandInfo.commandParams.NoneUdmaCommand.device = (MV_U8)(MV_BIT6);
    qCommandInfo.commandParams.NoneUdmaCommand.command =
    buff[SMART_BUF_COMMAND_OFFSET];
    qCommandInfo.commandParams.NoneUdmaCommand.callBack =
    SmartCommandCompletionCB;
    qCommandInfo.commandParams.NoneUdmaCommand.commandId = (MV_VOID_PTR) pScb;
    result = mvSataQueueCommand(pSataAdapter, pScb->bus, &qCommandInfo);
    if (result != MV_QUEUE_COMMAND_RESULT_OK)
    {
        checkQueueCommandResult(pScb, result);
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }
    pDriveData->stats.totalIOs++;

    mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG, "SMART command: SMART command %02X, queued\n",
             buff[SMART_BUF_FEATURES_OFFSET]);
    return MV_SCSI_COMMAND_STATUS_QUEUED;
}

static void SmartFillReturnBuffer(IN MV_U8* buff,
                                  IN MV_STORAGE_DEVICE_REGISTERS *registerStruct)
{

    /*For PIO non-data return registers' values*/
    if (buff[SMART_BUF_COMMAND_OFFSET] == WIN_SMART &&
        buff[SMART_BUF_FEATURES_OFFSET] != SMART_READ_VALUES &&
        buff[SMART_BUF_FEATURES_OFFSET] != SMART_READ_THRESHOLDS &&
        buff[SMART_BUF_FEATURES_OFFSET] != SMART_READ_LOG_SECTOR)
    {
        buff[6+SMART_BUF_COMMAND_OFFSET] =
        buff[SMART_BUF_COMMAND_OFFSET];
        buff[6+SMART_BUF_FEATURES_OFFSET] =
        buff[SMART_BUF_FEATURES_OFFSET];
        buff[6+SMART_BUF_LBALOW_OFFSET] =
        registerStruct->lbaLowRegister & 0xFF;
        buff[6+SMART_BUF_SECTORCOUNT_OFFSET] =
        registerStruct->sectorCountRegister & 0xFF;
        buff[6+SMART_BUF_LBAMID_OFFSET] =
        registerStruct->lbaMidRegister & 0xFF;
        buff[6+SMART_BUF_LBAHIGH_OFFSET] =
        registerStruct->lbaHighRegister & 0xFF;
        buff[6+SMART_BUF_DEVICE_OFFSET] =
        registerStruct->deviceRegister & 0xFF;
        buff[6+SMART_BUF_ERROR_OFFSET] =
        registerStruct->errorRegister & 0xFF;
    }
}

static MV_BOOLEAN
SmartCommandCompletionCB(MV_SATA_ADAPTER *pSataAdapter,
                         MV_U8 channelNum,
                         MV_COMPLETION_TYPE comp_type,
                         MV_VOID_PTR commandId,
                         MV_U16 responseFlags,
                         MV_U32 timeStamp,
                         MV_STORAGE_DEVICE_REGISTERS *registerStruct)
{
    MV_SATA_SCSI_CMD_BLOCK  *pScb;
    struct scsi_cmnd   *SCpnt;
    struct mv_comp_info *pInfo;

    if (commandId == NULL)
    {
        mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG_ERROR, " commandId is NULL, can't hanlde this !!!,adapterId=%d,"
                 " channel=%d \n", pSataAdapter->adapterId, channelNum);
        return MV_FALSE;
    }

    pScb = commandId;
    SCpnt = (struct scsi_cmnd *)pScb->IALData;
    pInfo = ( struct mv_comp_info *) &(SCpnt->SCp);

    switch (comp_type)
    {
    case MV_COMPLETION_TYPE_NORMAL:
        if (pScb->ScsiCdb[0] == SCSI_OPCODE_MVSATA_SMART)
        {
            SmartFillReturnBuffer(pScb->pDataBuffer, registerStruct);
            mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG, "SMART PIO command completed: "
                     "dev=%04X, Low=%04X, Mid=%04X, High=%04X, "
                     "SC=%04X, status = %04X,\n",
                     registerStruct->deviceRegister,
                     registerStruct->lbaLowRegister,
                     registerStruct->lbaMidRegister,
                     registerStruct->lbaHighRegister,
                     registerStruct->sectorCountRegister,
                     registerStruct->statusRegister);

	    if(pInfo->kmap_buffer)
	    {
		    struct scatterlist *sg;
		    MV_U8*		pBuffer;
		    sg = (struct scatterlist *)SCpnt->request_buffer;
		    
		    mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG, "SMART PIO command needs kmap for Data-in buffer\n");
		    
		    pBuffer = kmap_atomic(sg->page, KM_IRQ0) + sg->offset;
		    memcpy(pBuffer, pInfo->pSALBlock->pDataBuffer, sg->length); 	
		    kunmap_atomic(pBuffer - sg->offset, KM_IRQ0);
	    }
		    
        }
        pScb->dataTransfered = MV_ATA_IDENTIFY_DEV_DATA_LENGTH*2;
        pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
        mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG, "command completed. pScb %p\n", pScb);
        break;
    case MV_COMPLETION_TYPE_ABORT:
        mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG_ERROR, " command Aborted. Cdb: %02x %02x %02x %02x %02x "
                 "%02x %02x %02x %02x %02x\n", pScb->ScsiCdb[0],
                 pScb->ScsiCdb[1], pScb->ScsiCdb[2], pScb->ScsiCdb[3],
                 pScb->ScsiCdb[4], pScb->ScsiCdb[5], pScb->ScsiCdb[6],
                 pScb->ScsiCdb[7], pScb->ScsiCdb[8], pScb->ScsiCdb[9]);
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_ABORTED;
        pScb->dataTransfered = 0;
        pScb->senseDataLength = 0;
        break;
    case MV_COMPLETION_TYPE_ERROR:
        pScb->dataTransfered = 0;
        pScb->senseDataLength = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_ATA_FAILED;

        mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG_ERROR, " completion error, adapter =%d, channel=%d, flags=%x\n"
                 ,pSataAdapter->adapterId, channelNum, responseFlags);
        mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG_ERROR, "Failed command Cdb: %02x %02x %02x %02x %02x "
                 "%02x %02x %02x %02x %02x\n", pScb->ScsiCdb[0],
                 pScb->ScsiCdb[1], pScb->ScsiCdb[2], pScb->ScsiCdb[3],
                 pScb->ScsiCdb[4], pScb->ScsiCdb[5], pScb->ScsiCdb[6],
                 pScb->ScsiCdb[7], pScb->ScsiCdb[8], pScb->ScsiCdb[9]);
        /* here the  eDMA will be stopped, so we have to flush  */
        /* the pending commands                                 */
        handleNoneUdmaError(pScb, registerStruct);
        break;
    default:
        mvLogMsg(MV_IAL_LOG_ID, MV_DEBUG_ERROR, "Unknown completion type (%d)\n", comp_type);
        return MV_FALSE;
    }
    pScb->completionCallBack(pSataAdapter, pScb);
    return MV_TRUE;
}

