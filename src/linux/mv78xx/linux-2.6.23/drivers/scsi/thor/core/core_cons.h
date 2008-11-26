#ifndef _CORE_DIRVER_CONSOLIDATE_H
#define _CORE_DRIVER_CONSOLIDATE_H

#include "core_inter.h"
#include "core_exp.h"

/*
 * When you plug-in this command consolidate sub-module to some module
 * Please define the following the definition.
 * This is maintained by caller.
 */
/* Get the consolidate sub module extension */
#define CONS_GET_EXTENSION(This)					\
	(((PCore_Driver_Extension)(This))->pConsolid_Extent)

/* Get the device related information consolidate module needs */
#define CONS_GET_DEVICE(This, Device_Id)	\
	&(((PCore_Driver_Extension)(This))->pConsolid_Device[(Device_Id)])

/* For this device or port, is there any request running? If yes, busy. */
#define CONS_DEVICE_IS_BUSY(This, deviceId)	\
	(((PCore_Driver_Extension)(This))->Ports[PATA_MapPortId(deviceId)].Running_Slot!=0)

extern void Core_InternalSendRequest(MV_PVOID This, PMV_Request pReq);
/* In case there is something wrong. We need resend these requests and by pass them. */
#define CONS_SEND_REQUEST	Core_InternalSendRequest

#endif

