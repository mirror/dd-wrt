#include "mv_include.h"
#include "mv_os.h"

#include "hba_header.h"
#include "linux_helper.h"
#ifdef CACHE_MODULE_SUPPORT
#include "cache_mod.h"
#endif
/* For debug purpose only. */
PHBA_Extension gHBA = NULL;

extern Module_Interface module_set[];

/*
 * 
 * Module interface function table
 *
 */
MV_U32 HBA_ModuleGetResourceQuota(enum Resource_Type type, MV_U16 maxIo)
{
	MV_U32 size = 0;

	/* HBA Extension quota */
	if (type == RESOURCE_CACHED_MEMORY) {
		/* Fixed memory */
		size = OFFSET_OF(HBA_Extension, Memory_Pool);
		size = ROUNDING(size, 8);

		/* MV_Request pool */
		/* MV_Request is 64bit aligned. */
		size += maxIo * MV_REQUEST_SIZE;
		
		if (maxIo > 1)
			size += sizeof(MV_SG_Entry) * MAX_SG_ENTRY * maxIo; 
		else 
			size += sizeof(MV_SG_Entry) * MAX_SG_ENTRY_REDUCED * maxIo;

		/* Timer pool */
		size += Timer_GetResourceQuota(maxIo);

		MV_ASSERT(size == ROUNDING(size, 8));

#ifdef SUPPORT_EVENT
		size += sizeof(Driver_Event_Entry) * MAX_EVENTS;
#endif
		MV_ASSERT(size == ROUNDING(size, 8));

		return size;
	}

	/* HBA doesn't need other kind of memory resource. */
	return 0;
}

void HBA_ModuleInitialize(MV_PVOID This, MV_U32 extension_size, MV_U16 max_io)
{
	PHBA_Extension pHBA = (PHBA_Extension)This;
	MV_PTR_INTEGER temp = (MV_PTR_INTEGER)pHBA->Memory_Pool;
	MV_U8 i;
	PMV_Request pReq = NULL;
 	MV_U32 sgt_size, sg_num;

#ifdef SUPPORT_EVENT
	PDriver_Event_Entry pEvent = NULL;
#endif
    
	gHBA = pHBA;

	MV_ASSERT( sizeof(MV_Request)==ROUNDING(sizeof(MV_Request),8) );
	/* 
	 * Initialize data structure however following variables have been set already.
	 *	Device_Extension
	 *	Is_Dump
	 *	Base_Address
	 *	Adapter_Bus_Number and Adapter_Device_Number
	 *	Vendor_Id, Device_Id and Revision_Id
	 */
	pHBA->State = DRIVER_STATUS_IDLE;
	pHBA->Io_Count = 0;
	pHBA->Max_Io = max_io;

	pHBA->Module_Manage.status = 0;

	/* Initialize the free request queue. */
	MV_LIST_HEAD_INIT(&pHBA->Free_Request);
	MV_LIST_HEAD_INIT(&pHBA->Waiting_Request);
	temp = ROUNDING( ((MV_PTR_INTEGER)temp), 8 );

	if (max_io > 1)
                sg_num = MAX_SG_ENTRY;
        else
                sg_num = MAX_SG_ENTRY_REDUCED;
        sgt_size = sizeof(MV_SG_Entry) * sg_num; 

	for ( i=0; i<max_io; i++ )
	{
		pReq = (PMV_Request)temp;
		temp += MV_REQUEST_SIZE;

		/* sg table */
                pReq->SG_Table.Entry_Ptr = (PMV_SG_Entry) temp; 
                pReq->SG_Table.Max_Entry_Count = sg_num; 
                temp += sizeof(MV_SG_Entry) * sg_num;
		List_AddTail(&pReq->Queue_Pointer, &pHBA->Free_Request);
	}	

#ifdef SUPPORT_EVENT
	
	MV_LIST_HEAD_INIT(&pHBA->Stored_Events);
	MV_LIST_HEAD_INIT(&pHBA->Free_Events);
	pHBA->Num_Stored_Events = 0;
	pHBA->SequenceNumber = 0;	// Event sequence number

	MV_ASSERT( sizeof(Driver_Event_Entry)==ROUNDING(sizeof(Driver_Event_Entry),8) );
	temp = ROUNDING( ((MV_PTR_INTEGER)temp), 8 );

	for ( i=0; i<MAX_EVENTS; i++ )
	{
		pEvent = (PDriver_Event_Entry)temp;
		List_AddTail( &pEvent->Queue_Pointer, &pHBA->Free_Events );
		temp += sizeof( Driver_Event_Entry );
	}

#endif

	/* Initialize timer module. */
	Timer_Initialize(&pHBA->Timer_Module, (MV_PU8)temp);

#ifdef SUPPORT_TIMER
	/* kick off the timer */
	Timer_CheckRequest(pHBA->Device_Extension);
#endif
}

void HBA_ModuleStart(MV_PVOID This)
{
	/* There is nothing we need do here. Just finish this function. */
	HBA_ModuleStarted(This);
}

void HBA_ModuleShutdown(MV_PVOID This)
{
	PHBA_Extension pHBA = (PHBA_Extension)This;
	/* Clear the HBA data structure. Reset some variables if necessary. */

	/* At this momment, the outstanding request count should be zero. */
	MV_ASSERT(pHBA->Io_Count == 0);
	
	/* Stop the Timer */
	Timer_Stop(&pHBA->Timer_Module);
}

void HBA_ModuleNotification(MV_PVOID This, 
			    enum Module_Event event, 
			    MV_U32 event_param)
{
#ifdef SUPPORT_HOT_PLUG
	hba_msg_insert(This, event, event_param);
#endif /* SUPPORT_HOT_PLUG */
}

void HBA_ModuleSendRequest(MV_PVOID This, PMV_Request pReq)
{
	PHBA_Extension pHBA = (PHBA_Extension)This;
	switch ( pReq->Cdb[0] )
	{
	case APICDB0_ADAPTER:
		if (pReq->Cdb[1] == APICDB1_ADAPTER_GETINFO)
			mvGetAdapterInfo( pHBA, pReq );
		else
			pReq->Scsi_Status = REQ_STATUS_INVALID_REQUEST;
		break;

#ifdef SUPPORT_EVENT
	case APICDB0_EVENT:
		if (pReq->Cdb[1] == APICDB1_EVENT_GETEVENT)
			mvGetEvent( pHBA, pReq );
		else
			pReq->Scsi_Status = REQ_STATUS_INVALID_REQUEST;
		break;
#endif  /* SUPPORT_EVENT */

	default:
		pReq->Scsi_Status = REQ_STATUS_INVALID_REQUEST;
	}
	pReq->Completion(pReq->Cmd_Initiator, pReq);
}

/* helper functions related to HBA_ModuleSendRequest */
void mvGetAdapterInfo( MV_PVOID This, PMV_Request pReq )
{
	PHBA_Extension pHBA = (PHBA_Extension)This;
	PAdapter_Info pAdInfo;

	/* initialize */
	pAdInfo = (PAdapter_Info)pReq->Data_Buffer;
	MV_ZeroMemory(pAdInfo, sizeof(Adapter_Info));

	/* TBD: some info are missing, will fill in later */

	pAdInfo->DriverVersion.VerMajor = VER_MAJOR;
	pAdInfo->DriverVersion.VerMinor = VER_MINOR;
	pAdInfo->DriverVersion.VerOEM = VER_OEM;
	pAdInfo->DriverVersion.VerBuild = VER_BUILD;

	pAdInfo->SystemIOBusNumber = pHBA->Adapter_Bus_Number;
	pAdInfo->SlotNumber = pHBA->Adapter_Device_Number;
	pAdInfo->VenDevID = pHBA->Vendor_Id;
	pAdInfo->SubVenDevID = pHBA->Device_Id;

	if ( pHBA->Device_Id == DEVICE_ID_THORLITE_2S1P ||
	     pHBA->Device_Id == DEVICE_ID_THORLITE_2S1P_WITH_FLASH )
		pAdInfo->PortCount = 3;
	else if ( pHBA->Device_Id == DEVICE_ID_THORLITE_0S1P )
		pAdInfo->PortCount = 1;
	else
		pAdInfo->PortCount = 5;

	pAdInfo->AlarmSupport = MV_FALSE;
	pAdInfo->MaxBlockPerPD = 8;		/* hardcoded to 8 for now */

	pReq->Scsi_Status = REQ_STATUS_SUCCESS;
}

#ifdef SUPPORT_EVENT

void mvGetEvent( MV_PVOID This, PMV_Request pReq )
{
	PHBA_Extension pHBA = (PHBA_Extension)This;
	PEventRequest pEventReq = (PEventRequest)pReq->Data_Buffer;
	PDriver_Event_Entry pfirst_event;
	MV_U8 count = 0;

	pEventReq->Count = 0;
	
	if ( pHBA->Num_Stored_Events > 0 )
	{	
		MV_DASSERT( !List_Empty(&pHBA->Stored_Events) );
		while (!List_Empty(&pHBA->Stored_Events) && ( count < MAX_EVENTS_RETURNED))
		{
			pfirst_event = List_GetFirstEntry((&pHBA->Stored_Events), Driver_Event_Entry, Queue_Pointer);
			MV_CopyMemory( &pEventReq->Events[count], &pfirst_event->Event, sizeof (DriverEvent));
			pHBA->Num_Stored_Events--;
			List_AddTail( &pfirst_event->Queue_Pointer, &pHBA->Free_Events );
			count++;
		}
		pEventReq->Count = count;
	}

	pReq->Scsi_Status = REQ_STATUS_SUCCESS;
	return;
}

#endif

void HBA_ModuleReset(MV_PVOID extension)
{
	HBA_ModuleInitialize(extension, sizeof(HBA_Extension), 32);//TBD
}

/*
 * 
 * Other exposed functions
 *
 */
extern void HBA_HandleWaitingList(PHBA_Extension pHBA);
/* The extension is the calling module extension. It can be any module extension. */
void HBA_ModuleStarted(MV_PVOID extension)
{
	PHBA_Extension pHBA = (PHBA_Extension)Module_GetHBAExtension(extension);
	PModule_Manage p_module_manage = &pHBA->Module_Manage;
	MV_U16 module_id = Module_GetModuleId(extension);

	/*MV_ASSERT( (module_id>=0) && (module_id<MAX_MODULE_NUMBER) );*/
	MV_ASSERT( module_id<MAX_MODULE_NUMBER );

	p_module_manage->status |= (1<<module_id);

	/* Whether all the modules are started. */
	if ( module_id==0 )
	{
		MV_PRINT("success to init chip\n");
		
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11)
		atomic_set(&pHBA->hba_sync, 0);
#else
		complete(&pHBA->cmpl);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11) */

		/* We are totally ready for requests handling. */
		pHBA->State = DRIVER_STATUS_STARTED;

		/* Trigger request handling */
		HBA_HandleWaitingList(pHBA);

		/* Module 0 is the last module */
		HBA_ModuleNotification(pHBA, EVENT_MODULE_ALL_STARTED, 0);
	}
	else
	{
		/* Start the next module. From the lowerer to the higher. */
		Module_StartAll(p_module_manage, module_id-1);
	}
}

void HBA_GetResource(
	MV_PVOID extension,
	enum Resource_Type type,
	MV_PVOID resource
	)
{
	PHBA_Extension pHBA = (PHBA_Extension)Module_GetHBAExtension(extension);
	PModule_Manage pModuleManage = &pHBA->Module_Manage;
	MV_U16 moduleId = Module_GetModuleId(extension);

	PAssigned_Uncached_Memory pResource = (PAssigned_Uncached_Memory)resource;

	if ( type==RESOURCE_UNCACHED_MEMORY )
	{
		pResource->Physical_Address.value = pModuleManage->resource[moduleId].uncached_physical_address.value;
		pResource->Virtual_Address = pModuleManage->resource[moduleId].uncached_address;
		pResource->Byte_Size = pModuleManage->resource[moduleId].uncached_size;
		return;
	}

	MV_ASSERT(MV_FALSE);
}

void HBA_GetControllerInfor(
	IN MV_PVOID extension,
	OUT PController_Infor pController
	)
{
	PHBA_Extension pHBA = (PHBA_Extension)Module_GetHBAExtension(extension);
	MV_U8 i;
	for ( i=0; i<MAX_BASE_ADDRESS; i++ )
	{
		pController->Base_Address[i] = pHBA->Base_Address[i];
	}
	pController->Vendor_Id = pHBA->Vendor_Id;
	pController->Device_Id = pHBA->Device_Id;
	pController->Revision_Id = pHBA->Revision_Id;
}

void HBA_SleepMillisecond(
	IN MV_PVOID extension,
	IN MV_U32 millisecond
	)
{
	mdelay(millisecond);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0) && defined(__x86_64__)
	touch_nmi_watchdog();

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
	/* can't remember exactly in what version this was introduced. */
	touch_softlockup_watchdog();
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11) */

#endif

}
void HBA_SleepMicrosecond(
	IN MV_PVOID extension,
	IN MV_U32 microseconds
	)
{
	while (microseconds > 1000) {
		udelay(1000);
		microseconds -= 1000;
	}

	udelay(microseconds);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0) && defined(__x86_64__)
	touch_nmi_watchdog();

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
	/* can't remember exactly in what version this was introduced. */
	touch_softlockup_watchdog();
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11) */

#endif
								
}

void HBA_TimerRoutine(unsigned long DeviceExtension) //TO DO ???
{
#ifndef SUPPORT_TIMER
	PHBA_Extension pHBA   = (PHBA_Extension)Module_GetHBAExtension(DeviceExtension);
	PTimer_Module  pTimer = &pHBA->Timer_Module;
	unsigned long  flags;

#ifdef __AC_DBG__
	unsigned long now;
	
	MV_DASSERT( pTimer->routine!=NULL );
	
	now = jiffies;
	spin_lock_irqsave(&pHBA->lock, flags);
	pTimer->routine(pTimer->context);
	spin_unlock_irqrestore(&pHBA->lock, flags);
	MV_DBG(DMSG_ACDB, "Timer routine %p used %lu jiffies.\n", 
	       pTimer->routine, jiffies-now);
	/*dump_stack()*/
#else /* __AC_DBG__ */
	MV_DASSERT( pTimer->routine!=NULL );
	spin_lock_irqsave(pHBA->lock, flags);
	pTimer->routine(pTimer->context);
	spin_unlock_irqrestore(pHBA->lock, flags);
#endif /* __AC_DBG__ */
#endif /* SUPPORT_TIMER */

}

void HBA_RequestTimer(
	IN MV_PVOID extension,
	IN MV_U32 millisecond,
	MV_VOID (*routine) (MV_PVOID)
	)
{
	PHBA_Extension pHBA = (PHBA_Extension)Module_GetHBAExtension(extension);
	PTimer_Module pTimer = &pHBA->Timer_Module;
	u64 jif_offset;
	
	pTimer->routine = routine;
	pTimer->context = extension;

	del_timer(&pHBA->timer);
	pHBA->timer.function = HBA_TimerRoutine;
	pHBA->timer.data = (unsigned long)extension;
	jif_offset = (u64) (millisecond * HZ);
	do_div(jif_offset, 1000);
	pHBA->timer.expires = jiffies + 1 + jif_offset;
	add_timer(&pHBA->timer);
}

MV_VOID HBA_ModuleMonitor(MV_PVOID extension)
{
	PHBA_Extension pHBA = (PHBA_Extension)extension;
	MV_PRINT("HBA: Io_Count=0x%x.\n", pHBA->Io_Count);
}

MV_VOID 
HBA_GetNextModuleSendFunction(
	IN MV_PVOID self_extension,
	OUT MV_PVOID *next_extension,
	OUT MV_VOID (**next_function)(MV_PVOID , PMV_Request)
	)
{
	PHBA_Extension pHBA = (PHBA_Extension)Module_GetHBAExtension(self_extension);
	MV_U16 module_id = Module_GetModuleId(self_extension);

	module_id++;
	*next_extension = pHBA->Module_Manage.resource[module_id].module_extension;
	*next_function = module_set[module_id].module_sendrequest;
}

MV_VOID 
HBA_GetNextModuleExtension(
	IN MV_PVOID self_extension,
	OUT MV_PVOID *next_extension
	)
{
	PHBA_Extension pHBA = (PHBA_Extension)Module_GetHBAExtension(self_extension);
	MV_U16 module_id = Module_GetModuleId(self_extension);

	module_id++;
	*next_extension = pHBA->Module_Manage.resource[module_id].module_extension;
}

MV_PVOID
HBA_GetModuleExtension(
	IN MV_PVOID self_extension,
	IN MV_U8 module_id
	)
{
	PHBA_Extension pHBA = (PHBA_Extension)Module_GetHBAExtension(self_extension);

	MV_DASSERT( module_id<MAX_MODULE_NUMBER );
	return pHBA->Module_Manage.resource[module_id].module_extension;
}

/* Seconds since midnight, Jan 1, 1970 */
MV_U32 HBA_GetTimeInSecond(void)
{
	/*
	 * Seconds since January 1, 1970, 00:00:00 GMT. 
	 * A negative number is the number of milliseconds before January 1, 1970, 00:00:00 GMT.
	 */
	struct timeval tv;
	do_gettimeofday(&tv);
	return (MV_U32)tv.tv_sec;
}

/* Millisecond passed in this day */
MV_U32 HBA_GetMillisecondInDay(void)
{
	MV_U32 ret = 0;
	struct timespec tv;
	struct timeval x;
	do_gettimeofday(&x);
	tv.tv_sec = x.tv_sec;
	tv.tv_nsec = x.tv_usec*NSEC_PER_SEC;
	ret = (MV_U32)(((signed long long) tv.tv_sec * NSEC_PER_SEC) + tv.tv_nsec);
	return ret;
}

void hba_spin_lock_irq(spinlock_t* plock)
{
	WARN_ON(irqs_disabled());
	spin_lock_irq(plock);                             	
}

void hba_spin_unlock_irq(spinlock_t* plock)
{
	spin_unlock_irq(plock);                             	
}

void hba_swap_buf_le16(u16 *buf, unsigned int words)
{
#ifdef __BIG_ENDIAN
	unsigned int i;

	for (i=0; i < words; i++)
                buf[i] = le16_to_cpu(buf[i]);
#endif /* __BIG_ENDIAN */
}

#ifdef __AC_PROF__
unsigned long __hba_current_time(void)
{
	return jiffies;
}
#endif /* __AC_PROF__ */

#ifdef SUPPORT_EVENT
MV_BOOLEAN HBA_AddEvent( 
	IN MV_PVOID extension,
	IN MV_U32 eventID,
	IN MV_U16 deviceID,        
	IN MV_U8 severityLevel,        
	IN MV_U8 param_cnt, 
	IN MV_PU32 params
	)
{
	PHBA_Extension pHBA = (PHBA_Extension)Module_GetHBAExtension(extension);
	PDriver_Event_Entry pEvent;
	static MV_U32 sequenceNo = 1;
	if (param_cnt > MAX_EVENT_PARAMS)
		return MV_FALSE;

	if ( List_Empty(&pHBA->Free_Events) )
	{
		// No free entry, we need to reuse the oldest entry from Stored_Events.
		MV_ASSERT(!List_Empty(&pHBA->Stored_Events));
		MV_ASSERT(pHBA->Num_Stored_Events == MAX_EVENTS);
		pEvent = List_GetFirstEntry((&pHBA->Stored_Events), Driver_Event_Entry, Queue_Pointer);
	}
	else
	{
		pEvent = List_GetFirstEntry((&pHBA->Free_Events), Driver_Event_Entry, Queue_Pointer);
		pHBA->Num_Stored_Events++;
		MV_ASSERT(pHBA->Num_Stored_Events <= MAX_EVENTS);
	}

	pEvent->Event.AdapterID = pHBA->Adapter_Device_Number;  
	pEvent->Event.EventID = eventID; 
	pEvent->Event.SequenceNo = sequenceNo++;
	pEvent->Event.Severity = severityLevel;
	pEvent->Event.DeviceID = deviceID;
//	pEvent->Event.Param_Cnt = param_cnt;
	pEvent->Event.TimeStamp = HBA_GetTimeInSecond();

	if (param_cnt > 0 && params != NULL)
		MV_CopyMemory( (MV_PVOID)pEvent->Event.Params, (MV_PVOID)params, param_cnt * 4 );

	List_AddTail( &pEvent->Queue_Pointer, &pHBA->Stored_Events );

	return MV_TRUE;
}
#endif /* SUPPORT_EVENT */
