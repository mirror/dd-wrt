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
* mvIALCommon.h
*
* DESCRIPTION:
*       H implementation for IAL's common functions.
*
* DEPENDENCIES:
*   mvSata.h
*   mvStorageDev.h
*
*******************************************************************************/
#ifndef __INCmvIALCommonh
#define __INCmvIALCommonh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* includes */
#include "mvSata.h"
#include "mvStorageDev.h"

/* defines  */

/*Timer period in milliseconds*/
#define MV_IAL_ASYNC_TIMER_PERIOD       500
#define MV_IAL_SRST_TIMEOUT             31000

/* typedefs */



typedef enum mvAdapterState
{
    ADAPTER_INITIALIZING,
    ADAPTER_READY,
    ADAPTER_FATAL_ERROR
} MV_ADAPTER_STATE;

typedef enum mvChannelState
{
    CHANNEL_NOT_CONNECTED,
    CHANNEL_CONNECTED,
    CHANNEL_IN_SRST,
    CHANNEL_PM_STAGGERED_SPIN_UP,
    CHANNEL_PM_SRST_DEVICE,
    CHANNEL_READY,
    CHANNEL_PM_HOT_PLUG,
} MV_CHANNEL_STATE;

typedef struct mvDriveSerialNumber
{
    MV_U8 serial[IDEN_SERIAL_NUM_SIZE];    
}   MV_DRIVE_SERIAL_NUMBER;


typedef struct mvDrivesInfo
{
    MV_U16                      drivesSnapshotSaved;
    MV_DRIVE_SERIAL_NUMBER      driveSerialSaved[MV_SATA_PM_MAX_PORTS];    
    MV_U16                      drivesSnapshotCurrent;
    MV_DRIVE_SERIAL_NUMBER      driveSerialCurrent[MV_SATA_PM_MAX_PORTS];    
}   MV_DRIVES_INFO;


typedef struct mvIALChannelExtension
{
    MV_U8                       PMnumberOfPorts;
    MV_U16                      PMdevsToInit;
    MV_U8                       devInSRST;
    MV_BOOLEAN                  completionError;
    MV_U8                       pmAccessType;
    MV_U8                       pmReg;
    MV_BOOLEAN                  pmRegAccessInProgress;
    MV_BOOLEAN                  pmAsyncNotifyEnabled;
    MV_U8                       pmRegPollCounter;
    MV_U32                      SRSTTimerThreshold;
    MV_U32                      SRSTTimerValue;
    MV_VOID_PTR                 IALChannelPendingCmdQueue;
    MV_BOOLEAN                  bHotPlug;
    MV_DRIVES_INFO              drivesInfo;
#ifdef MV_SATA_STORE_COMMANDS_INFO_ON_IAL_STACK
    MV_QUEUE_COMMAND_INFO       commandInfo;
#endif
} MV_IAL_COMMON_CHANNEL_EXTENSION;


typedef struct mvIALCommonAdapterExtension
{
    MV_SATA_ADAPTER   *pSataAdapter;
    MV_ADAPTER_STATE  adapterState;
    MV_CHANNEL_STATE  channelState[MV_SATA_CHANNELS_NUM];
    MV_IAL_COMMON_CHANNEL_EXTENSION IALChannelExt[MV_SATA_CHANNELS_NUM];
} MV_IAL_COMMON_ADAPTER_EXTENSION;


/*Public functions*/
MV_BOOLEAN mvAdapterStartInitialization(MV_SATA_ADAPTER* pSataAdapter,
                                        MV_IAL_COMMON_ADAPTER_EXTENSION *ialExt,
                                        MV_SAL_ADAPTER_EXTENSION *scsiAdapterExt);

void mvRestartChannel(MV_IAL_COMMON_ADAPTER_EXTENSION *ialExt,
                      MV_U8 channelIndex,
                      MV_SAL_ADAPTER_EXTENSION *scsiAdapterExt,
                      MV_BOOLEAN    bBusReset);

void mvStopChannel(MV_IAL_COMMON_ADAPTER_EXTENSION *ialExt,
                   MV_U8 channelIndex,
                   MV_SAL_ADAPTER_EXTENSION *scsiAdapterExt);

void mvPMHotPlugDetected(MV_IAL_COMMON_ADAPTER_EXTENSION *ialExt,
                         MV_U8 channelIndex,
                         MV_SAL_ADAPTER_EXTENSION *scsiAdapterExt);


MV_SCSI_COMMAND_STATUS_TYPE mvExecuteScsiCommand(MV_SATA_SCSI_CMD_BLOCK  *pScb,
                                                 MV_BOOLEAN canQueue);

MV_BOOLEAN  mvIALTimerCallback(MV_IAL_COMMON_ADAPTER_EXTENSION *ialExt,
                               MV_SAL_ADAPTER_EXTENSION *scsiAdapterExt);

void mvCommandCompletionErrorHandler(MV_IAL_COMMON_ADAPTER_EXTENSION *ialExt,
                                     MV_U8 channelIndex);

MV_BOOLEAN mvRemoveFromSCSICommandQueue(MV_IAL_COMMON_ADAPTER_EXTENSION *ialExt,
                                        MV_U8 channelIndex,
                                        MV_SATA_SCSI_CMD_BLOCK *pScb);

/*The following functions which must be implemented in IAL*/

MV_BOOLEAN IALConfigQueuingMode(MV_SATA_ADAPTER *pSataAdapter,
                                MV_U8 channelIndex,
                                MV_EDMA_MODE mode,
                                MV_SATA_SWITCHING_MODE switchingMode,
                                MV_BOOLEAN  use128Entries);


MV_BOOLEAN IALInitChannel(MV_SATA_ADAPTER *pSataAdapter, MV_U8 channelIndex);

void IALReleaseChannel(MV_SATA_ADAPTER *pSataAdapter, MV_U8 channelIndex);
MV_BOOLEAN IALBusChangeNotify(MV_SATA_ADAPTER *pSataAdapter,
                              MV_U8 channelIndex);
MV_BOOLEAN IALBusChangeNotifyEx(MV_SATA_ADAPTER *pSataAdapter, 
                                MV_U8 channelIndex, 
                                MV_U16 targetsToRemove,
                                MV_U16 targetsToAdd);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCmvIALCommonh */

