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
* mvScsiAtaLayer.c
*
* DESCRIPTION:
*       C implementation for SCSI to ATA translation layer.
*
* DEPENDENCIES:
*   mvIALCommon.h
*   mvScsiAtaLayer.h
*
*******************************************************************************/

/* includes */
#include "mvScsiAtaLayer.h"
#include "mvIALCommon.h"

#ifdef MV_LOGGER
    #define SAL_SPRINTF     sprintf
#endif

/* ATA defines */
/* Bits for HD_ERROR */
#define NM_ERR          0x02    /* media present */
#define ABRT_ERR        0x04    /* Command aborted */
#define MCR_ERR         0x08    /* media change request */
#define IDNF_ERR        0x10    /* ID field not found */
#define MC_ERR          0x20    /* media changed */
#define UNC_ERR         0x40    /* Uncorrect data */
#define WP_ERR          0x40    /* write protect */
#define ICRC_ERR        0x80    /* new meaning:  CRC error during transfer */

#ifdef MV_LOGGER
static MV_VOID reportScbCompletion(MV_SATA_ADAPTER*    pSataAdapter,
                                   MV_SATA_SCSI_CMD_BLOCK *pScb);
#endif

/* Locals */
static MV_VOID mvAta2HostString(IN   MV_U16 *source,
                                OUT  MV_U16 *target,
                                IN   MV_U32 wordsCount);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaGetInquiryData(IN  MV_SATA_ADAPTER*    pSataAdapter,
                                                            IN  MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaTestUnitReady(IN  MV_SATA_ADAPTER*    pSataAdapter,
                                                           IN  MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaSendDataCommand(IN  MV_SATA_ADAPTER*    pSataAdapter,
                                                             IN  MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaGetReadCapacityData(IN MV_SATA_ADAPTER*    pSataAdapter,
                                                                 IN MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaReportLuns(IN MV_SATA_ADAPTER*    pSataAdapter,
							IN MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaSendVerifyCommand(IN MV_SATA_ADAPTER*    pSataAdapter,
                                                               IN MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaReassignBlocks(IN MV_SATA_ADAPTER    *pSataAdapter,
                                                            IN MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaSendSyncCacheCommand(IN MV_SATA_ADAPTER*    pSataAdapter,
                                                                  IN MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaGetRequestSenseData(IN MV_SATA_ADAPTER*    pSataAdapter,
                                                                 IN MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaGetModeSenseData(IN    MV_SATA_ADAPTER*    pSataAdapter,
                                                              IN    MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_BOOLEAN  mvScsiAtaGetModeSenseDataPhase2(IN MV_SATA_ADAPTER    *pSataAdapter,
                                                   IN  MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaModeSelect(IN    MV_SATA_ADAPTER*    pSataAdapter,
                                                        IN    MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_U8 modeSelect(IN MV_SATA_ADAPTER    *pSataAdapter,
                        IN  MV_SATA_SCSI_CMD_BLOCK  *pScb,
                        MV_SCSI_COMMAND_STATUS_TYPE *pCommandStatus);

static MV_U8 mvParseModeCachingPage(MV_SATA_ADAPTER *pSataAdapter,
                                    IN  MV_SATA_SCSI_CMD_BLOCK  *pScb,
                                    MV_U8 *buffer,
                                    MV_SCSI_COMMAND_STATUS_TYPE *pCommandStatus);
static MV_U32 mvModeSenseCachingPage(MV_SATA_SCSI_CMD_BLOCK  *pScb,
                                     MV_U8 *buffer, MV_U8 pageControl);

static MV_U32 mvModeSenseControlPage(MV_SATA_ADAPTER *pSataAdapter,
                                     MV_SATA_SCSI_CMD_BLOCK  *pScb,
                                     MV_U8 *buffer, MV_U8 pageControl);

static MV_BOOLEAN SALCommandCompletionCB(MV_SATA_ADAPTER *pSataAdapter,
                                         MV_U8 channelNum,
                                         MV_COMPLETION_TYPE comp_type,
                                         MV_VOID_PTR commandId,
                                         MV_U16 responseFlags,
                                         MV_U32 timeStamp,
                                         MV_STORAGE_DEVICE_REGISTERS *registerStruct);

MV_VOID setSenseData(IN MV_SATA_SCSI_CMD_BLOCK *pScb, IN MV_U8 SenseKey,
                     IN MV_U8 AdditionalSenseCode, IN MV_U8 ASCQ);

MV_VOID _fillSenseInformation(IN MV_SATA_SCSI_CMD_BLOCK *pScb, MV_SCSI_SENSE_DATA *SenseData,
			      MV_STORAGE_DEVICE_REGISTERS *registerStruct);

MV_VOID handleNoneUdmaError(MV_SATA_SCSI_CMD_BLOCK  *pScb,
                            MV_STORAGE_DEVICE_REGISTERS *registerStruct);

static MV_VOID handleUdmaError(MV_SATA_SCSI_CMD_BLOCK  *pScb,
                               MV_U32 responseFlags,
                               MV_STORAGE_DEVICE_REGISTERS *registerStruct);

/*static*/ MV_VOID  checkQueueCommandResult(MV_SATA_SCSI_CMD_BLOCK *pScb,
                                            MV_QUEUE_COMMAND_RESULT result);

static MV_VOID  mvScsiAtaSendSplittedVerifyCommand(IN MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_VOID  mvScsiAtaSendReadLookAhead(IN MV_SATA_ADAPTER*    pSataAdapter,
                                           IN MV_SATA_SCSI_CMD_BLOCK  *pScb);

static MV_VOID mvAta2HostString(IN MV_U16 *source,
                                OUT MV_U16 *target,
                                IN MV_U32 wordsCount
                               )
{
    MV_U32 i;
    for (i=0 ; i < wordsCount; i++)
    {
        target[i] = (source[i] >> 8) | ((source[i] & 0xff) << 8);
        target[i] = MV_LE16_TO_CPU(target[i]);
    }
}

MV_VOID setSenseData(IN MV_SATA_SCSI_CMD_BLOCK *pScb, IN MV_U8 SenseKey,
                     IN MV_U8 AdditionalSenseCode, IN MV_U8 ASCQ)
{
    MV_SCSI_SENSE_DATA SenseData;

    if (pScb->senseBufferLength == 0)
    {
        pScb->senseDataLength = 0;
        return;
    }
    memset(&SenseData, 0, sizeof(MV_SCSI_SENSE_DATA));
//    SenseData.Valid = 0;
    SenseData.ResponseCode = MV_SCSI_RESPONSE_CODE;
    SenseData.SenseKey = SenseKey;
    SenseData.AdditionalSenseCode = AdditionalSenseCode;
    SenseData.AdditionalSenseCodeQualifier = ASCQ;
    SenseData.AdditionalSenseLength = sizeof(MV_SCSI_SENSE_DATA) - 8;
    pScb->senseDataLength = sizeof(MV_SCSI_SENSE_DATA);
    if (pScb->senseBufferLength < pScb->senseDataLength)
    {
        pScb->senseDataLength = pScb->senseBufferLength;
    }
    memcpy(pScb->pSenseBuffer, &SenseData, pScb->senseDataLength);
}

MV_VOID _fillSenseInformation(IN MV_SATA_SCSI_CMD_BLOCK *pScb, MV_SCSI_SENSE_DATA *SenseData,
			      MV_STORAGE_DEVICE_REGISTERS *registerStruct)
{
        if (pScb->isExtended == MV_TRUE)
        {
		/* LBA 48 error handling */
		SenseData->InformationDesc.information[2] = (MV_U8)((registerStruct->lbaHighRegister >> 8) & 0xff);
		SenseData->InformationDesc.information[3] = (MV_U8)((registerStruct->lbaMidRegister >> 8) & 0xff);
		SenseData->InformationDesc.information[4] = (MV_U8)((registerStruct->lbaLowRegister >> 8) & 0xff);
        }
        else
        {
            /* LBA 28 error handling */
            SenseData->InformationDesc.information[4] =  (MV_U8)((registerStruct->deviceRegister) & 0x0f);
        }
        SenseData->InformationDesc.information[5] = (MV_U8)(registerStruct->lbaHighRegister & 0xff);
        SenseData->InformationDesc.information[6] = (MV_U8)(registerStruct->lbaMidRegister & 0xff);
        SenseData->InformationDesc.information[7] = (MV_U8)(registerStruct->lbaLowRegister & 0xff);
}
static MV_BOOLEAN checkLBAOutOfRange(IN MV_SATA_ADAPTER*    pSataAdapter,
                                     IN MV_SATA_SCSI_CMD_BLOCK *pScb,
                                     IN MV_U64 ATADiskSize, IN MV_U64 LBA,
                                     IN MV_U32 sectors)
{
    if ((ATADiskSize <= LBA) ||  ((ATADiskSize - LBA) < sectors) || sectors > 0xFFFF)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Error LBA out of range. DiskSize %x sectors %x LBA %x\n"
                 , ATADiskSize, sectors, LBA);

        setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                     SCSI_ADSENSE_ILLEGAL_BLOCK, 0);
        pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->dataTransfered = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_TRUE;

    }
    return MV_FALSE;
}
/*******************************************************************************
* mvScsiAtaGetInquiryData - Get the SCSI-3 standard inquiry(12h) data
*
* DESCRIPTION: This function fills the data buffer with Scsi standard inquiry
*       data according to the ATA Identify data
*
* INPUT:
*   pSataAdapter    - pointer to the SATA adapter data structure.
*   pScb->bus    - the index of the specific SATA channel.
*   pScb            - pointer to the Scsi command block.
*
* RETURN:
*   MV_TRUE on success, MV_FALSE on failure.
*
* COMMENTS:
*   No sanity check is done for the parameters.
*
*******************************************************************************/

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaGetInquiryData(IN  MV_SATA_ADAPTER*    pSataAdapter,
                                                            IN  MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    MV_U8           buff[42];
    MV_U32          inquiryLen;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];

    memset(buff, 0, 42);

    if ((pScb->ScsiCdb[1] & (MV_BIT0 | MV_BIT1)) ||
        (pScb->ScsiCdb[2]))
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%d %d: Inquiry completed with error: cmd[1] %x cmd[2] %x\n",
                 pSataAdapter->adapterId, pScb->bus, pScb->ScsiCdb[1],
                 pScb->ScsiCdb[2]);
        if (pDriveData->UAConditionPending == MV_TRUE)
        {

            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Unit Attention condition is pending.\n");

            if (pDriveData->UAEvents & MV_BIT0)
            {
                mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Report Bus Reset.\n");

                pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_UA_RESET;
                setSenseData(pScb, SCSI_SENSE_UNIT_ATTENTION, SCSI_ADSENSE_BUS_RESET
                             , 2);
                pDriveData->UAEvents &= ~MV_BIT0;
            }
            else if (pDriveData->UAEvents & MV_BIT1)
            {
                mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Report Mode Parameters Changed.\n");
                pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_UA_PARAMS_CHANGED;
                setSenseData(pScb, SCSI_SENSE_UNIT_ATTENTION,
                             SCSI_ADSENSE_PARAMETERS_CHANGED, 1);
                pDriveData->UAEvents &= ~MV_BIT1;
            }

            pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
            pScb->dataTransfered = 0;
#ifdef MV_LOGGER
            reportScbCompletion(pSataAdapter, pScb);
#endif
            pScb->completionCallBack(pSataAdapter, pScb);
            if (pDriveData->UAEvents == 0)
            {
                pDriveData->UAConditionPending = MV_FALSE;
            }
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }

        setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST, SCSI_ADSENSE_INVALID_CDB,
                     0);
        pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->dataTransfered = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    if (pScb->lun)
    {
        buff[0] = 0x7f;
        inquiryLen = 5;
    }
    else
    {
        MV_U8   Vendor[9],Product[17], temp[24];
        buff[0] = MV_SCSI_DIRECT_ACCESS_DEVICE;
        buff[1] = 0;    /* Not Removable disk */
        buff[2] = 5;    /*claim conformance to SCSI-3*/
        buff[3] = 2;    /* set RESPONSE DATA FORMAT to 2*/
        buff[4] = 41 - 4; /* n - 4, n start from 0 */
#if 0
        buff[6] = 0x80;     /* basic queuing*/
        buff[7] = 0;
#else
        buff[6] = 0x0;     /* tagged queuing*/
        buff[7] = 2;
#endif
        memcpy(temp, pDriveData->identifyInfo.model, 24);
        mvAta2HostString((MV_U16 *)temp, (MV_U16 *)(temp), 12);
        {
            MV_U32 i;
            for (i = 0; i < 9; i++)
            {
                if (temp[i] == ' ')
                {
                    break;
                }
            }
            if (i == 9)
            {
                if (((temp[0] == 'I') && (temp[1] == 'C')) ||
                    ((temp[0] == 'H') && (temp[1] == 'T')) ||
                    ((temp[0] == 'H') && (temp[1] == 'D')) ||
                    ((temp[0] == 'D') && (temp[1] == 'K')))
                { /*Hitachi*/
                    Vendor[0] = 'H';
                    Vendor[1] = 'i';
                    Vendor[2] = 't';
                    Vendor[3] = 'a';
                    Vendor[4] = 'c';
                    Vendor[5] = 'h';
                    Vendor[6] = 'i';
                    Vendor[7] = ' ';
                    Vendor[8] = '\0';
                }
                else if ((temp[0] == 'S') && (temp[1] == 'T'))
                {
                    /*Seagate*/
                    Vendor[0] = 'S';
                    Vendor[1] = 'e';
                    Vendor[2] = 'a';
                    Vendor[3] = 'g';
                    Vendor[4] = 'a';
                    Vendor[5] = 't';
                    Vendor[6] = 'e';
                    Vendor[7] = ' ';
                    Vendor[8] = '\0';
                }
                else
                {
                    /*Unkown*/
                    Vendor[0] = 'A';
                    Vendor[1] = 'T';
                    Vendor[2] = 'A';
                    Vendor[3] = ' ';
                    Vendor[4] = ' ';
                    Vendor[5] = ' ';
                    Vendor[6] = ' ';
                    Vendor[7] = ' ';
                    Vendor[8] = '\0';
                }
                memcpy(Product, temp, 16);
                Product[16] = '\0';
            }
            else
            {
                MV_U32 j = i;
                memcpy(Vendor, temp, j);
                for (; j < 9; j++)
                {
                    Vendor[j] = ' ';
                }
                Vendor[8] = '\0';
                for (; i < 24; i++)
                {
                    if (temp[i] != ' ')
                    {
                        break;
                    }
                }
                memcpy(Product, &temp[i], 24 - i);
                Product[16] = '\0';
            }
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Vendor %s Product %s\n", Vendor, Product);
            memcpy(&buff[8], Vendor, 8);
            memcpy(&buff[16], Product, 16);
            memcpy(&buff[32], pDriveData->identifyInfo.firmware, 4);
            mvAta2HostString((MV_U16 *)(&buff[32]), (MV_U16 *)(&buff[32]), 2);
        }
        memcpy(&buff[36], "MVSATA", 6);

        /*buff[32] = '3';*/

        inquiryLen = 42;
    }
    if (pScb->dataBufferLength > inquiryLen)
    {
        memcpy(pScb->pDataBuffer, buff, inquiryLen);
        pScb->dataTransfered = inquiryLen;
    }
    else
    {
        memcpy(pScb->pDataBuffer, buff, pScb->dataBufferLength);
        pScb->dataTransfered = pScb->dataBufferLength;
    }
    pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
    pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
    pScb->senseDataLength = 0;
#ifdef MV_LOGGER
    reportScbCompletion(pSataAdapter, pScb);
#endif
    pScb->completionCallBack(pSataAdapter, pScb);
    return MV_SCSI_COMMAND_STATUS_COMPLETED;
}

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaTestUnitReady(IN  MV_SATA_ADAPTER*    pSataAdapter,
                                                           IN  MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
    pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
    pScb->senseDataLength = 0;
    pScb->dataTransfered = 0;

#ifdef MV_LOGGER
    reportScbCompletion(pSataAdapter, pScb);
#endif
    pScb->completionCallBack(pSataAdapter, pScb);
    return MV_SCSI_COMMAND_STATUS_COMPLETED;
}


static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaSendDataCommand(IN  MV_SATA_ADAPTER*    pSataAdapter,
                                                             IN  MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    MV_U8           *cmd = pScb->ScsiCdb;
    MV_QUEUE_COMMAND_RESULT    result;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];
    MV_SAL_ADAPTER_EXTENSION *pAdapterExt = pScb->pSalAdapterExtension;
    MV_U32                  sectors;
    MV_U64		    LBA;
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO *pCommandInfo = pScb->pCommandInfo;
    MV_UDMA_COMMAND_PARAMS  *pUdmaParams = &pCommandInfo->commandParams.udmaCommand;
    pCommandInfo->type = MV_QUEUED_COMMAND_TYPE_UDMA;
    pCommandInfo->PMPort = pScb->target;
    pUdmaParams->readWrite = MV_UDMA_TYPE_WRITE;
    pUdmaParams->isEXT = MV_FALSE;
    pUdmaParams->FUA = MV_FALSE;
    pUdmaParams->highLBAAddress = 0;
    pUdmaParams->callBack = SALCommandCompletionCB;
    pUdmaParams->commandId = (MV_VOID_PTR )pScb;
#ifdef MV_SATA_SUPPORT_EDMA_SINGLE_DATA_REGION
    pUdmaParams->singleDataRegion = pScb->singleDataRegion;
    pUdmaParams->byteCount = pScb->byteCount;
#endif
#else    
    MV_QUEUE_COMMAND_INFO commandInfo =
    {
        MV_QUEUED_COMMAND_TYPE_UDMA,
        pScb->target,
        {
            {
                MV_UDMA_TYPE_WRITE, MV_FALSE, MV_FALSE, 0, 0, 0, 0, 0,
#ifdef MV_SATA_SUPPORT_EDMA_SINGLE_DATA_REGION
                pScb->singleDataRegion,
                pScb->byteCount,
#endif
                SALCommandCompletionCB, (MV_VOID_PTR )pScb
            }
        }
    };
    MV_QUEUE_COMMAND_INFO *pCommandInfo = &commandInfo;
    MV_UDMA_COMMAND_PARAMS  *pUdmaParams = &pCommandInfo->commandParams.udmaCommand;
#endif

    if ((cmd[0] == SCSI_OPCODE_READ6) || (cmd[0] == SCSI_OPCODE_WRITE6))
    {
        pUdmaParams->lowLBAAddress =
        ( (MV_U32)  cmd[3]) |
        (((MV_U32)  cmd[2]) << 8) |
        ((((MV_U32) cmd[1]) & 0x1f) << 16);
        sectors = (MV_U16) cmd[4];
    }
    else if ((cmd[0] == SCSI_OPCODE_READ10) || (cmd[0] == SCSI_OPCODE_WRITE10))
    {
        pUdmaParams->lowLBAAddress =
        (((MV_U32) cmd[5]) << 0) |
        (((MV_U32) cmd[4]) << 8) |
        (((MV_U32) cmd[3]) << 16) |
        (((MV_U32) cmd[2]) << 24);

        sectors = ((MV_U16) cmd[8]) |
		(((MV_U16) cmd[7]) << 8);
        if (cmd[1] & MV_BIT3)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%d %d: READ10/WRITE10 command "
                     "received with FUA\n",
                     pSataAdapter->adapterId, pScb->bus);
            pUdmaParams->FUA = MV_TRUE;
        }
    }
    else
    {
        pUdmaParams->lowLBAAddress =
        (((MV_U32) cmd[9]) << 0) |
        (((MV_U32) cmd[8]) << 8) |
        (((MV_U32) cmd[7]) << 16) |
        (((MV_U32) cmd[6]) << 24);

        pUdmaParams->highLBAAddress =
        (((MV_U32) cmd[5]) << 0) |
        (((MV_U32) cmd[4]) << 8) |
        (((MV_U32) cmd[3]) << 16) |
        (((MV_U32) cmd[2]) << 24);

        sectors = (cmd[13]) |
		  (cmd[12] << 8) |
		  (cmd[11] << 16) |
		  (cmd[10] << 24);

        if (cmd[1] & MV_BIT3)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%d %d: READ16/WRITE16 command "
                     "received with FUA\n",
                     pSataAdapter->adapterId, pScb->bus);
            pUdmaParams->FUA = MV_TRUE;
        }
    }
    LBA = ((MV_U64)pUdmaParams->highLBAAddress << 32) | (MV_U64)pUdmaParams->lowLBAAddress;
    pScb->isExtended = pUdmaParams->isEXT = pDriveData->identifyInfo.LBA48Supported;

    /* If READ10 / WRITE10 with 0 sectors (no data transfer), then complete */
    /* the command with OK.                                                 */
    /* If READ6 / WRITE6 with 0 sectors, seemse the Windows have problem with */
    /* this and doesn't allocate and buffers for this ; so complete this    */
    /* command with ILLEGAL REQUEST sense and INVLAID CDB in addition sense */
    /* code.                                                                */

    if (sectors == 0)
    {
        if ((cmd[0] == SCSI_OPCODE_READ10) || (cmd[0] == SCSI_OPCODE_WRITE10) || 
	    (cmd[0] == SCSI_OPCODE_WRITE16) || (cmd[0] == SCSI_OPCODE_WRITE16))
        {

            if (checkLBAOutOfRange(pSataAdapter, pScb,
                                   pDriveData->identifyInfo.ATADiskSize,
                                   LBA, 0) == MV_TRUE)
            {
                return MV_SCSI_COMMAND_STATUS_COMPLETED;
            }
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
            pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
            pScb->dataTransfered = 0;
            pScb->senseDataLength = 0;
#ifdef MV_LOGGER
            reportScbCompletion(pSataAdapter, pScb);
#endif
            pScb->completionCallBack(pSataAdapter, pScb);
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }
        else
        {
            /* READ6 / WRITE6 with sector count 0, which means 256 sectors */
            sectors = 256;
        }
    }
    if (checkLBAOutOfRange(pSataAdapter, pScb,
                           pDriveData->identifyInfo.ATADiskSize,
                           LBA, sectors) == MV_TRUE)
    {
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    /* If trying to send more than 256 sectors or DataTransferLength field is
     * not equal to number of sectors request in CDB then return invalid
     * request.
     */

    if (((sectors > 256) && (pUdmaParams->isEXT == MV_FALSE)) ||
        ((sectors * ATA_SECTOR_SIZE) != pScb->dataBufferLength))
    {
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCB;
        pScb->dataTransfered = 0;
        pScb->senseDataLength = 0;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    if ((cmd[0] == SCSI_OPCODE_READ6) || (cmd[0] == SCSI_OPCODE_READ10) || 
	(cmd[0] == SCSI_OPCODE_READ16))
    {
        pUdmaParams->readWrite = MV_UDMA_TYPE_READ;
    }
    pScb->dataTransfered = sectors * ATA_SECTOR_SIZE;
    pScb->udmaType = pUdmaParams->readWrite;
    pScb->commandType = MV_QUEUED_COMMAND_TYPE_UDMA;
    pScb->LowLbaAddress = pUdmaParams->lowLBAAddress;
    pUdmaParams->numOfSectors = sectors;

    if ((sectors == 256) &&
        ((pDriveData->identifyInfo.LBA48Supported == MV_FALSE)))
    {
        pUdmaParams->numOfSectors = 0;
    }
    pUdmaParams->prdLowAddr = pScb->PRDTableLowPhyAddress;
    pUdmaParams->prdHighAddr = pScb->PRDTableHighPhyAddress;

    result = mvSataQueueCommand(pSataAdapter, pScb->bus, pCommandInfo);
    if (result != MV_QUEUE_COMMAND_RESULT_OK)
    {
        checkQueueCommandResult(pScb, result);
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    /*update statistics*/

    pAdapterExt->totalAccumulatedOutstanding[pScb->bus] +=
    mvSataNumOfDmaCommands(pSataAdapter,pScb->bus);
    pDriveData->stats.totalIOs++;
    pDriveData->stats.totalSectorsTransferred += pUdmaParams->numOfSectors;

    return MV_SCSI_COMMAND_STATUS_QUEUED;
}

/*******************************************************************************
* mvScsiAtaGetReadCapacityData - Get the SCSI-3 Read Capacity (10h/16h) data
*
* DESCRIPTION: This function fills the data buffer with Scsi Read Capacity 10 or
*       Read Capacity 16 data according to the disk size as it is reported in
*       the ATA Identify data.
*
* INPUT:
*   pSataAdapter    - pointer to the SATA adapter data structure.
*   pScb->bus    - the index of the specific SATA channel.
*
* OUTPUT:
* RETURN:
*   MV_TRUE on success, MV_FALSE on failure.
*
* COMMENTS:
*   No sanity check is done for the parameters.
*
*******************************************************************************/
static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaGetReadCapacityData(IN MV_SATA_ADAPTER*    pSataAdapter,
                                                                 IN    MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    MV_U32  lastAddressableLBA;
    MV_U8   *buff;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];

    if ((pScb->ScsiCdb[8] & MV_BIT1) == 0)
    {
        if (pScb->ScsiCdb[2] || pScb->ScsiCdb[3] ||pScb->ScsiCdb[4] ||
            pScb->ScsiCdb[5])
        {

            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Inquiry completed with error: PMI = 0, LBA != 0\n",
                     pSataAdapter->adapterId, pScb->bus);
            setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                         SCSI_ADSENSE_INVALID_CDB, 0);
            pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
            pScb->dataTransfered = 0;
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
#ifdef MV_LOGGER
            reportScbCompletion(pSataAdapter, pScb);
#endif
            pScb->completionCallBack(pSataAdapter, pScb);
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }
    }
    if((pDriveData->identifyInfo.ATADiskSize >> 32) & 0xFFFFFFFF)
    {
	    lastAddressableLBA = 0xFFFFFFFF;
    }
    else
    {
	    lastAddressableLBA = pDriveData->identifyInfo.ATADiskSize - 1;
    }
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "MVSATA: last Addressable sector = 0x%x "
             " (sec size=%d bytes)\n", lastAddressableLBA, ATA_SECTOR_SIZE);

    /* The disk size as indicated by the ATA spec is the total addressable
     * secotrs on the drive ; while the SCSI translation of the command
     * should be the last addressable sector.
     */
    buff = pScb->pDataBuffer;
    memset(buff, 0, pScb->dataBufferLength);
    buff[0] = (MV_U8)(lastAddressableLBA >> 24);
    buff[1] = (MV_U8)((lastAddressableLBA >> 16) & 0xff);
    buff[2] = (MV_U8)((lastAddressableLBA >> 8) & 0xff);
    buff[3] = (MV_U8)(lastAddressableLBA & 0xff);
    buff[4] = 0;
    buff[5] = 0;
    buff[6] = (MV_U8)((ATA_SECTOR_SIZE >> 8) & 0xff);           /* 512 byte sectors */
    buff[7] = (MV_U8)(ATA_SECTOR_SIZE & 0xff);
    pScb->dataTransfered = 8;
    pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
    pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
    pScb->senseDataLength = 0;
#ifdef MV_LOGGER
    reportScbCompletion(pSataAdapter, pScb);
#endif
    pScb->completionCallBack(pSataAdapter, pScb);
    return MV_SCSI_COMMAND_STATUS_COMPLETED;
}

/*******************************************************************************
* mvScsiAtaGetReadCapacity16Data - Get the SCSI-3 Read Capacity (16h) data
*
* DESCRIPTION: This function fills the data buffer with Scsi Read Capacity 16
* data according to the disk size as it is reported in
*       the ATA Identify data.
*
* INPUT:
*   pSataAdapter    - pointer to the SATA adapter data structure.
*   pScb->bus    - the index of the specific SATA channel.
*
* OUTPUT:
* RETURN:
*   MV_TRUE on success, MV_FALSE on failure.
*
* COMMENTS:
*   No sanity check is done for the parameters.
*
*******************************************************************************/
static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaGetReadCapacity16Data(IN MV_SATA_ADAPTER*    pSataAdapter,
                                                                 IN    MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    MV_U64  lastAddressableLBA;
    MV_U8   *buff;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];

    if ((pScb->ScsiCdb[14] & MV_BIT1) == 0)
    {
        if (pScb->ScsiCdb[2] || pScb->ScsiCdb[3] ||pScb->ScsiCdb[4] ||
            pScb->ScsiCdb[5] || pScb->ScsiCdb[6] ||pScb->ScsiCdb[7] ||
	    pScb->ScsiCdb[8] || pScb->ScsiCdb[9])
        {

            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Read Capacity completed with error: PMI = 0, LBA != 0\n",
                     pSataAdapter->adapterId, pScb->bus);
            setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                         SCSI_ADSENSE_INVALID_CDB, 0);
            pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
            pScb->dataTransfered = 0;
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
#ifdef MV_LOGGER
            reportScbCompletion(pSataAdapter, pScb);
#endif
            pScb->completionCallBack(pSataAdapter, pScb);
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }
    }

    lastAddressableLBA = pDriveData->identifyInfo.ATADiskSize - 1;
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "MVSATA: last Addressable sector = 0x%x "
             " (sec size=%d bytes)\n", lastAddressableLBA, ATA_SECTOR_SIZE);

    /* The disk size as indicated by the ATA spec is the total addressable
     * secotrs on the drive ; while the SCSI translation of the command
     * should be the last addressable sector.
     */
    buff = pScb->pDataBuffer;
    memset(buff, 0, pScb->dataBufferLength);
    buff[0] = (MV_U8)((lastAddressableLBA >> 56) & 0xff);
    buff[1] = (MV_U8)((lastAddressableLBA >> 48) & 0xff);
    buff[2] = (MV_U8)((lastAddressableLBA >> 40) & 0xff);
    buff[3] = (MV_U8)((lastAddressableLBA >> 32) & 0xff);
    buff[4] = (MV_U8)((lastAddressableLBA >> 24) & 0xff);
    buff[5] = (MV_U8)((lastAddressableLBA >> 16) & 0xff);
    buff[6] = (MV_U8)((lastAddressableLBA >> 8) & 0xff);
    buff[7] = (MV_U8)(lastAddressableLBA & 0xff);
    buff[8] = 0;
    buff[9] = 0;
    buff[10] = (MV_U8)((ATA_SECTOR_SIZE >> 8) & 0xff);           /* 512 byte sectors */
    buff[11] = (MV_U8)(ATA_SECTOR_SIZE & 0xff);
    pScb->dataTransfered = 8;
    pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
    pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
    pScb->senseDataLength = 0;
#ifdef MV_LOGGER
    reportScbCompletion(pSataAdapter, pScb);
#endif
    pScb->completionCallBack(pSataAdapter, pScb);
    return MV_SCSI_COMMAND_STATUS_COMPLETED;
}


/*******************************************************************************
* mvScsiAtaReportLuns - handle the SCSI-3 Report LUNS
*
* DESCRIPTION: Report 1 LUN
*
* INPUT:
*   pSataAdapter    - pointer to the SATA adapter data structure.
*   pScb->bus    - the index of the specific SATA channel.
*
* OUTPUT:
* RETURN:
*   MV_TRUE on success, MV_FALSE on failure.
*
* COMMENTS:
*   No sanity check is done for the parameters.
*
*******************************************************************************/
static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaReportLuns(IN MV_SATA_ADAPTER*    pSataAdapter,
							IN    MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
	MV_U8   *buff;


    buff = pScb->pDataBuffer;
    memset(buff, 0, pScb->dataBufferLength);
    buff[3] = 8; /* 1 lun*/
    pScb->dataTransfered = 16;
    pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
    pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
    pScb->senseDataLength = 0;
#ifdef MV_LOGGER
    reportScbCompletion(pSataAdapter, pScb);
#endif
    pScb->completionCallBack(pSataAdapter, pScb);
    return MV_SCSI_COMMAND_STATUS_COMPLETED;
}

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaSendVerifyCommand(IN MV_SATA_ADAPTER*    pSataAdapter,
                                                               IN MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = pScb->pCommandInfo;
#else
    MV_QUEUE_COMMAND_INFO   commandInfo;
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = &commandInfo;
#endif
    MV_QUEUE_COMMAND_RESULT result;
    MV_U8                   *cmd = pScb->ScsiCdb;
    MV_U64                   LbaAddress;
    MV_U32                  sectors;
    MV_U16                  commands;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];
    MV_SAL_ADAPTER_EXTENSION *pAdapterExt = pScb->pSalAdapterExtension;

    if (cmd[0] == SCSI_OPCODE_VERIFY6)
    {
	    LbaAddress =
		    ( (unsigned)  cmd[3]) |
		    (((unsigned)  cmd[2]) << 8) |
		    ((((unsigned) cmd[1]) & 0x1f) << 16);
	    sectors = (unsigned) cmd[4];
    }
    else if (cmd[0] == SCSI_OPCODE_VERIFY10)
    {
	    LbaAddress =
		    (((unsigned) cmd[5]) << 0) |
		    (((unsigned) cmd[4]) << 8) |
		    (((unsigned) cmd[3]) << 16) |
		    (((unsigned) cmd[2]) << 24);
	    
	    
	    sectors = ((unsigned) cmd[8]) |
		    (((unsigned) cmd[7]) << 8);
    }
    else
    {
	    LbaAddress =
		    (((MV_U64) cmd[9]) << 0) |
		    (((MV_U64) cmd[8]) << 8) |
		    (((MV_U64) cmd[7]) << 16) |
		    (((MV_U64) cmd[6]) << 24) |
		    (((MV_U64) cmd[5]) << 32) |
		    (((MV_U64) cmd[4]) << 40) |
		    (((MV_U64) cmd[3]) << 48) |
		    (((MV_U64) cmd[2]) << 56);


	    sectors = 
		    ((unsigned) cmd[13]) |
		    (((unsigned) cmd[12]) << 8) |
		    (((unsigned) cmd[11]) << 16) |
		    (((unsigned) cmd[10]) << 24);
    }

    if (sectors == 0)
    {
        if ((cmd[0] == SCSI_OPCODE_VERIFY10) || (cmd[0] == SCSI_OPCODE_VERIFY16))
        {
            if (checkLBAOutOfRange(pSataAdapter, pScb,
                                   pDriveData->identifyInfo.ATADiskSize,
                                   LbaAddress, 0) == MV_TRUE)
            {
                return MV_SCSI_COMMAND_STATUS_COMPLETED;
            }
            pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
            pScb->senseDataLength = 0;
            pScb->dataTransfered = 0;
#ifdef MV_LOGGER
            reportScbCompletion(pSataAdapter, pScb);
#endif
            pScb->completionCallBack(pSataAdapter, pScb);
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }
        else
        {
            /* If VERIFY6 to 48bit device, then 256 sectors is OK ; otherwise
            the CORE driver must get sector count of 0 in order to understand that
            256 sectors must be transferred
            */

            if (pDriveData->identifyInfo.LBA48Supported == MV_TRUE)
            {
                sectors = 256;
            }
            else
            {
                sectors = 0;
            }
        }
        if (checkLBAOutOfRange(pSataAdapter, pScb,
                               pDriveData->identifyInfo.ATADiskSize,
                               LbaAddress, 256) == MV_TRUE)
        {
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }
    }
    else
    {
        if (checkLBAOutOfRange(pSataAdapter, pScb,
                               pDriveData->identifyInfo.ATADiskSize,
                               LbaAddress, sectors) == MV_TRUE)
        {
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }
    }

    pScb->commandType = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
    pScb->LowLbaAddress = (MV_U32)(LbaAddress & 0xFFFFFFFF);
//    pScb->highLbaAddress = (MV_U32)(LbaAddress >> 32);

    if (pDriveData->identifyInfo.LBA48Supported == MV_TRUE)
    {
        pScb->splitCount = 1;
        pScb->isExtended = MV_TRUE;
        pCommandInfo->type = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
        pCommandInfo->PMPort = pScb->target;
        pCommandInfo->commandParams.NoneUdmaCommand.bufPtr = NULL;
        pCommandInfo->commandParams.NoneUdmaCommand.callBack = SALCommandCompletionCB;
        pCommandInfo->commandParams.NoneUdmaCommand.command = MV_ATA_COMMAND_READ_VERIFY_SECTORS_EXT;
        pCommandInfo->commandParams.NoneUdmaCommand.commandId = (MV_VOID_PTR) pScb;
        pCommandInfo->commandParams.NoneUdmaCommand.count = 0;
        pCommandInfo->commandParams.NoneUdmaCommand.features = 0;
        pCommandInfo->commandParams.NoneUdmaCommand.isEXT = MV_TRUE;
        pCommandInfo->commandParams.NoneUdmaCommand.lbaHigh = 
	(MV_U16)(((LbaAddress & 0xff0000000000ULL) >> 40) | ((LbaAddress & 0xff0000) >> 16));
        pCommandInfo->commandParams.NoneUdmaCommand.lbaMid = 
        (MV_U16)(((LbaAddress & 0xff00000000ULL) >> 32) | ((LbaAddress & 0xff00) >> 8));
        pCommandInfo->commandParams.NoneUdmaCommand.lbaLow =
        (MV_U16)(((LbaAddress & 0xff000000) >> 24) | (LbaAddress & 0xff));
        pCommandInfo->commandParams.NoneUdmaCommand.protocolType = MV_NON_UDMA_PROTOCOL_NON_DATA;
        pCommandInfo->commandParams.NoneUdmaCommand.sectorCount = sectors;
        pCommandInfo->commandParams.NoneUdmaCommand.device = (MV_U8)(MV_BIT6);

        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Sending EXT Verify command: channel %d, code %x lba %x(%x.%x.%x), sectors %d[%d] Srb %p\n",
                 pScb->bus, cmd[0], LbaAddress,
                 pCommandInfo->commandParams.NoneUdmaCommand.lbaHigh,
                 pCommandInfo->commandParams.NoneUdmaCommand.lbaMid,
                 pCommandInfo->commandParams.NoneUdmaCommand.lbaLow,
                 pCommandInfo->commandParams.NoneUdmaCommand.sectorCount,
                 mvSataNumOfDmaCommands(pSataAdapter,pScb->bus), pScb);

        result = mvSataQueueCommand(pSataAdapter, pScb->bus, pCommandInfo);

        if (result != MV_QUEUE_COMMAND_RESULT_OK)
        {
            checkQueueCommandResult(pScb, result);
#ifdef MV_LOGGER
            reportScbCompletion(pSataAdapter, pScb);
#endif
            pScb->completionCallBack(pSataAdapter, pScb);
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }
        /* update stats*/
        pAdapterExt->totalAccumulatedOutstanding[pScb->bus] +=
        mvSataNumOfDmaCommands(pSataAdapter,pScb->bus);
        pDriveData->stats.totalIOs++;
        pDriveData->stats.totalSectorsTransferred += sectors;

    }
    else
    {
        /* The following only in case command is VERIFY 6 with 0 sector count */
        if (sectors == 0)
        {
            commands = 1;
        }
        else
        {
            commands = (MV_U16)((((MV_U32)sectors + 0xff) & 0x1ff00) >> 8);
        }

        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "split Verify to %d commands: channel %d, lba %x, sectors %d\n",
                 commands,pScb->bus, LbaAddress, sectors);

        pScb->splitCount = commands;
        pScb->sequenceNumber = 0;
        pScb->isExtended = MV_FALSE;
        mvScsiAtaSendSplittedVerifyCommand(pScb);
    }
    return MV_SCSI_COMMAND_STATUS_QUEUED;
}

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaSeek(IN MV_SATA_ADAPTER    *pSataAdapter,
                                                  IN  MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    MV_U32 lbaAddress;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];

    lbaAddress = (((MV_U32) pScb->ScsiCdb[5]) << 0) |
                 (((MV_U32) pScb->ScsiCdb[4]) << 8) |
                 (((MV_U32) pScb->ScsiCdb[3]) << 16) |
                 (((MV_U32) pScb->ScsiCdb[2]) << 24);
    if (checkLBAOutOfRange(pSataAdapter, pScb,
                           pDriveData->identifyInfo.ATADiskSize,
                           lbaAddress, 0) == MV_TRUE)
    {
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }
    pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
    pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
    pScb->dataTransfered = 0;
    pScb->senseDataLength = 0;
#ifdef MV_LOGGER
    reportScbCompletion(pSataAdapter, pScb);
#endif
    pScb->completionCallBack(pSataAdapter, pScb);
    return MV_SCSI_COMMAND_STATUS_COMPLETED;

}

static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaReassignBlocks(IN MV_SATA_ADAPTER    *pSataAdapter,
                                                            IN MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    setSenseData(pScb, SCSI_SENSE_HARDWARE_ERROR, 0x32, 0);
    pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
    pScb->dataTransfered = 0;
    pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_ATA_FAILED;
#ifdef MV_LOGGER
    reportScbCompletion(pSataAdapter, pScb);
#endif
    pScb->completionCallBack(pSataAdapter, pScb);
    return MV_TRUE;

}


#ifdef MV_SATA_SUPPORT_READ_WRITE_LONG
static MV_SCSI_COMMAND_STATUS_TYPE mvScsiAtaWriteLong(IN MV_SATA_ADAPTER    *pSataAdapter,
                                                      IN  MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = pScb->pCommandInfo;
#else
    MV_QUEUE_COMMAND_INFO   commandInfo;
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = &commandInfo;
#endif
    MV_QUEUE_COMMAND_RESULT result;
    MV_U32                  LBA;
    MV_U16                  eccBytes;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];
    MV_SAL_ADAPTER_EXTENSION *pAdapterExt = pScb->pSalAdapterExtension;

    memset(pCommandInfo, 0, sizeof(MV_QUEUE_COMMAND_INFO));


    LBA = (((MV_U32) pScb->ScsiCdb[5]) << 0) |
          (((MV_U32) pScb->ScsiCdb[4]) << 8) |
          (((MV_U32) pScb->ScsiCdb[3]) << 16) |
          (((MV_U32) pScb->ScsiCdb[2]) << 24);

    eccBytes = (MV_U16)pScb->ScsiCdb[8];

    if ((pScb->ScsiCdb[7] != 2) || ((eccBytes != 4) && (eccBytes != 8)))
    {
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCB;
        pScb->dataTransfered = 0;
        pScb->senseDataLength = 0;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }
    if (checkLBAOutOfRange(pSataAdapter, pScb,
                           pDriveData->identifyInfo.ATADiskSize,
                           LBA, 1) == MV_TRUE)
    {
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    /*if (checkLBAOutOfRange(pSataAdapter, pScb, MV_BIT28 - 2,
                        LBA, 1) == MV_TRUE)
    {
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }*/
    if (LBA & (MV_BIT31|MV_BIT30|MV_BIT29|MV_BIT28))
    {

        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Error LBA (0x%x) out of range.\n", LBA);

        setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                     SCSI_ADSENSE_ILLEGAL_BLOCK, 0);
        pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->dataTransfered = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_TRUE;
    }
    pScb->commandType = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;

    pCommandInfo->type = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
    pCommandInfo->PMPort = pScb->target;
    pCommandInfo->commandParams.NoneUdmaCommand.bufPtr = (MV_U16_PTR)pScb->pDataBuffer;
    pCommandInfo->commandParams.NoneUdmaCommand.isEXT = MV_FALSE;
    pCommandInfo->commandParams.NoneUdmaCommand.callBack = SALCommandCompletionCB;
    pCommandInfo->commandParams.NoneUdmaCommand.commandId = (MV_VOID_PTR) pScb;
    pCommandInfo->commandParams.NoneUdmaCommand.count = 256+4;
    pCommandInfo->commandParams.NoneUdmaCommand.features = eccBytes;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaHigh = (MV_U16)((LBA & 0xff0000) >> 16);
    pCommandInfo->commandParams.NoneUdmaCommand.lbaMid = (MV_U16)((LBA & 0xff00) >> 8) ;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaLow = (MV_U16)LBA & 0xff;
    pCommandInfo->commandParams.NoneUdmaCommand.protocolType = MV_NON_UDMA_PROTOCOL_PIO_DATA_OUT;
    pCommandInfo->commandParams.NoneUdmaCommand.sectorCount = 1;
    pCommandInfo->commandParams.NoneUdmaCommand.device = MV_BIT6 | (MV_U16)((LBA & 0xf000000) >> 24) ;
    pScb->isExtended = MV_FALSE;
    pCommandInfo->commandParams.NoneUdmaCommand.command = 0x32;


    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Sending WRITE LONG command : channel %d, code %x pScb %p\n",
             pScb->bus, pScb->ScsiCdb[0], pScb);

    result = mvSataQueueCommand(pSataAdapter, pScb->bus, pCommandInfo);
    if (result != MV_QUEUE_COMMAND_RESULT_OK)
    {
        checkQueueCommandResult(pScb, result);
        /* shoudl complete the Scsi request here*/
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }
    /* update stats*/
    pAdapterExt->totalAccumulatedOutstanding[pScb->bus] += mvSataNumOfDmaCommands(pSataAdapter,pScb->bus);
    pDriveData->stats.totalIOs++;

    return MV_SCSI_COMMAND_STATUS_QUEUED;
}
static MV_SCSI_COMMAND_STATUS_TYPE mvScsiAtaReadLong(IN MV_SATA_ADAPTER    *pSataAdapter,
                                                     IN  MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = pScb->pCommandInfo;
#else
    MV_QUEUE_COMMAND_INFO   commandInfo;
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = &commandInfo;
#endif
    MV_QUEUE_COMMAND_RESULT result;
    MV_U32                  LBA;
    MV_U16                  eccBytes;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];
    MV_SAL_ADAPTER_EXTENSION *pAdapterExt = pScb->pSalAdapterExtension;

    memset(pCommandInfo, 0, sizeof(MV_QUEUE_COMMAND_INFO));


    LBA = (((MV_U32) pScb->ScsiCdb[5]) << 0) |
          (((MV_U32) pScb->ScsiCdb[4]) << 8) |
          (((MV_U32) pScb->ScsiCdb[3]) << 16) |
          (((MV_U32) pScb->ScsiCdb[2]) << 24);

    eccBytes = (MV_U16)pScb->ScsiCdb[8];

    if ((pScb->ScsiCdb[7] != 2) || ((eccBytes != 4) && (eccBytes != 8)))
    {
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCB;
        pScb->dataTransfered = 0;
        pScb->senseDataLength = 0;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }
    if (checkLBAOutOfRange(pSataAdapter, pScb,
                           pDriveData->identifyInfo.ATADiskSize,
                           LBA, 1) == MV_TRUE)
    {
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    /*if (checkLBAOutOfRange(pSataAdapter, pScb, MV_BIT28 - 2,
                        LBA, 1) == MV_TRUE)
    {
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }*/
    if (LBA & (MV_BIT31|MV_BIT30|MV_BIT29|MV_BIT28))
    {

        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Error LBA (0x%x) out of range.\n", LBA);

        setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                     SCSI_ADSENSE_ILLEGAL_BLOCK, 0);
        pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->dataTransfered = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_TRUE;
    }
    pScb->commandType = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;

    pCommandInfo->type = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
    pCommandInfo->PMPort = pScb->target;
    pCommandInfo->commandParams.NoneUdmaCommand.bufPtr = (MV_U16_PTR)pScb->pDataBuffer;
    pCommandInfo->commandParams.NoneUdmaCommand.isEXT = MV_FALSE;
    pCommandInfo->commandParams.NoneUdmaCommand.callBack = SALCommandCompletionCB;
    pCommandInfo->commandParams.NoneUdmaCommand.commandId = (MV_VOID_PTR) pScb;
    pCommandInfo->commandParams.NoneUdmaCommand.count = 256+4;
    pCommandInfo->commandParams.NoneUdmaCommand.features = eccBytes;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaHigh = (MV_U16)((LBA & 0xff0000) >> 16);
    pCommandInfo->commandParams.NoneUdmaCommand.lbaMid = (MV_U16)((LBA & 0xff00) >> 8) ;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaLow = (MV_U16)LBA & 0xff;
    pCommandInfo->commandParams.NoneUdmaCommand.protocolType = MV_NON_UDMA_PROTOCOL_PIO_DATA_IN;
    pCommandInfo->commandParams.NoneUdmaCommand.sectorCount = 1;
    pCommandInfo->commandParams.NoneUdmaCommand.device = MV_BIT6 | (MV_U16)((LBA & 0xf000000) >> 24) ;
    pScb->isExtended = MV_FALSE;
    pCommandInfo->commandParams.NoneUdmaCommand.command = 0x22;


    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Sending READ LONG command : channel %d, code %x pScb %p\n",
             pScb->bus, pScb->ScsiCdb[0], pScb);

    result = mvSataQueueCommand(pSataAdapter, pScb->bus, pCommandInfo);
    if (result != MV_QUEUE_COMMAND_RESULT_OK)
    {
        checkQueueCommandResult(pScb, result);
        /* shoudl complete the Scsi request here*/
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }
    /* update stats*/
    pAdapterExt->totalAccumulatedOutstanding[pScb->bus] += mvSataNumOfDmaCommands(pSataAdapter,pScb->bus);
    pDriveData->stats.totalIOs++;

    return MV_SCSI_COMMAND_STATUS_QUEUED;
}
#endif
static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaSendSyncCacheCommand(IN MV_SATA_ADAPTER    *pSataAdapter,
                                                                      IN  MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = pScb->pCommandInfo;
#else
    MV_QUEUE_COMMAND_INFO   commandInfo;
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = &commandInfo;
#endif
    MV_QUEUE_COMMAND_RESULT result;
    MV_U64                  LBA;
    MV_U32                  sectors;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];
    MV_SAL_ADAPTER_EXTENSION *pAdapterExt = pScb->pSalAdapterExtension;

    /* Check if IMMED bit is set, if so then return ILLEGAL REQUEST */
    if (pScb->ScsiCdb[1] & MV_BIT1)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Synchronise cache completed with error:"
                 " IMMED is set\n", pSataAdapter->adapterId,
                 pScb->bus);

        setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST, SCSI_ADSENSE_INVALID_CDB,
                     0);
        pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->dataTransfered = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    memset(pCommandInfo, 0, sizeof(MV_QUEUE_COMMAND_INFO));

    if(pScb->ScsiCdb[0] == SCSI_OPCODE_SYNCHRONIZE_CACHE10)
    {
	    LBA = (((MV_U64) pScb->ScsiCdb[5]) << 0) |
		    (((MV_U64) pScb->ScsiCdb[4]) << 8) |
		    (((MV_U64) pScb->ScsiCdb[3]) << 16) |
		    (((MV_U64) pScb->ScsiCdb[2]) << 24);
	    
	    sectors = ((MV_U32) pScb->ScsiCdb[8]) |
		    (((MV_U32) pScb->ScsiCdb[7]) << 8);
    }
    else
    {
	    LBA = (((MV_U64) pScb->ScsiCdb[9]) << 0) |
		    (((MV_U64) pScb->ScsiCdb[8]) << 8) |
		    (((MV_U64) pScb->ScsiCdb[7]) << 16) |
		    (((MV_U64) pScb->ScsiCdb[6]) << 24) |
		    (((MV_U64) pScb->ScsiCdb[5]) << 32) |
		    (((MV_U64) pScb->ScsiCdb[4]) << 40) |
		    (((MV_U64) pScb->ScsiCdb[3]) << 48) |
		    (((MV_U64) pScb->ScsiCdb[2]) << 56);
	    
	    sectors = ((MV_U32) pScb->ScsiCdb[13]) |
		    (((MV_U32) pScb->ScsiCdb[12]) << 8) |
		    (((MV_U32) pScb->ScsiCdb[11]) << 16) |
		    (((MV_U32) pScb->ScsiCdb[10]) << 24);
    }
    if (checkLBAOutOfRange(pSataAdapter, pScb,
                           pDriveData->identifyInfo.ATADiskSize,LBA, sectors)
        == MV_TRUE)
    {
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    pScb->commandType = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;

    pCommandInfo->type = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
    pCommandInfo->PMPort = pScb->target;
    pCommandInfo->commandParams.NoneUdmaCommand.bufPtr = NULL;
    pCommandInfo->commandParams.NoneUdmaCommand.callBack = SALCommandCompletionCB;
    pCommandInfo->commandParams.NoneUdmaCommand.commandId = (MV_VOID_PTR) pScb;
    pCommandInfo->commandParams.NoneUdmaCommand.count = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.features = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaHigh = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaMid = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaLow = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.protocolType =
    MV_NON_UDMA_PROTOCOL_NON_DATA;
    pCommandInfo->commandParams.NoneUdmaCommand.sectorCount = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.device = (MV_U8)(MV_BIT6);

    if (pDriveData->identifyInfo.LBA48Supported == MV_TRUE)
    {
        pScb->isExtended = MV_TRUE;
        pCommandInfo->commandParams.NoneUdmaCommand.command = MV_ATA_COMMAND_FLUSH_CACHE_EXT;
        pCommandInfo->commandParams.NoneUdmaCommand.isEXT = MV_TRUE;
    }
    else
    {
        pScb->isExtended = MV_FALSE;
        pCommandInfo->commandParams.NoneUdmaCommand.command = MV_ATA_COMMAND_FLUSH_CACHE;
        pCommandInfo->commandParams.NoneUdmaCommand.isEXT = MV_FALSE;

    }
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Sending Flush Cache command : channel %d, code %x (extended -->"
             " %s) pScb %p\n", pScb->bus, pScb->ScsiCdb[0],
             (pScb->isExtended == MV_TRUE) ? "Yes":"No", pScb);

    result = mvSataQueueCommand(pSataAdapter, pScb->bus, pCommandInfo);
    if (result != MV_QUEUE_COMMAND_RESULT_OK)
    {
        checkQueueCommandResult(pScb, result);
        /* shoudl complete the Scsi request here*/
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }
    /* update stats*/
    pAdapterExt->totalAccumulatedOutstanding[pScb->bus] += mvSataNumOfDmaCommands(pSataAdapter,pScb->bus);
    pDriveData->stats.totalIOs++;

    return MV_SCSI_COMMAND_STATUS_QUEUED;
}

/*******************************************************************************
* mvScsiAtaGetRequestSenseData - Get the SCSI-3 Request Sense(03h) data
*
* DESCRIPTION: This function fills the sense buffer with a sense key of NO SENSE
*       and an additional sense code of NO ADDITIONAL SENSE INFORMATION.
*
* INPUT:
*   pSataAdapter    - pointer to the SATA adapter data structure.
*   pScb->bus    - the index of the specific SATA channel.
*   Cdb             - specifies the SCSI-3 command descriptor block.
*
* OUTPUT:
*   pScsiStatus     - pointer to the Scsi status to be returned.
*   pSenseBuffer    - pointer to the Scsi sense buffer.
*   senseBufferLength   - the size in bytes of the sense buffer.
*   pDataTransfered - the size in bytes of the data transfered into the data
*                     buffer(alwasy zero for this command).
*
* RETURN:
*   MV_TRUE on success, MV_FALSE on failure.
*
* COMMENTS:
*   No sanity check is done for the parameters.
*
*******************************************************************************/
static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaGetRequestSenseData(IN MV_SATA_ADAPTER*    pSataAdapter,
                                                                 IN MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    MV_SCSI_SENSE_DATA SenseData;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];

    memset(pScb->pDataBuffer, 0, pScb->dataBufferLength);

    memset(&SenseData, 0, sizeof(MV_SCSI_SENSE_DATA));
//    SenseData.Valid = 0;
    SenseData.ResponseCode = MV_SCSI_RESPONSE_CODE;
    SenseData.SenseKey = SCSI_SENSE_NO_SENSE;
    SenseData.AdditionalSenseCode = SCSI_ADSENSE_NO_SENSE;
    SenseData.AdditionalSenseLength = sizeof(MV_SCSI_SENSE_DATA) - 8;

    pScb->senseDataLength = 0;
    if (pDriveData->UAConditionPending == MV_TRUE)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Unit Attention condition is pending.\n");
        SenseData.SenseKey = SCSI_SENSE_UNIT_ATTENTION;
        if (pDriveData->UAEvents & MV_BIT0)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Report Bus Reset.\n");

            SenseData.AdditionalSenseCode = SCSI_ADSENSE_BUS_RESET;
            SenseData.AdditionalSenseCodeQualifier = 2;
            pDriveData->UAEvents &= ~MV_BIT0;
        }
        else if (pDriveData->UAEvents & MV_BIT1)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Report Mode Parameters Changed.\n");
            SenseData.AdditionalSenseCode = SCSI_ADSENSE_PARAMETERS_CHANGED;
            SenseData.AdditionalSenseCodeQualifier = 1;
            pDriveData->UAEvents &= ~MV_BIT1;
        }

        pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
        if (pDriveData->UAEvents == 0)
        {
            pDriveData->UAConditionPending = MV_FALSE;
        }
    }
    else
    {
        pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
    }
    if (pScb->dataBufferLength >= sizeof(MV_SCSI_SENSE_DATA))
    {
        pScb->dataTransfered = sizeof(MV_SCSI_SENSE_DATA);
        memcpy(pScb->pDataBuffer, &SenseData, pScb->dataTransfered);
        /*pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;*/
        /*pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;*/
    }
    else
    {
        pScb->dataTransfered = pScb->dataBufferLength;
        memcpy(pScb->pDataBuffer, &SenseData, pScb->dataTransfered);
        pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;/*TBD*/
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
    }
#ifdef MV_LOGGER
    reportScbCompletion(pSataAdapter, pScb);
#endif
    pScb->completionCallBack(pSataAdapter, pScb);
    return MV_SCSI_COMMAND_STATUS_COMPLETED;
}

/*******************************************************************************
* mvScsiAtaGetModeSenseData - Get the SCSI-3 Mode Sense data
*
* DESCRIPTION: This function issues ATA Identify command, in the command
*       completion, the Mode Sense data will be filled according to the returned
*       Identify Data.
*
* INPUT:
*   pSataAdapter    - pointer to the SATA adapter data structure.
*   pScb->bus    - the index of the specific SATA channel.
*   Cdb             - specifies the SCSI-3 command descriptor block.
*
* RETURN:
*   MV_TRUE on success, MV_FALSE on failure.
*
* COMMENTS:
*   No sanity check is done for the parameters.
*
*******************************************************************************/
static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaGetModeSenseData(IN MV_SATA_ADAPTER    *pSataAdapter,
                                                              IN  MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = pScb->pCommandInfo;
#else
    MV_QUEUE_COMMAND_INFO   commandInfo;
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = &commandInfo;
#endif
    MV_QUEUE_COMMAND_RESULT result;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];

    memset(pCommandInfo, 0, sizeof(MV_QUEUE_COMMAND_INFO));
    pScb->commandType = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
    pCommandInfo->type = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
    pCommandInfo->PMPort = pScb->target;
    pCommandInfo->commandParams.NoneUdmaCommand.bufPtr = pDriveData->identifyBuffer;
    pCommandInfo->commandParams.NoneUdmaCommand.count = 256;         /* 512 bytes */
    pCommandInfo->commandParams.NoneUdmaCommand.callBack = SALCommandCompletionCB;
    pCommandInfo->commandParams.NoneUdmaCommand.command = MV_ATA_COMMAND_IDENTIFY;
    pCommandInfo->commandParams.NoneUdmaCommand.commandId = (MV_VOID_PTR) pScb;
    pCommandInfo->commandParams.NoneUdmaCommand.isEXT = MV_FALSE;
    pCommandInfo->commandParams.NoneUdmaCommand.protocolType = MV_NON_UDMA_PROTOCOL_PIO_DATA_IN;
    pCommandInfo->commandParams.NoneUdmaCommand.device = (MV_U8)(MV_BIT6);
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Sending Identify command: channel %d, Srb %p\n",
             pScb->bus, pScb);

    result = mvSataQueueCommand(pSataAdapter, pScb->bus, pCommandInfo);
    if (result != MV_QUEUE_COMMAND_RESULT_OK)
    {
        checkQueueCommandResult(pScb, result);
        /* shoudl complete the Scsi request here*/
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    return MV_SCSI_COMMAND_STATUS_QUEUED;
}
static MV_BOOLEAN  mvScsiAtaGetModeSenseDataPhase2(IN MV_SATA_ADAPTER    *pSataAdapter,
                                                   IN  MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    MV_U8   AdditionalSenseCode = SCSI_ADSENSE_NO_SENSE;
    MV_U8   *cmd = pScb->ScsiCdb;
    MV_U8   pageCode= cmd[2] & 0x3f;
    MV_U8   pageControl = (MV_U8)((cmd[2] & 0xc0) >> 6);
    MV_U8   modeSenseResult[MV_MAX_MODE_SENSE_RESULT_LENGTH];
    MV_U32  offset;
    MV_U32  pageLength;
    MV_BOOLEAN  commandFailed = MV_FALSE;

    memset(pScb->pDataBuffer, 0, pScb->dataBufferLength);
    memset(modeSenseResult, 0, MV_MAX_MODE_SENSE_RESULT_LENGTH);
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Sense: cmd[2] 0x%xcode 0x%x control 0x%x "
             "allocation length %d \n", pSataAdapter->adapterId, pScb->bus,
             cmd[2], pageCode, pageControl, (MV_U32)cmd[4]);


    if (pageControl == 0x3)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Sense: save control not supported\n.",
                 pSataAdapter->adapterId, pScb->bus);
        AdditionalSenseCode = 0x39; /*SAVING PARAMETERS NOT SUPPORTED */
        commandFailed = MV_TRUE;
    }
    if (commandFailed != MV_TRUE)
    {
        memset(modeSenseResult, 0, MV_MAX_MODE_SENSE_RESULT_LENGTH);
        /*1. Mode parameter header*/
        /* Mode data length will be set later */
        /* Medium Type 0: Default medium type */
        /* Device-specific parameter 0:  write enabled, target */
        /*      supports the DPO and FUA bits only in NCQ mode*/
        if (pSataAdapter->sataChannel[pScb->bus]->queuedDMA == MV_EDMA_MODE_NATIVE_QUEUING)
        {
            modeSenseResult[2] = MV_BIT4;
        }

        /* Block descriptor length 0: no block descriptors*/

        /*2. Block descriptor(s): Empty list*/
        /*3. Page(s)*/
        offset = 4;

        switch (pageCode)
        {
        case 0x3f:
        case 0x8: /*Caching page */
            pageLength = mvModeSenseCachingPage(pScb,
                                                modeSenseResult + offset,
                                                pageControl);

            offset += pageLength;

            if (pageCode == 0x8)
            {
                break;
            }
        case 0xa:
            pageLength = mvModeSenseControlPage(pSataAdapter,pScb,
                                                modeSenseResult + offset,
                                                pageControl);

            offset += pageLength;
            break;
        default:
            AdditionalSenseCode = SCSI_ADSENSE_INVALID_CDB;
            commandFailed = MV_TRUE;
        }

        /* set the DATA LENGTH of the Mode parameter list not including the number*/
        /* of bytes of the DATA LENGTH itself ( 1 byte for Mode Selet(6)) */
        modeSenseResult[0] = (MV_U8)(offset - 1);

        if (pScb->dataBufferLength < offset)
        {
            memcpy(pScb->pDataBuffer, modeSenseResult, pScb->dataBufferLength);
            pScb->dataTransfered = pScb->dataBufferLength;
        }
        else
        {
            memcpy(pScb->pDataBuffer, modeSenseResult, offset);
            pScb->dataTransfered = offset;
        }
    }

    if (commandFailed == MV_TRUE)
    {
        setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST, AdditionalSenseCode, 0);

        pScb->dataTransfered = 0;
        pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
    }
    else
    {
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
        pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
    }
    return MV_TRUE;
}
static MV_SCSI_COMMAND_STATUS_TYPE  mvScsiAtaModeSelect(IN MV_SATA_ADAPTER    *pSataAdapter,
                                                        IN  MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    MV_U8 result;
    MV_SCSI_COMMAND_STATUS_TYPE     commandStatus;
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " MODE SELECT RECEIVED: cmd:");
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %02x, %02x, %02x, %02x, %02x, %02x\n", pScb->ScsiCdb[0], pScb->ScsiCdb[1],
                 pScb->ScsiCdb[2], pScb->ScsiCdb[3], pScb->ScsiCdb[4], pScb->ScsiCdb[5]);
    }
    result = modeSelect(pSataAdapter, pScb, &commandStatus);
    if (result != 0x0)
    {
        if (result == 0x1)/*PARAMETER LIST LENGTH ERROR*/
        {
            setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST, 0x1a, 0);

        }
        else if (result == 0x2)
        {
            setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                         SCSI_ADSENSE_INVALID_CDB, 0);

        }
        else if (result == 0x3)
        {
            setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                         SCSI_ADSENSE_INVALID_FIELD_IN_PARAMETER_LIST, 0);

        }
        else
        {
            setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                         SCSI_ADSENSE_NO_SENSE, 0);

        }

        pScb->dataTransfered = 0;
        pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }
    return commandStatus;
}

static MV_U8
modeSelect(IN MV_SATA_ADAPTER    *pSataAdapter,
           IN  MV_SATA_SCSI_CMD_BLOCK  *pScb,
           MV_SCSI_COMMAND_STATUS_TYPE *pCommandStatus)
{
    MV_U8   *cmd = pScb->ScsiCdb;
    MV_VOID_PTR pBuffer = pScb->pDataBuffer;
    MV_U32  length = pScb->dataBufferLength;
    MV_U8   PF = (cmd[1] & MV_BIT4) >> 4;
    MV_U8   SP = (cmd[1] & MV_BIT0);
    MV_U8   *list = (MV_U8 *)pBuffer;
    MV_U32  offset;
    MV_U32  cachePageOffset = 0;

    {
        MV_U32 i;
        for (i =0 ; i < length; i++)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %02x", list[i]);
        }
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "\n");
    }
    if (PF == 0)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%d %d: Mode Select Error: PF not supported\n.",
                 pSataAdapter->adapterId, pScb->bus);
        return 0x2; /* Invalid field in CDB */
    }
    if (SP == 1)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%d %d: Mode Select Error: SP not supported\n.",
                 pSataAdapter->adapterId, pScb->bus);
        return 0x2; /* PARAMETER LIST LENGTH ERROR */
    }
    if (length == 0)
    {
        pScb->dataTransfered = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
        pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        *pCommandStatus = MV_SCSI_COMMAND_STATUS_COMPLETED;
        return 0;
    }
    if (length < 4)
    {
        return 0x1; /* PARAMETER LIST LENGTH ERROR */
    }
    if (list[0] || (list[1] != MV_SCSI_DIRECT_ACCESS_DEVICE) || list[2])
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: invalid field in parameter "
                 "list\n", pSataAdapter->adapterId, pScb->bus);
        return 0x3; /* Invalid field in parameter list */
    }
    if (list[3])
    {
        if (list[3] != 8)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: wrong size for mode parameter"
                     " block descriptor, BLOCK DESCRIPTOR LENGTH %d\n.",
                     pSataAdapter->adapterId, pScb->bus, list[3]);
            return 0x3; /* Invalid field in parameter list */
        }
        if (length < 12)
        {
            return 0x1; /* PARAMETER LIST LENGTH ERROR */
        }
        if (list[4] || list[5] || list[6] || list[7] || list[8] || list[9] ||
            (list[10] != 0x2) || list[11])
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: invalid field in parameter "
                     "block descriptor list\n", pSataAdapter->adapterId,
                     pScb->bus);
            return 0x3; /* Invalid field in parameter list */
        }
    }
    offset = 4 + list[3];/* skip the mode parameter block descriptor */

    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select: PF 0x%x SP 0x%x parameter length %x "
             "length %d(0x%x)\n offset %d", pSataAdapter->adapterId, pScb->bus,
             PF, SP, (MV_U32)cmd[4], length, length,
             offset);
    if (length == offset)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select : no mode pages available\n",
                 pSataAdapter->adapterId, pScb->bus);
        pScb->dataTransfered = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
        pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        *pCommandStatus = MV_SCSI_COMMAND_STATUS_COMPLETED;
        return 0;
    }

    while ((offset + 2) < length)
    {
        switch (list[offset] & 0x3f)
        {
        case 0x8:
            if (list[offset + 1] != 0x12)
            {
                mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: bad length in caching mode "
                         "page %d\n.",
                         pSataAdapter->adapterId, pScb->bus, list[offset + 1]);
                return 0x3; /* Invalid field in parameter list */
            }
            cachePageOffset = offset;
            offset += list[offset + 1] + 2;
            break;
        case 0xa:
            if ((list[offset] != 0xa) || (list[offset+1] != 0xa))
            {
                mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: invalid field in"
                         " mode control page, list[%x] %x, list[%x] %x\n",
                         pSataAdapter->adapterId, pScb->bus, offset,
                         list[offset], offset + 1, list[offset+1]);
                return 0x3;
            }

            if (list[offset + 3] != MV_BIT4)
            {
                mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: invalid field in"
                         " mode control page, list[%x] %x\n",
                         pSataAdapter->adapterId, pScb->bus, offset + 3,
                         list[offset + 3]);
                return 0x3;
            }

            if (list[offset + 2] || list[offset + 4] || list[offset + 5] ||
                list[offset + 6] || list[offset + 7]||list[offset + 8] ||
                list[offset + 9]|| list[offset + 10] || list[offset + 11])
            {
                mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: invalid field in"
                         " mode control page, line %d\n",
                         pSataAdapter->adapterId, pScb->bus, __LINE__);
                return 0x3;
            }
            offset += list[offset + 1] + 2;
            break;
        default:
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: invalid field in parameter "
                     "list, mode page %d not supported, offset %d\n",
                     pSataAdapter->adapterId, pScb->bus, list[offset],
                     offset);
            return 0x3; /* Invalid field in parameter list */
        }
    }

    if (length != offset)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: bad length %d\n.",
                 pSataAdapter->adapterId, pScb->bus, length);
        return 0x1; /* PARAMETER LIST LENGTH ERROR */
    }

    if (cachePageOffset)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Mode Select: caching Page found, offset %d\n", cachePageOffset);
        return mvParseModeCachingPage(pSataAdapter, pScb,list + cachePageOffset, pCommandStatus);
    }
    else
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Mode Select: No caching Page found\n");
        pScb->dataTransfered = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
        pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        *pCommandStatus = MV_SCSI_COMMAND_STATUS_COMPLETED;
        return 0;
    }
}

static MV_U8
mvParseModeCachingPage(MV_SATA_ADAPTER *pSataAdapter,
                       IN  MV_SATA_SCSI_CMD_BLOCK  *pScb,
                       MV_U8 *buffer,
                       MV_SCSI_COMMAND_STATUS_TYPE *pCommandStatus)
{
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = pScb->pCommandInfo;
#else
    MV_QUEUE_COMMAND_INFO   commandInfo;
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = &commandInfo;
#endif
    MV_QUEUE_COMMAND_RESULT result;
    MV_U8                   index = 0;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];


    if ((buffer[index++] & 0xc0) || (buffer[index++] != 0x12) ||
        ((buffer[index++] | MV_BIT2)!= MV_BIT2) || (buffer[index++]) ||
        (buffer[index++] != 0xff) || (buffer[index++] != 0xff) ||
        buffer[index++] || buffer[index++] || buffer[index++] ||
        (buffer[index++] != 0x10) || buffer[index++] ||
        (buffer[index++] != 0x10) || ((buffer[index++] | MV_BIT5) != MV_BIT5) ||
        (buffer[index++] != 0x1) || (buffer[index++] != 0xff) ||
        (buffer[index++] != 0xff) || buffer[index++] || buffer[index++]
        || buffer[index++] || buffer[index++])
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: invalid field in caching mode"
                 " page, index %d\n", pSataAdapter->adapterId, pScb->bus,
                 index);
        return 0x3; /* Invalid field in parameter list */
    }

    pScb->splitCount = 2;
    pScb->sequenceNumber = 1;
    if (buffer[12] & MV_BIT5) /* Disable Look Ahead*/
    {
        if (pDriveData->identifyInfo.readAheadSupported == MV_FALSE)
        {
            pScb->splitCount--;
        }
        pScb->LowLbaAddress = 0;
    }
    else
    {
        if (pDriveData->identifyInfo.readAheadSupported == MV_FALSE)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: invalid field in caching mode"
                     " page, enable read ahead (feature not supported)\n",
                     pSataAdapter->adapterId, pScb->bus);
            return 0x3; /* Invalid field in parameter list */
        }
        pScb->LowLbaAddress = 1;
    }

    if (buffer[2] & MV_BIT2) /* enable write cache*/
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Parse Caching Page: enable Write Cache\n");
        if (pDriveData->identifyInfo.writeCacheSupported == MV_FALSE)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d: Mode Select Error: invalid field in caching mode"
                     " page, enable write cache (feature not supported)\n",
                     pSataAdapter->adapterId, pScb->bus);
            return 0x3; /* Invalid field in parameter list */
        }
        pCommandInfo->commandParams.NoneUdmaCommand.features = MV_ATA_SET_FEATURES_ENABLE_WCACHE;
    }
    else
    {
        if (pDriveData->identifyInfo.writeCacheSupported == MV_FALSE)
        {
            pScb->splitCount--;
            if (pScb->splitCount == 1)
            {
                mvScsiAtaSendReadLookAhead(pSataAdapter, pScb);
                *pCommandStatus = MV_SCSI_COMMAND_STATUS_QUEUED;
                return 0;
            }
        }
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Parse Caching Page: disable Write Cache\n");
        pCommandInfo->commandParams.NoneUdmaCommand.features = MV_ATA_SET_FEATURES_DISABLE_WCACHE;
    }

    if (pScb->splitCount == 0)
    {
        pScb->dataTransfered = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
        pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        *pCommandStatus = MV_SCSI_COMMAND_STATUS_COMPLETED;
        return 0;
    }

    pScb->commandType = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;

    pCommandInfo->type = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
    pCommandInfo->PMPort = pScb->target;
    pCommandInfo->commandParams.NoneUdmaCommand.bufPtr = NULL;
    pCommandInfo->commandParams.NoneUdmaCommand.callBack = SALCommandCompletionCB;
    pCommandInfo->commandParams.NoneUdmaCommand.command = MV_ATA_COMMAND_SET_FEATURES;
    pCommandInfo->commandParams.NoneUdmaCommand.commandId = (MV_VOID_PTR) pScb;
    pCommandInfo->commandParams.NoneUdmaCommand.count = 0;

    pCommandInfo->commandParams.NoneUdmaCommand.isEXT = MV_FALSE;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaHigh = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaMid = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaLow = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.protocolType = MV_NON_UDMA_PROTOCOL_NON_DATA;
    pCommandInfo->commandParams.NoneUdmaCommand.sectorCount = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.device = (MV_U8)(MV_BIT6);

    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Sending SET FEATURES command: features %d\n",
             pCommandInfo->commandParams.NoneUdmaCommand.features);

    result = mvSataQueueCommand(pSataAdapter, pScb->bus, pCommandInfo);

    if (result != MV_QUEUE_COMMAND_RESULT_OK)
    {
        checkQueueCommandResult(pScb, result);
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        *pCommandStatus = MV_SCSI_COMMAND_STATUS_COMPLETED;

        return 0;
    }
    *pCommandStatus = MV_SCSI_COMMAND_STATUS_QUEUED;
    return 0;
}

static MV_U32
mvModeSenseCachingPage(MV_SATA_SCSI_CMD_BLOCK  *pScb,
                       MV_U8 *buffer,MV_U8 pageControl)
{
    MV_SATA_SCSI_DRIVE_DATA *pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];


    buffer[0] = 0x8; /* caching page*/
    buffer[1] = 0x12; /* length = 2 + 0x12*/
    buffer[2] = 0;
    if (pageControl == 2) /*default values*/
    {
        if (pDriveData->identifyInfo.writeCacheSupported == MV_TRUE)
        {
            buffer[2] = MV_BIT2;
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Cache Page: writeCacheEnabledByDefault\n");
        }
    }
    else if (pageControl == 0)  /* current values*/
    {
        if ((pDriveData->identifyInfo.writeCacheSupported == MV_TRUE) &&
            (pDriveData->identifyBuffer[85] & MV_BIT5))
        {
            buffer[2] = MV_BIT2;
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Cache Page: writeCacheEnabled\n");
        }
    }
    else if (pageControl == 1)  /* changeable values*/
    {
        if (pDriveData->identifyInfo.writeCacheSupported == MV_TRUE)
        {
            buffer[2] = MV_BIT2;
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Cache Page: writeCacheSupported\n");
        }
    }

    buffer[3] = 0;
    if (pageControl != 1)
    {
        buffer[4] = 0xff;
        buffer[5] = 0xff;
        buffer[9] = 0x10;
        buffer[11] = 0x10;
    }
    if (pageControl == 2) /*default values*/
    {
        if (pDriveData->identifyInfo.readAheadSupported == MV_FALSE)
        {
            buffer[12] = MV_BIT5;
        }
    }
    else if (pageControl == 0)  /* current values*/
    {
        if ((pDriveData->identifyInfo.readAheadSupported == MV_TRUE) &&
            (pDriveData->identifyBuffer[85] & MV_BIT6))
        {
            buffer[12] = 0;
        }
        else
        {
            buffer[12] = MV_BIT5;
        }
    }
    else if (pageControl == 1)  /* changeable values*/
    {
        if (pDriveData->identifyInfo.readAheadSupported == MV_TRUE)
        {
            buffer[12] = MV_BIT5;
        }
    }
    if (pageControl != 1)
    {
        buffer[13] = 0x01;
        buffer[14] = 0xff;
        buffer[15] = 0xff;
    }

    {
        MV_U32 i;

        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Cache Page: \n");
        for (i = 0; i < 0x14; i++)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "[%d] %x\n",i, buffer[i]);
        }
    }
    return 0x14;
}

static MV_U32
mvModeSenseControlPage(MV_SATA_ADAPTER *pSataAdapter,
                       MV_SATA_SCSI_CMD_BLOCK  *pScb,
                       MV_U8 *buffer, MV_U8 pageControl)
{
    buffer[0] = 0xA;    /* control page */
    buffer[1] = 0xA;    /* length 2 + 0xa*/
    if (pageControl != 1)
    {
        buffer[3] = MV_BIT4/*Unrestricted reordering allowed*/;
    }

    {
        MV_U32 i;

        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Control Page: \n");
        for (i = 0; i < 0xc; i++)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "[%d] %x\n",i , buffer[i]);
        }
    }
    return 0xc;
}

static MV_BOOLEAN
SALCommandCompletionCB(MV_SATA_ADAPTER *pSataAdapter,
                       MV_U8 channelNum,
                       MV_COMPLETION_TYPE comp_type,
                       MV_VOID_PTR commandId,
                       MV_U16 responseFlags,
                       MV_U32 timeStamp,
                       MV_STORAGE_DEVICE_REGISTERS *registerStruct)
{
    MV_SATA_SCSI_CMD_BLOCK  *pScb;
    if (commandId == NULL)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_FATAL_ERROR, " commandId is NULL, can't hanlde this !!!,adapterId=%d,"
                 " channel=%d \n", pSataAdapter->adapterId, channelNum);
        return MV_FALSE;
    }

    pScb = commandId;
    switch (comp_type)
    {
    case MV_COMPLETION_TYPE_NORMAL:
        /* finish */
#ifdef  MV_SUPPORT_ATAPI
        if(pScb->commandType == MV_QUEUED_COMMAND_TYPE_PACKET)
        {
            if ((registerStruct->statusRegister & MV_ATA_ERROR_STATUS) ||
               (registerStruct->statusRegister & MV_ATA_DEVICE_FAULT_STATUS))  
            {
                pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
                mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "packet command completed ",
                        "with check condition\n", pScb);
            }
            else
            {
                pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
            }
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
            pScb->dataTransfered = timeStamp;
            break;
        }
#endif
        /* If splited VERIFY command, then SRB completion will be on the last fragment */
        if ((((pScb->ScsiCdb[0] == SCSI_OPCODE_VERIFY6) ||
              (pScb->ScsiCdb[0] == SCSI_OPCODE_VERIFY10) ||
              (pScb->ScsiCdb[0] == SCSI_OPCODE_VERIFY16) ||
              (pScb->ScsiCdb[0] == SCSI_OPCODE_MODE_SELECT6)))
            &&  (pScb->splitCount > pScb->sequenceNumber))
        {
            /* add the command to the list for post interrupt service*/
            pScb->pNext = pScb->pSalAdapterExtension->pHead;
            pScb->pSalAdapterExtension->pHead = pScb;
            return MV_TRUE;
        }
        if (pScb->ScsiCdb[0] == SCSI_OPCODE_MODE_SENSE6)
        {
            mvScsiAtaGetModeSenseDataPhase2(pSataAdapter, pScb);
        }
        else
        {
            pScb->ScsiStatus = MV_SCSI_STATUS_GOOD;
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_SUCCESS;
        }
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "command completed. pScb %p\n", pScb);

        break;
    case MV_COMPLETION_TYPE_ABORT:
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "Error: command Aborted. Cdb: %02x %02x %02x %02x %02x "
                 "%02x %02x %02x %02x %02x\n", pScb->ScsiCdb[0],
                 pScb->ScsiCdb[1], pScb->ScsiCdb[2], pScb->ScsiCdb[3],
                 pScb->ScsiCdb[4], pScb->ScsiCdb[5], pScb->ScsiCdb[6],
                 pScb->ScsiCdb[7], pScb->ScsiCdb[8], pScb->ScsiCdb[9]);

        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_ABORTED;
        pScb->dataTransfered = 0;
        pScb->senseDataLength = 0;
        mvCommandCompletionErrorHandler(pScb->pIalAdapterExtension, channelNum);
        break;
    case MV_COMPLETION_TYPE_ERROR:
        pScb->dataTransfered = 0;
        pScb->senseDataLength = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_ATA_FAILED;

        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "COMPLETION ERROR , adapter =%d, channel=%d, flags=%x\n",
                 pSataAdapter->adapterId, channelNum, responseFlags);
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " Failed command Cdb: %02x %02x %02x %02x %02x "
                 "%02x %02x %02x %02x %02x\n", pScb->ScsiCdb[0],
                 pScb->ScsiCdb[1], pScb->ScsiCdb[2], pScb->ScsiCdb[3],
                 pScb->ScsiCdb[4], pScb->ScsiCdb[5], pScb->ScsiCdb[6],
                 pScb->ScsiCdb[7], pScb->ScsiCdb[8], pScb->ScsiCdb[9]);
        /* here the  eDMA will be stopped, so we have to flush  */
        /* the pending commands                                 */

        if (pScb->commandType == MV_QUEUED_COMMAND_TYPE_UDMA)
        {
            handleUdmaError(pScb, responseFlags, registerStruct);
#ifdef MV_LOGGER
            memcpy(&pScb->ATAregStruct, registerStruct,
                   sizeof(pScb->ATAregStruct));
#endif
        }
        else
        {
            handleNoneUdmaError(pScb, registerStruct);
#ifdef MV_LOGGER
            memcpy(&pScb->ATAregStruct, registerStruct,
                   sizeof(pScb->ATAregStruct));
#endif
        }
        mvCommandCompletionErrorHandler(pScb->pIalAdapterExtension, channelNum);
        break;
    default:
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_FATAL_ERROR, " Unknown completion type (%d)\n", comp_type);
        return MV_FALSE;
    }
#ifdef MV_LOGGER
    reportScbCompletion(pSataAdapter, pScb);
#endif

    pScb->completionCallBack(pSataAdapter, pScb);
    return MV_TRUE;
}
MV_VOID
handleNoneUdmaError(MV_SATA_SCSI_CMD_BLOCK  *pScb,
                    MV_STORAGE_DEVICE_REGISTERS *registerStruct)
{
    MV_U8 errorReg = registerStruct->errorRegister;
    MV_SCSI_SENSE_DATA SenseData;

    memset(&SenseData, 0, sizeof(MV_SCSI_SENSE_DATA));

    pScb->dataBufferLength = 0;

    /*if (pSrb->SenseInfoBufferLength < 13)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "IAL ERROR: invalid Sense Info buffer len (%d)\n",
                 Srb->SenseInfoBufferLength);
        Srb->SrbStatus = SRB_STATUS_ERROR;
        return;
    }*/
    memset(pScb->pSenseBuffer, 0, pScb->senseBufferLength);
    /*pScb->ScsiCommandCompletion = ;*/
    pScb->ScsiStatus =  MV_SCSI_STATUS_CHECK_CONDITION;

    SenseData.ResponseCode = MV_SCSI_RESPONSE_CODE;
//    SenseData.Valid = 0;

    SenseData.AdditionalSenseLength = 12;
    SenseData.InformationDesc.type = 0;
    SenseData.InformationDesc.AdditionalLength = 0xA;
    SenseData.InformationDesc.valid = 1 << 7;

    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " ATA Drive Registers:\n");
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","Error", registerStruct->errorRegister);
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","SectorCount", registerStruct->sectorCountRegister);
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","LBA Low", registerStruct->lbaLowRegister);
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","LBA Mid", registerStruct->lbaMidRegister);
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","LBA High", registerStruct->lbaHighRegister);
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","Device", registerStruct->deviceRegister);
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","Status", registerStruct->statusRegister);

    /* If the command is synchronize cache */
    if ((pScb->ScsiCdb[0] == SCSI_OPCODE_SYNCHRONIZE_CACHE10) || 
	(pScb->ScsiCdb[0] == SCSI_OPCODE_SYNCHRONIZE_CACHE16))
    {
        if (!(registerStruct->errorRegister & ABRT_ERR))
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " received error completion on flush cache command"
                     " but ABORT bit in error register is not set\n");
        }
        SenseData.SenseKey = SCSI_SENSE_MEDIUM_ERROR;
	_fillSenseInformation(pScb, &SenseData, registerStruct);
    }
    else if ((pScb->ScsiCdb[0] == SCSI_OPCODE_VERIFY10) ||
	     (pScb->ScsiCdb[0] == SCSI_OPCODE_VERIFY6) ||
	     (pScb->ScsiCdb[0] == SCSI_OPCODE_VERIFY16))
    {
        if (errorReg & (NM_ERR | MC_ERR | MCR_ERR))
        {
            SenseData.SenseKey = SCSI_SENSE_UNIT_ATTENTION;
        }
        else if (errorReg & UNC_ERR)
        {
#if 0
            MV_U32  LowLbaAddress = pScb->LowLbaAddress;
#endif
            SenseData.SenseKey = SCSI_SENSE_MEDIUM_ERROR;
#if 0
            /* Since high 8 bit address are taken immediatly from LowLbaAddress and
            not from the completion info ; the following code is relevant for both
            48bit and 28bit LBA addressing*/
            SenseData.Information[0] = (MV_U8)((LowLbaAddress & 0xff000000) >> 24);
            SenseData.Information[1] = (MV_U8)(registerStruct->lbaHighRegister & 0xff);
            SenseData.Information[2] = (MV_U8)(registerStruct->lbaMidRegister & 0xff);
            SenseData.Information[3] = (MV_U8)(registerStruct->lbaLowRegister & 0xff);
#endif
	    _fillSenseInformation(pScb, &SenseData, registerStruct);
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " Read Verify failed on UNC at sector %02x %02x %02x %02x %02x %02x\n",
                     SenseData.InformationDesc.information[2],
                     SenseData.InformationDesc.information[3],
                     SenseData.InformationDesc.information[4],
                     SenseData.InformationDesc.information[5],
                     SenseData.InformationDesc.information[6],
                     SenseData.InformationDesc.information[7]
		     );
        }
        /*else if (errorReg & IDNF_ERR)
        {
            SenseData.SenseKey = SCSI_SENSE_VOL_OVERFLOW;
        }*/
        else if ((errorReg & ABRT_ERR) || (errorReg & IDNF_ERR))
        {
            SenseData.SenseKey = SCSI_SENSE_ABORTED_COMMAND;
            SenseData.AdditionalSenseCode = SCSI_ADSENSE_NO_SENSE;
        }
        else
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " in mapping ATA error to SCSI error\n");
            SenseData.SenseKey = SCSI_SENSE_NO_SENSE;
        }
    }
    else if (pScb->ScsiCdb[0] == SCSI_OPCODE_MODE_SELECT6)
    {
        /* MODE SELECT is only when enabling / disabling write cache */
        if (errorReg & ABRT_ERR)
        {
            SenseData.SenseKey = SCSI_SENSE_ABORTED_COMMAND;
            SenseData.AdditionalSenseCode = SCSI_ADSENSE_NO_SENSE;
        }
        else
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " in mapping ATA error to SCSI error\n");
            SenseData.SenseKey = SCSI_SENSE_NO_SENSE;
        }
    }
    pScb->senseDataLength = 20;
    memcpy(pScb->pSenseBuffer, &SenseData,
           (pScb->senseBufferLength > pScb->senseDataLength) ?
           pScb->senseDataLength : pScb->senseBufferLength);
}



static MV_VOID
handleUdmaError(MV_SATA_SCSI_CMD_BLOCK  *pScb,
                MV_U32 responseFlags,
                MV_STORAGE_DEVICE_REGISTERS *registerStruct)
{

    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "UDMA %s command failed\n", (pScb->udmaType == MV_UDMA_TYPE_READ) ?
             "READ" : "WRITE");
    if (responseFlags & (MV_BIT3))
    {
        /* prevent the error_handler from re-send any commands */
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_DISCONNECT;
    }
    else if (responseFlags & MV_BIT2)           /* ATA error*/
    {
        MV_SCSI_SENSE_DATA SenseData;

        memset(&SenseData, 0, sizeof(MV_SCSI_SENSE_DATA));
        pScb->ScsiStatus =  MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->dataTransfered = 0;
        pScb->senseDataLength = 13;
//        SenseData.Valid = 1;
        SenseData.ResponseCode = MV_SCSI_RESPONSE_CODE;
        SenseData.AdditionalSenseLength = 12;
	SenseData.InformationDesc.type = 0;
	SenseData.InformationDesc.AdditionalLength = 0xA;
	SenseData.InformationDesc.valid = 1 << 7;


        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " ATA Drive Registers:\n");
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","Error", registerStruct->errorRegister);
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","SectorCount", registerStruct->sectorCountRegister);
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","LBA Low", registerStruct->lbaLowRegister);
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","LBA Mid", registerStruct->lbaMidRegister);
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","LBA High", registerStruct->lbaHighRegister);
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","Device", registerStruct->deviceRegister);
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "%20s : %04x\n","Status", registerStruct->statusRegister);

        if ((registerStruct->errorRegister & ICRC_ERR)||
            (registerStruct->errorRegister == 0xC))/*error code injected by 88i8030*/
        {
            SenseData.SenseKey = SCSI_SENSE_ABORTED_COMMAND;
            SenseData.AdditionalSenseCode = SCSI_ADSENSE_NO_SENSE;
        }
        else if (registerStruct->errorRegister &
                 (NM_ERR | MC_ERR | MCR_ERR))
        {
            SenseData.SenseKey = SCSI_SENSE_UNIT_ATTENTION;
            SenseData.AdditionalSenseCode = SCSI_ADSENSE_NO_MEDIA_IN_DEVICE;
        }
        else if ((registerStruct->errorRegister & UNC_ERR) ||
		 (registerStruct->errorRegister == 1))
        {
#if 0
            MV_U32  LowLbaAddress = pScb->LowLbaAddress;

            SenseData.Valid = 1;
            SenseData.Information[0] = (MV_U8)((LowLbaAddress & 0xff000000) >> 24);
            SenseData.Information[1] = (MV_U8)(registerStruct->lbaHighRegister & 0xff);
            SenseData.Information[2] = (MV_U8)(registerStruct->lbaMidRegister & 0xff);
            SenseData.Information[3] = (MV_U8)(registerStruct->lbaLowRegister & 0xff);
#endif
	    _fillSenseInformation(pScb, &SenseData, registerStruct);
            if (pScb->udmaType == MV_UDMA_TYPE_READ)
            {
                SenseData.SenseKey = SCSI_SENSE_MEDIUM_ERROR;
                SenseData.AdditionalSenseCode = SCSI_ADSENSE_NO_SENSE;

		mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " DMA Read failed on UNC at sector %02x %02x %02x %02x %02x %02x\n",
			 SenseData.InformationDesc.information[2],
			 SenseData.InformationDesc.information[3],
			 SenseData.InformationDesc.information[4],
			 SenseData.InformationDesc.information[5],
			 SenseData.InformationDesc.information[6],
			 SenseData.InformationDesc.information[7]
		     );
            }
            else
            {
                SenseData.SenseKey = SCSI_SENSE_DATA_PROTECT;
                SenseData.AdditionalSenseCode = SCSI_ADSENSE_NO_SENSE;
            }
        }
        else if ((registerStruct->errorRegister & IDNF_ERR) &&
                 (!(registerStruct->errorRegister & ABRT_ERR)))
        {
            /* In case IDNF is set and ABRT reset OR IDNF reset and ABRT is set */
            SenseData.SenseKey = SCSI_SENSE_ILLEGAL_REQUEST;
            SenseData.AdditionalSenseCode = SCSI_ADSENSE_ILLEGAL_BLOCK;
        }
        else if (registerStruct->errorRegister & ABRT_ERR)
        {
            SenseData.SenseKey = SCSI_SENSE_ABORTED_COMMAND;
            SenseData.AdditionalSenseCode = SCSI_ADSENSE_NO_SENSE;
        }
        else
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " in mapping ATA error to SCSI error\n");
            SenseData.SenseKey = SCSI_SENSE_ABORTED_COMMAND;
            SenseData.AdditionalSenseCode = SCSI_ADSENSE_NO_SENSE;
        }
        pScb->senseDataLength = 20;
        memcpy(pScb->pSenseBuffer, &SenseData,
               (pScb->senseBufferLength > pScb->senseDataLength) ?
               pScb->senseDataLength : pScb->senseBufferLength);
    }
    else if (responseFlags & (MV_BIT0 | MV_BIT1))
    {
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_PARITY_ERROR;
        pScb->ScsiStatus =  MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->senseDataLength = 0;
        pScb->dataTransfered = 0;
    }
    else if (responseFlags & (MV_BIT6|MV_BIT5))
    {
        if (responseFlags & MV_BIT6)
        {
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_UNDERRUN;
        }
        else
        {
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_OVERRUN;
        }
        pScb->dataTransfered = 0;

    }
}

/*******************************************************************************
* checkQueueCommandResult -
*
* DESCRIPTION:  set the scsi request completion status and the Scsi Status
*       according to the result returned form the mvSataQueueCommand function
*
* INPUT:
*
* OUTPUT:
*
* RETURN:
*
* COMMENTS:
*
*
*******************************************************************************/

/*static*/ MV_VOID  checkQueueCommandResult(MV_SATA_SCSI_CMD_BLOCK *pScb,
                                            MV_QUEUE_COMMAND_RESULT  result)
{
    switch (result)
    {
    case MV_QUEUE_COMMAND_RESULT_BAD_LBA_ADDRESS:
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " Edma Queue command failed. Bad LBA \n");
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCB;
        break;
    case MV_QUEUE_COMMAND_RESULT_QUEUED_MODE_DISABLED:
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " Edma Queue command failed. EDMA disabled\n");
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_NOT_READY;

        break;
    case MV_QUEUE_COMMAND_RESULT_FULL:
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " Edma Queue command failed. Queue is Full\n");
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_QUEUE_FULL;
        pScb->ScsiStatus = MV_SCSI_STATUS_QUEUE_FULL;
        break;
    case MV_QUEUE_COMMAND_RESULT_BAD_PARAMS:
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " Edma Queue command failed. (Bad Params)\n");
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCB;
        break;
    default:
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " Bad result value (%d) from queue"
                 " command\n", result);
    }
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " mvSataQueueUDmaCommand Failed\n");
    pScb->dataTransfered = 0;
    pScb->senseDataLength = 0;
}
#ifdef MV_LOGGER
static MV_VOID reportScbCompletion(MV_SATA_ADAPTER*    pSataAdapter,
                                   MV_SATA_SCSI_CMD_BLOCK *pScb)
{
    if (pScb->ScsiCommandCompletion != MV_SCSI_COMPLETION_SUCCESS)
    {
        MV_U8   buffer[100];
        MV_U32  index = 0;
        MV_BOOLEAN      printInfo = MV_TRUE;

        switch (pScb->ScsiCommandCompletion)
        {
        case MV_SCSI_COMPLETION_BAD_SCSI_COMMAND:
            SAL_SPRINTF(buffer, "%s", "MV_SCSI_COMPLETION_BAD_SCSI_COMMAND");
            break;
        case MV_SCSI_COMPLETION_ATA_FAILED:
            SAL_SPRINTF(buffer, "%s", "MV_SCSI_COMPLETION_ATA_FAILED");
            break;
        case MV_SCSI_COMPLETION_PARITY_ERROR:
            SAL_SPRINTF(buffer, "%s", "MV_SCSI_COMPLETION_PARITY");
            break;
        default:
            printInfo = MV_FALSE;
        }

        if (printInfo == MV_TRUE)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " %d %d %d :Scsi command completed. pScb %p, ScsiStatus %d "
                     "completionStatus %s\n", pSataAdapter->adapterId,
                     pScb->bus, pScb->target, pScb, pScb->ScsiStatus, buffer);
        }
        else
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Scsi command completed. pScb %p, ScsiStatus %d "
                     "completionStatus %d\n", pScb, pScb->ScsiStatus,
                     pScb->ScsiCommandCompletion);
        }

        index = SAL_SPRINTF(buffer, "%s", "CDB:");
        {
            MV_U32  i;
            for (i =0 ; i < pScb->ScsiCdbLength; i++)
            {
                index += SAL_SPRINTF(&buffer[index], "%x ",
                                     pScb->ScsiCdb[i]);
            }
            buffer[index] = '\n';
            buffer[index+1] = 0;
            mvLogMsg(MV_SAL_LOG_ID,(printInfo == MV_TRUE) ?
                     MV_DEBUG_ERROR : MV_DEBUG, buffer);
            if (pScb->ScsiStatus == MV_SCSI_STATUS_CHECK_CONDITION)
            {
                if ((pScb->pSenseBuffer != NULL) && (pScb->senseBufferLength > 0))
                {
                    MV_U32  len = pScb->senseDataLength > pScb->senseBufferLength ?
                                  pScb->senseBufferLength:pScb->senseDataLength;
                    index = SAL_SPRINTF(buffer, "%s", "Sense Data:");
                    for (i = 0; i < len; i++)
                    {
                        index += SAL_SPRINTF(buffer + index, "%x ",
                                             pScb->pSenseBuffer[i]);
                    }
                    buffer[index] = '\n';
                    buffer[index+1] = 0;
                    mvLogMsg(MV_SAL_LOG_ID, (printInfo == MV_TRUE) ?
                             MV_DEBUG_ERROR : MV_DEBUG, buffer);
                }
            }
        }
        if (pScb->ScsiCommandCompletion == MV_SCSI_COMPLETION_ATA_FAILED)
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " ATA Drive Registers:\n");
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "%20s : %04x\n","Error", pScb->ATAregStruct.errorRegister);
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "%20s : %04x\n","SectorCount", pScb->ATAregStruct.sectorCountRegister);
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "%20s : %04x\n","LBA Low", pScb->ATAregStruct.lbaLowRegister);
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "%20s : %04x\n","LBA Mid", pScb->ATAregStruct.lbaMidRegister);
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "%20s : %04x\n","LBA High", pScb->ATAregStruct.lbaHighRegister);
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "%20s : %04x\n","Device", pScb->ATAregStruct.deviceRegister);
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "%20s : %04x\n","Status", pScb->ATAregStruct.statusRegister);
        }
    }
    else
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Scsi command completed. pScb %p, ScsiStatus %d "
                 "completionStatus %d  dataTransfered %d \n", pScb, pScb->ScsiStatus,
                 pScb->ScsiCommandCompletion,  pScb->dataTransfered);
    }
    
}
#endif
static MV_VOID  mvScsiAtaSendSplittedVerifyCommand(IN MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = pScb->pCommandInfo;
#else
    MV_QUEUE_COMMAND_INFO   commandInfo;
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = &commandInfo;
#endif
    MV_QUEUE_COMMAND_RESULT result;
    MV_U8                  sectors = 0;/*256 sectors*/

    pScb->sequenceNumber++;
    if (pScb->sequenceNumber == 1)/*for the first command*/
    {
        if (pScb->ScsiCdb[0] == SCSI_OPCODE_VERIFY6)
        {
            sectors = pScb->ScsiCdb[4];
        }
        else if (pScb->ScsiCdb[0] == SCSI_OPCODE_VERIFY10)
        {
            sectors = pScb->ScsiCdb[8];
        }
	else 
        {
            sectors = pScb->ScsiCdb[13];
        }
    }
    pCommandInfo->type = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
    pCommandInfo->PMPort = pScb->target;
    pCommandInfo->commandParams.NoneUdmaCommand.bufPtr = NULL;
    pCommandInfo->commandParams.NoneUdmaCommand.callBack = SALCommandCompletionCB;
    pCommandInfo->commandParams.NoneUdmaCommand.command = MV_ATA_COMMAND_READ_VERIFY_SECTORS;
    pCommandInfo->commandParams.NoneUdmaCommand.commandId = (MV_VOID_PTR) pScb;
    pCommandInfo->commandParams.NoneUdmaCommand.count = 0;

    pCommandInfo->commandParams.NoneUdmaCommand.features = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.isEXT = MV_FALSE;
    pCommandInfo->commandParams.NoneUdmaCommand.protocolType = MV_NON_UDMA_PROTOCOL_NON_DATA;

    pCommandInfo->commandParams.NoneUdmaCommand.lbaHigh = (MV_U16)((pScb->LowLbaAddress & 0xff0000) >> 16);
    pCommandInfo->commandParams.NoneUdmaCommand.lbaMid = (MV_U16)((pScb->LowLbaAddress & 0xff00) >> 8);
    pCommandInfo->commandParams.NoneUdmaCommand.lbaLow = (MV_U16)(pScb->LowLbaAddress & 0xff);
    pCommandInfo->commandParams.NoneUdmaCommand.sectorCount = sectors;
    pCommandInfo->commandParams.NoneUdmaCommand.device = (MV_U8)(MV_BIT6 |
                                                                 ((pScb->LowLbaAddress & 0xf000000) >> 24));

    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d %d: Sending Splitted Verify command:seq# %d code %x lba"
             " %x(%x.%x.%x), sectors %d[%d] Srb %p\n",
             pScb->pSalAdapterExtension->pSataAdapter->adapterId, pScb->bus,
             pScb->target,
             pScb->sequenceNumber,pScb->ScsiCdb[0],
             pScb->LowLbaAddress,
             pCommandInfo->commandParams.NoneUdmaCommand.lbaHigh,
             pCommandInfo->commandParams.NoneUdmaCommand.lbaMid,
             pCommandInfo->commandParams.NoneUdmaCommand.lbaLow,
             pCommandInfo->commandParams.NoneUdmaCommand.sectorCount,
             mvSataNumOfDmaCommands(pScb->pSalAdapterExtension->pSataAdapter,
                                    pScb->bus), pScb);

    if (sectors)
    {
        pScb->LowLbaAddress += sectors;
    }
    else
    {
        pScb->LowLbaAddress += 0x100;
    }

    result = mvSataQueueCommand(pScb->pSalAdapterExtension->pSataAdapter,
                                pScb->bus, pCommandInfo);

    if (result != MV_QUEUE_COMMAND_RESULT_OK)
    {
        checkQueueCommandResult(pScb, result);
#ifdef MV_LOGGER
        reportScbCompletion(pScb->pSalAdapterExtension->pSataAdapter, pScb);
#endif

        return;
    }

    /* update stats*/
    pScb->pSalAdapterExtension->totalAccumulatedOutstanding[pScb->bus] +=
    mvSataNumOfDmaCommands(pScb->pSalAdapterExtension->pSataAdapter,pScb->bus);
    pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target].stats.totalIOs++;
    pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target].stats.totalSectorsTransferred += sectors;

}
static MV_VOID  mvScsiAtaSendReadLookAhead(IN MV_SATA_ADAPTER *pSataAdapter,
                                           IN MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = pScb->pCommandInfo;
#else
    MV_QUEUE_COMMAND_INFO   commandInfo;
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = &commandInfo;
#endif
    MV_QUEUE_COMMAND_RESULT result;

    if (pScb->LowLbaAddress == 0) /* Disable Look Ahead*/
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Parse Caching Page: Disable Read Look Ahead\n");
        pCommandInfo->commandParams.NoneUdmaCommand.features = MV_ATA_SET_FEATURES_DISABLE_RLA;
    }
    else
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Parse Caching Page: Enable Look Ahead\n");
        pCommandInfo->commandParams.NoneUdmaCommand.features = MV_ATA_SET_FEATURES_ENABLE_RLA;
    }

    pScb->commandType = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;

    pCommandInfo->type = MV_QUEUED_COMMAND_TYPE_NONE_UDMA;
    pCommandInfo->PMPort = pScb->target;
    pCommandInfo->commandParams.NoneUdmaCommand.bufPtr = NULL;
    pCommandInfo->commandParams.NoneUdmaCommand.callBack = SALCommandCompletionCB;
    pCommandInfo->commandParams.NoneUdmaCommand.command = MV_ATA_COMMAND_SET_FEATURES;
    pCommandInfo->commandParams.NoneUdmaCommand.commandId = (MV_VOID_PTR) pScb;
    pCommandInfo->commandParams.NoneUdmaCommand.count = 0;

    pCommandInfo->commandParams.NoneUdmaCommand.isEXT = MV_FALSE;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaHigh = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaMid = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.lbaLow = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.protocolType = MV_NON_UDMA_PROTOCOL_NON_DATA;
    pCommandInfo->commandParams.NoneUdmaCommand.sectorCount = 0;
    pCommandInfo->commandParams.NoneUdmaCommand.device = (MV_U8)(MV_BIT6);

    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Sending SET FEATURES command: features %d\n",
             pCommandInfo->commandParams.NoneUdmaCommand.features);

    pScb->sequenceNumber++;
    result = mvSataQueueCommand(pSataAdapter, pScb->bus, pCommandInfo);

    if (result != MV_QUEUE_COMMAND_RESULT_OK)
    {
        checkQueueCommandResult(pScb, result);
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return;
    }
}

MV_VOID     mvSataScsiInitAdapterExt(MV_SAL_ADAPTER_EXTENSION *pAdapterExt,
                                     MV_SATA_ADAPTER* pSataAdapter)
{
    MV_U8   channelIndex;
    MV_U8   PMPort;
    pAdapterExt->pSataAdapter = pSataAdapter;
    pAdapterExt->pHead = NULL;
    pAdapterExt->UAMask = 0xFF;
    for (channelIndex = 0; channelIndex < MV_SATA_CHANNELS_NUM; channelIndex++)
    {
        for (PMPort = 0; PMPort < MV_SATA_PM_MAX_PORTS; PMPort++)
        {
            pAdapterExt->ataDriveData[channelIndex][PMPort].driveReady = MV_FALSE;
            /* one identify data buffer used for all the drives connected to */
            /* the same channel*/
            pAdapterExt->ataDriveData[channelIndex][PMPort].identifyBuffer =
            pAdapterExt->identifyBuffer[channelIndex];
        }
    }
}
#ifdef MV_SUPPORT_ATAPI
MV_SCSI_COMMAND_STATUS_TYPE mvScsiAtaSendATAPICommand(MV_SATA_ADAPTER *pSataAdapter, 
                                                        MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = pScb->pCommandInfo;
#else
    MV_QUEUE_COMMAND_INFO   commandInfo;
    MV_QUEUE_COMMAND_INFO   *pCommandInfo = &commandInfo;
#endif
    MV_QUEUE_COMMAND_RESULT result;

 
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Send Packet command, adapter %d bus %d target %d lun %"
             "d pScb %p\n  dir(%d)  Data Buffer %p (%d), cdb %p (%d)\n",
             pSataAdapter->adapterId, pScb->bus, pScb->target,
             pScb->lun, pScb, pScb->dataDirection,
            pScb->pDataBuffer, 
                pScb->dataBufferLength, pScb->ScsiCdb, pScb->ScsiCdbLength);
 
    pScb->commandType = MV_QUEUED_COMMAND_TYPE_PACKET;

    pCommandInfo->type = MV_QUEUED_COMMAND_TYPE_PACKET;
    pCommandInfo->PMPort = pScb->target;
    pCommandInfo->commandParams.packetCommand.bufPtr = (MV_U16_PTR)pScb->pDataBuffer;
    pCommandInfo->commandParams.packetCommand.buffer_len = pScb->dataBufferLength;
    pCommandInfo->commandParams.packetCommand.transfered_data = 0;
    pCommandInfo->commandParams.packetCommand.cdb_len = (pScb->ScsiCdbLength  >> 1);
    pCommandInfo->commandParams.packetCommand.cdb_buffer = (MV_U16_PTR)pScb->ScsiCdb;
    pCommandInfo->commandParams.packetCommand.flags = 0;
    pCommandInfo->commandParams.packetCommand.callBack = SALCommandCompletionCB;
    pCommandInfo->commandParams.packetCommand.commandId = (MV_VOID_PTR) pScb;
    pCommandInfo->commandParams.packetCommand.prdLowAddr = pScb->PRDTableLowPhyAddress;
    pCommandInfo->commandParams.packetCommand.prdHighAddr = pScb->PRDTableHighPhyAddress;
    
    if((pScb->dataDirection == MV_SCSI_COMMAND_DATA_DIRECTION_IN) && (pScb->pDataBuffer == NULL))
    {
        pCommandInfo->commandParams.packetCommand.protocolType = MV_NON_UDMA_PROTOCOL_PACKET_DMA;
    }
    else if((pScb->dataDirection == MV_SCSI_COMMAND_DATA_DIRECTION_OUT) && (pScb->pDataBuffer == NULL))
    {
        pCommandInfo->commandParams.packetCommand.protocolType = MV_NON_UDMA_PROTOCOL_PACKET_DMA;
        pCommandInfo->commandParams.packetCommand.flags = MV_BIT0;
    }
    else
    {
        switch(pScb->dataDirection)
        {
            case MV_SCSI_COMMAND_DATA_DIRECTION_IN:
                pCommandInfo->commandParams.packetCommand.protocolType = MV_NON_UDMA_PROTOCOL_PACKET_PIO_DATA_IN;
                break;
            case MV_SCSI_COMMAND_DATA_DIRECTION_OUT:
                pCommandInfo->commandParams.packetCommand.protocolType = MV_NON_UDMA_PROTOCOL_PACKET_PIO_DATA_OUT;
                break;
        default:
                pCommandInfo->commandParams.packetCommand.protocolType = MV_NON_UDMA_PROTOCOL_PACKET_PIO_NON_DATA;
        }
    }
    pScb->sequenceNumber = 0;
    result = mvSataQueueCommand(pSataAdapter, pScb->bus, pCommandInfo);

    if (result != MV_QUEUE_COMMAND_RESULT_OK)
    {
        checkQueueCommandResult(pScb, result);
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }
    return MV_SCSI_COMMAND_STATUS_QUEUED;
}
#endif
MV_SCSI_COMMAND_STATUS_TYPE mvSataExecuteScsiCommand(MV_SATA_SCSI_CMD_BLOCK  *pScb)
{
    MV_U8               *cmd = pScb->ScsiCdb;
    MV_BOOLEAN          invalidCDB = MV_FALSE;
    MV_SATA_ADAPTER *pSataAdapter = pScb->pSalAdapterExtension->pSataAdapter;
    MV_SATA_SCSI_DRIVE_DATA *pDriveData;

    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Scsi Command Received, adapter %d bus %d target %d lun %"
             "d pScb %p\n    Data Buffer length %d, Sense buffer length %x\n",
             pSataAdapter->adapterId, pScb->bus, pScb->target,
             pScb->lun, pScb, pScb->dataBufferLength, pScb->senseBufferLength);

#ifdef MV_LOGGER

    {
        MV_U8 buffer[50];
        MV_U32  i, index;

        index = SAL_SPRINTF(buffer, "%s", "CDB:");
        for (i =0 ; i < pScb->ScsiCdbLength; i++)
        {
            index += SAL_SPRINTF(&buffer[index], "%x ",
                                 pScb->ScsiCdb[i]);
        }
        buffer[index] = '\n';
        buffer[index+1] = 0;
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, buffer);
    }
#endif
    pScb->dataTransfered = 0;
    pScb->senseDataLength = 0;
    pScb->ScsiStatus = 0;
    if (pScb->bus >= pSataAdapter->numberOfChannels)
    {
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_INVALID_BUS;
        pScb->dataTransfered = 0;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    if ((pScb->target >= MV_SATA_PM_MAX_PORTS) ||
        (pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target].driveReady == MV_FALSE) ||
        ((pScb->lun) && (pScb->ScsiCdb[0] != SCSI_OPCODE_INQUIRY)))
    {
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_NO_DEVICE;
        pScb->dataTransfered = 0;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }
    pDriveData = &pScb->pSalAdapterExtension->ataDriveData[pScb->bus][pScb->target];
#ifdef MV_SUPPORT_ATAPI
    if(pDriveData->identifyInfo.deviceType == MV_SATA_DEVICE_TYPE_ATAPI_DEVICE)
    {
        return mvScsiAtaSendATAPICommand(pSataAdapter, pScb);
    }
#endif
    switch (cmd[0])
    {
    case SCSI_OPCODE_READ10:
    case SCSI_OPCODE_WRITE10:
    case SCSI_OPCODE_READ_CAPACITY10:
    case SCSI_OPCODE_VERIFY10:
    case SCSI_OPCODE_SYNCHRONIZE_CACHE10:

#ifdef MV_SATA_SUPPORT_READ_WRITE_LONG
    case SCSI_OPCODE_WRITE_LONG10:
    case SCSI_OPCODE_READ_LONG10:
#endif
        if (cmd[1] & MV_BIT0) /* if related address*/
        {
            invalidCDB = MV_TRUE;
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d %d: Scsi command received with "
                     "RELADR bit enabled - returning ILLEGAL REQUEST\n"
                     ,pSataAdapter->adapterId, pScb->bus, pScb->target);

        }
    }
    if (cmd[pScb->ScsiCdbLength - 1] != 0) /*if CONTROL is set*/
    {
        MV_BOOLEAN commandSupported = MV_TRUE;

        switch (cmd[0])
        {
        case SCSI_OPCODE_READ6:
        case SCSI_OPCODE_READ10:
        case SCSI_OPCODE_READ16:
        case SCSI_OPCODE_WRITE6:
        case SCSI_OPCODE_WRITE10:
        case SCSI_OPCODE_WRITE16:
        case SCSI_OPCODE_INQUIRY:
        case SCSI_OPCODE_TEST_UNIT_READY:
        case SCSI_OPCODE_MODE_SELECT6:
        case SCSI_OPCODE_MODE_SENSE6:
        case SCSI_OPCODE_READ_CAPACITY10:     /* read capctiy CDB*/
        case SCSI_OPCODE_REQUEST_SENSE6:
        case SCSI_OPCODE_VERIFY6:
        case SCSI_OPCODE_VERIFY10:
        case SCSI_OPCODE_VERIFY16:
        case SCSI_OPCODE_SYNCHRONIZE_CACHE10:
        case SCSI_OPCODE_SYNCHRONIZE_CACHE16:
        case SCSI_OPCODE_SEEK10:
        case SCSI_OPCODE_REASSIGN_BLOCKS:
	case SCSI_OPCODE_REPORT_LUNS:
#ifdef MV_SATA_SUPPORT_READ_WRITE_LONG
        case SCSI_OPCODE_WRITE_LONG10:
        case SCSI_OPCODE_READ_LONG10:
#endif

            break;
        default:
            commandSupported = MV_FALSE;
        }
        if (commandSupported == MV_TRUE)
        {
            invalidCDB = MV_TRUE;
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "[%d,%d] Scsi command received with "
                     "none zero CONTROL bits - returning ILLEGAL REQUEST\n"
                     ,pSataAdapter->adapterId, pScb->bus);
        }
    }

    if (pDriveData->UAConditionPending == MV_TRUE)
    {
        if (((cmd[0] != SCSI_OPCODE_INQUIRY) &&
	     (cmd[0] != SCSI_OPCODE_REPORT_LUNS) &&
             (cmd[0] != SCSI_OPCODE_REQUEST_SENSE6)) || (invalidCDB == MV_TRUE))
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " Unit Attention condition is pending.\n");

            if (pDriveData->UAEvents & MV_BIT0)
            {
                mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Report Bus Reset.\n");
                pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_UA_RESET;
                setSenseData(pScb, SCSI_SENSE_UNIT_ATTENTION, SCSI_ADSENSE_BUS_RESET
                             , 2);
                pDriveData->UAEvents &= ~MV_BIT0;
            }
            else if (pDriveData->UAEvents & MV_BIT1)
            {
                mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Report Mode Parameters Changed.\n");
                pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_UA_PARAMS_CHANGED;
                setSenseData(pScb, SCSI_SENSE_UNIT_ATTENTION,
                             SCSI_ADSENSE_PARAMETERS_CHANGED, 1);
                pDriveData->UAEvents &= ~MV_BIT1;
            }

            pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
            pScb->dataTransfered = 0;
#ifdef MV_LOGGER
            reportScbCompletion(pSataAdapter, pScb);
#endif
            pScb->completionCallBack(pSataAdapter, pScb);
            if (pDriveData->UAEvents == 0)
            {
                pDriveData->UAConditionPending = MV_FALSE;
            }
            return MV_SCSI_COMMAND_STATUS_COMPLETED;
        }
    }
    if (invalidCDB == MV_TRUE)
    {
        setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST, SCSI_ADSENSE_INVALID_CDB,
                     0);
        pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
        pScb->dataTransfered = 0;
        pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
#ifdef MV_LOGGER
        reportScbCompletion(pSataAdapter, pScb);
#endif
        pScb->completionCallBack(pSataAdapter, pScb);
        return MV_SCSI_COMMAND_STATUS_COMPLETED;
    }

    switch (cmd[0])
    {
    case SCSI_OPCODE_READ6:
    case SCSI_OPCODE_READ10:
    case SCSI_OPCODE_READ16:
    case SCSI_OPCODE_WRITE6:
    case SCSI_OPCODE_WRITE10:
    case SCSI_OPCODE_WRITE16:
        return mvScsiAtaSendDataCommand(pSataAdapter, pScb);
    case SCSI_OPCODE_INQUIRY:
        return mvScsiAtaGetInquiryData(pSataAdapter, pScb);
    case SCSI_OPCODE_TEST_UNIT_READY:
        return mvScsiAtaTestUnitReady(pSataAdapter, pScb);
    case SCSI_OPCODE_MODE_SELECT6:
        return mvScsiAtaModeSelect(pSataAdapter, pScb);
    case SCSI_OPCODE_MODE_SENSE6:
        return mvScsiAtaGetModeSenseData(pSataAdapter,pScb);
        /* Used to detect write protected status.*/
    case SCSI_OPCODE_READ_CAPACITY10:     /* read capctiy CDB*/
        return mvScsiAtaGetReadCapacityData(pSataAdapter, pScb);
    case SCSI_OPCODE_READ_CAPACITY16:     /* read capctiy CDB*/
        return mvScsiAtaGetReadCapacity16Data(pSataAdapter, pScb);
    case SCSI_OPCODE_REQUEST_SENSE6:
        return mvScsiAtaGetRequestSenseData(pSataAdapter, pScb);
    case SCSI_OPCODE_REPORT_LUNS:
        return mvScsiAtaReportLuns(pSataAdapter, pScb);
    case SCSI_OPCODE_VERIFY6:
    case SCSI_OPCODE_VERIFY10:
    case SCSI_OPCODE_VERIFY16:
        return mvScsiAtaSendVerifyCommand(pSataAdapter, pScb);
    case SCSI_OPCODE_SYNCHRONIZE_CACHE10:
    case SCSI_OPCODE_SYNCHRONIZE_CACHE16:
        return mvScsiAtaSendSyncCacheCommand(pSataAdapter, pScb);
    case SCSI_OPCODE_SEEK10:
        return mvScsiAtaSeek(pSataAdapter, pScb);
    case SCSI_OPCODE_REASSIGN_BLOCKS:
        return mvScsiAtaReassignBlocks(pSataAdapter, pScb);
#ifdef MV_SATA_SUPPORT_READ_WRITE_LONG
    case SCSI_OPCODE_WRITE_LONG10:
        return mvScsiAtaWriteLong(pSataAdapter, pScb);
    case SCSI_OPCODE_READ_LONG10:
        return mvScsiAtaReadLong(pSataAdapter, pScb);
#endif

    default:
        {
            setSenseData(pScb, SCSI_SENSE_ILLEGAL_REQUEST,
                         SCSI_ADSENSE_ILLEGAL_COMMAND, 0);
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, "mvExecuteScsiCommand: ERROR: Unsupported command %02X\n", pScb->ScsiCdb[0]);
            pScb->ScsiStatus = MV_SCSI_STATUS_CHECK_CONDITION;
            pScb->dataTransfered = 0;
            pScb->ScsiCommandCompletion = MV_SCSI_COMPLETION_BAD_SCSI_COMMAND;
#ifdef MV_LOGGER
            reportScbCompletion(pSataAdapter, pScb);
#endif
            pScb->completionCallBack(pSataAdapter, pScb);
            return MV_SCSI_COMMAND_STATUS_COMPLETED;

        }
    }

    return MV_SCSI_COMMAND_STATUS_FAILED;
}

MV_VOID mvSataScsiPostIntService(MV_SAL_ADAPTER_EXTENSION *pAdapterExt)
{
    MV_SATA_SCSI_CMD_BLOCK  *pScb = pAdapterExt->pHead;
    while (pScb)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, "Post Interrupt Service: pScb %p command %x\n", pScb,
                 pScb->ScsiCdb[0]);
        switch (pScb->ScsiCdb[0])
        {
	case    SCSI_OPCODE_VERIFY16:
        case    SCSI_OPCODE_VERIFY10:
        case    SCSI_OPCODE_VERIFY6:
            mvScsiAtaSendSplittedVerifyCommand(pScb);
            break;
        case    SCSI_OPCODE_MODE_SELECT6:
            mvScsiAtaSendReadLookAhead(pAdapterExt->pSataAdapter,
                                       pScb);
            break;
        default:
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG_ERROR, " Post Interrupt Service called for bad scsi"
                     " command(%x)\n", pScb->ScsiCdb[0]);
        }
        pScb = pScb->pNext;
    }
    pAdapterExt->pHead = NULL;
    return;
}

MV_VOID     mvSataScsiSetDriveReady(MV_SAL_ADAPTER_EXTENSION *pAdapterExt,
                                    MV_U8   channelIndex, MV_U8 PMPort,
                                    MV_BOOLEAN  isReady)
{
    if (isReady == MV_TRUE)
    {
        mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d %d: ATA Drive is Ready.\n",
                 pAdapterExt->pSataAdapter->adapterId, channelIndex, PMPort);
        pAdapterExt->ataDriveData[channelIndex][PMPort].driveReady = MV_TRUE;
        pAdapterExt->totalAccumulatedOutstanding[channelIndex] = 0;
        pAdapterExt->ataDriveData[channelIndex][PMPort].stats.totalIOs = 0;
        pAdapterExt->ataDriveData[channelIndex][PMPort].stats.totalSectorsTransferred = 0;
    }
    else
    {
        if (PMPort == 0xFF)
        {
            MV_U8   i;

            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d  : SATA Channel is Removed.\n",
                     pAdapterExt->pSataAdapter->adapterId, channelIndex);
            for (i = 0; i < MV_SATA_PM_MAX_PORTS; i++)
            {
                mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d %d: SATA Drive is Removed.\n",
                         pAdapterExt->pSataAdapter->adapterId, channelIndex,
                         i);
                pAdapterExt->ataDriveData[channelIndex][i].driveReady = MV_FALSE;
            }

        }
        else
        {
            mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d %d: SATA Drive is Removed.\n",
                     pAdapterExt->pSataAdapter->adapterId, channelIndex,
                     PMPort);
            pAdapterExt->ataDriveData[channelIndex][PMPort].driveReady = MV_FALSE;

        }
    }
}


/* notify the translation layer with Reset and Power on reset*/
MV_VOID mvSataScsiNotifyUA(MV_SAL_ADAPTER_EXTENSION *pAdapterExt,
                           MV_U8    channelIndex, MV_U8 PMPort)
{
    pAdapterExt->ataDriveData[channelIndex][PMPort].UAConditionPending = MV_TRUE;
    /* bit 0 - reset*/
    /* bit 1 - parameters changed*/
    pAdapterExt->ataDriveData[channelIndex][PMPort].UAEvents = MV_BIT1 | MV_BIT0;
    pAdapterExt->ataDriveData[channelIndex][PMPort].UAEvents &=
    pAdapterExt->UAMask;
    mvLogMsg(MV_SAL_LOG_ID, MV_DEBUG, " %d %d %d: Notify SAL with Unit Attention condition.\n",
             pAdapterExt->pSataAdapter->adapterId, channelIndex, PMPort);
}

