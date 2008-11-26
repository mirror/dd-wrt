#include "mv_include.h"
#ifdef _OS_BIOS
#include "biosmain.h"
extern PMV_DATA_STRUCT BASEATTR MyDriverDataBaseOff;
#endif

#include "com_event_define.h"

#include "core_exp.h"
#include "core_inter.h"
#include "com_tag.h"

#include "core_sata.h"
#include "core_ata.h"

#include "core_init.h"

#if defined(_OS_LINUX) 
#include "hba_header.h" /* to be removed */
#include "hba_exp.h"
#endif /* _OS_LINUX */

#ifdef __AC_DBG__
#include "linux_helper.h"
#endif /* __AC_DBG__ */

#ifdef CORE_SUPPORT_API
#include "core_api.h"
#endif

#ifdef SOFTWARE_XOR
#include "core_xor.h"
#endif

#define FIS_REG_H2D_SIZE_IN_DWORD	5
#ifndef _OS_BIOS
/* For debug purpose only. */
PCore_Driver_Extension gCore = NULL;
#endif

extern MV_VOID SCSI_To_FIS(MV_PVOID pCore, PMV_Request pReq, MV_U8 tag, PATA_TaskFile pTaskFile);

extern MV_BOOLEAN Category_CDB_Type(
	IN PDomain_Device pDevice,
	IN PMV_Request pReq
	);

extern MV_BOOLEAN ATAPI_CDB2TaskFile(
	IN PDomain_Device pDevice,
	IN PMV_Request pReq, 
	OUT PATA_TaskFile pTaskFile
	);

extern MV_BOOLEAN ATA_CDB2TaskFile(
	IN PDomain_Device pDevice,
	IN PMV_Request pReq, 
	IN MV_U8 tag,	//TBD: Do we really need it?
	OUT PATA_TaskFile pTaskFile
	);

extern void Device_IssueReadLogExt(
	IN PDomain_Port pPort,
	IN PDomain_Device pDevice
	);

extern MV_BOOLEAN mvDeviceStateMachine(
	PCore_Driver_Extension pCore,
	PDomain_Device pDevice
	);

void CompleteRequest(
	IN PCore_Driver_Extension pCore,
	IN PMV_Request pReq,
	IN PATA_TaskFile taskFiles
	);

void CompleteRequestAndSlot(
	IN PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
	IN PMV_Request pReq,
	IN PATA_TaskFile taskFiles,
	IN MV_U8 slotId
	);

#if defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX)
void Core_ResetChannel(MV_PVOID Device);

static MV_VOID __core_req_timeout_handler(MV_PVOID data)
{
	PMV_Request req = (PMV_Request) data;
	PCore_Driver_Extension pcore;
	PDomain_Device dev;
	PHBA_Extension phba;

	if ( NULL == req )
		return;

	pcore = HBA_GetModuleExtension(req->Cmd_Initiator, MODULE_CORE);
	dev   = &pcore->Ports[PATA_MapPortId(req->Device_Id)].Device[PATA_MapDeviceId(req->Device_Id)];
	phba = HBA_GetModuleExtension(req->Cmd_Initiator, MODULE_HBA);
	
	hba_spin_lock_irq(&phba->lock);
	Core_ResetChannel((MV_PVOID) dev);
	hba_spin_unlock_irq(&phba->lock);
}
#endif /* SUPPORT_ERROR_HANDLING && _OS_LINUX */

#ifdef SUPPORT_SCSI_PASSTHROUGH
// Read TaskFile
void readTaskFiles(IN PDomain_Port pPort, PDomain_Device pDevice, PATA_TaskFile pTaskFiles)
{
	MV_U32 taskFile[3];

	if (pPort->Type==PORT_TYPE_PATA)
	{
		if ( pDevice->Is_Slave )
		{
			taskFile[1] = MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_SLAVE_TF1);
			taskFile[2] = MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_SLAVE_TF2);
		}
		else
		{
			taskFile[1] = MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_MASTER_TF1);
			taskFile[2] = MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_MASTER_TF2);
		}

		pTaskFiles->Sector_Count = (MV_U8)((taskFile[1] >> 24) & 0xFF);
		pTaskFiles->Sector_Count_Exp = (MV_U8)((taskFile[1] >> 16) & 0xFF);
		pTaskFiles->LBA_Low = (MV_U8)((taskFile[1] >> 8) & 0xFF);
		pTaskFiles->LBA_Low_Exp = (MV_U8)(taskFile[1] & 0xFF);

		pTaskFiles->LBA_Mid = (MV_U8)((taskFile[2] >> 24) & 0xFF);
		pTaskFiles->LBA_Mid_Exp = (MV_U8)((taskFile[2] >> 16) & 0xFF);
		pTaskFiles->LBA_High = (MV_U8)((taskFile[2] >> 8) & 0xFF);
		pTaskFiles->LBA_High_Exp = (MV_U8)(taskFile[2] & 0xFF);
	}
	else
	{
//		taskFile[0] = MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_TFDATA);
		taskFile[1] = MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_SIG);
//		taskFile[2] = MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_SCR);

		pTaskFiles->Sector_Count = (MV_U8)((taskFile[1]) & 0xFF);
		pTaskFiles->LBA_Low = (MV_U8)((taskFile[1] >> 8) & 0xFF);
		pTaskFiles->LBA_Mid = (MV_U8)((taskFile[1] >> 16) & 0xFF);
		pTaskFiles->LBA_High = (MV_U8)((taskFile[1] >> 24) & 0xFF);

	}
}
#endif

//TBD: In Dump. How many request can be? Not always 1.
MV_U32 Core_ModuleGetResourceQuota(enum Resource_Type type, MV_U16 maxIo)
{
	MV_U32 size = 0;
	MV_U8 sgEntryCount;
	
	/* Extension quota */
	if ( type==RESOURCE_CACHED_MEMORY )		
	{
		size = ROUNDING(sizeof(Core_Driver_Extension), 8);
	#ifdef SUPPORT_CONSOLIDATE
		if ( maxIo>1 )
		{
			size += ROUNDING(sizeof(Consolidate_Extension), 8);
			size += ROUNDING(sizeof(Consolidate_Device), 8)*MAX_DEVICE_NUMBER;
		}
	#endif

		/* resource for SG Entry */
		if (maxIo==1)
			sgEntryCount = MAX_SG_ENTRY_REDUCED;
		else
			sgEntryCount = MAX_SG_ENTRY;
		size += sizeof(MV_SG_Entry) * sgEntryCount * INTERNAL_REQ_COUNT;

		size += sizeof(MV_Request) * INTERNAL_REQ_COUNT;

#ifdef SUPPORT_CONSOLIDATE
		/* resource for Consolidate_Extension->Requests[] SG Entry */
		if ( maxIo>1 )
			size += sizeof(MV_SG_Entry) * sgEntryCount * CONS_MAX_INTERNAL_REQUEST_COUNT;
#endif

		MV_DUMPC32(0xCCCC8801);
		MV_DUMPC32(size);
		//MV_HALTKEY;
        return size;
	}
	
	/* Uncached memory quota */
	if ( type==RESOURCE_UNCACHED_MEMORY )
	{
		/* 
		 * SATA port alignment quota:
		 * Command list and received FIS is 64 byte aligned.
		 * Command table is 128 byte aligned.
		 * Data buffer is 8 byte aligned.
		 * This is different with AHCI.
		 */
        /* 
		 * PATA port alignment quota: Same with SATA.
		 * The only difference is that PATA doesn't have the FIS.
		 */
	#ifndef _OS_BIOS
		MV_DPRINT(("Command List Size = 0x%x.\n", (MV_U32)SATA_CMD_LIST_SIZE));
		MV_DPRINT(("Received FIS Size = 0x%x.\n", (MV_U32)SATA_RX_FIS_SIZE));
		MV_DPRINT(("Command Table Size = 0x%x.\n", (MV_U32)SATA_CMD_TABLE_SIZE));
		MV_ASSERT(SATA_CMD_LIST_SIZE==ROUNDING(SATA_CMD_LIST_SIZE, 64));
		MV_ASSERT(SATA_RX_FIS_SIZE==ROUNDING(SATA_RX_FIS_SIZE, 64));
		MV_ASSERT(SATA_CMD_TABLE_SIZE==ROUNDING(SATA_CMD_TABLE_SIZE, 128));
		MV_ASSERT(SATA_SCRATCH_BUFFER_SIZE==ROUNDING(SATA_SCRATCH_BUFFER_SIZE, 8));
	#endif
		if ( maxIo>1 )
		{
			size = 64 + SATA_CMD_LIST_SIZE*MAX_PORT_NUMBER;								/* Command List*/
			size += 64 + SATA_RX_FIS_SIZE*MAX_SATA_PORT_NUMBER;							/* Received FIS */
			size += 128 + SATA_CMD_TABLE_SIZE*MAX_SLOT_NUMBER*MAX_PORT_NUMBER;			/* Command Table */
			size += 8 + SATA_SCRATCH_BUFFER_SIZE*MAX_DEVICE_NUMBER;						/* Buffer for initialization like identify */
		}
		else
		{
		#ifndef HIBERNATION_ROUNTINE
			size = 64 + SATA_CMD_LIST_SIZE*MAX_PORT_NUMBER;
			size += 64 + SATA_RX_FIS_SIZE*MAX_SATA_PORT_NUMBER;
			size += 128 + SATA_CMD_TABLE_SIZE*MAX_PORT_NUMBER;
			size += 8 + SATA_SCRATCH_BUFFER_SIZE*MAX_DEVICE_NUMBER;
		#else
			size = 64 + SATA_CMD_LIST_SIZE;			/* Command List*/
			size += 64 + SATA_RX_FIS_SIZE;			/* Received FIS */
			size += 128 + SATA_CMD_TABLE_SIZE; 		/* Command Table */	
			size += 8 + SATA_SCRATCH_BUFFER_SIZE;	/* Buffer for initialization like identify */
		#endif
		}

		//MV_DUMPC32(0xCC000002);
		//MV_DUMPC32(size);
		
		MV_DUMPC32(0xCCCC8802);
		MV_DUMPC32(size);
		//MV_HALTKEY;
		return size;
	}

	return 0;
}

//TBD: In Dump
void Core_ModuleInitialize(MV_PVOID This, MV_U32 extensionSize, MV_U16 maxIo)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;
	PMV_Request pReq;
	Assigned_Uncached_Memory dmaResource;
	PDomain_Port port;
	MV_PVOID memVir;
	MV_PHYSICAL_ADDR memDMA;
	Controller_Infor controller;
	MV_PTR_INTEGER temp, tmpSG;
	MV_U32 offset, internalReqSize;
	MV_U8 i,j, flagSaved, sgEntryCount;
	MV_U32 vsr_c[MAX_SATA_PORT_NUMBER];
	MV_U8 vsrSkipPATAPort = 0;
	MV_PVOID pTopLayer = HBA_GetModuleExtension(pCore, MODULE_HBA);
	
#ifndef _OS_BIOS
	gCore = pCore;
#endif
	MV_DUMPC32(0xCCCCBB12);

	flagSaved=pCore->VS_Reg_Saved;

	if(flagSaved==VS_REG_SIG)
	{
		for ( j=0; j<MAX_SATA_PORT_NUMBER; j++ )
		{
			port = &pCore->Ports[j];
			vsr_c[j]=port->VS_RegC;
		}
		/* Save the PATA Port detection skip flag */
		vsrSkipPATAPort = pCore->Flag_Fastboot_Skip & FLAG_SKIP_PATA_PORT;
	}

	/* 
	 * Zero core driver extension. After that, I'll ignore many variables initialization. 
	 */
	MV_ZeroMemory(This, extensionSize);

	if(flagSaved==VS_REG_SIG)
	{
		pCore->VS_Reg_Saved=flagSaved;

		for ( j=0; j<MAX_SATA_PORT_NUMBER; j++ )
		{
			port = &pCore->Ports[j];
			port->VS_RegC=vsr_c[j];
		}
		/* Restore the PATA Port detection skip flag */
		/* Only this flag should survive the S3 */
		/* The others should be kept as default (0) */
		pCore->Flag_Fastboot_Skip = vsrSkipPATAPort;
	}

	pCore->State = CORE_STATE_IDLE;

	/* Set up controller information */
	HBA_GetControllerInfor(pCore, &controller);
	pCore->Vendor_Id = controller.Vendor_Id;
	pCore->Device_Id = controller.Device_Id;
	pCore->Revision_Id = controller.Revision_Id;
	for ( i=0; i<MAX_BASE_ADDRESS; i++ )
	{
		pCore->Base_Address[i] = controller.Base_Address[i];
	}
	pCore->Mmio_Base = controller.Base_Address[MV_PCI_BAR];

	pCore->Adapter_State = ADAPTER_INITIALIZING;
	MV_LIST_HEAD_INIT(&pCore->Waiting_List);
	MV_LIST_HEAD_INIT(&pCore->Internal_Req_List);

	if ( maxIo==1 )
		pCore->Is_Dump = MV_TRUE;
	else
		pCore->Is_Dump = MV_FALSE;

	if (flagSaved!=VS_REG_SIG) {	/* Added for tuning boot up time */
		/* This initialization is during boot up time, but not S3 */
		/* Read registers modified by BIOS to set detection flag */
		if ( (pCore->Device_Id==DEVICE_ID_THOR_4S1P_NEW) ||
			 (pCore->Device_Id==DEVICE_ID_THORLITE_2S1P_WITH_FLASH) ||
			 (pCore->Device_Id==DEVICE_ID_THORLITE_2S1P)) {
			MV_U32 tmpReg = 0;
			/* Read Bit[3] of PCI CNFG offset 60h to get flag for */
			/* PATA port enable/disable (0 - default, need to detect) */
#ifdef _OS_WINDOWS
#undef MV_PCI_READ_CONFIG_DWORD
#define MV_PCI_READ_CONFIG_DWORD(mod_ext, offset, reg) \
                reg = MV_PCI_READ_DWORD(mod_ext, offset)
#endif /* _OS_WINDOWS */
			MV_PCI_READ_CONFIG_DWORD(pTopLayer, 0x60, tmpReg);
			tmpReg &= MV_BIT(3);

			pCore->Flag_Fastboot_Skip |= (tmpReg >> 3);		/* bit 0 */
			/* Read Bit[10], Bit [11] of BAR5 offset A4h to get flag for */
			/* PATA device detection (0 - default, need to detect) and */
			/* PM detection (0 - default, need to detect) */
			tmpReg = MV_REG_READ_DWORD(pCore->Mmio_Base, VENDOR_DETECT) & 
						(VENDOR_DETECT_PATA | VENDOR_DETECT_PM);
			pCore->Flag_Fastboot_Skip |= (tmpReg >> 9);		/* bit 1, 2 */
		}
	}

	if ( (pCore->Device_Id==DEVICE_ID_THORLITE_2S1P)||(pCore->Device_Id==DEVICE_ID_THORLITE_2S1P_WITH_FLASH) )
	{
		pCore->SATA_Port_Num = 2;
		pCore->PATA_Port_Num = 1;
		pCore->Port_Num = 3;
#ifndef _OS_BIOS
		MV_DPRINT(("DEVICE_ID_THORLITE_2S1P is found.\n"));
#endif

	}
	else if ( pCore->Device_Id==DEVICE_ID_THORLITE_0S1P )
	{
		pCore->SATA_Port_Num = 0;
		pCore->PATA_Port_Num = 1;
		pCore->Port_Num = 1;
#ifndef _OS_BIOS
		MV_DPRINT(("DEVICE_ID_THORLITE_0S1P is found.\n"));
#endif

	}
	else
	{
		pCore->SATA_Port_Num = 4;
		pCore->PATA_Port_Num = 1;
		pCore->Port_Num = 5;
#ifndef _OS_BIOS
		MV_DPRINT(("DEVICE_ID_THOR is found.\n"));
#endif

	}

#if /*(VER_OEM==VER_OEM_ASUS) ||*/(VER_OEM==VER_OEM_INTEL)
	pCore->Port_Num -= pCore->PATA_Port_Num;
	pCore->PATA_Port_Num = 0;
#else
	if (pCore->Flag_Fastboot_Skip & FLAG_SKIP_PATA_PORT) {
		pCore->Port_Num -= pCore->PATA_Port_Num;
		pCore->PATA_Port_Num = 0;
	}
#endif

	if (pCore->Is_Dump)
		sgEntryCount = MAX_SG_ENTRY_REDUCED;
	else
		sgEntryCount = MAX_SG_ENTRY;

	tmpSG = (MV_PTR_INTEGER)This + ROUNDING(sizeof(Core_Driver_Extension),8);
	temp = 	tmpSG + sizeof(MV_SG_Entry) * sgEntryCount * INTERNAL_REQ_COUNT;

	internalReqSize = MV_REQUEST_SIZE * INTERNAL_REQ_COUNT;
	MV_ASSERT( extensionSize >= ROUNDING(sizeof(Core_Driver_Extension),8) + internalReqSize );
	for ( i=0; i<INTERNAL_REQ_COUNT; i++ )
	{
		pReq = (PMV_Request)temp;
		pReq->SG_Table.Entry_Ptr = (PMV_SG_Entry)tmpSG;
		pReq->SG_Table.Max_Entry_Count = sgEntryCount;
		List_AddTail(&pReq->Queue_Pointer, &pCore->Internal_Req_List);
		tmpSG += sizeof(MV_SG_Entry) * sgEntryCount;
		temp += MV_REQUEST_SIZE;	/* MV_Request is 64bit aligned. */
	}	
//	temp = ROUNDING( (MV_PTR_INTEGER)temp, 8 );		/* Don't round the extension pointer */

#ifdef SUPPORT_CONSOLIDATE	
	// Allocate resource for Consolidate_Extension->Requests[].
	tmpSG = temp;
	temp = temp + sizeof(MV_SG_Entry) * sgEntryCount * CONS_MAX_INTERNAL_REQUEST_COUNT;

	if ( pCore->Is_Dump )
	{
		pCore->pConsolid_Device = NULL;
		pCore->pConsolid_Extent = NULL;
	}
	else
	{
		MV_ASSERT( extensionSize>=
			( ROUNDING(sizeof(Core_Driver_Extension),8) + internalReqSize + ROUNDING(sizeof(Consolidate_Extension),8) + ROUNDING(sizeof(Consolidate_Device),8)*MAX_DEVICE_NUMBER )
			); 
		pCore->pConsolid_Extent = (PConsolidate_Extension)(temp);

		//Initialize some fields for pCore->pConsolid_Extent->Requests[i]
		for (i=0; i<CONS_MAX_INTERNAL_REQUEST_COUNT; i++)
		{
			pReq = &pCore->pConsolid_Extent->Requests[i];

			pReq->SG_Table.Max_Entry_Count = sgEntryCount;
			pReq->SG_Table.Entry_Ptr = (PMV_SG_Entry)tmpSG;
			tmpSG += sizeof(MV_SG_Entry) * sgEntryCount;
		}

		pCore->pConsolid_Device = (PConsolidate_Device)((MV_PTR_INTEGER)pCore->pConsolid_Extent + ROUNDING(sizeof(Consolidate_Extension),8));
	}
#endif

	/* Port_Map and Port_Num will be read from the register */

	/* Init port data structure */
	for ( i=0; i<pCore->Port_Num; i++ )
	{
		port = &pCore->Ports[i];
		
		port->Id = i;
		port->Port_State = PORT_STATE_IDLE;

		port->Core_Extension = pCore;
#ifdef _OS_BIOS
/* BIOS use far pointer to access MMIO_BASE */
		port->Mmio_Base = (MV_U32)pCore->Mmio_Base + 0x100 + (i * 0x80);
		//port->Mmio_SCR = (MV_U32)port->Mmio_Base + PORT_SCR;
#else
		port->Mmio_Base = (MV_PU8)pCore->Mmio_Base + 0x100 + (i * 0x80);
		port->Mmio_SCR = (MV_PU8)port->Mmio_Base + PORT_SCR;
#endif

		Tag_Init(&port->Tag_Pool, MAX_TAG_NUMBER);

		for (j=0; j<MAX_DEVICE_PER_PORT; j++) 
		{
			port->Device[j].Id = i*MAX_DEVICE_PER_PORT + j;
			port->Device[j].PPort = port;
			port->Device[j].Is_Slave = 0;	/* Which one is the slave will be determined during discovery. */
#if defined(SUPPORT_TIMER) && defined(_OS_WINDOWS)
			port->Device[j].Timer_ID = NO_CURRENT_TIMER;
#endif
			port->Device[j].Reset_Count = 0;
		}

		port->Device_Number = 0;

		//TBD: Set function table for each port here.
		if ( i>=pCore->SATA_Port_Num )
			port->Type = PORT_TYPE_PATA;
		else
			port->Type = PORT_TYPE_SATA;
	}

	/* Get uncached memory */
	HBA_GetResource(pCore, RESOURCE_UNCACHED_MEMORY, &dmaResource);
	memVir = dmaResource.Virtual_Address;
	memDMA = dmaResource.Physical_Address;
	
	/* Assign uncached memory for command list (64 byte align) */
	offset = (MV_U32)(ROUNDING(memDMA.value,64)-memDMA.value);
	memDMA.value += offset;
	memVir = (MV_PU8)memVir + offset;
	for ( i=0; i<pCore->Port_Num; i++ )
	{
		port = &pCore->Ports[i];
		port->Cmd_List = memVir;
		port->Cmd_List_DMA = memDMA;
	#ifdef HIBERNATION_ROUNTINE
		if((!pCore->Is_Dump)|| (i==(pCore->Port_Num-1)))
	#endif
		{
			memVir = (MV_PU8)memVir + SATA_CMD_LIST_SIZE;
			memDMA.value += SATA_CMD_LIST_SIZE;
		}
	}

	/* Assign uncached memory for received FIS (64 byte align) */
	offset = (MV_U32)(ROUNDING(memDMA.value,64)-memDMA.value);
	memDMA.value += offset;
	memVir = (MV_PU8)memVir + offset;
	for ( i=0; i<pCore->SATA_Port_Num; i++ )
	{
		port = &pCore->Ports[i];	
		port->RX_FIS = memVir;
		port->RX_FIS_DMA = memDMA;
	#ifdef HIBERNATION_ROUNTINE
		if((!pCore->Is_Dump)|| (i==(pCore->SATA_Port_Num-1)))
	#endif
		{
			memVir = (MV_PU8)memVir + SATA_RX_FIS_SIZE;
			memDMA.value += SATA_RX_FIS_SIZE;
		}
	}

	/* Assign the 32 command tables. (128 byte align) */
	offset = (MV_U32)(ROUNDING(memDMA.value,128)-memDMA.value);
	memDMA.value += offset;
	memVir = (MV_PU8)memVir + offset;
	for ( i=0; i<pCore->Port_Num; i++ )
	{
		port = &pCore->Ports[i];
		port->Cmd_Table = memVir;
		port->Cmd_Table_DMA = memDMA;

		if ( !pCore->Is_Dump )
		{
			memVir = (MV_PU8)memVir + SATA_CMD_TABLE_SIZE * MAX_SLOT_NUMBER;
			memDMA.value += SATA_CMD_TABLE_SIZE * MAX_SLOT_NUMBER;
		}
		else
		{
		#ifdef HIBERNATION_ROUNTINE
			if(i==(pCore->Port_Num-1))
		#endif
			{
				memVir = (MV_PU8)memVir + SATA_CMD_TABLE_SIZE;
				memDMA.value += SATA_CMD_TABLE_SIZE;
			}
		}
	}

	/* Assign the scratch buffer (8 byte align) */
	offset = (MV_U32)(ROUNDING(memDMA.value,8)-memDMA.value);
	memDMA.value += offset;
	memVir = (MV_PU8)memVir + offset;
	for ( i=0; i<pCore->Port_Num; i++ )
	{
		port = &pCore->Ports[i];
		for ( j=0; j<MAX_DEVICE_PER_PORT; j++ )
		{
			port->Device[j].Scratch_Buffer = memVir;
			port->Device[j].Scratch_Buffer_DMA = memDMA;
		
		#ifdef HIBERNATION_ROUNTINE
			if((!pCore->Is_Dump)|| (i==(pCore->Port_Num-1)))
		#endif
			{
				memVir = (MV_PU8)memVir + SATA_SCRATCH_BUFFER_SIZE;
				memDMA.value += SATA_SCRATCH_BUFFER_SIZE;
			}
		}
	}

	/* Let me confirm the following assumption */
	MV_ASSERT( sizeof(SATA_FIS_REG_H2D)==sizeof(MV_U32)*FIS_REG_H2D_SIZE_IN_DWORD );
	MV_ASSERT( sizeof(MV_Command_Table)==0x80+MAX_SG_ENTRY*sizeof(MV_SG_Entry) );
	MV_ASSERT( sizeof(ATA_Identify_Data)==512 ); 
	MV_ASSERT( MAX_TAG_NUMBER==MAX_SLOT_NUMBER );

#ifdef SUPPORT_CONSOLIDATE
	if ( !pCore->Is_Dump )
	{
		Consolid_InitializeExtension(This);
		for ( i=0; i<MAX_DEVICE_NUMBER; i++ )
			Consolid_InitializeDevice(This, i);
	}
#endif
}

void Core_ModuleStart(MV_PVOID This)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;

	mvAdapterStateMachine(pCore);
}

#ifdef _OS_BIOS
void Core_ReInitBaseAddress(MV_PVOID This)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;
 	PDomain_Port pPort;
 	MV_U8 i;
  	PMV_DATA_STRUCT pDriverData = MyDriverDataBaseOff;
	for ( i = 0; i<pCore->Port_Num; i++) {
		pPort = &pCore->Ports[i];
		pPort->Cmd_List_DMA.low = MVVirtual2PhyicalAddress((MV_LPVOID)pPort->Cmd_List);
		pPort->RX_FIS_DMA.low = MVVirtual2PhyicalAddress((MV_LPVOID)pPort->RX_FIS);
		pPort->Cmd_Table_DMA.low = MVVirtual2PhyicalAddress((MV_LPVOID)pPort->Cmd_Table);

		/* Set the sata port register */
		MV_REG_WRITE_DWORD(pPort->Mmio_Base, PORT_LST_ADDR_HI, pPort->Cmd_List_DMA.high);
		MV_REG_WRITE_DWORD(pPort->Mmio_Base, PORT_LST_ADDR, pPort->Cmd_List_DMA.low);
		MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_LST_ADDR);

		MV_REG_WRITE_DWORD(pPort->Mmio_Base, PORT_FIS_ADDR_HI, pPort->RX_FIS_DMA.high);
		MV_REG_WRITE_DWORD(pPort->Mmio_Base, PORT_FIS_ADDR, pPort->RX_FIS_DMA.low);
		MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_FIS_ADDR);

	}
}
#endif

void Core_ModuleShutdown(MV_PVOID This)
{
#ifndef _OS_BIOS
	/* 
	 * This function is equivalent to ahci_port_stop 
	 */
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;
	MV_U32 tmp, i;
	MV_LPVOID mmio;
	for ( i=0; i<pCore->Port_Num; i++ )
	{
		mmio = pCore->Ports[i].Mmio_Base;

		tmp = MV_REG_READ_DWORD(mmio, PORT_CMD);
		if ( pCore->Ports[i].Type==PORT_TYPE_SATA )
			tmp &= ~(PORT_CMD_START | PORT_CMD_FIS_RX);
		else
			tmp &= ~PORT_CMD_START;
		MV_REG_WRITE_DWORD(mmio, PORT_CMD, tmp);
		MV_REG_READ_DWORD(mmio, PORT_CMD); /* flush */

		/* 
		 * spec says 500 msecs for each PORT_CMD_{START,FIS_RX} bit, so
		 * this is slightly incorrect.
		 */
		HBA_SleepMillisecond(pCore, 500);
	}

	/* Disable the controller interrupt */
	tmp = MV_REG_READ_DWORD(pCore->Mmio_Base, HOST_CTL);
	tmp &= ~(HOST_IRQ_EN);
	MV_REG_WRITE_DWORD(pCore->Mmio_Base, HOST_CTL, tmp);
#endif
}

void Core_ModuleNotification(MV_PVOID This, enum Module_Event event, MV_PVOID event_param)
{
}

void Core_HandleWaitingList(PCore_Driver_Extension pCore);
void Core_InternalSendRequest(MV_PVOID This, PMV_Request pReq);

void Core_ModuleSendRequest(MV_PVOID This, PMV_Request pReq)
{	
#ifdef SUPPORT_CONSOLIDATE
	{
		PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;
		PDomain_Device pDevice;
		MV_U8 portId = PATA_MapPortId(pReq->Device_Id);
		MV_U8 deviceId = PATA_MapDeviceId(pReq->Device_Id);
		
		pDevice = &pCore->Ports[portId].Device[deviceId];
		if ( (!(pDevice->Device_Type&DEVICE_TYPE_ATAPI)) && (!pCore->Is_Dump) )
			Consolid_ModuleSendRequest(pCore, pReq);
		else
			Core_InternalSendRequest(pCore, pReq);
	}
#else
	Core_InternalSendRequest(This, pReq);
#endif
}

void Core_InternalSendRequest(MV_PVOID This, PMV_Request pReq)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;
	//MV_DUMPRUN(0xCCF1);
	/* Check whether we can handle this request */
	switch (pReq->Cdb[0])
	{
		case SCSI_CMD_INQUIRY:
		case SCSI_CMD_START_STOP_UNIT:
		case SCSI_CMD_TEST_UNIT_READY:
		case SCSI_CMD_READ_10:
		case SCSI_CMD_WRITE_10:
		case SCSI_CMD_VERIFY_10:
		case SCSI_CMD_READ_CAPACITY_10:
		case SCSI_CMD_REQUEST_SENSE:
		case SCSI_CMD_MODE_SELECT_10:
		case SCSI_CMD_MODE_SENSE_10:
		case SCSI_CMD_MARVELL_SPECIFIC:
		default:
			if ( pReq->Cmd_Initiator==pCore )
			{
				if ( !SCSI_IS_READ(pReq->Cdb[0]) && !SCSI_IS_WRITE(pReq->Cdb[0]) )
				{
					/* Reset request or request sense command. */
					List_Add(&pReq->Queue_Pointer, &pCore->Waiting_List);		/* Add to the header. */
				}
				else
				{
					#ifdef SUPPORT_CONSOLIDATE
					/* Consolidate request */
					MV_DASSERT( !pCore->Is_Dump );
					List_AddTail(&pReq->Queue_Pointer, &pCore->Waiting_List);	/* Append to the tail. */
					#else
					MV_ASSERT(MV_FALSE);
					#endif
				}
			}
			else
			{
				List_AddTail(&pReq->Queue_Pointer, &pCore->Waiting_List);		/* Append to the tail. */
			}
			Core_HandleWaitingList(pCore);
			break;
	}
}

void SATA_PrepareCommandHeader(PDomain_Port pPort, PMV_Request pReq, MV_U8 tag)
{
	MV_PHYSICAL_ADDR table_addr;
	PMV_Command_Header header = NULL;
	PMV_SG_Table pSGTable = &pReq->SG_Table;
	PDomain_Device pDevice = &pPort->Device[PATA_MapDeviceId(pReq->Device_Id)];
#ifdef DEBUG_BIOS
	MV_PU8	pHead;
	MV_U16 tmp;
#endif

	header = SATA_GetCommandHeader(pPort, tag);
	/* 
	 * Set up the command header.
	 * TBD: Table_Address and Table_Address_High are fixed. Needn't set every time.
	 */
	header->FIS_Length = FIS_REG_H2D_SIZE_IN_DWORD;
	header->Packet_Command = (pReq->Cmd_Flag&CMD_FLAG_PACKET)?1:0;
	header->Reset = 0;
	header->NCQ = (pReq->Cmd_Flag&CMD_FLAG_NCQ)?1:0;

#ifdef SUPPORT_PM
	header->PM_Port = pDevice->PM_Number;
#else
	header->PM_Port = 0;
#endif
	*((MV_U16 *) header) = CPU_TO_LE_16( *((MV_U16 *) header) );
	header->PRD_Entry_Count = CPU_TO_LE_16(pSGTable->Valid_Entry_Count);

	table_addr.low = pPort->Cmd_Table_DMA.low + SATA_CMD_TABLE_SIZE*tag;
	MV_ASSERT(table_addr.low>=pPort->Cmd_Table_DMA.low);	//TBD
	table_addr.high = pPort->Cmd_Table_DMA.high;

	header->Table_Address = CPU_TO_LE_32(table_addr.low);
	header->Table_Address_High = CPU_TO_LE_32(table_addr.high);
#ifdef DEBUG_BIOS
	pHead=(MV_PU8)header;
	MV_DUMPC32(pReq->Cmd_Flag);
	MV_DUMPC32(0xCCCCDDD2);
	MV_DUMPC32(pPort->Cmd_List_DMA.low);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pHead[0]));
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pHead[1]));
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pHead[2]));
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pHead[3]));
#endif
}

void PATA_PrepareCommandHeader(PDomain_Port pPort, PMV_Request pReq, MV_U8 tag)
{
#ifdef DEBUG_BIOS
	MV_PU8	pHead;
	MV_U16 tmp;
#endif
	MV_PHYSICAL_ADDR table_addr;
	PMV_PATA_Command_Header header = NULL;
	PMV_SG_Table pSGTable = &pReq->SG_Table;

	header = PATA_GetCommandHeader(pPort, tag);
	/* 
	 * Set up the command header.
	 * TBD: TCQ, Diagnostic_Command, Reset
	 * TBD: Table_Address and Table_Address_High are fixed. Needn't set every time.
	 */
	header->PIO_Sector_Count = 0;		/* Only for PIO multiple sector commands */
	header->Controller_Command = 0;
	header->TCQ = 0;
	header->Packet_Command = (pReq->Cmd_Flag&CMD_FLAG_PACKET)?1:0;

#ifdef USE_DMA_FOR_ALL_PACKET_COMMAND
	if ( pReq->Cmd_Flag&CMD_FLAG_PACKET )
	{
		//if ( pReq->Cdb[0]!=SCSI_CMD_INQUIRY )	//ATAPI???
			header->DMA = (pReq->Cmd_Flag&CMD_FLAG_NON_DATA)?0:1;
		//else
		//	header->DMA = 0;
	}
	else
	{
		header->DMA = (pReq->Cmd_Flag&CMD_FLAG_DMA)?1:0;
	}
#elif defined(USE_PIO_FOR_ALL_PACKET_COMMAND)
	if ( pReq->Cmd_Flag&CMD_FLAG_PACKET )
	{
		header->DMA = 0;
	}
	else
	{
		header->DMA = (pReq->Cmd_Flag&CMD_FLAG_DMA)?1:0;
	}
#else	
	header->DMA = (pReq->Cmd_Flag&CMD_FLAG_DMA)?1:0;
#endif

	header->Data_In = (pReq->Cmd_Flag&CMD_FLAG_DATA_IN)?1:0;
	header->Non_Data = (pReq->Cmd_Flag&CMD_FLAG_NON_DATA)?1:0;

	header->PIO_Sector_Command = 0;
	header->Is_48Bit = (pReq->Cmd_Flag&CMD_FLAG_48BIT)?1:0;
	header->Diagnostic_Command = 0;
	header->Reset = 0;

	header->Is_Slave = pPort->Device[PATA_MapDeviceId(pReq->Device_Id)].Is_Slave;

	*((MV_U16 *) header) = CPU_TO_LE_16( *((MV_U16 *) header) );
	header->PRD_Entry_Count = CPU_TO_LE_16(pSGTable->Valid_Entry_Count);

	table_addr.low = pPort->Cmd_Table_DMA.low + SATA_CMD_TABLE_SIZE*tag;
	MV_ASSERT( table_addr.low>=pPort->Cmd_Table_DMA.low);	//TBD
	table_addr.high = pPort->Cmd_Table_DMA.high;

	header->Table_Address = CPU_TO_LE_32(table_addr.low);
	header->Table_Address_High = CPU_TO_LE_32(table_addr.high);

#ifdef DEBUG_BIOS
	pHead=(MV_PU8)header;
	MV_DUMPC32(pReq->Cmd_Flag);
	MV_DUMPC32(0xCCCC7701);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pHead[0]));
	MV_DUMPC32(0xCCCC7702);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pHead[4]));
	MV_DUMPC32(0xCCCC7703);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pHead[8]));
	MV_DUMPC32(0xCCCC7704);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pHead[12]));
#endif


}

/*
 * Fill SATA command table
 */
MV_VOID SATA_PrepareCommandTable(
	PDomain_Port pPort, 
	PMV_Request pReq, 
	MV_U8 tag,
	PATA_TaskFile pTaskFile
	)
{
	PMV_Command_Table pCmdTable = Port_GetCommandTable(pPort, tag);

	PMV_SG_Table pSGTable = &pReq->SG_Table;
	PMV_SG_Entry pSGEntry = NULL;
	MV_U8 i;
#ifdef DEBUG_BIOS
	MV_PU8 pTable;
#endif

	/* Step 1: fill the command FIS: MV_Command_Table */
	SCSI_To_FIS(pPort->Core_Extension, pReq, tag, pTaskFile);

	/* Step 2. fill the ATAPI CDB */
	if ( pReq->Cmd_Flag&CMD_FLAG_PACKET )
	{
		MV_CopyMemory(pCmdTable->ATAPI_CDB, pReq->Cdb, MAX_CDB_SIZE);
	}

	/* Step 3: fill the PRD Table if necessary. */
	if ( (pSGTable) && (pSGTable->Valid_Entry_Count) )
	{
		/* "Transfer Byte Count" in AHCI and 614x PRD table is zero based. */
		for ( i=0; i<pSGTable->Valid_Entry_Count; i++ )
		{
			pSGEntry = &pCmdTable->PRD_Entry[i];
			pSGEntry->Base_Address = CPU_TO_LE_32(pSGTable->Entry_Ptr[i].Base_Address);
			pSGEntry->Base_Address_High = CPU_TO_LE_32(pSGTable->Entry_Ptr[i].Base_Address_High);
			pSGEntry->Size = CPU_TO_LE_32(pSGTable->Entry_Ptr[i].Size-1);
		}
	}
	else
	{	
		MV_DASSERT( !SCSI_IS_READ(pReq->Cdb[0]) && !SCSI_IS_WRITE(pReq->Cdb[0]) );
	}
#ifdef DEBUG_BIOS	

	pTable=(MV_PU8)pPort->Cmd_Table;
	//MV_DUMPC32(0xCCCCEEE2);
	//MV_DUMPC16((MV_U16)pTable);
	//MV_DUMPC32(pPort->Cmd_Table_DMA.low);
	MV_DUMPC32(0xCCCCEE21);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->FIS[0]));
	MV_DUMPC32(0xCCCCEE22);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->FIS[4]));
	MV_DUMPC32(0xCCCCEE23);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->FIS[8]));
	MV_DUMPC32(0xCCCCEE24);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->FIS[12]));
#if 1
	MV_DUMPC32(0xCCCCEEE3);
	//MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[0]));
	//MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[4]));
	//MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[8]));
	//MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[12]));
	//MV_DUMPC32(0xCCCCEEE4);
	//(MV_U32)(*(MV_PU32)&pTable[136]) =(MV_U32) 0;
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pTable[128]));
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pTable[132]));
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pTable[136]));
	MV_DUMPC32(pCmdTable->PRD_Entry[0].Size);

	MV_DUMPC32(0xCCCCEEE5);

	#ifdef DEBUG_BIOS
		if((MV_U32)(*(MV_PU32)&pTable[132]) != 0)
		{
			MV_DUMPC32(0xCCCC990D);
			MV_HALTKEY;
		}
	#endif
	
	//pSGEntry = &pCmdTable->PRD_Entry[0];
	//MV_DUMPC32(pSGEntry->Base_Address);
	//MV_DUMPC32(pSGEntry->Base_Address_High);
	//MV_DUMPC32(pSGTable->Entry[0].Base_Address);
	//MV_DUMPC32(pSGTable->Entry[0].Base_Address_High);
	//MV_DUMPC8(pSGTable->Valid_Entry_Count);
	//MV_DPRINT("CmdTableAddr=0x%x,CmdTableAddrP=0x%x\n",(MV_U32)(MV_U16)pPort->Cmd_Table,(MV_U32)pPort->Cmd_Table_DMA.low);
	//MV_DPRINT("CmdBlk0=0x%x,CmdBlk1=0x%x,CmdBlk2=0x%x,CmdBlk3=0x%x,"
	//			,(MV_U32)(*(MV_PU32)&pCmdTable->FIS[0])
	//			,(MV_U32)(*(MV_PU32)&pCmdTable->FIS[4])
	//			,(MV_U32)(*(MV_PU32)&pCmdTable->FIS[8])
	//			,(MV_U32)(*(MV_PU32)&pCmdTable->FIS[12]));


	//MV_DPRINT("ATAPI0=0x%x,ATAPI1=0x%x,ATAPI2=0x%x,ATAPI3=0x%x\n"
	//			,(MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[0])
	//			,(MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[4])
	//			,(MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[8])
	//			,(MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[12]));
	//MV_DPRINT("SGAddr=0x%x,SGAddrH=0x%x,REV=0x%x,SGCnt=0x%x\n",(MV_U32)(*(MV_PU32)&pTable[128]),(MV_U32)(*(MV_PU32)&pTable[132]),(MV_U32)(*(MV_PU32)&pTable[136]),pCmdTable->PRD_Entry[0].Size);
#endif

	MV_ENTERLINE;
#endif

}

/*
 * Fill the PATA command table
*/
MV_VOID PATA_PrepareCommandTable(
	PDomain_Port pPort, 
	PMV_Request pReq, 
	MV_U8 tag,
	PATA_TaskFile pTaskFile
	)
{
	PMV_Command_Table pCmdTable = Port_GetCommandTable(pPort, tag);
#ifdef DEBUG_BIOS
	MV_PU8 pTable;
#endif

	PMV_SG_Table pSGTable = &pReq->SG_Table;
	PMV_SG_Entry pSGEntry = NULL;
	MV_PU8 pU8 = (MV_PU8)pCmdTable;
	MV_U8 i, device_index;

	device_index = PATA_MapDeviceId(pReq->Device_Id);

	/* Step 1: Fill the command block */
	(*pU8)=pTaskFile->Features; pU8++;
	(*pU8)=pTaskFile->Feature_Exp; pU8++;
	(*pU8)=pTaskFile->Sector_Count; pU8++;
	(*pU8)=pTaskFile->Sector_Count_Exp; pU8++;
	(*pU8)=pTaskFile->LBA_Low; pU8++;
	(*pU8)=pTaskFile->LBA_Low_Exp; pU8++;
	(*pU8)=pTaskFile->LBA_Mid; pU8++;
	(*pU8)=pTaskFile->LBA_Mid_Exp; pU8++;
	(*pU8)=pTaskFile->Command; pU8++;
	(*pU8)=pTaskFile->Device; pU8++;
	(*pU8)=pTaskFile->LBA_High; pU8++;
	(*pU8)=pTaskFile->LBA_High_Exp; pU8++;
	*((MV_PU32)pU8) = 0L;
    
	/* Step 2: Fill the ATAPI CDB */
	if ( pReq->Cmd_Flag&CMD_FLAG_PACKET )
	{
		MV_CopyMemory(pCmdTable->ATAPI_CDB, pReq->Cdb, MAX_CDB_SIZE);
	}

	/* Step 3: Fill the PRD Table if necessary. */
	if ( (pSGTable) && (pSGTable->Valid_Entry_Count) )
	{
		//TBD: C0 board AHCI is 0 based while Ax board AHCI PATA is 1 based.
		#if 0
		/* For thor, we can just copy the PRD table */
		MV_CopyMemory( 
			pCmdTable->PRD_Entry, 
			pSGTable->Entry,
			sizeof(MV_SG_Entry)*pSGTable->Valid_Entry_Count 
			);
		#else
		/* "Transfer Byte Count" in AHCI and 614x PRD table is zero based. */
		for ( i=0; i<pSGTable->Valid_Entry_Count; i++ )
		{
			pSGEntry = &pCmdTable->PRD_Entry[i];
			pSGEntry->Base_Address = CPU_TO_LE_32(pSGTable->Entry_Ptr[i].Base_Address);
			pSGEntry->Base_Address_High = CPU_TO_LE_32(pSGTable->Entry_Ptr[i].Base_Address_High);
			pSGEntry->Size = CPU_TO_LE_32(pSGTable->Entry_Ptr[i].Size-1);
		}
		#endif
	}
	else
	{	
		MV_DASSERT( !SCSI_IS_READ(pReq->Cdb[0]) && !SCSI_IS_WRITE(pReq->Cdb[0]) );
	}

#ifdef DEBUG_BIOS	

	pTable=(MV_PU8)pPort->Cmd_Table;
	//MV_DUMPC32(0xCCCCEEE2);
	//MV_DUMPC16((MV_U16)pTable);
	//MV_DUMPC32(pPort->Cmd_Table_DMA.low);
	MV_DUMPC32(0xCCCCEE21);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->FIS[0]));
	MV_DUMPC32(0xCCCCEE22);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->FIS[4]));
	MV_DUMPC32(0xCCCCEE23);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->FIS[8]));
	MV_DUMPC32(0xCCCCEE24);
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->FIS[12]));
#if 1
	MV_DUMPC32(0xCCCCEEE3);
	//MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[0]));
	//MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[4]));
	//MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[8]));
	//MV_DUMPC32((MV_U32)(*(MV_PU32)&pCmdTable->ATAPI_CDB[12]));
	//MV_DUMPC32(0xCCCCEEE4);
	//(MV_U32)(*(MV_PU32)&pTable[136]) =(MV_U32) 0;
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pTable[128]));
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pTable[132]));
	MV_DUMPC32((MV_U32)(*(MV_PU32)&pTable[136]));
	MV_DUMPC32(pCmdTable->PRD_Entry[0].Size);

	MV_DUMPC32(0xCCCCEEE5);

	#ifdef DEBUG_BIOS
		if((MV_U32)(*(MV_PU32)&pTable[132]) != 0)
		{
			MV_DUMPC32(0xCCCC990D);
			MV_HALTKEY;
		}
	#endif
	
#endif

	MV_ENTERLINE;
#endif
}

void SATA_SendFrame(PDomain_Port pPort, PMV_Request pReq, MV_U8 tag)
{
	MV_LPVOID portMmio = pPort->Mmio_Base;
#ifdef _OS_BIOS
	MV_U32 	DelayTimer=100;
	MV_U32 	irqStatus=0;
	PCore_Driver_Extension pCore= pPort->Core_Extension;
   	PMV_DATA_STRUCT pDriverData = MyDriverDataBaseOff;
#endif

	MV_DASSERT( (pPort->Running_Slot&(1<<tag))==0 );
	MV_DASSERT( pPort->Running_Req[tag]==0 );
	MV_DASSERT( (MV_REG_READ_DWORD(portMmio, PORT_CMD_ISSUE)&(1<<tag))==0 );
	MV_DASSERT( (MV_REG_READ_DWORD(portMmio, PORT_SCR_ACT)&(1<<tag))==0 );

#ifdef MV_DEBUG
	if ( pPort->Running_Slot!=0 )
	{
		//MV_DPRINT(("M: S=0x%x, T=0x%x.\n", pPort->Running_Slot, tag));
	}
#endif

	pPort->Running_Slot |= 1<<tag;
	pPort->Running_Req[tag] = pReq;

	if ( pReq->Cmd_Flag&CMD_FLAG_NCQ )
		pPort->Setting |= PORT_SETTING_NCQ_RUNNING;
	else
		pPort->Setting &= ~PORT_SETTING_NCQ_RUNNING;

	if ( pReq->Scsi_Status==REQ_STATUS_RETRY )
	{
		MV_PRINT("Retry request...");
		MV_DumpRequest(pReq, MV_FALSE);
		pPort->Setting |= PORT_SETTING_DURING_RETRY;
	}
	else
	{
		pPort->Setting &= ~PORT_SETTING_DURING_RETRY;
	}

	if ( pPort->Setting&PORT_SETTING_NCQ_RUNNING )
	{
		MV_REG_WRITE_DWORD(portMmio, PORT_SCR_ACT, 1<<tag);
		MV_REG_READ_DWORD(portMmio, PORT_SCR_ACT);	/* flush */
	}

#ifdef _OS_BIOS
/* Cleare All interrupt status */
	MV_REG_WRITE_DWORD(portMmio, PORT_IRQ_STAT,0xFFFFFFFF);
	//HBA_SleepMillisecond(NULL, 10);
#endif	

#ifdef DEBUG_BIOS
	MV_DUMPC32(0xCCCCCCFE);
	MV_DUMPC32(MV_REG_READ_DWORD(portMmio, PORT_CMD_ISSUE));
	MV_DUMPC32(MV_REG_READ_DWORD(portMmio, PORT_IRQ_STAT));
	if(MV_REG_READ_DWORD(portMmio, PORT_CMD_ISSUE)||MV_REG_READ_DWORD(portMmio, PORT_IRQ_STAT))	
	{	
		MV_DUMPC32(0xCCCCCCFF);
		MV_HALTKEY;
	}
	MV_ENTERLINE;
#endif

#ifdef MV_IRQ_MODE
	pDriverData->mvIRQStatus=MV_FALSE;
	__asm	sti;
#endif

	MV_REG_WRITE_DWORD(portMmio, PORT_CMD_ISSUE, 1<<tag);
	MV_REG_READ_DWORD(portMmio, PORT_CMD_ISSUE);	/* flush */
	
	/* The End of SATA_SendFrame() for OS code. */
	
#ifdef _OS_BIOS

#ifdef MV_IRQ_MODE
	while(!pDriverData->mvIRQStatus && DelayTimer)
	{
		MV_DUMPC16(0xC990);
		HBA_SleepMillisecond(NULL, 1);
		DelayTimer--;
	}	
#else	/* #ifdef MV_IRQ_MODE */
	while(DelayTimer && !irqStatus)
	{
		HBA_SleepMillisecond(NULL, 1);
		irqStatus = MV_REG_READ_DWORD(pCore->Mmio_Base, HOST_IRQ_STAT);
		DelayTimer--;
		MV_DUMPC16(0xC990);
	}
#endif	/* #ifdef MV_IRQ_MODE */

	if((pDriverData->mvIRQStatus==MV_FALSE)||(DelayTimer == 0))
	{
		MV_DUMPC32(0xCCC4422);
		if(!Core_InterruptServiceRoutine(pCore))
		{
			MV_DUMPC32(0xCCCC4423);
			MV_HALTKEY;
		}
	}
	
	if ( pPort->Type==PORT_TYPE_PATA )
		PATA_PortHandleInterrupt(pCore, pPort);
	else
		SATA_PortHandleInterrupt(pCore, pPort);
#endif	/* #ifdef _OS_BIOS */

	MV_DUMPC32(0xCCCC4421);
}

//Hardware Reset. Added by Lily

MV_BOOLEAN Core_WaitingForIdle(MV_PVOID pExtension)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)pExtension;
	PDomain_Port pPort = NULL;
	MV_U8 i;

	for ( i=0; i<pCore->Port_Num; i++ )
	{
		pPort = &pCore->Ports[i];

		if ( pPort->Running_Slot!=0 )
			return MV_FALSE;
	}
	
	return MV_TRUE;
}

MV_BOOLEAN ResetController(PCore_Driver_Extension pCore);

//TBD: Replace this function with existing functions.
void Core_ResetHardware(MV_PVOID pExtension)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)pExtension;
	MV_U32 i, j;
	PDomain_Port pPort = NULL;
	PDomain_Device pDevice = NULL;

	/* Re-initialize some variables to make the reset go. */
	//TBD: Any more variables?
	pCore->Adapter_State = ADAPTER_INITIALIZING;
	for ( i=0; i<MAX_PORT_NUMBER; i++ )
	{
		pPort = &pCore->Ports[i];
		pPort->Port_State = PORT_STATE_IDLE;
		for ( j=0; j<MAX_DEVICE_PER_PORT; j++ )
		{
			pDevice = &pPort->Device[j];
			pDevice->State = DEVICE_STATE_IDLE;
		}
	}

	/* Go through the mvAdapterStateMachine. */
	if( pCore->Resetting==0 )
	{
		pCore->Resetting = 1;
		if( !mvAdapterStateMachine(pCore) )
		{
			MV_ASSERT(MV_FALSE);
		}
	}
	else
	{
		/* I suppose that we only have one chance to call Core_ResetHardware. */
		MV_DASSERT(MV_FALSE);
	}
	
	return;
}

void PATA_LegacyPollSenseData(PCore_Driver_Extension pCore, PMV_Request pReq)
{
#ifndef _OS_BIOS
	/* Use legacy mode to poll the sense data. */
	//TBD: Let me fake the sense data first to see what's gonna happen.
	//The sense data is got from the trace.
	/* 
	 * This sense data says:
	 * Format: Fixed format sense data
	 * Sense key: Hardware error
	 * Sense code and qualifier: 08h 03h LOGICAL UNIT COMMUNICATION CRC ERROR
	 */
	MV_U8 fakeSense[]={0xF0, 0x00, 0x04, 0x00, 0x00, 0x01, 
		0xEA, 0x0A, 0x74, 0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x00, 0x00, 0x00};
	MV_U32 size = MV_MIN(sizeof(fakeSense)/sizeof(MV_U8), pReq->Sense_Info_Buffer_Length);

	MV_CopyMemory(pReq->Sense_Info_Buffer, fakeSense, size);
#endif

}

void Core_FillSenseData(PMV_Request pReq, MV_U8 senseKey, MV_U8 adSenseCode)
{
	if (pReq->Sense_Info_Buffer != NULL) {
		((MV_PU8)pReq->Sense_Info_Buffer)[0] = 0x70;	/* Current */
		((MV_PU8)pReq->Sense_Info_Buffer)[2] = senseKey;
		((MV_PU8)pReq->Sense_Info_Buffer)[7] = 0;		/* additional sense length */
		((MV_PU8)pReq->Sense_Info_Buffer)[12] = adSenseCode;	/* additional sense code */
	}
}

void mvScsiInquiry(PCore_Driver_Extension pCore, PMV_Request pReq)
{
#ifndef _OS_BIOS
	PDomain_Device pDevice = NULL;
	MV_U8 portId, deviceId;

	portId = PATA_MapPortId(pReq->Device_Id);
	deviceId = PATA_MapDeviceId(pReq->Device_Id);
	pDevice = &pCore->Ports[portId].Device[deviceId];
	MV_ZeroMemory(pReq->Data_Buffer, pReq->Data_Transfer_Length);

	if ( pReq->Cdb[1] & CDB_INQUIRY_EVPD )
	{
		MV_U8 MV_INQUIRY_VPD_PAGE0_DATA[6] = {0x00, 0x00, 0x00, 0x02, 0x00, 0x80};
		MV_U32 tmpLen = 0;
		pReq->Scsi_Status = REQ_STATUS_SUCCESS;

		/* Shall return the specific page of Vital Production Data */
		switch (pReq->Cdb[2]) {
		case 0x00:	/* Supported VPD pages */
			tmpLen = MV_MIN(pReq->Data_Transfer_Length, 6);
			MV_CopyMemory(pReq->Data_Buffer, MV_INQUIRY_VPD_PAGE0_DATA, tmpLen);
			break;
		case 0x80:	/* Unit Serial Number VPD Page */
			if (pReq->Data_Transfer_Length > 1)
				*(((MV_PU8)(pReq->Data_Buffer)) + 1) = 0x80;
			tmpLen = MV_MIN(pReq->Data_Transfer_Length, 4);
			if (tmpLen >= 4) {
				tmpLen = MV_MIN((pReq->Data_Transfer_Length-4), 20);
				MV_CopyMemory(((MV_PU8)(pReq->Data_Buffer)+4), pDevice->Serial_Number, tmpLen);
				*(((MV_PU8)(pReq->Data_Buffer)) + 3) = (MV_U8)tmpLen;
				tmpLen += 4;
			}
			break;
		case 0x83:	/* Device Identification VPD Page */
			/* Here is using Vendor Specific Identifier Format */
			if (pReq->Data_Transfer_Length > 8) {
				*(((MV_PU8)(pReq->Data_Buffer)) + 1) = 0x83;
				*(((MV_PU8)(pReq->Data_Buffer)) + 4) = 0x02;	/* Code Set */
				tmpLen = MV_MIN((pReq->Data_Transfer_Length-8), 40);
				MV_CopyMemory(((MV_PU8)(pReq->Data_Buffer)+8), pDevice->Model_Number, tmpLen);
				*(((MV_PU8)(pReq->Data_Buffer)) + 7) = (MV_U8)tmpLen;	/* Identifier Length */
				*(((MV_PU8)(pReq->Data_Buffer)) + 3) = (MV_U8)(tmpLen + 4);	/* Page Length */
			}
			tmpLen += 8;
			break;
		default:
			pReq->Scsi_Status = REQ_STATUS_HAS_SENSE;
			Core_FillSenseData(pReq, SCSI_SK_ILLEGAL_REQUEST, SCSI_ASC_INVALID_FEILD_IN_CDB);
			break;
		}
		pReq->Data_Transfer_Length = tmpLen;
	} 
	else
	{
		/* Standard inquiry */
		if (pReq->Cdb[2]!=0) {
			/* PAGE CODE field must be zero when EVPD is zero for a valid request */
			/* sense key as ILLEGAL REQUEST and additional sense code as INVALID FIELD IN CDB */
			pReq->Scsi_Status = REQ_STATUS_HAS_SENSE;
			Core_FillSenseData(pReq, SCSI_SK_ILLEGAL_REQUEST, SCSI_ASC_INVALID_FEILD_IN_CDB);
			return;
		}
		
#ifdef SUPPORT_SCSI_PASSTHROUGH

		/* console */
		if ( pReq->Device_Id == CONSOLE_ID )	
		{
			MV_U8 bConsoleInquiryPage[64] = {
				0x03,0x00,0x03,0x03,0xFA,0x00,0x00,0x30,
				'M', 'a', 'r', 'v', 'e', 'l', 'l', ' ',
				0x52,0x41,0x49,0x44,0x20,0x43,0x6F,0x6E,  /* "Raid Con" */
				0x73,0x6F,0x6C,0x65,0x20,0x20,0x20,0x20,  /* "sole    " */
				0x31,0x2E,0x30,0x30,0x20,0x20,0x20,0x20,  /* "1.00    " */
				0x53,0x58,0x2F,0x52,0x53,0x41,0x46,0x2D,  /* "SX/RSAF-" */
				0x54,0x45,0x31,0x2E,0x30,0x30,0x20,0x20,  /* "TE1.00  " */
				0x0C,0x20,0x20,0x20,0x20,0x20,0x20,0x20
			};

			MV_CopyMemory( pReq->Data_Buffer, 
						   bConsoleInquiryPage, 
						   MV_MIN(pReq->Data_Transfer_Length, 64)
						 );

			pReq->Scsi_Status = REQ_STATUS_SUCCESS;
			return;
		}

#endif 

		if ( (portId>=pCore->Port_Num)||(deviceId>=MAX_DEVICE_PER_PORT) )
		{
			pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
		}
		else
		{
//			pDevice = &pCore->Ports[portId].Device[deviceId];
			if ( pDevice->Status&DEVICE_STATUS_FUNCTIONAL )
			{
#if 0
				#define STANDARD_INQUIRY_DATA_SIZE	36
				MV_U8 MV_INQUIRY_DATA[STANDARD_INQUIRY_DATA_SIZE] = {
					0, 0, 0x02, 0x02, STANDARD_INQUIRY_DATA_SIZE - 5, 0, 0, 0x13,
					'M', 'a', 'r', 'v', 'e', 'l', 'l', ' ',
					'P', 'r', 'o', 'd ', 'u', 'c', 't', ' ', 'I', 'd', 'e', 'n', 't', 'i', 'f', 'c',
					'1', '.', '0', '1'};

				MV_CopyMemory( pReq->Data_Buffer, 
								MV_INQUIRY_DATA, 
								MV_MIN(pReq->Data_Transfer_Length, STANDARD_INQUIRY_DATA_SIZE)
								);
#else
				{
					MV_U8 Vendor[9],Product[17], temp[24];
				    MV_U8 buff[42];
					MV_U32 inquiryLen;
					MV_ZeroMemory(buff, 42);
					
					if (pDevice->Device_Type & \
					    DEVICE_TYPE_ATAPI) {
						buff[0] = 0x05;
						buff[1] = 0x00 | 1U<<7; 
					} else {
						buff[0] = 0;
						buff[1] = 0;
					}

					buff[2] = 0x05;   //TBD 3  /*claim conformance to SCSI-3*/
					buff[3] = 0x12;    /* set RESPONSE DATA FORMAT to 2*/
					buff[4] = 42 - 5;
					buff[6] = 0x0;     /* tagged queuing*/
					buff[7] = 0X13;	//TBD 2;

					MV_CopyMemory(temp, pDevice->Model_Number, 24);
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
							MV_CopyMemory(Product, temp, 16);
							Product[16] = '\0';
						}
						else
						{
							MV_U32 j = i;
							MV_CopyMemory(Vendor, temp, j);
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
							MV_CopyMemory(Product, &temp[i], 24 - i);
							Product[16] = '\0';
						}
						MV_CopyMemory(&buff[8], Vendor, 8);
						MV_CopyMemory(&buff[16], Product, 16);
						MV_CopyMemory(&buff[32], pDevice->Firmware_Revision, 4);
					}
					MV_CopyMemory(&buff[36], "MVSATA", 6);

					/*buff[32] = '3';*/

					inquiryLen = 42;
					MV_CopyMemory( pReq->Data_Buffer, 
								buff, 
								MV_MIN(pReq->Data_Transfer_Length, inquiryLen)
								);
					pReq->Data_Transfer_Length = MV_MIN(pReq->Data_Transfer_Length, inquiryLen);
				}
#endif
				pReq->Scsi_Status = REQ_STATUS_SUCCESS;
			}
			else
			{
				pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
			}
		}
	}
#endif	/* #ifndef _OS_BIOS */
}

#ifndef _OS_BIOS
#define MAX_MODE_PAGE_LENGTH	28
MV_U32 Core_get_mode_page_caching(MV_PU8 pBuf, PDomain_Device pDevice)
{
	pBuf[0] = 0x08;		/* Page Code, PS = 0; */
	pBuf[1] = 0x12;		/* Page Length */
	/* set the WCE and RCD bit based on device identification data */
	if (pDevice->Setting & DEVICE_SETTING_WRITECACHE_ENABLED)
		pBuf[2] |= MV_BIT(2);
	pBuf[3] = 0;	/* Demand read/write retention priority */
	pBuf[4] = 0xff;	/* Disable pre-fetch trnasfer length (4,5) */
	pBuf[5] = 0xff;	/* all anticipatory pre-fetching is disabled */
	pBuf[6] = 0;	/* Minimum pre-fetch (6,7) */
	pBuf[7] = 0;
	pBuf[8] = 0;	/* Maximum pre-fetch (8,9) */
	pBuf[9] = 0x01;
	pBuf[10] = 0;	/* Maximum pre-fetch ceiling (10,11) */
	pBuf[11] = 0x01;
//	pBuf[12] |= MV_BIT(5);	/* How do I know if Read Ahead is enabled or disabled???  */
	pBuf[12] = 0x00;
	pBuf[13] = 0x01;	/* Number of cache segments */
	pBuf[14] = 0xff;	/* Cache segment size (14, 15) */
	pBuf[15] = 0xff;
	return 0x14;	/* Total page length in byte */
}

#endif

void mvScsiModeSense(PCore_Driver_Extension pCore, PMV_Request pReq)
{
#ifndef _OS_BIOS
	MV_U8 pageCode = pReq->Cdb[2] & 0x3F;		/* Same for mode sense 6 and 10 */
	MV_U8 ptmpBuf[MAX_MODE_PAGE_LENGTH];
	MV_U32 pageLen = 0, tmpLen = 0;
	PDomain_Device pDevice = NULL;
	MV_U8 portId, deviceId;
	MV_U8 *buf = pReq->Data_Buffer;

	portId = PATA_MapPortId(pReq->Device_Id);
	deviceId = PATA_MapDeviceId(pReq->Device_Id);
	pDevice = &pCore->Ports[portId].Device[deviceId];

	MV_ZeroMemory(buf, pReq->Data_Transfer_Length);
	MV_ZeroMemory(ptmpBuf, MAX_MODE_PAGE_LENGTH);
	/* Block Descriptor Length set to 0 - No Block Descriptor */

	switch (pageCode) {
	case 0x3F:		/* Return all pages */
	case 0x08:		/* Caching mode page */
		if (pReq->Cdb[0]==SCSI_CMD_MODE_SENSE_6) {
			pageLen = Core_get_mode_page_caching((ptmpBuf+4), pDevice);
			ptmpBuf[0] = (MV_U8)(4 + pageLen - 1);	/* Mode data length */
			ptmpBuf[2] = 0x10;
			tmpLen = MV_MIN(pReq->Data_Transfer_Length, (pageLen+4));
		}
		else {	/* Mode Sense 10 */
			pageLen = Core_get_mode_page_caching((ptmpBuf+8), pDevice);
			/* Mode Data Length, it does not include the number of bytes in */
			/* Mode Data Length field */
			tmpLen = 8 + pageLen - 2;
			ptmpBuf[0] = (MV_U8)(((MV_U16)tmpLen) >> 8);
			ptmpBuf[1] = (MV_U8)tmpLen;
			ptmpBuf[2] = 0x00;
			ptmpBuf[3] = 0x10;
			tmpLen = MV_MIN(pReq->Data_Transfer_Length, (pageLen+8));
		}
		MV_CopyMemory(buf, ptmpBuf, tmpLen);
		pReq->Data_Transfer_Length = tmpLen;
		pReq->Scsi_Status = REQ_STATUS_SUCCESS;
		break;
	default:
		pReq->Scsi_Status = REQ_STATUS_HAS_SENSE;
		Core_FillSenseData(pReq, SCSI_SK_ILLEGAL_REQUEST, SCSI_ASC_INVALID_FEILD_IN_CDB);
		break;
	}
#endif
}

void mvScsiReportLun(PCore_Driver_Extension pCore, PMV_Request pReq)
{
#ifndef _OS_BIOS
	MV_U32 allocLen, lunListLen;
	MV_PU8 pBuf = pReq->Data_Buffer;

	allocLen = ((MV_U32)(pReq->Cdb[6] << 24)) |
			   ((MV_U32)(pReq->Cdb[7] << 16)) |
			   ((MV_U32)(pReq->Cdb[8] << 8)) |
			   ((MV_U32)(pReq->Cdb[9]));

	/* allocation length should not less than 16 bytes */
	if (allocLen < 16) {
		pReq->Scsi_Status = REQ_STATUS_HAS_SENSE;
		Core_FillSenseData(pReq, SCSI_SK_ILLEGAL_REQUEST, SCSI_ASC_INVALID_FEILD_IN_CDB);
		return;
	}

	MV_ZeroMemory(pBuf, pReq->Data_Transfer_Length);
	/* Only LUN 0 has device */
	lunListLen = 8;
	pBuf[0] = (MV_U8)((lunListLen & 0xFF000000) >> 24);
	pBuf[1] = (MV_U8)((lunListLen & 0x00FF0000) >> 16);
	pBuf[2] = (MV_U8)((lunListLen & 0x0000FF00) >> 8);
	pBuf[3] = (MV_U8)(lunListLen & 0x000000FF);
	pReq->Scsi_Status = REQ_STATUS_SUCCESS;
#endif
}

void mvScsiReadCapacity(PCore_Driver_Extension pCore, PMV_Request pReq)
{
#ifndef _OS_BIOS
	PDomain_Device pDevice = NULL;
	MV_LBA maxLBA;
	MV_U32 blockLength;
	MV_PU32 pU32Buffer;
	MV_U8 portId, deviceId;

	portId = PATA_MapPortId(pReq->Device_Id);
	deviceId = PATA_MapDeviceId(pReq->Device_Id);
#ifndef SECTOR_SIZE
	#define SECTOR_SIZE	512	//TBD
#endif

	MV_DASSERT( portId < MAX_PORT_NUMBER );

	if ((pReq->Cdb[8] & MV_BIT(1)) == 0)
	{
		if ( pReq->Cdb[2] || pReq->Cdb[3] || pReq->Cdb[4] || pReq->Cdb[5] )
		{
			pReq->Scsi_Status = REQ_STATUS_INVALID_REQUEST;
			return;
		}
	}

    /* 
	 * The disk size as indicated by the ATA spec is the total addressable
     * sectors on the drive ; while the SCSI translation of the command
     * should be the last addressable sector.
     */
	pDevice = &pCore->Ports[portId].Device[deviceId];
	maxLBA.value = pDevice->Max_LBA.value-1;
	blockLength = SECTOR_SIZE;			//TBD
	pU32Buffer = (MV_PU32)pReq->Data_Buffer;
	MV_ASSERT(maxLBA.high==0);	//TBD: Support Read Capactiy 16 

	pU32Buffer[0] = CPU_TO_BIG_ENDIAN_32(maxLBA.low);
	pU32Buffer[1] = CPU_TO_BIG_ENDIAN_32(blockLength);

	pReq->Scsi_Status = REQ_STATUS_SUCCESS;
#endif /* #ifndef _OS_BIOS */

}

void Port_Monitor(PDomain_Port pPort);
#if defined(SUPPORT_ERROR_HANDLING)

MV_BOOLEAN Core_IsInternalRequest(PCore_Driver_Extension pCore, 
				  PMV_Request pReq)
{
	PDomain_Device pDevice;
	MV_U8 portId = PATA_MapPortId(pReq->Device_Id);
	MV_U8 deviceId = PATA_MapDeviceId(pReq->Device_Id);

	if ( portId>=MAX_PORT_NUMBER )
		return MV_FALSE;
	if ( deviceId>=MAX_DEVICE_PER_PORT )
		return MV_FALSE;

	pDevice = &pCore->Ports[portId].Device[deviceId];
	if ( pReq==pDevice->Internal_Req ) 
		return MV_TRUE;
	else
		return MV_FALSE;
}

void Core_ResetChannel(MV_PVOID Device)
{
	PDomain_Device pDevice = (PDomain_Device)Device;
	PDomain_Port pPort = pDevice->PPort;
	PCore_Driver_Extension pCore = pPort->Core_Extension;
	PMV_Request pReq;
	MV_LPVOID portMmio = pPort->Mmio_Base;
	MV_U32 tmp;
	MV_U8 i;
	
	MV_DPRINT(("Request time out. Resetting channel %d.\n", pPort->Id));
#ifdef SUPPORT_EVENT
	HBA_AddEvent( pCore, EVT_ID_HD_TIMEOUT, pDevice->Id, 3, 0, NULL );
#endif /* SUPPORT_EVENT */

	/* toggle the start bit in cmd register */
	tmp = MV_REG_READ_DWORD( portMmio, PORT_CMD );
	MV_REG_WRITE_DWORD( portMmio, PORT_CMD, tmp & ~MV_BIT(0));
	MV_REG_WRITE_DWORD( portMmio, PORT_CMD, tmp | MV_BIT(0));
	HBA_SleepMillisecond( pCore, 100 );

	Port_Monitor( pPort );
	pDevice->Reset_Count++;

	/* Whether it's during reset, we got reset again. */
	if ( pPort->Port_State!=PORT_STATE_INIT_DONE )
	{
		MV_PRINT("Timeout during reset.\n");
	}

	/* put all the running requests back into waiting list */
	for ( i=0; i<MAX_SLOT_NUMBER; i++ )
	{
		pReq = pPort->Running_Req[i];

		if (pReq) {
			/*
			 * If this channel has multiple devices, pReq is 
			 * not the internal request of pDevice
			 */
			if ( !Core_IsInternalRequest(pCore, pReq) )
			{
				List_Add(&pReq->Queue_Pointer, &pCore->Waiting_List);
			}
			else 
			{
				/* Can be reset command or request sense command */
				if ( SCSI_IS_REQUEST_SENSE(pReq->Cdb[0]) )
				{
					MV_ASSERT( pReq->Org_Req!=NULL );
					if ( pReq->Org_Req )
						List_Add( &((PMV_Request)pReq->Org_Req)->Queue_Pointer, &pCore->Waiting_List);
				}
			}
			
#if defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX)
			hba_remove_timer(pReq);
			pReq->eh_flag = 1;
#endif /* defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX) */
			pPort->Running_Req[i] = NULL;
			pPort->Running_Slot &= ~(1L<<i);
			Tag_ReleaseOne(&pPort->Tag_Pool, i);
		}
	}
	
	/* reset device related variables */
	for ( i=0; i<MAX_DEVICE_PER_PORT; i++ )
	{
		pDevice = &pPort->Device[i];
		
		pDevice->Device_Type = 0;
		pDevice->Need_Notify = MV_FALSE;
#ifdef SUPPORT_TIMER 
		if( pDevice->Timer_ID != NO_CURRENT_TIMER )
		{
			Timer_CancelRequest( pCore, pDevice->Timer_ID );
			pDevice->Timer_ID = NO_CURRENT_TIMER;
		}
#endif /* SUPPORT_TIMER */
		pDevice->Outstanding_Req = 0;
		
		/*
		 * Go through the waiting list. If there is some reset 
		 * request, remove that request. 
		 */
		mvRemoveDeviceWaitingList(pCore, pDevice->Id, MV_FALSE);
	}

	// reset the tag stack - to guarantee soft reset is issued at slot 0
	Tag_Init( &pPort->Tag_Pool, MAX_TAG_NUMBER );

	for( i=0; i<MAX_DEVICE_PER_PORT; i++ )
	{
		if( (pPort->Device[i].Status & DEVICE_STATUS_FUNCTIONAL) && 
			(pPort->Device[i].Internal_Req != NULL) )
		{
			pCore->Total_Device_Count--;
			ReleaseInternalReqToPool( pCore, pPort->Device[i].Internal_Req );
			pPort->Device[i].Internal_Req = NULL;
		}
	}
	pPort->Port_State = PORT_STATE_IDLE;
	if ( pPort->Type == PORT_TYPE_PATA )
		PATA_PortReset( pPort, MV_TRUE );
	else
		SATA_PortReset( pPort, MV_FALSE );
}
#endif /* SUPPORT_ERROR_HANDLING || _OS_LINUX */

MV_BOOLEAN HandleInstantRequest(PCore_Driver_Extension pCore, PMV_Request pReq)
{
	/* 
	 * Some of the requests can be returned immediately without hardware 
	 * access. 
	 * Handle Inquiry and Read Capacity.
	 * If return MV_TRUE, means the request can be returned to OS now.
	 */
	PDomain_Device pDevice = NULL;
	MV_U8 portId, deviceId;

	portId = PATA_MapPortId(pReq->Device_Id);
	deviceId = PATA_MapDeviceId(pReq->Device_Id);

	if ( portId < MAX_PORT_NUMBER )				
		pDevice = &pCore->Ports[portId].Device[deviceId];

	if (pDevice && 
	    (pDevice->Device_Type & DEVICE_TYPE_ATAPI) &&
	    (pDevice->Status & DEVICE_STATUS_FUNCTIONAL))
	{
#ifdef _OS_LINUX
		switch (pReq->Cdb[0]) 
		{
		case SCSI_CMD_MODE_SENSE_6:
			/* convert to atapi cdb12 */
			pReq->Cdb[8] = pReq->Cdb[4];
			pReq->Cdb[4] = 0;
			pReq->Cdb[0] = SCSI_CMD_MODE_SENSE_10;
		case SCSI_CMD_INQUIRY:
		case SCSI_CMD_READ_CAPACITY_10:
		case SCSI_CMD_READ_CAPACITY_16:
		case SCSI_CMD_REPORT_LUN:
		case SCSI_CMD_MODE_SENSE_10:
			if (pReq->Cmd_Initiator == 
			    HBA_GetModuleExtension(pReq->Cmd_Initiator, MODULE_HBA))
				HBA_kunmap_sg(pReq);
			break;
		default:
			break;
		}
#endif /* _OS_LINUX */
		return MV_FALSE;
	}

	switch ( pReq->Cdb[0] )
	{
	case SCSI_CMD_INQUIRY:
		mvScsiInquiry(pCore, pReq);
		return MV_TRUE;
	case SCSI_CMD_MODE_SENSE_6:
	case SCSI_CMD_MODE_SENSE_10:
		mvScsiModeSense(pCore, pReq);
		return MV_TRUE;
	case SCSI_CMD_REPORT_LUN:
		mvScsiReportLun(pCore, pReq);
		return MV_TRUE;
	case SCSI_CMD_READ_CAPACITY_10:
		mvScsiReadCapacity(pCore, pReq);
		return MV_TRUE;
#ifdef _OS_LINUX
	case SCSI_CMD_READ_CAPACITY_16: /* 0x9e SERVICE_ACTION_IN */
		if ((pReq->Cdb[1] & 0x1f) == 0x10 /* SAI_READ_CAPACITY_16 */) {
			MV_PU32 pU32Buffer = (MV_PU32)pReq->Data_Buffer;
			MV_LBA maxLBA;
			MV_U32 blockLength = SECTOR_SIZE;
			maxLBA.value = pDevice->Max_LBA.value-1;;

			pU32Buffer[0] = CPU_TO_BIG_ENDIAN_32(maxLBA.low);
			pU32Buffer[1] = CPU_TO_BIG_ENDIAN_32(maxLBA.high);
			pU32Buffer[2] =  CPU_TO_BIG_ENDIAN_32(blockLength);;
			pReq->Scsi_Status = REQ_STATUS_SUCCESS;;
		}
		else
			pReq->Scsi_Status = REQ_STATUS_INVALID_REQUEST;
		return MV_TRUE;
#endif /* _OS_LINUX */
	case SCSI_CMD_REQUEST_SENSE:	/* This is only for Thor hard disk */
	case SCSI_CMD_TEST_UNIT_READY:
	case SCSI_CMD_RESERVE_6:	/* For Thor, just return good status */
	case SCSI_CMD_RELEASE_6:
#ifdef CORE_IGNORE_START_STOP_UNIT
	case SCSI_CMD_START_STOP_UNIT:
#endif
		pReq->Scsi_Status = REQ_STATUS_SUCCESS;
		return MV_TRUE;
#ifdef CORE_SUPPORT_API
	case APICDB0_PD:
		return Core_pd_command(pCore, pReq);
#endif /* CORE_SUPPORT_API */
	}

	return MV_FALSE;
}

MV_QUEUE_COMMAND_RESULT
PrepareAndSendCommand(
	IN PCore_Driver_Extension pCore,
	IN PMV_Request pReq
	)
{
#ifdef MV_IRQ_MODE
   	PMV_DATA_STRUCT pDriverData = MyDriverDataBaseOff;
#endif

	PDomain_Device pDevice = NULL;
	PDomain_Port pPort = NULL;
	MV_BOOLEAN isPATA = MV_FALSE;
	MV_U8 tag, i, count=0;
	ATA_TaskFile taskFile;
	MV_BOOLEAN ret;

	/* Associate this request to the corresponding device and port */
	pDevice = &pCore->Ports[PATA_MapPortId(pReq->Device_Id)].Device[PATA_MapDeviceId(pReq->Device_Id)];
	pPort = pDevice->PPort;

	if ( !(pDevice->Status&DEVICE_STATUS_FUNCTIONAL) )
	{
		pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
		return MV_QUEUE_COMMAND_RESULT_FINISHED;
	}

	/* Set the Cmd_Flag to indicate which type of commmand it is. */
	if ( !Category_CDB_Type(pDevice, pReq) )
	{
		pReq->Scsi_Status = REQ_STATUS_INVALID_REQUEST;
		/* Invalid request and can be returned to OS now. */
		return MV_QUEUE_COMMAND_RESULT_FINISHED;
	}
	//MV_DUMPC32(0xCCCC5502);

	MV_DASSERT( pPort!=NULL );
	if ( pPort->Running_Slot!=0 )	/* Some requests are running. */
	{
		if (	( (pReq->Cmd_Flag&CMD_FLAG_NCQ) && !(pPort->Setting&PORT_SETTING_NCQ_RUNNING) )
			||  ( !(pReq->Cmd_Flag&CMD_FLAG_NCQ) && (pPort->Setting&PORT_SETTING_NCQ_RUNNING) )
			)
		{
			return MV_QUEUE_COMMAND_RESULT_FULL;			
		}
	
		/* In order for request sense to immediately follow the error request. */
		if ( pDevice->Device_Type&DEVICE_TYPE_ATAPI )
			return MV_QUEUE_COMMAND_RESULT_FULL;

		/* One request at a time */
		if ( (pReq->Scsi_Status==REQ_STATUS_RETRY)
			|| (pPort->Setting&PORT_SETTING_DURING_RETRY) 
			)
			return MV_QUEUE_COMMAND_RESULT_FULL;
	}
	//MV_DUMPC32(0xCCCC5503);

	/* we always reserve one slot in case of PM hot plug */
	for (i=0; i<MAX_SLOT_NUMBER; i++)
	{
		if (pPort->Running_Slot & MV_BIT(i))
			count++;
	}
	if (count >= (MAX_SLOT_NUMBER - 1))
	{
		return MV_QUEUE_COMMAND_RESULT_FULL;
	}

	isPATA = (pPort->Type==PORT_TYPE_PATA)?1:0;
	//MV_DUMPC32(0xCCCC5504);

	/* Get one slot for this request. */
	tag = Tag_GetOne(&pPort->Tag_Pool);

	if ( pDevice->Device_Type&DEVICE_TYPE_ATAPI )
		ret = ATAPI_CDB2TaskFile(pDevice, pReq, &taskFile);
	else
		ret = ATA_CDB2TaskFile(pDevice, pReq, tag, &taskFile);
	if ( !ret )
	{
		pReq->Scsi_Status = REQ_STATUS_INVALID_REQUEST;
		Tag_ReleaseOne(&pPort->Tag_Pool, tag);
		/* Invalid request and can be returned to OS now. */
		return MV_QUEUE_COMMAND_RESULT_FINISHED;	
	}

	//MV_DUMPC32(0xCCCC5505);
	
#ifdef _OS_BIOS
	/* All ports share memeory, so need Cmd_List and Cmd_Table before send command */
	if(pPort->Cmd_List)
		MV_ZeroMemory((MV_PU8)pPort->Cmd_List + tag,SATA_CMD_LIST_SIZE);
	if(pPort->Cmd_Table)
		MV_ZeroMemory((MV_PU8)pPort->Cmd_Table+ tag,SATA_CMD_TABLE_SIZE);
	if(pPort->RX_FIS)
		MV_ZeroMemory((MV_PU8)pPort->RX_FIS+ tag,SATA_RX_FIS_SIZE);
#endif	/* #ifdef _OS_BIOS */

	MV_DUMPC32(0xCCCC5506);

	if ( !isPATA )
		SATA_PrepareCommandHeader(pPort, pReq, tag);
	else
		PATA_PrepareCommandHeader(pPort, pReq, tag);


	if ( !isPATA )
		SATA_PrepareCommandTable(pPort, pReq, tag, &taskFile);
	else
		PATA_PrepareCommandTable(pPort, pReq, tag, &taskFile);
	MV_DUMPC32(0xCCCC5507);



	SATA_SendFrame(pPort, pReq, tag);
	/* Request is send to the hardware and not finished yet. */
	return MV_QUEUE_COMMAND_RESULT_SENDTED;
}

void Core_HandleWaitingList(PCore_Driver_Extension pCore)
{
	PMV_Request pReq = NULL;
	MV_QUEUE_COMMAND_RESULT result;
#ifdef SUPPORT_HOT_PLUG	
	PDomain_Device pDevice;
	MV_U8 portId, deviceId;
#endif	
#if defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX)
	MV_U32 timeout;
#endif /* efined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX) */
	//MV_DUMPRUN(0xCCF2);

	/* Get the request header */
	while ( !List_Empty(&pCore->Waiting_List) )
	{
		pReq = (PMV_Request) List_GetFirstEntry(&pCore->Waiting_List, 
							MV_Request, 
							Queue_Pointer);
		if ( NULL == pReq ) {
			MV_ASSERT(0);
			break;
		}

#if defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX)
		pReq->eh_flag = 0;
		hba_init_timer(pReq);
#endif /* defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX) */

		/* During reset, we still have internal requests need to 
		 *be handled. */

		//TBD: Internal request is always at the beginning.
		if ( (pCore->Need_Reset)&&(pReq->Cmd_Initiator!=pCore) ) 
		{
			/* Return the request back. */
			List_Add(&pReq->Queue_Pointer, &pCore->Waiting_List);
			return;
		}
	
#ifdef SUPPORT_HOT_PLUG
		/* hot plug - device is gone, reject this request */
		if ( pReq->Device_Id != CONSOLE_ID )
		{
			portId = PATA_MapPortId(pReq->Device_Id);
			deviceId = PATA_MapDeviceId(pReq->Device_Id);
			pDevice = &pCore->Ports[portId].Device[deviceId];

			if ( !(pDevice->Status & DEVICE_STATUS_FUNCTIONAL) )
			{
				pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
				CompleteRequest(pCore, pReq, NULL);
				return;
			}

			/* Reset is not done yet. */
			if ( pDevice->State!=DEVICE_STATE_INIT_DONE )
			{
				/* check if it is the reset commands */
				if ( !Core_IsInternalRequest(pCore, pReq) )
				{
					List_Add(&pReq->Queue_Pointer, &pCore->Waiting_List); /* Return the request back. */
					return;
				} 
				else 
				{
					/* 
					 * Cannot be the request sense. 
					 * It's not pushed back. 
					 */
					MV_ASSERT( !SCSI_IS_REQUEST_SENSE(pReq->Cdb[0]) );
				}
			}
		}
#endif /* SUPPORT_HOT_PLUG */

		/* Whether we can handle this request without hardware access? */
		if ( HandleInstantRequest(pCore, pReq) ) 
		{
			CompleteRequest(pCore, pReq, NULL);
			continue;
		}

#if !defined(_OS_BIOS) && defined(_OS_WINDOWS)
		/* handle the cmd which data length is > 128k 
		 * We suppose the data length was multiples of 128k first. 
		 * If not, we will still verify multiples of 128k since 
		 * no data transfer.
		 */
		if(pReq->Cdb[0] == SCSI_CMD_VERIFY_10)
		{
			PDomain_Device pDevice = &pCore->Ports[PATA_MapPortId(pReq->Device_Id)].Device[PATA_MapDeviceId(pReq->Device_Id)];
			MV_U32 sectors = SCSI_CDB10_GET_SECTOR(pReq->Cdb);
			
			if((!(pDevice->Capacity&DEVICE_CAPACITY_48BIT_SUPPORTED)) && (sectors > MV_MAX_TRANSFER_SECTOR)){
				MV_ASSERT(!pReq->Splited_Count );
				pReq->Splited_Count = (MV_U8)((sectors + MV_MAX_TRANSFER_SECTOR -1)/MV_MAX_TRANSFER_SECTOR) - 1;
				sectors = MV_MAX_TRANSFER_SECTOR; 
				SCSI_CDB10_SET_SECTOR(pReq->Cdb, sectors);
			}
		}
#endif /* !defined(_OS_BIOS) && defined(_OS_WINDOWS) */

		result = PrepareAndSendCommand(pCore, pReq);	
		//MV_PRINT("Send request.\n");
		//MV_DumpRequest(pReq, MV_FALSE);

		switch ( result )
		{
			case MV_QUEUE_COMMAND_RESULT_FINISHED:
				CompleteRequest(pCore, pReq, NULL);
				break;

			case MV_QUEUE_COMMAND_RESULT_FULL:
				List_Add(&pReq->Queue_Pointer, &pCore->Waiting_List);
				return;

			case MV_QUEUE_COMMAND_RESULT_SENDTED:
			{
				portId = PATA_MapPortId(pReq->Device_Id);
				deviceId = PATA_MapDeviceId(pReq->Device_Id);
				pDevice = &pCore->Ports[portId].Device[deviceId];
				pDevice->Outstanding_Req++;
#if defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX)
				/*
				 * timeout to 15 secs if the port has just
				 * been reset.
				 */
				if (pReq->eh_flag) 
				{
					timeout = HBA_REQ_TIMER_AFTER_RESET;
					pReq->eh_flag = 0; 
				}
				else
				{
					timeout = HBA_REQ_TIMER;
				}

				/* double timeout value for atapi (writers) */
				if (pDevice->Device_Type & DEVICE_TYPE_ATAPI)
					timeout = timeout * 2 + 5;

				hba_add_timer(pReq,
					      timeout,
					      __core_req_timeout_handler);

#elif defined(SUPPORT_ERROR_HANDLING)
#ifdef SUPPORT_TIMER
				/* start timer for error handling */
				if( pDevice->Timer_ID == NO_CURRENT_TIMER )
				{
					// if no timer is running right now
					pDevice->Timer_ID = Timer_AddRequest( pCore, REQUEST_TIME_OUT, Core_ResetChannel, pDevice );
				}
#endif /* SUPPORT_TIMER */
#endif /* defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX) */
#if 0
				{
					MV_U8 i;
					PMV_Request pTmpRequest = NULL;
					PDomain_Port pPort = pDevice->PPort;
					/* When there is reset command, other commands won't come here. */
					if ( SCSI_IS_READ(pReq->Cdb[0]) || SCSI_IS_WRITE(pReq->Cdb[0]) )
					{
						for ( i=0; i<MAX_SLOT_NUMBER; i++ )
						{
							pTmpRequest = pPort->Running_Req[i];
							if ( pTmpRequest && (pTmpRequest->Device_Id==pReq->Device_Id) ) 
							{
								MV_DASSERT( !SCSI_IS_INTERNAL(pTmpRequest->Cdb[0]) );
							}
						}

					}
				}
#endif /* 0 */
				break;
			}
			default:
				MV_ASSERT(MV_FALSE);
		}
	}
	
#ifdef SUPPORT_CONSOLIDATE
	{
		MV_U8 i,j;
		PDomain_Port pPort;

		if ( pCore->Is_Dump ) return;

		/* 
		* If there is no more request we can do, 
		* force command consolidate to run the holding request. 
		*/
		for ( i=0; i<MAX_PORT_NUMBER; i++ )
		{
			pPort = &pCore->Ports[i];
			for ( j=0; j<MAX_DEVICE_PER_PORT; j++ )
			{
				if ( (pPort->Device[j].Status&DEVICE_STATUS_FUNCTIONAL)
					&& (pPort->Device[j].Outstanding_Req==0) )
				{
					Consolid_PushFireRequest(pCore, i*MAX_DEVICE_PER_PORT+j);
				}
			}
		}
	}
#endif /* SUPPORT_CONSOLIDATE */
}

/*
 * Interrupt service routine and related funtion
 * We can split this function to two functions. 
 * One is used to check and clear interrupt, called in ISR. 
 * The other is used in DPC.
 */
void SATA_PortHandleInterrupt(
	IN PCore_Driver_Extension pCore,
	IN PDomain_Port pPort
	);
void PATA_PortHandleInterrupt(
	IN PCore_Driver_Extension pCore,
	IN PDomain_Port pPort
	);
void SATA_HandleSerialError(
	IN PDomain_Port pPort,
	IN MV_U32 serialError
	);
void SATA_HandleHotplugInterrupt(
	IN PDomain_Port pPort,
	IN MV_U32 serialError
	);

MV_BOOLEAN Core_InterruptServiceRoutine(MV_PVOID This)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;
	MV_U32	irqStatus;
	MV_U8 i;
	PDomain_Port pPort = NULL;
#ifdef _OS_BIOS	
   	PMV_DATA_STRUCT pDriverData = MyDriverDataBaseOff;
	MV_U32                   tmp;
#endif

	/* Get interrupt status */
	irqStatus = MV_REG_READ_DWORD(pCore->Mmio_Base, HOST_IRQ_STAT);

	MV_DUMPC32(0xCCCC7781);
	MV_DUMPC32(irqStatus);

#ifdef _OS_BIOS
	if(pCore->host_reseting)
	{
		MV_REG_WRITE_DWORD(pCore->Mmio_Base, 8, 1);
		tmp = MV_IO_READ_DWORD(pCore->Base_Address[4], 0);
		MV_IO_WRITE_DWORD(pCore->Base_Address[4], 0, (tmp | MV_BIT(18)));
		tmp = MV_IO_READ_DWORD(pCore->Base_Address[4], 8);
		MV_IO_WRITE_DWORD(pCore->Base_Address[4], 8, (tmp | MV_BIT(18)));

		pDriverData->PortIRQStatus = 0;
		return MV_TRUE;
	}
#endif
	
	irqStatus &= pCore->Port_Map;

	if (!irqStatus ) 
	{
#ifdef _OS_BIOS	
		pDriverData->PortIRQStatus = 0;
#endif
		MV_DUMPC32(0xCCCC7782);
		MV_HALTKEY;
		return MV_FALSE;
	}

	
#ifndef _OS_BIOS
	for ( i=0; i<pCore->Port_Num; i++ ) 
	{
		MV_DUMPC16(0xCCFA);
		MV_DUMPC16(irqStatus);
		
		if ( !(irqStatus&(1<<i)) )	/* no interrupt for this port. */
			continue;

		pPort = &pCore->Ports[i];
		if ( pPort->Type==PORT_TYPE_PATA )
			PATA_PortHandleInterrupt(pCore, pPort);
		else
			SATA_PortHandleInterrupt(pCore, pPort);
	}
	/* If we need to do hard reset. And the controller is idle now. */
	if ( (pCore->Need_Reset) && (!pCore->Resetting) )
	{
		//MV_DUMPRUN(0xCCFE);
		if( Core_WaitingForIdle(pCore) )
			Core_ResetHardware(pCore);
	}

	Core_HandleWaitingList(pCore);
#else
/* Cleare All interrupt status */
		MV_REG_WRITE_DWORD(pCore->Mmio_Base, HOST_IRQ_STAT,irqStatus);
		MV_REG_READ_DWORD(pCore->Mmio_Base, HOST_IRQ_STAT);
		HBA_SleepMillisecond(NULL, 1);
		pDriverData->PortIRQStatus = irqStatus;
		pDriverData->mvIRQStatus=MV_TRUE;

#endif

	return MV_TRUE;
}
void SATA_HandleSerialError(
	IN PDomain_Port pPort,
	IN MV_U32 serialError
	)
{
	//TBD
#ifndef _OS_BIOS
	MV_PRINT("Error: Port_HandleSerialError port=%d error=0x%x.\n", pPort->Id, serialError);
#endif

}

/*added hot-plug by Lily */
void SATA_ResetPort(PCore_Driver_Extension pCore, MV_U8 portId);

#ifdef SUPPORT_HOT_PLUG
void Device_SoftReset(PDomain_Port pPort, PDomain_Device pDevice);

void mvRemoveDeviceWaitingList(MV_PVOID This, MV_U16 deviceId, 
			       MV_BOOLEAN returnOSRequest)
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;
	PMV_Request pReq = NULL;
	List_Head *pPos;
	List_Head remove_List;
	MV_U8 count = 0, myCount=0, i;
	PDomain_Device pDevice;
	MV_U8 portNum = PATA_MapPortId(deviceId);
	MV_U8 deviceNum = PATA_MapDeviceId(deviceId);
	pDevice = &pCore->Ports[portNum].Device[deviceNum];

	LIST_FOR_EACH(pPos, &pCore->Waiting_List) {
		count++;
	}

	if (count!=0){
		MV_LIST_HEAD_INIT(&remove_List);
	}

	/* 
	 * If returnOSRequest is MV_FALSE, actually we just remove the 
	 * internal reset command. 
	 */
	while ( count>0 )
	{
		pReq = (PMV_Request)List_GetFirstEntry(&pCore->Waiting_List, MV_Request, Queue_Pointer);

		if ( pReq->Device_Id==deviceId )
		{
			/* 
			 * TBD: should make change to the 
			 * mvRemovePortWaitingList too.
			 */
			if ( !Core_IsInternalRequest(pCore, pReq) )
			{
				if ( returnOSRequest ) {
					pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
					List_AddTail(&pReq->Queue_Pointer, &remove_List);
					myCount++;
				} else {
					List_AddTail(&pReq->Queue_Pointer, &pCore->Waiting_List);
				}
			}
			else 
			{
				/* Reset command or request sense */
				if ( SCSI_IS_REQUEST_SENSE(pReq->Cdb[0]) )
				{
					MV_ASSERT( pReq->Org_Req!=NULL );
					pReq = (PMV_Request)pReq->Org_Req;
					if ( pReq ) {
						if ( returnOSRequest ) {
							pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
							List_AddTail(&pReq->Queue_Pointer, &remove_List);
							myCount++;
						} else {
							List_AddTail(&pReq->Queue_Pointer, &pCore->Waiting_List);
						}
					}
				} else {
					/* Reset command is removed. */
				}
			}
		}
		else
		{
			List_AddTail(&pReq->Queue_Pointer, &pCore->Waiting_List);
		}
		count--;
	}//end of while

	for (i=0; i<myCount; i++){
		pReq = (PMV_Request)List_GetFirstEntry(&remove_List, MV_Request, Queue_Pointer);
		MV_DASSERT(pReq && (pReq->Scsi_Status==REQ_STATUS_NO_DEVICE));
		CompleteRequest(pCore, pReq, NULL);
	}//end of for
}

void mvRemovePortWaitingList( MV_PVOID This, MV_U8 portId )
{
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;
	PMV_Request pReq;
	List_Head *pPos;
	List_Head remove_List;
	MV_U8 count = 0, myCount=0, i;

	LIST_FOR_EACH(pPos, &pCore->Waiting_List) {
		count++;
	}

	if (count!=0){
		MV_LIST_HEAD_INIT(&remove_List);
	}

	while ( count>0 )
	{
		pReq = (PMV_Request)List_GetFirstEntry(&pCore->Waiting_List, MV_Request, Queue_Pointer);
		if ( PATA_MapPortId(pReq->Device_Id) == portId )
		{
			if ( pReq->Cmd_Initiator==pCore ) {
				if ( SCSI_IS_READ(pReq->Cdb[0]) || SCSI_IS_WRITE(pReq->Cdb[0]) ) {
					/* Command consolidate, should return */
					pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
					List_AddTail(&pReq->Queue_Pointer, &remove_List);
					myCount++;
				} else if ( SCSI_IS_REQUEST_SENSE(pReq->Cdb[0]) ) {
					/* Request sense */
					MV_ASSERT( pReq->Org_Req!=NULL );
					pReq = (PMV_Request)pReq->Org_Req;
					if ( pReq ) {
						pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
						List_AddTail(&pReq->Queue_Pointer, &remove_List);
						myCount++;
					} 
				} else {
					/* Reset command. Ignore. */
				}
			} else {
				pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
				List_AddTail(&pReq->Queue_Pointer, &remove_List);
				myCount++;
			}
		}
		else
		{
			List_AddTail(&pReq->Queue_Pointer, &pCore->Waiting_List);
		}
		count--;
	}//end of while

	for (i=0; i<myCount; i++){
		pReq = (PMV_Request)List_GetFirstEntry(&remove_List, MV_Request, Queue_Pointer);
		MV_DASSERT(pReq && (pReq->Scsi_Status==REQ_STATUS_NO_DEVICE));
		CompleteRequest(pCore, pReq, NULL);
	}//end of for

}

void mvHandleDeviceUnplug (PCore_Driver_Extension pCore, PDomain_Port pPort)
{
	PMV_Request pReq;
	//PDomain_Device pDevice;
	MV_LPVOID portMmio = pPort->Mmio_Base;
	MV_U8 i; //j = 0;
	MV_U32 temp;
	#ifdef RAID_DRIVER
	//MV_PVOID pUpperLayer = HBA_GetModuleExtension(pCore, MODULE_RAID);	//TBD;
	#else
	//MV_PVOID pUpperLayer = HBA_GetModuleExtension(pCore, MODULE_HBA);
	#endif

	/*
	 * wait for PORT_SCR_STAT to become non-zero(for a faked plug-out irq)
	 * 
	 */
/*	HBA_SleepMillisecond(pCore, 50);

	while (j < 95) {
		HBA_SleepMillisecond(pCore, 10);
		if ((MV_REG_READ_DWORD(portMmio, PORT_SCR_STAT) & 0xf))
			break;
		j++;
	} */

	if( !SATA_PortDeviceDetected(pPort) )
	{		
		/* clear the start bit in cmd register, 
		   stop the controller from handling anymore requests */
		temp = MV_REG_READ_DWORD( portMmio, PORT_CMD );
		MV_REG_WRITE_DWORD( portMmio, PORT_CMD, temp & ~MV_BIT(0));

		SATA_PortReportNoDevice( pCore, pPort );
		
		/* Device is gone. Return the Running_Req */
		for ( i=0; i<MAX_SLOT_NUMBER; i++ )
		{
			pReq =  pPort->Running_Req[i];
			if ( pReq !=NULL )
			{
				pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
				CompleteRequestAndSlot(pCore, pPort, pReq, NULL, i);
			}
		}
		
		if( pPort->Type == PORT_TYPE_PM )
		{
			pPort->Setting &= ~PORT_SETTING_PM_FUNCTIONAL;
			pPort->Setting &= ~PORT_SETTING_PM_EXISTING;
		}
	}
}

void sendDummyFIS( PDomain_Port pPort )
{
	MV_U8 tag = Tag_GetOne(&pPort->Tag_Pool);
	PMV_Command_Header header = SATA_GetCommandHeader(pPort, tag);
	PMV_Command_Table pCmdTable = Port_GetCommandTable(pPort, tag);
	PSATA_FIS_REG_H2D pFIS = (PSATA_FIS_REG_H2D)pCmdTable->FIS;
	PCore_Driver_Extension pCore = pPort->Core_Extension;
	MV_LPVOID portMmio = pPort->Mmio_Base;
	MV_U32 old_stat;

	MV_DASSERT( tag == 0 );

	mvDisableIntr(portMmio, old_stat);

	MV_ZeroMemory(header, sizeof(MV_Command_Header));
	MV_ZeroMemory(pCmdTable, sizeof(MV_Command_Table));
	
	header->FIS_Length = 0;
	header->Reset = 0;
	header->PM_Port = 0xE;
	
	header->Table_Address = pPort->Cmd_Table_DMA.low + SATA_CMD_TABLE_SIZE*tag;
	header->Table_Address_High = pPort->Cmd_Table_DMA.high;//TBD
	
	pFIS->FIS_Type = SATA_FIS_TYPE_REG_H2D;
	pFIS->PM_Port = 0;
	pFIS->Control = 0;

	MV_REG_WRITE_DWORD(portMmio, PORT_CMD_ISSUE, 1<<tag);
	MV_REG_READ_DWORD(portMmio, PORT_CMD_ISSUE);	/* flush */

	HBA_SleepMicrosecond(pCore, 10);
	
	Tag_ReleaseOne(&pPort->Tag_Pool, tag);
	mvEnableIntr(portMmio, old_stat);
}

void mvHandleDevicePlugin (PCore_Driver_Extension pCore, PDomain_Port pPort)
{
	PDomain_Device pDevice = &pPort->Device[0];
	MV_LPVOID portMmio = pPort->Mmio_Base;
	MV_U8 i;
	MV_U32 temp;

	if( pCore->Total_Device_Count >= MAX_DEVICE_SUPPORTED )
		return;
	
	/* hardware workaround - send dummy FIS first to clear FIFO */
	temp = MV_REG_READ_DWORD( portMmio, PORT_CMD );
	MV_REG_WRITE_DWORD( portMmio, PORT_CMD, temp & ~MV_BIT(0));
	MV_REG_WRITE_DWORD( portMmio, PORT_CMD, temp | MV_BIT(0));
	Tag_Init( &pPort->Tag_Pool, MAX_TAG_NUMBER );
	sendDummyFIS( pPort );

	// start command handling on this port
	temp = MV_REG_READ_DWORD( portMmio, PORT_CMD );
	MV_REG_WRITE_DWORD( portMmio, PORT_CMD, temp & ~MV_BIT(0));
	MV_REG_WRITE_DWORD( portMmio, PORT_CMD, temp | MV_BIT(0));

	HBA_SleepMillisecond(pCore, 2000);

	// reset the tag stack - to guarantee soft reset is issued at slot 0
	Tag_Init( &pPort->Tag_Pool, MAX_TAG_NUMBER );

	// do software reset
	MV_DPRINT(("Detected device plug-in, doing soft reset\n"));

	/* Always turn the PM bit on - otherwise won't work! */
	temp = MV_REG_READ_DWORD(portMmio, PORT_CMD);					
	MV_REG_WRITE_DWORD(portMmio, PORT_CMD, temp | MV_BIT(17));
	temp=MV_REG_READ_DWORD(portMmio, PORT_CMD);	/* flush */

	if (! (SATA_PortSoftReset( pCore, pPort )) )
		return;

	if( pPort->Type == PORT_TYPE_PM ) 
	{
		/* need to send notifications for all of these devices */
		for (i=0; i<MAX_DEVICE_PER_PORT; i++)
		{
			pDevice = &pPort->Device[i];
			pDevice->Id = (pPort->Id)*MAX_DEVICE_PER_PORT + i;
			pDevice->Need_Notify = MV_TRUE;
			pDevice->State = DEVICE_STATE_IDLE;
			pDevice->Device_Type = 0;
		}

		SATA_InitPM( pPort );
	} 
	else
	{
		/* not a PM - turn off the PM bit in command register */
		temp = MV_REG_READ_DWORD(portMmio, PORT_CMD);					
		MV_REG_WRITE_DWORD(portMmio, PORT_CMD, temp & (~MV_BIT(17)));
		temp=MV_REG_READ_DWORD(portMmio, PORT_CMD);	/* flush */

		if( SATA_PortDeviceDetected(pPort) )
		{
			if ( SATA_PortDeviceReady(pPort) )
			{					
				pDevice->Internal_Req = GetInternalReqFromPool(pCore);
				if( pDevice->Internal_Req == NULL )
				{
					MV_DPRINT(("ERROR: Unable to get an internal request buffer\n"));
					// can't initialize without internal buffer - just set this disk down
					pDevice->Status = DEVICE_STATUS_NO_DEVICE;
					pDevice->State = DEVICE_STATE_INIT_DONE;
				}
				else 
				{
					pDevice->Status = DEVICE_STATUS_EXISTING|DEVICE_STATUS_FUNCTIONAL;
					pDevice->State = DEVICE_STATE_RESET_DONE;
					pPort->Device_Number++;
					pDevice->Id = (pPort->Id)*MAX_DEVICE_PER_PORT;
					pDevice->Need_Notify = MV_TRUE;
				}
				
				#ifndef _OS_BIOS
				mvDeviceStateMachine (pCore, pDevice);
				#endif
			}
		}
	}
}

#ifdef SUPPORT_PM
void mvHandlePMUnplug (PCore_Driver_Extension pCore, PDomain_Device pDevice)
{
	PMV_Request pReq;
	PDomain_Port pPort = pDevice->PPort;
	MV_LPVOID portMmio = pPort->Mmio_Base;
	List_Head *pPos;
	MV_U8 i, count;
	MV_U32 temp, cmdIssue;
	#ifdef RAID_DRIVER
	MV_PVOID pUpperLayer = HBA_GetModuleExtension(pCore, MODULE_RAID);	//TBD;
	#else
	MV_PVOID pUpperLayer = HBA_GetModuleExtension(pCore, MODULE_HBA);
	#endif

	pDevice->Status = DEVICE_STATUS_NO_DEVICE;
	pPort->Device_Number--;
	if( pDevice->Internal_Req != NULL )
	{
		pCore->Total_Device_Count--;
		ReleaseInternalReqToPool( pCore, pDevice->Internal_Req );
		pDevice->Internal_Req = NULL;
	}

	cmdIssue = MV_REG_READ_DWORD( portMmio, PORT_CMD_ISSUE );

	/* toggle the start bit in cmd register */
	temp = MV_REG_READ_DWORD( portMmio, PORT_CMD );
	MV_REG_WRITE_DWORD( portMmio, PORT_CMD, temp & ~MV_BIT(0));
	MV_REG_WRITE_DWORD( portMmio, PORT_CMD, temp | MV_BIT(0));
	HBA_SleepMillisecond( pCore, 100 );

	/* check for requests that are not finished, clear the port,
	 * then resend again */
	for ( i=0; i<MAX_SLOT_NUMBER; i++ )
	{
		pReq = pPort->Running_Req[i];

		if( pReq != NULL )
		{
			if( pReq->Device_Id == pDevice->Id )
			{
				pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
				CompleteRequestAndSlot(pCore, pPort, pReq, NULL, i);
			}
			else if ( cmdIssue & (1<<i) )
			{
				if( PrepareAndSendCommand( pCore, pReq ) == MV_QUEUE_COMMAND_RESULT_SENDTED )
				{
#ifdef SUPPORT_ERROR_HANDLING
#ifdef SUPPORT_TIMER
					/* start timer for error handling */
					if( pDevice->Timer_ID == NO_CURRENT_TIMER )
					{
						// if no timer is running right now
						pDevice->Timer_ID = Timer_AddRequest( pCore, REQUEST_TIME_OUT, Core_ResetChannel, pDevice );
					}
#endif /* SUPPORT_TIMER */
#endif /* SUPPORT_ERROR_HANDLING */
					pDevice->Outstanding_Req++;
				}
				else
					MV_DASSERT(MV_FALSE);		// shouldn't happens
			}
		}
	}

	count = 0;
	LIST_FOR_EACH(pPos, &pCore->Waiting_List) {
		count++;
	}
	while ( count>0 )
	{
		pReq = (PMV_Request)List_GetFirstEntry(&pCore->Waiting_List, MV_Request, Queue_Pointer);

		if ( pReq->Device_Id == pDevice->Id )
		{
			pReq->Scsi_Status = REQ_STATUS_NO_DEVICE;
			CompleteRequest(pCore, pReq, NULL);
		}
		else
		{
			List_AddTail(&pReq->Queue_Pointer, &pCore->Waiting_List);
		}
		count--;
	}

	/* clear x bit */
	mvPMDevReWrReg( pPort, MV_Read_Reg, MV_SATA_PSCR_SERROR_REG_NUM, 0, pDevice->PM_Number, MV_TRUE );
	temp = MV_REG_READ_DWORD( portMmio, PORT_PM_FIS_0 );

	mvPMDevReWrReg( pPort, MV_Write_Reg, MV_SATA_PSCR_SERROR_REG_NUM, temp, pDevice->PM_Number, MV_TRUE);

#ifdef RAID_DRIVER
	RAID_ModuleNotification(pUpperLayer, EVENT_DEVICE_REMOVAL, 
				(MV_PVOID)(&pDevice->Id));
#else
#ifdef _OS_LINUX
	HBA_ModuleNotification(pUpperLayer, 
			       EVENT_DEVICE_REMOVAL, 
			       pDevice->Id);
#else /* _OS_LINUX */
	HBA_ModuleNotification(pUpperLayer, 
			       EVENT_DEVICE_REMOVAL, 
			       (MV_PVOID) (&pDevice->Id));
#endif /* _OS_LINUX */
#endif /* RAID_DRIVER */
}

extern MV_BOOLEAN mvDeviceStateMachine(
	PCore_Driver_Extension pCore,
	PDomain_Device pDevice
	);

void mvHandlePMPlugin (PCore_Driver_Extension pCore, PDomain_Device pDevice)
{
	PDomain_Port pPort = pDevice->PPort;

	if( pCore->Total_Device_Count >= MAX_DEVICE_SUPPORTED )
		return;

	pDevice->Need_Notify = MV_TRUE;
	pDevice->Device_Type = 0;
	HBA_SleepMillisecond(pCore, 1000);
	SATA_InitPMPort( pPort, pDevice->PM_Number );
	mvDeviceStateMachine(pCore, pDevice);
}
#endif	/* #ifdef SUPPORT_PM */
#endif	/* #ifdef SUPPORT_HOT_PLUG */

void SATA_HandleHotplugInterrupt(
	IN PDomain_Port pPort,
	IN MV_U32 intStatus
	)
{
#ifdef SUPPORT_HOT_PLUG
	PDomain_Device pDevice = &pPort->Device[0];
	PCore_Driver_Extension pCore = pPort->Core_Extension;
	MV_U8 i, plugout=0, plugin=0;
	//MV_U32 count = 0;
	MV_U32 temp;
	MV_LPVOID portMmio = pPort->Mmio_Base;
	//List_Head *pPos;
	#ifdef RAID_DRIVER
	//MV_PVOID pUpperLayer = HBA_GetModuleExtension(pCore, MODULE_RAID);	//TBD;
	#else
	//MV_PVOID pUpperLayer = HBA_GetModuleExtension(pCore, MODULE_HBA);
	#endif

	MV_U32 hotPlugDevice = intStatus & PORT_IRQ_PHYRDY;
	MV_U32 hotPlugPM = (intStatus & PORT_IRQ_ASYNC_NOTIF) || (intStatus & PORT_IRQ_SDB_FIS);	
		
#ifdef _OS_LINUX	
	MV_DBG(DMSG_ACDB, "__MV__ Hotplug int status : %x.\n", intStatus);
#endif /* _OS_LINUX */
	intStatus &= ~(PORT_IRQ_D2H_REG_FIS|PORT_IRQ_SDB_FIS|PORT_IRQ_PIO_DONE);

	/* if a hard drive or a PM is plugged in/out of the controller */
	if( hotPlugDevice )
	{
		intStatus &= ~PORT_IRQ_PHYRDY;

		/* use Phy status to determine if this is a plug in/plug out */
		HBA_SleepMillisecond(pCore, 500);
		if ((MV_REG_READ_DWORD(portMmio, PORT_SCR_STAT) & 0xf) == 0)
			plugout = MV_TRUE;
		else
			plugin = MV_TRUE;

		/* following are special cases, so we take care of these first */
		if( plugout )
		{
			if ( (pPort->Type != PORT_TYPE_PM ) && (pDevice->Status & DEVICE_STATUS_EXISTING) &&
			     !(pDevice->Status & DEVICE_STATUS_FUNCTIONAL) )
			{
				// a bad drive was unplugged
				pDevice->Status = DEVICE_STATUS_NO_DEVICE;
				MV_DPRINT(("bad drive was unplugged\n"));
				return;
			}

			if ( (pPort->Setting & PORT_SETTING_PM_EXISTING) && 
			     !(pPort->Setting & PORT_SETTING_PM_FUNCTIONAL) )
			{
				// a bad PM was unplugged
				pPort->Setting &= ~PORT_SETTING_PM_EXISTING;
				MV_DPRINT(("bad PM was unplugged\n"));
				return;
			}
		}
		
		if ( ((pPort->Type == PORT_TYPE_PM) && (pPort->Setting & PORT_SETTING_PM_FUNCTIONAL)) ||
		     ((pPort->Type != PORT_TYPE_PM) && (pDevice->Status & DEVICE_STATUS_FUNCTIONAL)) 
			)
		{
			if( plugout )
				mvHandleDeviceUnplug( pCore, pPort );
		}
		else
		{
			if( plugin )
				mvHandleDevicePlugin( pCore, pPort );
		}
	}
					
	/* if a drive was plugged in/out of a PM */
	if ( hotPlugPM ) 
	{
		intStatus &= ~PORT_IRQ_ASYNC_NOTIF;
		intStatus &= ~PORT_IRQ_SDB_FIS;

		mvPMDevReWrReg( pPort, MV_Read_Reg, MV_SATA_GSCR_ERROR_REG_NUM, 0, 0xF, MV_TRUE );
		temp = MV_REG_READ_DWORD( portMmio, PORT_PM_FIS_0 );

		if (temp == 0)
			return;

		// better solution???
		for (i=0; i<MAX_DEVICE_PER_PORT; i++)	
		{
			if( temp & MV_BIT(i) )
			{
				pDevice = &pPort->Device[i];
				pDevice->PM_Number = i;
				break;
			}
		}
		
		/* make sure it's a hot plug SDB */
		mvPMDevReWrReg( pPort, MV_Read_Reg, MV_SATA_PSCR_SERROR_REG_NUM, 0, pDevice->PM_Number, MV_TRUE );
		temp = MV_REG_READ_DWORD( portMmio, PORT_PM_FIS_0 );

		if( !( (temp & MV_BIT(16)) || (temp & MV_BIT(26)) ) )
			return;

		/* check phy status to determine plug in/plug out */
		HBA_SleepMillisecond(pCore, 500);
		mvPMDevReWrReg(pPort, MV_Read_Reg, MV_SATA_PSCR_SSTATUS_REG_NUM, 0, pDevice->PM_Number, MV_TRUE);
		temp = MV_REG_READ_DWORD( portMmio, PORT_PM_FIS_0 );

		if( (temp & 0xF) == 0 )
			plugout = MV_TRUE;
		else
			plugin = MV_TRUE;

		if ( plugout && (pDevice->Status & DEVICE_STATUS_EXISTING) &&
			 !(pDevice->Status & DEVICE_STATUS_FUNCTIONAL) )
		{
			// a bad drive was unplugged
			pDevice->Status = DEVICE_STATUS_NO_DEVICE;

			/* clear x bit */
			mvPMDevReWrReg( pPort, MV_Read_Reg, MV_SATA_PSCR_SERROR_REG_NUM, 0, pDevice->PM_Number, MV_TRUE );
			temp = MV_REG_READ_DWORD( portMmio, PORT_PM_FIS_0 );

			mvPMDevReWrReg( pPort, MV_Write_Reg, MV_SATA_PSCR_SERROR_REG_NUM, temp, pDevice->PM_Number, MV_TRUE);

			MV_DPRINT(("bad drive was unplugged\n"));
			return;
		}

		if( pDevice->Status & DEVICE_STATUS_FUNCTIONAL )
		{
			if (plugout)
				mvHandlePMUnplug(pCore, pDevice);
		}
		else
		{
			if (plugin)
			mvHandlePMPlugin( pCore, pDevice );
		}
	}

#endif
	if ( intStatus )
	{
#ifndef _OS_BIOS
		MV_PRINT("Error: SATA_HandleHotplugInterrupt port=%d intStatus=0x%x.\n", pPort->Id, intStatus);
#endif
	}
}

void mvCompleteSlots( PDomain_Port pPort, MV_U32 completeSlot, PATA_TaskFile taskFiles )
{
	PCore_Driver_Extension pCore = pPort->Core_Extension;
#ifdef MV_DEBUG
	MV_LPVOID port_mmio = pPort->Mmio_Base;
#endif
	PDomain_Device pDevice;
	PMV_Request pReq = NULL, pOrgReq = NULL;
	MV_U8 slotId;

	/* Complete finished commands. All of them are finished successfully.
	 * There are three situations code will come here.
	 * 1. No error for both NCQ and Non-NCQ.
	 * 2. Under NCQ, some requests are completed successfully. At lease one is not.
	 *	For the error command, by specification, SActive isn't cleared.
	 * 3. Under non-NCQ, since no interrupt coalescing, no succesful request. 
	 *  Hardware will return one request is completed. But software clears it above. */

	for ( slotId=0; slotId<MAX_SLOT_NUMBER; slotId++ )
	{
		if ( !(completeSlot&(1L<<slotId)) )
			continue;

		MV_DASSERT( (MV_REG_READ_DWORD(port_mmio, PORT_CMD_ISSUE)&(1<<slotId))==0 );
		MV_DASSERT( (MV_REG_READ_DWORD(port_mmio, PORT_SCR_ACT)&(1<<slotId))==0 );

		completeSlot &= ~(1L<<slotId);
				
		/* This slot is finished. */
		pReq = pPort->Running_Req[slotId];
		MV_DASSERT( pReq );
		pDevice = &pPort->Device[PATA_MapDeviceId(pReq->Device_Id)];

		if ( pReq->Scsi_Status==REQ_STATUS_RETRY )
		{
			MV_PRINT("Retried request is finished...");
			MV_DumpRequest(pReq, MV_FALSE);
		}
	
#ifndef _OS_BIOS
		if ( Core_IsInternalRequest(pCore, pReq)&&(pReq->Org_Req) )
		{
			/* This internal request is used to request sense. */
			MV_ASSERT( pDevice->Device_Type&DEVICE_TYPE_ATAPI );
			pOrgReq = pReq->Org_Req;
			/* Copy sense from the scratch buffer to the request sense buffer. */
			MV_CopyMemory(
					pOrgReq->Sense_Info_Buffer,
					pReq->Data_Buffer,
					MV_MIN(pOrgReq->Sense_Info_Buffer_Length, pReq->Data_Transfer_Length)
					);
			pOrgReq->Scsi_Status = REQ_STATUS_HAS_SENSE;
			/* remove internal req's timer */
			hba_remove_timer(pReq);
			pReq = pOrgReq;
		}
		else
#endif
		{
			pReq->Scsi_Status = REQ_STATUS_SUCCESS;
		}

		CompleteRequestAndSlot(pCore, pPort, pReq, taskFiles, slotId);
		MV_DUMPC32(0xCCCC4402);
		MV_ENTERLINE;
		//MV_HALTKEY

		if ( completeSlot==0 )
			break;
	}
}

void SATA_PortHandleInterrupt(
	IN PCore_Driver_Extension pCore,
	IN PDomain_Port pPort
	)
{
	PDomain_Device pDevice = &pPort->Device[0];
	MV_LPVOID mmio = pCore->Mmio_Base;
	MV_LPVOID port_mmio = pPort->Mmio_Base;
	MV_U32 orgIntStatus, intStatus, serialError, commandIssue, serialActive, temp;
	PMV_Request pReq = NULL, pOrgReq = NULL;
	MV_U32 completeSlot = 0;
	MV_U8 slotId = 0, i;
	MV_BOOLEAN hasError = MV_FALSE, finalError = MV_FALSE;
	MV_U32 errorSlot = 0;
	ATA_TaskFile	taskFiles;
#ifdef MV_DEBUG
	MV_U32 orgSerialError, orgCommandIssue, orgSerialActive, orgCompleteSlot, orgRunningSlot;
#endif

#ifdef _OS_BIOS	
	MV_U32 temp=2000;
	while ( (completeSlot==0) && (temp>0) )
	{
		HBA_SleepMillisecond(pCore, 1);
		commandIssue = MV_REG_READ_DWORD(port_mmio, PORT_CMD_ISSUE);
		completeSlot = (~commandIssue) & pPort->Running_Slot;
		temp--;
	}
#endif

#ifdef SUPPORT_SCSI_PASSTHROUGH
	readTaskFiles(pPort, pDevice, &taskFiles);
#endif

	/* Read port interrupt status register */
	orgIntStatus = MV_REG_READ_DWORD(port_mmio, PORT_IRQ_STAT);
	intStatus = orgIntStatus;

	MV_DUMPC32(0xCCCC4405);
	MV_DUMPC32(commandIssue);
	MV_DUMPC32(pPort->Running_Slot);
	MV_DUMPC32(0xCCCC4406);
	MV_DUMPC32(intStatus);

#ifndef _OS_BIOS
	if ( pPort->Setting&PORT_SETTING_NCQ_RUNNING )
	{
		serialActive = MV_REG_READ_DWORD(port_mmio, PORT_SCR_ACT);
		completeSlot =  (~serialActive) & pPort->Running_Slot;
	}
	else
	{
		commandIssue = MV_REG_READ_DWORD(port_mmio, PORT_CMD_ISSUE);
		completeSlot = (~commandIssue) & pPort->Running_Slot;
	}
#else
	if (completeSlot == 0)
	{
		MV_DUMPC32(0xCCCC44FF);
		MV_HALTKEY;
		pReq = pPort->Running_Req[slotId];
		CompleteRequestAndSlot(pCore, pPort, pReq, &taskFiles, slotId);
		MV_DUMPC32(0xCCCC4401);
		MV_HALTKEY;
		return;
	}
#endif

#ifdef MV_DEBUG
	orgCommandIssue = commandIssue;
	orgSerialActive = serialActive;
	orgCompleteSlot = completeSlot;
	orgRunningSlot = pPort->Running_Slot;
#endif

	intStatus &= ~(PORT_IRQ_D2H_REG_FIS|PORT_IRQ_SDB_FIS|PORT_IRQ_PIO_DONE);	/* Used to check request is done. */
	intStatus &= ~(PORT_IRQ_DMAS_FIS|PORT_IRQ_PIOS_FIS);						/* Needn't care. */

	/* Error handling */
	if ( 
			(intStatus&PORT_IRQ_TF_ERR)
		||	(intStatus&PORT_IRQ_LINK_RECEIVE_ERROR)
		||	(intStatus&PORT_IRQ_LINK_TRANSMIT_ERROR)
		)
	{
		MV_DUMPC32(0xCCCC4411);
		MV_HALTKEY;

		MV_PRINT("Interrupt Error: 0x%x orgIntStatus: 0x%x completeSlot=0x%x.\n", 
			intStatus, orgIntStatus, completeSlot);
		if (intStatus&PORT_IRQ_TF_ERR)
		{
			/* Don't do error handling when receive link error. 
			 * Wait until we got the Task File Error */

			/* read serial error only when there is error */
			serialError = MV_REG_READ_DWORD(port_mmio, PORT_SCR_ERR);
			MV_REG_WRITE_DWORD(port_mmio, PORT_SCR_ERR, serialError);

			/* Handle serial error interrupt */
			if ( serialError )
			{
				MV_DUMPC32(0xCCCC4405);
				MV_HALTKEY;
				SATA_HandleSerialError(pPort, serialError); 
			}

			MV_DUMPC32(serialError);
#ifdef MV_DEBUG
			orgSerialError = serialError;
#endif

			/* read errorSlot only when there is error */
			errorSlot = MV_REG_READ_DWORD(port_mmio, PORT_CMD);
			MV_DUMPC32(errorSlot);

			hasError = MV_TRUE;
			errorSlot = (errorSlot>>8)&0x1F;

			if ( pPort->Setting&PORT_SETTING_DURING_RETRY )
				finalError = MV_TRUE;
			else
			{
				/* if the error request is any internal requests, we don't retry 
				 *     1) read log ext - don't retry
				 *	   2) any initialization requests such as identify - buffer
				 *		  will conflict when we try to send read log ext to retry
				 *	   3) request sense - included in the ATAPI condition below
				 */
				pReq = pPort->Running_Req[errorSlot];
				if ( pReq != NULL && Core_IsInternalRequest(pCore, pReq) )
					finalError = MV_TRUE;

				/* For ATAPI device, we don't do retry. OS already has done a lot.
				* ATAPI device: one request at a time. */
				if ( completeSlot==((MV_U32)1L<<errorSlot) )
				{
					pReq = pPort->Running_Req[errorSlot];
					MV_ASSERT( pReq!=NULL );
					pDevice = &pPort->Device[PATA_MapDeviceId(pReq->Device_Id)];
					if ( pDevice->Device_Type&DEVICE_TYPE_ATAPI )
						finalError = MV_TRUE;
				}
			}
		}
		intStatus &= ~(PORT_IRQ_TF_ERR|PORT_IRQ_LINK_RECEIVE_ERROR|PORT_IRQ_LINK_TRANSMIT_ERROR);		
	}


	/* Final Error: we give up this error request. Only one request is running. 
	 * And during retry we won't use NCQ command. */
	if ( finalError )
	{
		MV_ASSERT( !(pPort->Setting&PORT_SETTING_NCQ_RUNNING) );
		MV_ASSERT( completeSlot==((MV_U32)1L<<errorSlot) );
		MV_ASSERT( pPort->Running_Slot==completeSlot );

	#ifndef _OS_BIOS
		/* clear global before channel */
		MV_REG_WRITE_DWORD(mmio, HOST_IRQ_STAT, (1L<<pPort->Id));
		MV_REG_WRITE_DWORD(port_mmio, PORT_IRQ_STAT, orgIntStatus);
	#endif /* _OS_BIOS */

		/* This is the failed request. */
		pReq = pPort->Running_Req[errorSlot];
		MV_ASSERT( pReq!=NULL );
		pDevice = &pPort->Device[PATA_MapDeviceId(pReq->Device_Id)];

	#ifndef _OS_BIOS
		if( Core_IsInternalRequest(pCore, pReq) )
		{
			if( pReq->Org_Req )
			{
				/* This internal request is used to request sense. */
				MV_ASSERT( pDevice->Device_Type&DEVICE_TYPE_ATAPI );
				pOrgReq = pReq->Org_Req;
				pOrgReq->Scsi_Status = REQ_STATUS_ERROR;

				/* remove internal req's timer */
				hba_remove_timer(pReq);
				pReq = pOrgReq;
			}
			else if( pReq->Cdb[2] == CDB_CORE_READ_LOG_EXT )
			{
				pReq->Scsi_Status = REQ_STATUS_ERROR;
			}
			else
			{
				/* This internal request is initialization request like identify */
				MV_DASSERT( pDevice->State != DEVICE_STATE_INIT_DONE );
				pReq->Scsi_Status = REQ_STATUS_ERROR;
			}
		}
		else
	#endif /* _OS_BIOS */
		{
			if ( pReq->Cmd_Flag&CMD_FLAG_PACKET )
			{
				pReq->Scsi_Status = REQ_STATUS_REQUEST_SENSE;
			}
			else
			{
			#ifndef _OS_BIOS	
				MV_DPRINT(("Finally SATA error for Req 0x%x.\n", pReq->Cdb[0]));
			#endif
				pReq->Scsi_Status = REQ_STATUS_ERROR;
			}
		}

		CompleteRequestAndSlot(pCore, pPort, pReq, &taskFiles, (MV_U8)errorSlot);

		/* Handle interrupt status register */
		if ( intStatus )
		{
			MV_DUMPC32(0xCCCC4403);
			MV_DUMPC32(intStatus);
			SATA_HandleHotplugInterrupt(pPort, intStatus);
		}
		return;
	}

	/* The first time to hit the error. Under error condition, figure out all the successful requests. */
	if ( hasError )
	{
		MV_ASSERT( !finalError );
		if ( pPort->Setting&PORT_SETTING_NCQ_RUNNING )
		{
			/* For NCQ command, if error happens on one slot.
			 * This slot is not completed. SActive is not cleared. */
		}
		else
		{
			/* For Non-NCQ command, last command is the error command. 
			 * ASIC will stop whenever there is an error.
			 * And we only have one request if there is no interrupt coalescing or NCQ. */
			MV_ASSERT( completeSlot==((MV_U32)1L<<errorSlot) );

			/* The error command is finished but we clear it to make it to be retried. */
			completeSlot=0;
		}
		/* Now all the completed commands are completed successfully. */

		/* Reset this port to prepare for the retry. At least one request will be retried. */

		MV_ASSERT( finalError==MV_FALSE );

		/* Toggle the port start bit to clear up the hardware to prepare for the retry. */
		temp = MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_CMD);
		temp &= ~PORT_CMD_START;
		MV_REG_WRITE_DWORD(pPort->Mmio_Base, PORT_CMD, temp );
		HBA_SleepMillisecond(pCore, 1);
		temp |= PORT_CMD_START;
		MV_REG_WRITE_DWORD(pPort->Mmio_Base, PORT_CMD, temp );
		HBA_SleepMillisecond(pCore, 1);
		MV_PRINT("Toggle CMD register start stop bit at port 0x%x.\n", pPort->Id);

		/* Toggle should before we clear the channel interrupt status but not the global interrupt. */
		MV_REG_WRITE_DWORD(mmio, HOST_IRQ_STAT, (1L<<pPort->Id));
		/* Abort all the others requests and retry. */
		for ( slotId=0; slotId<MAX_SLOT_NUMBER; slotId++ )
		{
			pReq = pPort->Running_Req[slotId];
			if ( !(completeSlot&(1L<<slotId)) && pReq )
			{
				pReq->Cmd_Flag &= 0xFF;	/* Remove NCQ setting. */
				pReq->Scsi_Status = REQ_STATUS_RETRY;

				/* Put requests to the queue head but don't run them. Should run ReadLogExt first. */
				pPort->Running_Req[slotId] = NULL;
				pPort->Running_Slot &= ~(1L<<slotId);
				Tag_ReleaseOne(&pPort->Tag_Pool, slotId);
				hba_remove_timer(pReq);
				List_Add(&pReq->Queue_Pointer, &pCore->Waiting_List);		/* Add to the header. */
				MV_PRINT("Abort error requests....\n");
				MV_DumpRequest(pReq, MV_FALSE);
			}
		}
		MV_ASSERT( MV_REG_READ_DWORD(port_mmio, PORT_CMD_ISSUE)==0 );
		MV_ASSERT( MV_REG_READ_DWORD(port_mmio, PORT_IRQ_STAT)==0 );
		MV_ASSERT( (MV_REG_READ_DWORD(mmio, HOST_IRQ_STAT)&(1L<<pPort->Id))==0 );

		/* Send ReadLogExt command to clear the outstanding commands on the device. 
		 * This request will be put to the queue head because it's Cmd_Initiator is Core Driver. 
		 * Consider the port multiplier. */
		for ( i=0; i<MAX_DEVICE_PER_PORT; i++ )
		{
			pDevice = &pPort->Device[i];
			if ( 
				!(pDevice->Device_Type&DEVICE_TYPE_ATAPI)
				&& (pDevice->Capacity&DEVICE_CAPACITY_READLOGEXT_SUPPORTED)
				//&& (pPort->Setting&PORT_SETTING_NCQ_RUNNING)
				)
			{
				Device_IssueReadLogExt(pPort, pDevice);
			}
			else
			{
				#ifndef _OS_BIOS
				Core_HandleWaitingList(pCore);	//TBD: Should port based.
				#endif
			}
		}

		/* Needn't run interrupt_handle_bottom_half except the hot plug.
		 * Toggle start bit will clear all the interrupt. So don't clear interrupt again. 
		 * Otherwise it'll clear Read Log Ext interrupt. 
		 * If Device_IssueReadLogExt is called, needn't run Core_HandleWaitingList. */
		//TBD: How about the hot plug.
		
		/* handle completed slots */
		if( completeSlot )
			mvCompleteSlots( pPort, completeSlot, &taskFiles );
		return;
	}

	/* if no error */

#ifndef _OS_BIOS
	/* clear global before channel */
	MV_REG_WRITE_DWORD(mmio, HOST_IRQ_STAT, (1L<<pPort->Id));
	MV_REG_WRITE_DWORD(port_mmio, PORT_IRQ_STAT, orgIntStatus);
#endif

	/* handle completed slots */
	if( completeSlot )
		mvCompleteSlots( pPort, completeSlot, &taskFiles );

	/* Handle interrupt status register */
	if ( intStatus )
	{
		MV_DUMPC32(0xCCCC4403);
		MV_DUMPC32(intStatus);
		SATA_HandleHotplugInterrupt(pPort, intStatus);
	}
}

void PATA_PortHandleInterrupt(
	IN PCore_Driver_Extension pCore,
	IN PDomain_Port pPort
	)
{
	MV_LPVOID mmio = pCore->Mmio_Base;
	MV_LPVOID port_mmio = pPort->Mmio_Base;
	MV_U32 intStatus, orgIntStatus, commandIssue, taskFile=0, stateMachine, portCommand;
	MV_U32 temp;
	PMV_Request pReq = NULL, pOrgReq = NULL;
	MV_U32 completeSlot = 0;
	MV_U8 slotId = 0;
	MV_BOOLEAN hasOneAlready = MV_FALSE;
	MV_BOOLEAN hasError = MV_FALSE, needReset = MV_FALSE;
	PDomain_Device pDevice=NULL;
	ATA_TaskFile	taskFiles;
    	
	/* Read port interrupt status register */
	intStatus = MV_REG_READ_DWORD(port_mmio, PORT_IRQ_STAT);
	orgIntStatus = intStatus;
#ifdef _OS_BIOS
	MV_REG_WRITE_DWORD(port_mmio, PORT_IRQ_STAT,intStatus);
	HBA_SleepMillisecond(pCore, 10);
#endif

	MV_DUMPC32(0xCCCCDD01);
	MV_DUMPC32(intStatus);
	/* 
	 * Workaround for PATA non-data command.
	 * PATA non-data command, CI is not ready yet when interrupt is triggered.
	 */
	commandIssue = MV_REG_READ_DWORD(port_mmio, PORT_CMD_ISSUE);
	completeSlot = (~commandIssue) & pPort->Running_Slot;

/* Thor Lite D0 and Thor B0 */
if ( (pCore->Device_Id!=DEVICE_ID_THOR_4S1P_NEW) && (pCore->Revision_Id!=0xB0) && (pCore->Revision_Id!=0xB1) )
{
	temp=1000;
	while ( (completeSlot==0) && (temp>0) )
	{
	#ifndef _OS_BIOS
		HBA_SleepMillisecond(pCore, 2);
	#else
		HBA_SleepMillisecond(pCore, 1);
	#endif
		commandIssue = MV_REG_READ_DWORD(port_mmio, PORT_CMD_ISSUE);
		completeSlot = (~commandIssue) & pPort->Running_Slot;
		temp--;
	}


#ifdef DEBUG_BIOS
	MV_DUMPC32(0xCCCC5501);
	MV_DUMPC32(MV_REG_READ_DWORD(port_mmio, PORT_CMD_ISSUE));
	MV_DUMPC32(pPort->Running_Slot);
	MV_DUMPC32(commandIssue);
#endif


	
	if ( (completeSlot==0)&&(pPort->Running_Slot!=0) )
	{
		MV_DPRINT(("INT but no request completed: 0x%x CI: 0x%x Running: 0x%x\n", 
			intStatus, commandIssue, pPort->Running_Slot));
		/*
		 * Workaround:
		 * If ATAPI read abort happens, got one interrupt but CI is not cleared.
		 */
		stateMachine = MV_REG_READ_DWORD(port_mmio, PORT_INTERNAL_STATE_MACHINE);
		if ( stateMachine==0x60007013 )
		{
            pCore->Need_Reset = 1;
			needReset = MV_TRUE;

			/* Actually one request is finished. We need figure out which one it is. */
			portCommand = MV_REG_READ_DWORD(port_mmio, PORT_CMD);
			MV_ASSERT( portCommand&MV_BIT(15) );	/* Command is still running */
			portCommand = (portCommand>>8)&0x1F;
			MV_ASSERT( portCommand<MAX_SLOT_NUMBER );
			MV_DPRINT(("Read abort happens on slot %d.\n", portCommand));
			completeSlot |= (1<<portCommand);
		}
	}
}
#ifdef _OS_BIOS
	if (completeSlot == 0)
	{
		MV_DUMPC32(0xCCCC55FF);
		MV_HALTKEY;
		pReq = pPort->Running_Req[slotId];
		pPort->Running_Req[slotId] = NULL;
		pPort->Running_Slot &= ~(1L<<slotId);
		Tag_ReleaseOne(&pPort->Tag_Pool, slotId);
		
#ifdef SUPPORT_ERROR_HANDLING
		pDevice = &pPort->Device[PATA_MapDeviceId(pReq->Device_Id)];
		pDevice->Outstanding_Req--;

#ifdef SUPPORT_TIMER
		/* request for this device came back, so we cancel the timer */
		Timer_CancelRequest( pCore, pDevice->Timer_ID );
		pDevice->Timer_ID = NO_CURRENT_TIMER;

		/* if there are more outstanding requests, we send a new timer */
		if ( pDevice->Outstanding_Req > 0 )
		{
			pDevice->Timer_ID = Timer_AddRequest( pCore, REQUEST_TIME_OUT, Core_ResetChannel, pDevice );
		}
#endif /* SUPPORT_TIMER */
#endif /* SUPPORT_ERROR_HANDLING */

		CompleteRequest(pCore, pReq, &taskFiles);
		MV_DUMPC32(0xCCCC550E);
		MV_HALTKEY;
		return;
	}
#endif

	/* Handle interrupt status register */
	intStatus &= ~MV_BIT(0); intStatus &= ~MV_BIT(2);
#ifdef ENABLE_PATA_ERROR_INTERRUPT
	hasError = (intStatus!=0) ? MV_TRUE : MV_FALSE;

	/*
	 * Workaround:
	 * If error interrupt bit is set. We cannot clear it.
	 * Try to use PORT_CMD PORT_CMD_PATA_START bit to clear the error interrupt but didn't work.
	 * So we have to disable PATA error interrupt.
	 */
#endif

	/* Complete finished commands */
	for ( slotId=0; slotId<MAX_SLOT_NUMBER; slotId++ )
	{
		MV_DUMPC32(0xCCCCDD03);
		if ( !(completeSlot&(1L<<slotId)) )
			continue;

		completeSlot &= ~(1L<<slotId);
		MV_DASSERT( completeSlot==0 );	//TBD: If no interrupt coleascing.

		/* This slot is finished. */
		pReq = pPort->Running_Req[slotId];
		MV_DASSERT(pReq);

#if defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX)
		hba_remove_timer(pReq);
#endif /* defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX) */
		pPort->Running_Req[slotId] = NULL;
		pPort->Running_Slot &= ~(1L<<slotId);
		Tag_ReleaseOne(&pPort->Tag_Pool, slotId);
		MV_DASSERT( (MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_CMD_ISSUE)&(1<<slotId))==0 );

		pDevice = &pPort->Device[PATA_MapDeviceId(pReq->Device_Id)];

	#ifndef ENABLE_PATA_ERROR_INTERRUPT
		/* 
 		 * Workaround:
 		 * Sometimes we got error interrupt bit but the status is still 0x50.
		 * In this case, the command is completed without error.
		 * So we have to check the task status to make sure it's really an error or not.
		 */
		#ifndef _OS_BIOS
		HBA_SleepMicrosecond(pCore, 2);
		#else
		HBA_SleepMillisecond(pCore, 2);
		#endif
		if ( !pDevice->Is_Slave )
			taskFile = MV_REG_READ_DWORD(port_mmio, PORT_MASTER_TF0);
		else
			taskFile = MV_REG_READ_DWORD(port_mmio, PORT_SLAVE_TF0);
			MV_DUMPC32(0xCCCCDD04);
			MV_DUMPC32(taskFile);
		if ( taskFile&MV_BIT(0) )
		{
			
			MV_DUMPC32(0xCCCCDD05);
			//MV_HALTKEY;
			hasError = MV_TRUE;
			MV_DPRINT(("PATA request returns with error 0x%x.\n", taskFile));
		}

		#ifdef MV_DEBUG
		if ( !(taskFile&MV_BIT(0)) && ( intStatus ) )
		{
			MV_DPRINT(("Error interrupt is set but status is 0x50.\n"));
		
		}
		#endif
	#endif

		if ( (hasError)&&(pCore->Device_Id!=DEVICE_ID_THOR_4S1P_NEW)&&(pCore->Revision_Id!=0xB0)&&(pCore->Revision_Id!=0xB1) )
		{
			MV_DUMPC32(0xCCCCDD06);
			if ( pDevice->Device_Type==DEVICE_TYPE_ATAPI )
			{
				/*
				 * Workaround: 
				 * Write request if device abort, hardware state machine got wrong.
				 * Need do reset to recover.
				 * If the error register is 0x40, we think the error happens.
			 	 * Suppose this problem only happens on ODD. HDD won't write abort.
 				 */
				MV_DUMPC32(0xCCCCDD07);
				taskFile = taskFile>>24;  /* Get the error register */
				if ( taskFile==0x40 )	//TBD: How come I see many read request got this kind of error?
				{
					pCore->Need_Reset = 1;
					needReset = MV_TRUE;
				}
			}
		}
	#ifndef _OS_BIOS	
		if ( Core_IsInternalRequest(pCore, pReq)&&(pReq->Org_Req) )
		{
			/* This internal request is used to request sense. */
			pOrgReq = pReq->Org_Req;
			if ( hasError )
			{
				MV_ASSERT( hasOneAlready==MV_FALSE );
				hasOneAlready = MV_TRUE;
				pOrgReq->Scsi_Status = REQ_STATUS_ERROR;
			}
			else
			{
				/* Copy sense from the scratch buffer to the request sense buffer. */
				MV_CopyMemory(
						pOrgReq->Sense_Info_Buffer,
						pReq->Data_Buffer,
						MV_MIN(pOrgReq->Sense_Info_Buffer_Length, pReq->Data_Transfer_Length)
						);
				pOrgReq->Scsi_Status = REQ_STATUS_HAS_SENSE;
			}
			pReq = pOrgReq;
		}
		else
	#endif
	
		{
			if ( hasError )
			{
				MV_DUMPC32(0xCCCCDD08);
#ifndef _OS_BIOS
				MV_ASSERT( hasOneAlready==MV_FALSE );
				hasOneAlready = MV_TRUE;

				if ( needReset )
				{
					/* Get sense data using legacy mode or fake a sense data here. */
					PATA_LegacyPollSenseData(pCore, pReq);
					pReq->Scsi_Status = REQ_STATUS_HAS_SENSE;
				}
				else
#endif
				{
					if ( pReq->Cmd_Flag&CMD_FLAG_PACKET )
						pReq->Scsi_Status = REQ_STATUS_REQUEST_SENSE;
					else
						pReq->Scsi_Status = REQ_STATUS_ERROR;
				}
			}
			else
			{
				pReq->Scsi_Status = REQ_STATUS_SUCCESS;
				MV_DUMPC32(0xCCCCDD09);
			}
		}

#ifdef SUPPORT_SCSI_PASSTHROUGH
		readTaskFiles(pPort, pDevice, &taskFiles);
#endif

#ifdef SUPPORT_ERROR_HANDLING
		pDevice->Outstanding_Req--;
#ifdef SUPPORT_TIMER
		/* request for this device came back, so we cancel the timer */
		Timer_CancelRequest( pCore, pDevice->Timer_ID );
		pDevice->Timer_ID = NO_CURRENT_TIMER;

		/* if there are more outstanding requests, we send a new timer */
		if ( pDevice->Outstanding_Req > 0 )
		{
			pDevice->Timer_ID = Timer_AddRequest( pCore, REQUEST_TIME_OUT, Core_ResetChannel, pDevice );
		}
#endif /* SUPPORT_TIMER */
#endif /* SUPPORT_ERROR_HANDLING */

		CompleteRequest(pCore, pReq, &taskFiles);  
		MV_DUMPC32(0xCCCC5502);
		MV_ENTERLINE;
		//MV_HALTKEY


		if ( completeSlot==0 )
			break;
	}

	/* 
	 * Clear the interrupt. It'll re-start the hardware to handle the next slot. 
	 * I clear the interrupt after I've checked the CI register.
	 * Currently we handle one request everytime in case if there is an error I don't know which one it is.
	 */
#ifndef _OS_BIOS
	MV_REG_WRITE_DWORD(mmio, HOST_IRQ_STAT, (1L<<pPort->Id));
#endif
	MV_REG_WRITE_DWORD(port_mmio, PORT_IRQ_STAT, orgIntStatus);

#ifndef _OS_BIOS
	/* If there is more requests on the slot, we have to push back there request. */
	//TBD: Request order can be wrong now.
	if ( needReset )
	{
		for ( slotId=0; slotId<MAX_SLOT_NUMBER; slotId++ )
		{
			pReq = pPort->Running_Req[slotId];
			if ( pReq )
			{
				List_Add(&pReq->Queue_Pointer, &pCore->Waiting_List);
				pPort->Running_Req[slotId] = NULL;
				Tag_ReleaseOne(&pPort->Tag_Pool, slotId);
			}
		}
		pPort->Running_Slot = 0;
	}

#endif
	MV_DUMPC32(0xCCCC5503);
	MV_DUMPC32(MV_REG_READ_DWORD(port_mmio, PORT_IRQ_STAT));
}

#ifndef _OS_BIOS
void Device_MakeRequestSenseRequest(
	IN PCore_Driver_Extension pCore,
	IN PDomain_Device pDevice,
	IN PMV_Request pNewReq,
	IN PMV_Request pOrgReq
	)
{
	PMV_SG_Table pSGTable = &pNewReq->SG_Table;
	//MV_U8 senseSize = SATA_SCRATCH_BUFFER_SIZE;
	MV_U8 senseSize = 18;

/*	MV_ZeroMemory(pNewReq, sizeof(MV_Request)); */
	MV_ZeroMvRequest(pNewReq);

	pNewReq->Device_Id = pDevice->Id;

	pNewReq->Scsi_Status = REQ_STATUS_PENDING;
	pNewReq->Cmd_Initiator = pCore;

	pNewReq->Data_Transfer_Length = senseSize;
	pNewReq->Data_Buffer = pDevice->Scratch_Buffer;

	pNewReq->Org_Req = pOrgReq;

	pNewReq->Cmd_Flag = CMD_FLAG_DATA_IN;
#ifdef USE_DMA_FOR_ALL_PACKET_COMMAND	
	pNewReq->Cmd_Flag |=CMD_FLAG_DMA;
#endif

	pNewReq->Completion = NULL;

	/* Make the SG table. */
	SGTable_Init(pSGTable, 0);
	SGTable_Append(
		pSGTable, 
		pDevice->Scratch_Buffer_DMA.low,
		pDevice->Scratch_Buffer_DMA.high,
		senseSize
		);
	MV_DASSERT( senseSize%2==0 );

	/* Request Sense request */
	pNewReq->Cdb[0]=SCSI_CMD_REQUEST_SENSE;
	pNewReq->Cdb[4]=senseSize;

	/* Fixed sense data format is 18 bytes. */
	MV_ZeroMemory(pNewReq->Data_Buffer, senseSize);
}
#endif

void CompleteRequest(
	IN PCore_Driver_Extension pCore,
	IN PMV_Request pReq,
	IN PATA_TaskFile pTaskFile
	)
{
#ifdef SUPPORT_SCSI_PASSTHROUGH
	PHD_Status pHDStatus;
#endif
	PDomain_Port pPort = &pCore->Ports[PATA_MapPortId(pReq->Device_Id)];
	PDomain_Device pDevice = &pPort->Device[PATA_MapDeviceId(pReq->Device_Id)];
	MV_DUMPC32(0xCCCC6601);
	MV_DUMPC32(pReq->Scsi_Status);

	//TBD: Some of the command, we need read the received FIS like smart command.
#ifdef _OS_BIOS
 #ifdef MV_SUPPORT_ATAPI
	if	((pReq->Scsi_Status != REQ_STATUS_SUCCESS) && (pReq->Cmd_Flag & CMD_FLAG_PACKET))	
		return;		/* ATAPI Command fail, do not call callback for BIOS.*/
 #endif
#else 	/* #ifdef _OS_BIOS */
#ifdef _OS_WINDOWS
	if(pReq->Splited_Count)
	{
		if(pReq->Scsi_Status == REQ_STATUS_SUCCESS)
		{
			MV_U32 sectors;
			MV_LBA lba;
			
			pReq->Splited_Count--;

			lba.value = SCSI_CDB10_GET_LBA(pReq->Cdb) + MV_MAX_TRANSFER_SECTOR;
			sectors = MV_MAX_TRANSFER_SECTOR;
			SCSI_CDB10_SET_LBA(pReq->Cdb, lba.value);
			SCSI_CDB10_SET_SECTOR(pReq->Cdb, sectors);

			pReq->Scsi_Status = REQ_STATUS_PENDING;

			Core_ModuleSendRequest(pCore, pReq);

			return;
		}
		else
			pReq->Splited_Count = 0;
	}
#endif	/* _OS_WINDOWS */

#if (SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX)
	hba_remove_timer(pReq);
#endif /* defined(SUPPORT_ERROR_HANDLING) && defined(_OS_LINUX) */	

	if (pReq->Scsi_Status == REQ_STATUS_REQUEST_SENSE)
	{
		/* Use the internal request to request sense. */
		Device_MakeRequestSenseRequest(pCore, pDevice, pDevice->Internal_Req, pReq);
		/* pReq is linked to the */
		Core_ModuleSendRequest(pCore, pDevice->Internal_Req);

		return;
	}


#ifdef SUPPORT_SCSI_PASSTHROUGH
	if (pTaskFile != NULL)
	
	{
		if (pReq->Scsi_Status == REQ_STATUS_SUCCESS)
		{
			if (pReq->Cdb[0] == SCSI_CMD_MARVELL_SPECIFIC && pReq->Cdb[1] == CDB_CORE_MODULE)
			{
				if (pReq->Cdb[2] == CDB_CORE_DISABLE_WRITE_CACHE)
					pDevice->Setting &= ~DEVICE_SETTING_WRITECACHE_ENABLED;
				else if (pReq->Cdb[2] == CDB_CORE_ENABLE_WRITE_CACHE)
					pDevice->Setting |= DEVICE_SETTING_WRITECACHE_ENABLED;
				else if (pReq->Cdb[2] == CDB_CORE_DISABLE_SMART)
					pDevice->Setting &= ~DEVICE_SETTING_SMART_ENABLED;
				else if (pReq->Cdb[2] == CDB_CORE_ENABLE_SMART)
					pDevice->Setting |= DEVICE_SETTING_SMART_ENABLED;
				else if (pReq->Cdb[2] == CDB_CORE_SMART_RETURN_STATUS)
				{
					pHDStatus = (PHD_Status)pReq->Data_Buffer;
					if (pHDStatus == NULL)
					{
#ifdef SUPPORT_EVENT
					if (pTaskFile->LBA_Mid == 0xF4 && pTaskFile->LBA_High == 0x2C)
							HBA_AddEvent( pCore, EVT_ID_HD_SMART_THRESHOLD_OVER, pDevice->Id, SEVERITY_WARNING, 0, NULL );
#endif
					}
					else
					{
						if (pTaskFile->LBA_Mid == 0xF4 && pTaskFile->LBA_High == 0x2C)
							pHDStatus->SmartThresholdExceeded = MV_TRUE;
						else
							pHDStatus->SmartThresholdExceeded = MV_FALSE;
					}
				}
			}
		}
		else
		{
			if (pReq->Sense_Info_Buffer != NULL)
				((MV_PU8)pReq->Sense_Info_Buffer)[0] = REQ_STATUS_ERROR;
			pReq->Scsi_Status = REQ_STATUS_ERROR_WITH_SENSE;
		}
	}
#endif

	/* Do something if necessary to return back the request. */
	if ( (pReq->Cdb[0]==SCSI_CMD_MARVELL_SPECIFIC) && (pReq->Cdb[1]==CDB_CORE_MODULE) ) 
	{
		if ( pReq->Cdb[2]==CDB_CORE_SHUTDOWN )
		{
			if ( pReq->Device_Id<MAX_DEVICE_NUMBER-1 )
			{
				pReq->Device_Id++;
				pReq->Scsi_Status = REQ_STATUS_PENDING;
				Core_ModuleSendRequest(pCore, pReq);
				return;
			}
			else
			{
				pReq->Scsi_Status = REQ_STATUS_SUCCESS;
			}
		}
	}
#endif /* #ifdef _OS_BIOS */

	pReq->Completion(pReq->Cmd_Initiator, pReq);
}

void CompleteRequestAndSlot(
	IN PCore_Driver_Extension pCore,
	IN PDomain_Port pPort,
	IN PMV_Request pReq,
	IN PATA_TaskFile pTaskFile,
	IN MV_U8 slotId
	)
{
#ifdef SUPPORT_ERROR_HANDLING
	PDomain_Device pDevice = &pPort->Device[PATA_MapDeviceId(pReq->Device_Id)];
#endif		
	pPort->Running_Req[slotId] = NULL;
	pPort->Running_Slot &= ~(1L<<slotId);
	Tag_ReleaseOne(&pPort->Tag_Pool, slotId);
	MV_DASSERT( (MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_CMD_ISSUE)&(1<<slotId))==0 );

	if ( pPort->Type!=PORT_TYPE_PATA )
	{
		MV_DASSERT( (MV_REG_READ_DWORD(pPort->Mmio_Base, PORT_SCR_ACT)&(1<<slotId))==0 );
	}

#ifdef SUPPORT_ERROR_HANDLING
	pDevice->Outstanding_Req--;
#ifdef SUPPORT_TIMER
	/* request for this device came back, so we cancel the timer */
	Timer_CancelRequest( pCore, pDevice->Timer_ID );
	pDevice->Timer_ID = NO_CURRENT_TIMER;

	/* if there are more outstanding requests, we send a new timer */
	if ( pDevice->Outstanding_Req > 0 )
	{
		pDevice->Timer_ID = Timer_AddRequest( pCore, REQUEST_TIME_OUT, Core_ResetChannel, pDevice );
	}
#endif /* SUPPORT_TIMER */
#endif /* SUPPORT_ERROR_HANDLING */
	CompleteRequest(pCore, pReq, pTaskFile);
}

void Port_Monitor(PDomain_Port pPort)
{
#ifndef _OS_BIOS
	MV_U8 i;
	MV_PRINT("Port_Monitor: Running_Slot=0x%x.\n", pPort->Running_Slot);
	
	for ( i=0; i<MAX_SLOT_NUMBER; i++ )
	{
		if ( pPort->Running_Req[i]!=NULL )
			MV_DumpRequest(pPort->Running_Req[i], MV_FALSE);
	}
#endif

}

void Core_ModuleMonitor(MV_PVOID This)
{
#ifndef _OS_BIOS
	PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;
	PMV_Request pReq = NULL;
	PList_Head head = &pCore->Waiting_List;
	PDomain_Port pPort = NULL;
	MV_U8 i;

	//TBD: typeof
	MV_PRINT("Core_ModuleMonitor Waiting_List:\n");
	for (pReq = LIST_ENTRY((head)->next, MV_Request, Queue_Pointer);	\
	     &pReq->Queue_Pointer != (head); 	\
	     pReq = LIST_ENTRY(pReq->Queue_Pointer.next, MV_Request, Queue_Pointer))
	{
		MV_DumpRequest(pReq, MV_FALSE);
	}

	for ( i=0; i<pCore->Port_Num; i++ )
	{
		MV_PRINT("Port[%d]:\n", i);
		pPort = &pCore->Ports[i];
		Port_Monitor(pPort);
	}

#endif	/* #ifndef _OS_BIOS */

}

void Core_ModuleReset(MV_PVOID This)
{
#ifndef _OS_BIOS	
	MV_U32 extensionSize = 0; 

	extensionSize = ( ROUNDING(sizeof(Core_Driver_Extension),8)
#ifdef SUPPORT_CONSOLIDATE
					+ ROUNDING(sizeof(Consolidate_Extension),8) + ROUNDING(sizeof(Consolidate_Device),8)*MAX_DEVICE_NUMBER
#endif
					);
			
	/* Re-initialize all the variables even discard all the requests. */
	Core_ModuleInitialize(This, extensionSize, 32);

	//TBD: Merge with Core_ResetHardware
#if 0
	{
		PCore_Driver_Extension pCore = (PCore_Driver_Extension)This;

		pCore->Adapter_State = ADAPTER_INITIALIZING;
		for ( i=0; i<MAX_PORT_NUMBER; i++ )
		{
			pPort = &pCore->Ports[i];
			pPort->Port_State = PORT_STATE_IDLE;
			for ( j=0; j<MAX_DEVICE_PER_PORT; j++ )
			{
				pDevice = &pPort->Device[j];
				pDevice->State = DEVICE_STATE_IDLE;
			}
		}
	}
#endif 
#endif
}

