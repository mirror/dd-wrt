#include "mv_include.h"

#ifdef SUPPORT_CONSOLIDATE

#include "consolid.h"

#ifdef MV_DEBUG
#define TUNE_CONSOLIDATE
#endif

#ifdef TUNE_CONSOLIDATE
#define CONSOLIDATE_STATISTICS_COUNT	8
static MV_U32 gConsolidateStatistics[CONSOLIDATE_STATISTICS_COUNT];
typedef enum{
	CONSOLIDATE_NOT_READ_WRITE,
	CONSOLIDATE_REQUEST_TOO_BIG,
	CONSOLIDATE_READ_WRITE_DIFFERENT,
	CONSOLIDATE_NO_RUNNING_REQUEST,
	CONSOLIDATE_LESS_THAN_SEQUENTIAL_THRESHOLD,
	CONSOLIDATE_NO_RESOURCE,
	CONSOLIDATE_GOT_PUSHED,
	CONSOLIDATA_RESERVED0
}Consolidate_Statistics_Enum;

void UpdateConsolidateStatistics(Consolidate_Statistics_Enum catogory)
{
	MV_U8 i;
	if ( gConsolidateStatistics[catogory]==0xFFFFFFFF )
	{
		for ( i=0; i<CONSOLIDATE_STATISTICS_COUNT; i++ )
			MV_DPRINT(("Consolidate statistics[%d]=0x%x.\n", i, 
			gConsolidateStatistics[i]));
		MV_ZeroMemory(gConsolidateStatistics, sizeof(MV_U32)*CONSOLIDATE_STATISTICS_COUNT);
	}

	gConsolidateStatistics[catogory]++;
}
#else
#define UpdateConsolidateStatistics(x)
#endif

/* 
 * Instruction: How to plug-in this command consolidate sub module to your own module.
 * 1. Include one .h file which supplies some helper funtions like CONS_GET_EXTENSION
 * 2. Allocate memory resouce for Consolidate_Extension and Consolidate_Device
 * 3. Initialize command consolidate module. 
 * 	Call Consolid_InitializeExtension to initialize Consolidate_Extension
 *	Call Consolid_InitializeDevice for each Consolidate_Device
 * 4. When you request comes call Consolid_ModuleSendRequest
 * 5. At proper time, please push command consolidate module.
 *	Sometimes command consolidate is accumulating requests and hasn't fired this internal request,
 *	if there is nothing running now, just push this internal request out.
 */
#include "core_cons.h"

PMV_Request Consolid_GetInternalRequest(MV_PVOID This);
void Consolid_InitialInternalRequest(MV_PVOID This, PMV_Request, MV_BOOLEAN);

void Consolid_ConsolidateRequest(PConsolidate_Extension, PMV_Request, PMV_Request);
void Consolid_CloseRequest(PConsolidate_Extension, PConsolidate_Device, PMV_Request);

void Consolid_RequestCallBack(MV_PVOID This, PMV_Request pReq);

/*
 * Consolidate sub-module has got a request.
 * Two parameters:
 * This: is the pointer of the command initiator extention pointer.
 * pReq: request
 * Will fire: 
 *		a. one internal request
 *		b. this external request and maybe one holding internal request if exists.
 *		c. NULL if consolidate module holds this request.
 */
void Consolid_ModuleSendRequest(MV_PVOID This, PMV_Request pReq)
{
	PConsolidate_Extension pCons = CONS_GET_EXTENSION(This);
	MV_U16 deviceId = pReq->Device_Id;
	PConsolidate_Device pConsDevice = NULL;
	PMV_Request pInternal = NULL;
	MV_LBA startLBA;
	MV_U32 sectorCount;
#ifdef _OS_BIOS
	ZeroU64(startLBA);
#endif

	if ( deviceId>=MAX_DEVICE_NUMBER )
	{
		goto return_original_req;
	}
	pConsDevice = CONS_GET_DEVICE(This, deviceId);

	/* 
	 * We only handle CDB 10 read/write. 
	 * Otherwise, change the following code which gets the LBA and Sector Count from the CDB. 
	 */
	if ( (pReq->Cdb[0]!=SCSI_CMD_READ_10)&&(pReq->Cdb[0]!=SCSI_CMD_WRITE_10) )
	{
		UpdateConsolidateStatistics(CONSOLIDATE_NOT_READ_WRITE);
		goto return_original_req;
	}

	/* It's read/write request. But is it too big for command consolidate */
	if ( pReq->Data_Transfer_Length>CONS_MAX_EXTERNAL_REQUEST_SIZE )
	{
		UpdateConsolidateStatistics(CONSOLIDATE_REQUEST_TOO_BIG);
		goto return_original_req;
	}

	/* Check whether they are all read requests or write requests. */
	if ( 
		( (pReq->Cdb[0]==SCSI_CMD_READ_10)&&(!pConsDevice->Is_Read) )
		||
		( (pReq->Cdb[0]==SCSI_CMD_WRITE_10)&&(pConsDevice->Is_Read) )
		)
	{
		UpdateConsolidateStatistics(CONSOLIDATE_READ_WRITE_DIFFERENT);
		pConsDevice->Is_Read = (pReq->Cdb[0]==SCSI_CMD_READ_10)?1:0;
		goto return_original_req;
	}

	/* Update the consolidate device statistic including last LBA and sequential counter. */
	U64_SET_VALUE(startLBA, SCSI_CDB10_GET_LBA(pReq->Cdb));
	sectorCount = SCSI_CDB10_GET_SECTOR(pReq->Cdb);
	/* Check whether it's a sequential request. */
	if ( U64_COMPARE_U64(startLBA, pConsDevice->Last_LBA) )
		pConsDevice->Sequential = 0; 
	else
		pConsDevice->Sequential++;	/* When equals, return 0. */

	/* Last_LBA is actually the next expect sequential LBA. */
	pConsDevice->Last_LBA = U64_ADD_U32(startLBA, sectorCount);
	if ( pConsDevice->Sequential>CONS_SEQUENTIAL_MAX )	/* To avoid overflow */
		pConsDevice->Sequential=CONS_SEQUENTIAL_THRESHOLD;

	/* Is there any requests running on this device? If no, by pass. */
	if ( !CONS_DEVICE_IS_BUSY(This, deviceId) )
	{
		UpdateConsolidateStatistics(CONSOLIDATE_NO_RUNNING_REQUEST);
		goto return_original_req;
	}

	/* Do we reach the sequential counter threshold? */
	if ( pConsDevice->Sequential<CONS_SEQUENTIAL_THRESHOLD )
	{
		UpdateConsolidateStatistics(CONSOLIDATE_LESS_THAN_SEQUENTIAL_THRESHOLD);
		goto return_original_req;
	}

	pInternal = pConsDevice->Holding_Request;

	/* Don't accumulate this request too big. */
	if ( pInternal && 
		( (pInternal->Data_Transfer_Length+pReq->Data_Transfer_Length>CONS_MAX_INTERNAL_REQUEST_SIZE)
		  ||
		  (pInternal->SG_Table.Valid_Entry_Count+pReq->SG_Table.Valid_Entry_Count>pInternal->SG_Table.Max_Entry_Count)
		)
	   )
	{
		Consolid_CloseRequest(pCons, pConsDevice, pInternal);
		CONS_SEND_REQUEST(This, pInternal);
		pInternal = NULL;	/* After Consolid_CloseRequest, pConsDevice->Holding_Request==NULL */
	}

	/* Get one internal request if we don't have. */
	if ( pConsDevice->Holding_Request==NULL )
	{
		pConsDevice->Holding_Request = Consolid_GetInternalRequest(This);
	}
	pInternal = pConsDevice->Holding_Request;

	/* We are out of resource. */
	if ( pInternal==NULL )
	{
		UpdateConsolidateStatistics(CONSOLIDATE_NO_RESOURCE);
		goto return_original_req;
	}

	/* Now we should be able to do consolidate requests now. */
	Consolid_ConsolidateRequest(pCons, pInternal, pReq);

	/* Is this internal request bigger enough to fire? */
	if ( pInternal->Data_Transfer_Length>=CONS_MIN_INTERNAL_REQUEST_SIZE )
	{
		Consolid_CloseRequest(pCons, pConsDevice, pInternal);
		CONS_SEND_REQUEST(This, pInternal);
		return;	/* Send this internal request. */
	}
	else
	{
		return;	/* Hold this request. */
	}

return_original_req:
	/* 
	 * To keep the command order, 
	 * if we cannot do the consolidate for pReq but we hold some internal request,
	 * run the internal request and then run the new pReq.
	 */
	if ( pConsDevice && (pConsDevice->Holding_Request) )
	{
		pInternal = pConsDevice->Holding_Request;
		Consolid_CloseRequest(pCons, pConsDevice, pInternal);
		/* After Consolid_CloseRequest, pConsDevice->Holding_Request is NULL. */
		CONS_SEND_REQUEST(This, pInternal);
	}
	CONS_SEND_REQUEST(This, pReq);
	return;
}

PMV_Request Consolid_GetInternalRequest(MV_PVOID This)
{
	PConsolidate_Extension pCons = CONS_GET_EXTENSION(This);
	PMV_Request pReq = NULL;
	if ( !List_Empty(&pCons->Free_Queue) )
		pReq = List_GetFirstEntry(&pCons->Free_Queue, MV_Request, Queue_Pointer);

	/* Let's intialize this request */
	if ( pReq )
		Consolid_InitialInternalRequest(This, pReq, MV_FALSE);

	return pReq;
}

void Consolid_ReleaseInternalRequest(PConsolidate_Extension pCons, PMV_Request pReq)
{
	List_AddTail(&pReq->Queue_Pointer, &pCons->Free_Queue);
}

void Consolid_ConsolidateRequest(
	IN PConsolidate_Extension pCons,
	IN OUT PMV_Request pInternal, 
	IN PMV_Request pExternal
	)
{
	MV_U8 i;
	PMV_Request pAttachedReq=NULL;

	/* So far we only handle SCSI Read 10 and SCSI Write 10 */
	MV_DASSERT( (pExternal->Cdb[0]==SCSI_CMD_READ_10) || (pExternal->Cdb[0]==SCSI_CMD_WRITE_10) );
	pAttachedReq = pInternal->Org_Req;

	if ( pInternal->Data_Transfer_Length==0 )
	{
		/* One external request is attached to that yet. */
		pInternal->Device_Id = pExternal->Device_Id;
		MV_DASSERT( pAttachedReq==NULL );

		pInternal->Org_Req = pExternal;
		MV_LIST_HEAD_INIT( &pExternal->Queue_Pointer );

		pInternal->Cdb[0] = pExternal->Cdb[0];	/* Command type */
		pInternal->Cdb[2] = pExternal->Cdb[2];	/* Start LBA */
		pInternal->Cdb[3] = pExternal->Cdb[3];
		pInternal->Cdb[4] = pExternal->Cdb[4];
		pInternal->Cdb[5] = pExternal->Cdb[5];

		if ( pExternal->Cdb[0]==SCSI_CMD_READ_10 )
		{
			pInternal->Cmd_Flag = CMD_FLAG_DMA | CMD_FLAG_DATA_IN;
		}
		else
		{
			pInternal->Cmd_Flag = CMD_FLAG_DMA;
		}
	}
	else
	{
		MV_DASSERT( pInternal->Device_Id==pExternal->Device_Id );
		MV_DASSERT( pAttachedReq!=NULL );
		List_AddTail(&pExternal->Queue_Pointer, &pAttachedReq->Queue_Pointer);
	}

	/* Don't set the sector count every time. Just before send, set the count. */
	pInternal->Data_Transfer_Length += pExternal->Data_Transfer_Length;

	for ( i=0; i<pExternal->SG_Table.Valid_Entry_Count; i++ )
	{
		SGTable_Append(&pInternal->SG_Table, 
			pExternal->SG_Table.Entry_Ptr[i].Base_Address,
			pExternal->SG_Table.Entry_Ptr[i].Base_Address_High,
			pExternal->SG_Table.Entry_Ptr[i].Size);
	}
}

void Consolid_CloseRequest(
	IN PConsolidate_Extension pCons,
	IN PConsolidate_Device pConsDevice,
	IN OUT PMV_Request pInternal
	)
{
	/* 
	 * This internal request is ready for handling now. 
	 * Do whatever we need do before we send this request.
	 */
	MV_U32 sectorCount = pInternal->Data_Transfer_Length/512;	//TBD
	MV_DASSERT(  pInternal->Data_Transfer_Length%512==0 );

	SCSI_CDB10_SET_SECTOR(pInternal->Cdb, sectorCount);
	pConsDevice->Holding_Request = NULL;
}

void Consolid_RequestCallBack(MV_PVOID This, PMV_Request pReq)
{
	PConsolidate_Extension pCons = CONS_GET_EXTENSION(This);
	PConsolidate_Device pConsDevice = CONS_GET_DEVICE(This, pReq->Device_Id);
	PMV_Request pExternal;
	PMV_Request pAttachedReq = pReq->Org_Req;

	if ( pReq->Scsi_Status==REQ_STATUS_SUCCESS )
	{
		/* Extract all the external requests. Update status and return. */
		while ( !List_Empty(&pAttachedReq->Queue_Pointer) )
		{
			pExternal = List_GetFirstEntry(&pAttachedReq->Queue_Pointer, MV_Request, Queue_Pointer);
			pExternal->Scsi_Status = REQ_STATUS_SUCCESS;
			pExternal->Completion(pExternal->Cmd_Initiator, pExternal);
		}
		pAttachedReq->Scsi_Status = REQ_STATUS_SUCCESS;
		pAttachedReq->Completion(pAttachedReq->Cmd_Initiator, pAttachedReq);
	}
	else
	{
		/* Make sure we won't do consolidate again for these requests. */		
		pConsDevice->Sequential = 0;
		MV_DPRINT(("Request error in consolidate.\n"));

		/* If consolidate request has error, Re-send these original requests.
		 * They go to the hardware directly. Bypass the consolidate module. */
		while ( !List_Empty(&pAttachedReq->Queue_Pointer) )
		{
			pExternal = List_GetFirstEntry(&pAttachedReq->Queue_Pointer, MV_Request, Queue_Pointer);
			CONS_SEND_REQUEST(This, pExternal);
		}
		CONS_SEND_REQUEST(This, pAttachedReq);
	}

	/* Release this request back to the pool. */
	Consolid_ReleaseInternalRequest(pCons, pReq);
}

/* Initialize the command consolidate internal request. */
void Consolid_InitialInternalRequest(
	IN MV_PVOID This,
	IN OUT PMV_Request pInternal,
	IN MV_BOOLEAN firstTime
	)
{
	/*
	 * Link pointer: 
	 * When request is free, Queue_Pointer is linked together in the request pool queue.
	 * When request is in use, Org_Req is pointer to the first external request.
	 * This first external request uses Queue_Pointer to link other external requests.
	 * We cannot use internal request's Queue_Pointer to link external requests.
	 * Because after sendting to core driver, this pointer will be destroyed.
	 */
	pInternal->Org_Req = NULL;				/* Use Queue_Pointer as the linker */
	pInternal->Req_Flag = 0;
	pInternal->Scsi_Status = REQ_STATUS_PENDING;
	pInternal->Data_Transfer_Length = 0;
	pInternal->Cmd_Flag = 0;
	SGTable_Init(&pInternal->SG_Table, 0);

	/* 
	 * Some variables only need initialization once. 
	 * It won't change no matter during the life time. 
	 */
	if ( firstTime )
	{
		pInternal->Device_Id = 0;
		pInternal->Tag = 0;						/* Haven't used. */
		pInternal->Cmd_Initiator = This;
		pInternal->Sense_Info_Buffer_Length = 0;
		pInternal->Sense_Info_Buffer = NULL;
		pInternal->Data_Buffer = NULL;			/* After consolidate, virtual address is not valid. */
		pInternal->Context = NULL;
		pInternal->Completion = Consolid_RequestCallBack;
		MV_LIST_HEAD_INIT(&pInternal->Queue_Pointer);
		MV_ZeroMemory(pInternal->Cdb, MAX_CDB_SIZE);
		U64_SET_VALUE(pInternal->LBA, 0);
		pInternal->Sector_Count = 0;
	}
}

/* Initialize the Consolidate_Extension */
void Consolid_InitializeExtension(MV_PVOID This)
{
	PConsolidate_Extension pCons = CONS_GET_EXTENSION(This);
	//PConsolidate_Device pConsDevice;
	PMV_Request pReq;
	MV_U32 i;

	MV_LIST_HEAD_INIT(&pCons->Free_Queue);
	for ( i=0; i<CONS_MAX_INTERNAL_REQUEST_COUNT; i++ )
	{
		pReq = &pCons->Requests[i];

		Consolid_InitialInternalRequest(This, pReq, MV_TRUE);
		List_AddTail(&pReq->Queue_Pointer, &pCons->Free_Queue);
	}

	//MV_ASSERT( CONS_SEQUENTIAL_THRESHOLD>MAX_REQUEST_NUMBER );//TBD
}

/* 
 * Initialize the Consolidate_Device.
 * I don't initialize all the devices at once.
 * Caller should call device one by one.
 * So in this way, consolidate module doesn't all the Consolidate_Device are together
 * or they are embedded in some caller data structure.
 * One more advantage is that caller itself can map the Device_Id to related Consolidate_Device buffer.
 * We don't need contiguous Device_Id.
 */
void Consolid_InitializeDevice(MV_PVOID This, MV_U16 Device_Id)
{
	PConsolidate_Device pConsDevice = CONS_GET_DEVICE(This, Device_Id);

	MV_ZeroMemory(pConsDevice, sizeof(Consolidate_Device));
}


/*
 * Caller pushes us to fire the holding request if any.
 */
void
Consolid_PushFireRequest(
	MV_PVOID This,
	MV_U16 Device_Id
	)
{
	PConsolidate_Extension pCons = CONS_GET_EXTENSION(This);
	PConsolidate_Device pConsDevice = CONS_GET_DEVICE(This, Device_Id);
	PMV_Request pInternal = pConsDevice->Holding_Request;

	if ( pInternal==NULL ) return;

	UpdateConsolidateStatistics(CONSOLIDATE_GOT_PUSHED);

	Consolid_CloseRequest(pCons, pConsDevice, pInternal);
	/* After Consolid_CloseRequest pConsDevice->Holding_Request is NULL. */
	CONS_SEND_REQUEST(This, pInternal);
}
#endif

