#include "mv_include.h"

#ifdef CORE_SUPPORT_API

#include "core_api.h"
#include "core_exp.h"
#include "core_inter.h"
#include "core_init.h"
#include "com_error.h"

typedef MV_BOOLEAN (*CORE_Management_Command_Handler)(PCore_Driver_Extension, PMV_Request);
#ifndef _OS_BIOS
CORE_Management_Command_Handler core_pd_cmd_handler[];
#endif

MV_BOOLEAN
Core_MapHDId(
	IN PCore_Driver_Extension pCore,
	IN MV_U16 HDId,
	OUT MV_PU8 portId,
	OUT MV_PU8 deviceId
	);

MV_VOID
Core_GetHDInformation(
	PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
	IN PDomain_Device pDevice,
    OUT PHD_Info pHD 
	);

MV_VOID
Core_GetExpInformation(
	PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
	IN PDomain_Device pDevice,
    OUT PExp_Info pExp 
	);

MV_VOID
Core_GetPMInformation(
	PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
    OUT PPM_Info pPM 
	);

#ifndef BIOS_NOT_SUPPORT
MV_VOID
Core_GetHDConfiguration(
	PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
	IN PDomain_Device pDevice,
    OUT PHD_Config pHD 
	);

MV_VOID
Core_SetHDConfiguration(
	PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
	IN PDomain_Device pDevice,
    IN PHD_Config pHD 
	);
#endif /* #ifndef BIOS_NOT_SUPPORT */

/*
 * Exposed Functions
 */
MV_VOID
Core_GetHDInfo(
	IN MV_PVOID extension,
	IN MV_U16 HDId,
    OUT PHD_Info pHD 
	)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)extension;
	MV_U16 startId, endId;
	MV_U8 portId, deviceId;
	PDomain_Port pPort = NULL;
	PDomain_Device pDevice = NULL;
	MV_U16 i;

	if ( HDId==0xFF )	/* Get all the HD information */
	{
		/* First set invalid flag in buffer */
		for (i=0; i<MAX_HD_SUPPORTED_API; i++) {
			pHD[i].ID = i;
			pHD[i].Type = DEVICE_TYPE_NONE;
		}
		startId = 0; 
		endId = MV_MAX_HD_DEVICE_ID-1;
	}
	else
	{
		startId = HDId;
		endId = HDId;
	}

	for ( i=startId; i<=endId; i++ )
	{
		if ( Core_MapHDId(pCore, i, &portId, &deviceId) )
		{
			pPort = &pCore->Ports[portId];
			pDevice = &pPort->Device[deviceId];
			Core_GetHDInformation( pCore, pPort, pDevice, pHD );
		} 
/*
		else
		{
			pHD->ID = i;
			pHD->Type = DEVICE_TYPE_NONE;
		}
*/
		//MV_DASSERT( pHD->Id==i );
		pHD++;
	}
}

#ifdef SUPPORT_PM
MV_VOID
Core_GetExpInfo(
	IN MV_PVOID extension,
	IN MV_U16 ExpId,
    OUT PExp_Info pExp 
	)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)extension;
	MV_U16 startId, endId;
	MV_U8 portId, deviceId;
	PDomain_Port pPort = NULL;
	PDomain_Device pDevice = NULL;
	MV_U16 i;

	if ( ExpId==0xFF )	/* Get all the HD information */
	{
		startId = 0; 
		endId = MAX_EXPANDER_SUPPORTED-1;
	}
	else
	{
		startId = ExpId;
		endId = ExpId;
	}

	for ( i=startId; i<=endId; i++ )
	{
		// TBD: dunno how to map IDs for expander yet, just used HD map for now
		if ( Core_MapHDId(pCore, i, &portId, &deviceId) )
		{
			pPort = &pCore->Ports[portId];
			pDevice = &pPort->Device[deviceId];
			Core_GetExpInformation( pCore, pPort, pDevice, pExp );
		} 
		else
		{
			pExp->ID = i;
			pExp->Type = DEVICE_TYPE_NONE;
		}
		pExp++;
	}
}

MV_VOID
Core_GetPMInfo(
	IN MV_PVOID extension,
	IN MV_U16 PMId,
    OUT PPM_Info pPM 
	)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)extension;
	MV_U16 startId, endId;
	//MV_U8 portId, deviceId;
	PDomain_Port pPort = NULL;
	//PDomain_Device pDevice = NULL;
	MV_U16 i;

	if ( PMId==0xFF )	/* Get all the HD information */
	{
		startId = 0; 
		endId = pCore->Port_Num - 1;
	}
	else
	{
		startId = PMId;
		endId = PMId;
	}

	for ( i=startId; i<=endId; i++ )
	{
		pPort = &pCore->Ports[i];
		if ( pPort->Type != PORT_TYPE_PM )
		{
			if ( PMId != 0xFF )
			{
				// TBD: not a PM, return error
			}
		}
		else 
		{
			Core_GetPMInformation( pCore, pPort, pPM );
			pPM++;
		}
	}
}
#endif	/* #ifdef SUPPORT_PM */

#ifndef BIOS_NOT_SUPPORT
MV_VOID
Core_GetHDConfig(
	IN MV_PVOID extension,
	IN MV_U16 HDId,
    OUT PHD_Config pHD 
	)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)extension;
	MV_U16 startId, endId;
	MV_U8 portId, deviceId;
	PDomain_Port pPort = NULL;
	PDomain_Device pDevice = NULL;
	MV_U16 i;

	if ( HDId==0xFF )	/* Get all the HD configuration */
	{
		/* First set invalid flag in buffer */
		for (i=0; i<MAX_HD_SUPPORTED_API; i++)
			pHD[i].HDID = 0xFF;
		startId = 0; 
		endId = MV_MAX_HD_DEVICE_ID-1;
	}
	else
	{
		startId = HDId;
		endId = HDId;
	}


	for ( i=startId; i<=endId; i++ )
	{
		if ( Core_MapHDId(pCore, i, &portId, &deviceId) )
		{
			pPort = &pCore->Ports[portId];
			pDevice = &pPort->Device[deviceId];
			Core_GetHDConfiguration( pCore, pPort, pDevice, pHD );
		} 
//		else
//			pHD->HDID = 0xFF;
		pHD++;
	}
}
#endif	/* #ifndef BIOS_NOT_SUPPORT */
/*
 * Internal Functions
 */
MV_BOOLEAN
Core_MapHDId(
	IN PCore_Driver_Extension pCore,
	IN MV_U16 HDId,
	OUT MV_PU8 portId,
	OUT MV_PU8 deviceId
	)
{
	if ( portId ) *portId = PATA_MapPortId(HDId);
	if ( deviceId ) *deviceId = PATA_MapDeviceId(HDId);

	if ( ((portId)&&(*portId>=MAX_PORT_NUMBER))
		|| ((deviceId)&&(*deviceId>=MAX_DEVICE_PER_PORT))
		)
		return MV_FALSE;
	else
		return MV_TRUE;
}

MV_VOID
Core_GetHDInformation(
	PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
	IN PDomain_Device pDevice,
    OUT PHD_Info pHD 
	)
{
	pHD->ID = pDevice->Id ;	
	if ( !(pDevice->Status&DEVICE_STATUS_FUNCTIONAL) )
	{
		pHD->Type = DEVICE_TYPE_NONE;
		return;
	}

	// TBD: check if device type is correct; if not, generate sense
	pHD->Type = DEVICE_TYPE_HD;
	pHD->ParentID = pPort->Id;
#if 0
	pHD->ParentType = DEVICE_TYPE_PORT;
	pHD->PhyID = 0;
#else
	if ( pPort->Type==PORT_TYPE_PM )
	{
		pHD->ParentType = DEVICE_TYPE_PM;
		pHD->PhyID = pDevice->PM_Number;
	}
	else
	{
		pHD->ParentType = DEVICE_TYPE_PORT;
		pHD->PhyID = pPort->Id;
	}
#endif

	pHD->Status = 0;
	if ( pPort->Type==PORT_TYPE_PATA )
        pHD->HDType = HD_TYPE_PATA;
	else
		pHD->HDType = HD_TYPE_SATA;
	if ( pDevice->Device_Type&DEVICE_TYPE_ATAPI )
		pHD->HDType |= HD_TYPE_ATAPI;

	pHD->PIOMode = pDevice->PIO_Mode;
	pHD->MDMAMode = pDevice->MDMA_Mode;
	pHD->UDMAMode = pDevice->UDMA_Mode;

	if ( pDevice->Capacity & DEVICE_CAPACITY_NCQ_SUPPORTED )
		pHD->FeatureSupport |= HD_FEATURE_NCQ;		
	if ( pDevice->Capacity & DEVICE_CAPACITY_WRITECACHE_SUPPORTED )
		pHD->FeatureSupport |= HD_FEATURE_WRITE_CACHE;		
	if ( pDevice->Capacity & DEVICE_CAPACITY_48BIT_SUPPORTED )
		pHD->FeatureSupport |= HD_FEATURE_48BITS;		
	if ( pDevice->Capacity & DEVICE_CAPACITY_SMART_SUPPORTED )
		pHD->FeatureSupport |= HD_FEATURE_SMART;	

	if ( pDevice->Capacity & DEVICE_CAPACITY_RATE_1_5G )
		pHD->FeatureSupport |= HD_FEATURE_1_5G;
	else if ( pDevice->Capacity & DEVICE_CAPACITY_RATE_3G )
		pHD->FeatureSupport |= HD_FEATURE_3G;

	MV_CopyMemory(pHD->Model, pDevice->Model_Number, 40);
#ifndef BIOS_NOT_SUPPORT
	MV_CopyMemory(pHD->SerialNo, pDevice->Serial_Number, 20);
	MV_CopyMemory(pHD->FWVersion, pDevice->Firmware_Revision, 8);
#endif

	*(MV_PU32)pHD->WWN = pDevice->WWN;
	pHD->Size = pDevice->Max_LBA;
}

#ifdef SUPPORT_PM
MV_VOID
Core_GetExpInformation(
	PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
	IN PDomain_Device pDevice,
    OUT PExp_Info pExp 
	)
{
	// TBD
}

MV_VOID
Core_GetPMInformation(
	PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
    OUT PPM_Info pPM 
	)
{
#ifndef RAID_SIMULATE_CONFIGURATION
	//MV_U32 temp;
	//MV_LPVOID portMmio = pPort->Mmio_Base;

	pPM->Type = DEVICE_TYPE_PM;
	pPM->ParentType = DEVICE_TYPE_PORT;
	pPM->ID = pPort->Id;
	pPM->ParentID = pPort->Id;

	pPM->VendorId = pPort->PM_Vendor_Id;
	pPM->DeviceId = pPort->PM_Device_Id;
	pPM->ProductRevision = pPort->PM_Product_Revision;
	pPM->PMSpecRevision = pPort->PM_Spec_Revision;
	pPM->NumberOfPorts = pPort->PM_Num_Ports;
#endif
}
#endif	/* #ifdef SUPPORT_PM */

#ifndef BIOS_NOT_SUPPORT
MV_VOID
Core_GetHDConfiguration(
	PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
	IN PDomain_Device pDevice,
    OUT PHD_Config pHD 
	)
{
	if ( !(pDevice->Status & DEVICE_STATUS_FUNCTIONAL) )
	{
		pHD->HDID = 0xFF;
		return;
	}

	pHD->HDID = pDevice->Id ;	

	if (pDevice->Setting & DEVICE_SETTING_WRITECACHE_ENABLED)
		pHD->WriteCacheOn = MV_TRUE;
	else
		pHD->WriteCacheOn = MV_FALSE;

	if ( pDevice->Setting & DEVICE_SETTING_SMART_ENABLED )
		pHD->SMARTOn = MV_TRUE;
	else
		pHD->SMARTOn = MV_FALSE;
}
#endif /* #ifndef BIOS_NOT_SUPPORT */

MV_BOOLEAN core_pd_request_get_HD_info(PCore_Driver_Extension pCore, PMV_Request pMvReq)
{
	PHD_Info pHDInfo = (PHD_Info)pMvReq->Data_Buffer;
	MV_U16 HDID; 

#ifdef DEBUG_BIOS
	//MV_DUMP32(0xCCCCEEE1);
#endif
	MV_CopyMemory(&HDID, &pMvReq->Cdb[2], 2);
	Core_GetHDInfo( pCore, HDID, pHDInfo );
	if (HDID != 0xFF && pHDInfo->Type == DEVICE_TYPE_NONE)
	{
		if (pMvReq->Sense_Info_Buffer != NULL)
			((MV_PU8)pMvReq->Sense_Info_Buffer)[0] = ERR_INVALID_HD_ID;
		pMvReq->Scsi_Status = REQ_STATUS_ERROR_WITH_SENSE;
	}
	else
		pMvReq->Scsi_Status = REQ_STATUS_SUCCESS;
	return MV_TRUE;
}

#ifdef SUPPORT_PM
MV_BOOLEAN core_pd_request_get_expander_info( PCore_Driver_Extension pCore, PMV_Request pMvReq )
{
	PExp_Info pExpInfo = (PExp_Info)pMvReq->Data_Buffer;
	MV_U16 ExpID; 
	MV_U8	status = REQ_STATUS_SUCCESS;

	MV_CopyMemory(&ExpID, &pMvReq->Cdb[2], 2);
	if (ExpID != 0xFF && ExpID > MAX_EXPANDER_SUPPORTED)
	{
		status = ERR_INVALID_EXP_ID;
	}
	else
	{
		Core_GetExpInfo( pCore, ExpID, pExpInfo );

		if (ExpID != 0xFF && pExpInfo->Type == DEVICE_TYPE_NONE)
		{
			status = ERR_INVALID_EXP_ID;
		}
	}

	if (status != REQ_STATUS_SUCCESS)
	{
		if (pMvReq->Sense_Info_Buffer != NULL)
			((MV_PU8)pMvReq->Sense_Info_Buffer)[0] = status;
		pMvReq->Scsi_Status = REQ_STATUS_ERROR_WITH_SENSE;
	}
	else
		pMvReq->Scsi_Status = REQ_STATUS_SUCCESS;

	return MV_TRUE;
}

MV_BOOLEAN core_pd_request_get_PM_info( PCore_Driver_Extension pCore, PMV_Request pMvReq )
{
	PPM_Info pPMInfo = (PPM_Info)pMvReq->Data_Buffer;
	MV_U16 PMID; 

	MV_CopyMemory(&PMID, &pMvReq->Cdb[2], 2);
	if (PMID != 0xFF && PMID > MAX_PM_SUPPORTED)
	{
		if (pMvReq->Sense_Info_Buffer != NULL)
			((MV_PU8)pMvReq->Sense_Info_Buffer)[0] = ERR_INVALID_PM_ID;
		pMvReq->Scsi_Status = REQ_STATUS_ERROR_WITH_SENSE;
		return MV_TRUE;
	}

	Core_GetPMInfo( pCore, PMID, pPMInfo );
	
	pMvReq->Scsi_Status = REQ_STATUS_SUCCESS;
	return MV_TRUE;
}
#endif	/*#ifdef SUPPORT_PM */

#ifndef BIOS_NOT_SUPPORT
MV_BOOLEAN core_pd_request_get_HD_config( PCore_Driver_Extension pCore, PMV_Request pMvReq )
{
	PHD_Config pHDConfig = (PHD_Config)pMvReq->Data_Buffer;
	MV_U16 ConfigID; 

	MV_CopyMemory(&ConfigID, &pMvReq->Cdb[2], 2);
	Core_GetHDConfig( pCore, ConfigID, pHDConfig );

	if (ConfigID != 0xFF && pHDConfig->HDID == 0xFF)
	{
		if (pMvReq->Sense_Info_Buffer != NULL)
			((MV_PU8)pMvReq->Sense_Info_Buffer)[0] = ERR_INVALID_HD_ID;
		pMvReq->Scsi_Status = REQ_STATUS_ERROR_WITH_SENSE;
	}
	else
		pMvReq->Scsi_Status = REQ_STATUS_SUCCESS;

	return MV_TRUE;
}

MV_BOOLEAN core_pd_request_get_HD_status( PCore_Driver_Extension pCore, PMV_Request pMvReq )
{
	PHD_Status pHDStatus = (PHD_Status)pMvReq->Data_Buffer;
	PDomain_Port pPort = NULL;
	PDomain_Device pDevice = NULL;
	MV_U8 portId, deviceId;
	MV_U16 HDId; 
	MV_U8	cacheMode = 0;
	MV_U8 status = REQ_STATUS_SUCCESS;

	MV_CopyMemory(&HDId, &pMvReq->Cdb[2], 2);

	if ( Core_MapHDId(pCore, HDId, &portId, &deviceId) )
	{
		pPort = &pCore->Ports[portId];
		pDevice = &pPort->Device[deviceId];

		if (pDevice->Setting & DEVICE_SETTING_SMART_ENABLED)
		{
			if ( !(pDevice->Status & DEVICE_STATUS_FUNCTIONAL) )
			{
				status = ERR_INVALID_HD_ID;
			}
			else 
			{
				if (pMvReq->Cdb[4] == APICDB4_PD_SMART_RETURN_STATUS)
				{
					cacheMode = CDB_CORE_SMART_RETURN_STATUS;
				}
				else
					status = ERR_INVALID_REQUEST;
			}

			if (status == REQ_STATUS_SUCCESS)
			{
				// Convert it into SCSI_CMD_MARVELL_SPECIFIC request.
				pMvReq->Cdb[0] = SCSI_CMD_MARVELL_SPECIFIC;
				pMvReq->Cdb[1] = CDB_CORE_MODULE;
				pMvReq->Cdb[2] = cacheMode;
				pMvReq->Device_Id = pDevice->Id;
				if (pHDStatus)
					pHDStatus->HDID = pDevice->Id;
			}
		}
		else
			status = ERR_INVALID_REQUEST;
	} 
	else
	{
		status = ERR_INVALID_HD_ID;
	}

	if (status != REQ_STATUS_SUCCESS)
	{
		if (pMvReq->Sense_Info_Buffer != NULL)
			((MV_PU8)pMvReq->Sense_Info_Buffer)[0] = status;
		pMvReq->Scsi_Status = REQ_STATUS_ERROR_WITH_SENSE;
		return MV_TRUE;	
	}
	else
	{
		pMvReq->Scsi_Status = REQ_STATUS_SUCCESS;
		return MV_FALSE;	// Need to access hardware.
	}
}

MV_BOOLEAN core_pd_request_set_HD_config( PCore_Driver_Extension pCore, PMV_Request pMvReq )
{
	//PHD_Config pHDConfig = (PHD_Config)pMvReq->Data_Buffer;
	PDomain_Port pPort = NULL;
	PDomain_Device pDevice = NULL;
	MV_U8 portId, deviceId;
	MV_U16 HDId; 
	MV_U8	cacheMode = 0;
	MV_U8 status = REQ_STATUS_PENDING;

	MV_CopyMemory(&HDId, &pMvReq->Cdb[2], 2);

	if ( Core_MapHDId(pCore, HDId, &portId, &deviceId) )
	{
		pPort = &pCore->Ports[portId];
		pDevice = &pPort->Device[deviceId];

		if ( !(pDevice->Status & DEVICE_STATUS_FUNCTIONAL) )
		{
			status = ERR_INVALID_HD_ID;
		}
		else if ( pDevice->Device_Type & DEVICE_TYPE_ATAPI )
		{
			status = ERR_INVALID_REQUEST;
		}
		else 
		{
			if (pMvReq->Cdb[4] == APICDB4_PD_SET_WRITE_CACHE_OFF)
			{
				cacheMode = CDB_CORE_DISABLE_WRITE_CACHE;
			}
			else if (pMvReq->Cdb[4] == APICDB4_PD_SET_WRITE_CACHE_ON)
			{
				cacheMode = CDB_CORE_ENABLE_WRITE_CACHE;
			}
			else if (pMvReq->Cdb[4] == APICDB4_PD_SET_SMART_OFF)
			{
				if ( !(pDevice->Setting&DEVICE_SETTING_SMART_ENABLED) )
					status = REQ_STATUS_SUCCESS;
				cacheMode = CDB_CORE_DISABLE_SMART;
			}
			else if (pMvReq->Cdb[4] == APICDB4_PD_SET_SMART_ON)
			{
				if ( pDevice->Setting&DEVICE_SETTING_SMART_ENABLED )
					status = REQ_STATUS_SUCCESS;
				cacheMode = CDB_CORE_ENABLE_SMART;
			}
			else
			{
				status = ERR_INVALID_REQUEST;
			}
		}

		if (status == REQ_STATUS_PENDING)
		{
			// Convert it into SCSI_CMD_MARVELL_SPECIFIC request.
			pMvReq->Cdb[0] = SCSI_CMD_MARVELL_SPECIFIC;
			pMvReq->Cdb[1] = CDB_CORE_MODULE;
			pMvReq->Cdb[2] = cacheMode;
			pMvReq->Device_Id = pDevice->Id;
		}
	} 
	else
	{
		status = ERR_INVALID_HD_ID;
	}

	if (status == REQ_STATUS_SUCCESS)
	{
		pMvReq->Scsi_Status = status;
		return MV_TRUE;
	}
	else if (status == REQ_STATUS_PENDING)
	{
		//pMvReq->Scsi_Status = REQ_STATUS_SUCCESS;
		return MV_FALSE;	// Need to access hardware.
	}
	else
	{
		if (pMvReq->Sense_Info_Buffer != NULL)
			((MV_PU8)pMvReq->Sense_Info_Buffer)[0] = status;
		pMvReq->Scsi_Status = REQ_STATUS_ERROR_WITH_SENSE;
		return MV_TRUE;	
	}
}

MV_BOOLEAN core_pd_request_BSL_dump( PCore_Driver_Extension pCore, PMV_Request pMvReq )
{
	// TBD
	pMvReq->Scsi_Status = REQ_STATUS_ERROR;
	return MV_TRUE;
}

MV_BOOLEAN core_pd_request_HD_MP_check( PCore_Driver_Extension pCore, PMV_Request pMvReq )
{
	// TBD
	pMvReq->Scsi_Status = REQ_STATUS_ERROR;
	return MV_TRUE;
}

MV_BOOLEAN core_pd_request_HD_get_MP_status( PCore_Driver_Extension pCore, PMV_Request pMvReq )
{
	// TBD
	pMvReq->Scsi_Status = REQ_STATUS_ERROR;
	return MV_TRUE;
}
#endif	/* #ifndef BIOS_NOT_SUPPORT */

CORE_Management_Command_Handler BASEATTR core_pd_cmd_handler[APICDB1_PD_MAX] = 
{
	core_pd_request_get_HD_info,
#ifdef SUPPORT_PM
	core_pd_request_get_expander_info,
	core_pd_request_get_PM_info,
#else
	NULL,
	NULL,
#endif
#ifndef BIOS_NOT_SUPPORT
	core_pd_request_get_HD_config,
	core_pd_request_set_HD_config,
	core_pd_request_BSL_dump,
	core_pd_request_HD_MP_check,
	core_pd_request_HD_get_MP_status,
	core_pd_request_get_HD_status
#else
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
#endif
};

MV_BOOLEAN 
Core_pd_command(
	IN MV_PVOID extension,
	IN PMV_Request pReq
	)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)extension;

	if ( pReq->Cdb[1] >= APICDB1_PD_MAX ) 
	{
		pReq->Scsi_Status = REQ_STATUS_INVALID_PARAMETER;
		return MV_TRUE;
	}
	return core_pd_cmd_handler[pReq->Cdb[1]](pCore, pReq);
}

#endif

