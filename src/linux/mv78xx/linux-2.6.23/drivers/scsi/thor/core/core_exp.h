#if !defined(CORE_EXPOSE_H)
#define CORE_EXPOSE_H

/* Product device id */
#define VENDOR_ID                           0x11AB

#define DEVICE_ID_THORLITE_2S1P             0x6121
#define DEVICE_ID_THORLITE_0S1P             0x6101
#define DEVICE_ID_THORLITE_1S1P             0x6111
#define DEVICE_ID_THOR_4S1P                 0x6141
#define DEVICE_ID_THOR_4S1P_NEW             0x6145
/* Revision ID starts from B1 */
#define DEVICE_ID_THORLITE_2S1P_WITH_FLASH  0x6122

MV_U32 Core_ModuleGetResourceQuota(enum Resource_Type type, MV_U16 maxIo);
void Core_ModuleInitialize(MV_PVOID, MV_U32, MV_U16);
void Core_ModuleStart(MV_PVOID);
void Core_ModuleShutdown(MV_PVOID);
void Core_ModuleNotification(MV_PVOID, enum Module_Event, MV_PVOID);
void Core_ModuleSendRequest(MV_PVOID, PMV_Request);
void Core_ModuleMonitor(MV_PVOID);
void Core_ModuleReset(MV_PVOID pExtension);
#ifdef _OS_BIOS
void Core_ReInitBaseAddress(MV_PVOID This);
#endif

MV_BOOLEAN Core_InterruptServiceRoutine(MV_PVOID This);

#ifdef SUPPORT_ERROR_HANDLING
#define REQUEST_TIME_OUT			5		// in multiples of TIMER_INTERVAL, see hba_timer.h
#endif

void mvRemoveDeviceWaitingList(MV_PVOID This, MV_U16 deviceId,
			       MV_BOOLEAN returnOSRequest);
void mvRemovePortWaitingList( MV_PVOID This, MV_U8 portId );

#endif /* CORE_EXPOSE_H */

