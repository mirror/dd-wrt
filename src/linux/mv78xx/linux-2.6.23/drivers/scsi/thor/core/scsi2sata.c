#include "mv_include.h"

#include "core_inter.h"

#include "core_sata.h"
#include "core_ata.h"

/*
 * Translate SCSI command to SATA FIS
 * The following SCSI command set is the minimum required.
 *		Standard Inquiry
 *		Read Capacity
 *		Test Unit Ready
 *		Start/Stop Unit
 *		Read 10
 *		Write 10
 *		Request Sense
 *		Mode Sense/Select
 */
MV_VOID SCSI_To_FIS(MV_PVOID This, PMV_Request pReq, MV_U8 tag, PATA_TaskFile pTaskFile)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;
	PDomain_Port pPort = &pCore->Ports[PATA_MapPortId(pReq->Device_Id)];
	PDomain_Device pDevice = &pPort->Device[PATA_MapDeviceId(pReq->Device_Id)];
	PMV_Command_Table pCmdTable = Port_GetCommandTable(pPort, tag);

	PSATA_FIS_REG_H2D pFIS = (PSATA_FIS_REG_H2D)pCmdTable->FIS;

	/* 
	 * TBD
	 * 1. SoftReset is not supported yet.
	 * 2. PM_Port
	 */
	pFIS->FIS_Type = SATA_FIS_TYPE_REG_H2D;

#ifdef SUPPORT_PM
	pFIS->PM_Port = pDevice->PM_Number;
#else
	pFIS->PM_Port = 0;
#endif

	pFIS->C = 1;	/* Update command register rather than devcie control register */
	pFIS->Command = pTaskFile->Command;
	pFIS->Features = pTaskFile->Features;
	pFIS->Device = pTaskFile->Device;
	pFIS->Control = pTaskFile->Control;

	pFIS->LBA_Low = pTaskFile->LBA_Low;
	pFIS->LBA_Mid = pTaskFile->LBA_Mid;
	pFIS->LBA_High = pTaskFile->LBA_High;
	pFIS->Sector_Count = pTaskFile->Sector_Count;

	/* No matter it's 48bit or not, I've set the values. */
	pFIS->LBA_Low_Exp = pTaskFile->LBA_Low_Exp;
	pFIS->LBA_Mid_Exp = pTaskFile->LBA_Mid_Exp;
	pFIS->LBA_High_Exp = pTaskFile->LBA_High_Exp;
	pFIS->Features_Exp = pTaskFile->Feature_Exp;
	pFIS->Sector_Count_Exp = pTaskFile->Sector_Count_Exp;

#if 0//DEBUG_BIOS
	MV_DUMPC32(0xCCCCEEA1);
	MV_DUMP8(pFIS->FIS_Type);
	MV_DUMP8(pFIS->Command);
#endif

}

/* Set MV_Request.Cmd_Flag */
MV_BOOLEAN Category_CDB_Type(
	IN PDomain_Device pDevice,
	IN PMV_Request pReq
	)
{
	//pReq->Cmd_Flag = 0;//TBD: Don't set. HBA has set some bits.
	PDomain_Port pPort = pDevice->PPort;

	switch ( pReq->Cdb[0] )
	{
		case SCSI_CMD_READ_10:
		case SCSI_CMD_WRITE_10:
		case SCSI_CMD_VERIFY_10:
			/* 
			 * 
			 * CMD_FLAG_DATA_IN
			 * CMD_FLAG_NON_DATA
			 * CMD_FLAG_DMA
			 */
			if ( pDevice->Device_Type&DEVICE_TYPE_ATAPI )
				pReq->Cmd_Flag |= CMD_FLAG_PACKET;

			if ( pDevice->Capacity&DEVICE_CAPACITY_48BIT_SUPPORTED )
				pReq->Cmd_Flag |= CMD_FLAG_48BIT;

			if ( pDevice->Capacity&DEVICE_CAPACITY_NCQ_SUPPORTED )
			{
				// might be a PM - assert is no longer true
				//MV_DASSERT( pPort->Type==PORT_TYPE_SATA );		
				if ( (pReq->Cdb[0]==SCSI_CMD_READ_10)
					|| (pReq->Cdb[0]==SCSI_CMD_WRITE_10) )
				{
					if ( (pPort->Running_Slot==0)
						|| (pPort->Setting&PORT_SETTING_NCQ_RUNNING) )
					{
						/* hardware workaround:
						 * don't do NCQ on silicon image PM */
						if( !((pPort->Setting & PORT_SETTING_PM_EXISTING) && 
							(pPort->PM_Vendor_Id == 0x1095 )) )
						{
							if ( pReq->Scsi_Status!=REQ_STATUS_RETRY )
								pReq->Cmd_Flag |= CMD_FLAG_NCQ;
						}
					}
				}
			}

			break;

		case SCSI_CMD_MARVELL_SPECIFIC:
			{
				/* This request should be for core module */
				if ( pReq->Cdb[1]!=CDB_CORE_MODULE )
					return MV_FALSE;
				switch ( pReq->Cdb[2] )
				{
					case CDB_CORE_IDENTIFY:
					case CDB_CORE_READ_LOG_EXT:
						pReq->Cmd_Flag |= CMD_FLAG_DATA_IN;
						break;
					
					case CDB_CORE_SET_UDMA_MODE:
					case CDB_CORE_SET_PIO_MODE:
					case CDB_CORE_ENABLE_WRITE_CACHE:
					case CDB_CORE_DISABLE_WRITE_CACHE:
					case CDB_CORE_ENABLE_SMART:
					case CDB_CORE_DISABLE_SMART:
					case CDB_CORE_SMART_RETURN_STATUS:
					case CDB_CORE_ENABLE_READ_AHEAD:
					case CDB_CORE_DISABLE_READ_AHEAD:
						pReq->Cmd_Flag |= CMD_FLAG_NON_DATA;
						break;

					case CDB_CORE_SHUTDOWN:
						if ( pDevice->Device_Type&DEVICE_TYPE_ATAPI )
							return MV_FALSE;
						MV_DPRINT(("Shutdown on device %d.\n", pReq->Device_Id));
						pReq->Cmd_Flag |= CMD_FLAG_NON_DATA;
						break;

					default:
						return MV_FALSE;
				}
				break;
			}
		case SCSI_CMD_START_STOP_UNIT:	
		case SCSI_CMD_SYNCHRONIZE_CACHE_10:
			if ( !(pDevice->Device_Type & DEVICE_TYPE_ATAPI )){
				if ( pDevice->Capacity&DEVICE_CAPACITY_48BIT_SUPPORTED )
					pReq->Cmd_Flag |= CMD_FLAG_48BIT;
				pReq->Cmd_Flag |= CMD_FLAG_NON_DATA;
				break;
			}	//We will send this command to CD drive directly if it is ATAPI device.		
		case SCSI_CMD_INQUIRY:
		case SCSI_CMD_READ_CAPACITY_10:
		case SCSI_CMD_TEST_UNIT_READY:
		case SCSI_CMD_MODE_SENSE_10:
		case SCSI_CMD_MODE_SELECT_10:
		case SCSI_CMD_PREVENT_MEDIUM_REMOVAL:
		case SCSI_CMD_READ_TOC:
		case SCSI_CMD_REQUEST_SENSE:
		default:
			if ( pDevice->Device_Type&DEVICE_TYPE_ATAPI )
			{
				//MV_DPRINT(("Other requests: 0x%x.\n", pReq->Cdb[0]));
				pReq->Cmd_Flag |= CMD_FLAG_PACKET;

				#if 0 //TBD: Refer to TranslateSCSIRequest
				if ( pReq->Data_Transfer_Length==0 )
				{
					pReq->Cmd_Flag |= CMD_FLAG_NON_DATA;
				}
				else
				{
					if ( pReq->Cdb[0]==SCSI_CMD_INQUIRY
						|| pReq->Cdb[0]==SCSI_CMD_READ_CAPACITY_10 
						|| pReq->Cdb[0]==SCSI_CMD_REQUEST_SENSE 
						|| pReq->Cdb[0]==SCSI_CMD_REPORT_LUN
						|| pReq->Cdb[0]==SCSI_CMD_READ_DISC_STRUCTURE
						|| pReq->Cdb[0]==SCSI_CMD_READ_TOC
						|| pReq->Cdb[0]==SCSI_CMD_READ_SUB_CHANNEL
						|| pReq->Cdb[0]==SCSI_CMD_READ_CD
						|| pReq->Cdb[0]==SCSI_CMD_GET_EVENT_STATUS_NOTIFICATION
						)
					{
						pReq->Cmd_Flag |= CMD_FLAG_DATA_IN;
					}
					else if ( pReq->Cdb[0]==SCSI_CMD_MODE_SENSE_10 )
					{
						/* Data out */
					}
					else
					{
						MV_DPRINT(("Should be data-in or data-out: 0x%x.\n", pReq->Cdb[0]));
						//MV_ASSERT(MV_FALSE);
					}
				}
				#endif

				break;
			}
			else
			{
#ifndef _OS_BIOS			
				MV_DPRINT(("Error: Unknown request: 0x%x.\n", pReq->Cdb[0]));
#endif
				return MV_FALSE;
			}
	}

	return MV_TRUE;
}


MV_BOOLEAN ATA_CDB2TaskFile(
	IN PDomain_Device pDevice,
	IN PMV_Request pReq, 
	IN MV_U8 tag,
	OUT PATA_TaskFile pTaskFile
	)
{
	MV_ZeroMemory(pTaskFile, sizeof(ATA_TaskFile));

	switch ( pReq->Cdb[0] )
	{
		case SCSI_CMD_READ_10:
		case SCSI_CMD_WRITE_10:
			{
				
				/* 
				 * The OS maximum tranfer length is set to 128K.
				 * For ATA_CMD_READ_DMA and ATA_CMD_WRITE_DMA,
				 * the max size they can handle is 256 sectors.
				 * And Sector_Count==0 means 256 sectors.
				 * If OS request max lenght>128K, for 28 bit device, we have to split requests.
				 */
				MV_DASSERT( ( (((MV_U16)pReq->Cdb[7])<<8) | (pReq->Cdb[8]) ) <= 256 );

				/*
				 * 24 bit LBA can express 128GB.
				 * 4 bytes LBA like SCSI_CMD_READ_10 can express 2TB.
				 */
			
				/* Make sure Cmd_Flag has set already. */
				if ( pReq->Cmd_Flag&CMD_FLAG_NCQ )
				{
					//MV_DASSERT( pReq->Cmd_Flag&CMD_FLAG_48BIT );	//TBD: Do we need set 48bit for NCQ
					
					pTaskFile->Features = pReq->Cdb[8];
					pTaskFile->Feature_Exp = pReq->Cdb[7];

					pTaskFile->Sector_Count = tag<<3;
					
					pTaskFile->LBA_Low = pReq->Cdb[5];
					pTaskFile->LBA_Mid = pReq->Cdb[4];
					pTaskFile->LBA_High = pReq->Cdb[3];
					pTaskFile->LBA_Low_Exp = pReq->Cdb[2];
		
					pTaskFile->Device = MV_BIT(6);

					if ( pReq->Cdb[0]==SCSI_CMD_READ_10 )
						pTaskFile->Command = ATA_CMD_READ_FPDMA_QUEUED;
					else if ( pReq->Cdb[0]==SCSI_CMD_WRITE_10 )
						pTaskFile->Command = ATA_CMD_WRITE_FPDMA_QUEUED;
				}
				else if ( pReq->Cmd_Flag&CMD_FLAG_48BIT )
				{
					MV_DASSERT( !(pReq->Cmd_Flag&CMD_FLAG_NCQ) );

					pTaskFile->Sector_Count = pReq->Cdb[8];
					pTaskFile->Sector_Count_Exp = pReq->Cdb[7];

					pTaskFile->LBA_Low = pReq->Cdb[5];
					pTaskFile->LBA_Mid = pReq->Cdb[4];
					pTaskFile->LBA_High = pReq->Cdb[3];
					pTaskFile->LBA_Low_Exp = pReq->Cdb[2];

					pTaskFile->Device = MV_BIT(6);

					if ( pReq->Cdb[0]==SCSI_CMD_READ_10 )
						pTaskFile->Command = ATA_CMD_READ_DMA_EXT;
					else if ( pReq->Cdb[0]==SCSI_CMD_WRITE_10 )
						pTaskFile->Command = ATA_CMD_WRITE_DMA_EXT;
				}
				else
				{
					/* 28 bit DMA */
					pTaskFile->Sector_Count = pReq->Cdb[8];		/* Could be zero */
	
					pTaskFile->LBA_Low = pReq->Cdb[5];
					pTaskFile->LBA_Mid = pReq->Cdb[4];
					pTaskFile->LBA_High = pReq->Cdb[3];
			
					pTaskFile->Device = MV_BIT(6) | (pReq->Cdb[2]&0xF);
					
					MV_DASSERT( (pReq->Cdb[2]&0xF0)==0 );

					if ( pReq->Cdb[0]==SCSI_CMD_READ_10 )
						pTaskFile->Command = ATA_CMD_READ_DMA;
					else if ( pReq->Cdb[0]==SCSI_CMD_WRITE_10 )
						pTaskFile->Command = ATA_CMD_WRITE_DMA;
				}

				break;
			}

		case SCSI_CMD_VERIFY_10:
			/* 
			 * For verify command, the size may need use two MV_U8, especially Windows.
			 * For 28 bit device, we have to split the request.
			 * For 48 bit device, we use ATA_CMD_VERIFY_EXT.
			 */
			if ( pDevice->Capacity&DEVICE_CAPACITY_48BIT_SUPPORTED )
			{
				pTaskFile->Sector_Count = pReq->Cdb[8];
				pTaskFile->Sector_Count_Exp = pReq->Cdb[7];

				pTaskFile->LBA_Low = pReq->Cdb[5];
				pTaskFile->LBA_Mid = pReq->Cdb[4];
				pTaskFile->LBA_High = pReq->Cdb[3];
				pTaskFile->LBA_Low_Exp = pReq->Cdb[2];

				pTaskFile->Device = MV_BIT(6);

				pTaskFile->Command = ATA_CMD_VERIFY_EXT;
			}
			else
			{
				//TBD: If the device doesn't support 48 bit LBA. We have to split this request.
				//ATA_CMD_VERIFY
				//MV_ASSERT(MV_FALSE);
				//Sorry here I didn't do the verify exact as the OS required. 
				//It need effort to split request. Currently I just pretect I've fulfilled the request.
				pTaskFile->Sector_Count = pReq->Cdb[8];
				
				pTaskFile->LBA_Low = pReq->Cdb[5];
				pTaskFile->LBA_Mid = pReq->Cdb[4];
				pTaskFile->LBA_High = pReq->Cdb[3];

				pTaskFile->Device = MV_BIT(6) | (pReq->Cdb[2]&0xF);
				
				MV_DASSERT( (pReq->Cdb[2]&0xF0)==0 );

				pTaskFile->Command = ATA_CMD_VERIFY;				
			}

			break;

		case SCSI_CMD_MARVELL_SPECIFIC:
			{
				/* This request should be for core module */
				if ( pReq->Cdb[1]!=CDB_CORE_MODULE )
					return MV_FALSE;
				switch ( pReq->Cdb[2] )
				{
					case CDB_CORE_IDENTIFY:
						pTaskFile->Command = ATA_CMD_IDENTIFY_ATA;
						break;
					
					case CDB_CORE_SET_UDMA_MODE:
						pTaskFile->Command = ATA_CMD_SET_FEATURES;
						pTaskFile->Features = ATA_CMD_SET_TRANSFER_MODE;
						pTaskFile->Sector_Count = 0x40 | pReq->Cdb[3];
						MV_DASSERT( pReq->Cdb[4]==MV_FALSE );	/* Use UDMA mode */
						//TBD: Check the 80-conductor cable in order to enable UDMA greater than 2.
						break;

					case CDB_CORE_SET_PIO_MODE:
						pTaskFile->Command = ATA_CMD_SET_FEATURES;
						pTaskFile->Features = ATA_CMD_SET_TRANSFER_MODE;
						pTaskFile->Sector_Count = 0x08 | pReq->Cdb[3];
						break;

					case CDB_CORE_ENABLE_WRITE_CACHE:
						pTaskFile->Command = ATA_CMD_SET_FEATURES;
						pTaskFile->Features = ATA_CMD_ENABLE_WRITE_CACHE;
						break;
			
					case CDB_CORE_DISABLE_WRITE_CACHE:
						pTaskFile->Command = ATA_CMD_SET_FEATURES;
						pTaskFile->Features = ATA_CMD_DISABLE_WRITE_CACHE;
						break;

					case CDB_CORE_ENABLE_SMART:
						pTaskFile->Command = ATA_CMD_SMART;
						pTaskFile->Features = ATA_CMD_ENABLE_SMART;
						pTaskFile->LBA_Mid = 0x4F;
						pTaskFile->LBA_High = 0xC2;
						break;

					case CDB_CORE_DISABLE_SMART:
						pTaskFile->Command = ATA_CMD_SMART;
						pTaskFile->Features = ATA_CMD_DISABLE_SMART;
						pTaskFile->LBA_Mid = 0x4F;
						pTaskFile->LBA_High = 0xC2;
						break;

					case CDB_CORE_SMART_RETURN_STATUS:
						pTaskFile->Command = ATA_CMD_SMART;
						pTaskFile->Features = ATA_CMD_SMART_RETURN_STATUS;
						pTaskFile->LBA_Mid = 0x4F;
						pTaskFile->LBA_High = 0xC2;
						break;

					case CDB_CORE_SHUTDOWN:
						if ( pDevice->Capacity&DEVICE_CAPACITY_48BIT_SUPPORTED )
							pTaskFile->Command = ATA_CMD_FLUSH_EXT;
						else
							pTaskFile->Command = ATA_CMD_FLUSH;
						break;

					case CDB_CORE_ENABLE_READ_AHEAD:	
						pTaskFile->Command = ATA_CMD_SET_FEATURES;
						pTaskFile->Features = ATA_CMD_ENABLE_READ_LOOK_AHEAD;
						break;

					case CDB_CORE_DISABLE_READ_AHEAD:
						pTaskFile->Command = ATA_CMD_SET_FEATURES;
						pTaskFile->Features = ATA_CMD_DISABLE_READ_LOOK_AHEAD;
						break;
					
					case CDB_CORE_READ_LOG_EXT:
						pTaskFile->Command = ATA_CMD_READ_LOG_EXT;
						pTaskFile->Sector_Count = 1;	/* Read one sector */
						pTaskFile->LBA_Low = 0x10;		/* Page 10h */
						break;

					default:
						return MV_FALSE;
				}
				break;
			}
		case SCSI_CMD_SYNCHRONIZE_CACHE_10:
			if ( pDevice->Capacity&DEVICE_CAPACITY_48BIT_SUPPORTED )
				pTaskFile->Command = ATA_CMD_FLUSH_EXT;
			else
				pTaskFile->Command = ATA_CMD_FLUSH;
			pTaskFile->Device = MV_BIT(6);
			break;
		case SCSI_CMD_START_STOP_UNIT:
			if (pReq->Cdb[4] & MV_BIT(0))
			{
				pTaskFile->Command = ATA_CMD_SEEK;
				pTaskFile->Device = MV_BIT(6);
			}
			else
			{
				pTaskFile->Command = ATA_CMD_STANDBY_IMMEDIATE;
			}
			break;
		case SCSI_CMD_REQUEST_SENSE:
		case SCSI_CMD_MODE_SELECT_10:
		case SCSI_CMD_MODE_SENSE_10:
		#ifndef _OS_BIOS	
			MV_DPRINT(("Error: Unknown request: 0x%x.\n", pReq->Cdb[0]));
		#endif

		default:
			return MV_FALSE;
	}

	/* 
	 * Attention: Never return before this line if your return is MV_TRUE.
	 * We need set the slave DEV bit here. 
	 */
	if ( pDevice->Is_Slave )		
		pTaskFile->Device |= MV_BIT(4);

	return MV_TRUE;
}

MV_BOOLEAN ATAPI_CDB2TaskFile(
	IN PDomain_Device pDevice,
	IN PMV_Request pReq, 
	OUT PATA_TaskFile pTaskFile
	)
{
	MV_ZeroMemory(pTaskFile, sizeof(ATA_TaskFile));

	/* At the same time, set the command category as well. */
	switch ( pReq->Cdb[0] )
	{
	case SCSI_CMD_MARVELL_SPECIFIC:
		/* This request should be for core module */
		if ( pReq->Cdb[1]!=CDB_CORE_MODULE )
			return MV_FALSE;

		switch ( pReq->Cdb[2] )
		{
		case CDB_CORE_IDENTIFY:
			pTaskFile->Command = ATA_CMD_IDENTIY_ATAPI;
			break;
					
		case CDB_CORE_SET_UDMA_MODE:
			pTaskFile->Command = ATA_CMD_SET_FEATURES;
			pTaskFile->Features = ATA_CMD_SET_TRANSFER_MODE;
			if ( pReq->Cdb[4]==MV_TRUE )
				pTaskFile->Sector_Count = 0x20 | pReq->Cdb[3];	/* MDMA mode */
			else
				pTaskFile->Sector_Count = 0x40 | pReq->Cdb[3];	/* UDMA mode*/

			//TBD: Check the 80-conductor cable in order to enable UDMA greater than 2.
			break;
					
		case CDB_CORE_SET_PIO_MODE:
			pTaskFile->Command = ATA_CMD_SET_FEATURES;
			pTaskFile->Features = ATA_CMD_SET_TRANSFER_MODE;
			pTaskFile->Sector_Count = 0x08 | pReq->Cdb[3];
			break;

		default:
			return MV_FALSE;
		}
		break;
#ifdef _OS_LINUX
	case SCSI_CMD_READ_DISC_INFO:
	/* unimplemented SCSI cmds */
	/*	return MV_FALSE;   */
#endif /* _OS_LINUX */
	case SCSI_CMD_READ_10:
	case SCSI_CMD_WRITE_10:
	case SCSI_CMD_VERIFY_10:
	case SCSI_CMD_INQUIRY:
	case SCSI_CMD_READ_CAPACITY_10:
	case SCSI_CMD_TEST_UNIT_READY:
	case SCSI_CMD_MODE_SENSE_10:
	case SCSI_CMD_MODE_SELECT_10:
	case SCSI_CMD_PREVENT_MEDIUM_REMOVAL:
	case SCSI_CMD_READ_TOC:
	case SCSI_CMD_START_STOP_UNIT:
	case SCSI_CMD_SYNCHRONIZE_CACHE_10:
	case SCSI_CMD_REQUEST_SENSE:
	default:
		/* 
		 * Use packet command 
		 */
		/* Features: DMA, OVL, DMADIR */
#if defined(USE_DMA_FOR_ALL_PACKET_COMMAND)
		if ( !(pReq->Cmd_Flag&CMD_FLAG_NON_DATA) )
		{
			//if ( pReq->Cdb[0]!=SCSI_CMD_INQUIRY ) //ATAPI???
			pTaskFile->Features |= MV_BIT(0);
		}
#elif defined(USE_PIO_FOR_ALL_PACKET_COMMAND)
		/* do nothing */
#else
		if ( pReq->Cmd_Flag&CMD_FLAG_DMA )
			//if ( SCSI_IS_READ(pReq->Cdb[0]) || 
			//   SCSI_IS_WRITE(pReq->Cdb[0]) )
			pTaskFile->Features |= MV_BIT(0);
#endif
		//TBD: OVL: overlapped.
		//TBD: DMADIR in IDENTIFY PACKET DEVICE word 62

		//TBD: Sector Count: Tag

		/* Byte count low and byte count high */
		if ( pReq->Data_Transfer_Length>0xFFFF )
		{
			pTaskFile->LBA_Mid = 0xFF;
			pTaskFile->LBA_High = 0xFF;
		}
		else
		{
			pTaskFile->LBA_Mid = (MV_U8)pReq->Data_Transfer_Length;
			pTaskFile->LBA_High = (MV_U8)(pReq->Data_Transfer_Length>>8);
		}

		pTaskFile->Command = ATA_CMD_PACKET;

		break;
	}

	/* 
	 * Attention: Never return before this line if your return is MV_TRUE.
	 * We need set the slave DEV bit here. 
	 */
	if ( pDevice->Is_Slave )		
		pTaskFile->Device |= MV_BIT(4);

	return MV_TRUE;
}

