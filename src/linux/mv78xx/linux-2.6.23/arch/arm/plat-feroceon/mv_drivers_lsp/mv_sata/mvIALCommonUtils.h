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
* mvIALCommonUtils.h
*
* DESCRIPTION:
*       H implementation for IAL's extension utility functions.
*
* DEPENDENCIES:
*   mvSata.h
*   mvStorageDev.h
*
*******************************************************************************/
#ifndef __INCmvIALCommonUtilsh
#define __INCmvIALCommonUtilsh

#ifdef __cplusplus
extern "C"  {
#endif /* __cplusplus */

/* includes */
#include "mvSata.h"
#include "mvStorageDev.h"

/* defines */
#define MV_IAL_COMMON_LOG_ID            2

/* typedefs */
typedef struct serialATACapabilites
{
    MV_BOOLEAN  SATA_GEN_I_supported:1;
    MV_BOOLEAN  SATA_GEN_II_supported:1;
    MV_BOOLEAN  NCQSupported:1;/*native command queuing*/
    MV_BOOLEAN  RxHostInitiatedPMSupported:1;/* Supports receipt of host-initiated
                                              interface power management
                                              requests*/
    MV_BOOLEAN  TxDeviceInitiatedPMSupported:1;/* device supports initiating
                                                interface power management*/
    MV_BOOLEAN  TxDeviceInitiatedPMEnabled:1;
    MV_BOOLEAN  DMASetupAutoActiveSupported:1;/* supports DMA Setup Auto-Activate
                                               optimization*/
    MV_BOOLEAN  DMASetupAutoActiveEnables:1;
    MV_BOOLEAN  NonZeroBufferOffsetSupported:1;/* supports non-zero buffer offsets
                                                in DMA Setup FIS*/
    MV_BOOLEAN  NonZeroBufferOffsetEnabled:1;
}SERIAL_ATA_CAPABILITIES;

typedef struct ATAIdentifyInfo
{
    MV_U8           version;
    MV_U8           model[24];
    MV_U8           firmware[4];
    MV_U8           UdmaMode;
    MV_U8           PIOMode;
    MV_BOOLEAN      LBA48Supported:1;/* used for READ/WRITE commands*/
    MV_BOOLEAN      writeCacheSupported:1;
    MV_BOOLEAN      writeCacheEnabled:1;
    MV_BOOLEAN      readAheadSupported:1;
    MV_BOOLEAN      readAheadEnabled:1;
    MV_BOOLEAN      DMAQueuedModeSupported:1;
    MV_U8           DMAQueuedModeDepth;
    MV_U64          ATADiskSize;
    SERIAL_ATA_CAPABILITIES SATACapabilities;/*valid only for ATA-7 or higher*/
    MV_U8           commandPacketLength;
    MV_SATA_DEVICE_TYPE deviceType;
} ATA_IDENTIFY_INFO;


typedef struct mvSataPMDeviceInfo
{
    MV_U16      vendorId;
    MV_U16      deviceId;
    MV_U8       productRevision;
    MV_U8       PMSpecRevision:4;
    MV_U8       numberOfPorts:4;
} MV_SATA_PM_DEVICE_INFO;


MV_BOOLEAN mvParseIdentifyResult(MV_U16_PTR  iden,ATA_IDENTIFY_INFO *pIdentifyInfo);

MV_SATA_DEVICE_TYPE mvGetSataDeviceType(MV_STORAGE_DEVICE_REGISTERS *mvStorageDevRegisters);

MV_BOOLEAN mvInitSataDisk(MV_SATA_ADAPTER   *pSataAdapter, MV_U8 channelIndex,
                          MV_U8 PMPort, ATA_IDENTIFY_INFO   *pIdentifyInfo,
                          MV_U16_PTR identifyBuffer
                         );

MV_BOOLEAN mvInitSataATAPI(MV_SATA_ADAPTER   *pSataAdapter, MV_U8 channelIndex,
                          MV_U8 PMPort, ATA_IDENTIFY_INFO   *pIdentifyInfo,
                          MV_U16_PTR identifyBuffer
                         );


MV_BOOLEAN  mvGetPMDeviceInfo(MV_SATA_ADAPTER   *pSataAdapter,
                              MV_U8 channelIndex,
                              MV_SATA_PM_DEVICE_INFO *pPMDeviceInfo);

MV_VOID mvSelectConfiguration(MV_SATA_ADAPTER        *pSataAdapter,
                              MV_U8                  channelIndex,
                              MV_SATA_GEN            gen,
                              MV_SATA_DEVICE_TYPE    connectedDevice,
                              MV_SATA_PM_DEVICE_INFO *pPMInfo,
                              MV_BOOLEAN             AllNCQ,
                              MV_BOOLEAN             AllTCQ,
                              MV_BOOLEAN             disks,
                              MV_EDMA_MODE           *pEdmaMode,
                              MV_SATA_SWITCHING_MODE *pSwitchingMode,
                              MV_BOOLEAN             *pUse128Entries);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCmvIALCommonh */

