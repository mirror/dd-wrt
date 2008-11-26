#include "mv_include.h"
#include "mv_os.h"

#include "hba_header.h"

#include "linux_main.h"
#include "linux_sense.h"
#include "linux_helper.h"

void GenerateSGTable(
	IN PHBA_Extension pHBA,
	IN struct scsi_cmnd *SCpnt,
	OUT PMV_SG_Table pSGTable
	);
void HBARequestCallback(
	MV_PVOID This,
	PMV_Request pReq
	);

void __hba_dump_req_info(PMV_Request preq)
{
	unsigned long lba =0;

	switch (preq->Cdb[0]) {
	case SCSI_CMD_READ_10:
	case SCSI_CMD_WRITE_10:
		lba = preq->Cdb[2]<<24 | preq->Cdb[3]<<16 | preq->Cdb[4]<<8 | \
			preq->Cdb[5];
		break;
	default:
		lba = 0;
		break;
	} 

	MV_DBG(DMSG_PROF_FREQ, 
	       "_MV_ req "RED("%p")
	       " dev %d : cmd %2X : lba %lu - %lu : length %d.\n",
	       preq, preq->Device_Id, preq->Cdb[0], lba,
	       lba + preq->Data_Transfer_Length/512,
	       preq->Data_Transfer_Length);
}

MV_BOOLEAN TranslateSCSIRequest(PHBA_Extension pHBA, struct scsi_cmnd *pSCmd, PMV_Request pReq)
{
	
	pReq->Device_Id = mv_scmd_target(pSCmd);	

	/* Cmd_Flag */	//TBD: For Linux: Is that possible to set these flags or need read the Cdb
	pReq->Cmd_Flag = 0;

	/*
	 * Set three flags: CMD_FLAG_NON_DATA, CMD_FLAG_DATA_IN and CMD_FLAG_DMA
	 */
	if ( pSCmd->request_bufflen==0 ) //TBD lily
	{
		pReq->Cmd_Flag |= CMD_FLAG_NON_DATA;
	}
	else
	{
		//if ( Srb->SrbFlags&SRB_FLAGS_DATA_IN )
		//	pReq->Cmd_Flag |= CMD_FLAG_DATA_IN; TBD ?? Lily
		/*We need to optimize the flags setting. Lily*/
		if(SCSI_IS_READ(pSCmd->cmnd[0]))
			pReq->Cmd_Flag |= CMD_FLAG_DATA_IN; /*NOTE!possible to result in ERROR */
		if ( SCSI_IS_READ(pSCmd->cmnd[0]) || SCSI_IS_WRITE(pSCmd->cmnd[0]) )
			pReq->Cmd_Flag |= CMD_FLAG_DMA;
	}

	pReq->Sense_Info_Buffer_Length = SCSI_SENSE_BUFFERSIZE;  //TBD
	pReq->Data_Transfer_Length = pSCmd->request_bufflen;

	//To handle some special CMDs,lily
	memset(pReq->Cdb, 0, MAX_CDB_SIZE);
	
	switch (pSCmd->sc_data_direction) {
	case DMA_FROM_DEVICE:
		pReq->Cmd_Flag = CMD_FLAG_DATA_IN | CMD_FLAG_DMA;
		break;
	default:
		break;
	}

	switch(pSCmd->cmnd[0]){
	case READ_TOC:
		pReq->Cdb[0] = READ_TOC;
		pReq->Cdb[1] = pSCmd->cmnd[1];
		pReq->Cdb[2] = pSCmd->cmnd[2];
		pReq->Cdb[6] = pSCmd->cmnd[6];
		pReq->Cdb[7] = pSCmd->cmnd[7];
		pReq->Cdb[8] = pSCmd->cmnd[8];
		break;
	case REQUEST_SENSE:
		break;
	case MODE_SELECT:
		pReq->Cdb[0] = MODE_SELECT_10;
		pReq->Cdb[1] = pSCmd->cmnd[1];
		pReq->Cdb[8] = pSCmd->cmnd[4];
		break;
		
	case FORMAT_UNIT:
		pReq->Cdb[0] = 0x24; //ATAPI opcodes
		break;
		
	case READ_CAPACITY: //TBD
		pReq->Cdb[0] = pSCmd->cmnd[0];
		break;

	case TEST_UNIT_READY:                       //TBD
		pReq->Cdb[0] = pSCmd->cmnd[0];
		break;

	case READ_6:
		pReq->Cdb[0] = READ_10;
		pReq->Cdb[3] = pSCmd->cmnd[1]&0x1f;
		pReq->Cdb[4] = pSCmd->cmnd[2];
		pReq->Cdb[5] = pSCmd->cmnd[3];
		pReq->Cdb[8] = pSCmd->cmnd[4];
		pReq->Cdb[9] = pSCmd->cmnd[5];
		break;

	case WRITE_6:
		pReq->Cdb[0] = WRITE_10;
		pReq->Cdb[3] = pSCmd->cmnd[1]&0x1f;
		pReq->Cdb[4] = pSCmd->cmnd[2];
		pReq->Cdb[5] = pSCmd->cmnd[3];
		pReq->Cdb[8] = pSCmd->cmnd[4];
		pReq->Cdb[9] = pSCmd->cmnd[5];
		break;
#if 0
	case READ_12:
		pReq->Cdb[0] = READ_10;
		pReq->Cdb[1] = pSCmd->cmnd[1];
		pReq->Cdb[2] = pSCmd->cmnd[2];
		pReq->Cdb[3] = pSCmd->cmnd[3];
		pReq->Cdb[4] = pSCmd->cmnd[4];
		pReq->Cdb[5] = pSCmd->cmnd[5];
		pReq->Cdb[7] = pSCmd->cmnd[8];
		pReq->Cdb[8] = pSCmd->cmnd[9];
		pReq->Cdb[9] = pSCmd->cmnd[11];
		break;

	case WRITE_12:
		pReq->Cdb[0] = WRITE_10;
		pReq->Cdb[1] = pSCmd->cmnd[1];
		pReq->Cdb[2] = pSCmd->cmnd[2];
		pReq->Cdb[3] = pSCmd->cmnd[3];
		pReq->Cdb[4] = pSCmd->cmnd[4];
		pReq->Cdb[5] = pSCmd->cmnd[5];
		pReq->Cdb[7] = pSCmd->cmnd[8];
		pReq->Cdb[8] = pSCmd->cmnd[9];
		pReq->Cdb[9] = pSCmd->cmnd[11];
		break;
#endif
	default:
		memcpy(pReq->Cdb, pSCmd->cmnd, MAX_CDB_SIZE);
		break;
	}
	
	if (SCSI_IS_INSTANT(pSCmd->cmnd[0]) && pSCmd->use_sg) {
		struct scatterlist *sg = (struct scatterlist *) pSCmd->request_buffer;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14)
		if ( pSCmd->use_sg > 1 )
			MV_DBG(DMSG_SCSI, 
			       "_MV_ more than 1 sg list in instant cmd.\n");
		pReq->Data_Buffer = kmalloc(sg->length, GFP_ATOMIC);
		if (pReq->Data_Buffer) {
			memset(pReq->Data_Buffer, 0, sg->length);
		}
#else
		pReq->Data_Buffer = kzalloc(sg->length, GFP_ATOMIC);
#endif /* 2.6.14 */
		if ( NULL == pReq->Data_Buffer )
			return MV_FALSE;

		pReq->Data_Transfer_Length = sg->length;
		MV_SCp(pSCmd)->map_atomic = 1;
//#elif //TBD
#endif /* 2.5.0  */
	} else {
		pReq->Data_Buffer = pSCmd->request_buffer;
	}

	pReq->Sense_Info_Buffer = pSCmd->sense_buffer;

	/* Init the SG table first no matter it's data command or non-data command. */
	SGTable_Init(&pReq->SG_Table, 0);
	if ( pSCmd->request_bufflen )
	{
		GenerateSGTable(pHBA, pSCmd, &pReq->SG_Table);
	}

	pReq->Org_Req = pSCmd;
	pReq->Context = NULL;

	pReq->LBA.value = 0;
	pReq->Sector_Count = 0;

	pReq->Tag = pSCmd->tag; 
	pReq->Scsi_Status = REQ_STATUS_PENDING;

	pReq->Completion = HBARequestCallback;

#ifdef __AC_REQ_TRACE__
	MV_DBG(DMSG_PROF_FREQ, "_MV_ OS REQ : ");
	__hba_dump_req_info(pReq);
#endif /* __AC_REQ_TRACE__ */
	return MV_TRUE;
}

MV_BOOLEAN TranslateOSRequest(
	IN PHBA_Extension pHBA,
	IN struct scsi_cmnd * pSCmd,
	OUT PMV_Request pReq
	)
{
	pReq->Cmd_Initiator = pHBA; //TODO
	pReq->Org_Req = pSCmd;
	pReq->Scsi_Status = REQ_STATUS_INVALID_REQUEST; //TBD??

 	return TranslateSCSIRequest(pHBA, pSCmd, pReq);
}

/* This is the only function that OS request can be returned. */
void HBARequestCallback(
	MV_PVOID This,
	PMV_Request pReq
	)
{
	PHBA_Extension pHBA = (PHBA_Extension)This;
	struct scsi_cmnd *pSCmd = (struct scsi_cmnd *)pReq->Org_Req;

	/* Return this request to OS. */
	HBA_Translate_Req_Status_To_OS_Status(pHBA, pSCmd, pReq);
	
	List_Add(&pReq->Queue_Pointer, &pHBA->Free_Request);
	pHBA->Io_Count--;
}

void GenerateSGTable(
	IN PHBA_Extension pHBA,
	IN struct scsi_cmnd *SCpnt,
	OUT PMV_SG_Table pSGTable
	)
{
	struct scatterlist *sg = (struct scatterlist *)SCpnt->request_buffer;
	unsigned int sg_count = 0;
	BUS_ADDRESS busaddr = 0;
	int i;

	MV_DBG(DMSG_FREQ,
	       "In GenerateSGTable.\n");

	if (SCpnt->request_bufflen > (mv_scmd_host(SCpnt)->max_sectors << 9)) {
		MV_DBG(DMSG_SCSI, "ERROR: request length exceeds "
		       "the maximum alowed value, %x %x\n",
		       pHBA->Device_Id, pHBA->Revision_Id);
	}

	if (SCpnt->use_sg) {
		unsigned int length;
		sg = (struct scatterlist *) SCpnt->request_buffer;
		if (MV_SCp(SCpnt)->mapped == 0) {
			MV_DBG(DMSG_FREQ,"__MV__ call pci_map_sg.\n");
			sg_count = pci_map_sg(pHBA->pcidev, 
					      sg,
					      SCpnt->use_sg,
					      scsi_to_pci_dma_dir(SCpnt->sc_data_direction));
			if (sg_count != SCpnt->use_sg) {
				MV_PRINT("WARNING sg_count(%d) != "
					 "SCpnt->use_sg(%d)\n",
					 (unsigned int) sg_count, 
					 SCpnt->use_sg);
			}
			MV_SCp(SCpnt)->mapped = 1;
		}

		for (i = 0; i < sg_count; i++) {
			busaddr = sg_dma_address(&sg[i]);
			length = sg_dma_len(&sg[i]);
			
			SGTable_Append( pSGTable, 
					LO_BUSADDR(busaddr), 
					HI_BUSADDR(busaddr),
					length );
		}
	} else {
		if (MV_SCp(SCpnt)->mapped == 0) {
			MV_DBG(DMSG_SCSI_FREQ, 
			       "_MV_ pci_map_single for scmd.\n");

			busaddr = pci_map_single(pHBA->pcidev,
						 SCpnt->request_buffer,
						 SCpnt->request_bufflen,
						 scsi_to_pci_dma_dir(SCpnt->sc_data_direction));
			MV_SCp(SCpnt)->bus_address = busaddr;

			MV_SCp(SCpnt)->mapped = 1;
		}

		SGTable_Append( pSGTable, 
				LO_BUSADDR(busaddr), 
				HI_BUSADDR(busaddr),
				SCpnt->request_bufflen);
	}
}

/*need to be optimized lily*/
void HBA_kunmap_sg(void* pReq)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	void *buf;
	struct scsi_cmnd *scmd = NULL;
	struct scatterlist *sg = NULL;
	PMV_Request        req = NULL;

	req  = (PMV_Request) pReq;
	scmd = (struct scsi_cmnd *) req->Org_Req;

	if (scmd)
		sg = (struct scatterlist *) scmd->request_buffer;

	if (NULL == sg) {
		MV_DBG(DMSG_HBA, "no org_req found in the req.\n");
		return;
	}
		
	if (MV_SCp(scmd)->map_atomic) {
		WARN_ON(!irqs_disabled());
		buf = kmap_atomic(sg->page, KM_IRQ0) + sg->offset;
		memcpy(buf, req->Data_Buffer, sg->length);
		kunmap_atomic(buf, KM_IRQ0);
		kfree(req->Data_Buffer);
		/* other process might want access to it ... */
		req->Data_Buffer = scmd->request_buffer;
		MV_SCp(scmd)->map_atomic = 0;
	}
#endif	
}

static void hba_shutdown_req_cb(MV_PVOID this, PMV_Request req)
{
	PHBA_Extension phba = (PHBA_Extension) this;

	List_Add(&req->Queue_Pointer, &phba->Free_Request);
	phba->Io_Count--;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	atomic_set(&phba->hba_sync, 0);
#else
	complete(&phba->cmpl);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
/* will wait for atomic value atomic to become zero until timed out */
/* return how much 'timeout' is left or 0 if already timed out */
int __hba_wait_for_atomic_timeout(atomic_t *atomic, unsigned long timeout)
{
	unsigned intv = HZ/20; 

	while (timeout) {
		if ( 0 == atomic_read(atomic) )
			break;

		if ( timeout < intv )
			intv = timeout;
		set_current_state(TASK_INTERRUPTIBLE);
		timeout -= (intv - schedule_timeout(intv));
	}
	return timeout;
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */

void hba_send_shutdown_req(PHBA_Extension phba)
{
	unsigned long flags;
	PMV_Request pReq;

	/*Send MV_REQUEST to do something.*/	
	pReq = kmalloc(sizeof(MV_Request), GFP_ATOMIC);

	/* should we reserve a req for this ? */
	if ( NULL == pReq ) {
		printk("THOR : cannot allocate memory for req.\n");
		return;
	}

	pReq->Cmd_Initiator = phba;
	pReq->Org_Req = pReq; /*no ideas.*/
	pReq->Scsi_Status = REQ_STATUS_INVALID_REQUEST;
	pReq->Completion = hba_shutdown_req_cb;
	
#ifdef RAID_DRIVER
	pReq->Cdb[0] = APICDB0_LD;
	pReq->Cdb[1] = APICDB1_LD_SHUTDOWN;
#else
	pReq->Device_Id = 0;
	pReq->Cmd_Flag = 0;
	pReq->Cmd_Flag |= CMD_FLAG_NON_DATA;
	pReq->Sense_Info_Buffer_Length = 0;  
	pReq->Data_Transfer_Length = 0;
	pReq->Data_Buffer = NULL;
	pReq->Sense_Info_Buffer = NULL;
	SGTable_Init(&pReq->SG_Table, 0);
	pReq->Cdb[0] = SCSI_CMD_MARVELL_SPECIFIC;
	pReq->Cdb[1] = CDB_CORE_MODULE;
	pReq->Cdb[2] = CDB_CORE_SHUTDOWN;
	pReq->Context = NULL;
	pReq->LBA.value = 0;
	pReq->Sector_Count = 0;
	pReq->Scsi_Status = REQ_STATUS_PENDING;
#endif

	spin_lock_irqsave(&phba->lock, flags);
	List_AddTail(&pReq->Queue_Pointer, &phba->Waiting_Request);
	phba->Io_Count++;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	atomic_set(&phba->hba_sync, 1);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
	HBA_HandleWaitingList(phba);
	spin_unlock_irqrestore(&phba->lock, flags);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
	__hba_wait_for_atomic_timeout(&phba->hba_sync, 10*HZ);
#else
	wait_for_completion_timeout(&phba->cmpl, 10*HZ);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */
}

MV_BOOLEAN HBA_CanHandleRequest (PMV_Request pReq)
{
	switch ( pReq->Cdb[0] )
	{
		case APICDB0_ADAPTER:
			if ( pReq->Cdb[1] == APICDB1_ADAPTER_GETINFO )
				return MV_TRUE;
			else
				return MV_FALSE;
#ifdef SUPPORT_EVENT
		case APICDB0_EVENT:
			return MV_TRUE;
#endif  /* SUPPORT_EVENT */
		default:
			return MV_FALSE;
	}
}

void HBA_HandleWaitingList(PHBA_Extension pHBA)
{
	PMV_Request pReq = NULL;
	MV_PVOID pNextExtension = NULL;
	MV_VOID (*pNextFunction)(MV_PVOID , PMV_Request) = NULL;

	/* Get the request header */
	while ( !List_Empty(&pHBA->Waiting_Request) ) {
		pReq = (PMV_Request)List_GetFirstEntry(&pHBA->Waiting_Request,
						       MV_Request, 
						       Queue_Pointer);
		MV_DASSERT( pReq != NULL );

		if ( NULL == pReq )
			break;
#if 0
		pCore = pHBA->Module_Manage.resource[MODULE_CORE].module_extension;
		//TBD: To the lower module
		module_set[MODULE_CORE].module_sendrequest(pCore, pReq);
#else
		if ( HBA_CanHandleRequest(pReq) ) {
			HBA_ModuleSendRequest( pHBA, pReq );
		} else {
			//TBD: performance
			HBA_GetNextModuleSendFunction(pHBA, 
							&pNextExtension, 
							&pNextFunction);
			MV_DASSERT( pNextExtension!=NULL );
			MV_DASSERT( pNextFunction!=NULL );
			pNextFunction(pNextExtension, pReq);
		}
#endif
	}
}
