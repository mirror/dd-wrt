#if !defined(HBA_EXPOSE_H)
#define HBA_EXPOSE_H

#ifdef SUPPORT_EVENT
#include "com_event_struct.h"
#include "com_event_define.h"
#endif

/*
 * Module_Interface function table
 */
MV_U32 HBA_ModuleGetResourceQuota(enum Resource_Type type, MV_U16 maxIo);
void HBA_ModuleInitialize(MV_PVOID, MV_U32, MV_U16);
void HBA_ModuleStart(MV_PVOID);
void HBA_ModuleShutdown(MV_PVOID);
void HBA_ModuleNotification(MV_PVOID, enum Module_Event, MV_U32);
void HBA_ModuleSendRequest(MV_PVOID, PMV_Request);
void HBA_ModuleMonitor(MV_PVOID);
void HBA_ModuleReset(MV_PVOID extension);

/*
 * Other exposed functions
 */
void HBA_ModuleStarted(MV_PVOID extension);

void HBA_GetResource(
	MV_PVOID extension,
	enum Resource_Type type,
	MV_PVOID resource
	);
/* 
 * For HBA_GetResource. If the type is RESOURCE_UNCACHED_MEMORY, 
 * resource data type is PAssigned_Uncached_Memory.
 */
typedef struct _Assigned_Uncached_Memory
{
	MV_PVOID			Virtual_Address;
	MV_PHYSICAL_ADDR	Physical_Address;
	MV_U32				Byte_Size;
	MV_U32				Reserved0;
} Assigned_Uncached_Memory, *PAssigned_Uncached_Memory;

typedef struct _Controller_Infor
{
	MV_LPVOID Base_Address[MAX_BASE_ADDRESS];
	MV_U16 Vendor_Id;
	MV_U16 Device_Id;
	MV_U8 Revision_Id;
	MV_U8 Reserved[3];
} Controller_Infor, *PController_Infor;

#ifdef SUPPORT_EVENT
// wrapper for DriverEvent, needed to implement queue
typedef struct _Driver_Event_Entry
{
	List_Head Queue_Pointer;
	DriverEvent Event;
} Driver_Event_Entry, *PDriver_Event_Entry;
#endif

void HBA_GetControllerInfor(
	IN MV_PVOID extension,
	OUT PController_Infor pController
	);

void HBA_SleepMillisecond(
	IN MV_PVOID extension,
	IN MV_U32 millisecond
	);

void HBA_SleepMicrosecond(
	IN MV_PVOID extension,
	IN MV_U32 microsecond
	);

void HBA_RequestTimer(
	IN MV_PVOID extension,
	IN MV_U32 millisecond,
	MV_VOID (*TimerService) (MV_PVOID)
	);

void HBA_GetNextModuleSendFunction(
	IN MV_PVOID self_extension,
	OUT MV_PVOID *next_extension,
	OUT MV_VOID (**next_function)(MV_PVOID , PMV_Request)
	);

MV_PVOID
HBA_GetModuleExtension(
	IN MV_PVOID self_extension,
	IN MV_U8 module_id
	);

void HBA_GetNextModuleExtension(
	IN MV_PVOID self_extension,
	OUT MV_PVOID *next_extension
	);

/* It returns the seconds since midnight, Jan 1, 1970*/
MV_U32 HBA_GetTimeInSecond(void);

/* It returns the millisecond since midnight. It's in one day only. */
MV_U32 HBA_GetMillisecondInDay(void);

#ifdef SUPPORT_EVENT
MV_BOOLEAN HBA_AddEvent( 
	IN MV_PVOID extension,
	IN MV_U32 eventId,
	IN MV_U16 deviceId,
	IN MV_U8 severityLevel,
	IN MV_U8 param_cnt,
	IN MV_PU32 params
	);
#endif

void mvGetAdapterInfo( MV_PVOID This, PMV_Request pReq );

void hba_spin_lock_irq(spinlock_t* plock);
void hba_spin_unlock_irq(spinlock_t* plock);
void hba_swap_buf_le16(u16 *buf, unsigned int words);

#ifdef __AC_PROF__
unsigned long __hba_current_time(void);
#endif /* __AC_PROF__ */

#ifdef SUPPORT_EVENT
void mvGetEvent( MV_PVOID This, PMV_Request pReq );
#endif /* SUPPORT_EVENT */

#endif /* HBA_EXPOSE_H */
